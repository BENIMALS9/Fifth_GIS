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
#include <windowsx.h>


//文件对话框


#include <windows.h>  
#include <commdlg.h>
#define MAX_LOADSTRING 100

using namespace std;

//绘图函数
void MapOnDraw(HDC hDC, Map* mp, double xrate, double yrate)
{
    mp->Draw(hDC, xrate, yrate);
}

void FeatureOnDraw(HDC hDC, Geometry* go)
{
    go->Draw(hDC);
}

void LayerOnDraw(HDC hDC, Layer* ly, double xrate, double yrate)
{
    ly->Draw(hDC, xrate, yrate);
}

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

std::string TCHAR2STRING(TCHAR* str)
{
    std::string strstr;
    try
    {
        int iLen = WideCharToMultiByte(CP_ACP, 0, str, -1, NULL, 0, NULL, NULL);

        char* chRtn = new char[iLen * sizeof(char)];

        WideCharToMultiByte(CP_ACP, 0, str, -1, chRtn, iLen, NULL, NULL);

        strstr = chRtn;
    }
    catch (std::exception e)
    {
    }

    return strstr;
}



// 全局变量:
HINSTANCE hInst;                                // 当前实例
WCHAR szTitle[MAX_LOADSTRING];                  // 标题栏文本
WCHAR szWindowClass[MAX_LOADSTRING];            // 主窗口类名


TCHAR szFile[MAX_PATH] = _T("NULL");            // 保存获取文件名称的缓冲区。

//----------------------------------------------------------------------------------
POINT OFFSET;
POINT OFFSET_TEMP;
POINT MOUSEMOVE;                                // 鼠标移动时坐标
POINT MOUSEDOWN;                                // 鼠标按下时坐标
POINT MOUSEUP;                                  // 鼠标弹起时坐标
BOOL  MOUSEEVENT = FALSE;                       // 鼠标按键指标
INT   MOUSEWHEEL = 1;                           // 鼠标滚轮偏移

int Type;
std::vector<Geometry*> go;                 // 地理要素集合
std::vector<Layer*> ly;                    // 图层集合
BOOLEAN ReadFILE = FALSE;                  // 是否读入数据判定
std::vector<Map*> mp;                      // 地图集合
int go_count = 0;                          // 要素数量
int ly_count = 0;                          // 图层数量
int mp_count = 0;                          // 地图数量


