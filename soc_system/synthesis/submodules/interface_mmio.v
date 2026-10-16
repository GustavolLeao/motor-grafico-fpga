module interface_mmio (
    input clock,
    input reset,

    // Barramento Avalon ligados ao ARM
    input wire [1:0]  avs_endereco,   // Qual endereço o ARM quer acessar?
    input wire        avs_escrever,   // Sinal ARM quer escrever dados 
    input wire        avs_ler,        // Sinal ARM quer ler dados
    input wire [31:0] avs_instrucao,  // Instrução de 32 bits 
    output reg [31:0] avs_SinalFila,  // Sinal enviado para o ARM da leitura de Status da Fila

    // Sinais da Fila
    input wire coe_fila_cheia, //Pulso enviado pela fila para indicar se está cheia ou não
    output reg coe_fila_escrever,  // Pulso de ativação para a fila gravar a instrução
    output reg [31:0] coe_fila_instrucao
);

    // ARM para FPGA (Lógica Sequencial)
    always @(posedge clock or posedge reset) begin  

        if (reset) begin

            coe_fila_escrever <= 1'd0; // Iniciar sinal de escrita da fila em zero
            coe_fila_instrucao <= 32'd0; // Instruções no endereço de dados da fila zeradas

        end else begin
            // Garantir sinal de escrita em zero a cada subida do clock
            coe_fila_escrever <= 1'd0;
            
            // Se o sinal de escrita do ARM está ativo e o endereço que ele quer é o de dados
            if (avs_escrever && (avs_endereco == 2'b00)) begin
                coe_fila_instrucao <= avs_instrucao; // Escreve as instruções do ARM no endereço da fila
                coe_fila_escrever <= 1'd01; // Ativa o sinal de escrita da fila para gravar a instrução
            end
        end
    end
    
    // FPGA para ARM (Independente do clock, lógica combinacional)
    always @(*) begin
        // Se o sinal de leitura do ARM estiver ativo e o endereço que deseja ler é o de sinal da fila
        if (avs_ler && (avs_endereco == 2'd01)) begin
            avs_SinalFila = {31'd0, coe_fila_cheia}; // Passa o status enviado pela fila
        end else begin
            avs_SinalFila = 32'd0; // Caso contrário, mantém por padrão sinal enviado pro ARM de fila vazia
        end
    end

endmodule