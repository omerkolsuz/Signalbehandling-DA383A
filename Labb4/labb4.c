#include <stdio.h>
#include <esp_task_wdt.h>
#include <driver/dac.h>
#include <soc/soc.h>
#include <soc/sens_reg.h>
#include <soc/syscon_reg.h>
#include <driver/adc.h>
#include <esp_timer.h>
#include <driver/timer.h>

//Length of the filter
#define M 4
#define N 4

//The sampling freq to be used 
int freq = 10000;

static float xbuff[M+1]={0};
static float b[M+1] = {
                            0.01488697472657, 
                            -0.02695899404537,  
                            0.03705935223574, 
                            -0.02695899404537,
                            0.01488697472657    };
static float ybuff[N+1]={0};
static float a[N+1]={
                            -1,   
                            3.338693232847,    
                            -4.401916486793,   
                            2.691625646031,
                            -0.6428936122854    };


//Callback for how often you sample.
static void periodic_timer_callback(void *arg)
{
    gpio_set_level(GPIO_NUM_27,1);

    float val = adc1_get_raw(ADC1_CHANNEL_6);
    val = val / 16;

    

    ///////// lab 2 ///////// 
    //static uint16_t buffer[10000]={0};
    //static uint32_t k=0;
    //buffer[k]=val;
    //k++;
    //if (k==10000) k=0;
    //float outvalue = buffer[k]+val;
    //////////////////////////// 

    /////////////-FIR-/////////////// 
    static uint32_t k=0;
    float outvalue; 
    float sum = 0;


    // make room for new input value
    for (k = M; k > 0; k--) {
        xbuff[k] = xbuff[k-1];
    }
    xbuff[0] = (float)val;

    // form sum
    for (k = 0; k <= M; k++) {
        sum += xbuff[k] * b[k];
    }

    // convert and store in outvalue
    // outvalue = (float)sum;
    //////////////////////////// 


    /////////////-IIR-/////////////// 
    //static float a[N+1]={0.9};

    // form filtered output
    float filtered_sum = 0;
    for (k = 0; k <= N; k++) {
        filtered_sum += a[k+1] * ybuff[k+1];
    }
    filtered_sum += sum;

    // make room for new output value
    for (k = N; k > 0; k--) {
        ybuff[k] = ybuff[k-1];
    }

    // store new output value
    ybuff[1] = filtered_sum;

    outvalue = (uint32_t)ybuff[1];
    ///////////////////////////


    //original, but with val instead of outvalue.
    dac_output_voltage(DAC_CHANNEL_1, (int)outvalue);

    gpio_set_level(GPIO_NUM_27,0);

}

void app_main()
{

    adc1_config_width(ADC_WIDTH_BIT_12);
    adc1_config_channel_atten(ADC1_CHANNEL_6, ADC_ATTEN_DB_11);
    gpio_pullup_en(32);
    dac_output_enable(DAC_CHANNEL_1);

    gpio_config_t io_conf;
    io_conf.intr_type = GPIO_INTR_DISABLE;
    io_conf.mode = GPIO_MODE_INPUT;
    io_conf.pin_bit_mask = (1 << GPIO_NUM_27);
    io_conf.pull_down_en = 0;
    io_conf.pull_up_en = 0;
    gpio_config(&io_conf);

    const esp_timer_create_args_t periodic_timer_args = {
        .callback = &periodic_timer_callback,
        //name is optional, but may help identify the timer when debugging 
        .name = "periodic"};

    esp_timer_handle_t periodic_timer;
    ESP_ERROR_CHECK(esp_timer_create(&periodic_timer_args, &periodic_timer));
    ESP_ERROR_CHECK(esp_timer_start_periodic(periodic_timer, 1000000/freq));
}
