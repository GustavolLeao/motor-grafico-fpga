module Motor_Background (

    input wire        clk,
    input wire        reset,

    input wire [9:0]  next_x,
    input wire [9:0]  next_y,

    // Interface de comando

    input wire        valid,
    input wire [2:0]  indice_tilemap,
    input wire        mover,
    input wire [1:0]  direcao,

    output reg        busy,
    output reg        done,

    // Saída de vídeo

    output wire [7:0] indice_cor

);

    // Direções

    localparam CIMA     = 2'b00;
    localparam BAIXO    = 2'b01;
    localparam ESQUERDA = 2'b10;
    localparam DIREITA  = 2'b11;


    // Resolução lógica

    wire [9:0] x_logico;
    wire [9:0] y_logico;

    assign x_logico = next_x;
    assign y_logico = next_y;


    // Tilemaps

    reg [8:0] tilemap_0 [0:1199];
    reg [8:0] tilemap_1 [0:1199];
    reg [8:0] tilemap_2 [0:1199];
    reg [8:0] tilemap_3 [0:1199];

    // Tilemap selecionado

    reg [2:0] tilemap_selecionado;


    // Deslocamento

    reg signed [10:0] deslocamento_x;
    reg signed [10:0] deslocamento_y;


    // Contador de velocidade

    reg [21:0] contador;

    localparam LIMITE = 500_000 - 1;


    // FSM

    localparam IDLE     = 2'b00;
    localparam EXECUTAR = 2'b01;
    localparam DONE     = 2'b10;

    reg [1:0] estado;


    // Inicialização

    initial begin
        $readmemh("tilemap_0.hex", tilemap_0);
        $readmemh("tilemap_1.hex", tilemap_1);
        $readmemh("tilemap_2.hex", tilemap_2);
        $readmemh("tilemap_3.hex", tilemap_3);
    end


    // Controle do motor
// Controle do motor
    always @(posedge clk or posedge reset) begin
        if (reset) begin
            tilemap_selecionado <= 3'd0;
            deslocamento_x <= 11'sd0;
            deslocamento_y <= 11'sd0;
            estado <= IDLE;
            busy <= 1'b0;
            done <= 1'b0;
        end
        else begin
            done <= 1'b0;

            case (estado)
                // IDLE
                IDLE: begin
                    busy <= 1'b0;

                    if (valid) begin
                        busy <= 1'b1;

                        // APLICA CONFIGURAÇÃO IMEDIATAMENTE
                        if (!mover) begin
                            if (indice_tilemap == 3'd0) begin
                                tilemap_selecionado <= 3'd0;
                            end
                            else if (indice_tilemap == 3'd1) begin
                                tilemap_selecionado <= 3'd1;
                            end
                            else if (indice_tilemap == 3'd2) begin
                                tilemap_selecionado <= 3'd2;
                            end
                            else if (indice_tilemap == 3'd3) begin
                                tilemap_selecionado <= 3'd3;
                            end
                        end
                        else begin
                            case (direcao)
                                CIMA:     deslocamento_y <= deslocamento_y - 11'sd1;
                                BAIXO:    deslocamento_y <= deslocamento_y + 11'sd1;
                                ESQUERDA: deslocamento_x <= deslocamento_x - 11'sd1;
                                DIREITA:  deslocamento_x <= deslocamento_x + 11'sd1;
                                default: ;
                            endcase
                        end

                        estado <= DONE;
                    end
                end

                // EXECUTAR (Obsoleto)
                EXECUTAR: begin
                    estado <= DONE;
                end

                // DONE
                DONE: begin
                    busy <= 1'b0;
                    done <= 1'b1;
                    estado <= IDLE;
                end

                default: begin
                    estado <= IDLE;
                    busy <= 1'b0;
                    done <= 1'b0;
                end
            endcase
        end
    end


    // Contador de velocidade

    always @(posedge clk or posedge reset) begin
        if (reset) begin
            contador <= 22'd0;
        end
        else begin
            if (contador == LIMITE) begin
                contador <= 22'd0;
            end
            else begin
                contador <= contador + 22'd1;
            end
        end
    end


    // Coordenada do mapa

    wire signed [10:0] mapa_x;
    wire signed [10:0] mapa_y;

    assign mapa_x =
        $signed({1'b0, x_logico}) + deslocamento_x;

    assign mapa_y =
        $signed({1'b0, y_logico}) + deslocamento_y;


    // Verifica se está dentro do mapa

    wire dentro_do_mapa;

    assign dentro_do_mapa =
        (mapa_x >= 0) && (mapa_x < 320) &&
        (mapa_y >= 0) && (mapa_y < 240);


    // Tile atual

    wire [6:0] tile_x;
    wire [6:0] tile_y;

    assign tile_x = mapa_x / 8;
    assign tile_y = mapa_y / 8;


    // Pixel dentro do tile

    wire [2:0] pixel_x;
    wire [2:0] pixel_y;

    assign pixel_x = mapa_x % 8;
    assign pixel_y = mapa_y % 8;


    // Endereço no tilemap

    wire [10:0] endereco_tilemap;

    assign endereco_tilemap =
        tile_y * 40 + tile_x;


    // Tile ID

    reg [8:0] tile_id;

    always @(*) begin
        if (dentro_do_mapa) begin
            case (tilemap_selecionado)
                3'd0: tile_id = tilemap_0[endereco_tilemap];
                3'd1: tile_id = tilemap_1[endereco_tilemap];
                3'd2: tile_id = tilemap_2[endereco_tilemap];
                3'd3: tile_id = tilemap_3[endereco_tilemap];
                default: tile_id = 9'h000;
            endcase
        end
        else begin
            tile_id = 9'h000;
        end
    end


    // Endereço do pixel dentro do tile

    wire [5:0] endereco_pixel;

    assign endereco_pixel =
        pixel_y * 8 + pixel_x;


    // Endereço da ROM dos tiles

    wire [14:0] endereco_rom;

    assign endereco_rom =
        {tile_id, endereco_pixel};


    // ROM dos tiles

    wire [7:0] dado_rom;

    tiles400 memoria_tiles (
        .address(endereco_rom),
        .clock(clk),
        .q(dado_rom)
    );


    // Saída

    assign indice_cor =
        dentro_do_mapa ?
        dado_rom :
        8'h00;

endmodule