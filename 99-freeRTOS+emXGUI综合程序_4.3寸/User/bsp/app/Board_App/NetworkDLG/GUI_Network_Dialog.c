#include <emXGUI.h>
#include <string.h>
#include <stdio.h>
#include "x_libc.h"
#include "GUI_AppDef.h"
#include "netconf.h"
#include "GUI_Network_Dialog.h"
#include "tcp_echoclient.h"
#include "tcp_echoserver.h"
#include "udp_echoclient.h"

int		number_input_box(int x, int y, int w, int h,
							const WCHAR *pCaption,
							WCHAR *pOut,
							int MaxCount,
							HWND hwndParent);

/* ��ѡ�� ID */
#define ID_RB1    (0x1100 | (1<<16))
#define ID_RB2    (0x1101 | (1<<16))
#define ID_RB3    (0x1102 | (1<<16))

TaskHandle_t Network_Task_Handle =NULL;
TaskHandle_t TCPIP_Init_Task_Handle=NULL;
SemaphoreHandle_t Wait_TCPIP_Init_Sem;
extern void My_TCPIP_initialization(uint8_t *ipaddr_test);
int8_t NetworkTypeSelection = 0;

HWND Send_Handle;
HWND Receive_Handle;
HWND Network_Main_Handle;
HWND Message_Hwnd;

uint8_t network_start_flag=0;
uint8_t LWIP_Init_Start = 0;
uint8_t LWIP_Init_End = 0;
//extern __IO uint8_t EthLinkStatus;
__IO uint8_t EthLinkStatus;//�ò����ı���
__IO uint32_t LocalTime = 0; /* this variable is used to create a time reference incremented by 10ms */
DRV_NETWORK drv_network,drv_network_server;

extern uint8_t IP_ADDRESS[4];

uint16_t bsp_result=0;

/* �� Backend_vidoplayer.c ���� */
extern void TIM3_Config(uint16_t period,uint16_t prescaler);
extern TIM_HandleTypeDef TIM3_Handle;
/* �� sys_arch.c ���� */


/* LWIPЭ��ջ��ʼ��,��ʼ���ɹ���ɾ������,����������ں����ڡ� */
static void TCPIP_Init_Task(void *p)
{	
	LWIP_Init_End = 1;
	if(network_start_flag == 0)//����ɹ���һ��,���´򿪳���,�����ٳ�ʼ����
	{
		My_TCPIP_initialization(IP_ADDRESS);
	}
	while(1)
	{
		vTaskDelay(5000);
	}
}

void Network_Dispose_Task(void *p) 
{
	EnableWindow(GetDlgItem(Network_Main_Handle, eID_LINK_STATE), DISABLE);
	
	if(network_start_flag == 0)
	{
	if(xSemaphoreTake( Wait_TCPIP_Init_Sem,5000) != pdTRUE)
		{
      network_start_flag=0;
      bsp_result |=1;
      /* ��ʼ������ */
      SetTimer(Network_Main_Handle, 10, 50, TMR_SINGLE|TMR_START, NULL);
      vTaskSuspend(Network_Task_Handle);    // �����Լ� ����ִ�� 
    }
    else
    {
      network_start_flag=1;
      bsp_result &=~ 1;  
    }
	}
	DestroyWindow(Message_Hwnd);
	EnableWindow(GetDlgItem(Network_Main_Handle, eID_Network_EXIT), ENABLE);
	EnableWindow(GetDlgItem(Network_Main_Handle, eID_LINK_STATE), ENABLE);
	
#if 1
  if((drv_network.net_init==0)&&((bsp_result&1)==0))
  {
    /* Initilaize the LwIP stack */
		/* Config TCP Server IP and Local IP*/
    drv_network.net_local_ip1  = IP_ADDRESS[0];
    drv_network.net_local_ip2  = IP_ADDRESS[1];
    drv_network.net_local_ip3  = IP_ADDRESS[2];
    drv_network.net_local_ip4  = IP_ADDRESS[3];
    drv_network.net_local_port = LOCAL_PORT;
    
	  drv_network_server.net_local_ip1  = IP_ADDRESS[0];
    drv_network_server.net_local_ip2  = IP_ADDRESS[1];
    drv_network_server.net_local_ip3  = IP_ADDRESS[2];
    drv_network_server.net_local_ip4  = IP_ADDRESS[3];
    drv_network_server.net_local_port = LOCAL_PORT;
		drv_network_server.net_type = 1;//TCP_SERVER
		
    drv_network.net_remote_ip1  = DEST_IP_ADDR0;
    drv_network.net_remote_ip2  = DEST_IP_ADDR1;
    drv_network.net_remote_ip3  = DEST_IP_ADDR2;
    drv_network.net_remote_ip4  = DEST_IP_ADDR3;
    drv_network.net_remote_port = DEST_PORT;     
    
    drv_network.net_init=1;
  }
 #endif 
//  PostCloseMessage(GetDlgItem(Network_Main_Handle, ID_Hint_Win));

  InvalidateRect(Network_Main_Handle, NULL, TRUE);
  drv_network.net_connect=0;
  drv_network.net_type=0; 
//  TIM3_Config(999,899);//10ms��ʱ�� 
  LocalTime=0;
//  TIM_SetCounter(TIM3,0);
//  HAL_TIM_Base_Start_IT(&TIM3_Handle); 
  EthLinkStatus=0;
		/* ���жϴ�����ܵ������� ---> HAL_ETH_RxCpltCallback */
	GUI_Thread_Delete(Network_Task_Handle);    // ɾ�����紦������
	GUI_Thread_Delete(TCPIP_Init_Task_Handle); //ɾ����ʼ������		
	while(1){GUI_Yield();}
}

