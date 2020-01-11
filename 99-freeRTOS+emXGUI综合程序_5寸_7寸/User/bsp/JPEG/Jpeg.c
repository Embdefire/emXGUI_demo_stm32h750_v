#include "Jepg.h"

#define CHUNK_SIZE_IN  ((uint32_t)(64 * 1024)) /* һ��Ҫ��������ݿ��С */
#define CHUNK_SIZE_OUT ((uint32_t)(64 * 1024)) 

uint32_t *Decode_Buffer;	/* ���ļ�ϵͳ�ж�ȡ�������ݻ����� */
uint32_t *OUT_Buffer; 		/* Ӳ��������ݴ�ŵĻ����� */

uint32_t JPEG_Source_Address;		/* JPEG����Դ��ַ */
uint32_t Frame_Buffer_Address;	/* ��������ݴ�ŵĻ����� */
uint32_t Input_Frame_Size;			/* JPEGԴ���ݴ�С */
uint32_t Input_Frame_Index;			/* JPEG������,�Ѿ���������ݴ�С */

__IO uint8_t Jpeg_HWDecodingEnd = 0;		/* ����������ɱ�־ */

JPG_DEC	*Decode_dec;		/* ͼƬ��Ϣ���,���ͼƬ��BUFF�׵�ַ,��С��Ϣ */

struct	__jpg_dec
{
	const void	*pSrcData;
	U32		DataSize;	
};

static DMA2D_HandleTypeDef    DMA2D_Handle;		/* DMA2D��� */


void JPEG_picture(uint32_t *databuf)
{
	JPEG_Source_Address = (uint32_t)Decode_dec->pSrcData;		/* ��DEC�л�ȡ�����뻺�����׵�ַ */
	Input_Frame_Size = Decode_dec->DataSize;								/* ��DEC�л�ȡ���������ݴ�С(ͼƬ��С) */
	Frame_Buffer_Address = (uint32_t)databuf;								/* ��ֵ��������ݵĴ�Ż����� */
	Input_Frame_Index = 0;		/* ��λ����ƫ���� */
	Jpeg_HWDecodingEnd = 0;		/* ������ɱ�־ */
	HAL_JPEG_Decode_DMA(&JPEG_Handle ,(uint8_t *)JPEG_Source_Address,CHUNK_SIZE_IN ,(uint8_t *)Frame_Buffer_Address ,CHUNK_SIZE_OUT);		/* ��ʼ���� */
}

/*----------------------------���봦�����-------------------------------*/
static MDMA_HandleTypeDef   hmdmaIn;
static MDMA_HandleTypeDef   hmdmaOut; 

JPEG_HandleTypeDef    JPEG_Handle;		/* ��� */
JPEG_ConfTypeDef      JPEG_Info;			/* ������ȡ��JPEG��Ϣ��� */

/**
  * @brief  �������ݽ������,�����µ�δ�������ݻص�����
  * @param hjpeg: JPEG���ָ��
  * @param NbDecodedData: ������Ļ�����������������ݴ�С
  * @retval None
  */
void HAL_JPEG_GetDataCallback(JPEG_HandleTypeDef *hjpeg, uint32_t NbDecodedData)
{
	uint32_t Input_DataLength = 0; 
	
	/* �����Ѿ���������ݴ�С,Input_Frame_Index��ʶ��ǰ�����λ�� */
	Input_Frame_Index += NbDecodedData;
	
	/* ��ǰ�Ѿ����������С�����ļ���С,�������� */
	if( Input_Frame_Index < Input_Frame_Size)
	{
		/* ƫ�ƽ�������λ��,��δ��������ݿ�ʼ�������� */
		JPEG_Source_Address = JPEG_Source_Address + NbDecodedData;

		/* ����Ҫ��������ݴ�С */
		if((Input_Frame_Size - Input_Frame_Index) >= CHUNK_SIZE_IN)
		{
			Input_DataLength = CHUNK_SIZE_IN;
		}
		else
		{
			Input_DataLength = Input_Frame_Size - Input_Frame_Index;
		}    
	}
	else
	{
		/* ������ɻ��߽������,����0������,ֹͣ */
		Input_DataLength = 0; 
	}
	
	/* �������뻺�� */
	HAL_JPEG_ConfigInputBuffer(hjpeg,(uint8_t *)JPEG_Source_Address, Input_DataLength);  
}

