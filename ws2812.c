#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "hardware/timer.h"
#include "hardware/clocks.h"
#include "hardware/uart.h"
#include "ws2812.pio.h"
#include "hardware/i2c.h"
#include "inc/ssd1306.h"
#include "inc/font.h" 

// Definições de hardware existentes
#define LED_RGB_G 11
#define LED_RGB_B 12
#define LED_RGB_R 13
#define BUTTON_A 5
#define BUTTON_B 6
#define WS2812_PIN 7
#define NUM_PIXELS 25
#define IS_RGBW false
#define I2C_PORT i2c1
#define I2C_SDA 14
#define I2C_SCL 15
#define endereco 0x3C

// Definições UART
#define UART_ID uart0
#define BAUD_RATE 115200
#define UART_TX_PIN 0
#define UART_RX_PIN 1

// Intervalo de tempo
const int intervalo_ms = 400;
const int intervalo_micro = intervalo_ms * 1000;

// Variáveis globais
volatile int numero_atual = 0;
volatile uint32_t ultimo_estado_a = 0;
volatile uint32_t ultimo_estado_b = 0;

int estado_led_verde = 0;
int estado_led_azul = 0;
ssd1306_t ssd;

bool cor = true;
char c = '\0';  // Nenhum caractere exibido inicialmente
int num = 10;

// Buffer para números na matriz (mantido como estava)
bool numeros[11][NUM_PIXELS] = {
    { // Zero
        0, 1, 1, 1, 0,
        0, 1, 0, 1, 0,
        0, 1, 0, 1, 0,
        0, 1, 0, 1, 0,
        0, 1, 1, 1, 0
    },
    { // Um
        0, 1, 0, 0, 0,
        0, 0, 0, 1, 0,
        0, 1, 0, 0, 0,
        0, 0, 0, 1, 0,
        0, 1, 0, 0, 0
    },
    { // Dois
        0, 1, 1, 1, 0,
        0, 1, 0, 0, 0,
        0, 1, 1, 1, 0,
        0, 0, 0, 1, 0,
        0, 1, 1, 1, 0
    },
    { // Três
        0, 1, 1, 1, 0,
        0, 0, 0, 1, 0,
        0, 1, 1, 0, 0,
        0, 0, 0, 1, 0,
        0, 1, 1, 1, 0
    },
    { // Quatro
        0, 1, 0, 0, 0,
        0, 0, 0, 1, 0,
        0, 1, 1, 1, 0,
        0, 1, 0, 1, 0,
        0, 1, 0, 1, 0
    },
    { // Cinco
        0, 1, 1, 1, 0,
        0, 0, 0, 1, 0,
        0, 1, 1, 1, 0,
        0, 1, 0, 0, 0,
        0, 1, 1, 1, 0
    },
    { // Seis
        0, 1, 1, 1, 0,
        0, 1, 0, 1, 0,
        0, 1, 1, 1, 0,
        0, 1, 0, 0, 0,
        0, 1, 1, 1, 0
    },
    { // Sete
        0, 1, 0, 0, 0,
        0, 0, 0, 1, 0,
        0, 1, 0, 0, 0,
        0, 0, 0, 1, 0,
        0, 1, 1, 1, 0
    },
    { // Oito
        0, 1, 1, 1, 0,
        0, 1, 0, 1, 0,
        0, 1, 1, 1, 0,
        0, 1, 0, 1, 0,
        0, 1, 1, 1, 0
    },
    { // Nove
        0, 1, 1, 1, 0,
        0, 0, 0, 1, 0,
        0, 1, 1, 1, 0,
        0, 1, 0, 1, 0,
        0, 1, 1, 1, 0
    },
    { // Dez (apagado)
        0, 0, 0, 0, 0,
        0, 0, 0, 0, 0,
        0, 0, 0, 0, 0,
        0, 0, 0, 0, 0,
        0, 0, 0, 0, 0
    }
};

// Funções auxiliares existentes permanecem iguais
static inline uint32_t urgb_u32(uint8_t r, uint8_t g, uint8_t b) {
    return ((uint32_t)(r) << 8) | ((uint32_t)(g) << 16) | (uint32_t)(b);
}

static inline void put_pixel(uint32_t pixel_grb) {
    pio_sm_put_blocking(pio0, 0, pixel_grb << 8u);
}

void display_numeros(int numero) {
    uint32_t color = urgb_u32(100, 0, 0);
    for (int i = 0; i < NUM_PIXELS; i++) {
        put_pixel(numeros[numero][i] ? color : 0);
    }
}

