/**
  ******************************************************************************
  * @file    zbar_user.c
  * @author  fire
  * @version V1.0
  * @date    2016-xx-xx
  * @brief   ��ά��ʶ��
  ******************************************************************************
  * @attention
  *
  * ʵ��ƽ̨:����  STM32 F429 ������  
  * ��̳    :http://www.chuxue123.com
  * �Ա�    :http://firestm32.taobao.com
  *
  ******************************************************************************
  */

#include "zbar_user.h"

uint8_t qr_recongized = 0;
uint8_t qr_init_finish=1;
uint8_t data_ready;//��ȡ��ʶ��ͼ��״̬��1����ȡ�ɹ�  0��δ��ȡ

unsigned short  dcmi_buf[img_width * img_height] ;//__attribute((at(IMG_SDRAM_ADDR))) = {0};
unsigned char   img_now[img_width * img_height]  ;//__attribute((at(IMG_SDRAM_ADDR +0x100000)));
unsigned char   img_bin[img_width * img_height]  ;//__attribute((at(IMG_SDRAM_ADDR +0x200000)));


zbar_image_scanner_t *scanner;
zbar_image_t *img;

int frame_transmit = 0;//��ά��ʶ���־λ��1������ʶ��  0��ʶ�����

uint8_t data_ready =0;//��ȡ��ʶ��ͼ��״̬��1����ȡ�ɹ�  0��δ��ȡ

void DCMI_VS_IRQ(uint32_t src_addr,uint32_t size)
{		

    if(frame_transmit == 0)
    {
        frame_transmit = 1;
        memcpy(dcmi_buf,(uint16_t *)src_addr,sizeof(uint16_t)*size);
        data_ready = 1;
    }
}
void zbar_img_cleanup(zbar_image_t *image)
{
}
/**
  * @brief  ��ȡRGB565����ֵ
  * @param  
  * @param  
  * @note 	
  */
unsigned char rgb565_to_y(unsigned short c)
{
	unsigned char r, g, b;
	unsigned short y;
	r = (c >> 11) & 0x1F;
	g = (c >> 5) & 0x3F;
	b = c & 0x1F;
	
	y = (r * 2 + g + b * 2) * 256 / 192;
	
	if(y > 255)
		y = 255;

	return y;
}
/**
  * @brief  �Ҷ�ֵ���
  * @param  
  * @param  
  * @note 	
  */
unsigned char corse_check(unsigned char *c, int len)
{
	int max = 0, min = 255, max_cnt, min_cnt;
	
	for(int y = 0; y < len; y += 4)
	{
		if(max < c[y])
			max = c[y];
		else if(min > c[y])
			min = c[y];
	}
	if(max - min < 128)
		return 1;
	
	for(int y = 0; y < len; y += 4)
	{
		if(max - 30 < c[y])
			max_cnt++;
		if(min + 30 > c[y])
			min_cnt++;
	}
	if(max_cnt < 30 || min_cnt < 30)
		return 2;
	
	return 0;
}

/**
  * @brief  ��ά��ʶ����
  * @param  
  * @param  
  * @note 	
  */
void QR_decoder(void)
{

    if(qr_init_finish)//��ʼ����ά��ʶ��˿�ֻ��Ҫһ��
    {
        qr_init_finish = 0;
        scanner = zbar_image_scanner_create();
        img = zbar_image_create();
        zbar_image_scanner_set_config(scanner, ZBAR_NONE, ZBAR_CFG_ENABLE, 1);
        zbar_image_set_userdata(img, NULL);
        zbar_image_set_size(img, img_width, img_height);
        zbar_image_set_format(img, zbar_fourcc_parse("Y800"));
        zbar_image_set_data(img, img_now, img_width * img_height, zbar_img_cleanup);   
    }
    if(data_ready)//������ͷ��ȡһ֡ͼ����ɱ�־
	{
		data_ready = 0;			
		for(int y = 0; y < img_height; y++)
		{
			for(int x = 0; x < img_width; x++)
			{
				int idx = y * img_width + x;
				img_now[idx] = rgb565_to_y(dcmi_buf[idx]);
			}
        }

		if(corse_check(&img_now[(img_height/2-1) * img_width], img_width * 2) == 0)
		{

			if(zbar_scan_image(scanner, img) > 0)
			{
				const zbar_symbol_set_t *syms = zbar_image_get_symbols(img);
				zbar_symbol_t *sym = syms->head;
							
				for(int i = 0; i < syms->nsyms; i++)
				{
					char *symstr = sym->data;
					int len = strlen(symstr);
					sym = sym->next;
					uart_send_buf((unsigned char *)symstr, len);
					while(uart_send_sta());
					uart_send_buf((unsigned char *)"\r\n", 2);
					while(uart_send_sta());

				}
                qr_recongized = 1;
			}

		}       
        frame_transmit = 0;      
	}

}
