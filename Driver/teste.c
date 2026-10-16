/* ============================================================================
 * main.c
 *
 * Controle em C para a GPU customizada (FPGA) via MMIO, usando
 * exclusivamente as funções do driver Assembly declaradas em gpu_isa.h.
 *
 * Este arquivo não reimplementa nada do protocolo de FIFO/MMIO -- toda a
 * comunicação com o hardware passa pelas funções asm_*, que já tratam
 * corretamente o backpressure da fila.
 *
 * Compilar com algo como:
 *
 *     gcc -O2 -Wall -o gpu_demo main.c gpu_driver.S -lm
 *
 * ============================================================================
 */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <signal.h>
#include <math.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>

#include "gpu_isa.h"


/* ============================================================================
 * MAPEAMENTO DE MEMÓRIA
 * ============================================================================ */

#define MMIO_BASE   0xFF200000UL
#define MAP_SIZE    0x1000

static volatile uint32_t *mmio;


/* ============================================================================
 * ENCERRAMENTO LIMPO COM CTRL+C
 *
 * Em vez de deixar o SIGINT matar o processo no meio de uma cena (o que
 * deixaria o mmap/fd abertos de forma abrupta), capturamos o sinal e usamos
 * uma flag para sair do loop da cena de volta para o menu principal.
 * ============================================================================ */

static volatile sig_atomic_t rodando = 1;

static void trata_sigint(int sinal) {
    (void)sinal;
    rodando = 0;
}


/* ============================================================================
 * GEOMETRIA E LIMITES LÓGICOS DO HARDWARE
 * ============================================================================ */

#define TELA_LARGURA    320
#define TELA_ALTURA     240

/* Limites aproximados informados para os sprites (16x16 lógicos) */
#define SPRITE_X_MAX    304
#define SPRITE_Y_MAX    224

#define NUM_SPRITES     32

/* IMPORTANTE: o campo de índice de POLI_DEFINIR/POLI_MOVER na ISA só tem
 * 3 bits (0 a 7). O Motor_Rasterizador guarda 10 posições internamente,
 * mas as posições 8 e 9 não são endereçáveis por essa instrução -- por
 * isso ficamos limitados a 8 polígonos aqui, não 10. */
#define NUM_POLIGONOS   8

#define DIR_CIMA        0
#define DIR_BAIXO       1
#define DIR_ESQUERDA    2
#define DIR_DIREITA     3

#define POLI_RETANGULO  0
#define POLI_TRIANGULO  1

/* Registrador da GPU reservado como "rascunho" de X/Y, recarregado antes
 * de cada SPR_CRIAR/POLI_DEFINIR. */
#define REG_XY          7

/* Índice de paleta reservado para o efeito de ciclo de cor (ver
 * cor_ciclica()). Qualquer índice livre de 6 a 255 serve. */
#define INDICE_COR_CICLICA  20


/* ============================================================================
 * EMPACOTAMENTO DE X/Y
 *
 * Tanto SPR_CRIAR quanto POLI_DEFINIR recebem X/Y através de um registrador
 * do banco, no formato [16:8]=X (9 bits) / [7:0]=Y (8 bits).
 * ============================================================================ */

static inline uint32_t empacotar_xy(int x, int y) {
    return ((uint32_t)(x & 0x1FF) << 8) | (uint32_t)(y & 0xFF);
}


/* ============================================================================
 * CORES
 *
 * Formato RRRGGGBB (8 bits): 3 bits de vermelho, 3 de verde, 2 de azul.
 * ============================================================================ */

static inline uint8_t cor_rgb(int r, int g, int b) {
    return (uint8_t)(((r & 0x7) << 5) | ((g & 0x7) << 2) | (b & 0x3));
}

/* Gera uma cor que varre o círculo cromático conforme a fase (radianos).
 * Usada para o efeito de "ciclo de cor" via reprogramação de paleta. */
static uint8_t cor_ciclica(float fase) {

    int r = (int)lroundf(sinf(fase)              * 3.5f + 3.5f);
    int g = (int)lroundf(sinf(fase + 2.094f)      * 3.5f + 3.5f); /* +120 graus */
    int b = (int)lroundf(sinf(fase + 4.188f)      * 1.5f + 1.5f); /* +240 graus */

    if (r > 7) r = 7; if (r < 0) r = 0;
    if (g > 7) g = 7; if (g < 0) g = 0;
    if (b > 3) b = 3; if (b < 0) b = 0;

    return cor_rgb(r, g, b);
}