//�˳���ť�ػ���
static void Ent_ExitButton_OwnerDraw(DRAWITEM_HDR *ds)
{
  HDC hdc;
  RECT rc;
//  HWND hwnd;

	hdc = ds->hDC;   
	rc = ds->rc; 
//  hwnd = ds->hwnd;

//  GetClientRect(hwnd, &rc_tmp);//�õ��ؼ���λ��
//  WindowToScreen(hwnd, (POINT *)&rc_tmp, 1);//����ת��

//  BitBlt(hdc, rc.x, rc.y, rc.w, rc.h, hdc_bk, rc_tmp.x, rc_tmp.y, SRCCOPY);

  if (ds->State & BST_PUSHED)
	{ //��ť�ǰ���״̬
		SetPenColor(hdc, MapRGB(hdc, 1, 191, 255));
	}
	else
	{ //��ť�ǵ���״̬

		SetPenColor(hdc, MapRGB(hdc, 250, 250, 250));      //���û���ɫ
	}

  rc.w = 25;

  OffsetRect(&rc, 0, 3);
  
  for(int i=0; i<4; i++)
  {
    HLine(hdc, rc.x, rc.y, rc.w);
    rc.y += 6;
  }

}

// �ػ���ͨ��ť
static void Ent_Button_OwnerDraw(DRAWITEM_HDR *ds)
{
  HDC hdc;
  RECT rc;
  WCHAR wbuf[128];
  HWND hwnd;

	hdc = ds->hDC;   
	rc = ds->rc; 
  hwnd = ds->hwnd;

  SetBrushColor(hdc, MapRGB(hdc, 0, 202, 252));
  EnableAntiAlias(hdc, ENABLE);
  FillRoundRect(hdc, &rc, 5);
  EnableAntiAlias(hdc, DISABLE);

  if (ds->State & BST_PUSHED)
	{ //��ť�ǰ���״̬
    OffsetRect(&rc, 1, 1);
		SetTextColor(hdc, MapRGB(hdc, 200, 200, 200));
	}
	else
	{ //��ť�ǵ���״̬

		SetTextColor(hdc, MapRGB(hdc, 255, 255, 255));      //���û���ɫ
	}

  GetWindowText(hwnd, wbuf, 128);    // �õ�ԭ�ı�

  SetFont(hdc, defaultFont);
  DrawText(hdc, wbuf, -1, &rc, DT_VCENTER|DT_CENTER);     // ��������(������ʾ)
}

