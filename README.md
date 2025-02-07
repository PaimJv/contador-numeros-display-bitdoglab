# Projeto: Interface de Comunicação Serial e Controle de Periféricos com RP2040

## Descrição
Este projeto tem como objetivo explorar as funcionalidades da placa BitDogLab, consolidando os conceitos de comunicação serial, manipulação de LEDs e botões com interrupções. O sistema embarcado desenvolvido permite a interação do usuário por meio do **Serial Monitor**, **botões físicos** e **display OLED SSD1306**, combinando hardware e software para um controle dinâmico e interativo.

## Componentes Utilizados
- **Matriz 5x5 de LEDs WS2812** (conectada à GPIO 7)
- **LED RGB** (pinos conectados às GPIOs 11, 12 e 13)
- **Botão A** (conectado à GPIO 5)
- **Botão B** (conectado à GPIO 6)
- **Display OLED SSD1306** via I2C (GPIO 14 e GPIO 15)
- **Comunicação Serial UART** via USB

## Funcionalidades Implementadas
1. **Adição de caracteres minúsculos para exibição no display OLED**
   - Modificação da Biblioteca **font.h**.
   - Modificação da função **ssd1306_draw_char()**, do **ssd1306.c**.

2. **Entrada de Caracteres via Serial**
   - O usuário pode enviar caracteres pelo Serial Monitor do VS Code.
   - O caractere digitado é exibido no display OLED.
   - Se um número (0-9) for enviado, o símbolo correspondente é exibido na matriz WS2812.

3. **Interação com o Botão A**
   - Pressionar o botão alterna o estado do LED Verde.
   - O estado é atualizado no display OLED e uma mensagem é enviada ao Serial Monitor.

4. **Interação com o Botão B**
   - Pressionar o botão alterna o estado do LED Azul.
   - O estado é atualizado no display OLED e uma mensagem é enviada ao Serial Monitor.

## Como Executar o Projeto
1. **Compilar e Carregar o Código**
   - Utilize o VS Code com a extensão do **Pico SDK**.
   - Compile e carregue o arquivo **ws2812.c** na placa BitDogLab.

2. **Interagir com o Sistema**
   - Abra o **Serial Monitor** e envie caracteres para exibição.
   - Pressione os botões para alterar os LEDs e visualizar as mensagens.

## Estrutura do Projeto
- **ws2812.c**              # Arquivo principal do projeto
- **inc/ssd1306.c**         # Controle do display OLED
- **inc/font.h**            # Biblioteca de fontes (modificada)
- **inc/ssd1306.h**         # Header do display OLED
- **ws2812.pio.h**          # Controle da matriz de LEDs WS2812
- **README.md**             # Documentação do projeto

## Dependências
- **Pico SDK**
- **Bibliotecas para controle do OLED SSD1306**
- **Bibliotecas para controle dos LEDs WS2812**