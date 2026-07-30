#include "drivers/microphone/microphone.h"

#include "hardware/adc.h"
#include "pico/stdlib.h"
#include "microphone.h"
#include "board.h"

static const uint MIC_ADC_INPUT = 2;
static const uint MIC_GPIO_PIN = 28;

static const float ADC_SAMPLE_RATE = 44100.0f;
static const float ADC_CLOCK_HZ = 48000000.0f;

void mic_init() {
    adc_init();

    adc_gpio_init(MIC_GPIO_PIN);

    adc_select_input(MIC_ADC_INPUT);

    // slows down 48MHz to 44.1kHz
    float clkdiv = (ADC_CLOCK_HZ / ADC_SAMPLE_RATE) - 1.0f;
    adc_set_clkdiv(clkdiv);

    adc_fifo_setup(
        true,
        false,
        1,
        false,
        false
    );
}

void mic_read(uint16_t* buffer, uint32_t sample_count) {
    adc_run(true);

    for (uint32_t i = 0; i < sample_count; i++) {
        buffer[i] = adc_fifo_get_blocking();
    }
}