/*
 * @brief  �ػ���ʾ���ȵ�͸���ı�
 * @param  ds:	�Զ�����ƽṹ��
 * @retval NONE
*/
static void Brigh_Textbox_OwnerDraw(DRAWITEM_HDR *ds) //����һ����ť���
{
	HWND hwnd;
	HDC hdc;
	RECT rc;
	WCHAR *wbuf;
  uint32_t WinTexeLen = 0;

	hwnd = ds->hwnd; //button�Ĵ��ھ��.
	hdc = ds->hDC;   //button�Ļ�ͼ�����ľ��.
  rc = ds->rc;

  SetTextColor(hdc, MapRGB(hdc, 170, 170, 170));
	SetBrushColor(hdc,MapRGB(hdc,250,250,250));
  if (ds->ID == ID_TEXTBOX_Receive)
  {
    DrawText(hdc, L"������", -1, &rc, DT_VCENTER|DT_CENTER);  // ��������
  }
  else
  {
    SetPenColor(hdc, MapRGB(hdc, 121, 121, 121));
		FillRect(hdc,&rc);
    EnableAntiAlias(hdc, ENABLE);
    DrawRoundRect(hdc, &rc, 7);
    EnableAntiAlias(hdc, DISABLE);
    DrawText(hdc, L"������", -1, &rc, DT_VCENTER|DT_CENTER);  // ��������
  }
  
  OffsetRect(&rc, 5, 5);
  SetTextColor(hdc, MapRGB(hdc, 0, 0, 0));
  WinTexeLen = GetWindowTextLength(hwnd);               // ��ȡ�ı�����

  if (WinTexeLen > 0)
  {
    wbuf = (WCHAR *)GUI_VMEM_Alloc(WinTexeLen*2);         // �����ı����� + ����Ϣ���ȵ��ڴ�
    GetWindowText(hwnd, wbuf, WinTexeLen+1);    // �õ�ԭ�ı�

    SetFont(hdc, defaultFont);
    DrawText(hdc, wbuf, -1, &rc, DT_TOP|DT_LEFT|DT_WORDBREAK);     // ��������( �������룬����룬���ַ��������α߽�ʱ,�Զ�����)
  }
}