/* Programa um conjunto de cores extras na paleta, além das 6 padrão
 * (0=transparente, 1=branco, 2=vermelho, 3=verde, 4=azul, 5=amarelo) já
 * inicializadas no hardware. */
static void configurar_paleta_extra(void) {

    asm_pale_definir(mmio, cor_rgb(7, 3, 0), 6);   /* laranja       */
    asm_pale_definir(mmio, cor_rgb(7, 0, 3), 7);   /* rosa          */
    asm_pale_definir(mmio, cor_rgb(5, 0, 3), 8);   /* magenta       */
    asm_pale_definir(mmio, cor_rgb(0, 7, 3), 9);   /* ciano         */
    asm_pale_definir(mmio, cor_rgb(4, 0, 7), 10);  /* roxo          */
    asm_pale_definir(mmio, cor_rgb(3, 5, 0), 11);  /* lima          */
    asm_pale_definir(mmio, cor_rgb(7, 5, 0), 12);  /* âmbar         */
    asm_pale_definir(mmio, cor_rgb(0, 4, 4), 13);  /* petróleo      */
}


/* ============================================================================
 * FÍSICA: SPRITES E POLÍGONOS
 *
 * Posições guardadas em ponto flutuante para permitir velocidades
 * fracionárias. A cada quadro, só enviamos para a GPU a QUANTIDADE de
 * passos de 1 pixel necessária para alcançar a nova posição inteira --
 * nunca a posição em si.
 * ============================================================================ */

typedef struct {
    float x, y;
    float vx, vy;
    int   mif;
} SpriteFisica;

typedef struct {
    float x, y;
    float vx, vy;
    int   largura, altura;
    int   tipo;
    int   reg_cor;
} PoligonoFisica;


static inline void mover_sprite_n(unsigned indice, unsigned direcao, int passos) {
    for (int i = 0; i < passos; i++) {
        asm_spr_mover(mmio, indice, direcao);
    }
}

static inline void mover_poligono_n(unsigned indice, unsigned direcao, int passos) {
    for (int i = 0; i < passos; i++) {
        asm_poli_mover(mmio, indice, direcao);
    }
}


/* Aplica velocidade + rebatimento nas bordas de um sprite, e envia à GPU
 * só a diferença de posição inteira resultante. */
static void atualizar_sprite(unsigned indice, SpriteFisica *s) {

    int x_antes = (int)lroundf(s->x);
    int y_antes = (int)lroundf(s->y);

    s->x += s->vx;
    s->y += s->vy;

    if (s->x < 0)                   { s->x = 0;              s->vx = -s->vx; }
    else if (s->x > SPRITE_X_MAX)   { s->x = SPRITE_X_MAX;   s->vx = -s->vx; }

    if (s->y < 0)                   { s->y = 0;              s->vy = -s->vy; }
    else if (s->y > SPRITE_Y_MAX)   { s->y = SPRITE_Y_MAX;   s->vy = -s->vy; }

    int x_depois = (int)lroundf(s->x);
    int y_depois = (int)lroundf(s->y);

    int dx = x_depois - x_antes;
    int dy = y_depois - y_antes;

    if (dx > 0)       mover_sprite_n(indice, DIR_DIREITA,  dx);
    else if (dx < 0)  mover_sprite_n(indice, DIR_ESQUERDA, -dx);

    if (dy > 0)       mover_sprite_n(indice, DIR_BAIXO, dy);
    else if (dy < 0)  mover_sprite_n(indice, DIR_CIMA,  -dy);
}


/* Mesma lógica para um polígono, considerando sua largura/altura para o
 * rebatimento nas bordas. */
