#include <stdio.h>
#include <string.h>
#include "stm32h7xx.h"
//#include "ff.h"
#include "Backend_avifile.h"
#include "Backend_vidoplayer.h"
#include "./sai/bsp_sai.h" 
#include "./Bsp/wm8978/bsp_wm8978.h"  
#include "emXGUI.h"
#include "emXGUI_JPEG.h"
#include "GUI_VEDIOPLAYER_DIALOG.h"
#include "x_libc.h"
//#include "./mjpegplayer/GUI_AVIList_DIALOG.h"
//#include "./mjpegplayer/GUI_AVIPLAYER_DIALOG.h"
FIL       fileR ;
UINT      BytesRD;
#define   Frame_Buf_Size    (1024*30)
uint8_t   *Frame_buf;

static volatile uint8_t audiobufflag=0;
//__align(4) uint8_t   Sound_buf[4][1024*5]	__attribute__((at(0xd1bc0000)));
__align(4) uint8_t   Sound_buf[4][1024*5]	__EXRAM;

static uint8_t   *pbuffer;

uint32_t  mid;
uint32_t  Strsize;
uint16_t  Strtype;

TIM_HandleTypeDef TIM3_Handle;

volatile uint8_t video_timeout;
extern WAVEFORMAT*   wavinfo;
extern avih_TypeDef* avihChunk;
//extern HWND avi_wnd_time;
//extern int avi_chl;
void MUSIC_SAI_DMA_TX_Callback(void);

extern GUI_MUTEX*	AVI_JPEG_MUTEX;    // ����ȷ��һ֡ͼ���ú��ͷ������˳��߳�

static volatile int frame=0;
static volatile int t0=0;
volatile int avi_fps=0;

volatile BOOL bDrawVideo=FALSE;
extern SURFACE *pSurf1;

