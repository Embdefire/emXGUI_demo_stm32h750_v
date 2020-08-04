/*
******************************************************************************
* @file    recorder.c
* @author  fire
* @version V1.0
* @date    2015-xx-xx
* @brief   WM8978�������ܲ���+mp3����
******************************************************************************
* @attention
*
* ʵ��ƽ̨:����  STM32 F767 ������  
* ��̳    :http://www.chuxue123.com
* �Ա�    :http://firestm32.taobao.com
*
******************************************************************************
*/
#include <stdio.h>
#include <string.h>
#include "./usart/bsp_usart.h"
#include "./wm8978/bsp_wm8978.h"
#include "ff.h" 
#include "./mp3Player/mp3Player.h"
#include "mp3dec.h"
#include "./sai/bsp_sai.h" 
#include "x_libc.h"
#include "./mp3_player/GUI_MUSICPLAYER_DIALOG.h"
/* �Ƽ�ʹ�����¸�ʽmp3�ļ���
 * �����ʣ�44100Hz
 * ��  ����2
 * �����ʣ�320kbps
 */

/* ������������Ƶ����ʱ�������������Ҫ������СΪ2304*16/8�ֽ�(16ΪPCM����Ϊ16λ)��
 * �������Ƕ���MP3BUFFER_SIZEΪ2304��ʵ�����������ΪMP3BUFFER_SIZE*2���ֽ�
 */
#define MP3BUFFER_SIZE  2304		
#define INPUTBUF_SIZE   3000	

static HMP3Decoder		Mp3Decoder;			/* mp3������ָ��	*/
static MP3FrameInfo		Mp3FrameInfo;		/* mP3֡��Ϣ  */

MP3_TYPE mp3player;         /* mp3�����豸 */
volatile uint8_t Isread=0;           /* DMA������ɱ�־ */
static uint8_t bufflag=0;          /* ���ݻ�����ѡ���־ */

uint32_t led_delay=0;

uint8_t inputbuf[INPUTBUF_SIZE]={0};        /* �������뻺������1940�ֽ�Ϊ���MP3֡��С  */
short outbuffer[2][MP3BUFFER_SIZE];  /* ���������������Ҳ��I2S�������ݣ�ʵ��ռ���ֽ�����RECBUFFER_SIZE*2 */



/*wav������*/
uint16_t buffer0[RECBUFFER_SIZE];  /* ���ݻ�����1 ��ʵ��ռ���ֽ�����RECBUFFER_SIZE*2 */
uint16_t buffer1[RECBUFFER_SIZE];  /* ���ݻ�����2 ��ʵ��ռ���ֽ�����RECBUFFER_SIZE*2 */
static WavHead rec_wav;            /* WAV�豸  */

FIL file;											/* file objects */
FRESULT result; 
UINT bw;            					/* File R/W count */






/**
  * @brief  ��ȡMP3ID3V2�ļ�ͷ�Ĵ�С
  * @param  ����MP3�ļ���ͷ�����ݣ�����10���ֽ�
  * @retval ID3V2�Ĵ�С
  */
uint32_t mp3_GetID3V2_Size(unsigned char *buf)
{
 uint32_t ID3V2_size;
	
 if(buf[0] == 'I' && buf[1] == 'D' && buf[2] =='3')//����ID3V2
 {
 	 ID3V2_size = (buf[6]<<21) | (buf[7]<<14) | (buf[8]<<7) | buf[9];
   ID3V2_size = (buf[6]&0x7F)*0x200000+ (buf[7]&0x7F)*0x400 + (buf[8]&0x7F)*0x80 +(buf[9]&0x7F);
 }
 else//������ID3V2
	 ID3V2_size = 0;

 return ID3V2_size;

}
/**
  * @brief   MP3��ʽ��Ƶ����������
  * @param  ��
  * @retval ��
  */
