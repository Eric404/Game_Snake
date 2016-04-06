// Snake.cpp : ����Ӧ�ó������ڵ㡣
//

#include "stdafx.h"
#include "Snake.h"
#include<deque>//

//�궨��
#define MAX_LOADSTRING 100
#define SNAKE_SIZE 15           //ÿ���ߣ����飩�Ĵ�С
#define X_FORM 50               //X����50������
#define Y_FORM 50               //Y����50������
#define X_AREA X_FORM*SNAKE_SIZE//X����Ĵ�С=�������*����Ĵ�С
#define Y_AREA Y_FORM*SNAKE_SIZE//Y����Ĵ�С
#define OFFSET 0//��������Ϊ 0��û�����߿򣬰汾 1.0������Ϊ 10

//��ʱ��
#define ID_TIMER 1

//��ˢ
HBRUSH g_hBrushOut = (HBRUSH)CreateSolidBrush(RGB(0x98, 0xFB, 0x98));
HBRUSH g_hBrushIn = (HBRUSH)CreateSolidBrush(RGB(0x9A, 0xCD, 0x32));
HBRUSH g_hBrushFood = (HBRUSH)CreateSolidBrush(RGB(0xff, 0x20, 0x20));

//DC
HDC g_hdc;//ȫ��DC
HDC g_hdcBuf;//����DC
HDC g_hdcBg;//��ͼDC
HBITMAP g_hBmp;//���ػ�����������DC�õ�

//�ߵĸ��ڵ������б�
std::deque<POINT>deqSnake;

//ʳ��
POINT g_ptFood;

//�ߵĵ�ǰ���� 0,1,2,3�ȱ��ʾ�������� 
INT g_Direct;

//�ߵ���һ������ 
INT g_preDirect;//�Ȳ��ڳ�����ʹ�ã��ݲ����޸ģ����簴�ϼ������ϰ�����ͻ�gameover��This is a bug.

//��ʼֵΪ 0
int kase = 0;

//��Ϸ��������
INT g_iGameFail;//��Init_Snake() �г�ʼ��

//
INT g_Tmp=1;

//ö���ߵķ���0,1,2,3
typedef enum tagDIRECT
{
	D_UP,
	D_DOWN,
	D_RIGHT,
	D_LEFT,
};


// ȫ�ֱ���: 
HINSTANCE hInst;							    	// ��ǰʵ��
TCHAR szTitle[MAX_LOADSTRING] = L"Snake";		    // �������ı�
TCHAR szWindowClass[MAX_LOADSTRING];			    // ����������
VOID CentWindow(HWND hWnd, int nSizeX, int nSizeY); //���ڵ�������
VOID Load_Game(HWND);//������Դ����Ϸ
VOID Clear_Game();//ɾ��������DC
VOID Paint_Game();
VOID Init_Snake();
VOID Init_Game();//��ʱû�ӣ��Ǹ��պ���
VOID GenerateFood();//��ʼ��ʳ��λ��
VOID Move_Snake();//�ߵ��ƶ�
BOOL CheckSnake(POINT ptHead);//����Ƿ�ײǽ

// �˴���ģ���а����ĺ�����ǰ������: 
ATOM				MyRegisterClass(HINSTANCE hInstance);
BOOL				InitInstance(HINSTANCE, int);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK	About(HWND, UINT, WPARAM, LPARAM);

int APIENTRY _tWinMain(_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPTSTR    lpCmdLine,
	_In_ int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	// TODO:  �ڴ˷��ô��롣
	MSG msg;
	HACCEL hAccelTable;

	// ��ʼ��ȫ���ַ���
	LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadString(hInstance, IDC_SNAKE, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);

	// ִ��Ӧ�ó����ʼ��: 
	if (!InitInstance(hInstance, nCmdShow))
	{
		return FALSE;
	}

	hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_SNAKE));

	// ����Ϣѭ��: 
	while (GetMessage(&msg, NULL, 0, 0))
	{
		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}
	Clear_Game();//dcʲô������һ��
	return (int)msg.wParam;
}



//
//  ����:  MyRegisterClass()
//
//  Ŀ��:  ע�ᴰ���ࡣ
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_SNAKE));
	wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = MAKEINTRESOURCE(IDC_SNAKE);
	wcex.lpszClassName = szWindowClass;
	wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	return RegisterClassEx(&wcex);
}

