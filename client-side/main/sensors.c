#include"sensors.h"


int step=0;

const uint32_t pin_num[1] =
{
        SERVO,
};
// duties table, real_duty = duties     [x]/PERIOD
uint32_t duties[1] =
{
        1000
};
// phase table, delay = (phase[x]/360)*PERIOD
float phase[1] =
{
        0,
};



void adc(void){

    adc_config_t adc_config;
    adc_config.mode = ADC_READ_TOUT_MODE;
    adc_config.clk_div = 8;
    adc_init(&adc_config);
printDEBUG(DSYS,"ADC\n");

}


void LDR_init(void){

    gpio_config_t io_conf_LDR;
    io_conf_LDR.intr_type = GPIO_INTR_DISABLE;
    io_conf_LDR.mode = GPIO_MODE_INPUT;
    io_conf_LDR.pin_bit_mask =  (1 << LDR);
    io_conf_LDR.pull_down_en = 0;
    io_conf_LDR.pull_up_en = 0;
    gpio_config(&io_conf_LDR);
    printDEBUG(DSYS,"LDR\n");
}

int LDR_output(void){
uint8_t brightness=0;
adc_read(&brightness);
printDEBUG(DSYS,"br %d\n",brightness);
if (brightness>=0 && brightness<=80){
	return 1; //mracno
}
else if (brightness>80 && brightness<150){
	return 1;// srednje osvijetljenoi
}
else{	
	return 0;//osvijetljeno
}
}

void servo_init(void){

    pwm_init(PWM_PERIOD, duties, 1, pin_num);
    pwm_set_phases(phase);
    pwm_start();
    pwm_stop(0);
}

void buzzer_init(void){
int PIN_NUM=BUZZER_pin;
pwm_init(BUZZER_period, duties,1,PIN_NUM);
pwm_set_phases(phase);
pwm_start();

}
/*za prestanak zvuka pozvati funkciju pwm_stop(1);*/
void servo(int input){
    if(input == 0){
        step=750;
        pwm_set_duty(0,step);
        pwm_start();
    }
    if(input == 1 ){
        step=1500;
        pwm_set_duty(0,step);
        pwm_start();
    }
}

void hcsr_init(void){

	gpio_config_t io_conf_TRIGGER;
        io_conf_TRIGGER.intr_type = GPIO_INTR_DISABLE;
        io_conf_TRIGGER.mode = GPIO_MODE_OUTPUT;
        io_conf_TRIGGER.pin_bit_mask = (1 << TRIGGER_PIN);
        io_conf_TRIGGER.pull_down_en = 0;
        io_conf_TRIGGER.pull_up_en = 0;
        gpio_config(&io_conf_TRIGGER);

        gpio_config_t io_conf_ECHO;
        io_conf_ECHO.intr_type = GPIO_INTR_DISABLE;
        io_conf_ECHO.mode = GPIO_MODE_INPUT;
        io_conf_ECHO.pin_bit_mask = (1 << ECHO_PIN);
        io_conf_ECHO.pull_down_en = 0;
        io_conf_ECHO.pull_up_en = 0;
        gpio_config(&io_conf_ECHO);
        printDEBUG(DSYS,"HCSR\n");
}
uint8_t get_ultrasonic(void)
{
    const float speedOfSound = 0.0343;
    float distance = 0;
    uint32_t numTicks = 0;

start:
    while (1)
    {
        printDEBUG(DSYS,"racuna ultrasonic");

        gpio_set_level(TRIGGER_PIN, 0);
        vTaskDelay(0.003 / portTICK_RATE_MS);

        //1. Output 10 usec TRIG

        gpio_set_level(TRIGGER_PIN, 1);
        vTaskDelay(0.01 / portTICK_RATE_MS);
        gpio_set_level(TRIGGER_PIN, 0);

        //2. Wait for ECHO pin rising edge

        int counter = 0;
        while (gpio_get_level(ECHO_PIN) == 0)
        {
            if (counter < 3)
            {
                counter++;
                continue;
            }

            goto start;
        }

        //3. Start measuring ECHO pulse width in usec

        numTicks = 0;
        while (gpio_get_level(ECHO_PIN) == 1)
        {
            numTicks++;
            vTaskDelay(0.0028 / portTICK_RATE_MS);
        };

        //4. Estimate distance in cm

        float num = (float)numTicks;
        distance = ((num * 0.0028 * speedOfSound) / 2) * 2000;
        //real_distance = (uint8_t)distance;
 	
        return (uint8_t)distance;
    }
}


void motion_init(void){

gpio_config_t io_conf_MOTION;
    io_conf_MOTION.intr_type = GPIO_INTR_DISABLE;
    io_conf_MOTION.mode = GPIO_MODE_INPUT;
    io_conf_MOTION.pin_bit_mask =  (1 << MOTION);
    io_conf_MOTION.pull_down_en = 0;
    io_conf_MOTION.pull_up_en = 0;
    gpio_config(&io_conf_MOTION);

}

int motion_detection(void)
{
   int state;
   state = gpio_get_level(MOTION);

   return state;
}

void LED_init(void){
    gpio_config_t io_conf_LED1;
    io_conf_LED1.intr_type=GPIO_INTR_DISABLE;
    io_conf_LED1.mode=GPIO_MODE_OUTPUT;
    io_conf_LED1.pin_bit_mask=(1<<LED1);
    io_conf_LED1.pull_down_en=0;
    io_conf_LED1.pull_up_en=0;
    gpio_config(&io_conf_LED1);

    gpio_config_t io_conf_LED2;
    io_conf_LED2.intr_type=GPIO_INTR_DISABLE;
    io_conf_LED2.mode=GPIO_MODE_OUTPUT;
    io_conf_LED2.pin_bit_mask=(1<<LED2);
    io_conf_LED2.pull_down_en=0;
    io_conf_LED2.pull_up_en=0;
    gpio_config(&io_conf_LED2);
}



void LED_on(int LED_number){
    gpio_set_level(LED_number,1);
}

void LED_off(int LED_number){
    gpio_set_level(LED_number,0);
}