extern void TCP_Echo_Init(void);
extern int SelectDialogBox(HWND hwndParent, RECT rc,const WCHAR *pText,const WCHAR *pCaption,const MSGBOX_OPTIONS *ops);
WCHAR I[128];
static LRESULT	win_proc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
  switch(msg)
  {
    case WM_CREATE:
    {
			
			SetTimer(hwnd, 5, 50, TMR_START, NULL);
      RECT rc;
      GetClientRect(hwnd, &rc); 
      HWND Temp_Handle;
			
			Wait_TCPIP_Init_Sem = GUI_SemCreate(0,1);
			BaseType_t xReturn = pdPASS;
			if(LWIP_Init_End == 0)
			{
				xReturn = xTaskCreate((TaskFunction_t )TCPIP_Init_Task,      /* ������ں��� */
															(const char*    )"TCPIP Init Task",    /* �������� */
															(uint16_t       )1024,                  /* ����ջ��СFreeRTOS������ջ����Ϊ��λ */
															(void*          )NULL,                      /* ������ں������� */
															(UBaseType_t    )6,                         /* ��������ȼ� */
															(TaskHandle_t*  )&TCPIP_Init_Task_Handle);     /* ������ƿ�ָ�� */
															
			 if(xReturn != pdPASS)  
					GUI_ERROR("Fail to create Network_Dispose_Task!\r\n");

      xReturn = xTaskCreate((TaskFunction_t )Network_Dispose_Task,      /* ������ں��� */
														(const char*    )"Network Dispose Task",    /* �������� */
														(uint16_t       )1024,                  /* ����ջ��СFreeRTOS������ջ����Ϊ��λ */
														(void*          )NULL,                      /* ������ں������� */
														(UBaseType_t    )6,                         /* ��������ȼ� */
														(TaskHandle_t*  )&Network_Task_Handle);     /* ������ƿ�ָ�� */
      if(xReturn != pdPASS)  
				GUI_ERROR("Fail to create Network_Dispose_Task!\r\n");
			}else
			{
				if(network_start_flag == 0)
				{
					SetTimer(Network_Main_Handle, 10, 50, TMR_SINGLE|TMR_START, NULL);
					LWIP_Init_End = 2;
				}
			}			
      CreateWindow(BUTTON, L"O", WS_TRANSPARENT|BS_FLAT | BS_NOTIFY |WS_OWNERDRAW|WS_VISIBLE,
                  444, 0, 36, 30, hwnd, eID_Network_EXIT, NULL, NULL); 

      /* ����һ�鵥ѡ�� */
      rc.x = 232;
      rc.y = 51;
      rc.w = 100;
      rc.h = 18;
      CreateWindow(BUTTON,L"TCPServer",BS_RADIOBOX|WS_VISIBLE,
      rc.x,rc.y,rc.w,rc.h,hwnd,ID_RB1,NULL,NULL);
      
      OffsetRect(&rc, rc.w, 0);
      CreateWindow(BUTTON,L"TCPClient",BS_RADIOBOX|WS_VISIBLE,
      rc.x,rc.y,rc.w,rc.h,hwnd,ID_RB2,NULL,NULL);
			SendMessage(GetDlgItem(hwnd, ID_RB2&0xFFFF), BM_SETSTATE, BST_CHECKED, 0);    // Ĭ��ѡ��

      OffsetRect(&rc, rc.w, 0);
      rc.w = 52;
      CreateWindow(BUTTON,L"UDP",BS_RADIOBOX|WS_VISIBLE,
      rc.x,rc.y,rc.w,rc.h,hwnd,ID_RB3,NULL,NULL);
      
      CreateWindow(BUTTON, L"δ����", WS_TRANSPARENT | BS_NOTIFY|WS_VISIBLE|WS_OWNERDRAW,
                  415, 131, 64, 20, hwnd, eID_LINK_STATE, NULL, NULL);
      
      /* ���ݷ����ı����� */
      rc.w = 245;
      rc.h = 116;
      rc.x = 232;
      rc.y = 154;
      Send_Handle = CreateWindow(TEXTBOX, L"��ã�������Ұ�𿪷��� ^_^", WS_VISIBLE|WS_OWNERDRAW, rc.x, rc.y, rc.w, rc.h, hwnd, ID_TEXTBOX_Send, NULL, NULL);

      /* �������մ��� */
      rc.w = 226;
      rc.h = 214;
      rc.x = 3;
      rc.y = 31;
      Receive_Handle = CreateWindow(TEXTBOX, L"", WS_TRANSPARENT|WS_VISIBLE|WS_OWNERDRAW, rc.x, rc.y, rc.w, rc.h, hwnd, ID_TEXTBOX_Receive, NULL, NULL);
      
      rc.x = 431;
      rc.y = 247;
      rc.w = 43;
      rc.h = 20;
      CreateWindow(BUTTON, L"����", WS_TRANSPARENT | BS_NOTIFY|WS_VISIBLE|WS_OWNERDRAW,
                         rc.x,rc.y,rc.w,rc.h, hwnd, eID_Network_Send, NULL, NULL); 
                         
      rc.x = 156;
      rc.h = 20;
      rc.w = 68;
      rc.y = 247;
      CreateWindow(BUTTON, L"��ս���", WS_TRANSPARENT | BS_NOTIFY|WS_VISIBLE|WS_OWNERDRAW,
                         rc.x,rc.y,rc.w,rc.h, hwnd, eID_Receive_Clear,       NULL, NULL); 

      /* ���ݷ����ı����� */
      rc.w = 31;
      rc.h = 20;
      rc.x = 232;
      rc.y = 131;
      Temp_Handle = CreateWindow(TEXTBOX, L"192", WS_VISIBLE|WS_BORDER, rc.x, rc.y, rc.w, rc.h, hwnd, ID_TEXTBOX_RemoteIP1, NULL, NULL);//
      SendMessage(Temp_Handle, TBM_SET_TEXTFLAG, 0, DT_VCENTER | DT_CENTER | DT_BKGND);

      OffsetRect(&rc, rc.w+3, 0);
      Temp_Handle = CreateWindow(TEXTBOX, L"168", WS_VISIBLE|WS_BORDER, rc.x, rc.y, rc.w, rc.h, hwnd, ID_TEXTBOX_RemoteIP2, NULL, NULL);//
      SendMessage(Temp_Handle, TBM_SET_TEXTFLAG, 0, DT_VCENTER | DT_CENTER | DT_BKGND);

      OffsetRect(&rc, rc.w+3, 0);
      Temp_Handle = CreateWindow(TEXTBOX, L"000", WS_VISIBLE|WS_BORDER, rc.x, rc.y, rc.w, rc.h, hwnd, ID_TEXTBOX_RemoteIP3, NULL, NULL);//
      SendMessage(Temp_Handle, TBM_SET_TEXTFLAG, 0, DT_VCENTER | DT_CENTER | DT_BKGND);

      OffsetRect(&rc, rc.w+3, 0);
      Temp_Handle = CreateWindow(TEXTBOX, L"138", WS_VISIBLE|WS_BORDER, rc.x, rc.y, rc.w, rc.h, hwnd, ID_TEXTBOX_RemoteIP4, NULL, NULL);//
      SendMessage(Temp_Handle, TBM_SET_TEXTFLAG, 0, DT_VCENTER | DT_CENTER | DT_BKGND);

      OffsetRect(&rc, rc.w+3 ,0);
      rc.w = 48;
      Temp_Handle = CreateWindow(TEXTBOX, L"8080", WS_VISIBLE|WS_BORDER, rc.x, rc.y, rc.w-2, rc.h, hwnd, ID_TEXTBOX_RemotePort, NULL, NULL);//
      SendMessage(Temp_Handle, TBM_SET_TEXTFLAG, 0, DT_VCENTER | DT_CENTER | DT_BKGND);    
			if(LWIP_Init_Start == 0)
			{
				LWIP_Init_Start = 1;
				RECT RC;
				RC.w = 180;
				RC.h = 80;
				RC.x = (GUI_XSIZE - RC.w) >> 1;
				RC.y = (GUI_YSIZE - RC.h) >> 1;
				Message_Hwnd = CreateWindow(TEXTBOX, L"���ڳ�ʼ��\r\n\n��ȴ�...", WS_VISIBLE|WS_BORDER|WS_OVERLAPPED, RC.x, RC.y, RC.w, RC.h, hwnd, ID_TEXTBOX_Wait, NULL, NULL);//
				SendMessage(Message_Hwnd, TBM_SET_TEXTFLAG, 0, DT_VCENTER | DT_CENTER | DT_BKGND);
				EnableWindow(GetDlgItem(hwnd, eID_Network_EXIT), DISABLE);
			}
			
      break;
    } 
    case WM_TIMER:
    {
      int tmr_id;

      tmr_id = wParam;    // ��ʱ�� ID

      if (tmr_id == 10)    // ��̫����ʼ������
      {
        RECT RC;
        MSGBOX_OPTIONS ops;
        const WCHAR *btn[] ={L"ȷ��",L"ȡ��"};      //�Ի����ڰ�ť������

        ops.Flag =MB_ICONERROR;
        ops.pButtonText =btn;
        ops.ButtonCount =2;
        RC.w = 200;
        RC.h = 100;
        RC.x = (GUI_XSIZE - RC.w) >> 1;
        RC.y = (GUI_YSIZE - RC.h) >> 1;
        SelectDialogBox(hwnd, RC, L"��̫����ʼ��ʧ��\n�����¼������\r\n����λ�����塣", L"����", &ops);    // ��ʾ������ʾ��
        PostCloseMessage(hwnd);                                                          // ���͹رմ��ڵ���Ϣ
      }
			
      if (tmr_id == 5)    // ��̫����ʼ������
      {

      }
			
      break;
    }

    case WM_PAINT:
    {
      HDC hdc;
      PAINTSTRUCT ps;
      WCHAR tempstr[30];
      
      RECT rc =  {0, 0, GUI_XSIZE, GUI_YSIZE};
      // hdc_pointer = CreateMemoryDC(SURF_SCREEN, PANEL_W, PANEL_H);
      hdc = BeginPaint(hwnd, &ps);

      SetBrushColor(hdc, MapRGB(hdc, 255, 255, 255));
      FillRect(hdc, &rc);

      rc.h = 28;
      GradientFillRect(hdc, &rc, MapRGB(hdc, 1, 218, 254), MapRGB(hdc, 1, 168, 255), FALSE);
      SetTextColor(hdc, MapRGB(hdc, 255, 255, 255));
      DrawText(hdc, L"��̫��", -1, &rc, DT_VCENTER|DT_CENTER);

      SetPenColor(hdc, MapRGB(hdc, 121, 121, 121));

      rc.x = 3;
      rc.y = 31;
      rc.w = 226;
      rc.h = 239;
      EnableAntiAlias(hdc, ENABLE);
      DrawRoundRect(hdc, &rc, 7);     // ���ƽ����������
      EnableAntiAlias(hdc, DISABLE);
      
      SetFont(hdc, defaultFont);
      SetTextColor(hdc, MapRGB(hdc, 0x16, 0x9B, 0xD5));

      rc.x = 232;
      rc.y = 32;
      rc.w = 91;
      rc.h = 20;
      DrawText(hdc, L"ͨѶЭ�飺", -1, &rc, DT_LEFT|DT_TOP);

      // rc.x = 550;
      // rc.y = 36;
      // rc.w = 60;
      // rc.h = 30;
      // DrawText(hdc, L"���ݷ��ͣ�", -1, &rc, DT_LEFT|DT_TOP);
      
      rc.w = 143;
      rc.h = 20;
      rc.x = 232;
      rc.y = 71;
      DrawText(hdc, L"����IP��ַ&�˿ڣ�", -1, &rc, DT_LEFT|DT_TOP);
      
      rc.y = 111;
      DrawText(hdc, L"Զ��IP��ַ&�˿ڣ�", -1, &rc, DT_LEFT|DT_TOP);
      
      SetTextColor(hdc, MapRGB(hdc, 10, 10, 10));
//      x_wsprintf(tempstr, L"[%d.%d.%d.%d:%d]",drv_network.net_local_ip1,drv_network.net_local_ip2,\
//                                       drv_network.net_local_ip3,drv_network.net_local_ip4,\
//                                       drv_network.net_local_port);
      x_wsprintf(tempstr, L"[%d.%d.%d.%d:%d]",IP_ADDRESS[0],IP_ADDRESS[1],IP_ADDRESS[2],IP_ADDRESS[3],LOCAL_PORT);
      rc.w = 184;
      rc.h = 20;
      rc.x = 232;
      rc.y = 91;
      DrawText(hdc, tempstr, -1, &rc, DT_LEFT|DT_TOP);
      
      SetTextColor(hdc, MapRGB(hdc, 0x16, 0x9B, 0xD5));

      // rc.w = 120;
      // rc.h = 30;
      // rc.x = 10;
      // rc.y = 210;
      // DrawText(hdc, L"���ݽ��գ�", -1, &rc, DT_LEFT|DT_TOP);

      rc.w = 16;
      rc.h = 20;
      rc.x = 259;
      rc.y = 133;
      DrawText(hdc, L".", -1, &rc, DT_LEFT|DT_BOTTOM);
      
      rc.x = 293;
      DrawText(hdc, L".", -1, &rc, DT_LEFT|DT_BOTTOM);
      
      rc.x = 327;
      DrawText(hdc, L".", -1, &rc, DT_LEFT|DT_BOTTOM);
	  
      rc.x = 360;
      DrawText(hdc, L":", -1, &rc, DT_LEFT|DT_BOTTOM);

      EndPaint(hwnd, &ps);
      break;
    }

    case WM_DRAWITEM:
    {
       DRAWITEM_HDR *ds;
       ds = (DRAWITEM_HDR*)lParam;
       switch(ds->ID)
       {
          case eID_Network_EXIT:
          {
            Ent_ExitButton_OwnerDraw(ds);
            return TRUE;             
          }  

          case ID_TEXTBOX_Send:
          case ID_TEXTBOX_Receive:
          {
            Brigh_Textbox_OwnerDraw(ds);
            return TRUE;   
          }

          case eID_LINK_STATE:
          case eID_Network_Send:
          case eID_Receive_Clear:
          {
            Ent_Button_OwnerDraw(ds);
            return TRUE;   
          }
       }

       break;
    }

    //����TEXTBOX�ı�����ɫ�Լ�������ɫ
		case	WM_CTLCOLOR:
		{
			/* �ؼ��ڻ���ǰ���ᷢ�� WM_CTLCOLOR��������.
			 * wParam����ָ���˷��͸���Ϣ�Ŀؼ�ID;lParam����ָ��һ��CTLCOLOR�Ľṹ��ָ��.
			 * �û�����ͨ������ṹ��ı�ؼ�����ɫֵ.�û��޸���ɫ�������践��TRUE������ϵͳ
			 * �����Ա��β���������ʹ��Ĭ�ϵ���ɫ���л���.
			 *
			 */
			u16 id;
			id =LOWORD(wParam);
      
			if(id >= ID_TEXTBOX_RemoteIP1 && id <= ID_TEXTBOX_RemotePort)
			{
				CTLCOLOR *cr;
				cr =(CTLCOLOR*)lParam;
				cr->TextColor =RGB888(0, 0, 0);//������ɫ��RGB888��ɫ��ʽ)
				cr->BackColor =RGB888(255, 255, 255);//������ɫ��RGB888��ɫ��ʽ)
				cr->BorderColor =RGB888(10,10,10);//�߿���ɫ��RGB888��ɫ��ʽ)
				return TRUE;				
			}

      return FALSE;
		}

    case WM_NOTIFY:
    {
      u16 code, id;
      id  =LOWORD(wParam);//��ȡ��Ϣ��ID��
      code=HIWORD(wParam);//��ȡ��Ϣ������    
      if(code == BN_CLICKED && id == eID_Network_EXIT)    // �˳���ť����
      {
				switch(NetworkTypeSelection)
				{
					case 0:
						tcp_echoserver_close();
						break;
					case 1:
						tcp_echoclient_disconnect();
						break;
					case 2:
						udp_echoclient_disconnect();	
						break;   
				}
        PostCloseMessage(hwnd);    // ���͹رմ��ڵ���Ϣ
        break;
      }
      if(code == BN_CLICKED && id == eID_Receive_Clear)    // ��ս��յĴ��ڵİ�ť������
      {
        SetWindowText(Receive_Handle, L"");
        break;
      }
      
      if(code == TBN_CLICKED && id == ID_TEXTBOX_RemoteIP1)    // IP1 �༭�򱻰���
      {
        number_input_box(0, 0, GUI_XSIZE, GUI_YSIZE, L"IP1", I, 3, hwnd);
        SetWindowText(GetDlgItem(hwnd, ID_TEXTBOX_RemoteIP1), I);
        break;
      }
      
      if(code == TBN_CLICKED && id == ID_TEXTBOX_RemoteIP2){    // IP2 �༭�򱻰���
        number_input_box(0, 0, GUI_XSIZE, GUI_YSIZE, L"IP2", I, 3, hwnd);
        SetWindowText(GetDlgItem(hwnd, ID_TEXTBOX_RemoteIP2), I);
        break;
      }
      
      if(code == TBN_CLICKED && id == ID_TEXTBOX_RemoteIP3){    // IP3 �༭�򱻰���
        number_input_box(0, 0, GUI_XSIZE, GUI_YSIZE, L"IP3", I, 3, hwnd);
        SetWindowText(GetDlgItem(hwnd, ID_TEXTBOX_RemoteIP3), I);
        break;
      }
      
      if(code == TBN_CLICKED && id == ID_TEXTBOX_RemoteIP4){    // IP4 �༭�򱻰���
        number_input_box(0, 0, GUI_XSIZE, GUI_YSIZE, L"IP4", I, 3, hwnd);
        SetWindowText(GetDlgItem(hwnd, ID_TEXTBOX_RemoteIP4), I);
        break;
      }
      
      if(code == TBN_CLICKED && id == ID_TEXTBOX_RemotePort)    // �˿� �༭�򱻰���
      {
        number_input_box(0, 0, GUI_XSIZE, GUI_YSIZE, L"PORT", I, 5, hwnd);
        SetWindowText(GetDlgItem(hwnd, ID_TEXTBOX_RemotePort), I);
        break;
      }
      
      if(code == BN_CLICKED && id == eID_LINK_STATE)
      {
        if((bsp_result&1)||EthLinkStatus)
          {
            break;
          }
          if(drv_network.net_connect==0)
          {
            uint8_t connectflag;
            WCHAR wbuf[128];
            char buf[128];

            GetWindowText(GetDlgItem(hwnd, ID_TEXTBOX_RemoteIP1), wbuf, 128);    // ��ȡ�ı�����ı�
            x_wcstombs_cp936(buf, wbuf, 128);                                    // �����ַ���תΪ���ַ���
            drv_network.net_remote_ip1 = x_atoi(buf);                            // �ַ���ת����
//						drv_network.net_local_ip1 = x_atoi(buf);
            GetWindowText(GetDlgItem(hwnd, ID_TEXTBOX_RemoteIP2), wbuf, 128);
            x_wcstombs_cp936(buf, wbuf, 128);
            drv_network.net_remote_ip2 = x_atoi(buf);
//						drv_network.net_local_ip2 = x_atoi(buf);
            GetWindowText(GetDlgItem(hwnd, ID_TEXTBOX_RemoteIP3), wbuf, 128);
            x_wcstombs_cp936(buf, wbuf, 128);
            drv_network.net_remote_ip3 = x_atoi(buf);
//						drv_network.net_local_ip3 = x_atoi(buf);
            GetWindowText(GetDlgItem(hwnd, ID_TEXTBOX_RemoteIP4), wbuf, 128);
            x_wcstombs_cp936(buf, wbuf, 128);
            drv_network.net_remote_ip4 = x_atoi(buf);
//						drv_network.net_local_ip4 = x_atoi(buf);
            GetWindowText(GetDlgItem(hwnd, ID_TEXTBOX_RemotePort), wbuf, 128);
            x_wcstombs_cp936(buf, wbuf, 128);
            drv_network.net_remote_port = x_atoi(buf);
//						drv_network.net_local_port = x_atoi(buf);
            drv_network.net_type=NetworkTypeSelection;
            switch(drv_network.net_type)
            {
              case 0:
                /*create tcp server */ 
                connectflag=tcp_echoserver_init(drv_network_server);
                break;
              case 1:
                /*connect to tcp server */
                connectflag=tcp_echoclient_connect(drv_network);
                break;
              case 2:
                /* Connect to tcp server */ 
                connectflag=udp_echoclient_connect(drv_network);		
                break;            
            }
            if(connectflag==0)    // ���ӳɹ�
            {
              drv_network.net_connect=1; 
              SetWindowText(GetDlgItem(hwnd, eID_LINK_STATE), L"����Ͽ�");
            }else
						{
						
						}
          }
          else
          {
            /* �Ͽ����� */
            SetWindowText(GetDlgItem(hwnd, eID_LINK_STATE), L"δ����");
            switch(drv_network.net_type)
            {
              case 0:
                tcp_echoserver_close();
                break;
              case 1:
                tcp_echoclient_disconnect();
                break;
              case 2:
                udp_echoclient_disconnect();	
                break;            
            }
            drv_network.net_connect=0;
          }
        }
        if(code == BN_CLICKED && id == eID_Network_Send)
        {
          if(drv_network.net_connect==1)          
          {
            WCHAR wbuf[128];
            char comdata[128];

            /* ��ȡ���ʹ��ڵ��ַ��� */
            GetWindowText(GetDlgItem(hwnd, ID_TEXTBOX_Send), wbuf, 128);
            x_wcstombs_cp936(comdata, wbuf, 128);

            /* ������Ϣ */
            switch(drv_network.net_type)
            {
              case 0:
                network_tcpserver_send((char *)comdata);
                break;
              case 1:
                network_tcpclient_send((char *)comdata);
                break;
              case 2:
                udp_echoclient_send((char *)comdata);
                break;            
            }
          }
        }
      
        /* ��ѡ��ť������ */
        if( (id >= (ID_RB1 & ~(1<<16))) && (id <= (ID_RB3 & ~(1<<16))))
        {
          if (code == BN_CLICKED)
          {
            NetworkTypeSelection = id & 3;
          }
        }

      break;
    } 

    case WM_DESTROY:
    { 

      NetworkTypeSelection = 0;                  // ��λĬ�ϵ�ѡ��
      udp_echoclient_disconnect();
			tcp_echoclient_disconnect();
			tcp_echoserver_close();
			GUI_SemDelete(Wait_TCPIP_Init_Sem);

      return PostQuitMessage(hwnd);	
    } 

    default:
      return	DefWindowProc(hwnd, msg, wParam, lParam);   
  }
  
  return WM_NULL;
  
}

void GUI_NetworkDLG_Dialog(void)
{
	
	WNDCLASS	wcex;
	MSG msg;
	wcex.Tag = WNDCLASS_TAG;

	wcex.Style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = win_proc; //������������Ϣ����Ļص�����.
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = NULL;//hInst;
	wcex.hIcon = NULL;//LoadIcon(hInstance, (LPCTSTR)IDI_WIN32_APP_TEST);
	wcex.hCursor = NULL;//LoadCursor(NULL, IDC_ARROW);
   
	//����������
	Network_Main_Handle = CreateWindowEx(WS_EX_NOFOCUS|WS_EX_FRAMEBUFFER,
                              &wcex,
                              L"GUI_Network_Dialog",
                              WS_VISIBLE|WS_CLIPCHILDREN,
                              0, 0, GUI_XSIZE, GUI_YSIZE,
                              NULL, NULL, NULL, NULL);

   //��ʾ������
	ShowWindow(Network_Main_Handle, SW_SHOW);

	//��ʼ������Ϣѭ��(���ڹرղ�����ʱ,GetMessage������FALSE,�˳�����Ϣѭ��)��
	while (GetMessage(&msg, Network_Main_Handle))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
}


