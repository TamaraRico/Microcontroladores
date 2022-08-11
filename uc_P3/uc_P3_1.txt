/* Reaction time game */
#include <stdio.h>
#include <stdlib.h>     
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_spi_flash.h"
#include "driver/gpio.h"

//leds asociados a los puertos del micro
#define LED_GPIO1   GPIO_NUM_12
#define LED_GPIO2   GPIO_NUM_13
#define LED_GPIO3   GPIO_NUM_14
#define LED_GPIO4   GPIO_NUM_27

#define BTN_GPIO    GPIO_NUM_4
#define TIME_WINDOW 300       //300 ms

#define DEBOUNCE_TIME 60 //60ms
#define longPress 1000   //1seg

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
    // FIXME:
    // Replace the following code and insert
    // code to initialize all IO pins for the assigment
    /*gpio_reset_pin(LED_GPIO1);
    gpio_reset_pin(LED_GPIO2);
    gpio_reset_pin(LED_GPIO3);
    gpio_reset_pin(LED_GPIO4);*/

    gpio_set_direction(LED_GPIO1, GPIO_MODE_INPUT);
    gpio_set_direction(LED_GPIO2, GPIO_MODE_INPUT);
    gpio_set_direction(LED_GPIO3, GPIO_MODE_INPUT);
    gpio_set_direction(LED_GPIO4, GPIO_MODE_INPUT);

    //gpio_reset_pin(LED_GPIO); ESTO PARECE IMPORTANTE
    gpio_reset_pin(BTN_GPIO);
    /* Set LED GPIO as a push/pull output */
    //gpio_set_direction(LED_GPIO, GPIO_MODE_OUTPUT);
    /* Set LED GPIO as a push/pull output */
    gpio_set_direction(BTN_GPIO, GPIO_MODE_INPUT);
    gpio_pullup_en(BTN_GPIO);
}

void delayMs(uint16_t ms)
{
    vTaskDelay(ms / portTICK_PERIOD_MS);
}
/*
uint8_t checkBtn(void)
{
    // FIXME:
    // Replace the following code and insert
    // code to initialize all IO pins for the assigment
    uint8_t time_pressed=0, inicial_time=0, estado, estado_inicial=1; //maybe estado deba ser static 

    if(!gpio_get_level(GPIO_NUM_4))//if 0 = botton presionado  //regresa 0 si el estado es 0 y 1 si el estado es 1
    {
        estado=gpio_get_level(GPIO_NUM_4);
        inicial_time=_millis;//checar
        delayMs(DEBOUNCE_TIME);
        if(estado==gpio_get_level(GPIO_NUM_4))
        {
            time_pressed = _millis - inicial_time;
            if(time_pressed >= longPress) estado = eBtnLongPressed;
            else if(time_pressed > 50) estado = eBtnShortPressed;
        }
        //Siempre que este presionado el estado sera indefinido 
        return estado;
        
    }
  return eBtnUndefined;
}*/


