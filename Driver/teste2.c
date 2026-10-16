/* ============================================================================
 * teste_cenas.c
 * GPU Customizada (FPGA) - Cenas Criativas 2.0
 * Compilar: gcc -std=gnu99 teste_cenas.c gpu_driver.S -o programa_gpu -lm
 * ============================================================================ */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <signal.h>
#include <math.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>

#include "gpu_isa.h"

#define MMIO_BASE   0xFF200000UL
#define MAP_SIZE    0x1000

static volatile uint32_t *mmio;
static volatile sig_atomic_t rodando = 1;

static void trata_sigint(int sinal) {
    (void)sinal;
    rodando = 0;
}

// Limites da Tela
#define TELA_LARGURA    320
#define TELA_ALTURA     240
#define SPRITE_X_MAX    304
#define SPRITE_Y_MAX    224
#define NUM_SPRITES     32
#define NUM_POLIGONOS   8

// Direções
#define DIR_CIMA        0
#define DIR_BAIXO       1
#define DIR_ESQUERDA    2
#define DIR_DIREITA     3

// Formas
#define POLI_RETANGULO  0
#define POLI_TRIANGULO  1

#define REG_XY          7
#define INDICE_COR_CICLICA  20

/* ============================================================================
 * FUNÇÕES AUXILIARES E FÍSICA
 * ============================================================================ */
static inline uint32_t empacotar_xy(int x, int y) {
    return ((uint32_t)(x & 0x1FF) << 8) | (uint32_t)(y & 0xFF);
}

static inline uint8_t cor_rgb(int r, int g, int b) {
    return (uint8_t)(((r & 0x7) << 5) | ((g & 0x7) << 2) | (b & 0x3));
}

static uint8_t cor_ciclica(float fase) {
    int r = (int)lroundf(sinf(fase)              * 3.5f + 3.5f);
    int g = (int)lroundf(sinf(fase + 2.094f)     * 3.5f + 3.5f); 
    int b = (int)lroundf(sinf(fase + 4.188f)     * 1.5f + 1.5f); 
    if (r > 7) r = 7; if (r < 0) r = 0;
    if (g > 7) g = 7; if (g < 0) g = 0;
    if (b > 3) b = 3; if (b < 0) b = 0;
    return cor_rgb(r, g, b);
}

static void configurar_paleta_extra(void) {
    asm_pale_definir(mmio, cor_rgb(7, 3, 0), 6);   // laranja
    asm_pale_definir(mmio, cor_rgb(7, 0, 3), 7);   // rosa
    asm_pale_definir(mmio, cor_rgb(5, 0, 3), 8);   // magenta
    asm_pale_definir(mmio, cor_rgb(0, 7, 3), 9);   // ciano
    asm_pale_definir(mmio, cor_rgb(4, 0, 7), 10);  // roxo
    asm_pale_definir(mmio, cor_rgb(3, 5, 0), 11);  // lima
    asm_pale_definir(mmio, cor_rgb(7, 5, 0), 12);  // âmbar
    asm_pale_definir(mmio, cor_rgb(0, 4, 4), 13);  // petróleo
}

static inline void mover_sprite_n(unsigned indice, unsigned direcao, int passos) {
    for (int i = 0; i < passos; i++) asm_spr_mover(mmio, indice, direcao);
}
static inline void mover_poligono_n(unsigned indice, unsigned direcao, int passos) {
    for (int i = 0; i < passos; i++) asm_poli_mover(mmio, indice, direcao);
}


/* ============================================================================
 * CENA 1: INVASÃO ESPACIAL (Arcade Shooter)
 * O Sprite 0 é a sua nave, os polígonos são os lasers subindo, e os outros 
 * sprites são inimigos varrendo a tela. O fundo simula o espaço descendo.
 * ============================================================================ */
