module unidade_de_busca (
    input  wire        clk,                 
    input  wire        reset,               

    input  wire [31:0] dado_fifo,           // instrução de 32 bits que vem da FIFO
    input  wire        fifo_vazia,          // se ta vazio é 1, se tem dados é 0
    output reg         valid_leitura_fifo,  // pode ler o dado da fifo

    output reg  [31:0] instrucao,           // a instrução de 32 bits que foi lida da FIFO
    output reg         instrucao_valida,    // a instrução atual é válida e pode ser usada

    // Sinais de Controle
    input  wire        valid_busca          // ver se pode buscar uma nova instrução
);

    // Definição dos estados da Máquina de Estados (FSM)
    localparam PARADO  = 2'b00;             // Estado inicial: aguardando permissão para buscar
    localparam LEITURA = 2'b01;             // Estado de leitura: envia o pedido para a FIFO
    localparam VALIDO  = 2'b10;             // Estado válido: captura o dado e avisa que ele está pronto

    reg [1:0] estado;

    always @(posedge clk) begin

        if (reset) begin

            estado             <= PARADO;
            valid_leitura_fifo   <= 1'b0;
            instrucao          <= 32'b0;
            instrucao_valida   <= 1'b0;

        end else begin

            valid_leitura_fifo   <= 1'b0;
            instrucao_valida   <= 1'b0;

            case (estado)

                PARADO: begin

                    // Se a busca estiver habilitada e a FIFO tiver alguma instrução
                    if (valid_busca && !fifo_vazia) begin
                        estado <= LEITURA; 
                    end

                end

                LEITURA: begin

                    valid_leitura_fifo <= 1'b1;
                    estado <= VALIDO;

                end

                VALIDO: begin

                    // Pega o dado que saiu da FIFO
                    instrucao <= dado_fifo;

						  // o Registrador de Instrução (IR) pode ler esta instrução
                    instrucao_valida <= 1'b1;

                    // Volta para o estado inicial
                    estado <= PARADO;

                end

                default: begin
                    
                    estado <= PARADO;
                end

            endcase
        end
    end

endmodule











