// RemoteCtrl.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include "pch.h"
#include "framework.h"
#include "RemoteCtrl.h"
#include "ServerSocket.h"
#include <direct.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// 唯一的应用程序对象

CWinApp theApp;

using namespace std;
void Dump(BYTE * pData, size_t nSize) {
    std::string strOut;
    for (size_t i = 0; i < nSize; i++) {
        char buf[8] = "";
        sprintf_s(buf, sizeof(buf), "%02X ", pData[i] & 0xff);
        strOut += buf;
    }
	strOut += "\n";
    OutputDebugStringA(strOut.c_str());
  
}

std::string MakeDriveInfo() {//1=>A 2=>B 3=>C
	std::string res;
    for(int i=1;i<=26; i++) {
        if(_chdrive(i) == 0) {
            if(res.size()>0)
                res += ",";
			res += 'A' + i - 1;
        }
	}
    CPacket pack(1, (BYTE*)res.c_str(), res.size());//打包用的
    Dump((BYTE*)pack.Data(), pack.size());
    /*CServerSocket::GetInstance()->Send(pack);*/
    return "";
}


int main()
{
    int nRetCode = 0;

    HMODULE hModule = ::GetModuleHandle(nullptr);

    if (hModule != nullptr)
    {
        // 初始化 MFC 并在失败时显示错误
        if (!AfxWinInit(hModule, nullptr, ::GetCommandLine(), 0))
        {
            // TODO: 在此处为应用程序的行为编写代码。
            wprintf(L"错误: MFC 初始化失败\n");
            nRetCode = 1;
        }
        else
        {
            //// TODO: socket、bind、listent、accept、recv、send、close等操作
            ////套接字初始化
            //CServerSocket* pserver = CServerSocket::GetInstance();
            //int count = 0;
            //if (pserver) {
            //    if (pserver->InitSocket() == false) {
            //        MessageBox(NULL, _T("网络初始化异常，未能成功初始化，请检查网络状态！"), _T("网络初始化失败！"), MB_OK | MB_ICONERROR);
            //        exit(0);
            //    }
            //    if (pserver->AcceptClient() == false) {
            //        if (count >= 3) {
            //            MessageBox(NULL, _T("多从无法正常接入用户，结束程序！"), _T("接入用户失败！"), MB_OK | MB_ICONERROR);
            //            exit(0);
            //        }
            //        MessageBox(NULL, _T("无法正常接入用户，自动重试！"), _T("接入用户失败！"), MB_OK | MB_ICONERROR);
            //        count++;
            //    }
            //    int ret = pserver->DealCommand();
            //}
            MakeDriveInfo();



        
        }
    }
    else
    {
        // TODO: 更改错误代码以符合需要
        wprintf(L"错误: GetModuleHandle 失败\n");
        nRetCode = 1;
    }

    return nRetCode;
}
