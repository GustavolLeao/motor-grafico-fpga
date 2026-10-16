/* ============================================================================
 * teste_instrucoes.c
 * Bateria de Testes Passo a Passo para a GPU Customizada (FPGA)
 * Compilar: gcc -std=gnu99 teste3.c gpu_driver.S -o teste3 -lm
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

#define DIR_CIMA        0
#define DIR_BAIXO       1
#define DIR_ESQUERDA    2
#define DIR_DIREITA     3

#define POLI_RETANGULO  0
#define POLI_TRIANGULO  1
#define REG_XY          7

static inline uint32_t empacotar_xy(int x, int y) {
    return ((uint32_t)(x & 0x1FF) << 8) | (uint32_t)(y & 0xFF);
}

static inline uint8_t cor_rgb(int r, int g, int b) {
    return (uint8_t)(((r & 0x7) << 5) | ((g & 0x7) << 2) | (b & 0x3));
}

static void configurar_paleta_extra(void) {
    asm_pale_definir(mmio, cor_rgb(7, 3, 0), 6);   /* laranja */
    asm_pale_definir(mmio, cor_rgb(7, 0, 3), 7);   /* rosa    */
    asm_pale_definir(mmio, cor_rgb(5, 0, 3), 8);   /* magenta */
    asm_pale_definir(mmio, cor_rgb(0, 7, 3), 9);   /* ciano   */
}


/* ============================================================================
 * TESTE 1: BG_TILE e BG_MOVER (Rolagem Constante)
 * ============================================================================ */
static void teste_bg_mover_e_tile(void) {
    printf("\n=== TESTE: BG_TILE e BG_MOVER ===\n");
    printf("1. Definindo o background para o indice 1...\n");
    asm_bg_tile(mmio, 1);
    usleep(1000000);

    printf("2. Movendo o background continuamente para a DIREITA...\n");
    for (int i = 0; i < 40; i++) {
        asm_espera_vsync(mmio);
        asm_bg_mover(mmio, DIR_DIREITA);
        usleep(50000);
    }

    printf("3. Movendo para BAIXO...\n");
    for (int i = 0; i < 40; i++) {
        asm_espera_vsync(mmio);
        asm_bg_mover(mmio, DIR_BAIXO);
        usleep(50000);
    }

    printf("4. Movendo para ESQUERDA...\n");
    for (int i = 0; i < 40; i++) {
        asm_espera_vsync(mmio);
        asm_bg_mover(mmio, DIR_ESQUERDA);
        usleep(50000);
    }

    printf("5. Movendo para CIMA...\n");
    for (int i = 0; i < 40; i++) {
        asm_espera_vsync(mmio);
        asm_bg_mover(mmio, DIR_CIMA);
        usleep(50000);
    }

    printf("Teste de Background finalizado. Pressione Enter para voltar ao menu.");
    getchar(); getchar();
}


/* ============================================================================
 * TESTE 2: SPR_CRIAR (32 Sprites em grade 160x120 + Espelhamentos + Visibilidade)
 * ============================================================================ */
static void teste_sprite_criar(void) {
    printf("\n=== TESTE: SPR_CRIAR (32 Sprites - Limite 160x120) ===\n");
    asm_bg_tile(mmio, 1);
    configurar_paleta_extra();

    printf("-> Passo A: Criando os 32 sprites um a um dentro da area 160x120 (com quebra de linha)\n");
    
    int largura_passo = 18; 
    int altura_passo  = 18; 
    int x_inicial     = 10;
    int y_inicial     = 10;
    int limite_largura= 160;

    for (int i = 0; i < 32; i++) {
        int coluna = i % 8; 
        int linha  = i / 8;

        int x = x_inicial + (coluna * largura_passo);
        int y = y_inicial + (linha  * altura_passo);

        if (x > limite_largura) x = limite_largura;

        printf("   Criando Sprite %2d -> X=%3d, Y=%3d (MIF=%d)\n", i, x, y, i % 32);
        
        asm_load_reg(mmio, REG_XY, empacotar_xy(x, y));
        asm_spr_criar(mmio, REG_XY, i, i % 32, 1, 0, 0);
        
        usleep(150000); 
    }

    printf("\n-> Passo B: Testando Espelhamento Horizontal (EspH = 1) nos sprites pares da primeira linha...\n");
    for(int i = 0; i < 8; i += 2) {
        int x = x_inicial + (i * largura_passo);
        int y = y_inicial;
        asm_load_reg(mmio, REG_XY, empacotar_xy(x, y));
        asm_spr_criar(mmio, REG_XY, i, i, 1, 1, 0); 
    }
    usleep(1000000);

    printf("\n-> Passo C: Testando Espelhamento Vertical (EspV = 1) nos sprites ímpares da primeira linha...\n");
    for(int i = 1; i < 8; i += 2) {
        int x = x_inicial + (i * largura_passo);
        int y = y_inicial;
        asm_load_reg(mmio, REG_XY, empacotar_xy(x, y));
        asm_spr_criar(mmio, REG_XY, i, i, 1, 0, 1); 
    }
    usleep(1000000);

    printf("\n-> Passo D: Ocultando temporariamente a última linha de sprites (Visivel = 0)...\n");
    for(int i = 24; i < 32; i++) {
        int coluna = i % 8;
        int linha  = i / 8;
        int x = x_inicial + (coluna * largura_passo);
        int y = y_inicial + (linha  * altura_passo);
        asm_load_reg(mmio, REG_XY, empacotar_xy(x, y));
        asm_spr_criar(mmio, REG_XY, i, i, 0, 0, 0); 
    }
    usleep(1200000);

    printf("\n-> Passo E: Mostrando-os novamente (Visivel = 1)...\n");
    for(int i = 24; i < 32; i++) {
        int coluna = i % 8;
        int linha  = i / 8;
        int x = x_inicial + (coluna * largura_passo);
        int y = y_inicial + (linha  * altura_passo);
        asm_load_reg(mmio, REG_XY, empacotar_xy(x, y));
        asm_spr_criar(mmio, REG_XY, i, i, 1, 0, 0); 
    }
    usleep(1200000);

    printf("Teste de SPR_CRIAR finalizado. Pressione Enter para voltar ao menu.");
    getchar(); getchar();
}


