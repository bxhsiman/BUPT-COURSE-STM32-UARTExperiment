# 串口实验报告

##  引入：
串口常规的阻塞式IO让单片机性能浪费在长时间的数据等待之中。

而当引入DMA管理数据收发时，数据如何在DMA中存取成了我们要考虑的主要问题，本文将对一份串口通信的示例代码进行分析，了解其是如何使用一个队列对收发数据进行管理的。

## 为什么是队列：
队列是一个FIFO(先进先出)的数据结构，非常适合传输数据这种异步的生产者消费者模型。

任意“生产任务”产生数据后，直接放入队列队尾，主循环中“消费任务”会从队头开始依次将消息发送，从而使产生数据与发送数据任务剥离，让任务粒度变细，充分利用单片机计算时间。

本文涉及的队列是一个二维数组作为存储结构的循环队列,队满条件是单独存储的，充分使用空间（（。

## 如何构建队列
参考本工程的文件 `uart_dma.h` 关于DMA的相关定义
``` c
typedef struct 	
{
	UART_HandleTypeDef 	* huart ; 
	DMA_HandleTypeDef 	* hdma_rx ;
	DMA_HandleTypeDef 	* hdma_tx ;
	
	uint8_t uart_rx_buf[ MAX_UART_BUF_NUM ][ UART_RX_BUF_SIZE + 1 ] ; //读取缓冲
	uint32_t  rx_buf_size[ MAX_UART_BUF_NUM ] ;
	__IO uint32_t  rx_buf_head ;
	__IO uint32_t  rx_buf_tail ;
	__IO uint32_t  rx_buf_full ;	
	void (* RxCallback)( uint8_t * buf , uint32_t len );   
	
	uint8_t uart_tx_buf[ MAX_UART_BUF_NUM ][ UART_RX_BUF_SIZE + 1 ] ; //发送缓冲
	uint32_t  tx_buf_size[ MAX_UART_BUF_NUM ] ;
	__IO uint32_t  tx_buf_head ;
	__IO uint32_t  tx_buf_tail ;
	__IO uint32_t  tx_buf_full ;	
	__IO uint32_t  tx_busy     ;	
	void (* TxCallback)( char * buf , uint32_t len );   
} uart_dma_t;

```
我们不难发现其中定义了两个队列，这里使用`发送缓冲`队列进行分析
``` 
uint8_t uart_tx_buf[ MAX_UART_BUF_NUM ][ UART_RX_BUF_SIZE + 1 ] ;
```
这是一个二维数组，但是后面我们还有三维的，为了统一风格，我们用json的风格来描述一下
``` json
uart_tx_buf: {
    "缓冲块1" : {"内容1"},
    "缓冲块2" : {"内容2"}, 
    "缓冲块3" : {"内容3"},
    ... 
}
```

可以看到缓冲区被分成了 `MAX_UART_BUF_NUM` 块 每一块的长度为 `UART_RX_BUF_SIZE + 1` 为什么要+1呢，我认为是要容纳`\0`。
                                                                                                                        
*后面了解到这是古早写法，目前的代码无需这个+1*

## 队列的初始化
还是先来看代码 `uart_dma.h`
``` c
uart_dma_t g_uart_dma[ MAX_UART_DMA_NUM ] ;                 //创建dma缓冲区
const uart_map_t g_uart_port_map[ MAX_UART_PORT_NUM ] = {   //创建UART设备表
	{1,&huart1,&hdma_usart1_rx,&hdma_usart1_tx, &Uart1_RxDataCallback_ToUSB , 0} ,
	{0,0,0,0,0,0} ,
	{0,0,0,0,0,0} ,
	{0,0,0,0,0,0} ,
	{0,0,0,0,0,0} ,
	{0,0,0,0,0,0} ,
	{0,0,0,0,0,0} ,
	{0,0,0,0,0,0} ,
};
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
		HAL_UART_Receive_DMA( ... )
	}
	else                                                     //使能能中断接收
	{
		HAL_UART_Receive_IT(... );
	}
}
```
这里的`Uart_DMA_Init`主要操作的是`g_uart_dma`这个三维数组,将`uart_map`当中每一个UART设备的配置标准化地映射到DMA的内容当中，后续我们使用只要多重遍历`g_uart_dma`这个数组就可以对每一个UART设备进行操作了！

