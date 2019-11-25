#ifndef _GUI_MUSICPLAYER_DIALOG_H
#define _GUI_MUSICPLAYER_DIALOG_H


#define MUSIC_MAX_NUM           20
#define FILE_NAME_LEN           100			
#define MUSIC_NAME_LEN          100		
#define LYRIC_MAX_SIZE          200				
#define MAX_LINE_LEN            200				
#define _DF1S                   0x81
#define BUFFER_SIZE             1024*8
#define COMDATA_SIZE				  1024*5
#define COMDATA_UTF8_SIZE		  1024*2
#define MP3BUFFER_SIZE          2304		
#define INPUTBUF_SIZE           3000	

#define LRC_GBK   1  //1��ʾLRC���ΪGBK���룬0��ʾUTF8����
#define ID_TB1               0x1301
#define ID_TB2               0x1302
#define ID_TB5               0x1305

#define GUI_RGB_BACKGROUNG_PIC      "musicdesktop.jpg"

/******************��ť�ؼ�IDֵ***********************/
enum
{
	ID_BUTTON_Power = 0x1000,  //���� 
	ID_BUTTON_List   ,       //����List
	ID_BUTTON_Equa   ,       //������
	ID_BUTTON_Folder ,       //�ļ���
	ID_BUTTON_BACK   ,       //��һ��
	ID_BUTTON_START  ,       //��ͣ��
	ID_BUTTON_NEXT   ,       //��һ��
	ID_BUTTON_MINISTOP  ,    //�������ͣ��
	ID_BUTTON_BUGLE     ,    //���Ȱ�ť
};
/*****************�������ؼ�IDֵ*********************/
enum
{
 ID_SCROLLBAR_HORN =  0x1103,  //������
 ID_SCROLLBAR_POWER   ,   //������
 ID_SCROLLBAR_TIMER   ,   //������
};

/*****************�ı���ؼ�IDֵ*********************/
//��������ʾ���и��
enum
{
 ID_TEXTBOX_LRC1   =   0x1201 ,  //��ʵ�һ��
 ID_TEXTBOX_LRC2      ,          //��ʵڶ���
 ID_TEXTBOX_LRC3      ,          //��ʵ����У���ǰ�У�
 ID_TEXTBOX_LRC4      ,          //��ʵ�����
 ID_TEXTBOX_LRC5      ,          //��ʵ�����
};

#define ID_EXIT        0x3000

typedef struct{
   char *icon_name;//ͼ����
   RECT rc;        //λ����Ϣ
   BOOL state;     //״̬  
}icon_S;

typedef __packed struct 
{ 
	uint8_t 	indexsize; 	
  uint8_t		length_tbl[LYRIC_MAX_SIZE]; 
 	uint16_t	addr_tbl[LYRIC_MAX_SIZE];										
 	int32_t		curtime;										
  int32_t		oldtime;	
  int32_t 	time_tbl[LYRIC_MAX_SIZE];	
  uint8_t flag;//��ȡ�ļ���־λ
}LYRIC;

extern HWND MusicPlayer_hwnd;
extern int play_index;
extern char music_playlist[MUSIC_MAX_NUM][FILE_NAME_LEN];//����List
extern char music_lcdlist[MUSIC_MAX_NUM][MUSIC_NAME_LEN];//��ʾlist
extern uint8_t music_file_num;
extern uint8_t chgsch; //������������־λ
extern HWND music_wnd_time;
extern LYRIC lrc;
void com_utf82gbk(const char *src, char *str);
void com_gbk2utf8(const char *src, char *str);
extern uint8_t *comdata;
extern signed long hCOMUTF8Memory;
extern uint8_t *comdataUTF8;
extern icon_S music_icon[12];
extern HWND wnd_lrc1;//��ʴ��ھ��
extern HWND wnd_lrc2;//��ʴ��ھ��
extern HWND wnd_lrc3;//��ʴ��ھ��
extern HWND wnd_lrc4;//��ʴ��ھ��
extern HWND wnd_lrc5;//��ʴ��ھ��
extern uint8_t ReadBuffer1[1024*5];

extern HFONT Music_Player_hFont48;
extern HFONT Music_Player_hFont64;
extern HFONT Music_Player_hFont72;

#endif

