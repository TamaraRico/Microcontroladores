
#include <stdio.h>
#include <inttypes.h>
#include <string.h>
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_spi_flash.h"
#include "esp_task_wdt.h"

/* 
extern uint32_t test1(void);*/
//extern void uart(uint32_t, uint32_t);
/*extern uint32_t f(uint32_t, uint32_t);
extern void taller3(uint32_t, uint8_t, uint32_t);*/
extern uint32_t invertirCadena(uint32_t); 

#define _timer_cofig_reg (*(volatile uint32_t *)) 0x3ff5f000 
#define PAR 2
#define setBitsParity 0x3
#define setBitsParityZero 0xfffffffc
#define setBitsNdatos 0xc
#define setBitsNdatosZero 0xfffffff3
#define setStopBit 0x30
#define setDecimal 0xf00000
#define setEntero 0x000fffff

/* uint8_t UART0_Config (uint16_t velocidad, uint8_t ndatos, uint8_t paridad, uint8_t bit_paro){
    uint8_t result = 0; uint32_t divisor = 80000000, residuo;
    uint32_t *ptr_config_uart0 = (uint32_t *) 0x3ff40020;
    uint32_t *ptr_clkdiv_reg   = (uint32_t *) 0x3ff40014;
    printf("\nUART Config = 0x%X", *ptr_config_uart0);
    
    if(paridad > 2 || (bit_paro!=1 && bit_paro!=2) || (ndatos > 8 && ndatos < 5))
        result = 0;
    else
    {
        //paridad = ultimos dos bits del registro
        *ptr_config_uart0 = paridad!=0 ? ((*ptr_config_uart0 & ~setBitsParity) | paridad) : (*ptr_config_uart0 & ~setBitsParity);
        printf("\nUART Config = 0x%X", *ptr_config_uart0);

        //ndatos = bits 3-2 del registro
        ndatos -= 5; 
        *ptr_config_uart0 = ndatos!=0 ? ((*ptr_config_uart0 & ~setBitsNdatos) | (ndatos<<2)) : (*ptr_config_uart0 & ~setBitsNdatos);
        printf("\nUART Config = 0x%X", *ptr_config_uart0);

        //stop bit = bits 5-4 del registro
        *ptr_config_uart0 = (*ptr_config_uart0 & ~setStopBit) | ((bit_paro<<4));
        printf("\nUART Config = 0x%X", *ptr_config_uart0);

        //set baudaje
        residuo = (divisor * 16 / velocidad);
        *ptr_clkdiv_reg = ((*ptr_clkdiv_reg & ~setDecimal) | ((residuo<<20) & setDecimal)); 
        divisor = divisor / velocidad;
        ptr_clkdiv_reg = ((*ptr_clkdiv_reg & ~setEntero) | ((uint32_t)divisor & setEntero)); //1046
        //printf("\nregister = 0x%X", ((*ptr_clkdiv_reg & ~setEntero) | ((uint32_t)divisor & setEntero))); //0xA01046
        result = 1;
    }
} */

void Delay_100mS(void){
    //clear timer
    //set alarm
    //start timer
    //wait alarm 
    //80 prescalar 100,000 

    uint32_t *ptr_timer_cofig_reg = (uint32_t *) 0x3ff5f000;
    uint32_t *ptr_timer_alarmLOW_reg = (uint32_t *) 0x3ff5f010;
    uint32_t *ptr_timer_alarmHIGH_reg =(uint32_t *) 0x3ff5f014;
    uint32_t *ptr_timer_load_reg  = (uint32_t *) 0x3ff5f020;
    uint32_t *ptr_timer_loadLOW_reg  = (uint32_t *) 0x3ff5f018;
    uint32_t *ptr_timer_loadHIGH_reg = (uint32_t *) 0x3ff5f01c;
    uint16_t preescalador = 80;

    /*Clear timer*/
    *ptr_timer_cofig_reg = 0;//deshabilitar contador
    *ptr_timer_loadLOW_reg  = 0; //iniciar el contador en 0
    *ptr_timer_loadHIGH_reg = 0; 
    *ptr_timer_load_reg = 1;//cargar los valores anteriores 

    //config register
    *ptr_timer_cofig_reg |= (1<<30); //incrementar 
    //preescalador = frecuencia
    *ptr_timer_cofig_reg &= (preescalador<<13);
    //enable alarm
    *ptr_timer_cofig_reg &= (0x1<<10);
    *ptr_timer_cofig_reg &= ~(0xffff<<13);
     
    /*clk = 0.1 
    frecuencia = 80M
    preescalador = X
    valor = (uint64_t) (float) frcuencia / preescalador * clk 
    valor low 
    valor high 
    */

     /*Set alarm */
    *ptr_timer_alarmLOW_reg = 100000;
    *ptr_timer_alarmHIGH_reg = 0; //100ms

    /*start timer*/
    *ptr_timer_cofig_reg |= (1<<31);

    while(1){
        if(!(*ptr_timer_cofig_reg & (1<<10))){
            break;
        }
    }

}

