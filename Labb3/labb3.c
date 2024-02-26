#include <array>
#include <cstdint>
#include <queue>

#include "esp_task_wdt.h"
#include "driver/dac.h"
#include "driver/adc.h"
#include "driver/gpio.h"
#include "soc/dac_channel.h"
#include "esp32/rom/ets_sys.h"

extern "C" auto app_main() -> void;

constexpr auto M = 3;
static std::array<float, M + 1> b{0.2500, 0.2500, 0.2500, 0.2500};
static std::array<float, M + 1> x{};

constexpr std::uint32_t FREQUENCY = 10'000;
constexpr std::uint64_t US_ONE_S  = 1'000'000;
constexpr auto PIN                = GPIO_NUM_13;

static auto timer_callback(void arg) -> void {
    static auto k = 0;
    x[0] = float(adc1_get_raw(ADC1_CHANNEL_0) / 16);
    gpio_set_level(PIN, 1U);
    x[k++] = x[0];
    if (k == M + 1) k = 0;

    auto sum = 0.0f;
    for (auto i = M; i >= 0; i--) {
        sum += b[i] x[i];
    }

    gpio_set_level(PIN, 0U);
    dac_output_voltage(DAC_CHANNEL_1, uint8_t(sum));
}

auto app_main() -> void {
    adc_power_acquire();
    adc_gpio_init(adc_unit_t(ADC_UNIT_1), adc_channel_t(ADC1_CHANNEL_0));
    adc1_config_width(ADC_WIDTH_BIT_12);
    adc1_config_channel_atten(ADC1_CHANNEL_0, ADC_ATTEN_DB_11);
    gpio_pullup_en(GPIO_NUM_36);
    dac_output_enable(DAC_CHANNEL_1);

    gpio_config_t config;
    config.pin_bit_mask  = (uint64_t) 1 << PIN;
    config.mode          = gpio_mode_t(GPIO_MODE_DEF_OUTPUT);
    config.pull_down_en  = GPIO_PULLDOWN_DISABLE;
    config.pull_up_en    = GPIO_PULLUP_ENABLE;
    ESP_ERROR_CHECK(gpio_config(&config));

    esp_timer_create_args_t periodConfig{};
    periodConfig.callback = timer_callback;
    periodConfig.name     = "sample_callback";

    esp_timer_handle_t periodicHandle;
    ESP_ERROR_CHECK(esp_timer_create(&periodConfig, &periodicHandle));
    ESP_ERROR_CHECK(esp_timer_start_periodic(periodicHandle, US_ONE_S / FREQUENCY));
}