/* ============================================================================
 * TESTE 3: SPR_MOVER (Movimentação passo a passo)
 * ============================================================================ */
static void teste_sprite_mover(void) {
    printf("\n=== TESTE: SPR_MOVER (Movimentação controlada) ===\n");
    asm_bg_tile(mmio, 1);

    int x = 80, y = 60;
    asm_load_reg(mmio, REG_XY, empacotar_xy(x, y));
    asm_spr_criar(mmio, REG_XY, 0, 5, 1, 0, 0);
    printf("Sprite 0 criado no centro da área 160x120 (80, 60).\n");
    usleep(1000000);

    printf("Movendo Sprite 0 para a DIREITA passo a passo...\n");
    for (int i = 0; i < 40; i++) {
        asm_espera_vsync(mmio);
        asm_spr_mover(mmio, 0, DIR_DIREITA);
        usleep(40000);
    }

    printf("Movendo para BAIXO...\n");
    for (int i = 0; i < 30; i++) {
        asm_espera_vsync(mmio);
        asm_spr_mover(mmio, 0, DIR_BAIXO);
        usleep(40000);
    }

    printf("Movendo para ESQUERDA...\n");
    for (int i = 0; i < 40; i++) {
        asm_espera_vsync(mmio);
        asm_spr_mover(mmio, 0, DIR_ESQUERDA);
        usleep(40000);
    }

    printf("Movendo para CIMA...\n");
    for (int i = 0; i < 30; i++) {
        asm_espera_vsync(mmio);
        asm_spr_mover(mmio, 0, DIR_CIMA);
        usleep(40000);
    }

    printf("Teste de SPR_MOVER finalizado. Pressione Enter para voltar ao menu.");
    getchar(); getchar();
}


/* ============================================================================
 * TESTE 4: POLI_DEFINIR (Criação gradual e mudança de cores - Área 320x240)
 * ============================================================================ */
static void teste_poligono_definir(void) {
    printf("\n=== TESTE: POLI_DEFINIR (Criacao e Cores - Limite 320x240) ===\n");
    asm_bg_tile(mmio, 1);
    configurar_paleta_extra();

    asm_load_reg(mmio, 0, 2); // Vermelho
    asm_load_reg(mmio, 1, 3); // Verde
    asm_load_reg(mmio, 2, 4); // Azul
    asm_load_reg(mmio, 3, 5); // Amarelo

    printf("-> Criando poligonos espalhados pela tela 320x240 com pausa:\n");

    printf("   1. Poligono 0 (Retangulo Vermelho - Superior Esquerdo)\n");
    asm_load_reg(mmio, REG_XY, empacotar_xy(30, 40));
    asm_poli_definir(mmio, REG_XY, 0, 70, 50, 0, POLI_RETANGULO, 1);
    usleep(800000);

    printf("   2. Poligono 1 (Triangulo Verde - Superior Direito)\n");
    asm_load_reg(mmio, REG_XY, empacotar_xy(200, 40));
    asm_poli_definir(mmio, REG_XY, 1, 60, 60, 1, POLI_TRIANGULO, 1);
    usleep(800000);

    printf("   3. Poligono 2 (Retangulo Azul - Inferior Central)\n");
    asm_load_reg(mmio, REG_XY, empacotar_xy(110, 150));
    asm_poli_definir(mmio, REG_XY, 2, 90, 50, 2, POLI_RETANGULO, 1);
    usleep(800000);

    printf("\n-> Mudando a cor do Poligono 0 para Laranja (redefinindo via registrador 0)...\n");
    asm_load_reg(mmio, 0, 6); // Laranja
    asm_load_reg(mmio, REG_XY, empacotar_xy(30, 40));
    asm_poli_definir(mmio, REG_XY, 0, 70, 50, 0, POLI_RETANGULO, 1);
    usleep(1200000);

    printf("Teste de POLI_DEFINIR finalizado. Pressione Enter para voltar ao menu.");
    getchar(); getchar();
}


