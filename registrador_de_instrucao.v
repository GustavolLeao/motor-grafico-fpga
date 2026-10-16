module registrador_de_instrucao (
    input  wire        clk,
    input  wire        reset,

    input  wire [31:0] instrucao_in,
    input  wire        instrucao_valid,

    output reg  [31:0] IR
);

    always @(posedge clk) begin

        if (reset) begin
            IR <= 32'b0;
        end

        else if (instrucao_valid) begin
            IR <= instrucao_in;
        end

    end

endmodule