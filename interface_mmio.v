module interface_mmio (
    input clock,
    input reset,

    // Barramento Avalon ligados ao ARM
    input wire [1:0]  avs_endereco,
    input wire        avs_escrever,
    input wire        avs_ler,
    input wire [31:0] avs_instrucao,
    output reg [31:0] avs_SinalFila,

    // Sinais da Fila
    input wire        coe_fila_cheia,
    output reg        coe_fila_escrever,
    output reg [31:0] coe_fila_instrucao
);

    // Registrador para guardar o estado anterior do sinal de escrita
    reg avs_escrever_anterior;

    // ARM para FPGA (Lógica Sequencial com Detetor de Borda)
    always @(posedge clock or posedge reset) begin  
        if (reset) begin
            coe_fila_escrever <= 1'b0;
            coe_fila_instrucao <= 32'd0;
            avs_escrever_anterior <= 1'b0;
        end else begin
            // Armazena o estado do ciclo anterior
            avs_escrever_anterior <= avs_escrever;
            
            // Garante que o pulso da FIFO desce imediatamente no ciclo seguinte
            coe_fila_escrever <= 1'b0;
            
            // DETETOR DE BORDA DE SUBIDA: Só grava se "escrever" for 1 AGORA, mas era 0 ANTES
            if (avs_escrever && !avs_escrever_anterior && (avs_endereco == 2'b00)) begin
                coe_fila_instrucao <= avs_instrucao; 
                coe_fila_escrever <= 1'b1; // Pulso único e perfeito de 1 clock
            end
        end
    end
    
    // FPGA para ARM (Independente do clock, lógica combinacional)
    always @(*) begin
        if (avs_ler && (avs_endereco == 2'd01)) begin
            avs_SinalFila = {31'd0, coe_fila_cheia};
        end else begin
            avs_SinalFila = 32'd0;
        end
    end

endmodule