//----------------------------------------------------------------------------------

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

                    string szFile1;
                    szFile1 = TCHAR2STRING(szFile);
                    const char* filepath = szFile1.c_str();

					// 解析Json
                    string str = JsonFiletoString(filepath);
                    Json::Value GeoFeature = CreateFromGeojson(str, &Type);

					// 根据读入GeoJson类型初始化
                    if (Type == 0)
                    {
						Geometry* geo;
                        geo = new Point;
                        geo->InitialFromGeoJson(GeoFeature);
						go.push_back(geo);
						go_count++;
                    }
                    else if (Type == 1)
                    {
						Geometry* geo;
                        geo = new PolyLine;
                        geo->InitialFromGeoJson(GeoFeature);
						go.push_back(geo);
						go_count++;
                    }
                    else if (Type == 2)
                    {
						Geometry* geo;
                        geo = new PolyGon;
                        geo->InitialFromGeoJson(GeoFeature);
						go.push_back(geo);
						go_count++;
                    }
                    else if (Type == 3)
                    {
						Layer* lay;
                        lay = new PointLayer;
                        lay->InitialFromGeoJson(GeoFeature);
						ly.push_back(lay);
						ly_count++;
                    }
                    else if (Type == 4)
                    {
						Layer* lay;
                        lay = new PolyLineLayer;
                        lay->InitialFromGeoJson(GeoFeature);
						ly.push_back(lay);
						ly_count++;
                    }
                    else if (Type == 5)
                    {
						Layer* lay;
                        lay = new PolyGonLayer;
                        lay->InitialFromGeoJson(GeoFeature);
						ly.push_back(lay);
						ly_count++;
                    }
                    else if (Type == 6)
                    {
						Map* mmap;
                        mmap = new FeatureCollection;
                        mmap->InitialFromGeoJson(GeoFeature);
						mp.push_back(mmap);
						mp_count++;
                    }
                    ReadFILE = TRUE;

                }
                
                InvalidateRect(hWnd, NULL, TRUE);
                
                break;
            case ID_GEOJSON_REMOVE:
                // 删除读入文件
				if (go.size() > 0)
				{
					for (int i = 0; i < go.size(); ++i)
					{
						go.pop_back();
						go_count--;
					}
				}
				if (ly.size() > 0)
				{
					for (int i = 0; i < ly.size(); ++i)
					{
						ly.pop_back();
						ly_count--;
					}
				}
				if (mp.size() > 0)
				{
					for (int i = 0; i < mp.size(); ++i)
					{
						mp.pop_back();
						mp_count--;
					}
				}
                szFile[0] = '\0';

                InvalidateRect(hWnd, NULL, TRUE);

				ReadFILE = FALSE;

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
           
            string szFile1;
            szFile1 = TCHAR2STRING(szFile);


            //SetViewportOrgEx(hdc,0, 0, NULL);
            //SetWindowOrgEx(hdc, rtClient.left - OFFSET.x, rtClient.top - OFFSET.y, NULL);
            
            //----------------------------------------------------------------------------------
            RECT rtClient;                       
            GetClientRect(hWnd, &rtClient);
            SetMapMode(hdc, MM_ISOTROPIC);

            // + mp->GetMinX()*3  + mp->GetMinY()*3

            SetWindowOrgEx(hdc,0 - OFFSET.x * 0.01,0 - OFFSET.y * 0.01, NULL);
            

            
			// 读入数据才进行绘图
			if (ReadFILE != FALSE)
			{
				if (Type == 6)
				{
					SetViewportExtEx(hdc, (mp[mp_count - 1]->GetMaxX() - mp[mp_count - 1]->GetMinX()) * rtClient.right,
						(mp[mp_count - 1]->GetMaxY() - mp[mp_count - 1]->GetMinY()) * rtClient.bottom, NULL);
					SetWindowExtEx(hdc, (mp[mp_count - 1]->GetMaxX() - mp[mp_count - 1]->GetMinX()) * rtClient.right + MOUSEWHEEL,
						(mp[mp_count - 1]->GetMaxY() - mp[mp_count - 1]->GetMinY()) * rtClient.bottom + MOUSEWHEEL, NULL);
				}
				else if (Type == 0 || Type == 1 || Type == 2)
				{
					SetViewportExtEx(hdc, (go[go_count - 1]->GetMaxX() - go[go_count - 1]->GetMinX()) * rtClient.right,
						(go[go_count - 1]->GetMaxY() - go[go_count - 1]->GetMinY()) * rtClient.bottom, NULL);
					SetWindowExtEx(hdc, (go[go_count - 1]->GetMaxX() - go[go_count - 1]->GetMinX()) * rtClient.right + MOUSEWHEEL,
						(go[go_count - 1]->GetMaxY() - go[go_count - 1]->GetMinY()) * rtClient.bottom + MOUSEWHEEL, NULL);
				}
				else
				{
					SetViewportExtEx(hdc, (ly[ly_count - 1]->GetMaxX() - ly[ly_count - 1]->GetMinX()) * rtClient.right,
						(ly[ly_count - 1]->GetMaxY() - ly[ly_count - 1]->GetMinY()) * rtClient.bottom, NULL);
					SetWindowExtEx(hdc, (ly[ly_count - 1]->GetMaxX() - ly[ly_count - 1]->GetMinX()) * rtClient.right + MOUSEWHEEL,
						(ly[ly_count - 1]->GetMaxY() - ly[ly_count - 1]->GetMinY()) * rtClient.bottom + MOUSEWHEEL, NULL);
				}

				static int init_x = rtClient.right;
				static int init_y = rtClient.bottom;

				for (int i = 0; i < go.size(); ++i)
				{
					FeatureOnDraw(hdc, go[i]);
				}

				for (int i = 0; i < ly.size(); ++i)
				{
					LayerOnDraw(hdc, ly[i], init_x, init_y);
				}

				for (int i = 0; i < mp.size(); ++i)
				{
					MapOnDraw(hdc, mp[i], init_x, init_y);
				}
			}
			


            EndPaint(hWnd, &ps);
        }
        break;
    //----------------------------------------------------------------------------------
    case WM_LBUTTONDOWN:
        {
            MOUSEDOWN.x = GET_X_LPARAM(lParam);
            MOUSEDOWN.y = GET_Y_LPARAM(lParam);
            MOUSEEVENT  = TRUE;                 // 标记鼠标按下

            
        }
        break;
    case WM_LBUTTONUP:
        {
            MOUSEDOWN.x  = 0;
            MOUSEDOWN.y  = 0;
            MOUSEMOVE.x  = 0;
            MOUSEMOVE.y  = 0;
            MOUSEEVENT = FALSE;                 // 标记鼠标松开

            InvalidateRect(hWnd, NULL, TRUE);

        }
        break;
    case WM_MOUSEMOVE:
        {
            MOUSEMOVE.x = GET_X_LPARAM(lParam);
            MOUSEMOVE.y = GET_Y_LPARAM(lParam);

            OFFSET.x = OFFSET_TEMP.x;
            OFFSET.y = OFFSET_TEMP.y;

            if (MOUSEEVENT)                              // 判断是否按下鼠标
            {
                


                OFFSET.x += MOUSEMOVE.x - MOUSEDOWN.x;
                OFFSET.y += MOUSEMOVE.y - MOUSEDOWN.y;

                

                InvalidateRect(hWnd, NULL, TRUE);
            }

            OFFSET_TEMP.x = OFFSET.x;
            OFFSET_TEMP.y = OFFSET.y;
  
        }
        break;
    case WM_MOUSEWHEEL:
    {
        
        
        if (GET_WHEEL_DELTA_WPARAM(wParam) > 0)
        {
            MOUSEWHEEL += 100;
        }
        else if (GET_WHEEL_DELTA_WPARAM(wParam) < 0)
        {
            MOUSEWHEEL -= 100;
        }

        InvalidateRect(hWnd, NULL, TRUE);
    }
    break;

    //----------------------------------------------------------------------------------

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
