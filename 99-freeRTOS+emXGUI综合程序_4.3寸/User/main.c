/**
  *********************************************************************
  * @file    main.c
  * @author  fire
  * @version V1.0
  * @date    2018-xx-xx
  * @brief   FreeRTOS V9.0.0  + STM32 �̼�������
  *********************************************************************
  * @attention
  *
  * ʵ��ƽ̨:Ұ�� STM32 ������ 
  * ��̳    :http://www.firebbs.cn
  * �Ա�    :https://fire-stm32.taobao.com
  *
  **********************************************************************
  */ 
 
/*
*************************************************************************
*                             ������ͷ�ļ�
*************************************************************************
*/ 
/* FreeRTOSͷ�ļ� */
#include "FreeRTOS.h"
#include "task.h"
/* ������Ӳ��bspͷ�ļ� */
#include "board.h"
#include "string.h"
#include <cm_backtrace.h>
#include "./bsp/mpu/bsp_mpu.h" 
#include "diskio.h"
//#include "Backend_RGBLED.h" 

/* hardfault��������Ҫ�Ķ��� */
#define HARDWARE_VERSION               "V1.0.0"
#define SOFTWARE_VERSION               "V0.1.0"

/**************************** ������ ********************************/
/* 
 * ��������һ��ָ�룬����ָ��һ�����񣬵����񴴽���֮�����;�����һ��������
 * �Ժ�����Ҫ��������������Ҫͨ�������������������������������Լ�����ô
 * ����������ΪNULL��
 */

/********************************** �ں˶����� *********************************/
/*
 * �ź�������Ϣ���У��¼���־�飬�����ʱ����Щ�������ں˵Ķ���Ҫ��ʹ����Щ�ں�
 * ���󣬱����ȴ����������ɹ�֮��᷵��һ����Ӧ�ľ����ʵ���Ͼ���һ��ָ�룬������
 * �ǾͿ���ͨ��������������Щ�ں˶���
 *
 * �ں˶���˵���˾���һ��ȫ�ֵ����ݽṹ��ͨ����Щ���ݽṹ���ǿ���ʵ��������ͨ�ţ�
 * �������¼�ͬ���ȸ��ֹ��ܡ�������Щ���ܵ�ʵ��������ͨ��������Щ�ں˶���ĺ���
 * ����ɵ�
 * 
 */


/******************************* ȫ�ֱ������� ************************************/
/*
 * ��������дӦ�ó����ʱ�򣬿�����Ҫ�õ�һЩȫ�ֱ�����
 */

/*
*************************************************************************
*                             ��������
*************************************************************************
*/
static void GUI_Thread_Entry(void* pvParameters);/* Test_Task����ʵ�� */
static void DEBUG_Thread_Entry(void* parameter);
static void MPU_Config(void);
void vApplicationStackOverflowHook(TaskHandle_t xTask,char * pcTaskName);

void BSP_Init(void);/* ���ڳ�ʼ�����������Դ */
/***********************************************************************
  * @ ������  �� BSP_Init
  * @ ����˵���� �弶�����ʼ�������а����ϵĳ�ʼ�����ɷ��������������
  * @ ����    ��   
  * @ ����ֵ  �� ��
  *********************************************************************/
