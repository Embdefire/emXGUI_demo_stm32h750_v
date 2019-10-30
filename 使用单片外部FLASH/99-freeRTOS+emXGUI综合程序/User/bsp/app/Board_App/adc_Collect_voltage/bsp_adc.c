/**
  ******************************************************************
  * @file    bsp_adcd.c
  * @author  fire
  * @version V1.1
  * @date    2018-xx-xx
  * @brief   adcӦ�ú����ӿ�
  ******************************************************************
  * @attention
  *
  * ʵ��ƽ̨:Ұ�� STM32H750������ 
  * ��˾    :http://www.embedfire.com
  * ��̳    :http://www.firebbs.cn
  * �Ա�    :https://fire-stm32.taobao.com
  *
  ******************************************************************
  */
#include "./bsp_adc.h" 

extern double ADC_vol;

ADC_HandleTypeDef Init_ADC_Handle;
DMA_HandleTypeDef hdma_adc;
uint16_t ADC_ConvertedValue;


/**
  * @brief  ADC�������ú���
  * @param  ��
  * @retval ��
  */  
static void ADC_GPIO_Mode_Config(void)
{
    /* ����һ��GPIO_InitTypeDef���͵Ľṹ�� */
    GPIO_InitTypeDef  GPIO_InitStruct;
    /* ʹ��ADC���ŵ�ʱ�� */
    RHEOSTAT_ADC_GPIO_CLK_ENABLE();
    
    GPIO_InitStruct.Mode = GPIO_MODE_ANALOG; 
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Pin = RHEOSTAT_ADC_PIN; 
    /* ����Ϊģ�����룬����Ҫ�������� */ 
    HAL_GPIO_Init(RHEOSTAT_ADC_GPIO_PORT, &GPIO_InitStruct);
  
}

/**
  * @brief  ADC����ģʽ���ú���
  * @param  ��
  * @retval ��
  */ 
