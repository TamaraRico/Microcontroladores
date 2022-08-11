/* UART Echo Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <stdio.h>
#include <inttypes.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "sdkconfig.h"
#include "myUart.h"

#define UART_NUM0        (0)
#define UART_RX_PIN0     (3)
#define UART_TX_PIN0     (1)

#define UART_NUM1        (1)
#define UART_RX_PIN1     (9)
#define UART_TX_PIN1     (10)

#define UART_NUM2        (2)
#define UART_RX_PIN2     (16)
#define UART_TX_PIN2     (17)

#define UARTS_BAUD_RATE         (115200)
#define TASK_STACK_SIZE         (1048)
#define READ_BUF_SIZE           (1024)

/* Message printed by the "consoletest" command.
 * It will also be used by the default UART to check the reply of the second
 * UART. As end of line characters are not standard here (\n, \r\n, \r...),
 * let's not include it in this string. */
const char test_message[] = "This is an example string, if you can read this, the example is a success!";

/**
 * @brief Configure and install the default UART, then, connect it to the
 * console UART.
 */

void uartInit(uart_port_t uart_num, uint32_t baudrate, uint8_t size, uint8_t parity, uint8_t stop, uint8_t txPin, uint8_t rxPin)
{
    /* Configure parameters of an UART driver,
     * communication pins and install the driver */
    uart_config_t uart_config = {
        .baud_rate = baudrate,
        .data_bits = size-5,
        .parity    = parity,
        .stop_bits = stop,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_APB,
    };

    ESP_ERROR_CHECK(uart_driver_install(uart_num, READ_BUF_SIZE, READ_BUF_SIZE, 0, NULL, 0));
    ESP_ERROR_CHECK(uart_param_config(uart_num, &uart_config));
    ESP_ERROR_CHECK(uart_set_pin(uart_num, txPin, rxPin,
                                 UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE));
}


void delayMs(uint16_t ms)
{
    vTaskDelay(ms / portTICK_PERIOD_MS);
}

void uartClrScr(uart_port_t uart_num)
{
    //uartPuts(uart_num, "\e[2J"); // para limpiar la plantalla 
    //uartPuts(uart_num, "\e[H");  // hace que el cursor vuelva al inicio 
    // Uso "const" para sugerir que el contenido del arreglo lo coloque en Flash y no en RAM
    const char caClearScr[] = "\e[2J";
    const char regresarInicio[]="\e[H";
    uart_write_bytes(uart_num, caClearScr, sizeof(caClearScr));
    uart_write_bytes(uart_num, regresarInicio, sizeof(regresarInicio));
}
void uartGoto11(uart_port_t uart_num)
{
    // Limpie un poco el arreglo de caracteres, los siguientes tres son equivalentes:
     // "\e[1;1H" == "\x1B[1;1H" == {27,'[','1',';','1','H'}
    const char caGoto11[] = "\e[1;1H";
    uart_write_bytes(uart_num, caGoto11, sizeof(caGoto11));
}

bool uartKbhit(uart_port_t uart_num)
{
    uint8_t length;
    uart_get_buffered_data_len(uart_num, (size_t*)&length);
    return (length > 0);
}

void uartPutchar(uart_port_t uart_num, char c)
{
    uart_write_bytes(uart_num, &c, sizeof(c));
}

void uartPuts(uart_port_t uart_num, char *str)
{
    while(*str)
    {
        uartPutchar(uart_num, *(str++));
        //*(str++);
    }
}

char uartGetchar(uart_port_t uart_num)
{
    char c;
    // Wait for a received byte
    while(!uartKbhit(uart_num))
    {
        delayMs(10);
    }
    // read byte, no wait
    uart_read_bytes(UART_NUM0, &c, sizeof(c), 0);

    return c;
}

void uartGets(uart_port_t uart_num, char *str)
{
    const uint8_t tam_cadena=50;//cadena a capturar
    char *inicio=str, c;//apuntador al inicio de la cadena, podria ser static 

    c=uartGetchar(uart_num);
    while(c!=13){//mientras no se presione ENTER
        if(c!=8){//8 in ascii is BACKSPACE/Borrar
            //si no se ha terminado la cadena
            if(str<(inicio+tam_cadena-1))//-1 por el caracter '\0'
                *(str++) = c;
        }
        else {
           if(str==inicio){//si es el primer caracter 
                uartPutchar(uart_num, 8);//32 en ascii es (space)
                uartPutchar(uart_num, 32);//luego borra ese espacio y se queda en la posicion inicial 
                *str=0;
            }
            else {
                uartPutchar(uart_num, 8);
                uartPutchar(uart_num, 32);
                *str=0;
                str--;//tiene que restar para que no se use ese espacio
            }
        }
        uartPutchar(uart_num, c);
        c=uartGetchar(uart_num);
    }
    uartPutchar(uart_num, 10);//SALTO DE LINEA
    *str=0; //NULL, fin de la cadena
}

void uartSetColor(uart_port_t uart_num, uint8_t color)
{
    char y[]="\033[0;1;33m", g[]="\033[0;0;32m", b[]="\033[0;0;34m";//\x1B[38;5;{33}m	0;49;93
    char cadena[15];
    switch (color)
    {
        case YELLOW: memcpy(cadena, y, strlen(y)+1);// str="\[\033[1;33m\]";
            break;
        case GREEN: memcpy(cadena, g, strlen(g)+1);
            break;
        case BLUE: memcpy(cadena, b, strlen(b)+1); 
            break;
        default:
            break;
    }
    uartPuts(uart_num, cadena);
}


