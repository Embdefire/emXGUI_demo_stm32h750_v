#ifndef __QR_DECODER_USER_H
#define	__QR_DECODER_USER_H

#include <stdio.h>
#include <stdint.h>

// ����SDRAM��3M�ֽ���Ϊ���ݻ��棬����ʹ���Դ�����Ŀռ䣬
// 0xD0800000-0x300000 = 0xD0500000
#define  QR_FRAME_BUFFER  ((uint32_t)(0xd18c0000))

/*ɨ�贰�ڲ���*/
#define  Frame_width           ((uint16_t)320)//ɨ�贰�ڱ߳��������Σ�

/*ɨ�����������*/
#define  Frame_line_length     ((uint16_t)30) //ɨ�����������
#define  Frame_line_size       ((uint16_t)3)  //ɨ����������

#define  QR_SYMBOL_NUM  5    //ʶ���ά���������
#define  QR_SYMBOL_SIZE 512  //ÿ���ά��ĵ��������

//�������ݷ�װΪ��ά����decoded_buf����ʽΪ��
// ����һ�飺�������ͳ���(8bit)+������������+�������ݳ���(16bit,��λ��ǰ��λ�ں�)+�������ݣ�
// ���ڶ��飺�������ͳ���(8bit)+������������+�������ݳ���(16bit,��λ��ǰ��λ�ں�)+�������ݣ�
//  ������
//�Դ�����
extern char decoded_buf[QR_SYMBOL_NUM][QR_SYMBOL_SIZE];

//���뺯��������ֵΪʶ������ĸ���
char QR_decoder(void);

//��ȡһ֡ͼ��
void get_image(uint32_t qr_src_addr,uint16_t qr_img_width,uint16_t qr_img_height);

#endif /* __QR_DECODER_USER_H */
