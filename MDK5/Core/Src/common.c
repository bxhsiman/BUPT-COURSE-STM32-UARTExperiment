#include <stdio.h>
#include <string.h>
#include "main.h"
#include "tim.h"
#include "usart.h"
#include "uart_dma.h"
#include "usb_uart.h"
#include "gpio.h"
#include "stop_watch.h"
#include "music.h"
#include "common.h"
#include "timer.h"

extern uint8_t g_stopwatch_flag ;
extern uint8_t g_music_flag ;

uint32_t rxdatanum = 0;
uint32_t uartrxdatanum = 0;

uint8_t uart1_busy = 0 ;

UART_HandleTypeDef * g_user_uart = &huart1;

int fputc(int ch,FILE *p) 
{
	HAL_UART_Transmit( g_user_uart, (uint8_t *)&ch, 1, 10 );
	return ch ;
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
	if ( htim == &htim1 )
	{
		g_stopwatch_flag = 1 ;
	}
	else if ( htim == &htim3 )
	{
		g_music_flag = 1 ;
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
}

//////////////////////////////////////////////////////////////////////////////////////////////
void SetTime(uint32_t *settime)
{
    if (settime[0] > 24 || settime[1] > 59 || settime[2] > 59) {
        printf("Value Error!\n");
        return;
    }
    hours = settime[0];
    minutes = settime[1];
    seconds = settime[2];
}

void Uart1_RxDataCallback( uint8_t * buf , uint32_t len )
{
		uint8_t i ;
		uint32_t settime[3];

		if ( ( len >= 3 ) && (strncmp( buf , "SET_TIME " , 9 ) == 0) )
		{
			
			if( len < 14 ) return;
			
			sscanf( buf ,"SET_TIME %d:%d:%d",settime,settime+1,settime+2);
      SetTime(settime);
		}
		if ( ( len >= 5 ) && (strncmp( buf , "RESET" , 5 ) == 0) )
		{
			for (int i = 0 ; i < 2 ; i++){
				   settime[i] = 0;
			}

			__set_FAULTMASK(1);
			NVIC_SystemReset();
		
		}
}


uint8_t g_uart_rx_buf[32] ;
//////////////////////////////////////////////////////////////////
void Uart1_RxDataCallback_ToUSB( uint8_t * buf , uint32_t len ){
	uartrxdatanum += len;
	USBTxDataDMA( buf, len);
}

//////////////////////////////////////////////////////////////////
void System_Init( void )
{

	HAL_TIM_Base_Start_IT( &htim1 );
	HAL_TIM_Base_Start_IT( &htim3 );

    StartAllUartDMAReceive();
    Usb2UartInit();

}


void UserTasks( void)
{	
//	stop_watch_process( );		
//	stop_watch_show( );
//	music_process();
	
	
		timer_process();
	
		timer_show();

		CheckUartRxData();
		CheckUSBTxData();
    	CheckUSBRxData();
		CheckUartTxData();
		
    

	
//	CheckUSBRxData();
//	CheckUSBTxData();
	
}
