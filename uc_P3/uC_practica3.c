/* Reaction time game */
#include <stdio.h>
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_spi_flash.h"
#include "driver/gpio.h"

#define LED_GPIO_1    GPIO_NUM_12
#define LED_GPIO_2    GPIO_NUM_13
#define LED_GPIO_3    GPIO_NUM_14
#define LED_GPIO_4    GPIO_NUM_27
#define LED_GPIO_5    GPIO_NUM_23

#define BTN_GPIO      GPIO_NUM_4
#define BTN_GPIO_G    GPIO_NUM_5
#define DEBOUNCE_TIME 600
#define LONG_PRESSED  100
#define TIME_WINDOW   300//300 ms

// Global variable
uint32_t _millis;

enum eButtonStates
{
    eBtnUndefined = 0,
    eBtnShortPressed,
    eBtnLongPressed
};

enum eGameStates
{
    eGameRestart = 0,
    eWaitForStart,
    eStartCount,
    eEndCount,
    eGameOver,
    eYouWin,
    eYouLoose
};

static void initIO(void)
{
    gpio_reset_pin(LED_GPIO_1);
    gpio_reset_pin(LED_GPIO_2);
    gpio_reset_pin(LED_GPIO_3);
    gpio_reset_pin(LED_GPIO_4);
    gpio_reset_pin(LED_GPIO_5);
    gpio_reset_pin(BTN_GPIO);

    /* Set LED GPIO as a push/pull output */
    gpio_set_direction(LED_GPIO_1, GPIO_MODE_OUTPUT);
    gpio_set_direction(LED_GPIO_2, GPIO_MODE_OUTPUT);
    gpio_set_direction(LED_GPIO_3, GPIO_MODE_OUTPUT);
    gpio_set_direction(LED_GPIO_4, GPIO_MODE_OUTPUT);
    gpio_set_direction(LED_GPIO_5, GPIO_MODE_OUTPUT);

    /* Set LED GPIO as a push/pull output */
    gpio_set_direction(BTN_GPIO, GPIO_MODE_INPUT);
    gpio_pullup_en(BTN_GPIO);
    gpio_set_direction(BTN_GPIO_G, GPIO_MODE_OUTPUT);//poner el pin5 en tierra, cambio de estado
}

static void update_led(gpio_num_t pin_activar, gpio_num_t pin_desactivar, gpio_num_t pin_alta1, gpio_num_t pin_alta2)
{
    gpio_set_direction(pin_activar, GPIO_MODE_OUTPUT);
    gpio_set_level(pin_activar, 1);
    gpio_set_direction(pin_desactivar, GPIO_MODE_OUTPUT);
    gpio_set_level(pin_desactivar, 0);
    gpio_set_direction(pin_alta1, GPIO_MODE_INPUT);
    gpio_set_direction(pin_alta2, GPIO_MODE_INPUT);
}

void encenderLED(int led)
{
    switch (led)
    {
        case 1:
            update_led(LED_GPIO_2, LED_GPIO_1, LED_GPIO_3, LED_GPIO_4);
            break;
        case 2:
            update_led(LED_GPIO_1, LED_GPIO_2, LED_GPIO_3, LED_GPIO_4);
            break;
        case 3:
            update_led(LED_GPIO_3, LED_GPIO_2, LED_GPIO_1, LED_GPIO_4);
            break;
        case 4:
            update_led(LED_GPIO_2, LED_GPIO_3, LED_GPIO_1, LED_GPIO_4);
            break;
        case 5:
            update_led(LED_GPIO_1, LED_GPIO_3, LED_GPIO_2, LED_GPIO_4);
            break;
        case 6:
            update_led(LED_GPIO_3, LED_GPIO_1, LED_GPIO_2, LED_GPIO_4);
            break;
        case 7:
            update_led(LED_GPIO_4, LED_GPIO_3, LED_GPIO_1, LED_GPIO_2);
            break;
        case 8:
            update_led(LED_GPIO_3, LED_GPIO_4, LED_GPIO_1, LED_GPIO_2);
            break;
        default:
            break;
    }
}

void delayMs(uint16_t ms)
{
    vTaskDelay(ms/portTICK_PERIOD_MS);
}

uint8_t checkBtn(void)
{
    //uint8_t pasado = 0;
    uint32_t tiempo, inicial;
    if(!gpio_get_level(BTN_GPIO)){
        inicial = _millis;
        delayMs(DEBOUNCE_TIME);
        if(!gpio_get_level(BTN_GPIO)){
            tiempo=_millis-inicial;
            if(tiempo>=LONG_PRESSED)
                return eBtnLongPressed;
            else
                return eBtnShortPressed;
        }
    }
    return eBtnUndefined;
}