#define def_reg_32b(addr) (*(volatile uint32_t *) (addr))
#define uart0_clkdiv def_reg_32b(0x3FF40014)

int UARTn_getSpeed(){
    uint32_t speed = 0;
    speed = (uart0_clkdiv & 0x00F00000) >> 20;
    speed |= (uart0_clkdiv & 0x000FFFFF) << 4;
    speed = (1280000000) / speed;
    return speed;
}

#define gpio_output_enable def_reg_32b(0x3FF44020)
int GPIOn_isINPUT(int num_gpio){
	if((gpio_output_enable >> num_gpio) & 1){
		return 0;
	}
	return 1;
}

uint8_t UART0_Config(uint32_t velocidad, uint8_t ndatos, uint8_t paridad, uint8_t bit_paro){
    uint8_t result = 0; uint32_t divisor = 80000000, residuo;
    uint32_t *ptr_config_uart0 = (uint32_t *) 0x3ff40020;
    uint32_t *ptr_clkdiv_reg   = (uint32_t *) 0x3ff40014;

    //verificar que los parametros esten dentro del rango establecido
    if(paridad > 2 || (bit_paro!=1 && bit_paro!=2) || (ndatos > 8 && ndatos < 5)) result = 0;
    else
    {
        //paridad = ultimos dos bits del registro
        *ptr_config_uart0 = paridad == 1 ? (*ptr_config_uart0 | 3) : ((*ptr_config_uart0 | paridad));
        //ndatos = bits 3-2 del registro
        *ptr_config_uart0 = (ndatos-5)!=0 ? ((*ptr_config_uart0) | ((ndatos-5)<<2)) : (*ptr_config_uart0 & ~(3<<2));
        //stop bit = bits 5-4 del registro
        *ptr_config_uart0 = (*ptr_config_uart0 & ~setStopBit) | ((bit_paro<<4));
        //set baudaje
        residuo = (divisor * 16) / velocidad;
        printf("CLK register: 0x%X", *ptr_clkdiv_reg);
        printf("\nCLK register: 0x%X", (((residuo & 0xf) << 20) | (residuo >> 4)) );
        result = 1;
    }
    return result;
}


void app_main(void)
{
    printf("Inicio\n");
    //uint32_t *ptr_timer_cofig_reg = (uint32_t *) 0x3ff5f000;
    //int x=1;
    //int result=23;
    char cadena[] = "hola";
    uint32_t *dir = &cadena;

    while(1){

            //int speed = UARTn_getSpeed();
            //printf("\nVelocidad : %d", speed);
            //printf("prueba...\n");
            //printf("prueba test1() -- > 0x%08X\n", test1());
            /*esp_task_wdt_init(999, false);
            printf("Han pasado 100 ms\n");
            Delay_100mS();*/

            //vTaskDelay(1000 / portTICK_PERIOD_MS);
            /*uart(19200, 0x3ff40014);
            printf("prueba uart() -- > 1\n");
            vTaskDelay(1000 / portTICK_PERIOD_MS);*/
            /*
            vTaskDelay(1000 / portTICK_PERIOD_MS);
            printf("prueba f() -- > %d\n", f(9, 1));
            vTaskDelay(1000 / portTICK_PERIOD_MS);
            
            vTaskDelay(1000 / portTICK_PERIOD_MS);
            taller3(&result, 3, 1);
            printf("prueba f() -- > %d\n", result);
            vTaskDelay(1000 / portTICK_PERIOD_MS);*/

            vTaskDelay(1000 / portTICK_PERIOD_MS);
            invertirCadena(dir);
            printf("prueba fcadena() -- > %s\n", cadena);
            vTaskDelay(1000 / portTICK_PERIOD_MS); 

            /* Configura UART0 para operar a 19200 Baud, 8 bits de dato, paridad PAR y 1 bit de paro */
            //uint8_t result = UART0_Config (19100, 8, 1, 1);
            //printf("\nConfiguracion : %d\n", result);
            vTaskDelay(1000 / portTICK_PERIOD_MS);

            /* Configura UART0 para operar a 19200 Baud, 8 bits de dato, No paridad y 1 bit de paro */
            //UART_Config ("COM0:19200,8,N,1") ;
    }
}