/* ============================================================================
 * TESTE 5: POLI_MOVER (Movimentação passo a passo em 320x240)
 * ============================================================================ */
static void teste_poligono_mover(void) {
    printf("\n=== TESTE: POLI_MOVER (Movimentação controlada em 320x240) ===\n");
    asm_bg_tile(mmio, 1);
    configurar_paleta_extra();

    asm_load_reg(mmio, 0, 7); // Rosa
    asm_load_reg(mmio, REG_XY, empacotar_xy(120, 90));
    asm_poli_definir(mmio, REG_XY, 0, 70, 50, 0, POLI_RETANGULO, 1);
    printf("Polígono 0 criado na resolução 320x240.\n");
    usleep(1000000);

    printf("Movendo Polígono 0 para a DIREITA passo a passo...\n");
    for (int i = 0; i < 70; i++) {
        asm_espera_vsync(mmio);
        asm_poli_mover(mmio, 0, DIR_DIREITA);
        usleep(30000);
    }

    printf("Movendo para BAIXO...\n");
    for (int i = 0; i < 50; i++) {
        asm_espera_vsync(mmio);
        asm_poli_mover(mmio, 0, DIR_BAIXO);
        usleep(30000);
    }

    printf("Movendo para ESQUERDA...\n");
    for (int i = 0; i < 70; i++) {
        asm_espera_vsync(mmio);
        asm_poli_mover(mmio, 0, DIR_ESQUERDA);
        usleep(30000);
    }

    printf("Movendo para CIMA...\n");
    for (int i = 0; i < 50; i++) {
        asm_espera_vsync(mmio);
        asm_poli_mover(mmio, 0, DIR_CIMA);
        usleep(30000);
    }

    printf("Teste de POLI_MOVER finalizado. Pressione Enter para voltar ao menu.");
    getchar(); getchar();
}


/* ============================================================================
 * MENU PRINCIPAL
 * ============================================================================ */
static void mostrar_menu(void) {
    printf("\n==================================================\n");
    printf("   TESTES PASSO A PASSO DA ISA - GPU FPGA\n");
    printf("==================================================\n");
    printf(" 1 - Testar: BG_TILE e BG_MOVER (Rolagem de Fundo)\n");
    printf(" 2 - Testar: SPR_CRIAR (32 Sprites em 160x120 + Espelho/Vis)\n");
    printf(" 3 - Testar: SPR_MOVER (Movimentacao Passo a Passo)\n");
    printf(" 4 - Testar: POLI_DEFINIR (Criacao de Poligonos em 320x240)\n");
    printf(" 5 - Testar: POLI_MOVER (Movimentacao de Poligonos em 320x240)\n");
    printf("--------------------------------------------------\n");
    printf(" 0 - Sair\n");
    printf("Escolha a instrucao para testar: ");
}

int main(void) {
    signal(SIGINT, trata_sigint);

    int fd = open("/dev/mem", O_RDWR | O_SYNC);
    if (fd < 0) {
        perror("Erro ao abrir /dev/mem");
        return 1;
    }

    void *mapa = mmap(NULL, MAP_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, MMIO_BASE);
    if (mapa == MAP_FAILED) {
        perror("Erro no mmap");
        close(fd);
        return 1;
    }

    mmio = (volatile uint32_t *)mapa;
    int opcao;

    do {
        mostrar_menu();
        if (scanf("%d", &opcao) != 1) {
            int c; while ((c = getchar()) != '\n' && c != EOF) {}
            continue;
        }

        switch (opcao) {
            case 1: teste_bg_mover_e_tile(); break;
            case 2: teste_sprite_criar();    break;
            case 3: teste_sprite_mover();    break;
            case 4: teste_poligono_definir();break;
            case 5: teste_poligono_mover();  break;
            case 0: printf("Encerrando...\n"); break;
            default: printf("Opcao invalida.\n"); break;
        }

    } while (opcao != 0 && rodando);

    munmap(mapa, MAP_SIZE);
    close(fd);
    return 0;
}