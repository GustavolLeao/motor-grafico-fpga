// ============================================================================
// Copyright (c) 2013 by Terasic Technologies Inc.
// ============================================================================
//
// Permission:
//
//   Terasic grants permission to use and modify this code for use
//   in synthesis for all Terasic Development Boards and Altera Development 
//   Kits made by Terasic.  Other use of this code, including the selling 
//   ,duplication, or modification of any portion is strictly prohibited.
//
// Disclaimer:
//
//   This VHDL/Verilog or C/C++ source code is intended as a design reference
//   which illustrates how these types of functions can be implemented.
//   It is the user's responsibility to verify their design for
//   consistency and functionality through the use of formal
//   verification methods.  Terasic provides no warranty regarding the use 
//   or functionality of this code.
//
// ============================================================================
//           
//  Terasic Technologies Inc
//  9F., No.176, Sec.2, Gongdao 5th Rd, East Dist, Hsinchu City, 30070. Taiwan
//  
//  
//                     web: http://www.terasic.com/  
//                     email: support@terasic.com
//
// ============================================================================
//Date:  Mon Jun 17 20:35:29 2013
// ============================================================================

`define ENABLE_HPS

module ghrd_top(

      ///////// ADC /////////
      output             ADC_CONVST,
      output             ADC_DIN,
      input              ADC_DOUT,
      output             ADC_SCLK,

      ///////// AUD /////////
      input              AUD_ADCDAT,
      inout              AUD_ADCLRCK,
      inout              AUD_BCLK,
      output             AUD_DACDAT,
      inout              AUD_DACLRCK,
      output             AUD_XCK,

      ///////// CLOCK2 /////////
      input              CLOCK2_50,

      ///////// CLOCK3 /////////
      input              CLOCK3_50,

      ///////// CLOCK4 /////////
      input              CLOCK4_50,

      ///////// CLOCK /////////
      input              CLOCK_50,

      ///////// DRAM /////////
      output      [12:0] DRAM_ADDR,
      output      [1:0]  DRAM_BA,
      output             DRAM_CAS_N,
      output             DRAM_CKE,
      output             DRAM_CLK,
      output             DRAM_CS_N,
      inout       [15:0] DRAM_DQ,
      output             DRAM_LDQM,
      output             DRAM_RAS_N,
      output             DRAM_UDQM,
      output             DRAM_WE_N,

      ///////// FAN /////////
      output             FAN_CTRL,

      ///////// FPGA /////////
      output             FPGA_I2C_SCLK,
      inout              FPGA_I2C_SDAT,

      ///////// GPIO /////////
      inout     [35:0]         GPIO_0,
      inout     [35:0]         GPIO_1,
 

      ///////// HEX0 /////////
      output      [6:0]  HEX0,

      ///////// HEX1 /////////
      output      [6:0]  HEX1,

      ///////// HEX2 /////////
      output      [6:0]  HEX2,

      ///////// HEX3 /////////
      output      [6:0]  HEX3,

      ///////// HEX4 /////////
      output      [6:0]  HEX4,

      ///////// HEX5 /////////
      output      [6:0]  HEX5,

`ifdef ENABLE_HPS
      ///////// HPS /////////
      inout              HPS_CONV_USB_N,
      output      [14:0] HPS_DDR3_ADDR,
      output      [2:0]  HPS_DDR3_BA,
      output             HPS_DDR3_CAS_N,
      output             HPS_DDR3_CKE,
      output             HPS_DDR3_CK_N,
      output             HPS_DDR3_CK_P,
      output             HPS_DDR3_CS_N,
      output      [3:0]  HPS_DDR3_DM,
      inout       [31:0] HPS_DDR3_DQ,
      inout       [3:0]  HPS_DDR3_DQS_N,
      inout       [3:0]  HPS_DDR3_DQS_P,
      output             HPS_DDR3_ODT,
      output             HPS_DDR3_RAS_N,
      output             HPS_DDR3_RESET_N,
      input              HPS_DDR3_RZQ,
      output             HPS_DDR3_WE_N,
      output             HPS_ENET_GTX_CLK,
      inout              HPS_ENET_INT_N,
      output             HPS_ENET_MDC,
      inout              HPS_ENET_MDIO,
      input              HPS_ENET_RX_CLK,
      input       [3:0]  HPS_ENET_RX_DATA,
      input              HPS_ENET_RX_DV,
      output      [3:0]  HPS_ENET_TX_DATA,
      output             HPS_ENET_TX_EN,
      inout       [3:0]  HPS_FLASH_DATA,
      output             HPS_FLASH_DCLK,
      output             HPS_FLASH_NCSO,
      inout              HPS_GSENSOR_INT,
      inout              HPS_I2C1_SCLK,
      inout              HPS_I2C1_SDAT,
      inout              HPS_I2C2_SCLK,
      inout              HPS_I2C2_SDAT,
      inout              HPS_I2C_CONTROL,
      inout              HPS_KEY,
      inout              HPS_LED,
      inout              HPS_LTC_GPIO,
      output             HPS_SD_CLK,
      inout              HPS_SD_CMD,
      inout       [3:0]  HPS_SD_DATA,
      output             HPS_SPIM_CLK,
      input              HPS_SPIM_MISO,
      output             HPS_SPIM_MOSI,
      inout              HPS_SPIM_SS,
      input              HPS_UART_RX,
      output             HPS_UART_TX,
      input              HPS_USB_CLKOUT,
      inout       [7:0]  HPS_USB_DATA,
      input              HPS_USB_DIR,
      input              HPS_USB_NXT,
      output             HPS_USB_STP,