//
//   ����:  InitInstance(HINSTANCE, int)
//
//   Ŀ��:  ����ʵ�����������������
//
//   ע��: 
//
//        �ڴ˺����У�������ȫ�ֱ����б���ʵ�������
//        ��������ʾ�����򴰿ڡ�
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
	HWND hWnd;

	hInst = hInstance; // ��ʵ������洢��ȫ�ֱ�����

	hWnd = CreateWindow(szWindowClass, szTitle, WS_OVERLAPPED|WS_MINIMIZEBOX|WS_SYSMENU,
		CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, NULL, NULL, hInstance, NULL);

	if (!hWnd)
	{
		return FALSE;
	}
	Load_Game(hWnd);
	Init_Game();
	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);

	return TRUE;
}



//
//  ����:  WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  Ŀ��:    ���������ڵ���Ϣ��
//
//  WM_COMMAND	- ����Ӧ�ó���˵�
//  WM_PAINT	- ����������
//  WM_DESTROY	- �����˳���Ϣ������
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{

	int wmId, wmEvent;
	PAINTSTRUCT ps;
	HDC hdc;

	switch (message)
	{
	case WM_CREATE:
		CentWindow(hWnd, X_AREA , Y_AREA );
		//std::deque<POINT> ivec()
		Init_Snake();
		GenerateFood();
		Move_Snake();
		Paint_Game();
		//SetTimer(hWnd, ID_TIMER, 300, 0);
		break;
	case WM_TIMER:
		g_Tmp ^= 1;
		Move_Snake();
		if (g_iGameFail)
		{
			KillTimer(hWnd,ID_TIMER);
			MessageBox(hWnd, L"��Ϸ��������ҿ���'��ʼ�˵�'������¿�ʼ��Ϸ", L"Failed", MB_OK);
			break;
		}
		Paint_Game();
		//InvalidateRect(hWnd, NULL, TRUE);
		break;
	case WM_KEYDOWN:
		switch (wParam)
		{
		case VK_UP:
			if (g_Direct != D_DOWN) g_Direct = D_UP;
			break;
		case VK_DOWN:
			if (g_Direct != D_UP) g_Direct = D_DOWN;
			break;
		case VK_LEFT:
			if (g_Direct != D_RIGHT) g_Direct = D_LEFT;
			break;
		case VK_RIGHT:
			if (g_Direct != D_LEFT) g_Direct = D_RIGHT;
			break;
		case VK_SPACE:
			if (!g_iGameFail)
			SendMessage(hWnd, WM_COMMAND, (kase^=1)?IDB_START:IDB_PAUSE, lParam);
			break;
		default:
			break;
		}
		break;
	case WM_COMMAND:
		wmId = LOWORD(wParam);
		wmEvent = HIWORD(wParam);
		// �����˵�ѡ��: 
		switch (wmId)
		{
		case IDM_ABOUT:
			DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
			break;
		case IDM_EXIT:
			DestroyWindow(hWnd);
			break;
		case IDB_START:
			SetTimer(hWnd, ID_TIMER, 300, 0);
			break;
		case IDB_PAUSE:
			KillTimer(hWnd, ID_TIMER);
			break;
		case IDB_RESTART:
			Init_Snake();
			SetTimer(hWnd, ID_TIMER, 300, 0);
			break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
		break;
	case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);
		// TODO:  �ڴ���������ͼ����...
		/*Move_Snake();*/
		Paint_Game();
		EndPaint(hWnd, &ps);
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

// �����ڡ������Ϣ�������
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG:
		return (INT_PTR)TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
		{
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		break;
	}
	return (INT_PTR)FALSE;
}

VOID CentWindow(HWND hWnd, int nSizeX, int nSizeY)//���ڵ�������
{
	int nWinX, nWinY, nClientX, nClientY;
	RECT rect;
	int nScreenX = GetSystemMetrics(SM_CXSCREEN);
	int nScreenY = GetSystemMetrics(SM_CYSCREEN);

	GetWindowRect(hWnd, &rect);
	nWinX = rect.right - rect.left;
	nWinY = rect.bottom - rect.top;

	GetClientRect(hWnd, &rect);
	nClientX = rect.right - rect.left;
	nClientY = rect.bottom - rect.top;

	nSizeX += (nWinX - nClientX);
	nSizeY += (nWinY - nClientY);

	MoveWindow(hWnd, (nScreenX - nSizeX) / 2, (nScreenY - nSizeY) / 2,
		nSizeX, nSizeY, TRUE);
	return;
}

VOID Load_Game(HWND hWnd)//���ر���
{
	/*HDC g_hdc;//ȫ��DC
	HDC g_hdcBuf;//����DC
	HDC g_hdcBg;//��ͼDC
	HBITMAP g_hBmp;//���ػ�����������DC�õ�*/
	g_hdc = GetDC(hWnd);
	g_hdcBuf = CreateCompatibleDC(g_hdc);
	g_hBmp = CreateCompatibleBitmap(g_hdc, X_AREA , Y_AREA );
	SelectObject(g_hdcBuf, g_hBmp);

	HBITMAP hBmp = LoadBitmap(hInst, MAKEINTRESOURCE(IDB_BITMAP1));
	g_hdcBg = CreateCompatibleDC(g_hdc);
	SelectObject(g_hdcBg, hBmp);
	DeleteObject(hBmp);
	return;
}

VOID Clear_Game()
{
	DeleteObject(g_hBmp);
	DeleteObject(g_hdcBuf);
	DeleteObject(g_hdcBg);
	return;
}

VOID Init_Snake()
{
	deqSnake.clear();//��ʼ��˫����
	POINT stNode = { 0 };

	for (int i = 0; i < 4; i++)
	{
		stNode.x = 4 - i;
		stNode.y = 1;

		deqSnake.push_back(stNode);
	}
	g_Direct = D_RIGHT;
	g_iGameFail = 0;//��ʼ��
	return;
}


//
VOID Paint_Game()
{
	std::deque<POINT>::iterator it;
	POINT ptHead;
	POINT pt1;
	POINT pt2;

	//����Ϸ��
	BitBlt(g_hdcBuf, OFFSET, OFFSET, X_AREA, Y_AREA, g_hdcBg, OFFSET, OFFSET, SRCCOPY);

	//����ͷ�Ƚ��鷳����˵�ˡ�����
	ptHead = deqSnake.front();

	switch (g_Direct)
	{
	case D_RIGHT:
		pt1.x = ptHead.x + 1;
		pt1.y = ptHead.y;
		pt2.x = ptHead.x + 1;
		pt2.y = ptHead.y + 1;
		break;
	case D_LEFT:
		pt1.x = ptHead.x;
		pt1.y = ptHead.y + 1;
		pt2.x = ptHead.x;
		pt2.y = ptHead.y;
		break;
	case D_DOWN:
		pt1.x = ptHead.x + 1;
		pt1.y = ptHead.y + 1;
		pt2.x = ptHead.x;
		pt2.y = ptHead.y + 1;
		break;
	case D_UP:
		pt1.x = ptHead.x;
		pt1.y = ptHead.y;
		pt2.x = ptHead.x + 1;
		pt2.y = ptHead.y;
		break;
	}



	//SelectObject(g_hdcBuf, g_hBrushOut);
	//Rectangle(g_hdcBuf, SNAKE_SIZE * ptHead.x, SNAKE_SIZE * ptHead.y,		SNAKE_SIZE * (ptHead.x + 1) - SNAKE_SIZE / 2, SNAKE_SIZE * (ptHead.y + 1)		);
	if (g_Tmp)
	{
		SelectObject(g_hdcBuf, g_hBrushIn);
		Pie(g_hdcBuf, SNAKE_SIZE * ptHead.x, SNAKE_SIZE * ptHead.y,
			SNAKE_SIZE * (ptHead.x + 1), SNAKE_SIZE * (ptHead.y + 1),
			SNAKE_SIZE * pt1.x, SNAKE_SIZE * pt1.y,
			SNAKE_SIZE * pt2.x, SNAKE_SIZE * pt2.y);
	}
	else
	{
		SelectObject(g_hdcBuf, g_hBrushIn);
		Ellipse(g_hdcBuf, SNAKE_SIZE * ptHead.x, SNAKE_SIZE * ptHead.y,
			SNAKE_SIZE * (ptHead.x + 1), SNAKE_SIZE * (ptHead.y + 1));
	}


	/*
	SelectObject(g_hdcBuf, g_hBrushIn);
	Pie(g_hdcBuf,
	SNAKE_SIZE * ptHead.x + 3, SNAKE_SIZE * ptHead.y + 3,
	SNAKE_SIZE * (1 + ptHead.x) - 3, SNAKE_SIZE * (1 + ptHead.y) - 3,
	SNAKE_SIZE * pt1.x, SNAKE_SIZE * pt1.y,
	SNAKE_SIZE * pt2.x, SNAKE_SIZE * pt2.y);
	*/


	//��������
	for (it = deqSnake.begin() + 1; it != deqSnake.end(); it++)
	{
		SelectObject(g_hdcBuf, g_hBrushOut);
		Rectangle(g_hdcBuf, SNAKE_SIZE * it->x, SNAKE_SIZE * it->y, SNAKE_SIZE * (1 + it->x), SNAKE_SIZE * (1 + it->y));

		SelectObject(g_hdcBuf, g_hBrushIn);
		Rectangle(g_hdcBuf, SNAKE_SIZE * it->x + 3, SNAKE_SIZE * it->y + 3, SNAKE_SIZE * (1 + it->x) - 3, SNAKE_SIZE * (1 + it->y) - 3);
	}

	//��ʳ��
	SelectObject(g_hdcBuf, g_hBrushFood);
	Ellipse(g_hdcBuf, SNAKE_SIZE * g_ptFood.x, SNAKE_SIZE * g_ptFood.y, SNAKE_SIZE * (1 + g_ptFood.x), SNAKE_SIZE * (1 + g_ptFood.y));

	BitBlt(g_hdc, OFFSET, OFFSET, X_AREA, Y_AREA, g_hdcBuf, 0, 0, SRCCOPY);
	return;
}


VOID Init_Game()
{
	return;
}

VOID GenerateFood()
{
	std::deque<POINT>::iterator it;
	srand(GetTickCount());
	while (1)//�ߵ�λ�ò�����ʳ���ص�
	{
		for (it = deqSnake.begin(); it != deqSnake.end(); it++)
		{
			if (g_ptFood.x == it->x&&g_ptFood.y == it->y)
				break;
		}

		g_ptFood.x = rand() % X_FORM;
		g_ptFood.y = rand() % Y_FORM;

		if (it == deqSnake.end())//forѭ�������
		{
			break;
		}
	}
	return;
}

BOOL CheckSnake(POINT ptHead)
{
	//��ǽ
	if (ptHead.x < 0) return FALSE;
	//��ǽ
	if (ptHead.x > X_FORM - 1) return FALSE;

	if (ptHead.y < 0) return FALSE;

	if (ptHead.y > Y_FORM - 1) return FALSE;

	std::deque<POINT>::iterator it;
	//����ײ���Լ�
	for (it = deqSnake.begin(); it != deqSnake.end(); it++)
	{
		if (it->x == ptHead.x && it->y == ptHead.y)
		{
			return FALSE;
		}
	}

	return TRUE;
}

VOID Move_Snake()
{
	POINT stNode = { 0 };

	stNode.x = deqSnake.front().x;
	stNode.y = deqSnake.front().y;

	/*if (g_preDirect + g_Direct == 3)//�����ǰһ�������ͻ�����򲻱�
	{
		g_Direct = g_preDirect;
	}
    */

	switch (g_Direct)
	{
	case D_UP:
		stNode.y--;
		break;
	case D_DOWN:
		stNode.y++;
		break;
	case D_LEFT:
		stNode.x--;
		break;
	case D_RIGHT:
		stNode.x++;
		break;
	default:
		break;
	}

	if (CheckSnake(stNode))
	{
		deqSnake.push_front(stNode);//�µ���ͷ
		//deqSnake.pop_back();//ȥ���ɵ���β
		//��ʳ�����һ�ڣ���β��ȥ������ͷ����
		if (stNode.x == g_ptFood.x && stNode.y == g_ptFood.y)
		{
			//�Ե�ʳ��
			GenerateFood();
		}
		else
		{
			deqSnake.pop_back(); //ȥ���ɵ���β
		}
	}
	else
	{
		g_iGameFail = 1;
	}
	g_preDirect = g_Direct;
	return;
}