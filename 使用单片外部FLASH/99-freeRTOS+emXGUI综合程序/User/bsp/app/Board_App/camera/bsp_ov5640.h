#ifndef __OV5640_H
#define	__OV5640_H

/**
  ******************************************************************************
  * @file    DCMI/Camera/dcmi_ov5640.h
  * @author  MCD Application Team
  * @version V1.0.1
  * @date    13-April-2012
  * @brief   Header for dcmi_ov5640.c module
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT 2012 STMicroelectronics</center></h2>
  *
  * Licensed under MCD-ST Liberty SW License Agreement V2, (the "License");
  * You may not use this file except in compliance with the License.
  * You may obtain a copy of the License at:
  *
  *        http://www.st.com/software_license_agreement_liberty_v2
  *
  * Unless required by applicable law or agreed to in writing, software 
  * distributed under the License is distributed on an "AS IS" BASIS, 
  * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  * See the License for the specific language governing permissions and
  * limitations under the License.
  *
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __DCMI_OV5640_H
#define __DCMI_OV5640_H

/* Includes ------------------------------------------------------------------*/
#include "stm32h7xx.h"
#include "./lcd/bsp_lcd.h"
#include "stm32h7xx_hal.h"

extern DCMI_HandleTypeDef DCMI_Handle;


#define FRAME_RATE_30FPS	0 //30֡
#define FRAME_RATE_15FPS	1 //15֡


/*����ͷ�ɼ�ͼ��Ĵ�С���ı�������ֵ���Ըı���������
������ӿ�ɼ��ٶȣ�Ҫ�ӿ�ɼ��ٶ���Ҫ�ĳ�SVGAģʽ*/
#define img_width  800
#define img_height 480


/* Exported constants --------------------------------------------------------*/
#define DCMI_DR_ADDRESS       0x50050028
#define FSMC_LCD_ADDRESS      LCD_FRAME_BUFFER


/*����ͷ���ýṹ��*/
typedef struct
{	
	uint8_t frame_rate;	//���֡��
	
	uint16_t cam_isp_sx; //����ͷISP X��ʼλ��
	uint16_t cam_isp_sy; //����ͷISP Y��ʼλ��

	uint16_t cam_isp_width; //����ͷISP ��
	uint16_t cam_isp_height; //����ͷISP ��

	uint8_t scaling;				//�Ƿ�ʹ���Զ����ţ��Ƽ�ʹ�ã�1:ʹ�ã�0:��ʹ��
	
	uint16_t cam_out_sx; //����ͷ�������X��ʼλ��
	uint16_t cam_out_sy; //����ͷ�������Y��ʼλ��
	
	uint16_t cam_out_width;//���ͼ��ֱ��ʣ���
	uint16_t cam_out_height;//���ͼ��ֱ��ʣ���
	
	uint16_t lcd_sx;//ͼ����ʾ��Һ������X��ʼλ��
	uint16_t lcd_sy;//ͼ����ʾ��Һ������Y��ʼλ��
	uint8_t lcd_scan;//Һ������ɨ��ģʽ��0-7��
	
	uint8_t light_mode;//����ģʽ��������Χ[0~4]
	int8_t saturation;//���Ͷ�,������Χ[-3 ~ +3]   
	int8_t brightness;//���նȣ�������Χ[-4~+4]
	int8_t contrast;//�Աȶȣ�������Χ[-3~+3]
	uint8_t effect;	//����Ч����������Χ[0~9]:	
	int8_t exposure;//�عⲹ����������Χ[-3~+3]

	
	uint8_t auto_focus;//�Ƿ�ʹ���Զ��Խ����� 1:ʹ�ã�0:��ʹ��

}OV5640_MODE_PARAM;

/* Image Sizes enumeration */
typedef enum   
{
  BMP_320x240          =   0x00,	  /* BMP Image 320x240 Size */
  BMP_352x288          =   0x01,      /* BMP Image 352x288 Size */
  BMP_640x480          =   0x02,	  /* BMP Image 640x480 Size */
  BMP_800x480          =   0x03,	  /* BMP Image 800x480 Size */
}ImageFormat_TypeDef;