`endif /*ENABLE_HPS*/

      ///////// IRDA /////////
      input              IRDA_RXD,
      output             IRDA_TXD,

      ///////// KEY /////////
      input       [3:0]  KEY,

      ///////// LEDR /////////
      output      [9:0]  LEDR,

      ///////// PS2 /////////
      inout              PS2_CLK,
      inout              PS2_CLK2,
      inout              PS2_DAT,
      inout              PS2_DAT2,

      ///////// SW /////////
      input       [9:0]  SW,

      ///////// TD /////////
      input              TD_CLK27,
      input      [7:0]  TD_DATA,
      input             TD_HS,
      output             TD_RESET_N,
      input             TD_VS,

      ///////// VGA /////////
      output      [7:0]  VGA_B,
      output             VGA_BLANK_N,
      output             VGA_CLK,
      output      [7:0]  VGA_G,
      output             VGA_HS,
      output      [7:0]  VGA_R,
      output             VGA_SYNC_N,
      output             VGA_VS,

      ///////// UART /////////	
      output                     UART_TX,
      input                      UART_RX,
      output                     UART_RTS,
      input                      UART_CTS,

      ///////// QSPI /////////	
      output                     QSPI_FLASH_SCLK,
      inout           [ 3: 0]    QSPI_FLASH_DATA,
      output                     QSPI_FLASH_CE_n,

      ///////// RISC-V JTAG /////////
      input                      RISCV_JTAG_TCK,
      input                      RISCV_JTAG_TDI,
      output                     RISCV_JTAG_TDO,
      input                      RISCV_JTAG_TMS 
);

// internal wires and registers declaration
wire [3:0]  fpga_debounced_buttons;
wire [9:0]  fpga_led_internal;
wire        hps_fpga_reset_n;
wire [2:0]  hps_reset_req;
wire        hps_cold_reset;
wire        hps_warm_reset;
wire        hps_debug_reset;
wire [27:0] stm_hw_events;

// connection of internal logics
assign stm_hw_events    = {{3{1'b0}},SW, fpga_led_internal, fpga_debounced_buttons};



soc_system u0 (
    .clk_clk                               ( CLOCK_50           ),      //                            clk.clk
    .reset_reset_n                         ( hps_fpga_reset_n   ),      //                          reset.reset_n

    .memory_mem_a                          ( HPS_DDR3_ADDR  ),          //                         memory.mem_a
    .memory_mem_ba                         ( HPS_DDR3_BA    ),          //                               .mem_ba
    .memory_mem_ck                         ( HPS_DDR3_CK_P  ),          //                               .mem_ck
    .memory_mem_ck_n                       ( HPS_DDR3_CK_N  ),          //                               .mem_ck_n
    .memory_mem_cke                        ( HPS_DDR3_CKE   ),          //                               .mem_cke
    .memory_mem_cs_n                       ( HPS_DDR3_CS_N  ),          //                               .mem_cs_n
    .memory_mem_ras_n                      ( HPS_DDR3_RAS_N ),          //                               .mem_ras_n
    .memory_mem_cas_n                      ( HPS_DDR3_CAS_N ),          //                               .mem_cas_n
    .memory_mem_we_n                       ( HPS_DDR3_WE_N  ),          //                               .mem_we_n
    .memory_mem_reset_n                    ( HPS_DDR3_RESET_N   ),      //                               .mem_reset_n
    .memory_mem_dq                         ( HPS_DDR3_DQ    ),          //                               .mem_dq
    .memory_mem_dqs                        ( HPS_DDR3_DQS_P ),          //                               .mem_dqs
    .memory_mem_dqs_n                      ( HPS_DDR3_DQS_N ),          //                               .mem_dqs_n
    .memory_mem_odt                        ( HPS_DDR3_ODT   ),          //                               .mem_odt
    .memory_mem_dm                         ( HPS_DDR3_DM    ),          //                               .mem_dm
    .memory_oct_rzqin                      ( HPS_DDR3_RZQ   ),          //                               .oct_rzqin
   		
    .hps_0_hps_io_hps_io_emac1_inst_TX_CLK ( HPS_ENET_GTX_CLK),         //                   hps_0_hps_io.hps_io_emac1_inst_TX_CLK
    .hps_0_hps_io_hps_io_emac1_inst_TXD0   ( HPS_ENET_TX_DATA[0] ),     //                               .hps_io_emac1_inst_TXD0
    .hps_0_hps_io_hps_io_emac1_inst_TXD1   ( HPS_ENET_TX_DATA[1] ),     //                               .hps_io_emac1_inst_TXD1
    .hps_0_hps_io_hps_io_emac1_inst_TXD2   ( HPS_ENET_TX_DATA[2] ),     //                               .hps_io_emac1_inst_TXD2
    .hps_0_hps_io_hps_io_emac1_inst_TXD3   ( HPS_ENET_TX_DATA[3] ),     //                               .hps_io_emac1_inst_TXD3
    .hps_0_hps_io_hps_io_emac1_inst_RXD0   ( HPS_ENET_RX_DATA[0] ),     //                               .hps_io_emac1_inst_RXD0
    .hps_0_hps_io_hps_io_emac1_inst_MDIO   ( HPS_ENET_MDIO ),           //                               .hps_io_emac1_inst_MDIO
    .hps_0_hps_io_hps_io_emac1_inst_MDC    ( HPS_ENET_MDC  ),           //                               .hps_io_emac1_inst_MDC
    .hps_0_hps_io_hps_io_emac1_inst_RX_CTL ( HPS_ENET_RX_DV),           //                               .hps_io_emac1_inst_RX_CTL
    .hps_0_hps_io_hps_io_emac1_inst_TX_CTL ( HPS_ENET_TX_EN),           //                               .hps_io_emac1_inst_TX_CTL
    .hps_0_hps_io_hps_io_emac1_inst_RX_CLK ( HPS_ENET_RX_CLK),          //                               .hps_io_emac1_inst_RX_CLK
    .hps_0_hps_io_hps_io_emac1_inst_RXD1   ( HPS_ENET_RX_DATA[1] ),     //                               .hps_io_emac1_inst_RXD1
    .hps_0_hps_io_hps_io_emac1_inst_RXD2   ( HPS_ENET_RX_DATA[2] ),     //                               .hps_io_emac1_inst_RXD2
    .hps_0_hps_io_hps_io_emac1_inst_RXD3   ( HPS_ENET_RX_DATA[3] ),     //                               .hps_io_emac1_inst_RXD3
    
	  
    .hps_0_hps_io_hps_io_qspi_inst_IO0     ( HPS_FLASH_DATA[0]    ),    //                               .hps_io_qspi_inst_IO0
    .hps_0_hps_io_hps_io_qspi_inst_IO1     ( HPS_FLASH_DATA[1]    ),    //                               .hps_io_qspi_inst_IO1
    .hps_0_hps_io_hps_io_qspi_inst_IO2     ( HPS_FLASH_DATA[2]    ),    //                               .hps_io_qspi_inst_IO2
    .hps_0_hps_io_hps_io_qspi_inst_IO3     ( HPS_FLASH_DATA[3]    ),    //                               .hps_io_qspi_inst_IO3
    .hps_0_hps_io_hps_io_qspi_inst_SS0     ( HPS_FLASH_NCSO    ),       //                               .hps_io_qspi_inst_SS0
    .hps_0_hps_io_hps_io_qspi_inst_CLK     ( HPS_FLASH_DCLK    ),       //                               .hps_io_qspi_inst_CLK
    
    .hps_0_hps_io_hps_io_sdio_inst_CMD     ( HPS_SD_CMD    ),           //                               .hps_io_sdio_inst_CMD
    .hps_0_hps_io_hps_io_sdio_inst_D0      ( HPS_SD_DATA[0]     ),      //                               .hps_io_sdio_inst_D0
    .hps_0_hps_io_hps_io_sdio_inst_D1      ( HPS_SD_DATA[1]     ),      //                               .hps_io_sdio_inst_D1
    .hps_0_hps_io_hps_io_sdio_inst_CLK     ( HPS_SD_CLK   ),            //                               .hps_io_sdio_inst_CLK
    .hps_0_hps_io_hps_io_sdio_inst_D2      ( HPS_SD_DATA[2]     ),      //                               .hps_io_sdio_inst_D2
    .hps_0_hps_io_hps_io_sdio_inst_D3      ( HPS_SD_DATA[3]     ),      //                               .hps_io_sdio_inst_D3
    		  
    .hps_0_hps_io_hps_io_usb1_inst_D0      ( HPS_USB_DATA[0]    ),      //                               .hps_io_usb1_inst_D0
    .hps_0_hps_io_hps_io_usb1_inst_D1      ( HPS_USB_DATA[1]    ),      //                               .hps_io_usb1_inst_D1
    .hps_0_hps_io_hps_io_usb1_inst_D2      ( HPS_USB_DATA[2]    ),      //                               .hps_io_usb1_inst_D2
    .hps_0_hps_io_hps_io_usb1_inst_D3      ( HPS_USB_DATA[3]    ),      //                               .hps_io_usb1_inst_D3
    .hps_0_hps_io_hps_io_usb1_inst_D4      ( HPS_USB_DATA[4]    ),      //                               .hps_io_usb1_inst_D4
    .hps_0_hps_io_hps_io_usb1_inst_D5      ( HPS_USB_DATA[5]    ),      //                               .hps_io_usb1_inst_D5
    .hps_0_hps_io_hps_io_usb1_inst_D6      ( HPS_USB_DATA[6]    ),      //                               .hps_io_usb1_inst_D6
    .hps_0_hps_io_hps_io_usb1_inst_D7      ( HPS_USB_DATA[7]    ),      //                               .hps_io_usb1_inst_D7
    .hps_0_hps_io_hps_io_usb1_inst_CLK     ( HPS_USB_CLKOUT    ),       //                               .hps_io_usb1_inst_CLK
    .hps_0_hps_io_hps_io_usb1_inst_STP     ( HPS_USB_STP    ),          //                               .hps_io_usb1_inst_STP
    .hps_0_hps_io_hps_io_usb1_inst_DIR     ( HPS_USB_DIR    ),          //                               .hps_io_usb1_inst_DIR
    .hps_0_hps_io_hps_io_usb1_inst_NXT     ( HPS_USB_NXT    ),          //                               .hps_io_usb1_inst_NXT
    		  
    .hps_0_hps_io_hps_io_spim1_inst_CLK    ( HPS_SPIM_CLK  ),           //                               .hps_io_spim1_inst_CLK
    .hps_0_hps_io_hps_io_spim1_inst_MOSI   ( HPS_SPIM_MOSI ),           //                               .hps_io_spim1_inst_MOSI
    .hps_0_hps_io_hps_io_spim1_inst_MISO   ( HPS_SPIM_MISO ),           //                               .hps_io_spim1_inst_MISO
    .hps_0_hps_io_hps_io_spim1_inst_SS0    ( HPS_SPIM_SS ),             //                               .hps_io_spim1_inst_SS0
  		
    .hps_0_hps_io_hps_io_uart0_inst_RX     ( HPS_UART_RX    ),          //                               .hps_io_uart0_inst_RX
    .hps_0_hps_io_hps_io_uart0_inst_TX     ( HPS_UART_TX    ),          //                               .hps_io_uart0_inst_TX
	
    .hps_0_hps_io_hps_io_i2c0_inst_SDA     ( HPS_I2C1_SDAT    ),        //                               .hps_io_i2c0_inst_SDA
    .hps_0_hps_io_hps_io_i2c0_inst_SCL     ( HPS_I2C1_SCLK    ),        //                               .hps_io_i2c0_inst_SCL
	
    .hps_0_hps_io_hps_io_i2c1_inst_SDA     ( HPS_I2C2_SDAT    ),        //                               .hps_io_i2c1_inst_SDA
    .hps_0_hps_io_hps_io_i2c1_inst_SCL     ( HPS_I2C2_SCLK    ),        //                               .hps_io_i2c1_inst_SCL
    
    .hps_0_hps_io_hps_io_gpio_inst_GPIO09  ( HPS_CONV_USB_N),           //                               .hps_io_gpio_inst_GPIO09
    .hps_0_hps_io_hps_io_gpio_inst_GPIO35  ( HPS_ENET_INT_N),           //                               .hps_io_gpio_inst_GPIO35
    .hps_0_hps_io_hps_io_gpio_inst_GPIO40  ( HPS_LTC_GPIO),             //                               .hps_io_gpio_inst_GPIO40
    .hps_0_hps_io_hps_io_gpio_inst_GPIO48  ( HPS_I2C_CONTROL),          //                               .hps_io_gpio_inst_GPIO48
    .hps_0_hps_io_hps_io_gpio_inst_GPIO53  ( HPS_LED),                  //                               .hps_io_gpio_inst_GPIO53
    .hps_0_hps_io_hps_io_gpio_inst_GPIO54  ( HPS_KEY),                  //                               .hps_io_gpio_inst_GPIO54
    .hps_0_hps_io_hps_io_gpio_inst_GPIO61  ( HPS_GSENSOR_INT),          //                               .hps_io_gpio_inst_GPIO61

    .hps_0_f2h_stm_hw_events_stm_hwevents  (stm_hw_events),             //        hps_0_f2h_stm_hw_events.stm_hwevents
    .hps_0_h2f_reset_reset_n               (hps_fpga_reset_n),          //                hps_0_h2f_reset.reset_n
    .hps_0_f2h_warm_reset_req_reset_n      (~hps_warm_reset),           //       hps_0_f2h_warm_reset_req.reset_n
    .hps_0_f2h_debug_reset_req_reset_n     (~hps_debug_reset),          //      hps_0_f2h_debug_reset_req.reset_n
	 
	 .mmio_conduit_fila_cheia               (fila_cheia),
	 .mmio_conduit_fila_escrever            (fio_escrever),
    .mmio_conduit_fila_instrucao           (arm_instrucao),

    .hps_0_f2h_cold_reset_req_reset_n      (~hps_cold_reset)            //       hps_0_f2h_cold_reset_req.reset_n
);
  
// Source/Probe megawizard instance
hps_reset hps_reset_inst (
    .source_clk (CLOCK_50),
    .source     (hps_reset_req)
);

altera_edge_detector pulse_cold_reset (
    .clk       (CLOCK_50),
    .rst_n     (hps_fpga_reset_n),
    .signal_in (hps_reset_req[0]),
    .pulse_out (hps_cold_reset)
);
defparam pulse_cold_reset.PULSE_EXT = 6;
defparam pulse_cold_reset.EDGE_TYPE = 1;
defparam pulse_cold_reset.IGNORE_RST_WHILE_BUSY = 1;

altera_edge_detector pulse_warm_reset (
    .clk       (CLOCK_50),
    .rst_n     (hps_fpga_reset_n),
    .signal_in (hps_reset_req[1]),
    .pulse_out (hps_warm_reset)
);
defparam pulse_warm_reset.PULSE_EXT = 2;
defparam pulse_warm_reset.EDGE_TYPE = 1;
defparam pulse_warm_reset.IGNORE_RST_WHILE_BUSY = 1;
  
altera_edge_detector pulse_debug_reset (
    .clk       (CLOCK_50),
    .rst_n     (hps_fpga_reset_n),
    .signal_in (hps_reset_req[2]),
    .pulse_out (hps_debug_reset)
);
defparam pulse_debug_reset.PULSE_EXT = 32;
defparam pulse_debug_reset.EDGE_TYPE = 1;
defparam pulse_debug_reset.IGNORE_RST_WHILE_BUSY = 1;


// COPROCESSADOR: MODULOS INSTANCIADOS

wire reset;

assign reset = ~KEY[0];



reg clknovo_100;
reg lckd;

clknew (
		.refclk(CLOCK_50),   //  refclk.clk
		.rst(reset),      //   reset.reset
		.outclk_0(clknovo_100), // outclk0.clk
		.locked(lckd)    //  locked.export
	);

reg clocknovo;

wire [9:0]SW_estavel; 
	 
Debounce_SW (

    .clk(clocknovo),
    .SW(SW),

    .SW_estavel(SW_estavel)

);	 




wire [9:0] next_x, next_y;

wire [7:0]corpaleta;

always@(posedge CLOCK2_50) begin
    clocknovo <= ~clocknovo;
    
    end
    




// Calculos da GPU

reg [9:0] gpu_x = 0;
reg [9:0] gpu_y = 0;
reg desenhando_quadro = 1; // Flag para saber se a GPU está trabalhando

reg vsync_anterior = 1;
reg swap_request = 0;
reg buffer_exibicao = 0;
wire vsync_borda = (vsync_anterior == 1'b1) && (VGA_VS == 1'b0);

always @(posedge CLOCK_50) begin

    vsync_anterior <= VGA_VS;

    if (desenhando_quadro) begin
        // GPU varrendo a tela 320x240
        if (gpu_x < 10'd319) begin
            gpu_x <= gpu_x + 10'd1;
        end else begin
            gpu_x <= 10'd0;
            if (gpu_y < 10'd239) begin
                gpu_y <= gpu_y + 10'd1;
            end else begin
                gpu_y <= 10'd0;
                desenhando_quadro <= 1'b0;
                swap_request <= 1'b1;
            end
        end
    end

    // Gerencia a troca de buffer no VSYNC
    if (vsync_borda && swap_request) begin
        buffer_exibicao <= ~buffer_exibicao;
        swap_request <= 1'b0;
        desenhando_quadro <= 1'b1;
    end
end

wire [9:0] render_x = gpu_x;
wire [9:0] render_y = gpu_y;

wire [7:0] pixel_color;

wire [7:0] poligono_color;
wire [7:0] sprite_color;
wire [7:0] back_color;


// Comunicação com a Unidade de Controle

wire valid_busca;
wire instrucao_valida;
wire [31:0] instrucao;
wire [31:0] IR;


// =======================================================
// Instanciação do Subsistema Platform Designer (arm_fpga)
// =======================================================



// =======================================================
// Fila
// =======================================================

wire fio_escrever;
wire [31:0] dado_fifo;
wire fila_cheia;
wire valid_leitura_fifo;
wire [31:0] arm_instrucao;
wire fifo_vazia;

fila_fifo minha_fila (
    .clock           (CLOCK_50),
    .reset           (reset),
    .fila_escrever   (fio_escrever),
    .arm_instrucao   (arm_instrucao),
    .fila_cheia      (fila_cheia),
    .unidade_ler     (valid_leitura_fifo),
    .instrucao_saida (dado_fifo),
    .fila_vazia      (fifo_vazia)
);

// Unidade de Busca

unidade_de_busca busca (
    .clk(CLOCK_50),
    .reset(reset),

    .dado_fifo(dado_fifo),
    .fifo_vazia(fifo_vazia),

    .valid_leitura_fifo(valid_leitura_fifo),
    .instrucao(instrucao),
    .instrucao_valida(instrucao_valida),

    .valid_busca(valid_busca)
);


// Registrador de Instrução

registrador_de_instrucao registrador_instrucao (
    .clk(CLOCK_50),
    .reset(reset),

    .instrucao_in(instrucao),
    .instrucao_valid(instrucao_valida),

    .IR(IR)
);


// Banco de Registradores

wire escrever_registrador;
wire [2:0] endereco_escrita;
wire [31:0] dado_escrita;

wire [2:0] endereco_leitura_1;
wire [31:0] dado_leitura_1;

wire [2:0] endereco_leitura_2;
wire [31:0] dado_leitura_2;

banco_registradores banco (
    .clk(CLOCK_50),
    .reset(reset),

    .escrever(escrever_registrador),
    .endereco_escrita(endereco_escrita),
    .dado_escrita(dado_escrita),

    .endereco_leitura_1(endereco_leitura_1),
    .dado_leitura_1(dado_leitura_1),

    .endereco_leitura_2(endereco_leitura_2),
    .dado_leitura_2(dado_leitura_2)
);


// Sinais da Unidade de Controle

wire valid_rasterizador;
wire busy_rasterizador;
wire done_rasterizador;

wire [2:0] indice_rasterizador;
wire [8:0] config_x_rasterizador;
wire [7:0] config_y_rasterizador;
wire [8:0] largura_rasterizador;
wire [7:0] altura_rasterizador;
wire [7:0] cor_rasterizador;
wire tipo_rasterizador;
wire ativo_rasterizador;
wire mover_rasterizador;
wire [1:0] direcao_rasterizador;


wire valid_sprite;
wire busy_sprite;
wire done_sprite;

wire [4:0] indice_sprite;
wire [8:0] config_x_sprite;
wire [7:0] config_y_sprite;
wire [4:0] config_img_sprite;
wire ativo_sprite;
wire espelhado_H_sprite;
wire espelhado_V_sprite;
wire mover_sprite;
wire [1:0] direcao_sprite;


wire valid_background;
wire busy_background;
wire done_background;

wire [4:0] indice_background;
wire mover_background;
wire [1:0] direcao_background;


wire valid_paleta;
wire [7:0] endereco_paleta;
wire [7:0] codigo_cor_paleta;

wire [31:0] issp_source;
	wire [9:0] issp_probe;
	
	altsource_probe #(
		.sld_instance_index (0),
		.source_width (32),
		.probe_width (10),
        .source_initial_value ("0"),
		.enable_metastability ("NO")
	) issp_inst (
		.source (issp_source),
		.probe  (issp_probe)
    );

	 
	
	

// Unidade de Controle

Unidade_Controle controle (
    .clk(CLOCK_50),
    .reset(reset),

    // Busca
    .valid_busca(valid_busca),
    .instrucao_valida(instrucao_valida),

    // Instrução
    .IR(IR),

	 // Instrução
    //.IR(issp_source),
	 
    // VSYNC
    .vsync(VGA_VS),

    // Banco de registradores
    .reg_write(escrever_registrador),
    .reg_endereco_escrita(endereco_escrita),
    .reg_dado_escrita(dado_escrita),

    .reg_endereco_leitura1(endereco_leitura_1),
    .reg_dado_leitura1(dado_leitura_1),

    .reg_endereco_leitura2(endereco_leitura_2),
    .reg_dado_leitura2(dado_leitura_2),

    // Rasterizador
    .valid_rasterizador(valid_rasterizador),
    .busy_rasterizador(busy_rasterizador),
    .done_rasterizador(done_rasterizador),

    .raster_indice(indice_rasterizador),
    .raster_x(config_x_rasterizador),
    .raster_y(config_y_rasterizador),
    .raster_largura(largura_rasterizador),
    .raster_altura(altura_rasterizador),
    .raster_cor(cor_rasterizador),
    .raster_tipo(tipo_rasterizador),
    .raster_visibilidade(ativo_rasterizador),
    .raster_mover(mover_rasterizador),
    .raster_direcao(direcao_rasterizador),

    // Sprite
    .valid_sprite(valid_sprite),
    .busy_sprite(busy_sprite),
    .done_sprite(done_sprite),

    .sprite_indice(indice_sprite),
    .sprite_x(config_x_sprite),
    .sprite_y(config_y_sprite),
    .sprite_mif(config_img_sprite),
    .sprite_visibilidade(ativo_sprite),
    .sprite_espelhado_H(espelhado_H_sprite),
    .sprite_espelhado_V(espelhado_V_sprite),
    .sprite_mover(mover_sprite),
    .sprite_direcao(direcao_sprite),

    // Background
    .valid_background(valid_background),
    .busy_background(busy_background),
    .done_background(done_background),

    .background_indice(indice_background),
    .background_mover(mover_background),
    .background_direcao(direcao_background),

    // Palette
    .valid_palette(valid_paleta),
    // Como os fios de busy/done da paleta não foram criados no seu arquivo principal, 
    // coloquei valores fixos para não dar erro. Se a paleta for instantânea, done fica em 1.
    .busy_palette(busy_paleta), 
    .done_palette(done_paleta), 
    .palette_endereco(endereco_paleta),
    .palette_cor(codigo_cor_paleta)
);

// Motor do Rasterizador

Motor_Rasterizador rasterizador (
    .clk(CLOCK_50),
	 .reset(reset),
    .next_x(render_x),
    .next_y(render_y),

    .valid(valid_rasterizador),

    .indice(indice_rasterizador),

    .config_x(config_x_rasterizador),
    .config_y(config_y_rasterizador),
    .config_largura(largura_rasterizador),
    .config_altura(altura_rasterizador),
    .config_cor(cor_rasterizador),
    .config_tipo(tipo_rasterizador),
    .config_ativo(ativo_rasterizador),

    .mover(mover_rasterizador),
    .direcao(direcao_rasterizador),

    .busy(busy_rasterizador),
    .done(done_rasterizador),

    .color_index(corpaleta)
);


// Motor dos Sprites

Motor_sprite sprite (
    .clk(CLOCK_50),
	 .reset(reset),
    .next_x(render_x),
    .next_y(render_y),

    .valid(valid_sprite),

    .indice(indice_sprite),

    .config_x(config_x_sprite),
    .config_y(config_y_sprite),
    .config_img(config_img_sprite),
    .config_ativo(ativo_sprite),
    .config_espelhado_H(espelhado_H_sprite),
    .config_espelhado_V(espelhado_V_sprite),

    .mover(mover_sprite),
    .direcao(direcao_sprite),

    .busy(busy_sprite),
    .done(done_sprite),

    .cor(sprite_color)
);


// Motor do Background

Motor_Background background (
    .clk(CLOCK_50),
	 .reset(reset),
    .next_x(render_x),
    .next_y(render_y),

    .valid(valid_background),

    .indice_tilemap(indice_background),

    .mover(mover_background),
    .direcao(direcao_background),

    .busy(busy_background),
    .done(done_background),

    .indice_cor(back_color)
);


// Paleta

// Durante a programação, usa o endereço vindo da UC.
// Durante a renderização, usa o índice de cor do rasterizador.

wire [7:0] endereco_paleta_atual;

wire busy_paleta, done_paleta;

assign endereco_paleta_atual =
    valid_paleta ? endereco_paleta : corpaleta;

paleta paleta_inst (
    .clk(CLOCK_50),

    .valid(valid_paleta),
	 .busy(busy_paleta),
	 .done(done_paleta),
    .endereco(endereco_paleta_atual),
    .codigo_cor(codigo_cor_paleta),

    .cor(poligono_color)
);


// Fio para o compositor

wire [7:0] compositor_color;

Compositor compositor (
    .corPoligono(poligono_color),
    .corSprite(sprite_color),
    .corBack(back_color),

    .mostrarPoligono(1'b1),
    .mostrarSprite(1'b1),
    .mostrarBack(1'b1),

    .pixel_color(compositor_color)
);

// Double Buffer

// Calculamos o endereço.
wire [16:0] write_addr_calc;
assign write_addr_calc = (gpu_y * 17'd320) + gpu_x;

// Atraso de 1 clock.
reg [16:0] write_addr_pipe1 = 0;
reg we_pipe1 = 0;

always @(posedge CLOCK_50) begin
    // 1 único estágio de atraso.
    write_addr_pipe1 <= write_addr_calc;
    we_pipe1         <= desenhando_quadro;
end

// Endereço de leitura (Do VGA a 25MHz).
wire [16:0] read_addr;
wire [8:0] fb_x = next_x >> 1; 
wire [7:0] fb_y = next_y >> 1; 
assign read_addr = (fb_y * 17'd320) + fb_x;

// Sinais de controle de escrita.
wire fb0_write_enable = (buffer_exibicao == 1'b1) && we_pipe1;
wire fb1_write_enable = (buffer_exibicao == 1'b0) && we_pipe1;

wire [7:0] fb0_read_data;
wire [7:0] fb1_read_data;

frame_buffer frame_buffer_0 (
    .write_clk(CLOCK_50), 
    .read_clk(clocknovo),    
    
    .write_enable(fb0_write_enable),
    .write_address(write_addr_pipe1), // <-- Atraso de 1 clock
    .write_data(compositor_color),
    
    .read_address(read_addr),
    .read_data(fb0_read_data)
);

frame_buffer frame_buffer_1 (
    .write_clk(CLOCK_50), 
    .read_clk(clocknovo),    
    
    .write_enable(fb1_write_enable),
    .write_address(write_addr_pipe1), // <-- Atraso de 1 clock
    .write_data(compositor_color),
    
    .read_address(read_addr),
    .read_data(fb1_read_data)
);

wire [7:0] framebuffer_color;
assign framebuffer_color = (buffer_exibicao == 1'b0) ? fb0_read_data : fb1_read_data;

// Instantiate VGA driver                   
vga_driver draw   ( .clock(clocknovo),        // 25 MHz PLL
                    .reset(reset),      // Active high reset, manipulated by instantiating module
                    .color_in(framebuffer_color), // Pixel color (RRRGGGBB) for pixel being drawn
                    .next_x(next_x),        // X-coordinate (range [0, 639]) of next pixel to be drawn
                    .next_y(next_y),        // Y-coordinate (range [0, 479]) of next pixel to be drawn
                    .hsync(VGA_HS),         // All of the connections to the VGA screen below
                    .vsync(VGA_VS),
                    .red(VGA_R),
                    .green(VGA_G),
                    .blue(VGA_B),
                    .sync(VGA_SYNC_N),
                    .clk(VGA_CLK),
                    .blank(VGA_BLANK_N)
);







endmodule

  