static void cena_invasao_espacial(void) {
    printf("\n=== CENA 1: INVASAO ESPACIAL ===\n");
    printf("Pressione Ctrl+C para voltar.\n");

    asm_bg_tile(mmio, 1);
    configurar_paleta_extra();

    // Sprite 0: Nossa Nave
    float nave_x = 144.0f;
    float nave_y = 200.0f;
    asm_load_reg(mmio, REG_XY, empacotar_xy((int)nave_x, (int)nave_y));
    asm_spr_criar(mmio, REG_XY, 0, 0, 1, 0, 0);

    // Sprites 1 a 31: Inimigos
    float in_x[NUM_SPRITES], in_y[NUM_SPRITES];
    float in_vx = 3.0f;
    for(int i = 1; i < NUM_SPRITES; i++) {
        in_x[i] = (i % 8) * 35;
        in_y[i] = (i / 8) * 30 + 10;
        asm_load_reg(mmio, REG_XY, empacotar_xy((int)in_x[i], (int)in_y[i]));
        asm_spr_criar(mmio, REG_XY, i, i, 1, 0, 0);
    }

    // Polígonos: Tiros de Laser (Subindo)
    float laser_x[NUM_POLIGONOS], laser_y[NUM_POLIGONOS];
    for(int i=0; i<NUM_POLIGONOS; i++) {
        laser_x[i] = nave_x + 6;
        laser_y[i] = nave_y - (i * 40); // Espalhados verticalmente
        asm_load_reg(mmio, i%4, 2); // Cor vermelha
        asm_load_reg(mmio, REG_XY, empacotar_xy((int)laser_x[i], (int)laser_y[i]));
        asm_poli_definir(mmio, REG_XY, i, 4, 15, i%4, POLI_RETANGULO, 1);
    }

    uint32_t quadro = 0;
    while(rodando) {
        asm_espera_vsync(mmio);
        quadro++;

        if (quadro % 2 != 0) continue; // Controle de velocidade suave

        // Move Inimigos (Zigue-zague)
        for(int i = 1; i < NUM_SPRITES; i++) {
            in_x[i] += in_vx;
            if (in_x[i] > SPRITE_X_MAX || in_x[i] < 0) {
                in_vx = -in_vx; // Bateu na borda, inverte a tropa
                for(int j=1; j<NUM_SPRITES; j++) {
                    mover_sprite_n(j, DIR_BAIXO, 10); // Tropa desce!
                    in_y[j] += 10;
                }
            }
            if(in_vx > 0) mover_sprite_n(i, DIR_DIREITA, abs((int)in_vx));
            else mover_sprite_n(i, DIR_ESQUERDA, abs((int)in_vx));
        }

        // Move Tiros (Para cima)
        for(int i=0; i<NUM_POLIGONOS; i++) {
            laser_y[i] -= 8;
            if (laser_y[i] < 0) {
                // Laser saiu da tela, recria ele na ponta da nave!
                laser_y[i] = nave_y;
                asm_load_reg(mmio, REG_XY, empacotar_xy((int)laser_x[i], (int)laser_y[i]));
                asm_poli_definir(mmio, REG_XY, i, 4, 15, i%4, POLI_RETANGULO, 1);
            } else {
                mover_poligono_n(i, DIR_CIMA, 8);
            }
        }

        // Fundo simulando voo para frente (descendo a tela)
        if (quadro % 5 == 0) asm_bg_mover(mmio, DIR_BAIXO);

        usleep(15000);
    }
}

/* ============================================================================
 * CENA 2: CHUVA "MATRIX"
 * Todos os sprites e polígonos caem do topo da tela em velocidades 
 * aleatórias. Quando chegam ao fundo, reaparecem magicamente em cima.
 * ============================================================================ */
