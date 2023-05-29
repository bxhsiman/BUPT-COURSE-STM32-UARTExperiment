#ifndef __uart_dma_H
#define __uart_dma_H
#ifdef __cplusplus
 extern "C" {
#endif
#include "main.h"

#define UART_RX_BUF_SIZE    64
#define UART_TX_BUF_SIZE    64
	 
#define MAX_UART_PORT_NUM   8	 
#define MAX_UART_DMA_NUM    3	 
	 
#define MAX_UART_BUF_NUM    4	 
	 
typedef struct 	
{
	UART_HandleTypeDef 	* huart ;
	DMA_HandleTypeDef 	* hdma_rx ;
	DMA_HandleTypeDef 	* hdma_tx ;
	
	uint8_t uart_rx_buf[ MAX_UART_BUF_NUM ][ UART_RX_BUF_SIZE + 1 ] ;
	uint32_t  rx_buf_size[ MAX_UART_BUF_NUM ] ;
	__IO uint32_t  rx_buf_head ;
	__IO uint32_t  rx_buf_tail ;
	__IO uint32_t  rx_buf_full ;	
	void (* RxCallback)( uint8_t * buf , uint32_t len );   
	
	uint8_t uart_tx_buf[ MAX_UART_BUF_NUM ][ UART_RX_BUF_SIZE + 1 ] ;
	uint32_t  tx_buf_size[ MAX_UART_BUF_NUM ] ;
	__IO uint32_t  tx_buf_head ;
	__IO uint32_t  tx_buf_tail ;
	__IO uint32_t  tx_buf_full ;	
	__IO uint32_t  tx_busy     ;	
	void (* TxCallback)( char * buf , uint32_t len );   
}uart_dma_t;

typedef struct 	
{
	uint32_t  						index ;
	UART_HandleTypeDef 	* huart ;
	DMA_HandleTypeDef 	* hdma_rx ;
	DMA_HandleTypeDef 	* hdma_tx ;
	void (* RxCallback)( uint8_t * buf , uint32_t len ); 
	void (* TxCallback)( char * buf , uint32_t len );
}uart_map_t;

void Uart_DMA_Init( const uart_map_t * uart_map );
void Uart_RxIDLE_Handler( int32_t i );
void StartAllUartDMAReceive( void );
void StopAllUart( void );


void CheckUartRxData( void );
void CheckUartTxData( void );

void UartTxDataDMA( uint32_t port , uint8_t * pBuf , uint32_t len );
void UartRxDataCallback( uint8_t * buf , uint32_t len );
#ifdef __cplusplus
}
#endif
#endif /*__uart_dma_H */
