/**
  ******************************************************************************
  * @file    bsp_sdram.c
  * @author  fire
  * @version V1.0
  * @date    2015-xx-xx
  * @brief   OV5640����ͷ����
  ******************************************************************************
  * @attention
  *
  * ʵ��ƽ̨:����  STM32 F767 ������  
  * ��̳    :http://www.firebbs.cn
  * �Ա�    :http://firestm32.taobao.com
  *
  ******************************************************************************
  */


/* Includes ------------------------------------------------------------------*/
#include "./camera/bsp_ov5640.h"
#include "./i2c/i2c.h"
#include "GUI_CAMERA_DIALOG.h"
#include "./delay/core_delay.h"  
#include "qr_decoder_user.h"

DCMI_HandleTypeDef DCMI_Handle;
DMA_HandleTypeDef DMA_Handle_dcmi;
extern uint8_t QR_Task;
void TransferComplete(DMA2D_HandleTypeDef *hdma2d)
{
  //SCB_InvalidateDCache();
}

int cur_index;//��ǰ�ڴ��
//uint32_t cam_buff0[800*480];
//uint32_t cam_buff1[800*480];
/** @addtogroup DCMI_Camera
  * @{
  */ 
//����ͷ��ʼ������
//ע�⣺ʹ�����ַ�ʽ��ʼ���ṹ�壬Ҫ��c/c++ѡ����ѡ�� C99 mode
OV5640_MODE_PARAM cam_mode =
{
/*���°�����������ͷ���ã������в��ԣ�����һ�飬����������ע�͵�����*/
/************����1***854*480******������ʾ*****************************/
	.frame_rate = FRAME_RATE_15FPS,	
	
	//ISP����
	.cam_isp_sx = 0,
	.cam_isp_sy = 0,	
	
	.cam_isp_width = 1920,
	.cam_isp_height = 1080,
	
	//�������
	.scaling = 1,      //ʹ���Զ�����
	.cam_out_sx = 16,	//ʹ���Զ����ź�һ�����ó�16����
	.cam_out_sy = 4,	  //ʹ���Զ����ź�һ�����ó�4����
	.cam_out_width = 480,
	.cam_out_height = 272,
	
	//LCDλ��
	.lcd_sx = 0,
	.lcd_sy = 0,
	.lcd_scan = 5, //LCDɨ��ģʽ�����������ÿ���1��3��5��7ģʽ
	
	//���¿ɸ����Լ�����Ҫ������������Χ���ṹ�����Ͷ���	
	.light_mode = 0x04,//�Զ�����ģʽ
	.saturation = 0,	
	.brightness = 0,
	.contrast = 0,
	.effect = 0x00,		//����ģʽ
	.exposure = 0,		

	.auto_focus = 1,
	
/**********����2*****240*320****������ʾ****************************/	
//	.frame_rate = FRAME_RATE_30FPS,	
//	
//	//ISP����
//	.cam_isp_sx = 0,
//	.cam_isp_sy = 0,	
//	
//	.cam_isp_width = 1920,
//	.cam_isp_height = 1080,
//	
//	//�������
//	.scaling = 1,      //ʹ���Զ�����
//	.cam_out_sx = 16,	//ʹ���Զ����ź�һ�����ó�16����
//	.cam_out_sy = 4,	  //ʹ���Զ����ź�һ�����ó�4����
//	.cam_out_width = 240,
//	.cam_out_height = 320,
//	
//	//LCDλ��
//	.lcd_sx = 120,
//	.lcd_sy = 267,
//	.lcd_scan = 6, //LCDɨ��ģʽ��
//	
//	//���¿ɸ����Լ�����Ҫ������������Χ���ṹ�����Ͷ���	
//	.light_mode = 0,//�Զ�����ģʽ
//	.saturation = 0,	
//	.brightness = 0,
//	.contrast = 0,
//	.effect = 0,		//����ģʽ
//	.exposure = 0,		

//	.auto_focus = 1,//�Զ��Խ�
	
	/*******����3********640*480****С�ֱ���****************************/	
//  .frame_rate = FRAME_RATE_30FPS,	
//	
//	//ISP����
//	.cam_isp_sx = 0,
//	.cam_isp_sy = 0,	
//	
//	.cam_isp_width = 1920,
//	.cam_isp_height = 1080,
//	
//	//�������
//	.scaling = 0,      //ʹ���Զ�����
//	.cam_out_sx = 16,	//ʹ���Զ����ź�һ�����ó�16����
//	.cam_out_sy = 4,	  //ʹ���Զ����ź�һ�����ó�4����
//	.cam_out_width = 640,
//	.cam_out_height = 480,
//	
//	//LCDλ��
//	.lcd_sx = 100,
//	.lcd_sy = 0,
//	.lcd_scan = 5, //LCDɨ��ģʽ��
//	
//	//���¿ɸ����Լ�����Ҫ������������Χ���ṹ�����Ͷ���	
//	.light_mode = 0,//�Զ�����ģʽ
//	.saturation = 0,	
//	.brightness = 0,
//	.contrast = 0,
//	.effect = 0,		//����ģʽ
//	.exposure = 0,		

//	.auto_focus = 1,//�Զ��Խ�

};
/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
#define  TIMEOUT  2

ImageFormat_TypeDef ImageFormat;


/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/

unsigned short RGB565_Init[][2]=