static void cena_chuva_matrix(void) {
    printf("\n=== CENA 2: CHUVA MATRIX ===\n");
    printf("Pressione Ctrl+C para voltar.\n");

    asm_bg_tile(mmio, 0); // Fundo estático escuro
    configurar_paleta_extra();

    float s_y[NUM_SPRITES];
    int s_vy[NUM_SPRITES];

    for(int i = 0; i < NUM_SPRITES; i++) {
        s_y[i] = (rand() % SPRITE_Y_MAX);
        s_vy[i] = (rand() % 5) + 2; // Velocidades variadas
        asm_load_reg(mmio, REG_XY, empacotar_xy(i * 9, (int)s_y[i]));
        asm_spr_criar(mmio, REG_XY, i, i, 1, 0, 0);
    }

    float p_y[NUM_POLIGONOS];
    int p_vy[NUM_POLIGONOS];
    for(int i = 0; i < NUM_POLIGONOS; i++) {
        p_y[i] = (rand() % TELA_ALTURA);
        p_vy[i] = (rand() % 4) + 3;
        asm_load_reg(mmio, i%4, 11); // Cor verde lima para as "gotas grossas"
        asm_load_reg(mmio, REG_XY, empacotar_xy(i * 35 + 15, (int)p_y[i]));
        asm_poli_definir(mmio, REG_XY, i, 4, 25, i%4, POLI_RETANGULO, 1);
    }

    uint32_t quadro = 0;
    while(rodando) {
        asm_espera_vsync(mmio);
        quadro++;

        if (quadro % 2 != 0) continue;

        // Anima Sprites
        for(int i = 0; i < NUM_SPRITES; i++) {
            s_y[i] += s_vy[i];
            if (s_y[i] > SPRITE_Y_MAX) {
                s_y[i] = 0; // Reseta no topo usando Redefinição Absoluta!
                asm_load_reg(mmio, REG_XY, empacotar_xy(i * 9, (int)s_y[i]));
                asm_spr_criar(mmio, REG_XY, i, i, 1, 0, 0);
            } else {
                mover_sprite_n(i, DIR_BAIXO, s_vy[i]);
            }
        }

        // Anima Poligonos
        for(int i = 0; i < NUM_POLIGONOS; i++) {
            p_y[i] += p_vy[i];
            if (p_y[i] > TELA_ALTURA) {
                p_y[i] = 0; // Reseta no topo
                asm_load_reg(mmio, REG_XY, empacotar_xy(i * 35 + 15, (int)p_y[i]));
                asm_poli_definir(mmio, REG_XY, i, 4, 25, i%4, POLI_RETANGULO, 1);
            } else {
                mover_poligono_n(i, DIR_BAIXO, p_vy[i]);
            }
        }

        // Fundo move-se super devagar para a direita
        if (quadro % 10 == 0) asm_bg_mover(mmio, DIR_DIREITA);

        usleep(20000);
    }
}

/* ============================================================================
 * CENA 3: RADAR SCANNER
 * Um polígono gigante gira a largura simulando um radar. 
 * Sprites aparecem apenas quando o radar cruza com eles!
 * ============================================================================ */