static void update_led(int N_LED_A_PRENDER)//checar!!
{
    switch (N_LED_A_PRENDER)
    {
    case 1: //para activar el led1
    //2 1 3 4 
        gpio_set_direction(LED_GPIO2, GPIO_MODE_OUTPUT);
        gpio_set_level(LED_GPIO2, 1);
        gpio_set_direction(LED_GPIO1, GPIO_MODE_OUTPUT);
        gpio_set_level(LED_GPIO1, 0);
        gpio_set_direction(LED_GPIO3, GPIO_MODE_INPUT);
        gpio_set_direction(LED_GPIO4, GPIO_MODE_INPUT);
        break;
    case 2: //para activar el led2
        gpio_set_direction(LED_GPIO1, GPIO_MODE_OUTPUT);
        gpio_set_level(LED_GPIO1, 1);
        gpio_set_direction(LED_GPIO2, GPIO_MODE_OUTPUT);
        gpio_set_level(LED_GPIO2, 0);
        gpio_set_direction(LED_GPIO3, GPIO_MODE_INPUT);
        gpio_set_direction(LED_GPIO4, GPIO_MODE_INPUT);
        break;
    case 3: //para activar el led2
        gpio_set_direction(LED_GPIO3, GPIO_MODE_OUTPUT);
        gpio_set_level(LED_GPIO3, 1);
        gpio_set_direction(LED_GPIO2, GPIO_MODE_OUTPUT);
        gpio_set_level(LED_GPIO2, 0);
        gpio_set_direction(LED_GPIO1, GPIO_MODE_INPUT);
        gpio_set_direction(LED_GPIO4, GPIO_MODE_INPUT);
        break;
    case 4: //para activar el led2
        gpio_set_direction(LED_GPIO2, GPIO_MODE_OUTPUT);
        gpio_set_level(LED_GPIO2, 1);
        gpio_set_direction(LED_GPIO3, GPIO_MODE_OUTPUT);
        gpio_set_level(LED_GPIO3, 0);
        gpio_set_direction(LED_GPIO1, GPIO_MODE_INPUT);
        gpio_set_direction(LED_GPIO4, GPIO_MODE_INPUT);
        break;
    case 5: //para activar el led2
        gpio_set_direction(LED_GPIO1, GPIO_MODE_OUTPUT);
        gpio_set_level(LED_GPIO1, 1);
        gpio_set_direction(LED_GPIO3, GPIO_MODE_OUTPUT);
        gpio_set_level(LED_GPIO3, 0);
        gpio_set_direction(LED_GPIO2, GPIO_MODE_INPUT);
        gpio_set_direction(LED_GPIO4, GPIO_MODE_INPUT);
        break;
    case 6: //para activar el led2
        gpio_set_direction(LED_GPIO3, GPIO_MODE_OUTPUT);
        gpio_set_level(LED_GPIO3, 1);
        gpio_set_direction(LED_GPIO1, GPIO_MODE_OUTPUT);
        gpio_set_level(LED_GPIO1, 0);
        gpio_set_direction(LED_GPIO2, GPIO_MODE_INPUT);
        gpio_set_direction(LED_GPIO4, GPIO_MODE_INPUT);
        break;
    case 7: //para activar el led2
        gpio_set_direction(LED_GPIO4, GPIO_MODE_OUTPUT);
        gpio_set_level(LED_GPIO4, 1);
        gpio_set_direction(LED_GPIO3, GPIO_MODE_OUTPUT);
        gpio_set_level(LED_GPIO3, 0);
        gpio_set_direction(LED_GPIO1, GPIO_MODE_INPUT);
        gpio_set_direction(LED_GPIO2, GPIO_MODE_INPUT);
        break;
    case 8: //para activar el led2
        gpio_set_direction(LED_GPIO3, GPIO_MODE_OUTPUT);
        gpio_set_level(LED_GPIO3, 1);
        gpio_set_direction(LED_GPIO4, GPIO_MODE_OUTPUT);
        gpio_set_level(LED_GPIO4, 0);
        gpio_set_direction(LED_GPIO1, GPIO_MODE_INPUT);
        gpio_set_direction(LED_GPIO2, GPIO_MODE_INPUT);
        break;
    default:
        break;
    }
    /* Set the GPIO level according to the button state (LOW or HIGH)*/
    //gpio_set_level(led_a_endender, 1); //prender un led
}
/*
void updateLeds(uint8_t gameState)//main part of the game 
{
    // FIXME:
    // Replace the following code and insert
    // code to initialize all IO pins for the assigment
    //gpio_set_level(LED_GPIO, !gpio_get_level(BTN_GPIO));
    //static uint16_t internalCounter=0;//contar cada cuanto se alternan los leds
    static int estado_nibble=1, led_a_apagar=8;
    int contador=0, led_a_prender=1, contador2=0, alt=0;
    switch(gameState) {
        case eWaitForStart:
            led_a_prender=1;
            led_a_apagar=8;
            for(contador=0; contador<8; contador++)
            {
                for(contador2=0; contador2<7; contador2++)
                {
                    if(led_a_prender!=led_a_apagar)
                    {
                        update_led(led_a_prender);
                    }
                    led_a_prender++;
                    delayMs(1);
                }
                //update_led(led_a_prender);
                //led_a_prender++;
                led_a_apagar--;
                //delayMs(1);
            }
            //delayMs(100);
            break;
        case eStartCount:
            //
            alt=rand()%8+1;
            update_led(alt);
            break;
        case eEndCount:
           //apagar todos
            break;
        case eYouLoose:
            if(estado_nibble)
            {
                update_led(1);
                update_led(2);
                update_led(3);
                update_led(4);
                estado_nibble=0;
            }
            else{
                update_led(5);
                update_led(6);
                update_led(7);
                update_led(8);
                estado_nibble=1;
            }
            break;
        case eYouWin:
            led_a_prender=1;
            for(contador=0; contador<8; contador++)
            {
                update_led(led_a_prender);
                led_a_prender++;
                delayMs(1);
            }
                //update_led(led_a_prender);
                //led_a_prender++;
                //delayMs(1);
            delayMs(100);
            break;
    }

}*/


int app_main(void)
{
    /*uint8_t  currentGameState = eGameRestart;
    uint16_t countdown = 0;
    uint16_t countup = 0;*/
    int i=0;
    initIO();

    while(1){

        update_led(8); delayMs(5000);
        for(i=1; i<9; i++){

        update_led(i); delayMs(1000);
        }
    }
    /*while(1)
    {    
        switch(checkBtn())
        {
            case eBtnShortPressed: currentGameState++;
                break;
            case eBtnLongPressed:  currentGameState = eGameRestart;
                break;
        }
        
        

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
        delayMs(10);
        _millis++;
    }*/
}

//funcion que encienda cada uno de los leds
//01 enciende el led 2
/*ewaitfor start=1
    update leds
    if(0)
        gpiosetlevel
*/

//if(revisa el estado del boton)
//todo el codigo debe estar ciclado en el if

//short press vale 1
//long press vale 2


/*random,    cuenta aleatoria 
gamerestart
waitforstart
startcountdown
endcount
gameover*/


/*PARA ENCENDER LOS LEDS
    R1  R2  R3  R4
LED1 0 1 entrada entrada
LED2 1 0 entrada entrada
LED3 entrada 0 1 entrada
LED4 entrada 1 0 entrada
LED5 1 entrada 0 entrada 
LED6 0 entrada 1 entrada 
LED7 entrada entrada 0 1 
LED8 entrada entrada 1 0

PRIMER NIBBLE ENCENDIDO


SEGUNDO NIBBLE ENCENDIDO


*/