{

    //15fps VGA RGB565 output

    // 24MHz input clock, 24MHz PCLK

    0x3103, 0x11, // system clock from pad, bit[1]

    0x3008, 0x82, // software reset, bit[7]

    // delay 5ms

    0x3008, 0x42, // software power down, bit[6]

    0x3103, 0x03, // system clock from PLL, bit[1]

    0x3017, 0xff, // FREX, Vsync, HREF, PCLK, D[9:6] output enable

    0x3018, 0xff, // D[5:0], GPIO[1:0] output enable

    0x3034, 0x1a, // MIPI 10-bit

    0x3037, 0x13, // PLL root divider, bit[4], PLL pre-divider, bit[3:0]

    0x3108, 0x01, // PCLK root divider, bit[5:4], SCLK2x root divider, bit[3:2]

    // SCLK root divider, bit[1:0]

    0x3630, 0x36,

    0x3631, 0x0e,

    0x3632, 0xe2,

    0x3633, 0x12,

    0x3621, 0xe0,

    0x3704, 0xa0,

    0x3703, 0x5a,

    0x3715, 0x78,

    0x3717, 0x01,

    0x370b, 0x60,

    0x3705, 0x1a,

    0x3905, 0x02,

    0x3906, 0x10,

    0x3901, 0x0a,

    0x3731, 0x12,

    0x3600, 0x08, // VCM control

    0x3601, 0x33, // VCM control

    0x302d, 0x60, // system control

    0x3620, 0x52,

    0x371b, 0x20,

    0x471c, 0x50,

    0x3a13, 0x43, // pre-gain = 1.047x

    0x3a18, 0x00, // gain ceiling

    0x3a19, 0xf8, // gain ceiling = 15.5x

    0x3635, 0x13,

    0x3636, 0x03,

    0x3634, 0x40,

    0x3622, 0x01,

    // 50/60Hz detection 50/60Hz �ƹ����ƹ���

    0x3c01, 0x34, // Band auto, bit[7]

    0x3c04, 0x28, // threshold low sum

    0x3c05, 0x98, // threshold high sum

    0x3c06, 0x00, // light meter 1 threshold[15:8]

    0x3c07, 0x08, // light meter 1 threshold[7:0]

    0x3c08, 0x00, // light meter 2 threshold[15:8]

    0x3c09, 0x1c, // light meter 2 threshold[7:0]

    0x3c0a, 0x9c, // sample number[15:8]

    0x3c0b, 0x40, // sample number[7:0]

    0x3810, 0x00, // Timing Hoffset[11:8]

    0x3811, 0x10, // Timing Hoffset[7:0]

    0x3812, 0x00, // Timing Voffset[10:8]

    0x3708, 0x64,

    0x4001, 0x02, // BLC start from line 2

    0x4005, 0x1a, // BLC always update

    0x3000, 0x00, // enable blocks

    0x3004, 0xff, // enable clocks

    0x300e, 0x58, // MIPI power down, DVP enable

    0x302e, 0x00,



    0x4300, 0x6f, // RGB565

    0x501f, 0x01, // RGB565



    0x440e, 0x00,

    0x5000, 0xa7, // Lenc on, raw gamma on, BPC on, WPC on, CIP on

    // AEC target �Զ��ع����

    0x3a0f, 0x30, // stable range in high

    0x3a10, 0x28, // stable range in low

    0x3a1b, 0x30, // stable range out high

    0x3a1e, 0x26, // stable range out low

    0x3a11, 0x60, // fast zone high

    0x3a1f, 0x14, // fast zone low

    // Lens correction for ? ��ͷ����

    0x5800, 0x23,

    0x5801, 0x14,

    0x5802, 0x0f,

    0x5803, 0x0f,

    0x5804, 0x12,

    0x5805, 0x26,

    0x5806, 0x0c,

    0x5807, 0x08,

    0x5808, 0x05,

    0x5809, 0x05,

    0x580a, 0x08,

    0x580b, 0x0d,

    0x580c, 0x08,

    0x580d, 0x03,

    0x580e, 0x00,

    0x580f, 0x00,

    0x5810, 0x03,

    0x5811, 0x09,

    0x5812, 0x07,

    0x5813, 0x03,

    0x5814, 0x00,

    0x5815, 0x01,

    0x5816, 0x03,

    0x5817, 0x08,

    0x5818, 0x0d,

    0x5819, 0x08,

    0x581a, 0x05,

    0x581b, 0x06,

    0x581c, 0x08,

    0x581d, 0x0e,

    0x581e, 0x29,

    0x581f, 0x17,

    0x5820, 0x11,

    0x5821, 0x11,

    0x5822, 0x15,

    0x5823, 0x28,

    0x5824, 0x46,

    0x5825, 0x26,

    0x5826, 0x08,

    0x5827, 0x26,

    0x5828, 0x64,

    0x5829, 0x26,

    0x582a, 0x24,

    0x582b, 0x22,

    0x582c, 0x24,

    0x582d, 0x24,

    0x582e, 0x06,

    0x582f, 0x22,

    0x5830, 0x40,

    0x5831, 0x42,

    0x5832, 0x24,

    0x5833, 0x26,

    0x5834, 0x24,

    0x5835, 0x22,

    0x5836, 0x22,

    0x5837, 0x26,

    0x5838, 0x44,

    0x5839, 0x24,

    0x583a, 0x26,

    0x583b, 0x28,

    0x583c, 0x42,

    0x583d, 0xce, // lenc BR offset

    // AWB �Զ���ƽ��

    0x5180, 0xff, // AWB B block

    0x5181, 0xf2, // AWB control

    0x5182, 0x00, // [7:4] max local counter, [3:0] max fast counter

    0x5183, 0x14, // AWB advanced

    0x5184, 0x25,

    0x5185, 0x24,

    0x5186, 0x09,

    0x5187, 0x09,

    0x5188, 0x09,

    0x5189, 0x75,

    0x518a, 0x54,

    0x518b, 0xe0,

    0x518c, 0xb2,

    0x518d, 0x42,

    0x518e, 0x3d,

    0x518f, 0x56,

    0x5190, 0x46,

    0x5191, 0xf8, // AWB top limit

    0x5192, 0x04, // AWB bottom limit

    0x5193, 0x70, // red limit

    0x5194, 0xf0, // green limit

    0x5195, 0xf0, // blue limit

    0x5196, 0x03, // AWB control

    0x5197, 0x01, // local limit

    0x5198, 0x04,

    0x5199, 0x12,

    0x519a, 0x04,

    0x519b, 0x00,

    0x519c, 0x06,

    0x519d, 0x82,

    0x519e, 0x38, // AWB control

    // Gamma ٤������

    0x5480, 0x01, // Gamma bias plus on, bit[0]

    0x5481, 0x08,

    0x5482, 0x14,

    0x5483, 0x28,

    0x5484, 0x51,

    0x5485, 0x65,

    0x5486, 0x71,

    0x5487, 0x7d,

    0x5488, 0x87,

    0x5489, 0x91,

    0x548a, 0x9a,

    0x548b, 0xaa,

    0x548c, 0xb8,

    0x548d, 0xcd,

    0x548e, 0xdd,

    0x548f, 0xea,

    0x5490, 0x1d,

    // color matrix ɫ�ʾ���

    0x5381, 0x1e, // CMX1 for Y

    0x5382, 0x5b, // CMX2 for Y

    0x5383, 0x08, // CMX3 for Y

    0x5384, 0x0a, // CMX4 for U

    0x5385, 0x7e, // CMX5 for U

    0x5386, 0x88, // CMX6 for U

    0x5387, 0x7c, // CMX7 for V

    0x5388, 0x6c, // CMX8 for V

    0x5389, 0x10, // CMX9 for V

    0x538a, 0x01, // sign[9]

    0x538b, 0x98, // sign[8:1]

    // UV adjust UV ɫ�ʱ��Ͷȵ���

    0x5580, 0x06, // saturation on, bit[1]

    0x5583, 0x40,

    0x5584, 0x10,

    0x5589, 0x10,

    0x558a, 0x00,

    0x558b, 0xf8,

    0x501d, 0x40, // enable manual offset of contrast

    // CIP �񻯺ͽ���

    0x5300, 0x08, // CIP sharpen MT threshold 1

    0x5301, 0x30, // CIP sharpen MT threshold 2

    0x5302, 0x10, // CIP sharpen MT offset 1

    0x5303, 0x00, // CIP sharpen MT offset 2

    0x5304, 0x08, // CIP DNS threshold 1

    0x5305, 0x30, // CIP DNS threshold 2

    0x5306, 0x08, // CIP DNS offset 1

    0x5307, 0x16, // CIP DNS offset 2

    0x5309, 0x08, // CIP sharpen TH threshold 1

    0x530a, 0x30, // CIP sharpen TH threshold 2

    0x530b, 0x04, // CIP sharpen TH offset 1

    0x530c, 0x06, // CIP sharpen TH offset 2

    0x5025, 0x00,

    0x3008, 0x02, // wake up from standby, bit[6]

   

//    0x503d, 0x80,//���Բ���

//    0x4741, 0x00,

};