static void cena_radar(void) {
    printf("\n=== CENA 3: PONG SOLITÁRIO COM CICLO DE COR ===\n");
    printf("Pressione Ctrl+C para voltar.\n");

    asm_bg_tile(mmio, 1);
    configurar_paleta_extra();

    // A BOLA (Sprite 0)
    float bola_x = 150.0f, bola_y = 100.0f;
    float bola_vx = 4.0f, bola_vy = 4.0f;
    asm_load_reg(mmio, REG_XY, empacotar_xy((int)bola_x, (int)bola_y));
    asm_spr_criar(mmio, REG_XY, 0, 5, 1, 0, 0); // MIF 5 para a bola

    // AS RAQUETES (Polígonos nas bordas)
    // 0: Esquerda, 1: Direita, 2: Cima, 3: Baixo
    asm_load_reg(mmio, 0, INDICE_COR_CICLICA);
    asm_load_reg(mmio, REG_XY, empacotar_xy(0, 80));
    asm_poli_definir(mmio, REG_XY, 0, 10, 80, 0, POLI_RETANGULO, 1);

    asm_load_reg(mmio, REG_XY, empacotar_xy(310, 80));
    asm_poli_definir(mmio, REG_XY, 1, 10, 80, 0, POLI_RETANGULO, 1);

    asm_load_reg(mmio, REG_XY, empacotar_xy(100, 0));
    asm_poli_definir(mmio, REG_XY, 2, 100, 10, 0, POLI_RETANGULO, 1);

    asm_load_reg(mmio, REG_XY, empacotar_xy(100, 230));
    asm_poli_definir(mmio, REG_XY, 3, 100, 10, 0, POLI_RETANGULO, 1);

    uint32_t quadro = 0;
    float fase_cor = 0.0f;

    while(rodando) {
        asm_espera_vsync(mmio);
        quadro++;
        if (quadro % 2 != 0) continue;

        bola_x += bola_vx;
        bola_y += bola_vy;

        // Rebatimento nas Raquetes/Bordas
        if (bola_x < 10) { bola_x = 10; bola_vx = -bola_vx; }
        else if (bola_x > 294) { bola_x = 294; bola_vx = -bola_vx; }

        if (bola_y < 10) { bola_y = 10; bola_vy = -bola_vy; }
        else if (bola_y > 214) { bola_y = 214; bola_vy = -bola_vy; }

        // Aplica o movimento da bola
        if (bola_vx > 0) mover_sprite_n(0, DIR_DIREITA, abs((int)bola_vx));
        else mover_sprite_n(0, DIR_ESQUERDA, abs((int)bola_vx));

        if (bola_vy > 0) mover_sprite_n(0, DIR_BAIXO, abs((int)bola_vy));
        else mover_sprite_n(0, DIR_CIMA, abs((int)bola_vy));

        // As raquetes se expandem e encolhem!
        int oscilacao = (int)lroundf(20.0f * sinf(quadro * 0.05f));
        asm_load_reg(mmio, REG_XY, empacotar_xy(100 - oscilacao, 0));
        asm_poli_definir(mmio, REG_XY, 2, 100 + oscilacao*2, 10, 0, POLI_RETANGULO, 1);

        asm_load_reg(mmio, REG_XY, empacotar_xy(100 - oscilacao, 230));
        asm_poli_definir(mmio, REG_XY, 3, 100 + oscilacao*2, 10, 0, POLI_RETANGULO, 1);

        // Ciclo de Cor nas raquetes (Techno Vibe)
        fase_cor += 0.1f;
        asm_pale_definir(mmio, cor_ciclica(fase_cor), INDICE_COR_CICLICA);

        usleep(15000);
    }
}


/* ============================================================================
 * MENU E MAIN (Mantidos exatos para preservar seus testes individuais)
 * ============================================================================ */
