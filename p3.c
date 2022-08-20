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

#define BTN_GPIO      GPIO_NUM_4
#define BTN_GPIO_G    GPIO_NUM_5
#define DEBOUNCE_TIME 20
#define LONG_PRESSED  10000
#define SHORT_PRESSED 300
#define TIME_WINDOW   25000//3000//300 ms

// Global variable
uint32_t _millis = 0;
uint8_t pasado = 1;
int boton = 0, arreglo[8], contador_alt=0;
int i = 1, j = 1, contador = 0, debounce_counter=0;
int estado_actual=1, estado_pasado=1, pressedTime=0, releasedTime=0;
uint8_t nibble_state=0;
int counter_nibble=0, led_s=1;

uint8_t win_state=1;
int win_counter=0;

uint8_t led_a_apagar=8;
int walking_zero_counter=0;

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

    gpio_reset_pin(BTN_GPIO);

    /* Set LED GPIO as a push/pull output */
    gpio_set_direction(LED_GPIO_1, GPIO_MODE_OUTPUT);
    gpio_set_direction(LED_GPIO_2, GPIO_MODE_OUTPUT);
    gpio_set_direction(LED_GPIO_3, GPIO_MODE_OUTPUT);
    gpio_set_direction(LED_GPIO_4, GPIO_MODE_OUTPUT);

    /* Set LED GPIO as a push/pull output */
    gpio_set_direction(BTN_GPIO, GPIO_MODE_INPUT);
    gpio_pullup_en(BTN_GPIO);
    gpio_set_direction(BTN_GPIO_G, GPIO_MODE_OUTPUT);//poner el pin5 en tierra, cambio de estado
}

static void update_led(gpio_num_t pin_en_uno, gpio_num_t pin_en_cero, gpio_num_t pin_alta_i_1, gpio_num_t pin_alta_i_2)
{
    gpio_set_direction(pin_en_uno, GPIO_MODE_OUTPUT);
    gpio_set_level(pin_en_uno, 1);
    gpio_set_direction(pin_en_cero, GPIO_MODE_OUTPUT);
    gpio_set_level(pin_en_cero, 0);
    gpio_set_direction(pin_alta_i_1, GPIO_MODE_INPUT);
    gpio_set_direction(pin_alta_i_2, GPIO_MODE_INPUT);
}

void prender_led_charlieplexing(int led)
{
    switch(led)
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
    estado_actual=gpio_get_level(BTN_GPIO);
    if(debounce_counter>0)  
    {
        debounce_counter++;
        if(debounce_counter>=DEBOUNCE_TIME)
        {
            if(estado_pasado==1 && estado_actual==0)
            {//el boton ha sido presionado
                pressedTime=_millis;
            }
            else if(estado_pasado==0 && estado_actual==1)
            {
                releasedTime=_millis;
                if(releasedTime-pressedTime > 4000)
                {
                    pressedTime=0;
                    releasedTime=0;
                    estado_pasado=1;
                    debounce_counter=0;
                    return eBtnLongPressed;
                }
                else if(releasedTime-pressedTime > 30)
                {
                    pressedTime=0;
                    releasedTime=0;
                    estado_pasado=1;
                    debounce_counter=0;
                    return eBtnShortPressed;
                }
            } 
            //estado_pasado=estado_actual;
        }
        else if(estado_actual==1 && estado_pasado==1) debounce_counter = 0;
    }
    else if(!estado_actual) debounce_counter=1;
    estado_pasado=estado_actual;
    return eBtnUndefined;
}

void sequencia_aleatoria()
{
    if(contador_alt == 0){
        for(int i=0; i<8; i++){
            arreglo[i] = rand()%2;
        }
    }
    if(led_s<=8){
        if(arreglo[led_s-1] != 0)
            prender_led_charlieplexing(led_s);
        contador_alt++;
        if(contador_alt>=100)
            contador_alt=0;
        if(led_s==8)
            led_s=1;
        else
            led_s++;
    }
           
}

void walking_zero_sequence()
{
    int i=0;
        if(walking_zero_counter<=5000)
        {
            for(i=1; i<=8; i++)
            {
                if(i!=led_a_apagar)
                {
                    prender_led_charlieplexing(i);
                    delayMs(1);
                    walking_zero_counter++;
                }
            }
            if(walking_zero_counter>=5000)
            {
                led_a_apagar--;
                walking_zero_counter=0;
                if(led_a_apagar==0)
                    led_a_apagar=8;
            }
        }
        delayMs(1);
}

void nibble_onoff()
{
    int i=0, j=0;
    if(!nibble_state)
    {
         for(i=1; i<=4; i++)
        {
            prender_led_charlieplexing(i);
            counter_nibble++;
            delayMs(1);
        }
        delayMs(1);
        if(counter_nibble>5000)
        {
            nibble_state=1;
            counter_nibble=0;
        }
    }
    else if(nibble_state)
    {
         for(j=5; j<=8; j++)
        {
            prender_led_charlieplexing(j);
            counter_nibble++;
            delayMs(2);
        }
        delayMs(2);
        if(counter_nibble>5000)
        {
            nibble_state=0;
            counter_nibble=0;
        }
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

void on_off()//todos los leds se encienden y despues se apagan 
{
    int i=0;
    if(!win_state)
    {
         for(i=1; i<=8; i++)
        {
            prender_led_charlieplexing(i);
            win_counter++;
            delayMs(1);
        }
        if(win_counter>10000)
        {
            win_state=1;
            win_counter=0;
        }
    }
    else if(win_state)
    {
        leds_off();
        delayMs(10);
        _millis+=10;
        win_counter++;
        if(win_counter>90)
        {
            win_state=0;
            win_counter=0;
        }
    }

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
            case eBtnShortPressed: 
                if(currentGameState!=eYouWin && currentGameState!=eYouLoose)
                    currentGameState++;
                i = 1, j = 1, contador = 0;
                break;
            case eBtnLongPressed: currentGameState = eGameRestart;
                i = 1, j = 1, contador = 0;
                break;
            default:
                break;
        }
        
        switch(currentGameState)
        {
            case eGameRestart:
                _millis=0;
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
        delayMs(1);
        _millis++;
    }
}