void mp3PlayerDemo(const char *mp3file)
{
	uint8_t *read_ptr=inputbuf;
	uint32_t frames=0;
  DWORD pos;//��¼���ֱ���
  static uint8_t timecount = 0;
	int err=0, i=0, outputSamps=0;
  static uint8_t lyriccount=0;//���index��¼	
	int	read_offset = 0;				/* ��ƫ��ָ�� */
	int	bytes_left = 0;					/* ʣ���ֽ��� */	
  uint32_t time_sum = 0; //���㵱ǰ�Ѳ��ŵ���ʱ��λ��
  uint16_t frame_size;//MP3֡�Ĵ�С
	uint32_t ID3V2_size;//MP3��ID3V2�Ĵ�С
	WCHAR wbuf[128];//�����ı�����
	mp3player.ucFreq = SAI_AUDIOFREQ_DEFAULT;
	mp3player.ucStatus = STA_IDLE;
	mp3player.ucVolume = MusicDialog.power;
	
	result=f_open(&file,mp3file,FA_READ);
	if(result!=FR_OK)
	{
		printf("Open mp3file :%s fail!!!->%d\r\n",mp3file,result);
		result = f_close (&file);
		return;	/* ֹͣ���� */
	}
	printf("��ǰ�����ļ� -> %s\n",mp3file);
	
	//��ʼ��MP3������
	Mp3Decoder = MP3InitDecoder();	
	if(Mp3Decoder==0)
	{
		printf("��ʼ��helix������豸\n");
		return;	/* ֹͣ���� */
	}
	printf("��ʼ����...\n");
	
	Delay_ms(10);	/* �ӳ�һ��ʱ�䣬�ȴ�I2S�жϽ��� */
	wm8978_Reset();		/* ��λWM8978����λ״̬ */
	/* ����WM8978оƬ������ΪDAC�����Ϊ���� */
  if(music_icon[2].state == FALSE)
  {
    wm8978_CfgAudioPath(DAC_ON,  EAR_LEFT_ON|EAR_RIGHT_ON);
  }
  //������icon������ʱ������Ϊ����ģʽ
  else
  {                
    wm8978_CfgAudioPath(DAC_ON,  SPK_ON|EAR_LEFT_ON|EAR_RIGHT_ON);
  } 

	/* ����������������ͬ���� */
	wm8978_SetOUT1Volume(mp3player.ucVolume);

	/* ����WM8978��Ƶ�ӿ�Ϊ�����ֱ�׼I2S�ӿڣ�16bit */
	wm8978_CfgAudioIF(SAI_I2S_STANDARD, 16);
	
	/*  ��ʼ��������I2S  */
  SAI_Play_Stop();
	SAI_GPIO_Config();
  SAI_DMA_TX_Callback = MusicPlayer_SAI_DMA_TX_Callback;
//	SAIxA_Tx_Config(SAI_I2S_STANDARD,SAI_PROTOCOL_DATASIZE_16BIT,mp3player.ucFreq);	
//	SAIA_TX_DMA_Init();	
	
	bufflag=0;
	Isread=0;

	MusicDialog.chgsch=0;
  lyriccount=0;
  timecount=0;

	mp3player.ucStatus = STA_PLAYING;		/* ����״̬ */
  result=f_read(&file,inputbuf,INPUTBUF_SIZE,&bw);
	if(result!=FR_OK)
	{
		printf("��ȡ%sʧ�� -> %d\r\n",mp3file,result);
		MP3FreeDecoder(Mp3Decoder);
		return;
	}

  ID3V2_size = mp3_GetID3V2_Size(inputbuf);
	read_ptr=inputbuf;
	bytes_left=bw;
  MusicDialog.angle = 0;

	/* ����������ѭ���� */
	while(mp3player.ucStatus == STA_PLAYING)
	{
    if(!(music_icon[6].state == FALSE))
    {
      GUI_msleep(10);
      continue;
    }
		read_offset = MP3FindSyncWord(read_ptr, bytes_left);	//Ѱ��֡ͬ�������ص�һ��ͬ���ֵ�λ��
		if(read_offset < 0)										//û���ҵ�ͬ����
		{
			result=f_read(&file,inputbuf,INPUTBUF_SIZE,&bw);
			if(result!=FR_OK)
			{
				printf("��ȡ%sʧ�� -> %d\r\n",mp3file,result);
				break;
			}
			read_ptr=inputbuf;
			bytes_left=bw;
			continue;		//����ѭ��2���ص�ѭ��1	
		}
		read_ptr += read_offset;				//ƫ����ͬ���ֵ�λ��
		bytes_left -= read_offset;				//ͬ����֮������ݴ�С	
		if(bytes_left < 1024)							//��������
		{
			/* ע������ط���Ϊ���õ���DMA��ȡ������һ��Ҫ4�ֽڶ���  */
			i=(uint32_t)(bytes_left)&3;			//�ж϶�����ֽ�
			if(i) i=4-i;						//��Ҫ������ֽ�
			memcpy(inputbuf+i, read_ptr, bytes_left);	//�Ӷ���λ�ÿ�ʼ����
			read_ptr = inputbuf+i;										//ָ�����ݶ���λ��
			result = f_read(&file, inputbuf+bytes_left+i, INPUTBUF_SIZE-bytes_left-i, &bw);//��������
			if(result!=FR_OK)
			{
				printf("��ȡ%sʧ�� -> %d\r\n",mp3file,result);
				break;
			}
			bytes_left += bw;		//��Ч��������С
		}
		err = MP3Decode(Mp3Decoder, &read_ptr, &bytes_left, outbuffer[bufflag], 0);	//bufflag��ʼ���� ������mp3����ṹ�塢������ָ�롢��������С�������ָ�롢���ݸ�ʽ
    time_sum +=26;//ÿ֡26ms      
		frames++;	
		if (err != ERR_MP3_NONE)	//������
		{
			switch (err)
			{
				case ERR_MP3_INDATA_UNDERFLOW:
					printf("ERR_MP3_INDATA_UNDERFLOW\r\n");
					result = f_read(&file, inputbuf, INPUTBUF_SIZE, &bw);
					read_ptr = inputbuf;
					bytes_left = bw;
					break;		
				case ERR_MP3_MAINDATA_UNDERFLOW:
					/* do nothing - next call to decode will provide more mainData */
					printf("ERR_MP3_MAINDATA_UNDERFLOW\r\n");
					break;		
				default:
					printf("UNKNOWN ERROR:%d\r\n", err);		
					// ������֡
					if (bytes_left > 0)
					{
						bytes_left --;
						read_ptr ++;
					}	
					break;
			}
			Isread=1;
		}
   
		else		//�����޴���׼�������������PCM
		{
			MP3GetLastFrameInfo(Mp3Decoder, &Mp3FrameInfo);		//��ȡ������Ϣ				
			/* �����DAC */
			outputSamps = Mp3FrameInfo.outputSamps;							//PCM���ݸ���
			if (outputSamps > 0)
			{
				if (Mp3FrameInfo.nChans == 1)	//������
				{
					//������������Ҫ����һ�ݵ���һ������
					for (i = outputSamps - 1; i >= 0; i--)
					{
						outbuffer[bufflag][i * 2] = outbuffer[bufflag][i];
						outbuffer[bufflag][i * 2 + 1] = outbuffer[bufflag][i];
					}
					outputSamps *= 2;
				}//if (Mp3FrameInfo.nChans == 1)	//������
			}//if (outputSamps > 0)
			
			/* ���ݽ�����Ϣ���ò����� */
			if (Mp3FrameInfo.samprate != mp3player.ucFreq)	//������ 
			{
        printf("Mp3FrameInfo%d\n",Mp3FrameInfo.samprate);
				mp3player.ucFreq = Mp3FrameInfo.samprate;
        frame_size = (((Mp3FrameInfo.version == MPEG1)? 144:72)*Mp3FrameInfo.bitrate)/Mp3FrameInfo.samprate+Mp3FrameInfo.paddingBit;				
        MusicDialog.alltime=(((file.fsize-ID3V2_size-128)/frame_size)*26+1000)/1000;
        
       
				if(mp3player.ucFreq >= SAI_AUDIOFREQ_DEFAULT)	//I2S_AudioFreq_Default = 2��������֡��ÿ�ζ�Ҫ������
				{

          
					SAIxA_Tx_Config(SAI_I2S_STANDARD,SAI_PROTOCOL_DATASIZE_16BIT,mp3player.ucFreq);						//���ݲ������޸�iis����
          SAIA_TX_DMA_Init((uint16_t *)outbuffer[0],(uint16_t *)outbuffer[1],outputSamps);
				}

				SAI_Play_Start();
			}
		}//else ��������
		
		if(file.fptr==file.fsize) 		//mp3�ļ���ȡ��ɣ��˳�
		{
      mp3player.ucStatus=STA_SWITCH;
      MusicDialog.playindex++;
      
      if(MusicDialog.playindex >= MusicDialog.music_file_num)
        MusicDialog.playindex = 0;      
			printf("END\r\n");
			break;
		}	

    int update = 0;//��¼ֻ����һ�εĲ���
//    static int times= 0 ;
		while(Isread==0)
		{
      if(MusicDialog.chgsch == 0)
      {
        //times = 0;
        if(timecount>=20)
        {
          //��ǰֵ
          MusicDialog.curtime = time_sum/1000;
          
     
          
          if(!MusicDialog.mList_State && update == 0)//�����б���棬�����и���
          {
            update = 1;
            x_wsprintf(wbuf, L"%02d:%02d",MusicDialog.alltime/60,MusicDialog.alltime%60);
            SetWindowText(GetDlgItem(MusicDialog.MUSIC_Hwnd, eID_ALL_TIME), wbuf);
            x_mbstowcs_cp936(wbuf, music_lcdlist[MusicDialog.playindex], 100);
            SetWindowText(GetDlgItem(MusicDialog.MUSIC_Hwnd, eID_MUSIC_ITEM), wbuf);                  
          }          
                      
          x_wsprintf(wbuf, L"%02d:%02d",MusicDialog.curtime/60,MusicDialog.curtime%60);
          if(!MusicDialog.mList_State && !(SendMessage(MusicDialog.TIME_Hwnd, SBM_GETSTATE,0,0)&SST_THUMBTRACK))//�����б���棬�����и���
          {
            SendMessage(MusicDialog.TIME_Hwnd, SBM_SETVALUE, TRUE, MusicDialog.curtime*255/MusicDialog.alltime);
            //InvalidateRect(MusicDialog.TIME_Hwnd, NULL, FALSE); 
            SetWindowText(GetDlgItem(MusicDialog.MUSIC_Hwnd, eID_CUR_TIME), wbuf);    
          }            
          lrc.curtime = MusicDialog.curtime;  
          if(lrc.flag == 1)
          {
           //+100����ǰ��ʾ����ʾ��Ҫ����һ��ʱ��
            if((lrc.oldtime <= lrc.curtime*100+100)&&(lrc.indexsize>7))
            {
              //��ʾ��ǰ�еĸ��
              x_mbstowcs_cp936(wbuf, (const char *)&ReadBuffer1[lrc.addr_tbl[lyriccount]-1], LYRIC_MAX_SIZE);
              SetWindowText(GetDlgItem(MusicDialog.LRC_Hwnd,eID_TEXTBOX_LRC3),wbuf);
              //��ʾ��i-1�еĸ�ʣ�ǰһ�У�
              if(lyriccount>0)
              {
                x_mbstowcs_cp936(wbuf, (const char *)&ReadBuffer1[lrc.addr_tbl[lyriccount-1]-1], LYRIC_MAX_SIZE);
                SetWindowText(GetDlgItem(MusicDialog.LRC_Hwnd,eID_TEXTBOX_LRC2),wbuf);
              }
              else
                SetWindowText(GetDlgItem(MusicDialog.LRC_Hwnd,eID_TEXTBOX_LRC2),L" ");
              //��ʾ��i-2�еĸ�ʣ�ǰ���У�
              if(lyriccount>0)
              {
                x_mbstowcs_cp936(wbuf, (const char *)&ReadBuffer1[lrc.addr_tbl[lyriccount-2]-1], LYRIC_MAX_SIZE);
                SetWindowText(GetDlgItem(MusicDialog.LRC_Hwnd,eID_TEXTBOX_LRC1),wbuf);
              }
              else
                SetWindowText(GetDlgItem(MusicDialog.LRC_Hwnd,eID_TEXTBOX_LRC1),L" ");
              //��ʾ��i+1�еĸ�ʣ���һ�У�   
              if(lyriccount < lrc.indexsize-1)
              {
                x_mbstowcs_cp936(wbuf, (const char *)&ReadBuffer1[lrc.addr_tbl[lyriccount+1]-1], LYRIC_MAX_SIZE);
                SetWindowText(GetDlgItem(MusicDialog.LRC_Hwnd,eID_TEXTBOX_LRC4),wbuf);                    
              }
              else
                SetWindowText(GetDlgItem(MusicDialog.LRC_Hwnd,eID_TEXTBOX_LRC4),L" ");
              //��ʾ��i+2�еĸ�ʣ�����У�   
              if(lyriccount < lrc.indexsize-2)
              {
                x_mbstowcs_cp936(wbuf, (const char *)&ReadBuffer1[lrc.addr_tbl[lyriccount+2]-1], LYRIC_MAX_SIZE);
                SetWindowText(GetDlgItem(MusicDialog.LRC_Hwnd,eID_TEXTBOX_LRC5),wbuf);                    
              }
              else
                SetWindowText(GetDlgItem(MusicDialog.LRC_Hwnd,eID_TEXTBOX_LRC5),L" ");

              do{
                lyriccount++;					
                if(lyriccount>=lrc.indexsize)
                {
                  lrc.oldtime=0xffffff;
                  break;
                }
                lrc.oldtime=lrc.time_tbl[lyriccount];
              }while(lrc.oldtime<=(lrc.curtime*100));
            }
          }
          else
          {
             
             SetWindowText(GetDlgItem(MusicDialog.LRC_Hwnd,eID_TEXTBOX_LRC3),L"����SDCard������Ӧ�ĸ���ļ�(*.lrc)");
             SetWindowText(GetDlgItem(MusicDialog.LRC_Hwnd,eID_TEXTBOX_LRC2),L" ");
             SetWindowText(GetDlgItem(MusicDialog.LRC_Hwnd,eID_TEXTBOX_LRC1),L" ");
             SetWindowText(GetDlgItem(MusicDialog.LRC_Hwnd,eID_TEXTBOX_LRC4),L" ");
             SetWindowText(GetDlgItem(MusicDialog.LRC_Hwnd,eID_TEXTBOX_LRC5),L" ");
          }           
          
          timecount=0;
        }         
      }
      else
      {
        uint8_t temp=0;	
           
        //���ݽ�������������λ��				
        temp=SendMessage(MusicDialog.TIME_Hwnd, SBM_GETVALUE, NULL, NULL);        
           
        //�����������ʾ��ʱ��
        time_sum = (float)MusicDialog.alltime/255*temp*1000;  	
        //����ʱ������ļ�λ�ò���ת����λ��
        pos = ID3V2_size + (time_sum/26)*frame_size;

//        if(pos%4!=0)          //����
//        {
//         uint8_t num_left = 0;
//         num_left = pos % 4;
//         pos += (4-num_left);
//     
//        } 
        if(MusicDialog.Update_Content == 1)
        {
          
          MusicDialog.Update_Content = 0;
          result = f_lseek(&file,pos);
        }
        lrc.oldtime=0;
        lyriccount=0;
         
        MusicDialog.chgsch=0;     
      }  
           
		}
		Isread=0; 
    timecount++;
	}
	SAI_Play_Stop();
  lyriccount=0;
	mp3player.ucStatus=STA_IDLE;
	MP3FreeDecoder(Mp3Decoder);
	f_close(&file);	
}