static void atualizar_poligono(unsigned indice, PoligonoFisica *p) {

    int x_antes = (int)lroundf(p->x);
    int y_antes = (int)lroundf(p->y);

    p->x += p->vx;
    p->y += p->vy;

    if (p->x < 0) {
        p->x = 0;
        p->vx = -p->vx;
    }
    else if (p->x + p->largura > TELA_LARGURA) {
        p->x = TELA_LARGURA - p->largura;
        p->vx = -p->vx;
    }

    if (p->y < 0) {
        p->y = 0;
        p->vy = -p->vy;
    }
    else if (p->y + p->altura > TELA_ALTURA) {
        p->y = TELA_ALTURA - p->altura;
        p->vy = -p->vy;
    }

    int x_depois = (int)lroundf(p->x);
    int y_depois = (int)lroundf(p->y);

    int dx = x_depois - x_antes;
    int dy = y_depois - y_antes;

    if (dx > 0)       mover_poligono_n(indice, DIR_DIREITA,  dx);
    else if (dx < 0)  mover_poligono_n(indice, DIR_ESQUERDA, -dx);

    if (dy > 0)       mover_poligono_n(indice, DIR_BAIXO, dy);
    else if (dy < 0)  mover_poligono_n(indice, DIR_CIMA,  -dy);
}


/* ============================================================================
 * INICIALIZAÇÃO DE SPRITES E POLÍGONOS (comum às cenas 1 e 2)
 * ============================================================================ */

static void inicializar_sprites(SpriteFisica sprites[NUM_SPRITES]) {

    for (int i = 0; i < NUM_SPRITES; i++) {

        sprites[i].x = (float)((i % 8) * 36);
        sprites[i].y = (float)((i / 8) * 50);

        /* Cada sprite recebe uma combinação diferente de velocidade e
         * sinal, para que o enxame nunca se mova em bloco. */
        sprites[i].vx = (float)(1 + (i % 3)) * ((i % 2 == 0) ? 1.0f : -1.0f);
        sprites[i].vy = (float)(1 + ((i / 3) % 3)) * (((i / 8) % 2 == 0) ? 1.0f : -1.0f);

        /* O sprite "i" usa exatamente a textura "i" (0 a 31) */
        sprites[i].mif = i;

        asm_load_reg(mmio, REG_XY, empacotar_xy((int)sprites[i].x, (int)sprites[i].y));
        asm_spr_criar(mmio, REG_XY, i, sprites[i].mif,
                      /* vis   */ 1,
                      /* esp_h */ (i % 3 == 0),
                      /* esp_v */ (i % 5 == 0));
    }
}

static void inicializar_poligonos(PoligonoFisica polys[NUM_POLIGONOS]) {

    static const struct {
        int x, y, largura, altura, tipo, cor_indice;
    } modelo[NUM_POLIGONOS] = {
        {  10,  10, 60, 40, POLI_RETANGULO, 2 },
        { 200,  20, 40, 40, POLI_TRIANGULO, 3 },
        {  50, 150, 30, 50, POLI_RETANGULO, 4 },
        { 250, 160, 50, 50, POLI_TRIANGULO, 5 },
        { 120,  90, 35, 35, POLI_RETANGULO, 6 },
        { 260,  60, 45, 30, POLI_TRIANGULO, 7 },
        {  20, 180, 55, 35, POLI_RETANGULO, 8 },
        { 150, 190, 40, 40, POLI_TRIANGULO, INDICE_COR_CICLICA },
    };

    for (int i = 0; i < NUM_POLIGONOS; i++) {

        polys[i].x       = (float)modelo[i].x;
        polys[i].y       = (float)modelo[i].y;
        polys[i].largura = modelo[i].largura;
        polys[i].altura  = modelo[i].altura;
        polys[i].tipo    = modelo[i].tipo;

        polys[i].vx = (float)(1 + (i % 2)) * ((i % 2 == 0) ? 1.0f : -1.0f);
        polys[i].vy = (float)(1 + ((i % 3) % 2)) * ((i % 3 == 0) ? -1.0f : 1.0f);

        /* Só existem 2 bits para "reg_cor" (registradores 0 a 3), então
         * reaproveitamos ciclicamente -- isso é seguro porque a cor é
         * lida e fixada no instante do POLI_DEFINIR, não continuamente. */
        polys[i].reg_cor = i % 4;
        asm_load_reg(mmio, polys[i].reg_cor, modelo[i].cor_indice);

        asm_load_reg(mmio, REG_XY, empacotar_xy((int)polys[i].x, (int)polys[i].y));
        asm_poli_definir(mmio, REG_XY, i,
                          polys[i].largura, polys[i].altura,
                          polys[i].reg_cor, polys[i].tipo,
                          /* vis */ 1);
    }
}


/* ============================================================================
 * CENA 1 -- CAOS TOTAL
 *
 * Os 32 sprites (cada um com sua própria textura, 0 a 31) e os 8 polígonos
 * quicam pela tela em velocidades diferentes, estilo "logo de DVD". O
 * fundo rola continuamente para a direita.
 * ============================================================================ */

