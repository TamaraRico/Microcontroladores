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
#define DEBOUNCE_TIME 60
#define LONG_PRESSED  10000
#define SHORT_PRESSED 300
#define TIME_WINDOW   3000//3000//300 ms

// Global variable
uint32_t _millis = 0;
uint8_t pasado = 1;
int boton = 0;
int i = 1, j = 1, contador = 0;
int debounce_counter=0;
int estado_actual=1, estado_pasado=1, pressedTime=0, releasedTime=0;
uint8_t nibble_state=0;
int counter_nibble=0;

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

uint8_t checkBtn2(void){
    estado_actual=gpio_get_level(BTN_GPIO);
        if((_millis-DEBOUNCE_TIME)>10)
        {
            if(estado_pasado==1 && estado_actual==0)
            {
                //el boton ha sido presionado
                pressedTime=_millis;
                //printf("\npressed time %d", pressedTime);
            }
            else if(estado_pasado==0 && estado_actual==1)
            {
                releasedTime=_millis;
                //printf("\nreleasedTime: %d", releasedTime);
                //printf("\ntIEMPO: %d", releasedTime-pressedTime);
                if(releasedTime-pressedTime > 10000)
                {
                    pressedTime=0;
                    releasedTime=0;
                    estado_pasado=1;
                    printf("\nLong pressed");
                    return eBtnLongPressed;
                }
                else
                {
                    pressedTime=0;
                    releasedTime=0;
                    estado_pasado=1;
                    printf("\nShort pressed");
                    return eBtnShortPressed;
                }
        }
    
        estado_pasado=estado_actual;
    }
    return eBtnUndefined;
}

uint8_t checkBtn1(void)
{
    uint8_t actual = gpio_get_level(BTN_GPIO);
    if(boton > 0){ //Si se habia presionado previamente
        //delayMs(1);
        boton++;
        printf("%d\n", boton);
        if(boton >= 20){ //Si duro 20 ms o mas es una presion valida
            if(actual && pasado){ //Si ya no esta presionado
                if(boton > 1000){
                    pasado = actual;
                    boton = 0;
                    return eBtnLongPressed;
                }
                else{
                    pasado = actual;
                    boton = 0;
                    return eBtnShortPressed;
                }
            }
        }
        else{
            if(actual && pasado)
                boton = 0;
        }
    }
    else{ //Primer ms que se presiono
        if(!actual){
            //delayMs(1);
            boton = 1;
        }
    }
    pasado = actual;
    return eBtnUndefined;
}

void sequencia_aleatoria()
{
    uint8_t alt;
    alt=rand()%8+1;
    prender_led_charlieplexing(alt);
    delayMs(10);
}

void walking_zero_sequence1(){
    if(j<=8){
        if(i<=8){
            if (i != j)
            {
                prender_led_charlieplexing(i);
            }
            contador++;
            printf("%d  ", j);
                    printf("%d  ", i);
                    printf("%d\n", contador);
            if(contador == 100){
                contador = 0;
                if (j == 8)
                    j=1;
                else
                    j++;
            }
            if(i == 8)
                i = 1;
            else
                i++;
        }
    }
}

void walking_zero_sequence()
{
    int i=0;// j=0, k=0, n=0;
        if(walking_zero_counter<=5000)
        {
            for(i=1; i<=8; i++)
            {
                if(i!=led_a_apagar)
                {
                    prender_led_charlieplexing(i);
                    delayMs(1);//5
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
        delayMs(1);//puede que quite este 
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
        delayMs(2);//puede que quite este 
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

void on_off()
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
        switch(checkBtn2())
        {
            case eBtnShortPressed: 
                if(currentGameState!=eYouWin && currentGameState!=eYouLoose)
                    currentGameState++;
                //printf("\nShort pressed");
                i = 1, j = 1, contador = 0;
                break;
            case eBtnLongPressed: currentGameState = eGameRestart;
                i = 1, j = 1, contador = 0;
                //printf("\nLong pressed");
                break;
            default:
                break;
        }
        
        switch(currentGameState)
        {
            case eGameRestart:
                //countdown = esp_random();
                //if(countdown>100)
                countdown=500;
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
                printf("\ncountdown: %d", countdown);
                printf("\ncountup: %d", countup);
                printf("\ncountdown+countup: %d", countdown+countup);
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