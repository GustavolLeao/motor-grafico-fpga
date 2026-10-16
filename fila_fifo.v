module fila_fifo(
    input clock,
    input reset,

    // Ligação entre Interface MMIO e Fila
    input fila_escrever, 
    input wire [31:0] arm_instrucao, 
    output reg fila_cheia, 

    // Ligação Fila e Unidade de Busca
    input unidade_ler, 
    output wire [31:0] instrucao_saida, // Alterado para wire (combinacional)
    output reg fila_vazia
);

    parameter CAPACIDADE = 16;
    
    reg [31:0] fila [0:CAPACIDADE-1];

    reg [3:0] ponteiro_escrever; 
    reg [3:0] ponteiro_ler; 

    reg [4:0] quantidade; 

    // ATUALIZAÇÃO FWFT: A instrução atual fica imediatamente visível na saída
    assign instrucao_saida = fila[ponteiro_ler];

    always@(posedge clock or posedge reset) begin

        if (reset) begin
            ponteiro_escrever <= 4'b0;
            ponteiro_ler <= 4'b0;
            quantidade <= 5'b0;

        // ARM e Unidade de Controle tentando escrever instrução e ler instrução ao mesmo tempo
        end else if (fila_escrever == 1'b1 && unidade_ler == 1'b1 && fila_cheia == 1'b0 && fila_vazia == 1'b0) begin
            // Unidade de Busca lê (avança ponteiro)
            ponteiro_ler <= ponteiro_ler + 1'b1;
            
            // ARM escreve
            fila[ponteiro_escrever] <= arm_instrucao;
            ponteiro_escrever <= ponteiro_escrever + 1'b1;
            
            // Quantidade mantém-se igual (entra 1, sai 1)

        // ARM tentando escrever a instrução na fila e ela não está cheia
        end else if (fila_escrever == 1'b1 && fila_cheia == 1'b0) begin
            fila[ponteiro_escrever] <= arm_instrucao; 
            ponteiro_escrever <= ponteiro_escrever + 1'b1; 
            quantidade <= quantidade + 1'b1; 

        // Unidade de Busca tentando ler instrução na fila e ela não está vazia
        end else if (unidade_ler == 1'b1 && fila_vazia == 1'b0) begin
            ponteiro_ler <= ponteiro_ler + 1'b1;
            quantidade <= quantidade - 1'b1;
        end
    end

    // Atualização combinacional de status
    always @(*) begin
        fila_cheia = (quantidade == CAPACIDADE); 
        fila_vazia = (quantidade == 5'b0); 
    end
    
endmodule