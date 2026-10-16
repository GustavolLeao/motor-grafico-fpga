module Unidade_Controle (
    input wire clk,
    input wire reset,

    // Unidade de busca
    output reg        valid_busca,
    input wire        instrucao_valida,

    // Registrador de instrução
    input wire [31:0] IR,

    // VSYNC
    input wire vsync,

    // Banco de registradores
    output reg        reg_write,
    output reg [2:0]  reg_endereco_escrita,
    output reg [31:0] reg_dado_escrita,
    output reg [2:0]  reg_endereco_leitura1,
    input wire [31:0] reg_dado_leitura1,
    output reg [2:0]  reg_endereco_leitura2,
    input wire [31:0] reg_dado_leitura2,

    // Rasterizador
    output reg        valid_rasterizador,
    input wire        busy_rasterizador,
    input wire        done_rasterizador,
    output reg [2:0]  raster_indice,
    output reg [8:0]  raster_x,
    output reg [7:0]  raster_y,
    output reg [8:0]  raster_largura,
    output reg [7:0]  raster_altura,
    output reg [7:0]  raster_cor,
    output reg        raster_tipo,
    output reg        raster_visibilidade,
    output reg        raster_mover,
    output reg [1:0]  raster_direcao,

    // Sprite
    output reg        valid_sprite,
    input wire        busy_sprite,
    input wire        done_sprite,
    output reg [4:0]  sprite_indice,
    output reg [8:0]  sprite_x,
    output reg [7:0]  sprite_y,
    output reg [4:0]  sprite_mif,
    output reg        sprite_visibilidade,
    output reg        sprite_espelhado_H,
    output reg        sprite_espelhado_V,
    output reg        sprite_mover,
    output reg [1:0]  sprite_direcao,

    // Background
    output reg        valid_background,
    input wire        busy_background,
    input wire        done_background,
    output reg [4:0]  background_indice,
    output reg        background_mover,
    output reg [1:0]  background_direcao,

    // Palette
    output reg        valid_palette,
    input wire        busy_palette,
    input wire        done_palette,
    output reg [7:0]  palette_endereco,
    output reg [7:0]  palette_cor
);

    // Opcodes Atualizados (Formato RISC)
    localparam OP_LOAD_REG      = 5'b00000;
    localparam OP_ESPERA_VSYNC  = 5'b00001;
    localparam OP_PALE_DEFINIR  = 5'b00010;
    localparam OP_SPR_CRIAR     = 5'b00011;
    localparam OP_SPR_MOVER     = 5'b00100;
    localparam OP_BG_TILE       = 5'b00101;
    localparam OP_BG_MOVER      = 5'b00110;
    localparam OP_POLI_DEFINIR  = 5'b00111;
    localparam OP_POLI_MOVER    = 5'b01000;

    // Estados
    localparam MONITORAR = 1'b0;
    localparam EXECUTAR  = 1'b1;
    reg estado;

    // Identificação dos motores
    localparam MOTOR_NENHUM     = 3'd0;
    localparam MOTOR_RASTER     = 3'd1;
    localparam MOTOR_SPRITE     = 3'd2;
    localparam MOTOR_BACKGROUND = 3'd3;
    localparam MOTOR_PALETTE    = 3'd4;

    reg [2:0] motor_esperado;
    reg esperando_motor;

    wire [4:0] opcode;
    assign opcode = IR[31:27];

    // Decodificação para Leitura no Banco de Registradores
    always @(*) begin
        reg_endereco_leitura1 = 3'b000;
        reg_endereco_leitura2 = 3'b000;
        case (opcode)
            OP_POLI_DEFINIR: begin
                reg_endereco_leitura1 = IR[26:24];       // Registrador Fonte (X e Y)
                reg_endereco_leitura2 = {1'b0, IR[3:2]}; // Registrador de Cor
            end
            OP_SPR_CRIAR: begin
                reg_endereco_leitura1 = IR[26:24];       // Registrador Fonte (X e Y)
            end
            default: begin
                reg_endereco_leitura1 = 3'b000;
                reg_endereco_leitura2 = 3'b000;
            end
        endcase
    end

    // Mapeamento Combinacional de Sinais para os Motores
    always @(*) begin
        // Valores Padrão (Evita corrupção de latches indesejados)
        raster_indice = 3'b0; raster_x = 9'b0; raster_y = 8'b0;
        raster_largura = 9'b0; raster_altura = 8'b0; raster_cor = 8'b0;
        raster_tipo = 1'b0; raster_visibilidade = 1'b0; raster_mover = 1'b0; raster_direcao = 2'b0;

        sprite_indice = 5'b0; sprite_x = 9'b0; sprite_y = 8'b0; sprite_mif = 5'b0;
        sprite_visibilidade = 1'b0; sprite_espelhado_H = 1'b0; sprite_espelhado_V = 1'b0;
        sprite_mover = 1'b0; sprite_direcao = 2'b0;

        background_indice = 5'b0; background_mover = 1'b0; background_direcao = 2'b0;
        palette_endereco = 8'b0; palette_cor = 8'b0;

        // Distribuição conforme os layouts RISC
        if (opcode == OP_POLI_DEFINIR) begin
            raster_x = reg_dado_leitura1[16:8];
            raster_y = reg_dado_leitura1[7:0];
            raster_cor = reg_dado_leitura2[7:0];
            raster_indice = IR[23:21];
            raster_largura = IR[20:12];
            raster_altura = IR[11:4];
            raster_tipo = IR[1];
            raster_visibilidade = IR[0];
        end
        else if (opcode == OP_POLI_MOVER) begin
            raster_indice = IR[26:24];
            raster_direcao = IR[23:22];
            raster_mover = 1'b1;
        end
        else if (opcode == OP_SPR_CRIAR) begin
            sprite_x = reg_dado_leitura1[16:8];
            sprite_y = reg_dado_leitura1[7:0];
            sprite_indice = IR[23:19];
            sprite_mif = IR[18:14];
            sprite_visibilidade = IR[13];
            sprite_espelhado_H = IR[12];
            sprite_espelhado_V = IR[11];
        end
        else if (opcode == OP_SPR_MOVER) begin
            sprite_indice = IR[23:19];
            sprite_direcao = IR[13:12];
            sprite_mover = 1'b1;
        end
        else if (opcode == OP_BG_TILE) begin
            background_indice = IR[26:22];
        end
        else if (opcode == OP_BG_MOVER) begin
            background_direcao = IR[26:25];
            background_mover = 1'b1;
        end
        else if (opcode == OP_PALE_DEFINIR) begin
            palette_cor = IR[26:19];
            palette_endereco = IR[18:11];
        end
    end

    // Máquina de Estados (Controle e Sincronismo)
    always @(posedge clk) begin
        if (reset) begin
            estado <= MONITORAR;
            motor_esperado <= MOTOR_NENHUM;
            esperando_motor <= 1'b0;
            valid_busca <= 1'b0;
            reg_write <= 1'b0;
            reg_endereco_escrita <= 3'b000;
            reg_dado_escrita <= 32'b0;
            valid_rasterizador <= 1'b0;
            valid_sprite <= 1'b0;
            valid_background <= 1'b0;
            valid_palette <= 1'b0;
        end
        else begin
            // Transição padrão dos pulsos (Ficam em nível ALTO por apenas 1 CLK)
            valid_busca <= 1'b0;
            reg_write <= 1'b0;
            valid_rasterizador <= 1'b0;
            valid_sprite <= 1'b0;
            valid_background <= 1'b0;
            valid_palette <= 1'b0;

            case (estado)
                MONITORAR: begin
                    esperando_motor <= 1'b0;
                    motor_esperado <= MOTOR_NENHUM;
                    valid_busca <= 1'b1; // Libera busca de instrução

                    if (instrucao_valida) begin
                        estado <= EXECUTAR;
                    end
                end

                EXECUTAR: begin
                    // 2. SE ESTÁ AGUARDANDO UM MOTOR, FICA PRESO AQUI ATÉ O DONE SUBIR
                    if (esperando_motor) begin
                        case (motor_esperado)
                            MOTOR_RASTER:     if (done_rasterizador) begin esperando_motor <= 1'b0; estado <= MONITORAR; end
                            MOTOR_SPRITE:     if (done_sprite)       begin esperando_motor <= 1'b0; estado <= MONITORAR; end
                            MOTOR_BACKGROUND: if (done_background)   begin esperando_motor <= 1'b0; estado <= MONITORAR; end
                            MOTOR_PALETTE:    if (done_palette)      begin esperando_motor <= 1'b0; estado <= MONITORAR; end
                            default:                                 begin esperando_motor <= 1'b0; estado <= MONITORAR; end
                        endcase
                    end
                    else begin
                        // 1. GATILHO: IDENTIFICA A INSTRUÇÃO E EMITE O VALID DE 1 CLOCK PARA O MOTOR
                        case (opcode)
                            OP_LOAD_REG: begin
                                reg_write <= 1'b1;
                                reg_endereco_escrita <= IR[26:24];
                                reg_dado_escrita <= IR[23:0];
                                estado <= MONITORAR; // É imediato, não precisa esperar motor
                            end
                            OP_ESPERA_VSYNC: begin
                                if (vsync) estado <= MONITORAR; // Trava até o VSYNC acontecer
                            end
                            OP_PALE_DEFINIR: begin
                                valid_palette <= 1'b1;
                                esperando_motor <= 1'b1;
                                motor_esperado <= MOTOR_PALETTE;
                            end
                            OP_SPR_CRIAR, OP_SPR_MOVER: begin
                                valid_sprite <= 1'b1;
                                esperando_motor <= 1'b1;
                                motor_esperado <= MOTOR_SPRITE;
                            end
                            OP_BG_TILE, OP_BG_MOVER: begin
                                valid_background <= 1'b1;
                                esperando_motor <= 1'b1;
                                motor_esperado <= MOTOR_BACKGROUND;
                            end
                            OP_POLI_DEFINIR, OP_POLI_MOVER: begin
                                valid_rasterizador <= 1'b1;
                                esperando_motor <= 1'b1;
                                motor_esperado <= MOTOR_RASTER;
                            end
                            default: begin
                                estado <= MONITORAR;
                            end
                        endcase
                    end
                end
            endcase
        end
    end
endmodule