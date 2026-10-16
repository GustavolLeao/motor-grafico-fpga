#ifndef GPU_ISA_H
#define GPU_ISA_H

#include <stdint.h>

void asm_load_reg(volatile uint32_t *mmio, uint32_t reg_dest, uint32_t valor);
void asm_espera_vsync(volatile uint32_t *mmio);
void asm_pale_definir(volatile uint32_t *mmio, uint32_t cor, uint32_t endereco);
void asm_spr_criar(volatile uint32_t *mmio, uint32_t reg_fonte, uint32_t indice, uint32_t mif, uint32_t vis, uint32_t esp_h, uint32_t esp_v);
void asm_spr_mover(volatile uint32_t *mmio, uint32_t indice, uint32_t direcao);
void asm_bg_tile(volatile uint32_t *mmio, uint32_t indice);
void asm_bg_mover(volatile uint32_t *mmio, uint32_t direcao);
void asm_poli_definir(volatile uint32_t *mmio, uint32_t reg_fonte, uint32_t indice, uint32_t largura, uint32_t altura, uint32_t reg_cor, uint32_t tipo, uint32_t vis);
void asm_poli_mover(volatile uint32_t *mmio, uint32_t indice, uint32_t direcao);

#endif