之所以采取这种结构，还是因为遍历的进度可以交给一个变量存储，可以将一次完整的操作均匀散开，模拟异步操作！这样init之后的`g_uart_dma`大概长这样：
``` json
    g_uart_dma: {
        "设备1" : {
            ...
            "队列头" : 0,
            "队列尾" : 0,
            "队列满标志" : 0,
            "回调" : "0xFF222"
            ...
        },
        "设备2" : {
            ...
            "队列头" : 0,
            "队列尾" : 0,
            "队列满标志" : 0,
            "回调" : "0xFF8i3"
            ...
        }
        ...
    }
```

## 如何入队(生产)
这段代码参考 `uart_dma.c` 当中的 `UartTxDataDMA`
``` c
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
	/*缓冲块队列操作*/
    if ( g_uart_dma[ i ].tx_buf_tail == ( MAX_UART_BUF_NUM - 1 ) )                    //检查是否到达缓冲区尾 进行循环
		g_uart_dma[ i ].tx_buf_tail = 0 ;
	else
		g_uart_dma[ i ].tx_buf_tail ++ ;                                              //队尾移动
	
	if ( g_uart_dma[ i ].tx_buf_tail == g_uart_dma[ i ].tx_buf_head ) 
	{
		g_uart_dma[ i ].tx_buf_full = 1 ;                                            //如果队尾和队头相等 则说明缓冲队列满了
	}
}
```
感觉代码可读性还是很高的，一点都不抽象 XD

大概就是将数据放到指定的DMA的缓冲队列的最后，进行一个入队操作
``` json
  g_uart_dma: {
        "设备1" : {
            ...
            "队列头" : 0,          
            "队列尾" : 0,        <- 把这里后移 然后判断是否需要循环
            "队列满标志" : 0,    <- 如果满了 就把这个也标记一下
            "回调" : "0xFF222"
            ...
        }
  }
```
队尾有三个操作，后移(++)，循环(=0)，队满(追上队头 置标志位)
但是这样有个问题，如果缓冲区溢出，队尾超过了队头就直接丢失一整个缓冲队列的数据了，可以在`memcpy`之前进行判满

这里就涉及了如何设计流控的问题，应当找到最慢的步骤，在其拥塞的时候向其消费者与生产者共同发出流控信号。

## 如何出队(消费)
这段代码参考 `uart_dma.c` 当中的 `CheckUartTxData`
``` c
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
```
大概就是每次队头指向的DMA缓冲块的数据交给DMA
``` json
  g_uart_dma: {
        "设备1" : {
            ...
            "队列头" : 0,         <- 先取当前位置，然后获取位置对应的数据 随后后移 判断是否需要循环  
            "队列尾" : 0,       
            "队列满标志" : 0,     <- 操作完了之后清零
            "回调" : "0xFF222"   <- 可以在合适的时候调用这里的回调处理一些东西
            ...
        }
  }
```

队头有两个操作，后移(++)，循环(=0)

队空是用头=尾且块满标志为0判断的，操作完了之后需要清除块满标志

## 总结
本例工程使用循环队列进行数据缓冲实现了USB与UART的数据透传,效果还是不错的，算是将数据结构课程给在单片机上学以致用了吧~

单片机编程总是在想办法模拟异步，将一个任务尽量的拆细，以防止阻塞掉一些无法抢断别人的中断。也会想着使用一些其他芯片/电路去简化一些复杂耗时的操作。

在本次场景当中没有设计流控，大量数据从USB发往串口时会出现丢包，可以设计乒乓结构来中止/启动接收，而对于流控的触发，要在缓冲区满后立刻向生产消费者均发出流控信号，避免数据的丢失。
