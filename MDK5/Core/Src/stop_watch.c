#include "main.h"
#include "tim.h"

extern uint8_t g_music_flag ;

__IO uint8_t g_stopwatch_flag = 0 ;
__IO uint32_t g_stopwatch_counter = 0 ;
__IO uint8_t  g_stopwatch_showbuf[6] = { 0,0,0,0,0,0 };

const uint8_t  g_seg7_coder[11] = { 0xFC, 0x60, 0xDA, 0xF2, 0x66, 0xB6, 0xBE, 0xE0, 0xFE, 0xF6 };
const uint32_t g_seg7_weight[6] = { 100000, 10000, 1000, 100, 10, 1 };
const uint32_t g_seg7_enable[6] = { 0x20, 0x10, 0x08, 0x04, 0x02, 0x01 };

void stop_watch_process( void )
{
	uint8_t i ;
	uint32_t counter ;
	if ( g_stopwatch_flag == 1 )
	{
		g_stopwatch_counter ++;
		if ( g_stopwatch_counter > 999999 ) g_stopwatch_counter = 0 ;
		counter = g_stopwatch_counter ;
		for ( i = 0 ; i < 6 ; i++ )
		{
			g_stopwatch_showbuf[i] = counter / g_seg7_weight[i] ;
			counter = counter % g_seg7_weight[i] ;
		}
		g_stopwatch_flag = 0 ;
	}
}

void seg7_enable( uint8_t en )
{
	uint8_t i ;
	GPIO_PinState state = GPIO_PIN_RESET ;
	
	for ( i = 0 ; i < 6 ; i ++ )
	{
		if ( en & 0x20 ) state = GPIO_PIN_RESET ;
		else state = GPIO_PIN_SET ;
		en = en << 1 ;
		
		switch (i)
		{
			case 0 :
				HAL_GPIO_WritePin( SEG_EN0_GPIO_Port , SEG_EN0_Pin , state);
				break ;
			case 1 :
				HAL_GPIO_WritePin( SEG_EN1_GPIO_Port , SEG_EN1_Pin , state);
				break ;
			case 2 :
				HAL_GPIO_WritePin( SEG_EN2_GPIO_Port , SEG_EN2_Pin , state);
				break ;
			case 3 :
				HAL_GPIO_WritePin( SEG_EN3_GPIO_Port , SEG_EN3_Pin , state);
				break ;
			case 4 :
				HAL_GPIO_WritePin( SEG_EN4_GPIO_Port , SEG_EN4_Pin , state);
				break ;
			case 5 :
				HAL_GPIO_WritePin( SEG_EN5_GPIO_Port , SEG_EN5_Pin , state);
				break ;
		}
	}
}

void seg7_show( uint8_t show , uint8_t point )
{
	uint8_t i ;
	GPIO_PinState state = GPIO_PIN_RESET ;
	
	show = show | point ;
	
	for ( i = 0 ; i < 8 ; i ++ )
	{
		if ( show & 0x80 ) state = GPIO_PIN_RESET ;
		else state = GPIO_PIN_SET ;
		show = show << 1 ;
		
		switch (i)
		{
			case 0 :
				HAL_GPIO_WritePin( SEG_A_GPIO_Port , SEG_A_Pin , state);
				break ;
			case 1 :
				HAL_GPIO_WritePin( SEG_B_GPIO_Port , SEG_B_Pin , state);
				break ;
			case 2 :
				HAL_GPIO_WritePin( SEG_C_GPIO_Port , SEG_C_Pin , state);
				break ;
			case 3 :
				HAL_GPIO_WritePin( SEG_D_GPIO_Port , SEG_D_Pin , state);
				break ;
			case 4 :
				HAL_GPIO_WritePin( SEG_E_GPIO_Port , SEG_E_Pin , state);
				break ;
			case 5 :
				HAL_GPIO_WritePin( SEG_F_GPIO_Port , SEG_F_Pin , state);
				break ;
			case 6 :
				HAL_GPIO_WritePin( SEG_G_GPIO_Port , SEG_G_Pin , state);
				break ;
			case 7 :
				HAL_GPIO_WritePin( SEG_P_GPIO_Port , SEG_P_Pin , state);
				break ;
		}
	}
}
	
void stop_watch_show( )
{
	uint8_t i ;
	uint8_t show_char ;
	uint8_t show_zero = 0 ;
	
	for ( i = 0 ; i < 6 ; i++ )
	{
		//seg7_enable( g_seg7_enable[i] );
		show_char = g_seg7_coder[ g_stopwatch_showbuf[i] ] ;
		if ( i > 1 || g_stopwatch_showbuf[i] > 0 ) show_zero = 1 ;
		
		if ( g_stopwatch_showbuf[i] == 0 && show_zero == 0 )
		{
			show_char = 0 ;
		}
		
		seg7_enable( 0 );
		if ( i == 2 ) seg7_show( show_char , 0x01 );
		else seg7_show( show_char , 0x00 );
		seg7_enable( g_seg7_enable[i] );
	}
}