/*����ͷ�ӿ� */
//IIC SCCB
//VSYNC
#define DCMI_VSYNC_GPIO_PORT        	    GPIOB
#define DCMI_VSYNC_GPIO_CLK_ENABLE()        __HAL_RCC_GPIOB_CLK_ENABLE()
#define DCMI_VSYNC_GPIO_PIN         	    GPIO_PIN_7
#define DCMI_VSYNC_AF			                GPIO_AF13_DCMI
// HSYNC
#define DCMI_HSYNC_GPIO_PORT        	    GPIOA
#define DCMI_HSYNC_GPIO_CLK_ENABLE()        __HAL_RCC_GPIOA_CLK_ENABLE()
#define DCMI_HSYNC_GPIO_PIN         	    GPIO_PIN_4
#define DCMI_HSYNC_AF			                GPIO_AF13_DCMI
//PIXCLK
#define DCMI_PIXCLK_GPIO_PORT        	    GPIOA
#define DCMI_PIXCLK_GPIO_CLK_ENABLE()       __HAL_RCC_GPIOA_CLK_ENABLE()
#define DCMI_PIXCLK_GPIO_PIN         	    GPIO_PIN_6
#define DCMI_PIXCLK_AF			              GPIO_AF13_DCMI
//PWDN
#define DCMI_PWDN_GPIO_PORT        	        GPIOB
#define DCMI_PWDN_GPIO_CLK_ENABLE()         __HAL_RCC_GPIOB_CLK_ENABLE()
#define DCMI_PWDN_GPIO_PIN         	        GPIO_PIN_10

//RST
#define DCMI_RST_GPIO_PORT        	        GPIOF
#define DCMI_RST_GPIO_CLK_ENABLE()          __HAL_RCC_GPIOF_CLK_ENABLE()
#define DCMI_RST_GPIO_PIN         	        GPIO_PIN_10

//�����ź���
#define DCMI_D0_GPIO_PORT        	        GPIOC
#define DCMI_D0_GPIO_CLK_ENABLE()         	__HAL_RCC_GPIOC_CLK_ENABLE()
#define DCMI_D0_GPIO_PIN         	        GPIO_PIN_6
#define DCMI_D0_AF			                  GPIO_AF13_DCMI

#define DCMI_D1_GPIO_PORT        	        GPIOC
#define DCMI_D1_GPIO_CLK_ENABLE()         	__HAL_RCC_GPIOC_CLK_ENABLE()
#define DCMI_D1_GPIO_PIN         	        GPIO_PIN_7
#define DCMI_D1_AF			                  GPIO_AF13_DCMI

#define DCMI_D2_GPIO_PORT        	        GPIOG
#define DCMI_D2_GPIO_CLK_ENABLE()         	__HAL_RCC_GPIOG_CLK_ENABLE()
#define DCMI_D2_GPIO_PIN         	        GPIO_PIN_10
#define DCMI_D2_AF			                  GPIO_AF13_DCMI

#define DCMI_D3_GPIO_PORT        	        GPIOG
#define DCMI_D3_GPIO_CLK_ENABLE()         	__HAL_RCC_GPIOG_CLK_ENABLE()
#define DCMI_D3_GPIO_PIN         	        GPIO_PIN_11
#define DCMI_D3_AF			                  GPIO_AF13_DCMI

#define DCMI_D4_GPIO_PORT        	        GPIOE
#define DCMI_D4_GPIO_CLK_ENABLE()         	__HAL_RCC_GPIOE_CLK_ENABLE()
#define DCMI_D4_GPIO_PIN         	        GPIO_PIN_4
#define DCMI_D4_AF			                  GPIO_AF13_DCMI

#define DCMI_D5_GPIO_PORT        	        GPIOD
#define DCMI_D5_GPIO_CLK_ENABLE()         __HAL_RCC_GPIOD_CLK_ENABLE()
#define DCMI_D5_GPIO_PIN         	        GPIO_PIN_3
#define DCMI_D5_AF			                  GPIO_AF13_DCMI

#define DCMI_D6_GPIO_PORT        	        GPIOE
#define DCMI_D6_GPIO_CLK_ENABLE()         __HAL_RCC_GPIOE_CLK_ENABLE()
#define DCMI_D6_GPIO_PIN         	        GPIO_PIN_5
#define DCMI_D6_AF			                  GPIO_AF13_DCMI

#define DCMI_D7_GPIO_PORT        	        GPIOE
#define DCMI_D7_GPIO_CLK_ENABLE()         __HAL_RCC_GPIOE_CLK_ENABLE()
#define DCMI_D7_GPIO_PIN         	        GPIO_PIN_6
#define DCMI_D7_AF			                  GPIO_AF13_DCMI


