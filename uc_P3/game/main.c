/* Reaction time game */
#include <stdio.h>
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_spi_flash.h"
#include "driver/gpio.h"

#define LED_GPIO    GPIO_NUM_2
#define BTN_GPIO    GPIO_NUM_4
#define TIME_WINDOW 300       //300 ms

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
    gpio_reset_pin(LED_GPIO);
    gpio_reset_pin(BTN_GPIO);
    /* Set LED GPIO as a push/pull output */
    gpio_set_direction(LED_GPIO, GPIO_MODE_OUTPUT);
    /* Set LED GPIO as a push/pull output */
    gpio_set_direction(BTN_GPIO, GPIO_MODE_INPUT);
    gpio_pullup_en(BTN_GPIO);
}

void updateLeds(uint8_t gameState)
{
    // FIXME:
    // Replace the following code and insert
    // code to initialize all IO pins for the assigment
    gpio_set_level(LED_GPIO, !gpio_get_level(BTN_GPIO)); 
}

uint8_t checkBtn(void)
{
    // FIXME:
    // Replace the following code and insert
    // code to initialize all IO pins for the assigment
  return eBtnUndefined;
}

void delayMs(uint16_t ms)
{
    vTaskDelay(ms / portTICK_PERIOD_MS);
}

static void update_led(void)
{
    /* Set the GPIO level according to the button state (LOW or HIGH)*/
    gpio_set_level(LED_GPIO, gpio_get_level(BTN_GPIO));
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
    }
}