unsigned short RGB565_QVGA[][2]=
{    
    0x3212, 0x03, // start group 3
    0x3808, 0x01, // DVPHO = 320
    0x3809, 0x40, // DVP HO
    0x380a, 0x00, // DVPVO = 240
    0x380b, 0xf0, // DVPVO
    0x3810, 0x00, // H offset = 16
    0x3811, 0x10, // H offset
    0x3812, 0x00, // V offset = 4
    0x3813, 0x04, // V offset
    0x3212, 0x13, // end group 3
    0x3212, 0xa3, // launch group 3   
};
unsigned short RGB565_VGA[][2]=
{
    0x3212, 0x03, // start group 3
    0x3808, 0x02, // DVPHO = 2048
    0x3809, 0x80, // DVP HO
    0x380a, 0x01, // DVPVO = 1536
    0x380b, 0xe0, // DVPVO
    0x3810, 0x00, // H offset = 16
    0x3811, 0x10, // H offset
    0x3812, 0x00, // V offset = 4
    0x3813, 0x04, // V offset
    0x3212, 0x13, // end group 3
    0x3212, 0xa3, // launch group 3

};



unsigned short RGB565_WVGA[][2]=

{

    // 800x480 15fps, night mode 5fps

    // input clock 24Mhz, PCLK 45.6Mhz

    0x3035, 0x41, // PLL

    0x3036, 0x72, // PLL

    0x3c07, 0x08, // light meter 1 threshold[7:0]

    0x3820, 0x42, // flip

    0x3821, 0x00, // mirror

    0x3814, 0x31, // timing X inc

    0x3815, 0x31, // timing Y inc

    0x3800, 0x00, // HS

    0x3801, 0x00, // HS

    0x3802, 0x00, // VS

    0x3803, 0xbe, // VS

    0x3804, 0x0a, // HW (HE)

    0x3805, 0x3f, // HW (HE)

    0x3806, 0x06, // VH (VE)

    0x3807, 0xe4, // VH (VE)

    0x3808, 0x03, // DVPHO

    0x3809, 0x20, // DVPHO

    0x380a, 0x01, // DVPVO

    0x380b, 0xe0, // DVPVO

    0x380c, 0x07, // HTS

    0x380d, 0x69, // HTS

    0x380e, 0x03, // VTS

    0x380f, 0x21, // VTS

    0x3813, 0x06, // timing V offset

    0x3618, 0x00,

    0x3612, 0x29,

    0x3709, 0x52,

    0x370c, 0x03,

    0x3a02, 0x09, // 60Hz max exposure, night mode 5fps

    0x3a03, 0x63, // 60Hz max exposure

    // banding filters are calculated automatically in camera driver

    //0x3a08, 0x00, // B50 step

    //0x3a09, 0x78, // B50 step

    //0x3a0a, 0x00, // B60 step

    //0x3a0b, 0x64, // B60 step

    //0x3a0e, 0x06, // 50Hz max band

    //0x3a0d, 0x08, // 60Hz max band

    0x3a14, 0x09, // 50Hz max exposure, night mode 5fps

    0x3a15, 0x63, // 50Hz max exposure

    0x4004, 0x02, // BLC line number

    0x3002, 0x1c, // reset JFIFO, SFIFO, JPG

    0x3006, 0xc3, // disable clock of JPEG2x, JPEG

    0x4713, 0x03, // JPEG mode 3

    0x4407, 0x04, // Quantization sacle

    0x460b, 0x35,

    0x460c, 0x22,

    0x4837, 0x22, // MIPI global timing

    0x3824, 0x02, // PCLK manual divider

    0x5001, 0xa3, // SDE on, CMX on, AWB on

    0x3503, 0x00, // AEC/AGC on//	  /* Initialize OV5640 */


//    0x503d, 0x80,//���Բ���

//    0x4741, 0x00,

};
/**
  * @brief  ��ʼ����������ͷʹ�õ�GPIO(I2C/DCMI)
  * @param  None
  * @retval None
  */
void OV5640_HW_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;

    /***DCMI��������***/
    /* ʹ��DCMIʱ�� */
    DCMI_PWDN_GPIO_CLK_ENABLE();
    DCMI_RST_GPIO_CLK_ENABLE();
    DCMI_VSYNC_GPIO_CLK_ENABLE();
    DCMI_HSYNC_GPIO_CLK_ENABLE();
    DCMI_PIXCLK_GPIO_CLK_ENABLE();    
    DCMI_D0_GPIO_CLK_ENABLE();
    DCMI_D1_GPIO_CLK_ENABLE();
    DCMI_D2_GPIO_CLK_ENABLE();
    DCMI_D3_GPIO_CLK_ENABLE();    
    DCMI_D4_GPIO_CLK_ENABLE();
    DCMI_D5_GPIO_CLK_ENABLE();
    DCMI_D6_GPIO_CLK_ENABLE();
    DCMI_D7_GPIO_CLK_ENABLE();

    /*����/ͬ���ź���*/
    GPIO_InitStructure.Pin = DCMI_VSYNC_GPIO_PIN;
    GPIO_InitStructure.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStructure.Pull = GPIO_PULLUP ;
    GPIO_InitStructure.Alternate = DCMI_VSYNC_AF;
    HAL_GPIO_Init(DCMI_VSYNC_GPIO_PORT, &GPIO_InitStructure);

    GPIO_InitStructure.Pin = DCMI_HSYNC_GPIO_PIN;
    GPIO_InitStructure.Alternate = DCMI_VSYNC_AF;
    HAL_GPIO_Init(DCMI_HSYNC_GPIO_PORT, &GPIO_InitStructure);


    GPIO_InitStructure.Pin = DCMI_PIXCLK_GPIO_PIN;
    GPIO_InitStructure.Alternate = DCMI_PIXCLK_AF;
    HAL_GPIO_Init(DCMI_PIXCLK_GPIO_PORT, &GPIO_InitStructure);

    /*�����ź�*/
    GPIO_InitStructure.Pin = DCMI_D0_GPIO_PIN;
    GPIO_InitStructure.Alternate = DCMI_D0_AF;
    HAL_GPIO_Init(DCMI_D0_GPIO_PORT, &GPIO_InitStructure);

    GPIO_InitStructure.Pin = DCMI_D1_GPIO_PIN;
    GPIO_InitStructure.Alternate = DCMI_D1_AF;
    HAL_GPIO_Init(DCMI_D1_GPIO_PORT, &GPIO_InitStructure);

    GPIO_InitStructure.Pin = DCMI_D2_GPIO_PIN;
    GPIO_InitStructure.Alternate = DCMI_D2_AF;
    HAL_GPIO_Init(DCMI_D2_GPIO_PORT, &GPIO_InitStructure);

    GPIO_InitStructure.Pin = DCMI_D3_GPIO_PIN;
    GPIO_InitStructure.Alternate = DCMI_D3_AF;
    HAL_GPIO_Init(DCMI_D3_GPIO_PORT, &GPIO_InitStructure);

    GPIO_InitStructure.Pin = DCMI_D4_GPIO_PIN;
    GPIO_InitStructure.Alternate = DCMI_D4_AF;
    HAL_GPIO_Init(DCMI_D4_GPIO_PORT, &GPIO_InitStructure);

    GPIO_InitStructure.Pin = DCMI_D5_GPIO_PIN;
    GPIO_InitStructure.Alternate = DCMI_D5_AF;
    HAL_GPIO_Init(DCMI_D5_GPIO_PORT, &GPIO_InitStructure);

    GPIO_InitStructure.Pin = DCMI_D6_GPIO_PIN;
    GPIO_InitStructure.Alternate = DCMI_D6_AF;
    HAL_GPIO_Init(DCMI_D6_GPIO_PORT, &GPIO_InitStructure);

    GPIO_InitStructure.Pin = DCMI_D7_GPIO_PIN;
    GPIO_InitStructure.Alternate = DCMI_D7_AF;
    HAL_GPIO_Init(DCMI_D7_GPIO_PORT, &GPIO_InitStructure);

    GPIO_InitStructure.Pin = DCMI_PWDN_GPIO_PIN;
    GPIO_InitStructure.Mode = GPIO_MODE_OUTPUT_PP;    
    HAL_GPIO_Init(DCMI_PWDN_GPIO_PORT, &GPIO_InitStructure);

    GPIO_InitStructure.Pin = DCMI_RST_GPIO_PIN;
    GPIO_InitStructure.Mode = GPIO_MODE_OUTPUT_PP;    
    HAL_GPIO_Init(DCMI_RST_GPIO_PORT, &GPIO_InitStructure);

    I2CMaster_Init();
    HAL_GPIO_WritePin(DCMI_RST_GPIO_PORT,DCMI_RST_GPIO_PIN,GPIO_PIN_RESET);
    /*PWDN���ţ��ߵ�ƽ�رյ�Դ���͵�ƽ����*/
    HAL_GPIO_WritePin(DCMI_PWDN_GPIO_PORT,DCMI_PWDN_GPIO_PIN,GPIO_PIN_SET);
   
    HAL_GPIO_WritePin(DCMI_PWDN_GPIO_PORT,DCMI_PWDN_GPIO_PIN,GPIO_PIN_RESET);
    Delay(10);
    HAL_GPIO_WritePin(DCMI_RST_GPIO_PORT,DCMI_RST_GPIO_PIN,GPIO_PIN_SET);
    //������ʱ50ms,ģ��Ż���������
    Delay(50);
}
/**
  * @brief  Resets the OV5640 camera.
  * @param  None
  * @retval None
  */