static void menu_load_reg(void) {
    unsigned reg; int valor;
    printf("\nRegistrador (0-7): "); scanf("%u", &reg);
    printf("Valor (decimal ou 0x..): "); scanf("%i", &valor);
    asm_load_reg(mmio, reg, (uint32_t)valor); printf("Enviado.\n");
}
static void menu_espera_vsync(void) {
    printf("\nEnviando ESPERA_VSYNC...\n");
    asm_espera_vsync(mmio); printf("Instrucao aceita pela fila.\n");
}
static void menu_pale_definir(void) {
    int cor; unsigned endereco;
    printf("\nCor (RRRGGGBB): "); scanf("%i", &cor);
    printf("Endereco (0-255): "); scanf("%u", &endereco);
    asm_pale_definir(mmio, (uint32_t)cor, endereco); printf("Enviado.\n");
}
static void menu_spr_criar(void) {
    unsigned i, m, v, eh, ev, x, y;
    printf("Indice(0-31): "); scanf("%u", &i);
    printf("X: "); scanf("%u", &x); printf("Y: "); scanf("%u", &y);
    printf("MIF(0-31): "); scanf("%u", &m);
    printf("Vis(0/1): "); scanf("%u", &v);
    printf("EspH(0/1): "); scanf("%u", &eh); printf("EspV(0/1): "); scanf("%u", &ev);
    asm_load_reg(mmio, REG_XY, empacotar_xy((int)x, (int)y));
    asm_spr_criar(mmio, REG_XY, i, m, v, eh, ev); printf("Enviado.\n");
}
static void menu_spr_mover(void) {
    unsigned i, d;
    printf("Indice(0-31): "); scanf("%u", &i);
    printf("Direcao(0C,1B,2E,3D): "); scanf("%u", &d);
    asm_spr_mover(mmio, i, d); printf("Enviado.\n");
}
static void menu_bg_tile(void) {
    unsigned i; printf("Indice cenario: "); scanf("%u", &i);
    asm_bg_tile(mmio, i); printf("Enviado.\n");
}
static void menu_bg_mover(void) {
    unsigned d; printf("Direcao(0C,1B,2E,3D): "); scanf("%u", &d);
    asm_bg_mover(mmio, d); printf("Enviado.\n");
}
static void menu_poli_definir(void) {
    unsigned i, l, a, c, t, v, x, y;
    printf("Indice(0-7): "); scanf("%u", &i);
    printf("X: "); scanf("%u", &x); printf("Y: "); scanf("%u", &y);
    printf("Larg: "); scanf("%u", &l); printf("Alt: "); scanf("%u", &a);
    printf("Cor(0-3): "); scanf("%u", &c);
    printf("Tipo(0R,1T): "); scanf("%u", &t); printf("Vis(0/1): "); scanf("%u", &v);
    asm_load_reg(mmio, REG_XY, empacotar_xy((int)x, (int)y));
    asm_poli_definir(mmio, REG_XY, i, l, a, c, t, v); printf("Enviado.\n");
}
static void menu_poli_mover(void) {
    unsigned i, d;
    printf("Indice(0-7): "); scanf("%u", &i);
    printf("Direcao(0C,1B,2E,3D): "); scanf("%u", &d);
    asm_poli_mover(mmio, i, d); printf("Enviado.\n");
}

static void mostrar_menu(void) {
    printf("\n==================================================\n");
    printf("   GPU FPGA - CENAS E TESTES MANUAIS\n");
    printf("==================================================\n");
    printf(" 1 - LOAD_REG        2 - ESPERA_VSYNC\n");
    printf(" 3 - PALE_DEFINIR    4 - SPR_CRIAR      5 - SPR_MOVER\n");
    printf(" 6 - BG_TILE         7 - BG_MOVER\n");
    printf(" 8 - POLI_DEFINIR    9 - POLI_MOVER\n");
    printf("--------------------------------------------------\n");
    printf("10 - Cena: Invasao Espacial (Arcade Shooter)\n");
    printf("11 - Cena: Chuva Matrix\n");
    printf("12 - Cena: Pong Solitario + Cores\n");
    printf(" 0 - Sair\nEscolha: ");
}

int main(void) {
    signal(SIGINT, trata_sigint);
    int fd = open("/dev/mem", O_RDWR | O_SYNC);
    if (fd < 0) return 1;
    void *mapa = mmap(NULL, MAP_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, MMIO_BASE);
    if (mapa == MAP_FAILED) return 1;
    mmio = (volatile uint32_t *)mapa;

    int opcao;
    do {
        rodando = 1;
        mostrar_menu();
        if (scanf("%d", &opcao) != 1) {
            int c; while ((c = getchar()) != '\n' && c != EOF) {} continue;
        }
        switch (opcao) {
            case 1: menu_load_reg(); break;
            case 2: menu_espera_vsync(); break;
            case 3: menu_pale_definir(); break;
            case 4: menu_spr_criar(); break;
            case 5: menu_spr_mover(); break;
            case 6: menu_bg_tile(); break;
            case 7: menu_bg_mover(); break;
            case 8: menu_poli_definir(); break;
            case 9: menu_poli_mover(); break;
            case 10: cena_invasao_espacial(); break;
            case 11: cena_chuva_matrix(); break;
            case 12: cena_radar(); break;
            case 0: printf("Encerrando...\n"); break;
            default: printf("Invalido.\n"); break;
        }
    } while (opcao != 0);

    munmap(mapa, MAP_SIZE);
    close(fd);
    return 0;
}