/*
******************************************************************************
* @file    recorder.c
* @author  fire
* @version V1.0
* @date    2015-xx-xx
* @brief   WM8978¼�����ܲ���
******************************************************************************
* @attention
*
* ʵ��ƽ̨:Ұ��  STM32 H743 ������  
* ��̳    :http://www.firebbs.cn
* �Ա�    :https://fire-stm32.taobao.com
*
******************************************************************************
*/
#include "Bsp/usart/bsp_usart.h"
//#include "bsp/key/bsp_key.h" 
#include "Bsp/wm8978/bsp_wm8978.h"
#include "ff.h" 
#include "Backend_Recorder.h"
#include "./mp3_player/Backend_mp3Player.h"
#include "./sai/bsp_sai.h"

/* ��Ƶ��ʽ�л��б�(�����Զ���) */
#define FMT_COUNT	6		/* ��Ƶ��ʽ����Ԫ�ظ��� */

/* ¼���ļ�·��ȫ�ƣ���ʼ��Ϊrec001.wav */
char recfilename[25]={"0:/recorder/rec001.wav"};   
extern REC_TYPE Recorder;          /* ¼���豸 */
WavHead rec_wav;            /* WAV�豸  */
uint8_t Isread=0;           /* DMA������ɱ�־ */
uint8_t bufflag=0;          /* ���ݻ�����ѡ���־ */
uint32_t wavsize=0;         /* wav��Ƶ���ݴ�С */
__align(4) uint16_t record_buffer0[RECBUFFER_SIZE]	__attribute__((at(0x24008000)));//__EXRAM;   /* ���ݻ�����1 ��ʵ��ռ���ֽ�����RECBUFFER_SIZE*2 BUF�������ⲿ���в��ܶ�ȡSD����BUG*/
__align(4) uint16_t record_buffer1[RECBUFFER_SIZE]	__attribute__((at(0x24004000)));//__EXRAM;   /* ���ݻ�����2 ��ʵ��ռ���ֽ�����RECBUFFER_SIZE*2 */

FIL record_file	__EXRAM;			/* file objects */
extern FRESULT result; 
extern UINT bw;            					/* File R/W count */

uint32_t g_FmtList[FMT_COUNT][3] =
{
	{SAI_I2S_STANDARD, SAI_PROTOCOL_DATASIZE_16BIT, SAI_AUDIO_FREQUENCY_44K},
	{SAI_I2S_STANDARD, SAI_PROTOCOL_DATASIZE_16BIT, SAI_AUDIO_FREQUENCY_44K},
	{SAI_I2S_STANDARD, SAI_PROTOCOL_DATASIZE_16BIT, SAI_AUDIO_FREQUENCY_44K},
	{SAI_I2S_STANDARD, SAI_PROTOCOL_DATASIZE_16BIT, SAI_AUDIO_FREQUENCY_44K},
	{SAI_I2S_STANDARD, SAI_PROTOCOL_DATASIZE_16BIT, SAI_AUDIO_FREQUENCY_44K},
	{SAI_I2S_STANDARD, SAI_PROTOCOL_DATASIZE_16BIT, SAI_AUDIO_FREQUENCY_44K},
};

//extern const uint16_t recplaybuf[4];//2��16λ����,����¼��ʱI2S Master����.ѭ������0.
__EXRAM uint16_t recplaybuf[4]={0X0000,0X0000};
/* �������ļ��ڵ��õĺ������� */

void MusicPlayer_SAI_DMA_TX_Callback(void);

/**
  * @brief  ����WM8978��STM32��I2S��ʼ¼����
  * @param  ��
  * @retval ��
  */