void wavplayer(const char *wavfile)
{
	static uint8_t timecount;//��¼ʱ��
  WCHAR wbuf[128];
  static COLORREF color;
	mp3player.ucStatus=STA_IDLE;    /* ��ʼ����Ϊ����״̬  */
   
  DWORD pos;//��¼���ֱ���
  static uint8_t lyriccount=0;//���index��¼
  
	/*  ��ʼ��������I2S  */
	SAI_Play_Stop();
	SAI_GPIO_Config();
	SAI_DMA_TX_Callback = MusicPlayer_SAI_DMA_TX_Callback;  
	bufflag=0;
	Isread=0;
  if(mp3player.ucStatus == STA_IDLE)
  {						
    printf("��ǰ�����ļ� -> %s\n",wavfile);

    result=f_open(&file,wavfile,FA_READ);
    if(result!=FR_OK)
    {
       printf("����Ƶ�ļ�ʧ��!!!->%d\r\n",result);
       result = f_close (&file);
       return;
    }
    //��ȡWAV�ļ�ͷ
    result = f_read(&file,&rec_wav,sizeof(rec_wav),&bw);
    
    mp3player.ucFreq =  rec_wav.dwSamplesPerSec;
    mp3player.ucbps =  mp3player.ucFreq*32;   
    MusicDialog.alltime=file.fsize*8/mp3player.ucbps;    
    
    //�ȶ�ȡ��Ƶ���ݵ�������
    result = f_read(&file,(uint16_t *)buffer0,RECBUFFER_SIZE*2,&bw);
    result = f_read(&file,(uint16_t *)buffer1,RECBUFFER_SIZE*2,&bw);  

    Delay_ms(10);	/* �ӳ�һ��ʱ�䣬�ȴ�I2S�жϽ��� */
    wm8978_Reset();		/* ��λWM8978����λ״̬ */	

    mp3player.ucStatus = STA_PLAYING;		/* ����״̬ */

    /* ����WM8978оƬ������ΪDAC�����Ϊ���� */
    wm8978_CfgAudioPath(DAC_ON, EAR_LEFT_ON | EAR_RIGHT_ON);
    /* ����������������ͬ���� */
    wm8978_SetOUT1Volume(MusicDialog.power);
    /* ����WM8978��Ƶ�ӿ�Ϊ�����ֱ�׼I2S�ӿڣ�16bit */
    wm8978_CfgAudioIF(SAI_I2S_STANDARD, 16);  
    
    SAIxA_Tx_Config(SAI_I2S_STANDARD,SAI_PROTOCOL_DATASIZE_16BIT,mp3player.ucFreq);						//���ݲ������޸�iis����
    SAIA_TX_DMA_Init(buffer0,buffer1,RECBUFFER_SIZE); 
    SAI_Play_Start();        
  }
  while(mp3player.ucStatus == STA_PLAYING)
  {
    int update = 0;//��¼ֻ����һ�εĲ���
    if(Isread==1)
    {
      Isread=0;
      if(MusicDialog.chgsch == 0)
      {
        if(timecount>=10)      
        {     
          MusicDialog.curtime=file.fptr*8/mp3player.ucbps;
          
          
           if(!MusicDialog.mList_State && update == 0)//�����б���棬�����и���
          {
            update = 1;
            x_wsprintf(wbuf, L"%02d:%02d",MusicDialog.alltime/60,MusicDialog.alltime%60);
            SetWindowText(GetDlgItem(MusicDialog.MUSIC_Hwnd, eID_ALL_TIME), wbuf);
            x_mbstowcs_cp936(wbuf, music_lcdlist[MusicDialog.playindex], 100);
            SetWindowText(GetDlgItem(MusicDialog.MUSIC_Hwnd, eID_MUSIC_ITEM), wbuf);                  
          }          
                      
          x_wsprintf(wbuf, L"%02d:%02d",MusicDialog.curtime/60,MusicDialog.curtime%60);
          if(!MusicDialog.mList_State && !(SendMessage(MusicDialog.TIME_Hwnd, SBM_GETSTATE,0,0)&SST_THUMBTRACK))//�����б���棬�����и���
          {
            SendMessage(MusicDialog.TIME_Hwnd, SBM_SETVALUE, TRUE, MusicDialog.curtime*255/MusicDialog.alltime);
            //InvalidateRect(MusicDialog.TIME_Hwnd, NULL, FALSE); 
            SetWindowText(GetDlgItem(MusicDialog.MUSIC_Hwnd, eID_CUR_TIME), wbuf);    
          }                     
          timecount=0;
        }
      }
      else
      {
      
      }
      timecount++;
      if(bufflag==0)
        result = f_read(&file,buffer0,RECBUFFER_SIZE*2,&bw);	
      else
        result = f_read(&file,buffer1,RECBUFFER_SIZE*2,&bw);
      /* ������ɻ��ȡ����ֹͣ���� */
      if((result!=FR_OK)||(file.fptr==file.fsize))
      {
        //�����и�״̬
        mp3player.ucStatus=STA_SWITCH;
        //������Ŀ����1
        MusicDialog.playindex++;
        //printf("%d, %d\n", play_index, music_file_num);
        //����Ϊ�б�ѭ������
        if(MusicDialog.playindex >= MusicDialog.music_file_num) MusicDialog.playindex = 0;
        if(MusicDialog.playindex < 0) MusicDialog.playindex = MusicDialog.music_file_num - 1;
        printf("��������߶�ȡ�����˳�...\r\n");
        SAI_Play_Stop();
        file.fptr=0;
        f_close(&file);
        wm8978_Reset();	/* ��λWM8978����λ״̬ */							
      }	      
      
    }
  }
  mp3player.ucStatus = STA_SWITCH;		/* ����״̬ */
  file.fptr=0;
  f_close(&file);
  lrc.oldtime=0;
  lyriccount=0;      
  SAI_Play_Stop();		/* ֹͣI2S¼���ͷ��� */
  wm8978_Reset();	/* ��λWM8978����λ״̬ */  
}

/* DMA��������жϻص����� */
/* �����������Ѿ�������ɣ���Ҫ�л��������������»��������ݲ��� 
   ͬʱ��ȡWAV�ļ�������䵽�Ѳ�������  */
void MusicPlayer_SAI_DMA_TX_Callback(void)
{

  if(DMA_Instance->CR&(1<<19)) //��ǰʹ��Memory1����
  {
    bufflag=0;                       //���Խ����ݶ�ȡ��������0
  }
  else                               //��ǰʹ��Memory0����
  {
    bufflag=1;                       //���Խ����ݶ�ȡ��������1
  }
  Isread=1;                          // DMA������ɱ�־
	
}

/***************************** (END OF FILE) *********************************/