/*debug*/

#define CAMERA_DEBUG_ON          0
#define CAMERA_DEBUG_ARRAY_ON   0
#define CAMERA_DEBUG_FUNC_ON    0
   
   
// Log define
#define CAMERA_INFO(fmt,arg...)           printf("<<-CAMERA-INFO->> "fmt"\n",##arg)
#define CAMERA_ERROR(fmt,arg...)          printf("<<-CAMERA-ERROR->> "fmt"\n",##arg)
#define CAMERA_DEBUG(fmt,arg...)          do{\
                                         if(CAMERA_DEBUG_ON)\
                                         printf("<<-CAMERA-DEBUG->> [%d]"fmt"\n",__LINE__, ##arg);\
                                       }while(0)
#define CAMERA_DEBUG_ARRAY(array, num)    do{\
                                         int32_t i;\
                                         uint8_t* a = array;\
                                         if(CAMERA_DEBUG_ARRAY_ON)\
                                         {\
                                            printf("<<-CAMERA-DEBUG-ARRAY->>\n");\
                                            for (i = 0; i < (num); i++)\
                                            {\
                                                printf("%02x   ", (a)[i]);\
                                                if ((i + 1 ) %10 == 0)\
                                                {\
                                                    printf("\n");\
                                                }\
                                            }\
                                            printf("\n");\
                                        }\
                                       }while(0)
#define CAMERA_DEBUG_FUNC()               do{\
                                         if(CAMERA_DEBUG_FUNC_ON)\
                                         printf("<<-CAMERA-FUNC->> Func:%s@Line:%d\n",__func__,__LINE__);\
                                       }while(0)


/* Exported types ------------------------------------------------------------*/
//�洢����ͷID�Ľṹ��
typedef struct
{
  uint8_t PIDH;
  uint8_t PIDL;
}OV5640_IDTypeDef;

/* Use this define to set the maximum delay timeout for the I2C DCMI_OV5640_SingleRandomWrite()
   and DCMI_OV5640_SingleRandomRead() operations. Exeeding this timeout delay, 
   the read/write functions will be aborted and return error code (0xFF).
   The period of the delay will depend on the system operating frequency. The following
   value has been set for system running at 168 MHz. */
#define DCMI_TIMEOUT_MAX               10000
#define OV5640_DEVICE_WRITE_ADDRESS    0x78
#define OV5640_DEVICE_READ_ADDRESS     0x79

/* OV5640 Registers definition when DSP bank selected (0xFF = 0x00) */
#define OV5640_DSP_R_BYPASS     0x05
#define OV5640_DSP_Qs           0x44
#define OV5640_DSP_CTRL         0x50
#define OV5640_DSP_HSIZE1       0x51
#define OV5640_DSP_VSIZE1       0x52
#define OV5640_DSP_XOFFL        0x53
#define OV5640_DSP_YOFFL        0x54
#define OV5640_DSP_VHYX         0x55
#define OV5640_DSP_DPRP         0x56
#define OV5640_DSP_TEST         0x57
#define OV5640_DSP_ZMOW         0x5A
#define OV5640_DSP_ZMOH         0x5B
#define OV5640_DSP_ZMHH         0x5C
#define OV5640_DSP_BPADDR       0x7C
#define OV5640_DSP_BPDATA       0x7D
#define OV5640_DSP_CTRL2        0x86
#define OV5640_DSP_CTRL3        0x87
#define OV5640_DSP_SIZEL        0x8C
#define OV5640_DSP_HSIZE2       0xC0
#define OV5640_DSP_VSIZE2       0xC1
#define OV5640_DSP_CTRL0        0xC2
#define OV5640_DSP_CTRL1        0xC3
#define OV5640_DSP_R_DVP_SP     0xD3
#define OV5640_DSP_IMAGE_MODE   0xDA
#define OV5640_DSP_RESET        0xE0
#define OV5640_DSP_MS_SP        0xF0
#define OV5640_DSP_SS_ID        0x7F
#define OV5640_DSP_SS_CTRL      0xF8
#define OV5640_DSP_MC_BIST      0xF9
#define OV5640_DSP_MC_AL        0xFA
#define OV5640_DSP_MC_AH        0xFB
#define OV5640_DSP_MC_D         0xFC
#define OV5640_DSP_P_STATUS     0xFE
#define OV5640_DSP_RA_DLMT      0xFF

