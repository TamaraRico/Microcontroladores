/* Microcontroladores
Rico Lopez Tamara Illian 1270673
Practica 2 - Codigo morse y manejo de leds
*/
#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "sdkconfig.h"

static const char *TAG = "example";

#define BLINK_GPIO      GPIO_NUM_2
#define BLINK_DELAY_MS  500
#define dot_MS    500 //dot len    
#define guion_MS 1500 //dot len X 3
/*const int wordLen = 7*dotLen; // space between words.*/

static uint8_t s_led_state = 0;//maybe zero
//arreglo de cadenas de caracteres con el codigo morse de la A - Z
const char morse[][50] = {".-", "-...", "-.-.", "-..", ".","..-.","--.", "....",
"..", ".---", "-.-", ".-..", "--", "-.", "---", ".--.", "--.-", ".-.", "...",
"-", "..-", "...-", ".--", "-..-", "-.--", "--.."}; //de la A-Z

static void blink_led(void){
  /* Set the GPIO level according to the state (LOW or HIGH)*/
  gpio_set_level(BLINK_GPIO, s_led_state);
}

static void configure_led(void){
  ESP_LOGI(TAG, "Example configured to blink GPIO LED!");
  gpio_reset_pin(BLINK_GPIO);
  /* Set the GPIO as a push/pull output */
  gpio_set_direction(BLINK_GPIO, GPIO_MODE_OUTPUT);
}

void delay_ms(uint16_t ms){//delay para el LED encendido o apagado
  vTaskDelay((ms*configTICK_RATE_HZ)/1000);
}

void app_main(void){
    char nombre[]="TAMARA ILLIAN RICO LOPEZ", c;
    int i=0, j=0;//contadores
    char x[50];  //cadena morse correspondiente a las letras de "nombre"

    /* Configure the peripheral according to the LED type */
    configure_led();
    delay_ms(2*guion_MS);//para evitar tomar el primer impulso como E
    s_led_state = 1;
    while (nombre[i]) {
        ESP_LOGI(TAG, "Turning the LED %s!", s_led_state == true ? "ON" : "OFF");
        if(nombre[i]==' '){
          delay_ms(3*dot_MS);
          i++;
        }
        else{
          c=nombre[i];
          strcpy(x, morse[c-65]);
          //x=morse[c-65];
          j=0;
          while(x[j]){
            blink_led();
            if(x[j]=='.'){
              delay_ms(dot_MS);
              s_led_state = !s_led_state;
            }
            else if(x[j]=='-'){
              delay_ms(guion_MS);
              s_led_state = !s_led_state;
            }
            j++;
            if(x[j]!=0){
              blink_led();
              s_led_state = !s_led_state;
              delay_ms(dot_MS);
            }
          }
        
        blink_led();
        s_led_state = !s_led_state;
        delay_ms(guion_MS);
        i++;
        }
    }
    delay_ms(2*guion_MS);
}

void setup() {
  app_main();
}

void loop() {
}