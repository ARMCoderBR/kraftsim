================================================================================                                                                                

$$\   $$\                     $$$$$$\    $$\      $$$$$$\  $$$$$$\ $$\      $$\ 
$$ | $$  |                   $$  __$$\   $$ |    $$  __$$\ \_$$  _|$$$\    $$$ |
$$ |$$  /  $$$$$$\  $$$$$$\  $$ /  \__|$$$$$$\   $$ /  \__|  $$ |  $$$$\  $$$$ |
$$$$$  /  $$  __$$\ \____$$\ $$$$\     \_$$  _|  \$$$$$$\    $$ |  $$\$$\$$ $$ |
$$  $$<   $$ |  \__|$$$$$$$ |$$  _|      $$ |     \____$$\   $$ |  $$ \$$$  $$ |
$$ |\$$\  $$ |     $$  __$$ |$$ |        $$ |$$\ $$\   $$ |  $$ |  $$ |\$  /$$ |
$$ | \$$\ $$ |     \$$$$$$$ |$$ |        \$$$$  |\$$$$$$  |$$$$$$\ $$ | \_/ $$ |
\__|  \__|\__|      \_______|\__|         \____/  \______/ \______|\__|     \__|

================================================================================                                                                                
                                                                                
KRAFTSIM 1.0
============

    KraftSIM é um aplicativo para Linux (testado em Ubuntu e Mint) que simula um
computador Z80. Este computador foi primeiro criado como uma máquina Z80 real, 
chamada Kraft 80.


OK, MAS O QUE É O KRAFT 80?
===========================

    Kraft 80 é um computador conhecido como "homebrew", ou seja, criado com um 
desenho autoral, sem se basear em nenhum projeto já existente na indústria. Já
existem várias implementações modernas (em hardware real e também em simulação
de software) de clássicos como o MSX, o ZX Spectrum, etc, isso dentro da linha
Z80, e também de outras arquiteturas como Commodore 64, Apple II, etc.


MAS PARA QUÊ FAZER UM COMPUTADOR QUE NINGUÉM TEM, NEM CONHECE?
==============================================================

    Diversão e aprendizado, claro! Fazer um novo computador é muito mais
didático (e divertido) do que reproduzir um computador que já existe. Mas isso
tem um custo: esse novo computador vai ter quase nada de software já pronto,
se compararmos com um computador padrão da indústria.


CARACTERÍSTICAS PRINCIPAIS DO KRAFT 80 (MÁQUINA REAL)
=====================================================

    - Processador Z80 de 4 MHz

    - Memória total de 64 kbytes em dois modos de mapeamento selecionáveis:
        - Modo 0: 16 kbytes ROM + 48 kbytes RAM
        - Modo 1: 8  kbytes ROM + 56 kbytes RAM

    - Display LCD alfanumérico 2 linhas x 16 colunas

    - 8 Pushbuttons que podem ser lidos por Software

    - 1 Pushbutton de Reset

    - 8 LEDs que podem ser acionados por Software

    - Módulo auxiliar FPGA com periféricos avançados:
        - Saída de vídeo HDMI padrão VGA
            - Texto 48 linhas x 80 colunas, monocromático
            - Gráfico 320 x 240 pontos, 16 cores
        - Porta para teclado PS/2
        - Porta serial assíncrona
        - Interrupção periódica (timer)
        - Sintetizador de som compatível com GI AY3-891X (vulgo "PSG do MSX")


SOFTWARE DO KRAFT 80
====================

    Existem dois tipos de programas para ele: as ROM e os aplicativos que rodam
a partir da RAM.

    Todos são fornecidos como arquivos no formato Intel Hex e possuem a extensão
IHX.

    São fornecidos os seguintes programas em ROM:

    - Demo:

        - Cálculo do PI, escrito em ASM.      ...     rom1-picalc.ihx
          Utiliza o método BBP e executa os
          cálculos com número arbitrário de
          casas decimais (limitado pela 
          memória e velocidade da CPU).

        - Cálculo do PI, escrito em C.        ...     rom1-pibbp-c.ihx
          Utiliza o método BBP e executa os
          cálculos com precisão limitada
          pela biblioteca numérica do
          ambiente C. Menos preciso que a
          versão em ASM.

    - Monitores:

        - Amon2, um monitor de memória        ...     amon2.ihx
          simples que usa os pushbuttons e
          o display LCD para inserir e
          executar pequenos programas. É
          baseado no Amon da WR Kits
          (créditos no final).

        - Kraftmon, o monitor "oficial" de    ...     kraftbios-v2.ihx
          memória, carga e execução de
          programas. Também fornece o BIOS.

    - Linguagem BASIC:

        - Basic 4.7b Microsoft (versão        ...     bas32k-standalone.ihx
          solitária) - funciona como ROM
          principal, sem monitor nem BIOS.

        - Basic 4.7b Microsoft (versão        ...     bas32k.ihx
          auxiliar do Kraftmon) - funciona
          como segunda ROM quando tem o
          Kraftmon instalado na primeira ROM.


    Os aplicativos que rodam em RAM dependem da ROM do Kraftmon que precisa
estar instalada no soquete da primeira ROM.

        - Chiptunes 1 e 2: demo de som        ...     chiptunes.ihx
          (músicas no PSG).                           chiptunes2.ihx

        - Relógios 1 e 2: marcam as horas,    ...     clock.ihx
          o primeiro apenas no LCD, o segundo         clock2.ihx
          no LCD e também no monitor VGA.
          Use os pushbuttons para acertar as
          horas, minutos e segundos.

        - Demos dos LEDs 1 e 2. Piscam os     ...     blinker.ihx
          LEDs da placa.                              kitt.ihx

        - Cálculo do PI, escrito em ASM.      ...     picalc.ihx
          Utiliza o método BBP e executa os
          cálculos com número arbitrário de
          casas decimais (limitado pela 
          memória e velocidade da CPU).

        - Cálculo do PI, escrito em C.        ...     pibbp.ihx
          Utiliza o método BBP e executa os
          cálculos com precisão limitada
          pela biblioteca numérica do
          ambiente C. Menos preciso que a
          versão em ASM.


COMPARAÇÃO DO KRAFTSIM COM O KRAFT 80
=====================================

    A simulação no PC ficou bastante fiel, onde todas as ROMs e programas do
Kraft 80 rodam de forma transparente no KraftSIM.

    