void OV5640_Reset(void)
{
	/*OV5640�����λ*/
  OV5640_WriteReg(0x3008, 0x80);
}

/**
  * @brief  ��ȡ����ͷ��ID.
  * @param  OV5640ID: �洢ID�Ľṹ��
  * @retval None
  */
void OV5640_ReadID(OV5640_IDTypeDef *OV5640ID)
{

	/*��ȡ�Ĵ�оƬID*/
  OV5640ID->PIDH = OV5640_ReadReg(OV5640_SENSOR_PIDH);
  OV5640ID->PIDL = OV5640_ReadReg(OV5640_SENSOR_PIDL);
}

/**
  * @brief  ���� DCMI/DMA �Բ�������ͷ����
  * @param  None
  * @retval None
  */
void OV5640_Init(void) 
{
  /*** ����DCMI�ӿ� ***/
  /* ʹ��DCMIʱ�� */
  __HAL_RCC_DCMI_CLK_ENABLE();

  /* DCMI ����*/
  //DCMI����Ĵ�������ַ
  DCMI_Handle.Instance              = DCMI;    
  //�����ɼ�ģʽ  
  DCMI_Handle.Init.SynchroMode      = DCMI_MODE_CONTINUOUS;
  //�����ɼ�ģʽ  
  DCMI_Handle.Init.SynchroMode      = DCMI_SYNCHRO_HARDWARE;
  //����ʱ����������Ч  
  DCMI_Handle.Init.PCKPolarity      = DCMI_PCKPOLARITY_RISING;
  //VSP�ߵ�ƽ��Ч  
  DCMI_Handle.Init.VSPolarity       = DCMI_VSPOLARITY_HIGH;
  //HSP�͵�ƽ��Ч    
  DCMI_Handle.Init.HSPolarity       = DCMI_HSPOLARITY_LOW;
  //ȫ�ɼ�  
  DCMI_Handle.Init.CaptureRate      = DCMI_CR_ALL_FRAME;
  //8λ���ݿ��  
  DCMI_Handle.Init.ExtendedDataMode = DCMI_EXTEND_DATA_8B;
  HAL_DCMI_Init(&DCMI_Handle); 	
    
	/* �����ж� */
  HAL_NVIC_SetPriority(DCMI_IRQn, 7, 0);
  HAL_NVIC_EnableIRQ(DCMI_IRQn); 	
  //dma_memory ��16λ����Ϊ��λ�� dma_bufsize��32λ����Ϊ��λ(�����ظ���/2)
  OV5640_DMA_Config((uint32_t)CamDialog.cam_buff0,cam_mode.cam_out_height*cam_mode.cam_out_width/2);	
}


/**
  * @brief  ���� DCMI/DMA �Բ�������ͷ����
	* @param  DMA_Memory0BaseAddr:���δ����Ŀ���׵�ַ
  * @param DMA_BufferSize�����δ����������(��λΪ��,��4�ֽ�)
  */
void OV5640_DMA_Config(uint32_t DMA_Memory0BaseAddr,uint32_t DMA_BufferSize)
{
  /* ����DMA��DCMI�л�ȡ����*/
  /* ʹ��DMA*/
  __HAL_RCC_DMA2_CLK_ENABLE(); 
  DMA_Handle_dcmi.Instance = DMA2_Stream1;
  DMA_Handle_dcmi.Init.Request = DMA_REQUEST_DCMI; 
  DMA_Handle_dcmi.Init.Direction = DMA_PERIPH_TO_MEMORY;
  DMA_Handle_dcmi.Init.PeriphInc = DMA_PINC_DISABLE;
  DMA_Handle_dcmi.Init.MemInc = DMA_MINC_ENABLE;    //�Ĵ�����ַ����
  DMA_Handle_dcmi.Init.PeriphDataAlignment = DMA_PDATAALIGN_WORD;
  DMA_Handle_dcmi.Init.MemDataAlignment = DMA_MDATAALIGN_WORD;
  DMA_Handle_dcmi.Init.Mode = DMA_CIRCULAR;		    //ѭ��ģʽ
  DMA_Handle_dcmi.Init.Priority = DMA_PRIORITY_HIGH;
  DMA_Handle_dcmi.Init.FIFOMode = DMA_FIFOMODE_ENABLE;
  DMA_Handle_dcmi.Init.FIFOThreshold = DMA_FIFO_THRESHOLD_FULL;
  DMA_Handle_dcmi.Init.MemBurst = DMA_MBURST_INC4;
  DMA_Handle_dcmi.Init.PeriphBurst = DMA_PBURST_SINGLE;

  /*DMA�ж����� */
  __HAL_LINKDMA(&DCMI_Handle, DMA_Handle, DMA_Handle_dcmi);
  
  HAL_NVIC_SetPriority(DMA2_Stream1_IRQn, 2, 0);
  HAL_NVIC_EnableIRQ(DMA2_Stream1_IRQn);
  
  HAL_DMA_Init(&DMA_Handle_dcmi);
  
  //ʹ��DCMI�ɼ�����
  HAL_DCMI_Start_DMA(&DCMI_Handle, DCMI_MODE_CONTINUOUS, (uint32_t)DMA_Memory0BaseAddr,DMA_BufferSize);

}