void sequencia_aleatoria()
{
    uint8_t alt;
    alt=rand()%8+1;
    encenderLED(alt);
    delayMs(100);
}


void walking_zero_sequence()
{
    int i=0, j=0, k=0, n=0, led_a_apagar=8;
    for(n=1; n<=8; n++)
    {
        for(k=0; k<500; k++)//100 1000
        {
            i=1;
            for(i=1; i<=8; i++)
            {
                if(i!=led_a_apagar)
                {
                    encenderLED(i);
                    delayMs(5);
                }
            }
            for(j=1; j<=8; j++)
            {
                if(j!=led_a_apagar)
                {
                encenderLED(j);
                delayMs(7);
                }
            }
            delayMs(1);
        }
        delayMs(1);
        led_a_apagar--;
    }
}

void nibble_onoff()
{
    int i=0, j=0, k=0, n=0;
    for(k=0; k<5000; k++)
    {
        for(i=1; i<=4; i++)
        {
            encenderLED(i);
            delayMs(1);
        }
        delayMs(1);
    }
    for(n=0; n<5000; n++)
    {
        for(j=5; j<=8; j++)
        {
            encenderLED(j);
            delayMs(2);
        }
        delayMs(2);
    }
}

void leds_off()
{
    gpio_set_direction(LED_GPIO_1, GPIO_MODE_OUTPUT);
    gpio_set_direction(LED_GPIO_2, GPIO_MODE_OUTPUT);
    gpio_set_direction(LED_GPIO_3, GPIO_MODE_OUTPUT);
    gpio_set_direction(LED_GPIO_4, GPIO_MODE_OUTPUT);

    gpio_set_level(LED_GPIO_1, 0);
    gpio_set_level(LED_GPIO_2, 0);
    gpio_set_level(LED_GPIO_3, 0);
    gpio_set_level(LED_GPIO_4, 0);
}

void on_off()
{
    int i, j, k;
    for(k=0; k<2500; k++)
    {
        for(i=1; i<=8; i++)
        {
            encenderLED(i);
            delayMs(1);
        }
        for(j=1; j<=8; j++)
        {
            encenderLED(j);
            delayMs(2);
        }
    }
    leds_off();
    delayMs(250);
}


void updateLeds(uint8_t gameState)//main part of the game 
{
    switch(gameState) {
        case eWaitForStart:
            walking_zero_sequence();
            break;
        case eStartCount:
            sequencia_aleatoria();
            break;
        case eEndCount:
            leds_off();
            break;
        case eYouLoose:
            nibble_onoff();
            break;
        case eYouWin:
            on_off();
            break;
    }
}

/*void updateLeds(uint8_t gameState)
{
    gpio_set_level(LED_GPIO, !gpio_get_level(BTN_GPIO)); 
}*/

int app_main(void)
{
    uint8_t  currentGameState = eGameRestart;
    uint16_t countdown = 0;
    uint16_t countup = 0;
    initIO();

    while(1)
    {   
        switch(checkBtn())
        {
            case eBtnShortPressed: currentGameState++;
                break;
            case eBtnLongPressed:  currentGameState = eGameRestart;
                break;
        }

       /*secuenica de leds eYouLoose*/
        /*for(k=0; k<5000; k++)
        {
            i=1;
            for(i=1; i<=4; i++)
            {
                encenderLED(i);
                delayMs(1);
            }
            delayMs(1);
        }

        for(n=0; n<5000; n++)
        {
         j=5;
            for(j=5; j<=8; j++)
            {
                encenderLED(j);
                delayMs(2);
            }
            delayMs(2);
        }*/

        /*
        switch(checkBtn())
        {
            case eBtnShortPressed: //currentGameState++;
                encenderLED(5);
                delayMs(500);
                break;
            case eBtnLongPressed:  //currentGameState = eGameRestart;
                encenderLED(4);
                break;
            default:
                encenderLED(8);
                break;
        }*/

        switch(currentGameState)
        {
            case eGameRestart:
                countdown = esp_random();
                countup = 0;
                currentGameState++;
                break;
            case eWaitForStart:
                break;
            case eStartCount:
                countdown--;
                if (countdown == 0)
                {
                currentGameState++;
                }
                break;
            case eEndCount:
                if (countdown != 0)
                {
                    currentGameState++;
                }
                else
                {
                    countup++;
                }
                break;
            case eGameOver:
                if ((countdown+countup) > TIME_WINDOW)
                {
                    currentGameState = eYouLoose;
                }
                else 
                {
                    currentGameState = eYouWin;
                }
                break;
        }
        updateLeds(currentGameState);
    }

}