/* OV5640 Registers definition when sensor bank selected (0xFF = 0x01) */
#define OV5640_SENSOR_GAIN       0x00
#define OV5640_SENSOR_COM1       0x03
#define OV5640_SENSOR_REG04      0x04
#define OV5640_SENSOR_REG08      0x08
#define OV5640_SENSOR_COM2       0x09

#define OV5640_SENSOR_PIDH       0x300A
#define OV5640_SENSOR_PIDL       0x300B

#define OV5640_SENSOR_COM3       0x0C
#define OV5640_SENSOR_COM4       0x0D
#define OV5640_SENSOR_AEC        0x10
#define OV5640_SENSOR_CLKRC      0x11
#define OV5640_SENSOR_COM7       0x12
#define OV5640_SENSOR_COM8       0x13
#define OV5640_SENSOR_COM9       0x14
#define OV5640_SENSOR_COM10      0x15
#define OV5640_SENSOR_HREFST     0x17
#define OV5640_SENSOR_HREFEND    0x18
#define OV5640_SENSOR_VSTART     0x19
#define OV5640_SENSOR_VEND       0x1A


#define OV5640_SENSOR_MIDH       0x1C
#define OV5640_SENSOR_MIDL       0x1D
   
#define OV5640_SENSOR_AEW        0x24
#define OV5640_SENSOR_AEB        0x25
#define OV5640_SENSOR_W          0x26
#define OV5640_SENSOR_REG2A      0x2A
#define OV5640_SENSOR_FRARL      0x2B
#define OV5640_SENSOR_ADDVSL     0x2D
#define OV5640_SENSOR_ADDVHS     0x2E
#define OV5640_SENSOR_YAVG       0x2F
#define OV5640_SENSOR_REG32      0x32
#define OV5640_SENSOR_ARCOM2     0x34
#define OV5640_SENSOR_REG45      0x45
#define OV5640_SENSOR_FLL        0x46
#define OV5640_SENSOR_FLH        0x47
#define OV5640_SENSOR_COM19      0x48
#define OV5640_SENSOR_ZOOMS      0x49
#define OV5640_SENSOR_COM22      0x4B
#define OV5640_SENSOR_COM25      0x4E
#define OV5640_SENSOR_BD50       0x4F
#define OV5640_SENSOR_BD60       0x50
#define OV5640_SENSOR_REG5D      0x5D
#define OV5640_SENSOR_REG5E      0x5E
#define OV5640_SENSOR_REG5F      0x5F
#define OV5640_SENSOR_REG60      0x60
#define OV5640_SENSOR_HISTO_LOW  0x61
#define OV5640_SENSOR_HISTO_HIGH 0x62

/* Exported macro ------------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */

void OV5640_HW_Init(void);
void OV5640_I2C_Init(void);
void OV5640_Reset(void);
void OV5640_ReadID(OV5640_IDTypeDef *OV5640ID);
void OV5640_Init(void);
void OV5640_QQVGAConfig(void);
void OV5640_WVGAConfig(void);
void OV5640_RGB565Config(void);

void OV5640_BrightnessConfig(int8_t Brightness);
//void OV5640_ContrastConfig(uint8_t value1, uint8_t value2);
void OV5640_BandWConfig(uint8_t BlackWhite);
void OV5640_ColorEffectsConfig(uint8_t value1, uint8_t value2);
uint8_t OV5640_WriteReg(uint16_t Addr, uint8_t Data);
uint8_t OV5640_ReadReg(uint16_t Addr);
uint8_t OV5640_WriteFW(uint8_t *pBuffer ,uint16_t BufferSize);
void OV5640_SpecialEffects(uint8_t mode);
void OV5640_LightMode(uint8_t mode);
void OV5640_USER_Config(void);
void OV5640_Capture_Control(FunctionalState state);
extern OV5640_MODE_PARAM cam_mode;

void OV5640_DMA_Config(uint32_t DMA_Memory0BaseAddr,uint32_t DMA_BufferSize);
void OV5640_Color_Saturation(int8_t sat);
void OV5640_ContrastConfig(int8_t cnst);
void OV5640_OutSize_Set(uint8_t scaling,uint16_t x_off,uint16_t y_off,uint16_t width,uint16_t height);
//void I2Cx_Error(I2C_HandleTypeDef *i2c_handler);
#endif /* __DCMI_OV5640_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/


#endif /* __LED_H */