unsigned short sensor_reg[(sizeof(RGB565_Init)/4)];
/**
  * @brief  Configures the OV5640 camera in BMP mode.
  * @param  BMP ImageSize: BMP image size
  * @retval None
  */
void OV5640_RGB565Config(void)
{
  uint32_t i;
  
	/*����ͷ��λ*/
  OV5640_Reset();
	
	Delay(50);

  /* д��Ĵ�������RGB565ģʽ */
	for(i=0; i<(sizeof(RGB565_Init)/4); i++)
  {
    OV5640_WriteReg(RGB565_Init[i][0], RGB565_Init[i][1]);
		
		Delay(5);

    sensor_reg[i] = OV5640_ReadReg(RGB565_Init[i][0]);
		
		CAMERA_DEBUG("sensor_reg[0x%x]:%x-%x\n",RGB565_Init[i][0],RGB565_Init[i][1],sensor_reg[i]);

  }

  Delay(100);
       
	//��Ĭ��ģʽ��ʼ��
	for(i=0; i<(sizeof(RGB565_WVGA)/2); i++)
	{
		OV5640_WriteReg(RGB565_WVGA[i][0], RGB565_WVGA[i][1]);
	}
}

/**δ����*/
/**
  * @brief  ����ͼ��ߴ��С,Ҳ������ѡ��ʽ������ֱ���
  * @param  width,height:ͼ���Ⱥ�ͼ��߶�
  * @retval 0,���óɹ�������,����ʧ��
  */
uint8_t OV5640_ImageSize_Set(uint16_t width,uint16_t height)
{
	uint8_t temp;
	OV5640_WriteReg(0XFF,0X00);
	OV5640_WriteReg(0XE0,0X04);
	OV5640_WriteReg(0XC0,(width)>>3&0XFF);		//����HSIZE��10:3λ
	OV5640_WriteReg(0XC1,(height)>>3&0XFF);		//����VSIZE��10:3λ
	temp=(width&0X07)<<3;
	temp|=height&0X07;
	temp|=(width>>4)&0X80;
	OV5640_WriteReg(0X8C,temp);
	OV5640_WriteReg(0XE0,0X00);
	return 0;
}
/**δ����*/
/**
  * @brief  ����ͼ���������
  * @param  sx,sy,��ʼ��ַ
						width,height:���(��Ӧ:horizontal)�͸߶�(��Ӧ:vertical)
  * @retval 0,���óɹ��� ����,����ʧ��
  */
void OV5640_Window_Set(uint16_t sx,uint16_t sy,uint16_t width,uint16_t height)
{
	uint16_t endx;
	uint16_t endy;
	uint8_t temp;
	endx=sx+width/2;	//V*2
 	endy=sy+height/2;

 	OV5640_WriteReg(0XFF,0X01);
	temp = OV5640_ReadReg(0X03);				//��ȡVref֮ǰ��ֵ
	temp&=0XF0;
	temp|=((endy&0X03)<<2)|(sy&0X03);
	OV5640_WriteReg(0X03,temp);				//����Vref��start��end�����2λ
	OV5640_WriteReg(0X19,sy>>2);			//����Vref��start��8λ
	OV5640_WriteReg(0X1A,endy>>2);			//����Vref��end�ĸ�8λ

	temp = OV5640_ReadReg(0X32);				//��ȡHref֮ǰ��ֵ
	temp&=0XC0;
	temp|=((endx&0X07)<<3)|(sx&0X07);
	OV5640_WriteReg(0X32,temp);				//����Href��start��end�����3λ
	OV5640_WriteReg(0X17,sx>>3);			//����Href��start��8λ
	OV5640_WriteReg(0X18,endx>>3);			//����Href��end�ĸ�8λ
}



//δ����
/**
  * @brief  ����ͼ�񿪴���С
						��:OV5640_ImageSize_Setȷ������������ֱ��ʴӴ�С.
						�ú������������Χ������п���,����OV5640_OutSize_Set�����
						ע��:�������Ŀ�Ⱥ͸߶�,������ڵ���OV5640_OutSize_Set�����Ŀ�Ⱥ͸߶�
						     OV5640_OutSize_Set���õĿ�Ⱥ͸߶�,���ݱ��������õĿ�Ⱥ͸߶�,��DSP
						     �Զ��������ű���,������ⲿ�豸.
  * @param  width,height:���(��Ӧ:horizontal)�͸߶�(��Ӧ:vertical),width��height������4�ı���
  * @retval 0,���óɹ��� ����,����ʧ��
  */
uint8_t OV5640_ImageWin_Set(uint16_t offx,uint16_t offy,uint16_t width,uint16_t height)
{
	uint16_t hsize;
	uint16_t vsize;
	uint8_t temp;
	if(width%4)return 1;
	if(height%4)return 2;
	hsize=width/4;
	vsize=height/4;
	OV5640_WriteReg(0XFF,0X00);
	OV5640_WriteReg(0XE0,0X04);
	OV5640_WriteReg(0X51,hsize&0XFF);		//����H_SIZE�ĵͰ�λ
	OV5640_WriteReg(0X52,vsize&0XFF);		//����V_SIZE�ĵͰ�λ
	OV5640_WriteReg(0X53,offx&0XFF);		//����offx�ĵͰ�λ
	OV5640_WriteReg(0X54,offy&0XFF);		//����offy�ĵͰ�λ
	temp=(vsize>>1)&0X80;
	temp|=(offy>>4)&0X70;
	temp|=(hsize>>5)&0X08;
	temp|=(offx>>8)&0X07;
	OV5640_WriteReg(0X55,temp);				//����H_SIZE/V_SIZE/OFFX,OFFY�ĸ�λ
	OV5640_WriteReg(0X57,(hsize>>2)&0X80);	//����H_SIZE/V_SIZE/OFFX,OFFY�ĸ�λ
	OV5640_WriteReg(0XE0,0X00);
	return 0;
}
/**
  * @brief  ����֡��
  * @param  ��ʹ�ú꣺FRAME_RATE_30FPS��FRAME_RATE_15FPS
  * @retval ��
  */
void OV5640_FrameRate_Set(uint8_t frame_rate)
{ 
	if(frame_rate == FRAME_RATE_30FPS)
	{
		OV5640_WriteReg(0x3035,0x21);		//X_ADDR_ST���ֽ�
    OV5640_WriteReg(0x3036,0x72);//X_ADDR_ST���ֽ�	

	}
	else if(frame_rate == FRAME_RATE_15FPS)
	{
		OV5640_WriteReg(0x3035,0x41);		//X_ADDR_ST���ֽ�
    OV5640_WriteReg(0x3036,0x72);//X_ADDR_ST���ֽ�	

	}

}