//static u32 alltime = 0;		//��ʱ�� 
//u32 cur_time; 		//��ǰ����ʱ�� 
uint8_t tmp=0;	
u32 pos;//�ļ�ָ��λ��
s32 time_sum = 0;
void AVI_play(char *filename)
{
	GUI_SemWait(Delete_VideoTask_Sem,0);//���һ���ź���,ȷ���ź������ɵ�ǰ���ų����ͷŵ�,��֤�˳����ź�����׼ȷ��
	
  FRESULT  res;
  uint32_t offset;
  uint16_t audiosize;
  uint8_t avires=0;
  uint8_t audiosavebuf;
  WCHAR buff[128];

	HDC hdc1;

  bDrawVideo=FALSE;
  Frame_buf = (uint8_t *)GUI_VMEM_Alloc(Frame_Buf_Size);
  
  hdc1 =CreateDC(pSurf1,NULL);
  
  pbuffer=Frame_buf;
//  GUI_DEBUG("%s", filename);
  res=f_open(&fileR,filename,FA_READ);
  if(res!=FR_OK)
  {
    GUI_VMEM_Free(Frame_buf);
    DeleteDC(hdc1);
		GUI_SemPost(Delete_VideoTask_Sem);//�ļ���ʧ��,����ǰ�ͷų�����Թرյ��ź���
    return;    
  }

  res=f_read(&fileR,pbuffer,20480,&BytesRD);


  avires=AVI_Parser(pbuffer);//����AVI�ļ���ʽ
  if(avires)
  {
    return;    
  }
  
  avires=Avih_Parser(pbuffer+32);//����avih���ݿ�
  if(avires)
  {
    return;    
  }
  //strl�б�
  avires=Strl_Parser(pbuffer+88);//����strh���ݿ�
  if(avires)
  {
    return;    
  }
  
  avires=Strf_Parser(pbuffer+164);//����strf���ݿ�
  if(res!=FR_OK)
  {
    return;    
  }
  
  mid=Search_Movi(pbuffer);//Ѱ��movi ID	�����ݿ飩	
  if(mid==0)
  {
    return;    
  }
//  
  Strtype=MAKEWORD(pbuffer+mid+6);//�����ͣ�movi�����������ַ���
  Strsize=MAKEDWORD(pbuffer+mid+8);//����С
  if(Strsize%2)Strsize++;//������1
  f_lseek(&fileR,mid+12);//������־ID  
//  
  offset=Search_Auds(pbuffer);
  if(offset==0)
  {
    return;    
  }  
  audiosize=*(uint8_t *)(pbuffer+offset+4)+256*(*(uint8_t *)(pbuffer+offset+5));
  if(audiosize==0)
  {
    offset=(uint32_t)pbuffer+offset+4;
    mid=Search_Auds((uint8_t *)offset);
    if(mid==0)
    {
      return;    
    }
    audiosize=*(uint8_t *)(mid+offset+4)+256*(*(uint8_t *)(mid+offset+5));
  }
  
  SAI_Play_Stop();			/* ֹͣI2S¼���ͷ��� */
	wm8978_Reset();		/* ��λWM8978����λ״̬ */	
  	/* ����WM8978оƬ������ΪDAC�����Ϊ���� */
	wm8978_CfgAudioPath(DAC_ON, SPK_ON|EAR_LEFT_ON | EAR_RIGHT_ON);
  wm8978_OutMute(0);
	/* ����������������ͬ���� */
	wm8978_SetOUT1Volume(VideoDialog.power);

	/* ����WM8978��Ƶ�ӿ�Ϊ�����ֱ�׼I2S�ӿڣ�16bit */
	wm8978_CfgAudioIF(SAI_I2S_STANDARD, 16);
  SAI_GPIO_Config();
  SAIxA_Tx_Config(SAI_I2S_STANDARD, SAI_PROTOCOL_DATASIZE_16BIT, SAI_AUDIO_FREQUENCY_11K);
  SAI_DMA_TX_Callback=MUSIC_SAI_DMA_TX_Callback;			//�ص�����ָwav_i2s_dma_callback
//  I2S_Play_Stop();
  SAIA_TX_DMA_Init((uint32_t )Sound_buf[1],(uint32_t )Sound_buf[2],audiosize/2);
  audiobufflag=0;	    
  video_timeout=0;
  audiosavebuf=0;
  audiobufflag=0;
  TIM3_Config((avihChunk->SecPerFrame/100)-1,20000-1);
  SAI_Play_Start();  
	
	t0= GUI_GetTickCount();

  
//   //�����ܳ���=ÿһ֡��Ҫ��ʱ�䣨s��*֡����
  VideoDialog.alltime=(avihChunk->SecPerFrame/1000)*avihChunk->TotalFrame;
  VideoDialog.alltime/=1000;//��λ����
//  WCHAR buff[128];
//  //char *str = NULL;
// // RECT rc0 = {0, 367,120,30};//��ǰʱ��
  x_wsprintf(buff, L"�ֱ��ʣ�%d*%d", videoplayer_img_w, videoplayer_img_h);
  SetWindowText(GetDlgItem(VideoDialog.Video_Hwnd, eID_TEXTBOX_RES), buff);

//  char *ss;
//  int length1=strlen(filename);
//  int length2=strlen(File_Path);
//  if(strncpy(filename,File_Path,length2))//�Ƚ�ǰn���ַ���������strcpy
//  {
//    ss = filename + length2;
//  }
  x_mbstowcs_cp936(buff, lcdlist[VideoDialog.playindex], 200);
  SetWindowText(GetDlgItem(VideoDialog.Video_Hwnd, eID_TEXTBOX_ITEM), buff);
  x_wsprintf(buff, L"%02d:%02d:%02d",
             VideoDialog.alltime/3600,(VideoDialog.alltime%3600)/60,VideoDialog.alltime%60);
  SetWindowText(GetDlgItem(VideoDialog.Video_Hwnd, eID_TEXTBOX_ALLTIME), buff);
//  
  while(!VideoDialog.SWITCH_STATE)//����ѭ��
  {

    if(!(avi_icon[3].state == FALSE))
    {
      GUI_msleep(10);
      continue;
    }    
		int t1;
    if(!VideoDialog.avi_chl)
    {

        
//        
//   //fptr������ļ�ָ���λ�ã�fsize���ļ����ܴ�С������֮��ı����͵�ǰʱ������ʱ���ı�����ͬ��fptr/fsize = cur/all��     
   VideoDialog.curtime=((double)fileR.fptr/fileR.fsize)*VideoDialog.alltime;
//   //���½�����
   //GUI_DEBUG("%d", VideoDialog.curtime*255/VideoDialog.alltime);
   if(!(SendMessage(VideoDialog.SBN_TIMER_Hwnd, SBM_GETSTATE,0,0)&SST_THUMBTRACK))
    SendMessage(VideoDialog.SBN_TIMER_Hwnd, SBM_SETVALUE, TRUE, VideoDialog.curtime*255/VideoDialog.alltime);     
   //InvalidateRect(VideoDialog.Video_Hwnd, NULL, FALSE);
   
   x_wsprintf(buff, L"%02d:%02d:%02d",///%02d:%02d:%02d alltime/3600,(alltime%3600)/60,alltime%60
              VideoDialog.curtime/3600,(VideoDialog.curtime%3600)/60,VideoDialog.curtime%60); 
   if(!VideoDialog.LIST_STATE)
    SetWindowText(GetDlgItem(VideoDialog.Video_Hwnd, eID_TEXTBOX_CURTIME), buff);    
	 if(Strtype==T_vids)//��ʾ֡
    {    	
			frame++;
			t1 =GUI_GetTickCount();
			if((t1 - t0) >= 1000)
			{
				
				avi_fps =frame;
				t0 =t1;
				frame =0;
			}

      //HDC hdc_mem,hdc;
      pbuffer=Frame_buf;
      

      res = f_read(&fileR,Frame_buf,Strsize+8,&BytesRD);//������֡+��һ������ID��Ϣ
//      GUI_DEBUG("%d", GUI_GetTickCount()-tt0);
      
      if(res != FR_OK)
      {
        GUI_DEBUG("E\n");
      }
         
			video_timeout=0;
		
			if(frame&1)
			{	
#if 1		//ֱ��д�����ڷ�ʽ.	

				HWND hwnd=VideoDialog.Video_Hwnd;
				//HDC hdc;
				//hdc =GetDC(VideoDialog.Video_Hwnd);
				
				//hdc =BeginPaint(hwnd,&ps);
        
        GUI_MutexLock(AVI_JPEG_MUTEX,0xFFFFFFFF);    // ��ȡ������
				//  JPEG_Out(hdc,160,89,Frame_buf,BytesRD);
				JPEG_Out(hdc1,0,0,Frame_buf,BytesRD);
//            ClrDisplay(hdc, &rc0, MapRGB(hdc, 0,0,0));
//            SetTextColor(hdc, MapRGB(hdc,255,255,255));
//            DrawText(hdc, buff,-1,&rc0,DT_VCENTER|DT_CENTER);
            

//           SetWindowText(GetDlgItem(VideoPlayer_hwnd, ID_TB5), buff);
        x_wsprintf(buff, L"֡�ʣ�%dFPS/s", avi_fps);
        if(!VideoDialog.LIST_STATE)
          SetWindowText(GetDlgItem(VideoDialog.Video_Hwnd, eID_TEXTBOX_FPS), buff);

        bDrawVideo=TRUE;
//        GUI_msleep(10);
        InvalidateRect(hwnd,NULL,TRUE); //������Ч��...

//			  ReleaseDC(VideoDialog.Video_Hwnd,hdc);
			 // EndPaint(hwnd,&ps);
        GUI_MutexUnlock(AVI_JPEG_MUTEX);              // ����������				
#endif

			}

//			while(bDrawVideo==TRUE)
//			{
//				GUI_msleep(5);
//			}

      while(video_timeout==0)
      {   
				//rt_thread_delay(1); //��Ҫ���ȣ�������ź���.				
        GUI_msleep(5);
      }      
	  bDrawVideo=FALSE;

      video_timeout=0;
    }//��ʾ֡
    else if(Strtype==T_auds)//��Ƶ���
    { 
      uint8_t i;
      audiosavebuf++;
      if(audiosavebuf>3)
			{
				audiosavebuf=0;
			}	
      do
      {
				//rt_thread_delay(1); 
        i=audiobufflag;
        if(i)
					i--;
        else 
					i=3; 

      }while(audiobufflag==i);
      AVI_DEBUG("S\n");

      res = f_read(&fileR,Sound_buf[audiosavebuf],Strsize+8,&BytesRD);//������֡+��һ������ID��Ϣ
      if(res != FR_OK)
      {
        GUI_DEBUG("E\n");
      }
      
      pbuffer=Sound_buf[audiosavebuf];      
      
    }
    else 
      break;
					   	
    }
    else
    {
         pos = fileR.fptr;
         //���ݽ�������������λ��				
         tmp=SendMessage(VideoDialog.SBN_TIMER_Hwnd, SBM_GETVALUE, NULL, NULL); 
         time_sum = fileR.fsize/VideoDialog.alltime*(tmp*VideoDialog.alltime/249-VideoDialog.curtime);//������������ ���㹫ʽ���ļ��ܴ�С/��Ҫ������������ = ��ʱ��/��ǰ��ʱ��
         //�����ǰ�ļ�ָ��δ�����
        	if(pos<fileR.fsize)pos+=time_sum; 
         //����ļ�ָ�뵽�����30K����
          if(pos>(fileR.fsize-1024*30))
          {
            pos=fileR.fsize-1024*30;
          }
         
          f_lseek(&fileR,pos);
//      
      #if 0
         if(pos == 0)
            mid=Search_Movi(Frame_buf);//Ѱ��movi ID  �ж��Լ��ǲ��ǻ������ݶ�
         else 
            mid = 0;  
        int iiii= 0;//����ƫ����
         while(1)
         {
            //ÿ�ζ�512���ֽڣ�ֱ���ҵ�����֡��֡ͷ
            u16 temptt = 0;//��������֡��λ��
            AVI_DEBUG("S\n");

            f_read(&fileR,Frame_buf,512,&BytesRD);
            AVI_DEBUG("E\n");

            temptt = Search_Fram(Frame_buf,BytesRD);
            iiii++;
            if(temptt)
            {            
               AVI_DEBUG("S temptt =%d\n",temptt);
               AVI_DEBUG("S Frame_buf[temptt] =%c %c %c %c\n",
                                      Frame_buf[temptt],
                                      Frame_buf[temptt+1],
                                      Frame_buf[temptt+2],
                                      Frame_buf[temptt+3]);
               /* ���ȡ512���ݣ���ֹ��־�ڱ߽�ʱ���� */
               f_read(&fileR,(u8*)Frame_buf+BytesRD,512,&BytesRD);
               AVI_DEBUG("E\n");
                pbuffer = Frame_buf;
               Strtype=MAKEWORD(pbuffer+temptt+2);//������
               Strsize=MAKEDWORD(pbuffer+temptt+4);//����С
               mid += temptt + 512*iiii-512;//����ƫ����
//               if(temptt == 16)
//                  continue;
               break;
            }

         }
         #else
         f_read(&fileR,Frame_buf,1024*30,&BytesRD);
         AVI_DEBUG("E\n");
         if(pos == 0)
            mid=Search_Movi(Frame_buf);//Ѱ��movi ID
         else 
            mid = 0;
         mid += Search_Fram(Frame_buf,1024*30);
         pbuffer = Frame_buf;
         Strtype=MAKEWORD(pbuffer+mid+2);//������
         Strsize=MAKEDWORD(pbuffer+mid+4);//����С
         #endif
         
         if(Strsize%2)Strsize++;//������1
         f_lseek(&fileR,pos+mid+8);//������־ID  
         AVI_DEBUG("S Strsize=%d\n",Strsize);

         f_read(&fileR,Frame_buf,Strsize+8,&BytesRD);//������֡+��һ������ID��Ϣ 
         
//         
         VideoDialog.avi_chl = 0;    
     }
         //�ж���һ֡��֡���� 
         Strtype=MAKEWORD(pbuffer+Strsize+2);//������
         Strsize=MAKEDWORD(pbuffer+Strsize+4);//����С									
         if(Strsize%2)Strsize++;//������1		  
//        
     }
//  
// 

	GUI_VMEM_Free(Frame_buf);
  DeleteDC(hdc1);
  if(!VideoDialog.SWITCH_STATE)
  {
    VideoDialog.playindex++;
    if(VideoDialog.playindex > VideoDialog.avi_file_num - 1) 
      VideoDialog.playindex = 0;
  }
  else
    VideoDialog.SWITCH_STATE = 0;
  SAI_Play_Stop();
	wm8978_Reset();	/* ��λWM8978����λ״̬ */
  HAL_TIM_Base_Stop_IT(&TIM3_Handle); //ֹͣ��ʱ��3�����ж�
  f_close(&fileR);
	GUI_SemPost(Delete_VideoTask_Sem);

}