/**
  * @brief  ���ݽ������,���������ݾ�������ص�����
  * @param hjpeg: JPEG���ָ��
  * @param pDataOut: ָ�������ݵĻ�������ַ
  * @param NbDecodedData: ������Ļ�����������������ݴ�С
  * @retval None
  */
void HAL_JPEG_DataReadyCallback (JPEG_HandleTypeDef *hjpeg, uint8_t *pDataOut, uint32_t OutDataLength)
{
	/* ��JPEG����������,��Ž�������,��ƫ�Ƴ���������ݿ��С */  
	Frame_Buffer_Address += OutDataLength;
	
	/* ������������� */  
	HAL_JPEG_ConfigOutputBuffer(hjpeg, (uint8_t *)Frame_Buffer_Address, CHUNK_SIZE_OUT); 
}

/**
  * @brief  JPEGȫ�����ݽ�����ɻص�����
  * @param hjpeg: JPEG ���ָ��
  * @retval None
  */
void HAL_JPEG_DecodeCpltCallback(JPEG_HandleTypeDef *hjpeg)
{
	/* ������ɱ�־λ,��λ */  
	Jpeg_HWDecodingEnd = 1;
}

/**
  * @brief  JPEG�ײ��ʼ����
  * @param hjpeg: JPEG ���ָ��
  * @retval None
  */
