#include <string.h>
#include <stdio.h>
#include "usart.h"
#include "uart_dma.h"
// uart7 wifi uart0
// uart4 wifi uart1
// uart3 user uart

/*声明回调*/
void Uart1_RxDataCallback( uint8_t * buf , uint32_t len );
void Uart1_RxDataCallback_ToUSB( uint8_t * buf , uint32_t len );

extern DMA_HandleTypeDef hdma_usart1_rx;
extern DMA_HandleTypeDef hdma_usart1_tx;
//extern DMA_HandleTypeDef hdma_uart7_rx;

uart_dma_t g_uart_dma[ MAX_UART_DMA_NUM ] ;                 //创建dma缓冲区

uint8_t g_active_uart_num = 0 ;

const uart_map_t g_uart_port_map[ MAX_UART_PORT_NUM ] = {  //创建UART接口表
	{1,&huart1,&hdma_usart1_rx,&hdma_usart1_tx, &Uart1_RxDataCallback_ToUSB , 0} ,
	{0,0,0,0,0,0} ,
	{0,0,0,0,0,0} ,
	{0,0,0,0,0,0} ,
	{0,0,0,0,0,0} ,
	{0,0,0,0,0,0} ,
	{0,0,0,0,0,0} ,
	{0,0,0,0,0,0} ,
};

void HAL_UART_EndRxTransfer_IT(UART_HandleTypeDef *huart)
{
  /* Disable RXNE, PE and ERR (Frame error, noise error, overrun error) interrupts */
  CLEAR_BIT(huart->Instance->CR1, (USART_CR1_RXNEIE | USART_CR1_PEIE));
  CLEAR_BIT(huart->Instance->CR3, USART_CR3_EIE);

  /* At end of Rx process, restore huart->RxState to Ready */
  huart->RxState = HAL_UART_STATE_READY;
}

void Uart_DMA_Init( const uart_map_t * uart_map){ // 初始化UART DMA
	
	uint32_t i ;
	 
	i = uart_map->index - 1 ;                    //index是从1开始的
	
	if ( i >= MAX_UART_DMA_NUM ) return ;        //index超出uartr dma最大数量
	
	HAL_UART_Init( uart_map->huart ) ;           //uart初始化
    /*dma初始化区 将uartmap中配置映射到dma*/
	g_uart_dma[ i ].huart       = uart_map->huart ;
	g_uart_dma[ i ].hdma_rx     = uart_map->hdma_rx ;	
	g_uart_dma[ i ].rx_buf_head = 0 ;
	g_uart_dma[ i ].rx_buf_tail = 0 ;
	g_uart_dma[ i ].rx_buf_full = 0 ;
	g_uart_dma[ i ].RxCallback = uart_map->RxCallback ;
	
	g_uart_dma[ i ].hdma_tx     = uart_map->hdma_tx ;	
	g_uart_dma[ i ].tx_buf_head = 0 ;
	g_uart_dma[ i ].tx_buf_tail = 0 ;
	g_uart_dma[ i ].tx_buf_full = 0 ;
	g_uart_dma[ i ].tx_busy = 0 ;
	g_uart_dma[ i ].TxCallback = uart_map->TxCallback ;
	
	
	__HAL_UART_ENABLE_IT( uart_map->huart , UART_IT_IDLE);    //使能IDLE中断
	if ( uart_map->hdma_rx != NULL )                          //使能DMA接收
	{
		HAL_UART_Receive_DMA( uart_map->huart , 
													( uint8_t *)g_uart_dma[ i ].uart_rx_buf[ 0 ] ,
													UART_RX_BUF_SIZE );
	}
	else                                                     //使能能中断接收
	{
		HAL_UART_Receive_IT( uart_map->huart , 
													( uint8_t *)g_uart_dma[ i ].uart_rx_buf[ 0 ] ,
													UART_RX_BUF_SIZE );
	}
}