void display_estado_leds() {
    ssd1306_fill(&ssd, false);

    // Estado do LED Verde
    if (estado_led_verde == 0) {
        ssd1306_draw_string(&ssd, "LED V   OFF", 15, 10);
    } else {
        ssd1306_draw_string(&ssd, "LED V   ON", 15, 10);
    }

    // Estado do LED Azul
    if (estado_led_azul == 0) {
        ssd1306_draw_string(&ssd, "LED A   OFF", 15, 20);
    } else {
        ssd1306_draw_string(&ssd, "LED A   ON", 15, 20);
    }

    // Só exibe o caractere se um valor já foi digitado
    if (c != '0') {
        ssd1306_draw_char(&ssd, c, 60, 40);
    }

    // Atualiza o display
    ssd1306_send_data(&ssd);

    // Envia estado dos LEDs pela USB e UART
    printf("Estado LED Verde: %s, LED Azul: %s\n", 
           estado_led_verde == 0 ? "OFF" : "ON", 
           estado_led_azul == 0 ? "OFF" : "ON");

    uart_puts(UART_ID, "Estado LED Verde: ");
    uart_puts(UART_ID, estado_led_verde == 0 ? "OFF" : "ON");
    uart_puts(UART_ID, ", LED Azul: ");
    uart_puts(UART_ID, estado_led_azul == 0 ? "OFF\n" : "ON\n");
}



void gpio_irq_handler(uint gpio, uint32_t events) {
    uint32_t current_time = to_us_since_boot(get_absolute_time());
    if (gpio == BUTTON_A && current_time - ultimo_estado_a > intervalo_micro) {
        ultimo_estado_a = current_time;
        estado_led_verde = !estado_led_verde;
        gpio_put(LED_RGB_G, estado_led_verde);
        display_estado_leds();
    }
    if (gpio == BUTTON_B && current_time - ultimo_estado_b > intervalo_micro) {
        ultimo_estado_b = current_time;
        estado_led_azul = !estado_led_azul;
        gpio_put(LED_RGB_B, estado_led_azul);
        display_estado_leds();
    }
}

// Função para processar caractere recebido
void process_received_char(char received_char) {
    // Armazena o caractere recebido
    c = received_char;

    // Redesenha apenas a parte do caractere, sem apagar os estados dos LEDs
    ssd1306_draw_char(&ssd, c, 60, 40);
    ssd1306_send_data(&ssd);

    // Se for um número, converte e exibe na matriz de LEDs
    if (received_char >= '0' && received_char <= '9') {
        num = c - '0';
        display_numeros(num);
    }

    // Envia confirmação para USB e UART
    printf("Recebido: '%c'\n", c);
    char uart_buffer[50];
    snprintf(uart_buffer, sizeof(uart_buffer), "Recebido: '%c'\n", c);
    uart_puts(UART_ID, uart_buffer);
}



int main() {
    // Inicialização do I2C (mantida como estava)
    i2c_init(I2C_PORT, 400 * 1000);
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA);
    gpio_pull_up(I2C_SCL);

    // Inicialização do UART
    uart_init(UART_ID, BAUD_RATE);
    gpio_set_function(UART_TX_PIN, GPIO_FUNC_UART);
    gpio_set_function(UART_RX_PIN, GPIO_FUNC_UART);

    // Resto das inicializações (mantidas como estavam)
    ssd1306_init(&ssd, WIDTH, HEIGHT, false, endereco, I2C_PORT);
    ssd1306_config(&ssd);
    ssd1306_send_data(&ssd);
    ssd1306_fill(&ssd, false);
    ssd1306_send_data(&ssd);
    ssd1306_fill(&ssd, !cor);
    ssd1306_rect(&ssd, 3, 3, 122, 58, cor, !cor);

    stdio_init_all();
    gpio_init(LED_RGB_R); gpio_set_dir(LED_RGB_R, GPIO_OUT);
    gpio_init(LED_RGB_G); gpio_set_dir(LED_RGB_G, GPIO_OUT);
    gpio_init(LED_RGB_B); gpio_set_dir(LED_RGB_B, GPIO_OUT);
    gpio_init(BUTTON_A); gpio_set_dir(BUTTON_A, GPIO_IN); gpio_pull_up(BUTTON_A);
    gpio_init(BUTTON_B); gpio_set_dir(BUTTON_B, GPIO_IN); gpio_pull_up(BUTTON_B);
    gpio_set_irq_enabled_with_callback(BUTTON_A, GPIO_IRQ_EDGE_FALL, true, &gpio_irq_handler);
    gpio_set_irq_enabled_with_callback(BUTTON_B, GPIO_IRQ_EDGE_FALL, true, &gpio_irq_handler);
    
    PIO pio = pio0;
    int sm = 0;
    uint offset = pio_add_program(pio, &ws2812_program);
    ws2812_program_init(pio, sm, offset, WS2812_PIN, 800000, IS_RGBW);
    
    display_estado_leds();

    while (1) {
        cor = !cor;
        display_numeros(num);

        // Verifica entrada USB
        if (stdio_usb_connected()) {
            if (scanf(" %c", &c) == 1) {
                process_received_char(c);
            }
        }

        // Verifica entrada UART
        if (uart_is_readable(UART_ID)) {
            char received_char = uart_getc(UART_ID);
            process_received_char(received_char);
        }

        sleep_ms(40);
    }
    return 0;
}