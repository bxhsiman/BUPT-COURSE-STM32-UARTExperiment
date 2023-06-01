#include "usbd_cdc_if.h"
#include "usb_uart.h"
#include "uart_dma.h"

extern USBD_HandleTypeDef hUsbDeviceFS;

usb_buf_t g_usb_buf;

/*创建缓冲块队列构成缓冲区*/
void Usb2UartInit(void) {
    g_usb_buf.rx_buf_full = 0; //rx缓冲区满指示
    g_usb_buf.rx_buf_tail = 0; //rx缓冲块队列尾
    g_usb_buf.rx_buf_head = 0; //rx缓冲块队列头
    g_usb_buf.tx_buf_full = 0; //tx缓冲区满指示
    g_usb_buf.tx_buf_tail = 0; //tx缓冲块尾指示
    g_usb_buf.tx_buf_head = 0; //tx缓冲块头指示
    g_usb_buf.tx_busy = 0;     //tx忙指示

    USBD_CDC_SetRxBuffer(&hUsbDeviceFS, (uint8_t *) g_usb_buf.usb_rx_buf[g_usb_buf.rx_buf_tail]); //CDC读取Buffer 初始化
    USBD_CDC_ReceivePacket(&hUsbDeviceFS);                                                              //CDC读取初始化
}
/*指定长度读取*/
void RcvDataFromUSB(uint8_t *Buf, uint32_t Len) { //按指定长度接收

    g_usb_buf.rx_buf_size[g_usb_buf.rx_buf_tail] = Len;

    if (g_usb_buf.rx_buf_tail == (MAX_USB_BUF_NUM - 1))
        g_usb_buf.rx_buf_tail = 0;
    else
        g_usb_buf.rx_buf_tail++;
    if (g_usb_buf.rx_buf_tail == g_usb_buf.rx_buf_head) {
        g_usb_buf.rx_buf_full = 1;
    }

    USBD_CDC_SetRxBuffer(&hUsbDeviceFS, (uint8_t *) g_usb_buf.usb_rx_buf[g_usb_buf.rx_buf_tail]);
    USBD_CDC_ReceivePacket(&hUsbDeviceFS);
}
/*USBTx缓冲区生产函数*/
void USBTxDataDMA(uint8_t *buf, uint32_t len) {
    /*缓冲块写入操作*/
    memcpy(g_usb_buf.usb_tx_buf[g_usb_buf.tx_buf_tail], buf, len); ////将buffer放入缓冲块队列队尾缓冲块
    g_usb_buf.tx_buf_size[g_usb_buf.tx_buf_tail] = len;            //将该块数据长度记录到表
    /*缓冲块队列入队操作(生产者)*/
    if (g_usb_buf.tx_buf_tail == (MAX_USB_BUF_NUM - 1))            //如果队尾到达最大值 则循环从头覆写
        g_usb_buf.tx_buf_tail = 0;
    else
        g_usb_buf.tx_buf_tail++;                                   //将队尾放到下一个块

    if (g_usb_buf.tx_buf_tail == g_usb_buf.tx_buf_head) {          //如果队尾和队头相等 则说明队列满了
        g_usb_buf.tx_buf_full = 1;
    }
}
/*USBRx缓冲区消费函数*/
void SendDataToUSB(uint8_t *Buf, uint32_t Len) {
    uint8_t result = USBD_OK;
    do {
        result = CDC_Transmit_FS(Buf, Len);
        if (result == USBD_FAIL) {
            break;
        } else if (result == USBD_BUSY) {
            break;
        }
    } while (result != USBD_OK);
    //HAL_Delay( 10 );
}
/*USBRx缓冲区消费函数*/
void CheckUSBRxData(void) {

    while (g_usb_buf.rx_buf_full == 1 ||
           g_usb_buf.rx_buf_head != g_usb_buf.rx_buf_tail) {
        UartTxDataDMA(1,
                      g_usb_buf.usb_rx_buf[g_usb_buf.rx_buf_head],
                       g_usb_buf.rx_buf_size[g_usb_buf.rx_buf_head]
                       ); //直接将USB缓冲区放到uart缓冲区
        /*缓冲块队列出队操作(消费)*/
        if (g_usb_buf.rx_buf_head == (MAX_USB_BUF_NUM - 1))
            g_usb_buf.rx_buf_head = 0;
        else
            g_usb_buf.rx_buf_head++;
        g_usb_buf.rx_buf_full = 0;
    }
}

/*USBTx缓冲区消费函数*/
void CheckUSBTxData(void) {

    //static uint8_t rx_buf[ UART_RX_BUF_SIZE + 1 ] ;
    uint8_t *tx_buf;
    int32_t len;

    //if ( g_usb_buf.tx_busy == 1 ) return ;
    if (g_usb_buf.tx_buf_full == 1) {                       //指示buffer满载
        printf("Uart Tx overflow!\n");
    }
    /*缓冲块队列出队操作(消费)*/
    if (g_usb_buf.tx_buf_full == 1 ||
        g_usb_buf.tx_buf_head != g_usb_buf.tx_buf_tail) {    //如果块满 或者缓冲块队列非空

        len = g_usb_buf.tx_buf_size[g_usb_buf.tx_buf_head];  //取出对应块头指针与长度
        tx_buf = g_usb_buf.usb_tx_buf[g_usb_buf.tx_buf_head];
        SendDataToUSB(tx_buf, len);                 //发送数据
        //g_usb_buf.tx_busy = 1 ;
        if (g_usb_buf.tx_buf_head == (MAX_USB_BUF_NUM - 1)) //当头到队列尾部 则从头开始处理
            g_usb_buf.tx_buf_head = 0;
        else
            g_usb_buf.tx_buf_head++;                        //开始处理下一个buffer块
        g_usb_buf.tx_buf_full = 0;                          //清除该块满标志
    }

}

