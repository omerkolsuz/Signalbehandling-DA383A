#include "esp_task_wdt.h"
#include "driver/dac.h"
#include "driver/adc.h"
#include "driver/gpio.h"
#include "soc/dac_channel.h"
#include "esp32/rom/ets_sys.h"
#include "esp_timer.h"

//The sampling freq to be used
int freq = 10000;
//filtrets längd
#define M 4

#define N 9

//initialisera arrays
static float xbuff[M+1] = {0};
//koefficienterna
//static const float b[M+1] = {0.2500,0.2500,0.2500,0.2500};
static const float b[M+1] = {0.1, 0.1, 0.1, 0.1, 0.1};
//Callback for how often you sample.

static float ybuff[N+1] = {0};
static const float a[N+1] = {0.9, 0.9, 0.9, 0.9, 0.9, 0.9, 0.9, 0.9, 0.9, 0.9};


static void periodic_timer_callback(void *arg)
{
    gpio_set_level(GPIO_NUM_14, 1);

    float val = adc1_get_raw(ADC1_CHANNEL_4);
    val = val / 16;
   
    //Uppdatera xbuff
    for (int i = M; i > 0; i--) {
        xbuff[i] = xbuff[i-1];
    }
    xbuff[0] = val;
    //beräkna filtervärdet
    float filtered_val = 0.0;
    for (int i = 0; i <= M; i++) {
        filtered_val += xbuff[i] * b[i];
    }


  float filtered_sum = 0;
    for (int j = 0; j <= N; j++) {
        filtered_sum += a[j+1] * ybuff[j+1];
    }
    filtered_sum += val;

    // make room for new output value
    for (int j = N; j > 0; j--) {
        ybuff[j] = ybuff[j-1];
    }

    // store new output value
    ybuff[1] = filtered_sum;

    val = (uint32_t)ybuff[1];
   
    dac_output_voltage(DAC_CHANNEL_1, (int)filtered_val);
    gpio_set_level(GPIO_NUM_14, 0);
}

void app_main()
{
    //init adc and dac
    adc1_config_width(ADC_WIDTH_BIT_12);
    adc1_config_channel_atten(ADC1_CHANNEL_4, ADC_ATTEN_DB_11);
    gpio_pullup_en(GPIO_NUM_32);
    dac_output_enable(DAC_CHANNEL_1);

    gpio_config_t config;
    config.pin_bit_mask = 1 << GPIO_NUM_14;
    config.mode = GPIO_MODE_DEF_OUTPUT;
    config.pull_down_en = GPIO_PULLDOWN_DISABLE;
    config.pull_up_en = GPIO_PULLUP_ENABLE;
    ESP_ERROR_CHECK(gpio_config(&config));

    const esp_timer_create_args_t periodic_timer_args = {
        .callback = &periodic_timer_callback,
        //name is optional, but may help identify the timer when debugging
        .name = "periodic"};

    esp_timer_handle_t periodic_timer;
    ESP_ERROR_CHECK(esp_timer_create(&periodic_timer_args, &periodic_timer));
    ESP_ERROR_CHECK(esp_timer_start_periodic(periodic_timer, 1000000/freq));

   }