static void ADC_Mode_Config(void)
{
    ADC_ChannelConfTypeDef ADC_Config;
  
    RCC_PeriphCLKInitTypeDef RCC_PeriphClkInit;  
	
	HAL_ADC_DeInit(&Init_ADC_Handle);
    /*            ����ADC3ʱ��Դ             */
    /*    HSE Frequency(Hz)    = 25000000   */                                             
    /*         PLL_M                = 5     */
    /*         PLL_N                = 160   */
    /*         PLL_P                = 25    */
    /*         PLL_Q                = 2     */
    /*         PLL_R                = 2     */
    /*     ADC_ker_clk         = 32000000   */
		RCC_PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_ADC;
//    RCC_PeriphClkInit.PLL3.PLL3FRACN = 0;
//    RCC_PeriphClkInit.PLL2.PLL2M = 5;
//    RCC_PeriphClkInit.PLL3.PLL3N = 144;
//    RCC_PeriphClkInit.PLL3.PLL3P = 2;
//    RCC_PeriphClkInit.PLL2.PLL2Q = 2;
//    RCC_PeriphClkInit.PLL2.PLL2R = 2;
//    RCC_PeriphClkInit.PLL3.PLL3RGE = RCC_PLL3VCIRANGE_2;
//    RCC_PeriphClkInit.PLL3.PLL3VCOSEL = RCC_PLL3VCOWIDE;
//    RCC_PeriphClkInit.AdcClockSelection = RCC_ADCCLKSOURCE_PLL2; 
		RCC_PeriphClkInit.AdcClockSelection = RCC_ADCCLKSOURCE_CLKP; 
		
		HAL_RCCEx_PeriphCLKConfig(&RCC_PeriphClkInit);  
 while (HAL_RCCEx_PeriphCLKConfig(&RCC_PeriphClkInit) != HAL_OK)
  {
		
  }
    /* ʹ��ADCʱ�� */
    RHEOSTAT_ADC_CLK_ENABLE();
    /* ʹ��DMAʱ�� */
    RHEOSTAT_ADC_DMA_CLK_ENABLE();
    
    //ѡ��DMA1��Stream1
    hdma_adc.Instance = RHEOSTAT_ADC_DMA_Base;
    //ADC1��DMA����
    hdma_adc.Init.Request = RHEOSTAT_ADC_DMA_Request;
    //���䷽������-���ڴ�
    hdma_adc.Init.Direction = DMA_PERIPH_TO_MEMORY;
    //�����ַ������
    hdma_adc.Init.PeriphInc = DMA_PINC_DISABLE;
    //�ڴ��ַ������
    hdma_adc.Init.MemInc = DMA_PINC_DISABLE;
    //�������ݿ�ȣ�����
    hdma_adc.Init.PeriphDataAlignment = DMA_PDATAALIGN_HALFWORD;
    //�ڴ����ݿ�ȣ�����
    hdma_adc.Init.MemDataAlignment = DMA_MDATAALIGN_HALFWORD;
    //DMAѭ������
    hdma_adc.Init.Mode = DMA_CIRCULAR;
    //DMA��������ȼ�����
    hdma_adc.Init.Priority = DMA_PRIORITY_LOW;
    //FIFOģʽ�ر�
    hdma_adc.Init.FIFOMode = DMA_FIFOMODE_DISABLE;
    //DMA��ʼ��
    HAL_DMA_Init(&hdma_adc);
    //hdma_adc��ADC_Handle.DMA_Handle����
    __HAL_LINKDMA(&Init_ADC_Handle,DMA_Handle,hdma_adc);    
      
    
		
    Init_ADC_Handle.Instance = RHEOSTAT_ADC;
    //ADCʱ��1��Ƶ
//    Init_ADC_Handle.Init.ClockPrescaler = ADC_CLOCK_ASYNC_DIV1;
    //ʹ������ת��ģʽ
    Init_ADC_Handle.Init.ContinuousConvMode = ENABLE;
    //���ݴ�������ݼĴ�����
    Init_ADC_Handle.Init.ConversionDataManagement = ADC_CONVERSIONDATA_DMA_CIRCULAR;
    //�رղ�����ת��ģʽ
    Init_ADC_Handle.Init.DiscontinuousConvMode = DISABLE;
    //����ת��
    Init_ADC_Handle.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
    //�������
    Init_ADC_Handle.Init.ExternalTrigConv = ADC_SOFTWARE_START;
    //�رյ͹����Զ��ȴ�
    Init_ADC_Handle.Init.LowPowerAutoWait = DISABLE;
    //�������ʱ������д��
    Init_ADC_Handle.Init.Overrun = ADC_OVR_DATA_OVERWRITTEN;
    //��ʹ�ܹ�����ģʽ
    Init_ADC_Handle.Init.OversamplingMode = DISABLE;
    //�ֱ���Ϊ��16bit
    Init_ADC_Handle.Init.Resolution = ADC_RESOLUTION_16B;
    //��ʹ�ܶ�ͨ��ɨ��
    Init_ADC_Handle.Init.ScanConvMode = DISABLE;
    //��ʼ�� ADC
    HAL_ADC_Init(&Init_ADC_Handle);
          
    //ʹ��ͨ��18
    ADC_Config.Channel = RHEOSTAT_ADC_CHANNEL;
    //ת��˳��Ϊ1
    ADC_Config.Rank = ADC_REGULAR_RANK_1;
    //��������Ϊ64.5������
    ADC_Config.SamplingTime = ADC_SAMPLETIME_64CYCLES_5;
    //��ʹ�ò������Ĺ���
    ADC_Config.SingleDiff = ADC_SINGLE_ENDED ;
    //����ADCͨ��
    HAL_ADC_ConfigChannel(&Init_ADC_Handle, &ADC_Config);    
    
    //ʹ��ADC1��2
    ADC_Enable(&Init_ADC_Handle);
    
    HAL_ADC_Start_DMA(&Init_ADC_Handle, (uint32_t*)&ADC_ConvertedValue, sizeof(ADC_ConvertedValue));
    
}
/**
  * @brief  ADC�ж����ȼ����ú���
  * @param  ��
  * @retval ��
  */  
void Rheostat_ADC_NVIC_Config(void)
{
    HAL_NVIC_SetPriority(Rheostat_ADC1_DMA_IRQ, 1, 0);
    HAL_NVIC_EnableIRQ(Rheostat_ADC1_DMA_IRQ);
}

/**
  * @brief  ADC��ʼ������
  * @param  ��
  * @retval ��
  */
void ADC_Init(void)
{
    
    ADC_GPIO_Mode_Config();
  
    ADC_Mode_Config();
  
		Rheostat_ADC_NVIC_Config();
	
    HAL_ADC_Start(&Init_ADC_Handle);
}

/**
  * @brief  ת������жϻص�������������ģʽ��
  * @param  AdcHandle : ADC���
  * @retval ��
  */
void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* AdcHandle)
{
  /* ��ȡ��� */
    ADC_ConvertedValue = HAL_ADC_GetValue(AdcHandle); 
}

void Rheostat_DISABLE(void)
{
	// ʹ��ADC DMA
	HAL_ADC_Stop(&Init_ADC_Handle);
	
	ADC_Disable(&Init_ADC_Handle);//��ֹͣ�ɼ�
	
	HAL_ADC_Stop_DMA(&Init_ADC_Handle);
	
}
void DMA1_Stream1_IRQHandler()
{
	HAL_DMA_IRQHandler(&hdma_adc);
}

/*********************************************END OF FILE**********************/