void Uart_RxIDLE_Handler( int32_t port )
{
	int32_t i , temp , len ;
	
	i = g_uart_port_map[port-1].index ;
	
	if ( i == 0 ) return ;
	i = i - 1 ;
	
	if( __HAL_UART_GET_FLAG( g_uart_dma[ i ].huart , UART_FLAG_IDLE) != RESET )
	{
		__HAL_UART_CLEAR_IDLEFLAG( g_uart_dma[ i ].huart );
		__HAL_UART_CLEAR_PEFLAG( g_uart_dma[ i ].huart );

//		temp = g_uart_dma[ i ].huart->Instance->SR;  
//		temp = g_uart_dma[ i ].huart->Instance->DR;
		if ( g_uart_dma[ i ].hdma_rx != NULL )
		{
			HAL_UART_DMAStop( g_uart_dma[ i ].huart );
			temp  = __HAL_DMA_GET_COUNTER( g_uart_dma[ i ].hdma_rx ) ;
			len = UART_RX_BUF_SIZE - temp ;
			if ( len > 0 )
			{
				g_uart_dma[ i ].rx_buf_size[ g_uart_dma[ i ].rx_buf_tail ] =  len;                           
				if ( g_uart_dma[ i ].rx_buf_tail == ( MAX_UART_BUF_NUM - 1 ) )
					g_uart_dma[ i ].rx_buf_tail = 0 ;
				else
					g_uart_dma[ i ].rx_buf_tail ++ ;
			}

			HAL_UART_Receive_DMA( g_uart_dma[ i ].huart , 
														( uint8_t *)g_uart_dma[ i ].uart_rx_buf[ g_uart_dma[ i ].rx_buf_tail ] ,
														UART_RX_BUF_SIZE );
		}
		else
		{
      HAL_UART_EndRxTransfer_IT( g_uart_dma[ i ].huart );			
			len = g_uart_dma[ i ].huart->RxXferCount ;
			if ( len > 0 )
			{
				g_uart_dma[ i ].rx_buf_size[ g_uart_dma[ i ].rx_buf_tail ] =  len ;
				if ( g_uart_dma[ i ].rx_buf_tail == ( MAX_UART_BUF_NUM - 1 ) )
					g_uart_dma[ i ].rx_buf_tail = 0 ;
				else
					g_uart_dma[ i ].rx_buf_tail ++ ;
			}

			HAL_UART_Receive_IT( g_uart_dma[ i ].huart , 
														( uint8_t *)g_uart_dma[ i ].uart_rx_buf[ g_uart_dma[ i ].rx_buf_tail ] ,
														UART_RX_BUF_SIZE );
			
		}
			
		if ( len >0 && g_uart_dma[ i ].rx_buf_tail == g_uart_dma[ i ].rx_buf_head ) 
		{
			g_uart_dma[ i ].rx_buf_full = 1 ;
		}
	}	
}

void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart)
{
	
//	if(huart->Instance == USART1) {	
//		i = 0 ;
//	} else if(huart->Instance == USART2) {	
//		i = 1 ;
//	} else if(huart->Instance == USART3) {	
//		i = 2 ;
//	} else if(huart->Instance == UART4) {	
//		i = 3 ;
//	} else if(huart->Instance == UART5) {	
//		i = 4 ;
//	} else  
//	{	
//		return ;
//	}
//  sprintf( ( char *)err , "uart %d : error code:%X\r\n",i+1 ,huart->ErrorCode ); 
//  SysLog( err ) ;			
//  SendUartDataToUART( g_host_monitor_uart_map ,  err );

}