void HAL_JPEG_MspInit(JPEG_HandleTypeDef *hjpeg)
{
	__HAL_RCC_JPEG_CLK_ENABLE();
	
	/* ʹ�� JPEG ʱ�� */
	__HAL_RCC_JPGDECEN_CLK_ENABLE();

	/* ʹ�� MDMA ʱ�� */
	__HAL_RCC_MDMA_CLK_ENABLE();
	
	/* ʹ�� JPEG �ж����ȼ� */
	HAL_NVIC_SetPriority(JPEG_IRQn, 0x07, 0);
	HAL_NVIC_EnableIRQ(JPEG_IRQn);  

	/* ����� MDMA ����*/
	hmdmaIn.Instance                = MDMA_Channel7;												/* ����MDMA ͨ��*/
	hmdmaIn.Init.Priority           = MDMA_PRIORITY_HIGH;										/* ����MDMA ���ȼ�*/
	hmdmaIn.Init.Endianness         = MDMA_LITTLE_ENDIANNESS_PRESERVE;			/* ����MDMA ��С��ģʽ:С��*/
	hmdmaIn.Init.SourceInc          = MDMA_SRC_INC_BYTE;										/* ����MDMA Դ���ݵ�ַ��BYTE����*/
	hmdmaIn.Init.DestinationInc     = MDMA_DEST_INC_DISABLE;								/* ����MDMA Ŀ���ַ������*/
	hmdmaIn.Init.SourceDataSize     = MDMA_SRC_DATASIZE_BYTE;								/* ����MDMA ����Դ���ݴ�СΪBYTE*/
	hmdmaIn.Init.DestDataSize       = MDMA_DEST_DATASIZE_WORD;							/* ����MDMA Ŀ�����ݴ�СΪ4��BYTE*/
	hmdmaIn.Init.DataAlignment      = MDMA_DATAALIGN_PACKENABLE; 						/* ����MDMA ���ݰ�����*/  
	hmdmaIn.Init.SourceBurst        = MDMA_SOURCE_BURST_32BEATS;						/* ����MDMA ԴΪ32λͻ������*/
	hmdmaIn.Init.DestBurst          = MDMA_DEST_BURST_16BEATS; 							/* ����MDMA Ŀ��Ϊ16λͻ������*/
	hmdmaIn.Init.SourceBlockAddressOffset = 0;	/* ����MDMA Դ���ݻ�����ƫ��*/
	hmdmaIn.Init.DestBlockAddressOffset  = 0;		/* ����MDMA Ŀ�����ݻ�����ƫ��*/
	
	/*ʹ��JPEG����FIFO���䴥��ģʽ,�Կ���MDMA���� */
	
	/* ����FIFO��������ı�־Ϊ:��������ʱ����*/  
	hmdmaIn.Init.Request = MDMA_REQUEST_JPEG_INFIFO_TH; 
	hmdmaIn.Init.TransferTriggerMode = MDMA_BUFFER_TRANSFER;  
	hmdmaIn.Init.BufferTransferLength = 32; /* ��MDMA��������С����ΪJPEG FIFO��ֵ��С32���ֽ�(8��) */

	/* ����JPEG�����MDMA��� */
	__HAL_LINKDMA(hjpeg, hdmain, hmdmaIn);

	/* ����ʼ������DMA�� */
	HAL_MDMA_DeInit(&hmdmaIn);  
	/* ��ʼ������DMA�� */
	HAL_MDMA_Init(&hmdmaIn);

	/* ����� MDMA ����,ͬ������*/
	hmdmaOut.Instance             = MDMA_Channel6;
	hmdmaOut.Init.Priority        = MDMA_PRIORITY_VERY_HIGH;
	hmdmaOut.Init.Endianness      = MDMA_LITTLE_ENDIANNESS_PRESERVE;
	hmdmaOut.Init.SourceInc       = MDMA_SRC_INC_DISABLE;
	hmdmaOut.Init.DestinationInc  = MDMA_DEST_INC_BYTE;
	hmdmaOut.Init.SourceDataSize  = MDMA_SRC_DATASIZE_WORD;
	hmdmaOut.Init.DestDataSize    = MDMA_DEST_DATASIZE_BYTE;
	hmdmaOut.Init.DataAlignment   = MDMA_DATAALIGN_PACKENABLE;
	hmdmaOut.Init.SourceBurst     = MDMA_SOURCE_BURST_32BEATS;
	hmdmaOut.Init.DestBurst       = MDMA_DEST_BURST_32BEATS;  
	hmdmaOut.Init.SourceBlockAddressOffset = 0;
	hmdmaOut.Init.DestBlockAddressOffset  = 0;

	/*Using JPEG Output FIFO Threshold as a trigger for the MDMA*/
	hmdmaOut.Init.Request              = MDMA_REQUEST_JPEG_OUTFIFO_TH; /* Set the MDMA HW trigger to JPEG Output FIFO Threshold flag*/ 
	hmdmaOut.Init.TransferTriggerMode  = MDMA_BUFFER_TRANSFER;    
	hmdmaOut.Init.BufferTransferLength = 32; /*Set the MDMA buffer size to the JPEG FIFO threshold size i.e 32 bytes (8 words)*/

	/* DeInitialize the DMA Stream */
	HAL_MDMA_DeInit(&hmdmaOut);  
	/* Initialize the DMA stream */
	HAL_MDMA_Init(&hmdmaOut);

	/* Associate the DMA handle */
	__HAL_LINKDMA(hjpeg, hdmaout, hmdmaOut);

	HAL_NVIC_SetPriority(MDMA_IRQn, 0x08, 0);
	HAL_NVIC_EnableIRQ(MDMA_IRQn); 
}

/**
	* @brief  JPEG�ײ㷴��ʼ����,���ڹر�JPEG����
  * @param hjpeg: JPEG ���ָ��
  * @retval None
  */
void HAL_JPEG_MspDeInit(JPEG_HandleTypeDef *hjpeg)
{
	  HAL_NVIC_DisableIRQ(JPEG_IRQn);
	
    __HAL_RCC_JPEG_CLK_DISABLE();	
		__HAL_RCC_JPGDECEN_CLK_DISABLE();
	
    HAL_MDMA_DeInit(hjpeg->hdmain);
    HAL_MDMA_DeInit(hjpeg->hdmaout);
}

/**
	* @brief  JPEG�жϺ���
  * @param None
  * @retval None
  */
void JPEG_IRQHandler(void)
{
	HAL_JPEG_IRQHandler(&JPEG_Handle);
}

/**
	* @brief  MDMA�жϺ���
  * @param None
  * @retval None
  */
void MDMA_IRQHandler()
{
	/* �ж��жϱ�־λ,��������Ӧ�Ļص����� */
	HAL_MDMA_IRQHandler(JPEG_Handle.hdmain);
	HAL_MDMA_IRQHandler(JPEG_Handle.hdmaout);  
}

