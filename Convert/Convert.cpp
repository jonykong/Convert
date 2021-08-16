// Convert.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "Convert.h"
#include <afxwin.h>
#include <string>
#include <iostream>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define _CRT_SECURE_CPP_OVERLOAD_SECURE_NAMES 1

// 唯一的应用程序对象

CWinApp theApp;

using namespace std;

void recursiveFile(CString strFileType);
void convertGBToUTF8(CString strWritePath, const char* gb2312);


int _tmain(int argc, TCHAR* argv[], TCHAR* envp[])
{
    int nRetCode = 0;

    // 初始化 MFC 并在失败时显示错误
    if (!AfxWinInit(::GetModuleHandle(NULL), NULL, ::GetCommandLine(), 0))
    {
        // TODO: 更改错误代码以符合您的需要
        _tprintf(_T("错误: MFC 初始化失败\n"));
        nRetCode = 1;
    }
    else
    {
        //接受一个参数作为源代码文件的根目录
        TCHAR *lpszDirName = argv[0];
        CString strFileType = lpszDirName;
        
        int pos = strFileType.ReverseFind('\\');
        strFileType = strFileType.Left(pos);
        //递归此目录下的.h文件和.cpp文件，如果发现不是utf8编码则转换为utf8编码
        recursiveFile(strFileType);

    }

    return nRetCode;
}

void recursiveFile(CString strFileType)
{
    CFileFind finder;
    BOOL isFinded = finder.FindFile(strFileType);//查找第一个文件
    while (isFinded)
    {
        isFinded = finder.FindNextFile(); //递归搜索其他的文件
        if (!finder.IsDots()) //如果不是"."目录
        {
            CString strFoundFile = finder.GetFilePath();
            if (finder.IsDirectory()) //如果是目录，则递归地调用
            {
                CString strNextFileType;
                strNextFileType.Format(_T("%s\\*.*"), strFoundFile);
                recursiveFile(strNextFileType);
            }
            else
            {
                //如果是头文件或cpp文件
                if (strFoundFile.Right(4) == _T(".cpp") || strFoundFile.Right(2) == _T(".h") || strFoundFile.Right(4) == _T(".hpp")) {
                    CFile fileReader(strFoundFile, CFile::modeRead);
                    byte head[3];
                    fileReader.Read(head, 3);
                    //判断是否带有BOM文件头
                    if (head[0] == 0xef && head[1] == 0xbb && head[2] == 0xbf)
                    {
                        fileReader.Close();
                        continue;
                    }
                    fileReader.SeekToBegin();

                    int bufLength = 256;
                    char *buf = new char[bufLength];
                    ZeroMemory(buf, bufLength);
                    int nReadLength;
                    std::string strContent;
                    while ((nReadLength = fileReader.Read(buf, bufLength)))
                    {
                        strContent.append(buf, nReadLength);
                        ZeroMemory(buf, nReadLength);
                    }
                    delete buf;
                    fileReader.Close();
                    convertGBToUTF8(strFoundFile, strContent.c_str());

                    TCHAR* fileName = new TCHAR[strFoundFile.GetLength() + 1];
                    //wcscpy_s(*fileName, strFoundFile);

                    // 中文路径存在问题，可以将下面的输出屏蔽，程序将静默运行
                    printf("%S已经转换为UTF-8编码", strFoundFile.GetBuffer(0));
                    cout << endl;

                    if (_tcslen(fileName) > 0)
                    {
                        delete[] fileName;
                    }
                }
            }
        }
    }
    finder.Close();
}

void convertGBToUTF8(CString strWritePath, const char* gb2312)
{
    CFile fp;
    fp.Open(strWritePath, CFile::modeCreate | CFile::modeWrite | CFile::typeBinary, NULL);
    int len = MultiByteToWideChar(CP_ACP, 0, gb2312, -1, NULL, 0);
    wchar_t* wstr = new wchar_t[len + 1];
    memset(wstr, 0, len + 1);
    MultiByteToWideChar(CP_ACP, 0, gb2312, -1, wstr, len);
    len = WideCharToMultiByte(CP_UTF8, 0, wstr, -1, NULL, 0, NULL, NULL);
    char* str = new char[len + 1];
    memset(str, 0, len + 1);
    len = WideCharToMultiByte(CP_UTF8, 0, wstr, -1, str, len, NULL, NULL);
    if (wstr) delete[] wstr;
    const unsigned char aryBOM[] = { 0xEF, 0xBB, 0xBF };
    fp.Write(aryBOM, sizeof(aryBOM));
    fp.Write(str, len - 1);
    delete[] str;
    fp.Close();
}
