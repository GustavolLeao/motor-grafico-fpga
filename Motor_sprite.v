module Motor_sprite (

    input wire clk,
    input wire reset,

    // VGA

    input wire [9:0] next_x,
    input wire [9:0] next_y,

    // Comunicação com a Unidade de Controle

    input wire       valid,
    input wire [4:0] indice,

    output reg       busy,
    output reg       done,

    // Dados da configuração do sprite

    input wire [8:0] config_x,
    input wire [7:0] config_y,

    input wire [4:0] config_img,

    input wire       config_ativo,

    input wire       config_espelhado_H,
    input wire       config_espelhado_V,

    // Movimento

    input wire       mover,
    input wire [1:0] direcao,

    // Cor

    output reg [7:0] cor

);


    // Resolução lógica

    wire [8:0] logico_x;
    wire [7:0] logico_y;

    assign logico_x = next_x / 2;
    assign logico_y = next_y / 2;


    // Direções

    localparam CIMA     = 2'b00;
    localparam BAIXO    = 2'b01;
    localparam ESQUERDA = 2'b10;
    localparam DIREITA  = 2'b11;


    // Registradores dos 32 sprites

    reg [8:0] sprite_x [0:31];
    reg [7:0] sprite_y [0:31];

    reg sprite_ativo [0:31];

    reg [4:0] sprite_img [0:31];

    reg sprite_espelhado_H [0:31];
    reg sprite_espelhado_V [0:31];


    // FSM

    localparam ESTADO_IDLE      = 2'b00;
    localparam ESTADO_ATUALIZAR = 2'b01;
    localparam ESTADO_DONE      = 2'b10;

    reg [1:0] estado;


    // Índice do sprite

    reg [4:0] indice_reg;


    // Controle
// Controle
    integer i;
    always @(posedge clk or posedge reset) begin
        if (reset) begin
            estado <= ESTADO_IDLE;
            indice_reg <= 5'd0;
            busy <= 1'b0;
            done <= 1'b0;
            for (i = 0; i < 32; i = i + 1) begin
                sprite_x[i] <= 9'd0;
                sprite_y[i] <= 8'd0;
                sprite_ativo[i] <= 1'b0;
                sprite_img[i] <= 5'd0;
                sprite_espelhado_H[i] <= 1'b0;
                sprite_espelhado_V[i] <= 1'b0;
            end
        end
        else begin
            case (estado)
                // IDLE
                ESTADO_IDLE: begin
                    busy <= 1'b0;
                    done <= 1'b0;

                    if (valid) begin
                        busy <= 1'b1;

                        // LÊ OS DADOS IMEDIATAMENTE NO CICLO DO VALID
                        if (mover) begin
                            case (direcao)
                                CIMA:     if (sprite_y[indice] > 0)   sprite_y[indice] <= sprite_y[indice] - 1'b1;
                                BAIXO:    if (sprite_y[indice] < 104) sprite_y[indice] <= sprite_y[indice] + 1'b1;
                                ESQUERDA: if (sprite_x[indice] > 0)   sprite_x[indice] <= sprite_x[indice] - 1'b1;
                                DIREITA:  if (sprite_x[indice] < 144) sprite_x[indice] <= sprite_x[indice] + 1'b1;
                            endcase
                        end
                        else begin
                            sprite_x[indice]           <= config_x;
                            sprite_y[indice]           <= config_y;
                            sprite_ativo[indice]       <= config_ativo;
                            sprite_img[indice]         <= config_img;
                            sprite_espelhado_H[indice] <= config_espelhado_H;
                            sprite_espelhado_V[indice] <= config_espelhado_V;
                        end

                        // Vai direto para DONE
                        estado <= ESTADO_DONE;
                    end
                end

                // ATUALIZAR (Mantido apenas por segurança, mas não é mais usado)
                ESTADO_ATUALIZAR: begin
                    estado <= ESTADO_DONE;
                end

                // DONE
                ESTADO_DONE: begin
                    busy <= 1'b0;
                    done <= 1'b1; // Emite o pulso de confirmação para a Unidade de Controle
                    estado <= ESTADO_IDLE;
                end

                // Segurança
                default: begin
                    estado <= ESTADO_IDLE;
                    busy <= 1'b0;
                    done <= 1'b0;
                end
            endcase
        end
    end


    // ROM dos sprites

    reg [14:0] endereco;

    wire [7:0] cor_rom;


    sprites32 rom (
        .address(endereco),
        .clock(clk),
        .q(cor_rom)
    );


    // Coordenadas dentro do sprite

    reg [8:0] pixel_x;
    reg [8:0] pixel_y;

    integer j;


    // Renderização

    always @(*) begin
        endereco = 15'd0;
        cor = 8'd0;
        pixel_x = 9'd0;
        pixel_y = 9'd0;

        // Testa os 32 sprites

        for (j = 0; j < 32; j = j + 1) begin
            if (
                sprite_ativo[j] &&
                (logico_x >= sprite_x[j]) &&
                (logico_x < sprite_x[j] + 16) &&
                (logico_y >= sprite_y[j]) &&
                (logico_y < sprite_y[j] + 16)
            ) begin

                // Coordenada dentro do sprite

                pixel_x = logico_x - sprite_x[j];
                pixel_y = logico_y - sprite_y[j];

                // Espelhamento horizontal

                if (sprite_espelhado_H[j])
                    pixel_x = 15 - pixel_x;

                // Espelhamento vertical

                if (sprite_espelhado_V[j])
                    pixel_y = 15 - pixel_y;

                // Endereço da ROM

                endereco =
                    (sprite_img[j] * 256) +
                    (pixel_y * 16) +
                    pixel_x;

                // Transparência

                if (cor_rom != 8'h00)
                    cor = cor_rom;
            end
        end
    end

endmodule