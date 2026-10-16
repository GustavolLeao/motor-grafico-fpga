#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>

/* ============================================================
- ENDEREÇOS E OPCODES
- ============================================================ */

#define MMIO_BASE       0xFF200000UL
#define OFFSET_INSTRUCAO 0x00
#define OFFSET_STATUS    0x04
#define MAP_SIZE        0x1000

#define OP_LOAD_REG      0x00
#define OP_ESPERA_VSYNC  0x01
#define OP_PALE_DEFINIR  0x02
#define OP_SPR_CRIAR     0x03
#define OP_SPR_MOVER     0x04
#define OP_BG_TILE       0x05
#define OP_BG_MOVER      0x06
#define OP_POLI_DEFINIR  0x07
#define OP_POLI_MOVER    0x08

static volatile uint32_t *mmio;

/* ============================================================
- ENVIO DE INSTRUÇÕES
- ============================================================ */

static void enviar_instrucao(uint32_t instrucao) {
    while ((mmio[OFFSET_STATUS / 4] & 0x1) == 1) { usleep(10); }
    mmio[OFFSET_INSTRUCAO / 4] = instrucao;
}

static void enviar_inst_silencioso(uint32_t instrucao) {
    while ((mmio[OFFSET_STATUS / 4] & 0x1) == 1) { usleep(10); }
    mmio[OFFSET_INSTRUCAO / 4] = instrucao;
}

/* ============================================================
- 12 - ANIMAÇÃO COM AS 32 IMAGENS DIFERENTES (MIF)
- ============================================================ */

static void executar_cena_criativa(void)
{
    printf("\n==================================================\n");
    printf(" MODO HARDWARE: 32 Texturas (MIF) e 4 Poligonos\n");
    printf(" Pressione Ctrl+C no terminal para encerrar.\n");
    printf("==================================================\n");

    typedef struct { int x, y, vx, vy, mif; } SpriteFisica;
    typedef struct { int x, y, vx, vy, largura, altura, tipo, cor_reg; } PoligonoFisica;

    SpriteFisica sprites[32];
    
    // INICIALIZAÇÃO DOS 32 SPRITES
    for (int i = 0; i < 32; i++) {
        // Coloca 8 sprites por linha
        sprites[i].x = (i % 8) * 17;        
        sprites[i].y = (i / 8) * 22;        
        
        sprites[i].vx = (i % 2 == 0) ? 1 : -1;
        sprites[i].vy = ((i / 4) % 2 == 0) ? 1 : -1;
        
        // AQUI ESTÁ A MÁGICA: O sprite 'i' vai usar exatamente a imagem/MIF 'i' (0 a 31)
        sprites[i].mif = i; 
    }

    PoligonoFisica poligonos[4] = {
        { 10,  10,   1,  1, 60, 40, 0, 0 }, 
        { 200, 20,  -1,  1, 40, 40, 1, 1 }, 
        { 50,  150,  1, -1, 30, 50, 0, 2 }, 
        { 250, 160, -1, -1, 50, 50, 1, 3 }  
    };

    // 1. INICIALIZA O FUNDO
    enviar_inst_silencioso((OP_BG_TILE << 27) | (1 << 22));

    // 2. CONFIGURA AS CORES PARA OS POLÍGONOS
    enviar_inst_silencioso((OP_LOAD_REG << 27) | (0 << 24) | 1); // Branco
    enviar_inst_silencioso((OP_LOAD_REG << 27) | (1 << 24) | 2); // Vermelho
    enviar_inst_silencioso((OP_LOAD_REG << 27) | (2 << 24) | 3); // Verde
    enviar_inst_silencioso((OP_LOAD_REG << 27) | (3 << 24) | 5); // Amarelo

    // 3. ENVIA OS 32 SPRITES INICIAIS PARA O HARDWARE
    for (int i = 0; i < 32; i++) {
        uint32_t val_xy = ((sprites[i].x & 0x1FF) << 8) | (sprites[i].y & 0xFF);
        enviar_inst_silencioso((OP_LOAD_REG << 27) | (7 << 24) | val_xy);
        
        // A instrução agora passa o valor único do MIF para cada sprite (bits 18:14)
        enviar_inst_silencioso((OP_SPR_CRIAR << 27) | (7 << 24) | ((i & 0x1F) << 19) | ((sprites[i].mif & 0x1F) << 14) | (1 << 13));
    }

    // 4. ENVIA OS 4 POLÍGONOS INICIAIS PARA O HARDWARE
    for (int i = 0; i < 4; i++) {
        uint32_t val_xy = ((poligonos[i].x & 0x1FF) << 8) | (poligonos[i].y & 0xFF);
        enviar_inst_silencioso((OP_LOAD_REG << 27) | (7 << 24) | val_xy);
        enviar_inst_silencioso(
            (OP_POLI_DEFINIR << 27) | (7 << 24) | ((i & 0x7) << 21) | 
            ((poligonos[i].largura & 0x1FF) << 12) | ((poligonos[i].altura & 0xFF) << 4) | 
            ((poligonos[i].cor_reg & 0x3) << 2) | ((poligonos[i].tipo & 0x1) << 1) | 1
        );
    }

    int frame_contador = 0;

    // 5. LOOP PRINCIPAL DE ANIMAÇÃO
    while (1) {
        enviar_inst_silencioso(OP_ESPERA_VSYNC << 27);

        frame_contador++;
        if (frame_contador < 40005) continue; 
        frame_contador = 0;

        // --- MOVIMENTO DOS 32 SPRITES ---
        for (int i = 0; i < 32; i++) {
            sprites[i].x += sprites[i].vx;
            sprites[i].y += sprites[i].vy;

            if (sprites[i].x <= 0) { sprites[i].x = 0; sprites[i].vx = -sprites[i].vx; }
            else if (sprites[i].x >= 144) { sprites[i].x = 144; sprites[i].vx = -sprites[i].vx; }

            if (sprites[i].y <= 0) { sprites[i].y = 0; sprites[i].vy = -sprites[i].vy; }
            else if (sprites[i].y >= 104) { sprites[i].y = 104; sprites[i].vy = -sprites[i].vy; }

            int dir_x = (sprites[i].vx > 0) ? 3 : 2;
            enviar_inst_silencioso((OP_SPR_MOVER << 27) | ((i & 0x1F) << 19) | (dir_x << 12));

            int dir_y = (sprites[i].vy > 0) ? 1 : 0;
            enviar_inst_silencioso((OP_SPR_MOVER << 27) | ((i & 0x1F) << 19) | (dir_y << 12));
        }

        // --- MOVIMENTO DOS 4 POLÍGONOS ---
        for (int i = 0; i < 4; i++) {
            poligonos[i].x += poligonos[i].vx;
            poligonos[i].y += poligonos[i].vy;

            if (poligonos[i].x <= 0) { poligonos[i].x = 0; poligonos[i].vx = -poligonos[i].vx; }
            else if (poligonos[i].x + poligonos[i].largura >= 320) { 
                poligonos[i].x = 320 - poligonos[i].largura; poligonos[i].vx = -poligonos[i].vx; 
            }

            if (poligonos[i].y <= 0) { poligonos[i].y = 0; poligonos[i].vy = -poligonos[i].vy; }
            else if (poligonos[i].y + poligonos[i].altura >= 240) { 
                poligonos[i].y = 240 - poligonos[i].altura; poligonos[i].vy = -poligonos[i].vy; 
            }

            int dir_x = (poligonos[i].vx > 0) ? 3 : 2;
            enviar_inst_silencioso((OP_POLI_MOVER << 27) | ((i & 0x7) << 24) | (dir_x << 22));

            int dir_y = (poligonos[i].vy > 0) ? 1 : 0;
            enviar_inst_silencioso((OP_POLI_MOVER << 27) | ((i & 0x7) << 24) | (dir_y << 22));
        }
    }
}