/**
	* @brief  ʹ��DMA2D�������ͼ������(YCBCR)ת����LCD��Ļ������ʾ��(RGB565)��ʽ
  * @param pSrc: Դ���ݻ�������ַ
  * @param pDst: Ŀ�����ݻ�������ַ(����Ϊ��Ļ������)
  * @param xsize: �����ͼ����
  * @param ysize: �����ͼ��߶�
  * @param ChromaSampling: ͼƬɫ�ʲ�����
  * @retval None
  */
void DMA2D_CopyBuffer(uint32_t *pSrc, uint32_t *pDst, uint16_t xsize, uint16_t ysize, uint32_t ChromaSampling)
{   
  uint32_t cssMode = DMA2D_CSS_420, inputLineOffset = 0;  
	
	/* ����ɫ�ʲ�����������ƫ�� */
  if(ChromaSampling == JPEG_420_SUBSAMPLING)
  {
    cssMode = DMA2D_CSS_420;
    
    inputLineOffset = xsize % 16;
    if(inputLineOffset != 0)
    {
      inputLineOffset = 16 - inputLineOffset;
    }    
  }
  else if(ChromaSampling == JPEG_444_SUBSAMPLING)
  {
    cssMode = DMA2D_NO_CSS;
    
    inputLineOffset = xsize % 8;
    if(inputLineOffset != 0)
    {
      inputLineOffset = 8 - inputLineOffset;
    }    
  }
  else if(ChromaSampling == JPEG_422_SUBSAMPLING)
  {
    cssMode = DMA2D_CSS_422;
    
    inputLineOffset = xsize % 16;
    if(inputLineOffset != 0)
    {
      inputLineOffset = 16 - inputLineOffset;
    }      
  }  
  
	/* ����DMA2Dģʽ�����ɫ��ģʽ���������ƫ�Ƶ� */
  DMA2D_Handle.Init.Mode         = DMA2D_M2M_PFC;					/* DMA2D�����ڴ浽�ڴ�İ��˷�ʽ */
  DMA2D_Handle.Init.ColorMode    = DMA2D_OUTPUT_RGB565;		/* �����ʽΪRGB565,����ֱ����ʾ��RGB��Ļ */  
  DMA2D_Handle.Init.OutputOffset = 0; 										/* ���ƫ�� */  
  DMA2D_Handle.Init.AlphaInverted = DMA2D_REGULAR_ALPHA;  /* ����תalphaֵ*/  
  DMA2D_Handle.Init.RedBlueSwap   = DMA2D_RB_REGULAR;     /* �����к������ؽ���,�ù���Ϊ��֧�� BGR �� ABGR ��ɫ��ʽ */  
  
  /* ����DMA2D��������жϻص�����,�����ûص� */
  DMA2D_Handle.XferCpltCallback  = NULL;
  
  /* ǰ�������� */
  DMA2D_Handle.LayerCfg[1].AlphaMode = DMA2D_REPLACE_ALPHA;			/* ԭʼǰ����ͼ��� alpha ͨ��ֵ�滻Ϊ InputAlpha - �Ĵ���ALPHA[7:0] */  
  DMA2D_Handle.LayerCfg[1].InputAlpha = 0xFF;										/* �Ĵ���ALPHA[7:0] */ 
  DMA2D_Handle.LayerCfg[1].InputColorMode = DMA2D_INPUT_YCBCR;	/* ����ǰ����ͼ�����ɫ��ʽ */ 
  DMA2D_Handle.LayerCfg[1].ChromaSubSampling = cssMode;					/* ɫ���Ӳ���ģʽ */ 
  DMA2D_Handle.LayerCfg[1].InputOffset = inputLineOffset;				/* ������ƫ�� */ 
  DMA2D_Handle.LayerCfg[1].RedBlueSwap = DMA2D_RB_REGULAR; 			/* �����к������ؽ���,�ù���Ϊ��֧�� BGR �� ABGR ��ɫ��ʽ */
  DMA2D_Handle.LayerCfg[1].AlphaInverted = DMA2D_REGULAR_ALPHA; /* ����תalphaֵ */  
  DMA2D_Handle.Instance          = DMA2D; 
  
  /* �����������ó�ʼ��DMA2D */
	if (   HAL_DMA2D_Init(&DMA2D_Handle) !=  HAL_OK) 
	{
		printf("ERROR 1\n");
	}
	if ( HAL_DMA2D_ConfigLayer(&DMA2D_Handle, 1) !=  HAL_OK) 
	{
		printf("ERROR 2\n");
	}
 
  /* ����DMA2D,ʹ�ÿ⺯���ᳬʱ����,ת��Ҳ����,�����������õĲ���,ֱ�Ӳ����Ĵ�����ok */	
	DMA2D->NLR     = (uint32_t)(xsize << 16) | (uint16_t)ysize;	/* �������ͼ���С */  
	DMA2D->OMAR    = (uint32_t)pDst;	/* ����Դ���ݵĻ�������ַ */  
	DMA2D->FGMAR   = (uint32_t)pSrc;  /* ���ô�����ݵĻ�������ַ */  
	DMA2D->CR   |= DMA2D_CR_START; 		/* ��ʼת�� */  
	while (DMA2D->CR & DMA2D_CR_START) {} 	/* �ȴ�ת����� */  

  /* �⺯��,���� */	
//  if ( HAL_DMA2D_Start(&DMA2D_Handle, (uint32_t)pSrc, (uint32_t)pDst, xsize, ysize) !=  HAL_OK) 
//	{
//		printf("ERROR 3\n");
//	}
//  if (HAL_DMA2D_PollForTransfer(&DMA2D_Handle, HAL_MAX_DELAY) != HAL_OK)  /* wait for the previous DMA2D transfer to ends */
//	{
//		printf("ERROR 4\n");
//	}
}

