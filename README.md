# Coprocessador Gráfico Programável em FPGA
### MI de Sistemas Digitais (TEC499) - TP03

---

## Introdução

Os sistemas computacionais exigem, de forma recorrente, a exibição de interfaces visuais complexas, o que demanda a atualização constante de uma vasta matriz de pixels para a formação de quadros na tela. Em um cenário onde a CPU (Central Processing Unit) precisa controlar individualmente cada módulo gráfico, o software passa a conhecer detalhes internos do hardware, o que aumenta a complexidade da aplicação e sobrecarrega o processador principal. Diante disso, surge a necessidade de um coprocessador gráfico programável, capaz de interpretar comandos de alto nível e acionar, por conta própria, os recursos necessários para a geração das imagens.

Diante dessa abordagem, foi solicitado aos alunos da disciplina TEC499 (Sistemas Digitais) da Universidade Estadual de Feira de Santana a evolução do núcleo gráfico desenvolvido no Problema 1 para um coprocessador gráfico programável em FPGA, inspirado na arquitetura de consoles de 16 bits. O novo sistema adota uma organização semelhante à de processadores convencionais, contando com uma ISA (Instruction Set Architecture) de 32 bits, unidade de busca de instruções, registrador de instrução, unidade de controle, banco de registradores, ULA, datapath e registrador de status. Os motores de background, sprites e rasterização de polígonos passam a atuar como unidades funcionais, cujas operações ocorrem como consequência da execução de instruções. O projeto utiliza a placa DE1-SoC e a linguagem de descrição de hardware Verilog, sendo preparado para, em etapas posteriores, ser controlado por um driver Linux em Assembly ARM para a execução de uma aplicação de jogo em C.

## Pré-requisitos

É necessário ter um kit de desenvolvimento DE1-SoC, pertencente à família Cyclone V. Também é necessário possuir o software Quartus Prime Lite Edition (a versão 23.1 foi utilizada no desenvolvimento) instalado no dispositivo para implementar o coprocessador na placa e realizar a síntese do hardware. Além disso, o uso de um monitor com interface VGA é essencial para a visualização das imagens geradas e dos resultados obtidos.

Os requisitos de hardware devem ser estritamente seguidos, visto que a pinagem e a configuração dos clocks estão estabelecidas para o kit de desenvolvimento especificado.

## Como instalar?

O projeto tem duas partes: o **hardware** (síntese e gravação na FPGA pelo Quartus) e o **software** (driver em Assembly ARM e programas em C, compilados e executados no Linux embarcado da DE1-SoC).

### 1. Gravação do hardware na FPGA

1. Faça o download do projeto como arquivo .zip do repositório e extraia no seu computador.
2. Abra o software Quartus Prime.
3. Acesse File > Open Project.
4. Encontre e selecione o arquivo `soc_system.qpf` na pasta raiz extraída.
5. Com o projeto aberto, inicie a compilação clicando no ícone de compilação ou acessando Processing > Start Compilation.
6. Vá em Tools > Programmer.
7. Clique em Hardware Setup para garantir que o Quartus reconheceu a placa conectada.
8. Carregue o arquivo .sof gerado e clique em Start para gravar o hardware na FPGA.
9. Conecte o monitor à saída VGA da placa. A cena será exibida a partir da execução dos programas do passo 2.

### 2. Execução do driver e dos programas no Linux da placa

Os arquivos do driver ficam na pasta `Driver/`:

| Arquivo | Descrição |
|---|---|
| `gpu_driver.S` | Driver em Assembly ARM. Monta as instruções da ISA e as envia à fila (FIFO) via MMIO |
| `gpu_isa.h` | Cabeçalho em C com os protótipos das funções `asm_*` do driver |
| `teste.c` | Programa com menu interativo (uma opção por instrução da ISA) e cenas animadas |
| `teste2.c` | Cenas criativas de demonstração |
| `teste3.c` | Bateria de testes passo a passo das instruções, incluindo `BG_TILE` e `BG_MOVER` |
| `cena.c` | Programa independente que acessa o MMIO diretamente, sem usar o driver em Assembly |

**Passo a passo:**

1. Com o hardware já gravado na FPGA, conecte seu computador à rede da placa e acesse-a por SSH, usando o IP impresso na placa:
```bash
   ssh aluno@<IP_DA_PLACA>
```
2. No seu computador, copie a pasta `Driver` para a placa com `scp`:
```bash
   scp -r Driver aluno@<IP_DA_PLACA>:/home/aluno/
```
3. Na placa, entre na pasta:
```bash
   cd ~/Driver
```
4. Compile o programa desejado junto com o driver em Assembly, usando o `gcc`:
```bash
   gcc -std=gnu99 teste.c gpu_driver.S -o programa_gpu -lm
```
   Para os demais programas, troque o arquivo `.c` e o nome do executável:
```bash
   gcc -std=gnu99 teste2.c gpu_driver.S -o programa_gpu2 -lm
   gcc -std=gnu99 teste3.c gpu_driver.S -o teste3 -lm
```

5. Execute o programa. O acesso à memória física (`/dev/mem`) exige privilégios de administrador:
```bash
   sudo ./programa_gpu
```
6. Acompanhe a saída no monitor VGA. Nos programas com animação, use `Ctrl+C` para voltar ao menu ou encerrar.

> **Observação:** a pasta `Driver/` já contém executáveis ARM pré-compilados (`programa_gpu`, `programa_gpu2`, `teste3` e `trio`). Eles podem ser executados diretamente na placa com `sudo ./<nome>`, sem recompilar.

## Requisitos do problema

O problema exige a evolução do núcleo gráfico desenvolvido anteriormente para um coprocessador gráfico programável, no qual a aplicação não precisa conhecer os detalhes internos de cada módulo e apenas envia comandos, cabendo ao hardware interpretá-los e acionar os recursos necessários. Os requisitos estipulados foram:

### Requisitos funcionais

* Hardware do coprocessador descrito em Verilog e implementado na placa DE1-SoC.
* ISA (Instruction Set Architecture) de 32 bits, com formatos de instrução, codificação e decodificação documentados pela equipe.
* Arquitetura baseada em Datapath e Unidade de Controle, implementando o ciclo de busca/execução de instruções e contendo, no mínimo:
   * Unidade de busca de instruções;
   * Registrador de Instrução (IR);
   * Unidade de Controle;
   * Banco de registradores;
   * Unidade Lógica e Aritmética (ULA);
   * Registrador de status;
   * Datapath.
* Obtenção das instruções por **busca passiva**, a partir de uma fila de comandos (FIFO) alimentada pelo processador ARM por meio da interface MMIO.
* Programa em Assembly ARM (driver) que monte as instruções da ISA e as envie à fila de comandos, respeitando o controle de fila cheia.
* Biblioteca em C (`gpu_isa.h`) que exponha as instruções do coprocessador para as aplicações, sem que estas conheçam os detalhes internos de cada módulo gráfico.
* Execução de toda operação gráfica como consequência de uma instrução pertencente à ISA definida.
* Comunicação entre o controle e as unidades funcionais por meio de sinais do tipo `valid`, `busy` e `done`.
* Mecanismo de sincronização com o VGA.
* Programa de demonstração escrito em Assembly da ISA criada pela equipe, com sintaxe própria definida pelo grupo.
* Programas de teste em C, compilados com `gcc` e executados no terminal do Linux embarcado da placa, que exercitem todas as instruções da ISA.

## Recursos utilizados

* **Placa DE1-SoC:** Hardware principal. O chip FPGA (família Cyclone V) recebe a descrição do projeto e instancia os blocos de memória. O HPS (Hard Processor System) da mesma placa, com processador ARM Cortex-A9 executando Linux embarcado, é usado para rodar o driver e os programas de teste.
* **Quartus Prime Lite 23.1:** Software que permite a criação, depuração e otimização de códigos em linguagem de descrição de hardware. Foi utilizado para gerar os IP Cores (circuitos lógicos pré-projetados) necessários (ALTSYNCRAM para memórias dedicadas e Altera PLL para geração de clock específico) e para sintetizar e gravar o projeto (`.sof`) na FPGA.
* **Visual Studio Code (versão atualizada):** Editor de código utilizado no desenvolvimento dos programas em linguagem C (`.c`), do cabeçalho (`.h`) e do driver em Assembly ARM (`.S`).
* **GCC (GNU Compiler Collection):** Compilador executado diretamente no Linux da DE1-SoC, utilizado para compilar os programas em C junto com o driver em Assembly ARM.
* **SSH e SCP:** Ferramentas utilizadas para acessar o terminal da placa pela rede e transferir os arquivos do driver e dos programas de teste do computador para a DE1-SoC.
* **GitHub:** Plataforma de hospedagem utilizada para versionar os arquivos de código-fonte (`.v`, `.c`, `.h`, `.S`), projetos (`.qpf`, `.qsf`) e arquivos de inicialização de memória (`.mif`, `.hex`).

## Metodologia

Os computadores possuem como um dos seus componentes principais o processador (CPU), que realiza o controle do fluxo de dados e o processamento de instruções do sistema. Contudo, muitas operações, como o cálculo contínuo das coordenadas de pixels, exigem muito poder de processamento, o que pode sobrecarregar a CPU e tornar o sistema lento. Assim, surge a ideia do coprocessador gráfico (GPU), uma unidade de processamento auxiliar que atua de forma independente e paralela à CPU, servindo para realizar as operações de geração de imagem de forma muito mais eficiente.