void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
  uint32_t port , i  ;

	if(huart->Instance == USART1) {	
		port = 1 ;
	} else if(huart->Instance == USART2) {	
		port = 2 ;
	} else if(huart->Instance == USART3) {	
		port = 3 ;
	} else
	{	
		return ;
	}
	
	i = g_uart_port_map[port-1].index ;
	
	if ( i == 0 ) return ;
	i = i - 1 ;

	g_uart_dma[ i ].rx_buf_size[ g_uart_dma[ i ].rx_buf_tail ] =  UART_RX_BUF_SIZE ;                           
	if ( g_uart_dma[ i ].rx_buf_tail == ( MAX_UART_BUF_NUM - 1 ) )
		g_uart_dma[ i ].rx_buf_tail = 0 ;
	else
		g_uart_dma[ i ].rx_buf_tail ++ ;

	if ( g_uart_dma[ i ].hdma_rx != NULL )
	{
		HAL_UART_Receive_DMA( g_uart_dma[ i ].huart , 
													( uint8_t *)g_uart_dma[ i ].uart_rx_buf[ g_uart_dma[ i ].rx_buf_tail ] ,
													UART_RX_BUF_SIZE );
	}
	else
	{
		HAL_UART_Receive_IT( g_uart_dma[ i ].huart , 
													( uint8_t *)g_uart_dma[ i ].uart_rx_buf[ g_uart_dma[ i ].rx_buf_tail ] ,
													UART_RX_BUF_SIZE );
	}

	if ( g_uart_dma[ i ].rx_buf_tail == g_uart_dma[ i ].rx_buf_head ) 
	{
		g_uart_dma[ i ].rx_buf_full = 1 ;
	}

}

/*
void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
  uint32_t port , i  ;

	if(huart->Instance == USART1) {	
		port = 1 ;
	} else if(huart->Instance == USART2) {	
		port = 2 ;
	} else if(huart->Instance == USART3) {	
		port = 3 ;
	} else 
	{	
		return ;
	}
	
	i = g_uart_port_map[port - 1].index ;
	
	if ( i == 0 ) return ;
	i = i - 1 ;

	if ( g_uart_dma[ i ].tx_buf_head == ( MAX_UART_BUF_NUM - 1 ) )
		g_uart_dma[ i ].tx_buf_head = 0 ;
	else
		g_uart_dma[ i ].tx_buf_head ++ ;
	g_uart_dma[ i ].tx_buf_full = 0 ;
  g_uart_dma[ i ].tx_busy = 0 ;
}
*/

/*UART Tx DMA缓冲区生产函数*/
void UartTxDataDMA( uint32_t port , uint8_t * buf , uint32_t len ) //
{
	uint32_t i ;
    /*参数合法性检查*/
	if ( len > UART_TX_BUF_SIZE ) return ;
	if ( port > MAX_UART_PORT_NUM || port == 0 ) return ;

    /*索引规范化*/
	i = g_uart_port_map[port-1].index ;	
	if ( i == 0 ) return ;
	i = i - 1 ;

    /*DMA 缓冲块写入操作*/
	memcpy( g_uart_dma[ i ].uart_tx_buf[ g_uart_dma[ i ].tx_buf_tail ] , buf , len ); //将buffer放入缓冲块队列队尾缓冲块
	g_uart_dma[ i ].tx_buf_size[ g_uart_dma[ i ].tx_buf_tail ] =  len ;               //将该块数据长度记录到表
	//printf("dma: %d head %d tail %d \n",i,g_uart_dma[ i ].tx_buf_head, g_uart_dma[ i ].tx_buf_tail);
	/*缓冲块队列操作*/
    if ( g_uart_dma[ i ].tx_buf_tail == ( MAX_UART_BUF_NUM - 1 ) )                    //检查是否到达缓冲区尾 进行循环覆写
		g_uart_dma[ i ].tx_buf_tail = 0 ;
	else
		g_uart_dma[ i ].tx_buf_tail ++ ;                                              //队尾移动
	
	if ( g_uart_dma[ i ].tx_buf_tail == g_uart_dma[ i ].tx_buf_head ) 
	{
		g_uart_dma[ i ].tx_buf_full = 1 ;                                            //如果队尾和队头相等 则说明缓冲队列满了
	}
}

/*UART DMA 启动函数*/
void StartAllUartDMAReceive( )
{
	uint32_t i ;
	for ( i = 0 ; i < MAX_UART_PORT_NUM ; i++ )
	{
		if ( g_uart_port_map[i].index > 0 		)    		 
		{
			g_active_uart_num ++ ;
			Uart_DMA_Init( g_uart_port_map+i ); 
		}
	}
}