void BSP_Init(void)
{
//	SCB->CACR|=1<<2;   //ǿ��D-Cache͸д,�粻����,ʵ��ʹ���п���������������	  

  /* ϵͳʱ�ӳ�ʼ����400MHz */
#if 0
  /* ����SDRAMΪNormal����,���ù���, ֱдģʽ*/  
	Board_MPU_Config(0,MPU_Normal_WT,0xD0000000,MPU_32MB);
	/* ����AXI RAMΪNormal����,���ù���, ֱдģʽ*/ 
	Board_MPU_Config(1,MPU_Normal_WT,0x24000000,MPU_512KB);
#endif
	/* ����SDRAMΪNormal����,���ù���, ֱдģʽ*/  
	Board_MPU_Config(0,MPU_Normal_WT,0xD0000000,MPU_32MB);
	
	/* ����AXI RAMΪNormal����,���ù���, ֱдģʽ*/ 
	Board_MPU_Config(1,MPU_Normal_WT,0x20000000,MPU_128KB);
  Board_MPU_Config(2,MPU_Normal_WT,0x00000000,MPU_64KB);
  Board_MPU_Config(3,MPU_Normal_WT,0x24000000,MPU_512KB);
  Board_MPU_Config(4,MPU_Normal_WT,0x08000000,MPU_2MB);
	
	MPU_Config();	
	
  /* Enable I-Cache */
  SCB_EnableICache(); 
  /* Enable D-Cache */
  SCB_EnableDCache();
	
	
  /*
	 * STM32�ж����ȼ�����Ϊ4����4bit��������ʾ��ռ���ȼ�����ΧΪ��0~15
	 * ���ȼ�����ֻ��Ҫ����һ�μ��ɣ��Ժ������������������Ҫ�õ��жϣ�
	 * ��ͳһ��������ȼ����飬ǧ��Ҫ�ٷ��飬�мɡ�
	 */
  HAL_NVIC_SetPriorityGrouping(NVIC_PRIORITYGROUP_4);

	/* Ӳ��BSP��ʼ��ͳͳ�����������LED�����ڣ�LCD�� */
//  SDRAM_Init();
	/* LED �˿ڳ�ʼ�� */
	LED_GPIO_Config();	
	
	/* usart �˿ڳ�ʼ�� */
  UARTx_Config();
	
  /* ������ʱ����ʼ��	*/
	TIM_Basic_Init();  
	
	/* wm8978 ��������ʼ��	*/
	if (wm8978_Init()==0)
  {
    printf("��ⲻ��WM8978оƬ!!!\n");
  }
	
	RTC_CLK_Config();
	if ( HAL_RTCEx_BKUPRead(&Rtc_Handle,RTC_BKP_DRX) != 0x32F2)
	{
		/* ����ʱ������� */
		RTC_TimeAndDate_Set();
	}
	else
	{
		/* ����Ƿ��Դ��λ */
		if (__HAL_RCC_GET_FLAG(RCC_FLAG_PORRST) != RESET)
		{
			printf("\r\n ������Դ��λ....\r\n");
		}
		/* ����Ƿ��ⲿ��λ */
		else if (__HAL_RCC_GET_FLAG(RCC_FLAG_PINRST) != RESET)
		{
			printf("\r\n �����ⲿ��λ....\r\n");
		}
		printf("\r\n ����Ҫ��������RTC....\r\n");    
		/* ʹ�� PWR ʱ�� */
		__HAL_RCC_RTC_ENABLE();
		/* PWR_CR:DBF��1��ʹ��RTC��RTC���ݼĴ����ͱ���SRAM�ķ��� */
		HAL_PWR_EnableBkUpAccess();
		/* �ȴ� RTC APB �Ĵ���ͬ�� */
		HAL_RTC_WaitForSynchro(&Rtc_Handle);
	} 
	
	MODIFY_REG(FMC_Bank1_R->BTCR[0],FMC_BCRx_MBKEN,0); //�ر�FMC_Bank1,��ȻLCD����.
	
  /*hardfault ��������ʼ��*/ 
  cm_backtrace_init("Fire_emxgui", HARDWARE_VERSION, SOFTWARE_VERSION);
  
}


/**
  * @brief  System Clock ����
  *         system Clock ��������: 
	*            System Clock source  = PLL (HSE)
	*            SYSCLK(Hz)           = 480000000 (CPU Clock)
	*            HCLK(Hz)             = 240000000 (AXI and AHBs Clock)
	*            AHB Prescaler        = 2
	*            D1 APB3 Prescaler    = 2 (APB3 Clock  100MHz)
	*            D2 APB1 Prescaler    = 2 (APB1 Clock  100MHz)
	*            D2 APB2 Prescaler    = 2 (APB2 Clock  100MHz)
	*            D3 APB4 Prescaler    = 2 (APB4 Clock  100MHz)
	*            HSE Frequency(Hz)    = 25000000
	*            PLL_M                = 5
	*            PLL_N                = 192
	*            PLL_P                = 2
	*             PLL_Q                = 4
	*            PLL_R                = 2
	*            VDD(V)               = 3.3
	*            Flash Latency(WS)    = 4
  * @param  None
  * @retval None
  */

/*****************************************************************
  * @brief  ������
  * @param  ��
  * @retval ��
  * @note   ��һ����������Ӳ����ʼ�� 
            �ڶ���������APPӦ������
            ������������FreeRTOS����ʼ���������
  ****************************************************************/
int main(void)
{	
  BaseType_t xReturn = pdPASS;/* ����һ��������Ϣ0����ֵ��Ĭ��ΪpdPASS */
  
  /* ������Ӳ����ʼ�� */
  BSP_Init();  
  
   /* ����AppTaskCreate���� */
  xReturn = xTaskCreate((TaskFunction_t )GUI_Thread_Entry,  /* ������ں��� */
                        (const char*    )"gui",/* �������� */
                        (uint16_t       )5*1024,  /* ����ջ��С */
                        (void*          )NULL,/* ������ں������� */
                        (UBaseType_t    )10, /* ��������ȼ� */
                        (TaskHandle_t*  )NULL);/* ������ƿ�ָ�� */ 

//	           xTaskCreate((TaskFunction_t )DEBUG_Thread_Entry,  /* ������ں��� */
//                        (const char*    )"DEBUG_Thread_Entry",/* �������� */
//                        (uint16_t       )2*1024,  /* ����ջ��С */
//                        (void*          )NULL,/* ������ں������� */
//                        (UBaseType_t    )2, /* ��������ȼ� */
//                        (TaskHandle_t*  )NULL);/* ������ƿ�ָ�� */ 
  /* ����������� */           
  if(pdPASS == xReturn)
    vTaskStartScheduler();   /* �������񣬿������� */
  else
    return -1;  
  
  while(1);   /* ��������ִ�е����� */    
}

extern void GUI_Startup(void);