/* ============================================================
- FUNÇÕES DE MENU E MAIN
- ============================================================ */

static void menu_load_reg(void) {
    unsigned int reg, valor;
    printf("\nRegistrador (0-7): "); scanf("%u", &reg);
    printf("Valor HEX: "); scanf("%x", &valor);
    enviar_instrucao((OP_LOAD_REG << 27) | ((reg & 0x7) << 24) | (valor & 0xFFFFFF));
}
static void menu_espera_vsync(void) { enviar_instrucao(OP_ESPERA_VSYNC << 27); }
static void menu_bg_tile(void) {
    unsigned int indice;
    printf("\nIndice cenario: "); scanf("%u", &indice);
    enviar_instrucao((OP_BG_TILE << 27) | ((indice & 0x1F) << 22));
}
static void menu_sprite_criar(void) {
    unsigned int reg, indice, mif, vis, esp_h, esp_v;
    printf("\nReg X/Y: "); scanf("%u", &reg);
    printf("Indice: "); scanf("%u", &indice);
    printf("MIF: "); scanf("%u", &mif);
    printf("Visivel: "); scanf("%u", &vis);
    printf("Esp H: "); scanf("%u", &esp_h);
    printf("Esp V: "); scanf("%u", &esp_v);
    uint32_t inst = (OP_SPR_CRIAR << 27) | ((reg & 0x7) << 24) | ((indice & 0x1F) << 19) |
                    ((mif & 0x1F) << 14) | ((vis & 0x1) << 13) | ((esp_h & 0x1) << 12) | ((esp_v & 0x1) << 11);
    enviar_instrucao(inst);
}

static void menu(void) {
    int opcao;
    do {
        printf("\n\n========================================\n");
        printf("       TESTE MMIO - FPGA / ARM\n");
        printf("========================================\n");
        printf("1  - LOAD_REG\n2  - ESPERA_VSYNC\n3  - SPR_CRIAR\n5  - BG_TILE\n");
        printf("12 - Cena Lenta (32 Sprites e 4 Poligonos)\n");
        printf("0  - Sair\n----------------------------------------\nEscolha: ");
        scanf("%d", &opcao);

        switch (opcao) {
            case 1:  menu_load_reg(); break;
            case 2:  menu_espera_vsync(); break;
            case 3:  menu_sprite_criar(); break;
            case 5:  menu_bg_tile(); break;
            case 12: executar_cena_criativa(); break;
            case 0:  printf("Encerrando...\n"); break;
            default: printf("Opcao invalida!\n"); break;
        }
    } while (opcao != 0);
}

int main(void) {
    int fd = open("/dev/mem", O_RDWR | O_SYNC);
    if (fd < 0) { perror("Erro"); return 1; }
    void *map = mmap(NULL, MAP_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, MMIO_BASE);
    if (map == MAP_FAILED) { close(fd); return 1; }

    mmio = (volatile uint32_t *)map;
    menu();
    munmap((void *)map, MAP_SIZE);
    close(fd);
    return 0;
}