/**
  * @brief  ����ISPͼ���С����ISPͼ���ڴ������еĴ���
  *         ��С��ISPͼ��������ͼ��ɼ�Ƶ��(��Ҫ����ʱ��)
  * @param  x_st,y_st����ʼλ��ƫ��
	* @param  width��height: ��ȣ��߶�
  * @note   x_st+width <= 2592,y_st+height <= 1944
  * @retval ��
  */
void OV5640_ISPSize_Set(uint16_t x_st,uint16_t y_st,uint16_t width,uint16_t height)
{ 
//    OV5640_WriteReg(0X3212,0X03);  	
	
	  OV5640_WriteReg(0x3800,x_st>>8);		//X_ADDR_ST���ֽ�
    OV5640_WriteReg(0x3801,x_st&0xff);//X_ADDR_ST���ֽ�	
    OV5640_WriteReg(0x3802,y_st>>8);		//Y_ADDR_ST���ֽ�
    OV5640_WriteReg(0x3803,y_st&0xff);//Y_ADDR_ST���ֽ�
	
		OV5640_WriteReg(0x3804,(x_st+width)>>8);		//X_ADDR_END���ֽ�
    OV5640_WriteReg(0x3805,(x_st+width)&0xff);	//X_ADDR_END���ֽ�	
    OV5640_WriteReg(0x3806,(y_st+height)>>8);		//Y_ADDR_END���ֽ�
    OV5640_WriteReg(0x3807,(y_st+height)&0xff);//Y_ADDR_END���ֽ�

//    OV5640_WriteReg(0X3212,0X13);		
//    OV5640_WriteReg(0X3212,0Xa3);		
}
/**
  * @brief  ����ͼ��������С��λ�ã��Լ��Ƿ�ʹ���Զ����Ź���
  * @param  scaling:0,�ر��Զ����Ź��ܣ�1�������Զ����Ź���
  * @param  x_off,y_off:-�ر��Զ����Ź���ʱ��������ֵΪ���������ISPͼ���е�ƫ�ơ�
												 ������ʹ���Զ����Ź���ʱ���ǳ����׳���

                         ����ʹ��15fps��Բ����׳����������Ƽ�ʹ���Զ����Ź��ܡ�

                        -�����Զ����Ź���ʱ��������ֵ����Ϊ 16,4 һ��������ʹ�ã�
                        Ҳ�ɸ���������ڵĿ�߱������������Ƚϸ��ӣ�
                        ��ο���OV5640 �Զ��Խ�����ģ��Ӧ��ָ�ϡ���46ҳ
  * @param  width,height:ͼ���Ⱥ�ͼ��߶�
  * @retval ��
  */
void OV5640_OutSize_Set(uint8_t scaling,uint16_t x_off,uint16_t y_off,uint16_t width,uint16_t height)
{ 
		uint8_t reg_temp;
	
		//��ȡԭ�Ĵ�������
		reg_temp = OV5640_ReadReg(0x5001);
	
		//��ʹ���Զ����Ź���
		if(scaling == 0 )
		{
			OV5640_WriteReg(0x5001,reg_temp &~(1<<5) );	//scaling off		
		}
		else
		{
			OV5640_WriteReg(0x5001,reg_temp|(1<<5) );	//scaling on		
		}	
	
		OV5640_WriteReg(0x3810,x_off>>8);	//X offset���ֽ�
		OV5640_WriteReg(0x3811,x_off&0xff);//X offset���ֽ�	
		OV5640_WriteReg(0x3812,y_off>>8);	//Y offset���ֽ�
		OV5640_WriteReg(0x3813,y_off&0xff);//Y offset���ֽ�
		
	
    OV5640_WriteReg(0X3212,0X03);  	
	
    OV5640_WriteReg(0x3808,width>>8);	//�����ȸ��ֽ�
    OV5640_WriteReg(0x3809,width&0xff);//�����ȵ��ֽ�  
    OV5640_WriteReg(0x380a,height>>8);//����߶ȸ��ֽ�
    OV5640_WriteReg(0x380b,height&0xff);//����߶ȵ��ֽ�
	
    OV5640_WriteReg(0X3212,0X13);		
    OV5640_WriteReg(0X3212,0Xa3);		
}
//ɫ�ʱ��ͶȲ�����
const static uint8_t OV5640_Saturation_reg[][6]=
{ 
	0X0C,0x30,0X3D,0X3E,0X3D,0X01,//-3 
	0X10,0x3D,0X4D,0X4E,0X4D,0X01,//-2  
	0X15,0x52,0X66,0X68,0X66,0X02,//-1  
	0X1A,0x66,0X80,0X82,0X80,0X02,//0  
	0X1F,0x7A,0X9A,0X9C,0X9A,0X02,//+1  
	0X24,0x8F,0XB3,0XB6,0XB3,0X03,//+2
	0X2B,0xAB,0XD6,0XDA,0XD6,0X04,//+3
}; 
/**
  * @brief  ���ñ��Ͷ�
  * @param  sat:���Ͷ�,������Χ[-3 ~ +3]             	
  * @retval ��
  */
void OV5640_Color_Saturation(int8_t sat)
{
	uint8_t i;
	
	OV5640_WriteReg(0x3212, 0x03); // start group 3
	OV5640_WriteReg(0x5381, 0x1c);	
	OV5640_WriteReg(0x5382, 0x5a);
	OV5640_WriteReg(0x5383, 0x06);
	
	for(i=0;i<6;i++)
	{
		OV5640_WriteReg(0x5384+i, OV5640_Saturation_reg[sat+3][i]);
	}
	
	OV5640_WriteReg(0x538b, 0x98);
	OV5640_WriteReg(0x538a, 0x01);	
	OV5640_WriteReg(0x3212, 0x13); // end group 3
	OV5640_WriteReg(0x3212, 0xa3); // launch group 3
	
}	
//�ԱȶȲ�����
const static uint8_t OV5640_Contrast_reg[][2]=
{
 0x2C,0x1C,
 0x28,0x18,
 0x24,0x10,
 0x20,0x00,
 0x1C,0x1C,
 0x18,0x18,
 0x14,0x14,
};

/**
  * @brief  ���öԱȶ�
	* @param  cnst:�Աȶȣ�������Χ[-3~+3]
  * @retval ��
  */
void OV5640_ContrastConfig(int8_t cnst)
{	
	OV5640_WriteReg(0x3212, 0x03); // start group 3
	OV5640_WriteReg(0x5586, OV5640_Contrast_reg[cnst+3][0]);
	OV5640_WriteReg(0x5585, OV5640_Contrast_reg[cnst+3][1]);
	OV5640_WriteReg(0x3212, 0x13); // end group 3
	OV5640_WriteReg(0x3212, 0xa3); // launch group 3
}
//�عⲹ�����ò�����
const static uint8_t OV5640_Exposure_reg[][6]=
{
    0x10,0x08,0x10,0x08,0x20,0x10,//-3  
    0x20,0x18,0x41,0x20,0x18,0x10,//-2
    0x30,0x28,0x61,0x30,0x28,0x10,//-1 
    0x38,0x30,0x61,0x38,0x30,0x10,//0  
    0x40,0x38,0x71,0x40,0x38,0x10,//+1 
    0x50,0x48,0x90,0x50,0x48,0x20,//+2   
    0x60,0x58,0xa0,0x60,0x58,0x20,//+3    
};

