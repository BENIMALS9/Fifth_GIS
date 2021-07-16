// FIfth_Gis.cpp : 定义应用程序的入口点。
//

#include "framework.h"
#include "FIfth_Gis.h"

#include<iostream>

#include <string>

#include "framework.h"
#include "Geometry.h"    // 设计的地理要素类头文件
#include <json/json.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <cassert>
#include <errno.h>
#include <string.h>
#include <vector>
#include <cmath>

//文件对话框


#include <windows.h>  
#include <commdlg.h>
#define MAX_LOADSTRING 100

using namespace std;


// 读取Json文件得到的Json字符串转Jsoncpp中的Json格式以进行解析
string JsonFiletoString(const char* FilePath)
{
	ifstream in(FilePath);
	if (in.bad())
	{
		printf("open file '%d' failed!", FilePath);
		return 0;
	}
	stringstream ss;

	ss << in.rdbuf();
	string str(ss.str());

	in.close();

	return str;
}

// 判断GeoJson中的地理要素类型，并返回Json解析结果数据
Json::Value CreateFromGeojson(string str, int* type_no)
{
	// Json解析过程
	bool res;
	JSONCPP_STRING errs;
	Json::Value root, type, properties, geometry, coordinates, xy, gtype;
	Json::CharReaderBuilder readerBuilder;

	std::unique_ptr<Json::CharReader> const jsonReader(readerBuilder.newCharReader());
	res = jsonReader->parse(str.c_str(), str.c_str() + str.length(), &root, &errs);
	if (!res || !errs.empty()) {
		std::cout << "parseJson err. " << errs << std::endl;
	}

	// 地理要素类型判断
	type = root["type"].asString();
	if (type != "Feature")
	{
		*type_no = 6;
		return root;
	}
	geometry = root["geometry"];
	gtype = geometry["type"].asString();
	if (gtype == "Point")
		* type_no = 0;
	else if (gtype == "LineString")
		* type_no = 1;
	else if (gtype == "Polygon")
		* type_no = 2;
	else if (gtype == "MultiPoint")
		* type_no = 3;
	else if (gtype == "MultiLineString")
		* type_no = 4;
	else if (gtype == "MultiPolygon")
		* type_no = 5;
	return root;
}


// 全局变量:
HINSTANCE hInst;                                // 当前实例
WCHAR szTitle[MAX_LOADSTRING];                  // 标题栏文本
WCHAR szWindowClass[MAX_LOADSTRING];            // 主窗口类名

// 此代码模块中包含的函数的前向声明:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);
    
    
    // TODO: 在此处放置代码。

    // 初始化全局字符串
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_FIFTHGIS, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // 执行应用程序初始化:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_FIFTHGIS));

    MSG msg;

    // 主消息循环:
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return (int) msg.wParam;
}



//
//  函数: MyRegisterClass()
//
//  目标: 注册窗口类。
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_FIFTHGIS));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_FIFTHGIS);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

//
//   函数: InitInstance(HINSTANCE, int)
//
//   目标: 保存实例句柄并创建主窗口
//
//   注释:
//
//        在此函数中，我们在全局变量中保存实例句柄并
//        创建和显示主程序窗口。
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance; // 将实例句柄存储在全局变量中

   HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);

   if (!hWnd)
   {
      return FALSE;
   }

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}

//
//  函数: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  目标: 处理主窗口的消息。
//
//  WM_COMMAND  - 处理应用程序菜单
//  WM_PAINT    - 绘制主窗口
//  WM_DESTROY  - 发送退出消息并返回
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    
    OPENFILENAME ofn;      // 公共对话框结构。     
    TCHAR szFile[MAX_PATH]; // 保存获取文件名称的缓冲区。
    switch (message)
    {
    case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            // 分析菜单选择:
            switch (wmId)
            {
            case IDM_ABOUT:
                DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
                break;
            case ID_GEOJSON_INSERT:
                //读入文件;
                
                               
                // 初始化选择文件对话框。     
                ZeroMemory(&ofn, sizeof(OPENFILENAME));
                ofn.lStructSize = sizeof(OPENFILENAME);
                ofn.hwndOwner = NULL;
                ofn.lpstrFile = szFile;
                ofn.lpstrFile[0] = '\0';
                ofn.nMaxFile = sizeof(szFile);
                ofn.lpstrFilter = L"All(*.*)\0*.*\0Text(*.txt)\0*.txt\0JPG(*.jpg)\0*.jpg";//(LPCWSTR)"All(*.*)\0*.*\0Text(*.txt)\0*.TXT\0\0";
                ofn.nFilterIndex = 1;
                ofn.lpstrFileTitle = NULL;
                ofn.nMaxFileTitle = 0;
                ofn.lpstrInitialDir = NULL;
                ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
                //ofn.lpTemplateName =  MAKEINTRESOURCE(ID_TEMP_DIALOG);    
                // 显示打开选择文件对话框。
                if (GetOpenFileName(&ofn))
                {
                    //显示选择的文件。 
                    std::wcout << szFile << std::endl;
                    //OutputDebugString(szFile);    //这一句是显示路径吗？为什么不显示？
                    //OutputDebugString((LPCWSTR)"\r\n");
                }
                
                
                
                break;
            case ID_GEOJSON_REMOVE:
                //删除读入文件
                break;

            case IDM_EXIT:
                DestroyWindow(hWnd);
                break;
            default:
                return DefWindowProc(hWnd, message, wParam, lParam);
            }
        }
        break;
    case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);
			const char* filepath = "gd.json";
			int Type;
			int x = 0;
			int y = 0;
			string id = "ssss";

			string str = JsonFiletoString(filepath);
			Json::Value GeoFeature = CreateFromGeojson(str, &Type);

			if (Type == 0)
			{
				Point po;
				po.InitialFromJSON(GeoFeature);
				po.Draw(hdc);
			}
			else if (Type == 1)
			{
				PolyLine pl;
				pl.InitialFromGeoJson(GeoFeature);
				pl.Draw(hdc);
			}
			else if (Type == 2)
			{
				PolyGon pg;
				pg.InitialFromGeoJson(GeoFeature);
				pg.Draw(hdc);
			}
			else if (Type == 3)
			{
				PointLayer pl;
				pl.InitialFromGeoJson(GeoFeature);
				pl.Draw(hdc);
			}
			else if (Type == 4)
			{
				PolyLineLayer pll;
				pll.InitialFromGeoJson(GeoFeature);
				pll.Draw(hdc);
			}
			else if (Type == 5)
			{
				PolyGonLayer pgl;
				pgl.InitialFromGeoJson(GeoFeature);
				pgl.Draw(hdc);
			}
			else if (Type == 6)
			{
				FeatureCollection map;
				map.InitialFromGeoJson(GeoFeature);
				map.Draw(hdc);
			}
            EndPaint(hWnd, &ps);
        }
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