static void cena_caos_total(void) {

    printf("\n=== CENA 1: CAOS TOTAL (32 sprites + 8 poligonos) ===\n");
    printf("Pressione Ctrl+C para voltar ao menu.\n");

    SpriteFisica   sprites[NUM_SPRITES];
    PoligonoFisica polys[NUM_POLIGONOS];

    configurar_paleta_extra();
    inicializar_sprites(sprites);
    inicializar_poligonos(polys);

    uint32_t quadro = 0;

    while (rodando) {

        
        asm_espera_vsync(mmio);
        quadro++;

        for (int i = 0; i < NUM_SPRITES; i++)
            atualizar_sprite(i, &sprites[i]);

        for (int i = 0; i < NUM_POLIGONOS; i++)
            atualizar_poligono(i, &polys[i]);

        /* Fundo rolando bem devagar para a direita */
        if (quadro % 30 == 0)
            asm_bg_mover(mmio, DIR_DIREITA);

        usleep(30000);
    }


    printf("Cena encerrada.\n");
}


/* ============================================================================
 * CENA 2 -- GRAVIDADE E CICLO DE CORES
 *
 * Os sprites caem com gravidade e quicam perdendo energia a cada batida
 * no chão (restituição < 1), enquanto um dos polígonos muda de cor
 * continuamente através da reprogramação de um único índice de paleta
 * (efeito clássico de "palette cycling" de demoscene). O fundo percorre
 * um caminho quadrado, alternando entre as 4 direções.
 * ============================================================================ */

static void cena_gravidade_e_cores(void) {

    printf("\n=== CENA 2: GRAVIDADE E CICLO DE CORES ===\n");
    printf("Pressione Ctrl+C para voltar ao menu.\n");

    SpriteFisica   sprites[NUM_SPRITES];
    PoligonoFisica polys[NUM_POLIGONOS];

    configurar_paleta_extra();
    inicializar_sprites(sprites);
    inicializar_poligonos(polys);

    const float GRAVIDADE   = 0.08f;
    const float RESTITUICAO = 0.85f;

    uint32_t quadro      = 0;
    float    fase_cor    = 0.0f;
    int      direcao_bg  = DIR_DIREITA;

    while (rodando) {

        asm_espera_vsync(mmio);
        quadro++;

        for (int i = 0; i < NUM_SPRITES; i++) {

            sprites[i].vy += GRAVIDADE;

            int x_antes = (int)lroundf(sprites[i].x);
            int y_antes = (int)lroundf(sprites[i].y);

            sprites[i].x += sprites[i].vx;
            sprites[i].y += sprites[i].vy;

            if (sprites[i].x < 0) {
                sprites[i].x = 0;
                sprites[i].vx = -sprites[i].vx;
            }
            else if (sprites[i].x > SPRITE_X_MAX) {
                sprites[i].x = SPRITE_X_MAX;
                sprites[i].vx = -sprites[i].vx;
            }

            if (sprites[i].y > SPRITE_Y_MAX) {
                sprites[i].y  = SPRITE_Y_MAX;
                sprites[i].vy = -sprites[i].vy * RESTITUICAO; /* perde energia ao quicar */
            }
            else if (sprites[i].y < 0) {
                sprites[i].y = 0;
                sprites[i].vy = -sprites[i].vy;
            }

            int x_depois = (int)lroundf(sprites[i].x);
            int y_depois = (int)lroundf(sprites[i].y);

            int dx = x_depois - x_antes;
            int dy = y_depois - y_antes;

            if (dx > 0)       mover_sprite_n(i, DIR_DIREITA,  dx);
            else if (dx < 0)  mover_sprite_n(i, DIR_ESQUERDA, -dx);

            if (dy > 0)       mover_sprite_n(i, DIR_BAIXO, dy);
            else if (dy < 0)  mover_sprite_n(i, DIR_CIMA,  -dy);
        }

        for (int i = 0; i < NUM_POLIGONOS; i++)
            atualizar_poligono(i, &polys[i]);

        /* Ciclo de cores: reprograma a paleta no índice reservado -- o
         * último polígono, que aponta para esse índice, muda de cor
         * sozinho, sem precisar ser redefinido. */
        fase_cor += 0.05f;
        asm_pale_definir(mmio, cor_ciclica(fase_cor), INDICE_COR_CICLICA);

        /* Fundo percorre um caminho quadrado */
        if (quadro % 30 == 0)
            asm_bg_mover(mmio, direcao_bg);

        if (quadro % 30 == 0)
            direcao_bg = (direcao_bg + 1) % 4;
        usleep(30000);
    }

    printf("Cena encerrada.\n");
}