void _Write_AllPixels_RGB(HDC hdc,const u8 *p, int x, int y, int xSize)
{
	BITMAP bm;
	bm.Format	 =BM_RGB565;
	bm.Width	 =JPEG_Info.ImageWidth;
	bm.Height	 =JPEG_Info.ImageHeight;;
	bm.WidthBytes =xSize*2;//JPEG_Info.ImageWidth * 2;
	bm.Bits	    =(u8*)p;
	bm.LUT		=NULL;
	DrawBitmap(hdc,x,y,&bm,NULL);
}

static	BOOL _Draw(HDC hdc, int x, int y,JPG_DEC *dec)
{
	HAL_JPEG_DeInit(&JPEG_Handle);

	JPEG_Handle.Instance = JPEG;

	HAL_JPEG_Init(&JPEG_Handle);
	 
	Decode_Buffer = (uint32_t *)GUI_VMEM_Alloc(3*512*1024);
	OUT_Buffer = GUI_VMEM_Alloc(800*480*2); 

	JPEG_picture((uint32_t *)Decode_Buffer);
	/* �ȴ�������� */
	while(Jpeg_HWDecodingEnd == 0)
	{
	}
	HAL_JPEG_GetInfo(&JPEG_Handle, &JPEG_Info);

	DMA2D_CopyBuffer((uint32_t *)Decode_Buffer, (uint32_t *)OUT_Buffer, JPEG_Info.ImageWidth, JPEG_Info.ImageHeight ,JPEG_Info.ChromaSubsampling);


	_Write_AllPixels_RGB(hdc,(uint8_t*)OUT_Buffer,0,0,JPEG_Info.ImageWidth);
	
	Jpeg_HWDecodingEnd = 0;//�������,�ָ�������־Ϊ��ʼ״̬
	
	GUI_VMEM_Free(Decode_Buffer);
	GUI_VMEM_Free(OUT_Buffer);
	return 1;
}

BOOL	JPG_Draw(HDC hdc,int x,int y,JPG_DEC *jdec)
{
	if(jdec == NULL)
	{
		return FALSE;
	}
	_Draw(hdc,x,y,jdec);
	return	TRUE;
}

JPG_DEC*	JPG_Open(const void *dat,int cbSize)
{
	JPG_DEC	*dec;
	
	dec	=GUI_VMEM_Alloc(sizeof(JPG_DEC));
  Decode_dec=GUI_VMEM_Alloc(sizeof(JPG_DEC));
	if(dec!=NULL)
	{
		dec->pSrcData =dat;
		dec->DataSize =cbSize;
	}
  if(Decode_dec!=NULL)
	{
		Decode_dec->pSrcData =dat;
		Decode_dec->DataSize =cbSize;
	}
	return	dec;
}

