#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "hardware/timer.h"
#include "hardware/clocks.h"
#include "ws2812.pio.h"
#include "hardware/i2c.h"
#include "inc/ssd1306.h"
#include "inc/font.h"

// Definições de hardware
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

// Intervalo de tempo
const int intervalo_ms = 400;
const int intervalo_micro = intervalo_ms * 1000;

// Variáveis globais
volatile int numero_atual = 0;
volatile uint32_t ultimo_estado_a = 0;
volatile uint32_t ultimo_estado_b = 0;

int estado_led_verde = 0;
int estado_led_azul = 0;
ssd1306_t ssd; // Inicializa a estrutura do display

bool cor = true;
char c = '0';  // Caracter '0'
int num = 10;

// Buffer para números na matriz
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

// Funções auxiliares
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
    // Atualiza o display para refletir o estado dos LEDs
    ssd1306_fill(&ssd, false);  // Limpa o display
    if (estado_led_verde == 0) {
        ssd1306_draw_string(&ssd, "LED Verde  OFF", 15, 10);  // Exibe mensagem
    } else {
        ssd1306_draw_string(&ssd, "LED Verde  ON", 15, 10);  // Exibe mensagem
    }
    
    if (estado_led_azul == 0) {
        ssd1306_draw_string(&ssd, "LED Azul   OFF", 15, 20);  // Exibe mensagem
    } else {
        ssd1306_draw_string(&ssd, "LED Azul   ON", 15, 20);  // Exibe mensagem
    }

    ssd1306_send_data(&ssd);  // Atualiza o display
    printf("Estado LED Verde: %s, LED Azul: %s\n", 
           estado_led_verde == 0 ? "OFF" : "ON", 
           estado_led_azul == 0 ? "OFF" : "ON");  // Envia para o Serial Monitor
}

void gpio_irq_handler(uint gpio, uint32_t events) {
    uint32_t current_time = to_us_since_boot(get_absolute_time());
    if (gpio == BUTTON_A && current_time - ultimo_estado_a > intervalo_micro) {
        ultimo_estado_a = current_time;
        estado_led_verde = !estado_led_verde;
        gpio_put(LED_RGB_G, estado_led_verde);  // Atualiza o estado do LED Verde
        display_estado_leds();  // Atualiza o display com o estado atual dos LEDs
    }
    if (gpio == BUTTON_B && current_time - ultimo_estado_b > intervalo_micro) {
        ultimo_estado_b = current_time;
        estado_led_azul = !estado_led_azul;
        gpio_put(LED_RGB_B, estado_led_azul);  // Atualiza o estado do LED Azul
        display_estado_leds();  // Atualiza o display com o estado atual dos LEDs
    }
}

int main() {
    // I2C Initialisation. Using it at 400Khz.
    i2c_init(I2C_PORT, 400 * 1000);
    
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C); // Set the GPIO pin function to I2C
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C); // Set the GPIO pin function to I2C
    gpio_pull_up(I2C_SDA); // Pull up the data line
    gpio_pull_up(I2C_SCL); // Pull up the clock line

    ssd1306_init(&ssd, WIDTH, HEIGHT, false, endereco, I2C_PORT); // Inicializa o display
    ssd1306_config(&ssd); // Configura o display
    ssd1306_send_data(&ssd); // Envia os dados para o display
    ssd1306_fill(&ssd, false);
    ssd1306_send_data(&ssd);
    ssd1306_fill(&ssd, !cor); // Limpa o display
    ssd1306_rect(&ssd, 3, 3, 122, 58, cor, !cor); // Desenha um retângulo    

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

        if (stdio_usb_connected()) { // Certifica-se de que o USB está conectado
            if (scanf(" %c", &c) == 1) { // Lê caractere da entrada padrão
                ssd1306_draw_char(&ssd, c, 15, 48); // Desenha uma string      
                ssd1306_send_data(&ssd); // Atualiza o display
                if (c >= '0' && c <= '9') { // Verifica se é um número
                    num = c - '0'; // Converte char para int
                    printf("Recebido: '%c' -> Número: %d\n", c, num);
                    display_numeros(num); // Atualiza a matriz de LEDs
                }
            }
        }
        sleep_ms(40); // Pausa para evitar uso excessivo da CPU
    }
    return 0;
}
