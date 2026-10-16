module banco_registradores (
    input wire        clk,
    input wire        reset,

	 
    // PORTA DE ESCRITA

    input wire        escrever,
    input wire [2:0]  endereco_escrita,
    input wire [23:0] dado_escrita,


    // PORTA DE LEITURA 1


    input wire [2:0]  endereco_leitura_1,
    output wire [23:0] dado_leitura_1,


    // PORTA DE LEITURA 2


    input wire [2:0]  endereco_leitura_2,
    output wire [23:0] dado_leitura_2

);


    // 8 REGISTRADORES DE 24 BITS


    reg [23:0] registradores [0:7];

    integer i;


    // ESCRITA


    always @(posedge clk) begin

//			registradores[1] <= 24'd4;
        if (reset) begin

            for (i = 0; i < 8; i = i + 1)
                registradores[i] <= 23'b0;


        end

        else if (escrever) begin

            registradores[endereco_escrita] <=
                dado_escrita;

        end

    end



    // LEITURAS
	 
	 
    assign dado_leitura_1 =
        registradores[endereco_leitura_1];

    assign dado_leitura_2 =
        registradores[endereco_leitura_2];


endmodule