BOOL JPG_GetImageSize(U16 *width,U16 *height,JPG_DEC* jdec)
{
	if(jdec==NULL)
	{
	  return FALSE;
	}	
	
  JPEG_Handle.Instance = JPEG;
	
  HAL_JPEG_Init(&JPEG_Handle);
       
	Decode_Buffer = (uint32_t *)GUI_VMEM_Alloc(3*512*1024);

	JPEG_picture((uint32_t *)Decode_Buffer);
	/* ������� */
	while(Jpeg_HWDecodingEnd == 0)
	{
	}
	Jpeg_HWDecodingEnd = 0;//�������,�ָ�������־Ϊ��ʼ״̬
		
	GUI_VMEM_Free(Decode_Buffer);
	
	HAL_JPEG_GetInfo(&JPEG_Handle, &JPEG_Info);
	/* �ָ�ָ��λ�úʹ�С */
	Decode_dec->pSrcData =jdec->pSrcData;
	Decode_dec->DataSize =jdec->DataSize;
	if(width!=NULL && height!=NULL)
	{
		*width =JPEG_Info.ImageWidth;
	  *height =JPEG_Info.ImageHeight;
   	return TRUE;	
	}
	else
	{
		return FALSE;	
	}
}

/**
  * @brief  ���ļ�ϵͳ������ͼ��������,��������
  * @param 	Buffer_Dest       : ����������
  * @param  Byte_Want_Read    : ���ȡ�����ֽ�
  * @param  Byte_Real_Read    : ʵ�ʶ�ȡ�����ֽ�(��Ϊ���ܵ����ļ�β��)
  * @retval True of False
  */
uint32_t Data_Offset = 0;

FRESULT Read_Buf_From_File(void* Buffer_Dest, uint32_t Byte_Want_Read , uint32_t* Byte_Real_Read)
{
	uint8_t * Temp_pointer = (uint8_t *)Decode_dec->pSrcData;
  if( (Buffer_Dest != NULL) && (Decode_dec->pSrcData != NULL) && (Decode_dec->DataSize >0))
  {
    if(Decode_dec->DataSize >= Byte_Want_Read)
    {
      /* ������ݵĴ�С>��Ҫ��������,ֱ�ӿ���,��������Դ��ַָ��ƫ�ƶ�ȡ���ֽ���,���ݴ�С��ȥ��ȡ�ֽڵ����� */
			taskENTER_CRITICAL();
			memcpy((uint8_t *)Buffer_Dest,Temp_pointer + Data_Offset,Byte_Want_Read);
			taskEXIT_CRITICAL();
			
      *Byte_Real_Read = Byte_Want_Read;
			Data_Offset += Byte_Want_Read; //����ƫ����
      Decode_dec->DataSize -= Byte_Want_Read;
      return FR_OK;
    }
    else
    {
      /* ������ݵĴ�С<��Ҫ��������,����ʣ������ݴ�СDecode_dec->DataSize,��������Դ��ַָ��ƫ�ƶ�ȡ���ֽ���,�������� */
			taskENTER_CRITICAL();
      memcpy((uint8_t *)Buffer_Dest,Temp_pointer + Data_Offset,Decode_dec->DataSize);
			taskEXIT_CRITICAL();
			
			*Byte_Real_Read = Decode_dec->DataSize;
      Decode_dec->DataSize = 0;
			Data_Offset = 0;
      return FR_OK;
    }
  }
  else
  {
    return FR_DISK_ERR;
  }
}

void	JPG_Close(JPG_DEC *jdec)
{
	if(jdec!=NULL)
	{	
		GUI_VMEM_Free(jdec);
	}
	if(Decode_dec!=NULL)
	{	
		GUI_VMEM_Free(Decode_dec);
	}

}

void JPEG_Out(HDC hdc,int x,int y,u8 *mjpegbuffer,s32 size)
{
	Decode_dec->pSrcData =mjpegbuffer;
	Decode_dec->DataSize =size;
	JPG_Draw(hdc,x,y,Decode_dec);
}

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