/**
  * @brief  �����عⲹ��
  * @param  ev:�عⲹ���ȼ���������Χ[-3 ~ +3]             	
  * @retval ��
  */
void OV5640_Exposure(int8_t ev)
{
	OV5640_WriteReg(0x3212, 0x03); // start group 3

	OV5640_WriteReg(0x3a0f, OV5640_Exposure_reg[ev+3][0]);
	OV5640_WriteReg(0x3a10, OV5640_Exposure_reg[ev+3][1]);
	OV5640_WriteReg(0x3a11, OV5640_Exposure_reg[ev+3][2]);
	OV5640_WriteReg(0x3a1b, OV5640_Exposure_reg[ev+3][3]);
	OV5640_WriteReg(0x3a1e, OV5640_Exposure_reg[ev+3][4]);
	OV5640_WriteReg(0x3a1f, OV5640_Exposure_reg[ev+3][5]);
	
	OV5640_WriteReg(0x3212, 0x13); // end group 3
	OV5640_WriteReg(0x3212, 0xa3); // launch group 3

}

/**
  * @brief  ʹ��cammode������ʼ����������
  * @param  None        	
  * @retval None
  */
void OV5640_USER_Config(void)
{	
	OV5640_FrameRate_Set(cam_mode.frame_rate);
	Delay(100);

//	OV5640_ISPSize_Set(cam_mode.cam_isp_sx,
//											 cam_mode.cam_isp_sy,
//											 cam_mode.cam_isp_width,
//											 cam_mode.cam_isp_height);
//	Delay(100);

  OV5640_OutSize_Set(cam_mode.scaling,
												cam_mode.cam_out_sx,
												cam_mode.cam_out_sy,
												cam_mode.cam_out_width,
												cam_mode.cam_out_height);
	Delay(100);
	OV5640_BrightnessConfig(cam_mode.brightness);
	Delay(100);

	OV5640_Color_Saturation(cam_mode.saturation);
	Delay(100);

	OV5640_ContrastConfig(cam_mode.contrast);
	Delay(100);

	OV5640_Exposure(cam_mode.exposure);
	Delay(100);

	OV5640_LightMode(cam_mode.light_mode);
	Delay(100);

	OV5640_SpecialEffects(cam_mode.effect);	
	Delay(100);

//	Delay(500);
}
//������ģʽ������
const static uint8_t OV5640_LightMode_reg[][7]=
{ 
	0x04,0X00,0X04,0X00,0X04,0X00,0X00,//Auto,�Զ� 
	0x06,0X1C,0X04,0X00,0X04,0XF3,0X01,//Sunny,�չ�
	0x05,0X48,0X04,0X00,0X07,0XCF,0X01,//Office,�칫��
	0x06,0X48,0X04,0X00,0X04,0XD3,0X01,//Cloudy,���� 
	0x04,0X10,0X04,0X00,0X08,0X40,0X01,//Home,����
}; 
/**
  * @brief  ���ù���ģʽ
  * @param  ��������ѡ�����ģʽ
  *         0x00 Auto     �Զ�
  *         0x01 Sunny    ����
  *         0x02 Cloudy   ����
  *         0x03 Office   �칫��
  *         0x04 Home     ����

  * @retval None
  */
void OV5640_LightMode(uint8_t mode)
{
   OV5640_WriteReg(0x3212, 0x03); // start group 3
	
	OV5640_WriteReg(0x3406, OV5640_LightMode_reg[mode][0]);
	OV5640_WriteReg(0x3400, OV5640_LightMode_reg[mode][1]);
	OV5640_WriteReg(0x3401, OV5640_LightMode_reg[mode][2]);
	OV5640_WriteReg(0x3402, OV5640_LightMode_reg[mode][3]);
	OV5640_WriteReg(0x3403, OV5640_LightMode_reg[mode][4]);
	OV5640_WriteReg(0x3404, OV5640_LightMode_reg[mode][5]);
	OV5640_WriteReg(0x3405, OV5640_LightMode_reg[mode][6]);
	
	OV5640_WriteReg(0x3212, 0x13); // end group 3
	OV5640_WriteReg(0x3212, 0xa3); // lanuch group 3
}
//��Ч���ò�����
const static uint8_t OV5640_Effect_reg[][4]=
{ 
	0X06,0x40,0X10,0X08,//���� 
	0X1E,0xA0,0X40,0X08,//��ɫ
	0X1E,0x80,0XC0,0X08,//ůɫ
	0X1E,0x80,0X80,0X08,//�ڰ�
	0X1E,0x40,0XA0,0X08,//���� 
	0X40,0x40,0X10,0X08,//��ɫ
	0X1E,0x60,0X60,0X08,//ƫ��
	0X1E,0xF0,0XF0,0X08,//����
	0X06,0x40,0X10,0X09,//����Ƭ����
};
/**
  * @brief  ����Ч��
  * @param  ��������ѡ�����ģʽ
	*   0x01  ���� 
	*   0x02  ��ɫ
	*   0x03  ůɫ
	*   0x04  �ڰ�
	*   0x05  ���� 
	*   0x06  ��ɫ
	*   0x07  ƫ��
	*   0x08  ����
	*   0x09  ����Ƭ����

  * @retval None
  */
void OV5640_SpecialEffects(uint8_t mode)
{
	OV5640_WriteReg(0x3212, 0x03); // start group 3
	
	OV5640_WriteReg(0x5580, OV5640_Effect_reg[mode][0]);
	OV5640_WriteReg(0x5583, OV5640_Effect_reg[mode][1]); // sat U
	OV5640_WriteReg(0x5584, OV5640_Effect_reg[mode][2]); // sat V
	OV5640_WriteReg(0x5003, OV5640_Effect_reg[mode][3]);
	
	OV5640_WriteReg(0x3212, 0x13); // end group 3
	OV5640_WriteReg(0x3212, 0xa3); // launch group 3
}


/**
  * @brief  Configures the OV5640 camera brightness.
  * @param  Brightness: Brightness value, where Brightness can be: 
  *         0x40 for Brightness +2,
  *         0x30 for Brightness +1,
  *         0x20 for Brightness 0,
  *         0x10 for Brightness -1,
  *         0x00 for Brightness -2,
  * @retval None
  */
void OV5640_BrightnessConfig(int8_t Brightness)
{
	OV5640_WriteReg(0x3212, 0x03); // start group 3
	
	OV5640_WriteReg(0x5587, (Brightness<<4)&0xF0);

	if(Brightness >= 0)	
	{
		OV5640_WriteReg(0x5588, 0x01);
	}
	else
	{
		OV5640_WriteReg(0x5588, 0x09);
	}
		
	OV5640_WriteReg(0x3212, 0x13); // end group 3
	OV5640_WriteReg(0x3212, 0xa3); // launch group 3
}

/**
  * @brief  Configures the OV5640 camera Black and white mode.
  * @param  BlackWhite: BlackWhite value, where BlackWhite can be: 
  *         0x18 for B&W,
  *         0x40 for Negative,
  *         0x58 for B&W negative,
  *         0x00 for Normal,
  * @retval None
  */