Para o desenvolvimento deste coprocessador, a arquitetura foi planejada com foco no paralelismo de hardware e na programabilidade. A CPU (ARM do HPS) não conhece os detalhes internos dos módulos gráficos: ela apenas envia instruções de 32 bits, definidas pela ISA do projeto. Cabe ao hardware buscar, decodificar e executar cada instrução, acionando o motor gráfico correspondente. A geração de imagens exige um sincronismo rigoroso com o monitor VGA. Por isso, o sistema opera com dois domínios de clock: um clock de 50 MHz para a fila de instruções, a unidade de controle, os motores gráficos e a escrita na memória de vídeo, e um clock de 25 MHz para a leitura contínua da memória de vídeo e o envio do sinal VGA.

Esta seção acompanha o caminho percorrido por uma instrução, do software até a imagem exibida no monitor, e está organizada na seguinte ordem: (1) **Instruções**, com o formato e o conjunto de instruções da ISA; (2) **Caminho da Instrução até a Fila**, que descreve o trajeto do programa em C até a entrada da fila, passando pelo driver em Assembly ARM e pela interface MMIO; (3) **Fila de Instruções**, que armazena os comandos enviados pelo processador; (4) **Unidade de Busca**, que retira as instruções da fila e as entrega ao registrador de instrução; (5) **Unidade de Controle**, que decodifica as instruções e coordena a execução; (6) **Motores gráficos**, que executam as operações de plano de fundo, sprites e polígonos e compõem a imagem final; (7) **Simulação**, com os testes e a validação do projeto; e (8) **Referências**, com as fontes consultadas.

### 4. Unidade de Busca

A unidade de busca é responsável por trazer, uma a uma, as instruções armazenadas na fila para dentro do coprocessador. O módulo `unidade_de_busca.v` implementa a estratégia de **busca passiva**: em vez de varrer uma memória de programa interna com um contador de programa (PC), ele consome as instruções que o processador ARM enfileirou por meio da interface MMIO. Dessa forma, o programa gráfico não fica gravado na FPGA, e o ARM pode alterá-lo a qualquer momento apenas enviando novas instruções à fila.

O módulo se comunica com a fila por três sinais. A entrada `dado_fifo` recebe a instrução de 32 bits que está na cabeça da fila, a entrada `fifo_vazia` informa se há instruções disponíveis e a saída `valid_leitura_fifo` pede à fila que descarte a instrução lida, avançando seu ponteiro de leitura. Como a fila trabalha no modo *first-word fall-through*, o dado já está visível em `dado_fifo` antes de qualquer pedido de leitura. A unidade pode, assim, capturar a instrução e só depois avisar a fila de que ela foi consumida. Com a unidade de controle, a comunicação é feita pelo sinal de entrada `valid_busca`, que autoriza uma nova busca, e pelo sinal de saída `instrucao_valida`, que avisa que a instrução capturada, disponível na saída `instrucao`, já pode ser usada. Todo o módulo opera no mesmo clock de 50 MHz da fila e da unidade de controle, o que dispensa circuitos de sincronização entre domínios de clock.

O funcionamento é descrito por uma Máquina de Estados Finitos (MEF) de três estados:

* **PARADO:** estado inicial, em que a unidade aguarda. Quando a unidade de controle autoriza a busca (`valid_busca` em nível alto) e a fila possui ao menos uma instrução (`fifo_vazia` em nível baixo), a máquina avança para o estado de leitura.
* **LEITURA:** a unidade emite o pedido de leitura para a fila (`valid_leitura_fifo`) e avança para o estado seguinte. Por ser um sinal registrado, o pedido chega à fila durante o estado VÁLIDO, quando a instrução ainda está na cabeça da fila.
* **VÁLIDO:** a unidade captura o conteúdo de `dado_fifo` no registrador `instrucao`, sinaliza `instrucao_valida` e retorna ao estado PARADO. Nesse mesmo ciclo, a fila atende ao pedido de leitura e avança para a próxima instrução.

Os sinais `valid_leitura_fifo` e `instrucao_valida` são pulsos de um único ciclo de clock: a cada ciclo, eles retornam ao nível baixo por padrão e só são elevados nos estados descritos. Esse formato de pulso é o mesmo adotado nos sinais `valid` entre a unidade de controle e os motores gráficos, o que mantém o protocolo de comunicação uniforme em todo o projeto. Uma busca completa leva quatro ciclos de clock, ou 80 ns a 50 MHz, um tempo desprezível diante da duração da execução das instruções pelos motores gráficos. A unidade de controle mantém `valid_busca` em nível alto apenas enquanto aguarda uma nova instrução (estado MONITORAR).

Ao receber o pulso `instrucao_valida`, o Registrador de Instrução (`registrador_de_instrucao.v`) armazena a instrução em `IR`, mantendo-a estável para que a unidade de controle decodifique o opcode e distribua os campos aos motores gráficos. Um sinal de reset zera a máquina de estados e todas as saídas, de modo que o coprocessador sempre inicie sem buscas pendentes.