/* 
文件：SelectFolderDlg.h 
说明：提供一个选择文件夹的对话框 
*/  
  
#include <string>
#include <minwinbase.h>

#ifndef SELECT_FOLDER_DLG_H  
#define SELECT_FOLDER_DLG_H  
  
  
#ifndef BIF_NEWDIALOGSTYLE  
#define  BIF_NEWDIALOGSTYLE  0x0040  
#endif  
 
LPITEMIDLIST WINAPI SHBrowseForFolder(LPBROWSEINFO lpbi);
typedef   struct   _browseinfo {
    HWND   hwndOwner;   //   父窗口的句柄  
    LPCITEMIDLIST   pidlRoot;   //   一个ITEMIDLIST结构变量，指定根目录  
    LPSTR   pszDisplayName;   //    
    LPCSTR   lpszTitle;   //   位于对话框顶端的一行文字  
    UINT   ulFlags;   //   标志变量，按位有效  
    BFFCALLBACK   lpfn;   //   回调函数  
    LPARAM   lParam;   //   传给回调函数的参数，一个32位值  
    int   iImage;   //   被选择的文件夹的图片序号，与shell32.dll中的图标号同  
}   BROWSEINFO, * PBROWSEINFO, * LPBROWSEINFO;


class CSelectFolderDlg  
{  
public:  
    typedef struct _browseinfo {
        HWND hwndOwner;            // 父窗口句柄
        LPCITEMIDLIST pidlRoot;    // 要显示的文件目录对话框的根(Root)
        LPTSTR pszDisplayName;     // 保存被选取的文件夹路径的缓冲区
        LPCTSTR lpszTitle;         // 显示位于对话框左上部的标题
        UINT ulFlags;              // 指定对话框的外观和功能的标志
        BFFCALLBACK lpfn;          // 处理事件的回调函数
        LPARAM lParam;             // 应用程序传给回调函数的参数
        int iImage;                // 文件夹对话框的图片索引
    } BROWSEINFO, * PBROWSEINFO, * LPBROWSEINFO;

    //创建一个选择文件夹的对话框，返回所选路径  
    string Show()
    {  
        TCHAR           szFolderPath[MAX_PATH] = {0};  
        string         strFolderPath = TEXT("");
          
        BROWSEINFO      sInfo;  
        ::ZeroMemory(&sInfo, sizeof(BROWSEINFO));  
        sInfo.pidlRoot   = 0;  
        sInfo.lpszTitle   = _T("请选择一个文件夹：");  
        sInfo.ulFlags   = BIF_DONTGOBELOWDOMAIN | BIF_RETURNONLYFSDIRS | BIF_NEWDIALOGSTYLE | BIF_EDITBOX;  
        sInfo.lpfn     = NULL;  
  
        // 显示文件夹选择对话框  
        LPITEMIDLIST lpidlBrowse = ::SHBrowseForFolder(&sInfo);   
        if (lpidlBrowse != NULL)  
        {  
            // 取得文件夹名  
            if (::SHGetPathFromIDList(lpidlBrowse,szFolderPath))    
            {  
                strFolderPath = szFolderPath;  
            }  
        }  
        if(lpidlBrowse != NULL)  
        {  
            ::CoTaskMemFree(lpidlBrowse);  
        }  
  
        return strFolderPath;  
  
    }  
  
};  
  
#endif