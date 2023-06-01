#include <stdio.h>
#include "main.h"
#include "tim.h"
uint8_t hours = 0, minutes = 0  , seconds = 0 ;

extern uint8_t  g_music_enable ;
extern uint8_t g_stopwatch_flag ;
extern const uint8_t  g_seg7_coder[11] ;
extern const uint32_t g_seg7_enable[6] ;




uint8_t  g_timer_showbuf[6] = { 0,0,0,0,0,0 };





void timer_process( void )
{
	static uint16_t i = 0  ;
	
	if ( g_stopwatch_flag == 1 )
	{
		i ++;
		if ( i == 1000 ) 
		{
			//printf("The Time Is %d:%d:%d\n",hours,minutes,seconds);
			if ( seconds == 59 )
			{
				seconds = 0 ;
				if ( minutes == 59 )
				{
					minutes = 0 ;
					if ( hours == 23 )
					{
						hours = 0 ;
					}
					else
						hours ++ ;
					g_timer_showbuf[4] = (hours % 10) ;
					g_timer_showbuf[5] = (hours / 10) ;
				}
				else
					minutes ++ ;
				g_timer_showbuf[2] = (minutes % 10) ;
				g_timer_showbuf[3] = (minutes / 10) ;

			}
			else
			 seconds ++ ;
			g_timer_showbuf[0] = (seconds % 10) ;
			g_timer_showbuf[1] = (seconds / 10) ;
			i = 0 ;
			
			if ( seconds < 10 && minutes == 0 ) g_music_enable = 1 ;
			else g_music_enable = 0 ;
		}
		g_stopwatch_flag = 0 ;
		
	}
}

void seg7_enable( uint8_t en );
void seg7_show( uint8_t show , uint8_t point );

void timer_show( )
{
	uint8_t i ;
	uint8_t show_char ;
	
	for ( i = 0 ; i < 6 ; i++ )
	{
		//seg7_enable( g_seg7_enable[i] );
		show_char = g_seg7_coder[ g_timer_showbuf[i] ] ;

		seg7_enable( 0 );
		seg7_show( show_char , 0x00 );
		seg7_enable( g_seg7_enable[5-i] );
	}
}
