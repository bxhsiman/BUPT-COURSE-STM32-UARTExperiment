#include "usbd_cdc_if.h"
#include "usb_uart.h"
#include "uart_dma.h"

extern USBD_HandleTypeDef hUsbDeviceFS;

usb_buf_t g_usb_buf;

/*�����������й��ɻ�����*/
void Usb2UartInit(void) {
    g_usb_buf.rx_buf_full = 0; //rx��������ָʾ
    g_usb_buf.rx_buf_tail = 0; //rx��������β
    g_usb_buf.rx_buf_head = 0; //rx��������ͷ
    g_usb_buf.tx_buf_full = 0; //tx��������ָʾ
    g_usb_buf.tx_buf_tail = 0; //tx�����βָʾ
    g_usb_buf.tx_buf_head = 0; //tx�����ͷָʾ
    g_usb_buf.tx_busy = 0;     //txæָʾ

    USBD_CDC_SetRxBuffer(&hUsbDeviceFS, (uint8_t *) g_usb_buf.usb_rx_buf[g_usb_buf.rx_buf_tail]); //CDC��ȡBuffer ��ʼ��
    USBD_CDC_ReceivePacket(&hUsbDeviceFS);                                                              //CDC��ȡ��ʼ��
}
/*ָ�����ȶ�ȡ*/
void RcvDataFromUSB(uint8_t *Buf, uint32_t Len) { //��ָ�����Ƚ���

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
/*USBTx��������������*/
void USBTxDataDMA(uint8_t *buf, uint32_t len) {
    /*�����д�����*/
    memcpy(g_usb_buf.usb_tx_buf[g_usb_buf.tx_buf_tail], buf, len); ////��buffer���뻺�����ж�β�����
    g_usb_buf.tx_buf_size[g_usb_buf.tx_buf_tail] = len;            //���ÿ����ݳ��ȼ�¼����
    /*����������Ӳ���(������)*/
    if (g_usb_buf.tx_buf_tail == (MAX_USB_BUF_NUM - 1))            //�����β�������ֵ ��ѭ����ͷ��д
        g_usb_buf.tx_buf_tail = 0;
    else
        g_usb_buf.tx_buf_tail++;                                   //����β�ŵ���һ����

    if (g_usb_buf.tx_buf_tail == g_usb_buf.tx_buf_head) {          //�����β�Ͷ�ͷ��� ��˵����������
        g_usb_buf.tx_buf_full = 1;
    }
}
/*USBRx���������Ѻ���*/
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
/*USBRx���������Ѻ���*/
void CheckUSBRxData(void) {

    while (g_usb_buf.rx_buf_full == 1 ||
           g_usb_buf.rx_buf_head != g_usb_buf.rx_buf_tail) {
        UartTxDataDMA(1,
                      g_usb_buf.usb_rx_buf[g_usb_buf.rx_buf_head],
                       g_usb_buf.rx_buf_size[g_usb_buf.rx_buf_head]
                       ); //ֱ�ӽ�USB�������ŵ�uart������
        /*�������г��Ӳ���(����)*/
        if (g_usb_buf.rx_buf_head == (MAX_USB_BUF_NUM - 1))
            g_usb_buf.rx_buf_head = 0;
        else
            g_usb_buf.rx_buf_head++;
        g_usb_buf.rx_buf_full = 0;
    }
}

/*USBTx���������Ѻ���*/
void CheckUSBTxData(void) {

    //static uint8_t rx_buf[ UART_RX_BUF_SIZE + 1 ] ;
    uint8_t *tx_buf;
    int32_t len;

    //if ( g_usb_buf.tx_busy == 1 ) return ;
    if (g_usb_buf.tx_buf_full == 1) {                       //ָʾbuffer����
        printf("Uart Tx overflow!\n");
    }
    /*�������г��Ӳ���(����)*/
    if (g_usb_buf.tx_buf_full == 1 ||
        g_usb_buf.tx_buf_head != g_usb_buf.tx_buf_tail) {    //������� ���߻������зǿ�

        len = g_usb_buf.tx_buf_size[g_usb_buf.tx_buf_head];  //ȡ����Ӧ��ͷָ���볤��
        tx_buf = g_usb_buf.usb_tx_buf[g_usb_buf.tx_buf_head];
        SendDataToUSB(tx_buf, len);                 //��������
        //g_usb_buf.tx_busy = 1 ;
        if (g_usb_buf.tx_buf_head == (MAX_USB_BUF_NUM - 1)) //��ͷ������β�� ���ͷ��ʼ����
            g_usb_buf.tx_buf_head = 0;
        else
            g_usb_buf.tx_buf_head++;                        //��ʼ������һ��buffer��
        g_usb_buf.tx_buf_full = 0;                          //����ÿ�����־
    }

}

