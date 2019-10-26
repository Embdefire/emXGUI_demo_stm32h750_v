/**
  ******************************************************************************
  * @file    qr_decoder_user.c
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

#include "qr_decoder_user.h"

/*ɨ���ǲɼ�ͼ��Ĵ�С*/
uint16_t  img_w;
uint16_t  img_h;

uint8_t data_ready;//��ȡ��ʶ��ͼ��״̬��1����ȡ�ɹ�  0��δ��ȡ

/*ɨ��������֧���Զ��壬ע��ɨ���Ŀ�Ȳ��ܳ���ͼ��ĸ߶� */
#define  qr_img_w   Frame_width
#define  qr_img_h   Frame_width

//���ݻ�������
//1M�ֽ�
unsigned short  dcmi_buf[1];//dcmi_buf[204800]    __attribute((at(QR_FRAME_BUFFER)));//
//1M�ֽ�
unsigned short  qr_img_buf[1];//qr_img_buf[102400]  __attribute((at(QR_FRAME_BUFFER +0x100000)));
//512k�ֽ�
unsigned char   img_now[1];//img_now[102400]     __attribute((at(QR_FRAME_BUFFER +0x200000)));
//512k�ֽ�
//unsigned char   img_bin[524288]     __attribute((at(QR_FRAME_BUFFER +0x280000)));
//zbar_image_scanner_t *scanner = NULL;
char decoded_buf[QR_SYMBOL_NUM][QR_SYMBOL_SIZE];

int frame_transmit = 0;//��ά��ʶ���־λ��1������ʶ��  0��ʶ�����

uint8_t data_ready =0;//��ȡ��ʶ��ͼ��״̬��1����ȡ�ɹ�  0��δ��ȡ
/**
  * @brief  ��ȡһ֡ͼ��
  * @param  
  * @param  
  * @note 	
  */
void get_image(uint32_t qr_src_addr,uint16_t qr_img_width,uint16_t qr_img_height)
{
    img_w = qr_img_width;
    img_h = qr_img_height;
    if(frame_transmit == 0)
    {
        frame_transmit = 1;
        memcpy(dcmi_buf,(uint16_t *)qr_src_addr,qr_img_width*qr_img_height*2);
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
  * @brief  ��ά��ʶ����
  * @param  
  * @param  
  * @note 	
  */
char QR_decoder(void)
{
    zbar_image_scanner_t *scanner;
    
    char type_len;
    short data_len;
    int addr=0;
    int i=0,j=0;
    int symbol_num=0;

    if(data_ready)//������ͷ��ȡһ֡ͼ����ɱ�־
	{
		data_ready = 0;       
        /* obtain image data */
        for(int y = 0; y < qr_img_h; y++)
		{
			for(int x = 0; x < qr_img_w; x++)
			{
				int idx = y * qr_img_w + x;
                qr_img_buf[idx] = dcmi_buf[((img_h-qr_img_h)/2+y)*img_w+(img_w-qr_img_w)/2+x];
				img_now[idx] = rgb565_to_y(qr_img_buf[idx]);
			}
        }

        /* wrap image data */
        zbar_image_t *img = zbar_image_create();
        if (img == NULL) 
        {
            return NULL;
        }
        //zbar_image_set_format(img, zbar_fourcc('Y','8','0','0'));
        zbar_image_set_format(img, zbar_fourcc_parse("Y800"));
        zbar_image_set_userdata(img, NULL);
        zbar_image_set_size(img, qr_img_w, qr_img_h);
        zbar_image_set_data(img, img_now, qr_img_w * qr_img_h, zbar_img_cleanup); 
       
        /* create a reader */
        scanner = zbar_image_scanner_create();
            /* configure the readerZBAR_NONE */
        zbar_image_scanner_set_config(scanner, ZBAR_CODE93, ZBAR_CFG_ENABLE, 1);        

        /* scan the image for barcodes */
        symbol_num = zbar_scan_image(scanner, img);//����ʶ������ĸ���
        
        if(symbol_num)
        {
            /* extract results */
            const zbar_symbol_t *symbol = zbar_image_first_symbol(img);
            for(; symbol; symbol = zbar_symbol_next(symbol)) 
            {
                zbar_symbol_type_t typ = zbar_symbol_get_type(symbol);
                
                const char *qr_typ =  zbar_get_symbol_name(typ);//��������
                type_len =  strlen(zbar_get_symbol_name(typ));  //���ͳ���
                
                const char *data   =  zbar_symbol_get_data(symbol);//���������
                data_len =  zbar_symbol_get_data_length(symbol);   //���ݳ���
                
                //�������ݷ�װΪ��ά���飬��ʽΪ��
                // ����һ�飺�������ͳ���(8bit)+������������+�������ݳ���(16bit,��λ��ǰ��λ�ں�)+�������ݣ�
                // ���ڶ��飺�������ͳ���(8bit)+������������+�������ݳ���(16bit,��λ��ǰ��λ�ں�)+�������ݣ�
                //  ������
                //�Դ�����
                decoded_buf[j][addr++] = type_len;
                i=type_len;
                for(; i > 0; i--)
                    decoded_buf[j][addr++] = *(qr_typ++);
                
                decoded_buf[j][addr++] = data_len>>8;
                decoded_buf[j][addr++] = data_len;
                i=data_len;
                for(; i > 0; i--)
                    decoded_buf[j][addr++] = *(data++);
                j++;
                addr=0;
            }
 
        }
        /* clean up */
        zbar_image_destroy(img);
        zbar_image_scanner_destroy(scanner);       
        frame_transmit = 0;     
    }
    
    return symbol_num;
}

