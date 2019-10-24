#ifndef __ZBAR_USER_H
#define	__ZBAR_USER_H

#include "zbar.h"
#include <stdio.h>
#include "./usart/bsp_usart.h"
#include "./lcd/bsp_lcd.h"

//使能串口发送二维码识别信息
#define QRC_UART_SEND 1
//使能二维码识别功能
#define IMG_QR_DETECT 1

#define  IMG_SDRAM_ADDR  LCD_FRAME_BUFFER+0x232800  //前面为显存
/*液晶屏的分辨率，用来计算地址偏移*/
#define lcd_width   800
#define lcd_height  480

/*摄像头采集图像的大小*/
#define img_width   800
#define img_height  480


extern unsigned char img_bin[img_width * img_height];
extern uint8_t qr_recongized;

void QR_decoder(void);
void DCMI_VS_IRQ(uint32_t src_addr,uint32_t size);
#endif /* __ZBAR_USER_H */
