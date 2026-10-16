.syntax unified
.arch armv7-a

// Diz ao Linux que o que se segue é código EXECUTÁVEL
.text 

// MACRO para declarar funções corretamente no padrão ARM Linux (AAPCS)
.macro FUNC_DEF name
    .global \name
    .type \name, %function
.endm

// Declaração de todas as funções
FUNC_DEF asm_load_reg
FUNC_DEF asm_espera_vsync
FUNC_DEF asm_pale_definir
FUNC_DEF asm_spr_criar
FUNC_DEF asm_spr_mover
FUNC_DEF asm_bg_tile
FUNC_DEF asm_bg_mover
FUNC_DEF asm_poli_definir
FUNC_DEF asm_poli_mover

// =======================================================
// ROTINA INTERNA: ENVIA PARA A FIFO
// =======================================================
_send_fifo:
1:  
    LDR R2, [R0, #4]      // Lê MMIO_STATUS (Offset 0x04)
    AND R2, R2, #1        // Isola o bit 0
    CMP R2, #1            
    NOP                   // Desafoga o barramento
    BEQ 1b                // Se a fila estiver cheia, repete a leitura

    STR R1, [R0, #0]      // Grava a instrução no Offset 0x00
    BX LR                 // Retorna com segurança para o C

// =======================================================
// 1. LOAD_REG (Opcode: 0x00)
// =======================================================
asm_load_reg:
    AND R1, R1, #7
    LSL R1, R1, #24
    LDR R3, =0xFFFFFF
    AND R2, R2, R3
    ORR R1, R1, R2
    B _send_fifo

// =======================================================
// 2. ESPERA_VSYNC (Opcode: 0x01)
// =======================================================
asm_espera_vsync:
    LDR R1, =0x08000000   
    B _send_fifo

// =======================================================
// 3. PALE_DEFINIR (Opcode: 0x02)
// =======================================================
asm_pale_definir:
    AND R1, R1, #0xFF
    LSL R1, R1, #19
    AND R2, R2, #0xFF
    LSL R2, R2, #11
    ORR R1, R1, R2
    LDR R3, =0x10000000
    ORR R1, R1, R3
    B _send_fifo

// =======================================================
// 4. SPR_CRIAR (Opcode: 0x03)
// =======================================================
asm_spr_criar:
    PUSH {R4, R5, R6, LR} 
    LDR R12, =0x18000000  
    AND R1, R1, #7
    ORR R12, R12, R1, LSL #24
    AND R2, R2, #0x1F
    ORR R12, R12, R2, LSL #19
    AND R3, R3, #0x1F
    ORR R12, R12, R3, LSL #14

    LDR R4, [SP, #16]     
    AND R4, R4, #1
    ORR R12, R12, R4, LSL #13
    LDR R4, [SP, #20]     
    AND R4, R4, #1
    ORR R12, R12, R4, LSL #12
    LDR R4, [SP, #24]     
    AND R4, R4, #1
    ORR R12, R12, R4, LSL #11

    MOV R1, R12           
    POP {R4, R5, R6, LR}  
    B _send_fifo          

// =======================================================
// 5. SPR_MOVER (Opcode: 0x04)
// =======================================================
asm_spr_mover:
    AND R1, R1, #0x1F
    LSL R1, R1, #19
    AND R2, R2, #3
    LSL R2, R2, #12
    ORR R1, R1, R2
    LDR R3, =0x20000000
    ORR R1, R1, R3
    B _send_fifo

// =======================================================
// 6. BG_TILE (Opcode: 0x05)
// =======================================================
asm_bg_tile:
    AND R1, R1, #0x1F
    LSL R1, R1, #22
    LDR R2, =0x28000000
    ORR R1, R1, R2
    B _send_fifo

// =======================================================
// 7. BG_MOVER (Opcode: 0x06)
// =======================================================
asm_bg_mover:
    AND R1, R1, #3
    LSL R1, R1, #25
    LDR R2, =0x30000000
    ORR R1, R1, R2
    B _send_fifo

// =======================================================
// 8. POLI_DEFINIR (Opcode: 0x07)
// =======================================================
asm_poli_definir:
    PUSH {R4, R5, R6, LR}
    LDR R12, =0x38000000
    AND R1, R1, #7
    ORR R12, R12, R1, LSL #24
    AND R2, R2, #7
    ORR R12, R12, R2, LSL #21
    LDR R4, =0x1FF
    AND R3, R3, R4
    ORR R12, R12, R3, LSL #12

    LDR R4, [SP, #16]
    AND R4, R4, #0xFF
    ORR R12, R12, R4, LSL #4
    LDR R4, [SP, #20]
    AND R4, R4, #3
    ORR R12, R12, R4, LSL #2
    LDR R4, [SP, #24]
    AND R4, R4, #1
    ORR R12, R12, R4, LSL #1
    LDR R4, [SP, #28]
    AND R4, R4, #1
    ORR R12, R12, R4

    MOV R1, R12
    POP {R4, R5, R6, LR}
    B _send_fifo

// =======================================================
// 9. POLI_MOVER (Opcode: 0x08)
// =======================================================
asm_poli_mover:
    AND R1, R1, #7
    LSL R1, R1, #24
    AND R2, R2, #3
    LSL R2, R2, #22
    ORR R1, R1, R2
    LDR R3, =0x40000000
    ORR R1, R1, R3
    B _send_fifo