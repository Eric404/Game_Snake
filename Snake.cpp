// Snake.cpp : 定义应用程序的入口点。
//

#include "stdafx.h"
#include "Snake.h"
#include<deque>//

//宏定义
#define MAX_LOADSTRING 100
#define SNAKE_SIZE 15           //每节蛇（方块）的大小
#define X_FORM 50               //X方向50个方块
#define Y_FORM 50               //Y方向50个方块
#define X_AREA X_FORM*SNAKE_SIZE//X方向的大小=方块个数*方块的大小
#define Y_AREA Y_FORM*SNAKE_SIZE//Y方向的大小
#define OFFSET 0//这里设置为 0，没有留边框，版本 1.0中设置为 10

//定时器
#define ID_TIMER 1

//画刷
HBRUSH g_hBrushOut = (HBRUSH)CreateSolidBrush(RGB(0x98, 0xFB, 0x98));
HBRUSH g_hBrushIn = (HBRUSH)CreateSolidBrush(RGB(0x9A, 0xCD, 0x32));
HBRUSH g_hBrushFood = (HBRUSH)CreateSolidBrush(RGB(0xff, 0x20, 0x20));

//DC
HDC g_hdc;//全局DC
HDC g_hdcBuf;//缓存DC
HDC g_hdcBg;//画图DC
HBITMAP g_hBmp;//加载画布，给缓存DC用的

//蛇的各节点坐标列表
std::deque<POINT>deqSnake;

//食物
POINT g_ptFood;

//蛇的当前方向 0,1,2,3等别表示上下左右 
INT g_Direct;

//蛇的上一个方向 
INT g_preDirect;//先不在程序中使用，暂不做修改，假如按上键后马上按左键就会gameover，This is a bug.

//初始值为 0
int kase = 0;

//游戏结束变量
INT g_iGameFail;//在Init_Snake() 中初始化

//
INT g_Tmp=1;

//枚举蛇的方向0,1,2,3
typedef enum tagDIRECT
{
	D_UP,
	D_DOWN,
	D_RIGHT,
	D_LEFT,
};


// 全局变量: 
HINSTANCE hInst;							    	// 当前实例
TCHAR szTitle[MAX_LOADSTRING] = L"Snake";		    // 标题栏文本
TCHAR szWindowClass[MAX_LOADSTRING];			    // 主窗口类名
VOID CentWindow(HWND hWnd, int nSizeX, int nSizeY); //窗口调整函数
VOID Load_Game(HWND);//加载资源到游戏
VOID Clear_Game();//删除创建的DC
VOID Paint_Game();
VOID Init_Snake();
VOID Init_Game();//暂时没加，是个空函数
VOID GenerateFood();//初始化食物位置
VOID Move_Snake();//蛇的移动
BOOL CheckSnake(POINT ptHead);//检查是否撞墙

// 此代码模块中包含的函数的前向声明: 
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

	// TODO:  在此放置代码。
	MSG msg;
	HACCEL hAccelTable;

	// 初始化全局字符串
	LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadString(hInstance, IDC_SNAKE, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);

	// 执行应用程序初始化: 
	if (!InitInstance(hInstance, nCmdShow))
	{
		return FALSE;
	}

	hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_SNAKE));

	// 主消息循环: 
	while (GetMessage(&msg, NULL, 0, 0))
	{
		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}
	Clear_Game();//dc什么的清理一下
	return (int)msg.wParam;
}



//
//  函数:  MyRegisterClass()
//
//  目的:  注册窗口类。
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
//   函数:  InitInstance(HINSTANCE, int)
//
//   目的:  保存实例句柄并创建主窗口
//
//   注释: 
//
//        在此函数中，我们在全局变量中保存实例句柄并
//        创建和显示主程序窗口。
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
	HWND hWnd;

	hInst = hInstance; // 将实例句柄存储在全局变量中

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
//  函数:  WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  目的:    处理主窗口的消息。
//
//  WM_COMMAND	- 处理应用程序菜单
//  WM_PAINT	- 绘制主窗口
//  WM_DESTROY	- 发送退出消息并返回
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
			MessageBox(hWnd, L"游戏结束，玩家可以'开始菜单'点击重新开始游戏", L"Failed", MB_OK);
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
		// 分析菜单选择: 
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
		// TODO:  在此添加任意绘图代码...
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

// “关于”框的消息处理程序。
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

VOID CentWindow(HWND hWnd, int nSizeX, int nSizeY)//窗口调整函数
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

VOID Load_Game(HWND hWnd)//加载背景
{
	/*HDC g_hdc;//全局DC
	HDC g_hdcBuf;//缓存DC
	HDC g_hdcBg;//画图DC
	HBITMAP g_hBmp;//加载画布，给缓存DC用的*/
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
	deqSnake.clear();//初始化双链表
	POINT stNode = { 0 };

	for (int i = 0; i < 4; i++)
	{
		stNode.x = 4 - i;
		stNode.y = 1;

		deqSnake.push_back(stNode);
	}
	g_Direct = D_RIGHT;
	g_iGameFail = 0;//初始化
	return;
}


//
VOID Paint_Game()
{
	std::deque<POINT>::iterator it;
	POINT ptHead;
	POINT pt1;
	POINT pt2;

	//画游戏区
	BitBlt(g_hdcBuf, OFFSET, OFFSET, X_AREA, Y_AREA, g_hdcBg, OFFSET, OFFSET, SRCCOPY);

	//画蛇头比较麻烦，不说了。。。
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


	//画蛇身体
	for (it = deqSnake.begin() + 1; it != deqSnake.end(); it++)
	{
		SelectObject(g_hdcBuf, g_hBrushOut);
		Rectangle(g_hdcBuf, SNAKE_SIZE * it->x, SNAKE_SIZE * it->y, SNAKE_SIZE * (1 + it->x), SNAKE_SIZE * (1 + it->y));

		SelectObject(g_hdcBuf, g_hBrushIn);
		Rectangle(g_hdcBuf, SNAKE_SIZE * it->x + 3, SNAKE_SIZE * it->y + 3, SNAKE_SIZE * (1 + it->x) - 3, SNAKE_SIZE * (1 + it->y) - 3);
	}

	//画食物
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
	while (1)//蛇的位置不能与食物重叠
	{
		for (it = deqSnake.begin(); it != deqSnake.end(); it++)
		{
			if (g_ptFood.x == it->x&&g_ptFood.y == it->y)
				break;
		}

		g_ptFood.x = rand() % X_FORM;
		g_ptFood.y = rand() % Y_FORM;

		if (it == deqSnake.end())//for循环到最后
		{
			break;
		}
	}
	return;
}

BOOL CheckSnake(POINT ptHead)
{
	//左墙
	if (ptHead.x < 0) return FALSE;
	//右墙
	if (ptHead.x > X_FORM - 1) return FALSE;

	if (ptHead.y < 0) return FALSE;

	if (ptHead.y > Y_FORM - 1) return FALSE;

	std::deque<POINT>::iterator it;
	//不能撞到自己
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

	/*if (g_preDirect + g_Direct == 3)//如果与前一个方向冲突，方向不变
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
		deqSnake.push_front(stNode);//新的蛇头
		//deqSnake.pop_back();//去掉旧的蛇尾
		//吃食物，增加一节，蛇尾不去掉，蛇头新增
		if (stNode.x == g_ptFood.x && stNode.y == g_ptFood.y)
		{
			//吃到食物
			GenerateFood();
		}
		else
		{
			deqSnake.pop_back(); //去掉旧的蛇尾
		}
	}
	else
	{
		g_iGameFail = 1;
	}
	g_preDirect = g_Direct;
	return;
}