/* ============================================================================
 * CENA 3 -- ÓRBITAS SENOIDAIS
 *
 * Os 32 sprites descrevem curvas de Lissajous (cada um com sua própria
 * combinação de frequência/fase), os polígonos "pulsam" de tamanho ao
 * serem redefinidos continuamente via POLI_DEFINIR, e o fundo faz uma
 * rolagem diagonal lenta.
 * ============================================================================ */

static void cena_orbitas_senoidais(void) {

    printf("\n=== CENA 3: ORBITAS SENOIDAIS ===\n");
    printf("Pressione Ctrl+C para voltar ao menu.\n");

    configurar_paleta_extra();

    float pos_x[NUM_SPRITES], pos_y[NUM_SPRITES];
    float fase[NUM_SPRITES];
    float freq_x[NUM_SPRITES], freq_y[NUM_SPRITES];

    const float centro_x = SPRITE_X_MAX / 2.0f;
    const float centro_y = SPRITE_Y_MAX / 2.0f;
    const float raio_x   = centro_x * 0.9f;
    const float raio_y   = centro_y * 0.9f;

    for (int i = 0; i < NUM_SPRITES; i++) {

        fase[i]   = (float)i * 0.19635f; /* espalha as fases ao redor do círculo */
        freq_x[i] = 1.0f + (i % 4) * 0.5f;
        freq_y[i] = 1.0f + ((i / 4) % 4) * 0.5f;

        pos_x[i] = centro_x + raio_x * sinf(fase[i]);
        pos_y[i] = centro_y + raio_y * cosf(fase[i]);

        asm_load_reg(mmio, REG_XY, empacotar_xy((int)pos_x[i], (int)pos_y[i]));
        asm_spr_criar(mmio, REG_XY, i, i, /* vis */ 1, /* esp_h */ 0, /* esp_v */ 0);
    }

    PoligonoFisica polys[NUM_POLIGONOS];

    static const struct { int x, y, cor, tipo; } base_poly[NUM_POLIGONOS] = {
        {  40,  40, 2, POLI_RETANGULO }, { 120,  40, 3, POLI_TRIANGULO },
        { 200,  40, 4, POLI_RETANGULO }, {  40, 160, 5, POLI_TRIANGULO },
        { 120, 160, 6, POLI_RETANGULO }, { 200, 160, 7, POLI_TRIANGULO },
        {  80, 100, 8, POLI_RETANGULO }, { 230, 100, INDICE_COR_CICLICA, POLI_TRIANGULO },
    };

    for (int i = 0; i < NUM_POLIGONOS; i++) {
        polys[i].x        = (float)base_poly[i].x;
        polys[i].y        = (float)base_poly[i].y;
        polys[i].tipo     = base_poly[i].tipo;
        polys[i].reg_cor  = i % 4;
        asm_load_reg(mmio, polys[i].reg_cor, base_poly[i].cor);
    }

    uint32_t quadro      = 0;
    float    fase_pulso  = 0.0f;
    float    fase_cor    = 0.0f;

    while (rodando) {

        asm_espera_vsync(mmio);
        quadro++;

        for (int i = 0; i < NUM_SPRITES; i++) {

            fase[i] += 0.015f;

            float nova_x = centro_x + raio_x * sinf(fase[i] * freq_x[i]);
            float nova_y = centro_y + raio_y * cosf(fase[i] * freq_y[i]);

            int x_antes = (int)lroundf(pos_x[i]);
            int y_antes = (int)lroundf(pos_y[i]);

            pos_x[i] = nova_x;
            pos_y[i] = nova_y;

            int dx = (int)lroundf(pos_x[i]) - x_antes;
            int dy = (int)lroundf(pos_y[i]) - y_antes;

            /* Limita passos por quadro: evita inundar a fila caso a
             * trigonometria produza um salto grande num único quadro. */
            if (dx >  4) dx =  4; if (dx < -4) dx = -4;
            if (dy >  4) dy =  4; if (dy < -4) dy = -4;

            if (dx > 0)       mover_sprite_n(i, DIR_DIREITA,  dx);
            else if (dx < 0)  mover_sprite_n(i, DIR_ESQUERDA, -dx);

            if (dy > 0)       mover_sprite_n(i, DIR_BAIXO, dy);
            else if (dy < 0)  mover_sprite_n(i, DIR_CIMA,  -dy);
        }

        /* Polígonos "pulsando" de tamanho via redefinição contínua */
        fase_pulso += 0.04f;
        int pulso = (int)lroundf(6.0f * sinf(fase_pulso));

        for (int i = 0; i < NUM_POLIGONOS; i++) {

            int largura = 50 + pulso + (i % 3) * 5;
            int altura  = 50 - pulso + (i % 2) * 5;

            if (largura < 10) largura = 10;
            if (altura  < 10) altura  = 10;

            asm_load_reg(mmio, REG_XY, empacotar_xy((int)polys[i].x, (int)polys[i].y));
            asm_poli_definir(mmio, REG_XY, i, largura, altura,
                              polys[i].reg_cor, polys[i].tipo, /* vis */ 1);
        }

        fase_cor += 0.03f;
        asm_pale_definir(mmio, cor_ciclica(fase_cor), INDICE_COR_CICLICA);

        if (quadro % 30 == 0) asm_bg_mover(mmio, DIR_DIREITA);
        if (quadro % 30 == 0) asm_bg_mover(mmio, DIR_BAIXO);
        usleep(30000);
    }

    printf("Cena encerrada.\n");
}


