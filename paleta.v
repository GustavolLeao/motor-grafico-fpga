module paleta (
    input wire        clk,

    // Comunicação com a Unidade de Controle
    input wire        valid,
    input wire [7:0]  endereco,
    input wire [7:0]  codigo_cor, // Ajustado para 8 bits

    output reg        busy,
    output reg        done,

    // Saída de vídeo (Leitura combinacional contínua)
    output wire [7:0] cor
);

    reg [7:0] paleta_memoria [0:255];

    initial begin
        paleta_memoria[0] = 8'b00000000; // transparente
        paleta_memoria[1] = 8'b11111111; // branco
        paleta_memoria[2] = 8'b11100000; // vermelho
        paleta_memoria[3] = 8'b00011100; // verde
        paleta_memoria[4] = 8'b00000011; // azul
        paleta_memoria[5] = 8'b11111100; // amarelo
    end

    // Leitura assíncrona/combinacional da cor (usada durante a renderização na tela)
    assign cor = paleta_memoria[endereco];

    // FSM de controle (Idêntica aos motores para manter o padrão)
    localparam ESTADO_IDLE = 1'b0;
    localparam ESTADO_DONE = 1'b1;

    reg estado;

    initial begin
        estado = ESTADO_IDLE;
        busy = 1'b0;
        done = 1'b0;
    end

    always @(posedge clk) begin
        case (estado)
            // IDLE
            ESTADO_IDLE: begin
                busy <= 1'b0;
                done <= 1'b0;

                if (valid) begin
                    busy <= 1'b1;
                    
                    // Realiza a escrita na memória no momento exato
                    paleta_memoria[endereco] <= codigo_cor;
                    
                    estado <= ESTADO_DONE;
                end
            end

            // DONE
            ESTADO_DONE: begin
                busy <= 1'b0;
                done <= 1'b1; // Envia o sinal para a Unidade de Controle avançar
                estado <= ESTADO_IDLE;
            end

            default: begin
                estado <= ESTADO_IDLE;
                busy <= 1'b0;
                done <= 1'b0;
            end
        endcase
    end

endmodule