/**********************************************************************
  * @ ������  �� gui_thread_entry
  * @ ����˵���� gui_thread_entry��������
  * @ ����    ��   
  * @ ����ֵ  �� ��
  ********************************************************************/
static void GUI_Thread_Entry(void* parameter)
{	
//  uint8_t CPU_RunInfo[400];		//������������ʱ����Ϣ
  printf("Ұ��emXGUI��ʾ����\n\n");
  /* ִ�б��������᷵�� */
	
	GUI_Startup();

  while (1)
  {
    LED1_ON;
    printf("Test_Task Running,LED1_ON\r\n");
    vTaskDelay(500);   /* ��ʱ500��tick */
    
    LED1_OFF;     
    printf("Test_Task Running,LED1_OFF\r\n");
    vTaskDelay(500);   /* ��ʱ500��tick */
  }
}

static void DEBUG_Thread_Entry(void* parameter)
{	
	char tasks_buf[512] = {0};
	
  while (1)
  {

    vTaskDelay(5000);   /* ��ʱ500��tick */
//	  uint16_t test_status = 0;
//		test_status =  disk_status(0);
//		printf("************************ test_status %d ************************\r\n",test_status);
{
	memset(tasks_buf, 0, 512);

	strcat((char *)tasks_buf, "��������    ���м���    ʹ����\r\n" );

	strcat((char *)tasks_buf, "---------------------------------------------\r\n");

	/* displays the amount of time each task has spent in the Running state

	* in both absolute and percentage terms. */

	vTaskGetRunTimeStats((char *)(tasks_buf + strlen(tasks_buf)));

	strcat((char *)tasks_buf, "\r\n");
	printf("%s\r\n",tasks_buf);
	
}
	memset(tasks_buf, 0, 512);

	strcat((char *)tasks_buf, "��������    ����״̬    ���ȼ�    ʣ���ջ    �������\r\n" );

	strcat((char *)tasks_buf, "---------------------------------------------\r\n");


{
	vTaskList((char *)(tasks_buf + strlen(tasks_buf)));

	strcat((char *)tasks_buf, "\r\n---------------------------------------------\r\n");


	strcat((char *)tasks_buf, "B : ����, R : ����, D : ɾ��, S : ��ͣ\r\n");
	printf("%s\r\n",tasks_buf);
}
  }
}

static void MPU_Config(void)
{
  MPU_Region_InitTypeDef MPU_InitStruct;

  /* Disable the MPU */
  HAL_MPU_Disable();

  /* Configure the MPU attributes as Device not cacheable 
     for ETH DMA descriptors */
  MPU_InitStruct.Enable = MPU_REGION_ENABLE;
  MPU_InitStruct.BaseAddress = 0x30040000;
  MPU_InitStruct.Size = MPU_REGION_SIZE_256B;
  MPU_InitStruct.AccessPermission = MPU_REGION_FULL_ACCESS;
  MPU_InitStruct.IsBufferable = MPU_ACCESS_BUFFERABLE;
  MPU_InitStruct.IsCacheable = MPU_ACCESS_NOT_CACHEABLE;
  MPU_InitStruct.IsShareable = MPU_ACCESS_NOT_SHAREABLE;
  MPU_InitStruct.Number = MPU_REGION_NUMBER6;
  MPU_InitStruct.TypeExtField = MPU_TEX_LEVEL0;
  MPU_InitStruct.SubRegionDisable = 0x00;
  MPU_InitStruct.DisableExec = MPU_INSTRUCTION_ACCESS_ENABLE;

  HAL_MPU_ConfigRegion(&MPU_InitStruct);

  /* Configure the MPU attributes as Cacheable write through 
     for LwIP RAM heap which contains the Tx buffers */
  MPU_InitStruct.Enable = MPU_REGION_ENABLE;
  MPU_InitStruct.BaseAddress = 0x30044000;
  MPU_InitStruct.Size = MPU_REGION_SIZE_16KB;
  MPU_InitStruct.AccessPermission = MPU_REGION_FULL_ACCESS;
  MPU_InitStruct.IsBufferable = MPU_ACCESS_NOT_BUFFERABLE;
  MPU_InitStruct.IsCacheable = MPU_ACCESS_CACHEABLE;
  MPU_InitStruct.IsShareable = MPU_ACCESS_NOT_SHAREABLE;
  MPU_InitStruct.Number = MPU_REGION_NUMBER7;
  MPU_InitStruct.TypeExtField = MPU_TEX_LEVEL0;
  MPU_InitStruct.SubRegionDisable = 0x00;
  MPU_InitStruct.DisableExec = MPU_INSTRUCTION_ACCESS_ENABLE;

  HAL_MPU_ConfigRegion(&MPU_InitStruct);

  /* Enable the MPU */
  HAL_MPU_Enable(MPU_PRIVILEGED_DEFAULT);
}

void vApplicationStackOverflowHook(TaskHandle_t xTask,
                                     char * pcTaskName)
{
	printf(" Heap Overflow! Cheak : %s \r\n",pcTaskName);
	while(1);
}

/********************************END OF FILE****************************/