void MUSIC_SAI_DMA_TX_Callback(void)
{
  audiobufflag++;
  if(audiobufflag>3)
	{
		audiobufflag=0;
	}
	
	if(DMA1_Stream2->CR&(1<<19)) //��ǰ��ȡMemory1����
	{
		//DMA_MemoryTargetConfig(DMA1_Stream2,(uint32_t)Sound_buf[audiobufflag], DMA_Memory_0);
    HAL_DMAEx_ChangeMemory(&h_txdma, (uint32_t)Sound_buf[audiobufflag], MEMORY0);
	}
	else
	{
    HAL_DMAEx_ChangeMemory(&h_txdma, (uint32_t)Sound_buf[audiobufflag], MEMORY1); 
	} 
}

/**
  * @brief  ͨ�ö�ʱ��3�жϳ�ʼ��
  * @param  period : �Զ���װֵ��
  * @param  prescaler : ʱ��Ԥ��Ƶ��
  * @retval ��
  * @note   ��ʱ�����ʱ����㷽��:Tout=((period+1)*(prescaler+1))/Ft us.
  *          Ft=��ʱ������Ƶ��,ΪSystemCoreClock/2=90,��λ:Mhz
  */
  
void TIM3_Config(uint16_t period,uint16_t prescaler)
{	
  __TIM3_CLK_ENABLE();

  TIM3_Handle.Instance = TIM3;
  /* �ۼ� TIM_Period�������һ�����»����ж�*/		
  //����ʱ����0������4999����Ϊ5000�Σ�Ϊһ����ʱ����
  TIM3_Handle.Init.Period = period;
  //��ʱ��ʱ��ԴTIMxCLK = 2 * PCLK1  
  //				PCLK1 = HCLK / 4 
  //				=> TIMxCLK=HCLK/2=SystemCoreClock/2=200MHz
  // �趨��ʱ��Ƶ��Ϊ=TIMxCLK/(TIM_Prescaler+1)=10000Hz
  TIM3_Handle.Init.Prescaler =  prescaler;
  TIM3_Handle.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  TIM3_Handle.Init.CounterMode = TIM_COUNTERMODE_UP;
  // ��ʼ����ʱ��TIM
  HAL_TIM_Base_Init(&TIM3_Handle);
  
  HAL_NVIC_SetPriority(TIM3_IRQn, 2, 0);
  HAL_NVIC_EnableIRQ(TIM3_IRQn); 
  // ������ʱ�������ж�
  HAL_TIM_Base_Start_IT(&TIM3_Handle);  
   
  
}

/**
  * @brief  ��ʱ��3�жϷ�����
  * @param  ��
  * @retval ��
  */
void TIM3_IRQHandler(void)
{
  HAL_TIM_IRQHandler(&TIM3_Handle);
}