void StartRecord(const char *filename)
{
	uint8_t ucRefresh;	/* ͨ�����ڴ�ӡ�����Ϣ��־ */
	DIR dir;
	
	Recorder.ucStatus=STA_IDLE;    /* ��ʼ����Ϊ����״̬  */
	Recorder.ucInput=0;            /* ȱʡMIC����  */
	Recorder.ucFmtIdx=3;           /* ȱʡ������I2S��׼��16bit���ݳ��ȣ�44K������  */
	Recorder.ucVolume=50;          /* ȱʡ��������  */
	if(Recorder.ucInput==0) //MIC 
	{
		Recorder.ucGain=50;          /* ȱʡMIC����  */
		rec_wav.wChannels=2;         /* ȱʡMIC��ͨ�� */
	}
	else                    //LINE
	{
		Recorder.ucGain=6;           /* ȱʡ��·�������� */
		rec_wav.wChannels=2;         /* ȱʡ��·����˫���� */
	}
	
	rec_wav.riff=0x46464952;       /* ��RIFF��; RIFF ��־ */
	rec_wav.size_8=0;              /* �ļ����ȣ�δȷ�� */
	rec_wav.wave=0x45564157;       /* ��WAVE��; WAVE ��־ */ 
	
	rec_wav.fmt=0x20746d66;        /* ��fmt ��; fmt ��־�����һλΪ�� */
	rec_wav.fmtSize=16;            /* sizeof(PCMWAVEFORMAT) */ 
	rec_wav.wFormatTag=1;          /* 1 ��ʾΪPCM ��ʽ���������� */ 
	/* ÿ����������λ������ʾÿ�������и�������������λ���� */
	rec_wav.wBitsPerSample=16;
	/* ����Ƶ�ʣ�ÿ���������� */
	rec_wav.dwSamplesPerSec=g_FmtList[Recorder.ucFmtIdx][2];
	/* ÿ������������ֵΪͨ������ÿ������λ����ÿ����������λ���� 8�� */
	rec_wav.dwAvgBytesPerSec=rec_wav.wChannels*rec_wav.dwSamplesPerSec*rec_wav.wBitsPerSample/8;  
	/* ���ݿ�ĵ����������ֽ���ģ�����ֵΪͨ������ÿ����������λֵ��8�� */
	rec_wav.wBlockAlign=rec_wav.wChannels*rec_wav.wBitsPerSample/8; 
	
	rec_wav.data=0x61746164;       /* ��data��; ���ݱ�Ƿ� */
	rec_wav.datasize=0;            /* �������ݴ�С Ŀǰδȷ��*/
	
	/*  ��ʼ��������I2S  */
  SAI_GPIO_Config();
  //SAI_Play_Stop();
  SAIxA_Tx_Config(g_FmtList[Recorder.ucFmtIdx][0],g_FmtList[Recorder.ucFmtIdx][1],g_FmtList[Recorder.ucFmtIdx][2]);
	SAIxB_Rx_Config(g_FmtList[Recorder.ucFmtIdx][0],g_FmtList[Recorder.ucFmtIdx][1],g_FmtList[Recorder.ucFmtIdx][2]);
  //SAI_DMA_TX_Callback=MusicPlayer_SAI_DMA_TX_Callback;
  ucRefresh = 1;
	bufflag=0;
	Isread=0;

	printf("��ǰ¼���ļ� -> %s\n",filename);
	result=f_open(&record_file,filename,FA_CREATE_ALWAYS|FA_WRITE);
	if(result!=FR_OK)
	{
		printf("Open wavfile fail!!!->%d\r\n",result);
		result = f_close (&record_file);
		Recorder.ucStatus = STA_ERR;
		return;
	}
	
	// д��WAV�ļ�ͷ���������д��д����ļ�ָ���Զ�ƫ�Ƶ�sizeof(rec_wav)λ�ã�
	// ������д����Ƶ���ݲŷ��ϸ�ʽҪ��
	result=f_write(&record_file,(const void *)&rec_wav,sizeof(rec_wav),&bw);
	
	GUI_msleep(10);		/* �ӳ�һ��ʱ�䣬�ȴ�I2S�жϽ��� */
	SAI_Rec_Stop();			/* ֹͣI2S¼���ͷ��� */	
	SAI_Play_Stop();
	wm8978_Reset();		/* ��λWM8978����λ״̬ */
  wm8978_CtrlGPIO1(0);
	Recorder.ucStatus = STA_RECORDING;		/* ¼��״̬ */
		
	/* ���ڷ���������������ͬ���� */
	wm8978_SetOUT1Volume(Recorder.ucVolume);

	if(Recorder.ucInput == 1)   /* ������ */
	{
		/* ����WM8978оƬ������Ϊ�����룬���Ϊ���� */
		wm8978_CfgAudioPath(LINE_ON | ADC_ON, EAR_LEFT_ON | EAR_RIGHT_ON);
		wm8978_SetLineGain(Recorder.ucGain);
	}
	else   /* MIC���� */
	{
		/* ����WM8978оƬ������ΪMic�����Ϊ���� */
		wm8978_CfgAudioPath(MIC_LEFT_ON | MIC_RIGHT_ON | ADC_ON, EAR_LEFT_ON | EAR_RIGHT_ON);	
		wm8978_SetMicGain(Recorder.ucGain);	
	}
		
	/* ����WM8978��Ƶ�ӿ�Ϊ�����ֱ�׼I2S�ӿڣ�16bit */
	wm8978_CfgAudioIF(SAI_I2S_STANDARD, 16);

	 SAIA_TX_DMA_Init((uint32_t)&recplaybuf[0],(uint32_t)&recplaybuf[1],1);
   __HAL_DMA_DISABLE_IT(&h_txdma,DMA_IT_TC);
   SAIB_RX_DMA_Init((uint32_t)record_buffer0,(uint32_t)record_buffer1,RECBUFFER_SIZE);

  SAI_Rec_Start();
  SAI_Play_Start();
}

/* DMA��������жϻص����� */
/* ¼�������Ѿ��������һ������������Ҫ�л���������
   ͬʱ���԰������Ļ���������д�뵽�ļ��� */
void MusicPlayer_SAI_DMA_RX_Callback(void)
{
	if(Recorder.ucStatus == STA_RECORDING)
	{
		if(DMA1_Stream3->CR&(1<<19)) //��ǰʹ��Memory1����
		{
			bufflag=0;                       //���Խ����ݶ�ȡ��������0
		}
		else                               //��ǰʹ��Memory0����
		{
			bufflag=1;                       //���Խ����ݶ�ȡ��������1
		}
		Isread=1;                          // DMA������ɱ�־
	}
}

/***************************** (END OF FILE) *********************************/
