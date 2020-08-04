#ifndef _GUI_CAMERA_DIALOG_H_
#define _GUI_CAMERA_DIALOG_H_

#include "emXGUI.h"

typedef struct
{
  HWND Cam_Hwnd;
  HWND SetWIN;
  
  uint16_t *cam_buff0;
  uint16_t *cam_buff1;
  
  uint8_t fps;
  int flag;
  int cur_Resolution;
  int cur_LightMode;
  int cur_SpecialEffects;
  BOOL update_flag;
  BOOL focus_status;
  BOOL AutoFocus_Thread;
  BOOL Update_Thread;
}Cam_DIALOG_Typedef;


typedef enum
{
  eID_SET = 0x1000,
  eID_EXIT,
  eID_RETURN,
  eID_FPS,

	eID_SET1,            //�Զ��Խ�
	eID_SET2,            //����
	eID_SET3,            //���Ͷ�
	eID_SET4,            //�Աȶ�
  eID_SET5,            //�ֱ���
  eID_SET6,            //����ģʽ
  eID_SET7,            //����Ч��
  eID_SCROLLBAR,       //���Ȼ�����
  eID_SCROLLBAR1,      //���ͶȻ�����
  eID_SCROLLBAR2,      //�ԱȶȻ�����  
  
  eID_TB1,             //��ǰ�ֱ�����ʾ
  eID_TB2,             //��ǰ����ģʽ��ʾ   
  eID_TB3,             //��ǰ����Ч����ʾ
   
  eID_switch,          //�Զ��Խ�����
  eID_Setting1,        //���÷ֱ��ʰ�ť
  eID_Setting2,        //���ù���ģʽ��ť
  eID_Setting3,        //��������Ч����ť
  
  
  //��ѡ��---�ֱ���
  eID_RB1,             //320*240
  eID_RB2,             //480*272
  eID_RB3,             //800*480��Ĭ�ϣ�
  //��ѡ��---����ģʽ
  eID_RB4,             //�Զ�
  eID_RB5,             //����
  eID_RB6,             //����
  eID_RB7,             //�칫��
  eID_RB8,             //����  
  //��ѡ��---����Ч��
   eID_RB9,              //��ɫ
  eID_RB10,             //ůɫ
  eID_RB11,             //�ڰ�
  eID_RB12,             //����
  eID_RB13,             //��ɫ   
  eID_RB14,             //ƫ��
  eID_RB15,             //����
  eID_RB16,             //����  
  
  
  eID_BT1,             //�ֱ��ʽ��淵�ذ���
  eID_BT2,             //����ģʽ���淵�ذ���
  eID_BT3,             //����Ч�����淵�ذ���
}VideoDlg_Master_ID;

extern void GUI_Camera_DIALOGTest(void *param);
extern Cam_DIALOG_Typedef CamDialog;
extern GUI_SEM *cam_sem;//����ͼ��ͬ���ź�������ֵ�ͣ�
#endif