void StopAllUart( void )
{
	uint32_t i , index ;
	for ( i = 0 ; i < MAX_UART_PORT_NUM ; i++ )
	{
		if ( g_uart_port_map[i].index > 0 ) 
		{
			index	= g_uart_port_map[i].index - 1 ;
			if ( g_uart_dma[ index ].hdma_rx != NULL )
			{
	      	HAL_UART_DMAStop( g_uart_dma[ index ].huart );
			}
			else
			{
          HAL_UART_EndRxTransfer_IT( g_uart_dma[ index ].huart );			
			}
    	__HAL_UART_DISABLE_IT( g_uart_dma[ index ].huart , UART_IT_IDLE);    //ʹ�ܿ������ж�
			HAL_UART_DeInit( g_uart_dma[ index ].huart ) ;
		}
	}
}

__weak void UartRxDataCallback( uint8_t * buf , uint32_t len )
{
	UNUSED(buf);
	UNUSED(len);
}
/*UART Rx DMA缓冲区消费函数*/
void CheckUartRxData( void )
{
	//static uint8_t rx_buf[ UART_RX_BUF_SIZE + 1 ] ;
    uint8_t * rx_buf ;
	int32_t len ;
	
	int32_t i ;
 	for ( i = 0 ; i < g_active_uart_num ; i++ )
	{
		if (  g_uart_dma[ i ].rx_buf_full == 1 )
		{
			printf("Uart Rx overflow!\n");
		}
		while ( g_uart_dma[ i ].rx_buf_full == 1 || 
				g_uart_dma[ i ].rx_buf_head != g_uart_dma[ i ].rx_buf_tail  )
		{				
			len = g_uart_dma[ i ].rx_buf_size[ g_uart_dma[ i ].rx_buf_head ] ;			
			rx_buf = g_uart_dma[ i ].uart_rx_buf[ g_uart_dma[ i ].rx_buf_head ] ;
			g_uart_dma[i].RxCallback( rx_buf , len );
			if ( g_uart_dma[ i ].rx_buf_head == ( MAX_UART_BUF_NUM - 1 ) )
				g_uart_dma[ i ].rx_buf_head = 0 ;
			else
				g_uart_dma[ i ].rx_buf_head ++ ;
			g_uart_dma[ i ].rx_buf_full = 0 ;
			
		}			
	}

}
/*UART Tx DMA缓冲区消费函数*/
void CheckUartTxData( void )
{
    //static uint8_t rx_buf[ UART_RX_BUF_SIZE + 1 ] ;
    uint8_t * tx_buf ;
    int32_t len ;
    int32_t i ;

 	for ( i = 0 ; i < g_active_uart_num ; i++ )
	{
		// if ( g_uart_dma[ i ].tx_busy == 1 ) continue; //检查发送是否忙
		if ( g_uart_dma[ i ].tx_buf_full == 1 )         //检查Tx缓冲区是否满
		{
			printf("Uart Tx overflow!\n");
		}
        /*缓冲块队列出队操作(消费)*/
		if ( g_uart_dma[ i ].tx_buf_full == 1 || 
				g_uart_dma[ i ].tx_buf_head != g_uart_dma[ i ].tx_buf_tail  ) //块队列满或块队列不空
		{

			len = g_uart_dma[ i ].tx_buf_size[ g_uart_dma[ i ].tx_buf_head ] ; //取出对应块头指针与长度
			tx_buf = g_uart_dma[ i ].uart_tx_buf[ g_uart_dma[ i ].tx_buf_head ] ;
			HAL_UART_Transmit_DMA(g_uart_dma[ i ].huart , tx_buf , len );//发送数据
			//g_uart_dma[ i ].tx_busy = 1 ;
			if ( g_uart_dma[ i ].tx_buf_head == ( MAX_UART_BUF_NUM - 1 ) ) //当头到队列尾部 则从头开始处理
				g_uart_dma[ i ].tx_buf_head = 0 ;
			else
				g_uart_dma[ i ].tx_buf_head ++ ;                           //开始处理下一个buffer块
			g_uart_dma[ i ].tx_buf_full = 0;                               //清除该块满标志
		}			
	}

}

