#include "emXGUI.h"
//͸���ı�
static void PicViewer_TBOX_OwnerDraw(DRAWITEM_HDR *ds) //����һ����ť���
{
	HWND hwnd;
	HDC hdc;
	RECT rc, rc_tmp;
	WCHAR wbuf[128];

	hwnd = ds->hwnd; //button�Ĵ��ھ��.
	hdc = ds->hDC;   //button�Ļ�ͼ�����ľ��.
  GetClientRect(hwnd, &rc_tmp);//�õ��ؼ���λ��
  GetClientRect(hwnd, &rc);//�õ��ؼ���λ��
  WindowToScreen(hwnd, (POINT *)&rc_tmp, 1);//����ת��
  
//  SetBrushColor(hdc, MapRGB(hdc, 0,0,0));
//  FillRect(hdc, &rc);
  //BitBlt(hdc, rc.x, rc.y, rc.w, rc.h, PicViewer.mhdc_pic, rc_tmp.x, rc_tmp.y, SRCCOPY);
  SetTextColor(hdc, MapRGB(hdc, 255, 255, 255));


  GetWindowText(hwnd, wbuf, 128); //��ð�ť�ؼ�������

  DrawText(hdc, wbuf, -1, &rc, DT_VCENTER|DT_CENTER);//��������(���ж��뷽ʽ)
}
static	LRESULT test_proc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
  switch(msg)
  {
    case WM_CREATE:
    {
      CreateWindow(BUTTON, L"֧��jpg��bmp��png��gif��ʽ", WS_TRANSPARENT|WS_OWNERDRAW|WS_VISIBLE, 100,0,600,35,          
                   hwnd, 0x1000, NULL, NULL);
      CreateWindow(BUTTON, L"dasd", WS_OWNERDRAW  |WS_TRANSPARENT|WS_VISIBLE, 100,35,600,35,          
                   hwnd, 0x1001, NULL, NULL);
      break;
    }     
    case WM_ERASEBKGND:
    {
      HDC hdc =(HDC)wParam;
      RECT rc =*(RECT*)lParam; 
      
      SetBrushColor(hdc, MapRGB(hdc, 0, 0, 0));
      FillRect(hdc, &rc);       
      return TRUE;
    } 
    case WM_DRAWITEM:
    {
       DRAWITEM_HDR *ds;
       ds = (DRAWITEM_HDR*)lParam;
       switch(ds->ID)
       {
          case 0x1000:
          {
            PicViewer_TBOX_OwnerDraw(ds);
            return TRUE;  
          }              
          case 0x1001:
          {
            PicViewer_TBOX_OwnerDraw(ds);
            return TRUE;  
          }          
                    
       }
       break;
    }    
    case WM_PAINT:
    {
       HDC hdc, hdc_mem;
       PAINTSTRUCT ps;
       RECT rc = {0,0,800,70};
       hdc_mem = CreateMemoryDC(SURF_ARGB4444, 800,70);
       
       hdc = BeginPaint(hwnd, &ps);
       
       SetBrushColor(hdc_mem, MapARGB(hdc_mem,155,105, 105, 105));
       FillRect(hdc_mem, &rc);
       
       BitBlt(hdc, 0,0,800,70,hdc_mem,0,0,SRCCOPY);
       DeleteDC(hdc_mem);
       EndPaint(hwnd, &ps);
       break;
    }      
    default:
      return	DefWindowProc(hwnd, msg, wParam, lParam);    
  }
  return WM_NULL;
}
void GUI_Test_DIALOG(void* param)
{
	HWND	hwnd;
	WNDCLASS	wcex;
	MSG msg;


	wcex.Tag = WNDCLASS_TAG;

	wcex.Style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = test_proc; //������������Ϣ����Ļص�����.
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = NULL;//hInst;
	wcex.hIcon = NULL;//LoadIcon(hInstance, (LPCTSTR)IDI_WIN32_APP_TEST);
	wcex.hCursor = NULL;//LoadCursor(NULL, IDC_ARROW);
   
	//����������
	hwnd = CreateWindowEx(WS_EX_NOFOCUS|WS_EX_FRAMEBUFFER,
                        &wcex,
                        L"GUI___DIALOG",
                        WS_VISIBLE|WS_CLIPCHILDREN,
                        0, 0, GUI_XSIZE, GUI_YSIZE,
                        NULL, NULL, NULL, NULL);
   //��ʾ������
	ShowWindow(hwnd, SW_SHOW);
	//��ʼ������Ϣѭ��(���ڹرղ�����ʱ,GetMessage������FALSE,�˳�����Ϣѭ��)��
	while (GetMessage(&msg, hwnd))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}  
}