void OV5640_BandWConfig(uint8_t BlackWhite)
{
  OV5640_WriteReg(0xff, 0x00);
  OV5640_WriteReg(0x7c, 0x00);
  OV5640_WriteReg(0x7d, BlackWhite);
  OV5640_WriteReg(0x7c, 0x05);
  OV5640_WriteReg(0x7d, 0x80);
  OV5640_WriteReg(0x7d, 0x80);
}

/**
  * @brief  Configures the OV5640 camera color effects.
  * @param  value1: Color effects value1
  * @param  value2: Color effects value2
  *         where value1 and value2 can be: 
  *         value1 = 0x40, value2 = 0xa6 for Antique,
  *         value1 = 0xa0, value2 = 0x40 for Bluish,
  *         value1 = 0x40, value2 = 0x40 for Greenish,
  *         value1 = 0x40, value2 = 0xc0 for Reddish,
  * @retval None
  */
void OV5640_ColorEffectsConfig(uint8_t value1, uint8_t value2)
{
  OV5640_WriteReg(0xff, 0x00);
  OV5640_WriteReg(0x7c, 0x00);
  OV5640_WriteReg(0x7d, 0x18);
  OV5640_WriteReg(0x7c, 0x05);
  OV5640_WriteReg(0x7d, value1);
  OV5640_WriteReg(0x7d, value2);
}
 /**
  * @brief  ������ر�����ͷ�ɼ�
	* @param  ENABLE��DISABLE
  */
void OV5640_Capture_Control(FunctionalState state)
{
  switch(state)
  {
    case ENABLE:
    {
      //OV5640_DMA_Config((uint32_t)CamDialog.cam_buff0,cam_mode.cam_out_height*cam_mode.cam_out_width);	   
      OV5640_Init();
      break;
    }
    case DISABLE:
    {
      HAL_DCMI_Stop(&DCMI_Handle);
      HAL_DMA_Abort(&DMA_Handle_dcmi);		
			
      HAL_DCMI_DeInit(&DCMI_Handle);
      break;
    }
  }
  
//  	DCMI_Cmd(state); 						//DCMI�ɼ�����
//		DCMI_CaptureCmd(state);//DCMI����  
//		DMA_Cmd(DMA2_Stream1, state);//DMA2,Stream1

}
//DMA2D_HandleTypeDef h_dma2d;
//static void CopyImageToLcdFrameBuffer(void *pSrc, void *pDst, uint32_t xSize, uint32_t ySize)
//{
// __HAL_UNLOCK(&h_dma2d);
//  h_dma2d.Init.Mode          = DMA2D_M2M;//DMA2D�ڴ浽�ڴ�ģʽ
//  h_dma2d.Init.ColorMode     = DMA2D_OUTPUT_RGB565; //���RGB565��ʽ
//  h_dma2d.Init.AlphaInverted = DMA2D_REGULAR_ALPHA;//����Ҫ͸����
//  h_dma2d.Init.RedBlueSwap   = DMA2D_RB_REGULAR;//������RB��λ��       

//  h_dma2d.Init.OutputOffset = 0;//���ƫ��Ϊ0
//  //����ǰ����
//  h_dma2d.LayerCfg[1].AlphaMode      = DMA2D_NO_MODIF_ALPHA;//
//  h_dma2d.LayerCfg[1].InputAlpha     = 0xFF; //��ȫ��͸��
//  h_dma2d.LayerCfg[1].InputColorMode = DMA2D_INPUT_RGB565;//������ɫ��ʽ
//  h_dma2d.LayerCfg[1].InputOffset    = 0;//ƫ��Ϊ0
//  h_dma2d.LayerCfg[1].RedBlueSwap    = DMA2D_RB_REGULAR; //������RB��λ��  
//  h_dma2d.LayerCfg[1].AlphaInverted  = DMA2D_REGULAR_ALPHA; //����Ҫ͸����
//  h_dma2d.XferCpltCallback  = TransferComplete;
//  h_dma2d.Instance = DMA2D;

//  //��ʼ��DMA2D
//  HAL_DMA2D_Init(&h_dma2d);
//  //����DMA2Dǰ����
//  HAL_DMA2D_ConfigLayer(&h_dma2d, 1);
//  
//  HAL_NVIC_SetPriority(DMA2D_IRQn, 0, 0);
//  HAL_NVIC_EnableIRQ(DMA2D_IRQn); 
//  //SCB_CleanDCache();  
//  //�����жϴ���
//  HAL_DMA2D_Start_IT(&h_dma2d, (uint32_t)pSrc, (uint32_t)pDst, xSize, ySize); 
//  
//  

//}

//int i = 0;
//void DMA2D_IRQHandler(void)
//{
//  i++;
//  HAL_DMA2D_IRQHandler(&h_dma2d);
//}
/**
  * @brief  DCMI֡ͬ���жϻص����� Line event callback.
  * @param  None
  * @retval None
  */
//uint8_t fps;count*BLOCKSIZE + ((uint32_t)buff - alignedAddr)
void HAL_DCMI_VsyncEventCallback(DCMI_HandleTypeDef *hdcmi)
{
    CamDialog.fps++; //֡�ʼ���

    GUI_SemPostISR(cam_sem);  

	if(cur_index == 0)//0--׼�����õڶ����ڴ棬��ǰʹ�õ��ǵ�һ���ڴ�
	 {
		  cur_index = 1;
			if (QR_Task)
			{
//				SCB_InvalidateDCache_by_Addr((uint32_t *)CamDialog.cam_buff0,cam_mode.cam_out_width * cam_mode.cam_out_height *2);
				cur_index = 0;
		    HAL_DCMI_Suspend(&DCMI_Handle);
        __HAL_DCMI_DISABLE(hdcmi);
				SCB_InvalidateDCache_by_Addr((uint32_t *)CamDialog.cam_buff0, cam_mode.cam_out_height*cam_mode.cam_out_width / 2);
        get_image((uint32_t)CamDialog.cam_buff0,cam_mode.cam_out_width , cam_mode.cam_out_height);//�ӻ���õĵ�һ���ڴ��л�ȡͼ������
				/*���¿�ʼ�ɼ�*/
				 HAL_DCMI_Resume(&DCMI_Handle);
//				 __HAL_DCMI_ENABLE(hdcmi);
       
				OV5640_DMA_Config((uint32_t)CamDialog.cam_buff0,
													cam_mode.cam_out_height*cam_mode.cam_out_width/2); 
			}
			else
			{
//				SCB_InvalidateDCache_by_Addr((uint32_t *)CamDialog.cam_buff0,cam_mode.cam_out_width * cam_mode.cam_out_height *2);
				cur_index = 1;
				OV5640_DMA_Config((uint32_t)CamDialog.cam_buff1,
													cam_mode.cam_out_height*cam_mode.cam_out_width/2);  
			}
		}
	else//1--���õ�һ���ڴ棬ʹ�õڶ����ڴ�
		{      
//			SCB_InvalidateDCache_by_Addr((uint32_t *)CamDialog.cam_buff0,cam_mode.cam_out_width * cam_mode.cam_out_height *2);
			cur_index = 0;
			OV5640_DMA_Config((uint32_t)CamDialog.cam_buff0,
												cam_mode.cam_out_height*cam_mode.cam_out_width/2);       
		}

}



/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