void uartGotoxy(uart_port_t uart_num, uint8_t x, uint8_t y)
{
    char s_de_movimiento[]="\x1b[00;00H";//secuencia de movimiento a la posicion 00,00
    s_de_movimiento[6]=x%10+'0'; //utiliza el modulo de 10 para sacar el ultimo digito de la coordenada, y se le suma '0' para que sea un valor ascii
    x=x/10; //se divide entre 10 para poder usar el siguiente digito, tomando en cuenta que solo se hacen movimientos de 2 digitos
    s_de_movimiento[5]=x+'0';
    //s_de_movimiento[4]=';'
	s_de_movimiento[3]=y%10+'0';
	y=y/10;
	s_de_movimiento[2]=y+'0';
    uartPuts(uart_num, s_de_movimiento);//para poner la cadena en esa posicion
}

void myItoa(uint16_t number, char* str, uint8_t base)
{//convierte un valor numerico en una cadena de texto
    char *str_aux = str, n, *end_ptr, ch;
    int i=0, j=0;

    do{
        n=number % base;
        number=number/base;
        n+='0';
        if(n>'9')
            n=n+7;
        *(str++)=n;
        j++;
    }while(number>0);

    *(str--)='\0';
    
    end_ptr = str;
  
    for (i = 0; i < j / 2; i++) {
        ch = *end_ptr;
        *end_ptr = *str_aux;
        *str_aux = ch;
          str_aux++;
        end_ptr--;
    }
}

uint16_t myAtoi(char *str)
{//convertir una cadena en un entero 
    uint16_t num=0, num_from_str=0;

    while(*str) {//mientras no se termine la cadena
        //num=num*10; //si la cadena no se ha terminado, multiplicar por 10
        num_from_str=*str-'0';//le restamos '0' para tener el valor crudo, no ascii
        if(num_from_str>=0 && num_from_str<=9) {
            num+=num_from_str;
            str++;
            if(*str >= '0' && *str <= '9') {
                num=num*10;
            }
        }
        else break;
    }
    return num;
}

void app_main(void)
{
// The following is only example code, delete this and implement
// what is inside the TO_IMPLEMENT check
  /*  char payload[] = "Hola mundo!";
    char cad[20];
    //printf("%s", payload);
    uartInit(0, 115200, 8, 0, 1, UART_TX_PIN0, UART_RX_PIN0);
    delayMs(500);
    uartGoto11(UART_NUM0);
    uartClrScr(UART_NUM0);

    uartPutchar(UART_NUM0,payload[0]);
    uartPutchar(UART_NUM0,payload[1]);
    uartPutchar(UART_NUM0,payload[2]);
    uartPutchar(UART_NUM0,payload[3]);
    uartPutchar(UART_NUM0,payload[10]);

    uartClrScr(UART_NUM0);*/
    
    // Wait for input
   /* delayMs(500);
    uartPuts(0,"Introduce un número:");
    uartGets(0,cad);
    uartClrScr(UART_NUM0);
    delayMs(500);
    uartPuts(0,cad);*/

    
    // echo forever
    /*while(1)
    {
        uartPutchar(UART_NUM0,uartGetchar(UART_NUM0));
    }*/

    char cad[20];
    char cadUart3[20];
    uint16_t num;

    uartInit(0, 115200, 8, 1, 2, UART_TX_PIN0, UART_RX_PIN0);
    uartInit(1, 115200, 8, 0, 1, UART_TX_PIN1, UART_RX_PIN1);
    uartInit(2, 115200, 8, 0, 1, UART_TX_PIN2, UART_RX_PIN2);

    /*
    uartInit(0, 9600, 7, 1, 2, UART_TX_PIN0, UART_RX_PIN0);
    uartInit(1, 9600, 7, 0, 1, UART_TX_PIN1, UART_RX_PIN1);
    uartInit(2, 9600, 7, 0, 1, UART_TX_PIN2, UART_RX_PIN2);*/

    while(1) 
    {
        uartGetchar(0);
        uartClrScr(0);

        uartGotoxy(0,2,2);
        uartSetColor(0,YELLOW);
        uartPuts(0,"Introduce un numero:");
        
        uartGotoxy(0,2,22);
        uartSetColor(0,GREEN);
        //uartPutchar(0,"a");
        //uartPuts(0,"b");
        uartGets(0,cad);
        //uartPuts(0, cad);
// For the following code to work, TX1 must be physically 
// connected with a jumper wire to RX2
// -------------------------------------------
        // Cycle through UART1->UART2
        uartPuts(1,cad);
        uartPuts(1,"\r");
        uartGets(2,cadUart3);
        uartGotoxy(0,5,3);
        uartPuts(0,cadUart3);
// -------------------------------------------
        num = myAtoi(cad);
        myItoa(num,cad,16);
        
        uartGotoxy(0,5,4);
        uartSetColor(0,BLUE);
        uartPuts(0,"Hex: ");
        uartPuts(0,cad);
        myItoa(num,cad,2);
        
        uartGotoxy(0,5,5);
        uartPuts(0,"Bin: ");
        uartPuts(0,cad);
    }
}