/* ============================================================================
 * MENU INTERATIVO -- DISPARA MANUALMENTE CADA INSTRUÇÃO
 * ============================================================================ */

static void menu_load_reg(void) {
    unsigned reg;
    int valor;
    printf("\nRegistrador (0-7): ");           scanf("%u", &reg);
    printf("Valor (decimal ou 0x..): ");       scanf("%i", &valor);
    asm_load_reg(mmio, reg, (uint32_t)valor);
    printf("Enviado.\n");
}

static void menu_espera_vsync(void) {
    printf("\nEnviando ESPERA_VSYNC...\n");
    asm_espera_vsync(mmio);
    printf("Instrucao aceita pela fila.\n");
}

static void menu_pale_definir(void) {
    int cor;
    unsigned endereco;
    printf("\nCor (8 bits, RRRGGGBB, decimal ou 0x..): "); scanf("%i", &cor);
    printf("Endereco na paleta (0-255): ");                scanf("%u", &endereco);
    asm_pale_definir(mmio, (uint32_t)cor, endereco);
    printf("Enviado.\n");
}

static void menu_spr_criar(void) {
    unsigned indice, mif, vis, esp_h, esp_v, x, y;
    printf("\nIndice do sprite (0-31): "); scanf("%u", &indice);
    printf("X: ");                         scanf("%u", &x);
    printf("Y: ");                         scanf("%u", &y);
    printf("MIF (0-31): ");                scanf("%u", &mif);
    printf("Visivel (0/1): ");             scanf("%u", &vis);
    printf("Espelhar H (0/1): ");          scanf("%u", &esp_h);
    printf("Espelhar V (0/1): ");          scanf("%u", &esp_v);

    asm_load_reg(mmio, REG_XY, empacotar_xy((int)x, (int)y));
    asm_spr_criar(mmio, REG_XY, indice, mif, vis, esp_h, esp_v);
    printf("Enviado.\n");
}

static void menu_spr_mover(void) {
    unsigned indice, direcao;
    printf("\nIndice do sprite (0-31): ");                  scanf("%u", &indice);
    printf("Direcao (0=Cima,1=Baixo,2=Esq,3=Dir): ");        scanf("%u", &direcao);
    asm_spr_mover(mmio, indice, direcao);
    printf("Enviado.\n");
}

static void menu_bg_tile(void) {
    unsigned indice;
    printf("\nIndice do tilemap (0 ou 1 -- unicos implementados): ");
    scanf("%u", &indice);
    asm_bg_tile(mmio, indice);
    printf("Enviado.\n");
}

static void menu_bg_mover(void) {
    unsigned direcao;
    printf("\nDirecao (0=Cima,1=Baixo,2=Esq,3=Dir): "); scanf("%u", &direcao);
    asm_bg_mover(mmio, direcao);
    printf("Enviado.\n");
}

