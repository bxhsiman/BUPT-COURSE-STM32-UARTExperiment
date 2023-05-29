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
uint8_t RXtimerFlag;
uint32_t settime[3];
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

void Uart1_RxDataCallback( uint8_t * buf , uint32_t len )
{
		uint8_t i ;
		extern uint32_t settime[3];
		;
		if ( ( len >= 3 ) && (strncmp( buf , "SET_TIME " , 9 ) == 0) )
		{
			
			if( len < 14 ) return ;
			//for ( i = 0 ; i < 6 ; i ++ )
				//{
					//if ( buf[i+3] < '0' || buf[i+3] > '9' ) return ;					
				//}
				//h= (buf[7]-'0' )*10+(buf[8]-'0' );
				//m= (buf[5]-'0' )*10+(buf[6]-'0' );
				//s= (buf[3]-'0' )*10+(buf[4]-'0' );
				//if ( h > 23 || m > 59 || s >59 ) return;
				//hours = h ;
				//minutes = m ; 
				//seconds = s; 
			
			sscanf( buf ,"SET_TIME %d:%d:%d",settime,settime+1,settime+2);
			
			RXtimerFlag = 1;
		}
		if ( ( len >= 5 ) && (strncmp( buf , "RESET" , 5 ) == 0) )
		{
			for (int i = 0 ; i < 2 ; i++){
				   settime[i] = 0;
			}
			RXtimerFlag = 1;
			__set_FAULTMASK(1);
			NVIC_SystemReset();
		
		}
}


uint8_t g_uart_rx_buf[32] ;
//////////////////////////////////////////////////////////////////
void System_Init( void )
{

	printf("Start all UARTS DMA receive ...\n");

	//UartTxDataDMA(1, "Start all UARTS DMA receive ...\n" , strlen("Start all UARTS DMA receive ...\n"));	
	HAL_TIM_Base_Start_IT( &htim1 );
	HAL_TIM_Base_Start_IT( &htim3 );
	StartAllUartDMAReceive();
  //UartTxDataDMA(1, "Start all UARTS DMA receive !...\n" , strlen("Start all UARTS DMA receive ...\n"));	
	//HAL_UART_Transmit_IT( g_user_uart , "Start all UARTS IT receive 1...\n" , strlen("Start all UARTS DMA receive ...\n"));
}


void UserTasks( void)
{	
//	stop_watch_process( );		
//	stop_watch_show( );
//	music_process();
	
	RXtimer_process();
	
	timer_process();
	
	timer_show();

	CheckUartRxData();

	CheckUartTxData();

	
	
//	CheckUSBRxData();
//	CheckUSBTxData();
	
}
