# C-GBoy-PI

O C-GBoy-PI é um porte do meu emulador de Game Boy, desenvolvido em C, para um console portátil utilizando um Raspberry Pi 3.

A ideia do projeto foi adaptar o emulador original para funcionar diretamente no Raspberry Pi, utilizando um display LCD e botões físicos no lugar do teclado, permitindo executar jogos sem precisar de um monitor externo.

## Projeto original

Este repositório é focado principalmente nas adaptações feitas para o Raspberry Pi.

O desenvolvimento do emulador em si, incluindo o funcionamento da CPU, PPU, gerenciamento de memória, controladores MBC e outros componentes, está explicado com mais detalhes no repositório original:

**[C-GBoy (Projeto Original)](https://github.com/ribeiro-boll/C-GBoy)**

## Hardware utilizado

O projeto utiliza os seguintes componentes:

- Raspberry Pi 3
- Display LCD ST7789 de 1,3 polegadas, com resolução de 240x240 pixels
- Botões físicos conectados aos GPIOs do Raspberry Pi

O sistema operacional utilizado é o Raspberry Pi OS Lite, já que o projeto não necessita de uma interface gráfica para funcionar.

## Display LCD

Uma das principais adaptações foi substituir a saída gráfica original pela comunicação direta com o display ST7789, utilizando a interface SPI do Raspberry Pi.

Os frames gerados pelo emulador são convertidos para o formato RGB565 e enviados ao display através do SPI.

A comunicação utiliza os seguintes GPIOs:

| Função | GPIO (BCM) |
|---|---|
| MOSI | 10 |
| SCLK | 11 |
| DC | 25 |
| RESET | 27 |
| Backlight | 18 |

Com isso, o emulador consegue renderizar os jogos diretamente no LCD, sem depender de SDL para a exibição das imagens.

## Controles físicos

Os controles também foram adaptados para utilizar os GPIOs do Raspberry Pi.

Cada botão está conectado a um GPIO, utilizando os resistores de pull-up internos e interrupções para identificar quando ele é pressionado ou solto.

O mapeamento utilizado é:

| Botão | GPIO (BCM) |
|---|---|
| Direita | 26 |
| Esquerda | 13 |
| Cima | 16 |
| Baixo | 12 |
| A | 4 |
| B | 17 |
| Start | 6 |
| Select | 5 |
| Reset | 22 |

Dessa forma, os botões físicos substituem os controles utilizados anteriormente no computador.

## ROM Selector

Além das adaptações no emulador, desenvolvi um programa separado chamado `ROM_selector`, responsável por mostrar os jogos disponíveis no console.

O programa procura as ROMs dentro de uma pasta, organiza os arquivos encontrados e exibe seus nomes no display LCD.

Para desenhar os textos do menu, são utilizadas fontes de 8x8 pixels convertidas para RGB565, incluindo versões com as cores invertidas para destacar o jogo selecionado.

### Controles do ROM Selector

A navegação pelo menu é feita utilizando os próprios botões físicos do console.

| Botão | GPIO (BCM) | Função |
|---|---|---|
| Cima | 16 | Selecionar a ROM anterior |
| Baixo | 12 | Selecionar a próxima ROM |
| A | 4 | Iniciar a ROM selecionada |

Ao selecionar um jogo, o ROM Selector executa o C-GBoy, passando o caminho da ROM como argumento.

O seletor e o emulador são programas separados. Quando o jogo é encerrado, o ROM Selector volta a ser executado, permitindo escolher outro jogo sem precisar reiniciar o Raspberry Pi.

## Inicialização do sistema

O projeto também foi configurado para iniciar automaticamente com o Raspberry Pi através do systemd.

Assim, ao ligar o dispositivo, o ROM Selector é executado sem a necessidade de realizar login ou iniciar o programa manualmente pelo terminal.

Também foram implementadas rotinas de encerramento para desligar o display e liberar os recursos utilizados pelo programa.

## Considerações finais

O objetivo deste porte foi transformar o C-GBoy em um projeto que pudesse ser utilizado em um dispositivo físico, com uma tela e controles próprios.

Além das mudanças na renderização e na entrada de dados, foi necessário lidar com a comunicação SPI, gerenciamento dos GPIOs e execução dos programas no Linux.

O resultado é uma versão do emulador adaptada para funcionar em um console portátil baseado no Raspberry Pi.