static void menu_poli_definir(void) {
    unsigned indice, largura, altura, reg_cor, tipo, vis, x, y;
    printf("\nIndice do poligono (0-7): ");  scanf("%u", &indice);
    printf("X: ");                           scanf("%u", &x);
    printf("Y: ");                           scanf("%u", &y);
    printf("Largura: ");                     scanf("%u", &largura);
    printf("Altura: ");                      scanf("%u", &altura);
    printf("Registrador com indice de cor (0-3): "); scanf("%u", &reg_cor);
    printf("Tipo (0=Retangulo,1=Triangulo): "); scanf("%u", &tipo);
    printf("Visivel (0/1): ");               scanf("%u", &vis);

    asm_load_reg(mmio, REG_XY, empacotar_xy((int)x, (int)y));
    asm_poli_definir(mmio, REG_XY, indice, largura, altura, reg_cor, tipo, vis);
    printf("Enviado.\n");
}

static void menu_poli_mover(void) {
    unsigned indice, direcao;
    printf("\nIndice do poligono (0-7): ");                 scanf("%u", &indice);
    printf("Direcao (0=Cima,1=Baixo,2=Esq,3=Dir): ");        scanf("%u", &direcao);
    asm_poli_mover(mmio, indice, direcao);
    printf("Enviado.\n");
}


/* ============================================================================
 * MENU PRINCIPAL
 * ============================================================================ */

static void mostrar_menu(void) {
    printf("\n==================================================\n");
    printf("   GPU CUSTOMIZADA - CONTROLE VIA MMIO (ARM + FPGA)\n");
    printf("==================================================\n");
    printf(" 1  - LOAD_REG\n");
    printf(" 2  - ESPERA_VSYNC\n");
    printf(" 3  - PALE_DEFINIR\n");
    printf(" 4  - SPR_CRIAR\n");
    printf(" 5  - SPR_MOVER\n");
    printf(" 6  - BG_TILE\n");
    printf(" 7  - BG_MOVER\n");
    printf(" 8  - POLI_DEFINIR\n");
    printf(" 9  - POLI_MOVER\n");
    printf("--------------------------------------------------\n");
    printf("10  - Cena: Caos Total (32 sprites + 8 poligonos)\n");
    printf("11  - Cena: Gravidade e Ciclo de Cores\n");
    printf("12  - Cena: Orbitas Senoidais\n");
    printf("--------------------------------------------------\n");
    printf(" 0  - Sair\n");
    printf("Escolha: ");
}

int main(void) {

    signal(SIGINT, trata_sigint);

    int fd = open("/dev/mem", O_RDWR | O_SYNC);
    if (fd < 0) {
        perror("Erro ao abrir /dev/mem");
        return 1;
    }

    void *mapa = mmap(NULL, MAP_SIZE, PROT_READ | PROT_WRITE,
                       MAP_SHARED, fd, MMIO_BASE);

    if (mapa == MAP_FAILED) {
        perror("Erro no mmap");
        close(fd);
        return 1;
    }

    mmio = (volatile uint32_t *)mapa;

    int opcao;

    do {
        /* Permite reentrar numa cena normalmente depois de um Ctrl+C
         * ter encerrado uma cena anterior. */
        rodando = 1;

        mostrar_menu();

        if (scanf("%d", &opcao) != 1) {
            int c;
            while ((c = getchar()) != '\n' && c != EOF) { /* limpa entrada invalida */ }
            continue;
        }

        switch (opcao) {
            case 1:  menu_load_reg();           break;
            case 2:  menu_espera_vsync();       break;
            case 3:  menu_pale_definir();       break;
            case 4:  menu_spr_criar();          break;
            case 5:  menu_spr_mover();          break;
            case 6:  menu_bg_tile();            break;
            case 7:  menu_bg_mover();           break;
            case 8:  menu_poli_definir();       break;
            case 9:  menu_poli_mover();         break;
            case 10: cena_caos_total();         break;
            case 11: cena_gravidade_e_cores();  break;
            case 12: cena_orbitas_senoidais();  break;
            case 0:  printf("Encerrando...\n"); break;
            default: printf("Opcao invalida.\n"); break;
        }

    } while (opcao != 0);

    munmap(mapa, MAP_SIZE);
    close(fd);

    return 0;
}