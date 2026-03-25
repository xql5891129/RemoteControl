#pragma once
#include "pch.h"
#include "framework.h"

#pragma 
class CPacket
{
public:
	CPacket() : sHead(0), nLength(0), sCmd(0), sSum(0) {}
	CPacket(WORD cmd, const BYTE* pData, size_t nSzie) {
		sHead = 0xFEFF;
		nLength = nSzie + 4;
		sCmd = cmd;
		strData.resize(nSzie);
		memcpy((void*)strData.c_str(), pData, nSzie);
		sSum = 0;
		for(size_t i=0; i<strData.size(); i++) {
			sSum += BYTE(strData[i]) & 0xFF;
		}

	}
	CPacket(const CPacket& packet) {
		sHead = packet.sHead;
		nLength = packet.nLength;
		sCmd = packet.sCmd;
		strData = packet.strData;
		sSum = packet.sSum;
	}
	CPacket(const BYTE* pData, size_t& nSize) {
		size_t i = 0;
		for (; i < nSize; i++) {
			if (*(WORD*)(pData + i) == 0xFFFE) {
				i += 2;
				break;
			}
		}
		if (i + 4 + 2 + 2 > nSize) {//包数据可能不全，或者包头未能全部接收到
			nSize = 0;
			return;
		}
		nLength = *(WORD*)(pData + i); i += 4;
		if (nLength + i > nSize) {//包未完全接收，就返回，解析失败
			nSize = 0;
			return;
		}
		sCmd = *(WORD*)(pData + i); i += 2;
		if (nLength > 4) {
			strData.resize(nLength - 2 - 2);
			_memccpy((void*)strData.c_str(), pData + i, 0, nLength - 2 - 2);
			i += nLength - 2 - 2;
		}
		sSum = *(WORD*)(pData + i);
		WORD sum = 0;
		for (size_t j = 0; j < strData.size(); j++) {
			sum += BYTE(strData[j]) & 0xFF;
		}
		if (sum == sSum) {
			nSize = nLength + 2 + 4;//包头2字节，长度4字节，数据...
			return;
		}
		nSize = 0;
	}
	~CPacket() {}
	CPacket& operator=(const CPacket& packet) {
		if (this != &packet) {
			sHead = packet.sHead;
			nLength = packet.nLength;
			sCmd = packet.sCmd;
			strData = packet.strData;
			sSum = packet.sSum;
		}
		return *this;
	}

	int size() {
		return nLength + 6;
	}
	const char* Data() {
		return (const char*)this;
		strOut.resize(nLength + 6);
		BYTE* pData = (BYTE*)strOut.c_str();
		*(WORD*)pData = sHead; pData += 2;
		*(DWORD*)pData = nLength; pData += 4;
		*(WORD*)(pData) = sCmd; pData += 2;
		_memccpy(pData, strData.c_str(), 0,strData.size()); pData += strData.size();
		*(WORD*)pData = sSum;
		return strOut.c_str();
	}


public:
	WORD sHead;//包头固定位 FE FF
	DWORD nLength;//数据长度（从控制命令开始，到校验和结束）
	WORD sCmd;//控制命令
	std::string strData;//数据
	WORD sSum;//校验和
	std::string strOut;//整个包的数据

};

class CServerSocket
{
public:
	static CServerSocket* GetInstance() {
		if (m_instance == NULL) { //静态函数没有this指针，所以不能直接访问成员变量
			m_instance = new CServerSocket();
		}
		return m_instance;
	}
	bool InitSocket() {
		if (m_sock == INVALID_SOCKET)
			return false;
		sockaddr_in serv_adr;
		memset(&serv_adr, 0, sizeof(serv_adr));
		serv_adr.sin_family = AF_INET;
		serv_adr.sin_addr.s_addr = INADDR_ANY;
		serv_adr.sin_port = htons(9527);
		//绑定
		bind(m_sock, (sockaddr*)&serv_adr, sizeof(serv_adr));// TODO:
		//TODO:
		listen(m_sock, 1);
		return true;
	}
	bool AcceptClient() {
		sockaddr_in serv_adr;
		int cli_sz = sizeof(sockaddr_in);
		SOCKET client = accept(m_sock, (sockaddr*)&serv_adr, &cli_sz);
		if (m_client == INVALID_SOCKET)
			return false;
		return true;
	}
#define BUFFER_SIZE 4096
	int DealCommand() {
		/*char buffer[1024];*/
		char* buffer = new char[BUFFER_SIZE];
		memset(buffer, 0, BUFFER_SIZE);
		size_t index = 0;
		while (true) {
			size_t len = recv(m_client, buffer + index, BUFFER_SIZE - index, 0);
			if (len <= 0) {
				return -1;
			}
			index += len;
			m_packet = CPacket((BYTE*)buffer, len);
			if (len > 0) {
				memmove(buffer, buffer + len, BUFFER_SIZE - len);
				index -= len;
				return m_packet.sCmd;
			}
		}
	}
	bool Send(const char* pData, int nSize) {
		if(m_client == INVALID_SOCKET) return false;
		return send(m_client, pData, nSize, 0) > 0;
	}
	bool Send(CPacket& pack) {
		if (m_client == INVALID_SOCKET) return false;
		return send(m_client, pack.Data(), pack.size(), 0) > 0;
	}
	bool GetFilePath(std::string& strPath) {
		if (m_packet.sCmd == 2) {
			strPath = m_packet.strData;
			return true;
		}
		return false;
	}

private:
	SOCKET m_client;
	SOCKET m_sock;
	CPacket m_packet;
	CServerSocket& operator=(const CServerSocket& ss) {}
	CServerSocket(const CServerSocket&) {}
	CServerSocket() {
		if (InitSockEnv() == FALSE) {
			MessageBox(NULL, _T("无法初始化套接字环境，请检查网络设置！"), _T("初始化错误！"), MB_OK | MB_ICONERROR);
			exit(0);
		}
		m_sock = socket(PF_INET, SOCK_STREAM, 0);
	}
	~CServerSocket() {
		WSACleanup();
		closesocket(m_sock);
	}
	BOOL InitSockEnv() {
		WSADATA data;
		if (WSAStartup(MAKEWORD(2, 2), &data) != 0) {
			return FALSE;
		}
		return TRUE;
	}
	static void releaseInstance() {
		if (m_instance != NULL) {
			CServerSocket* tmp = m_instance;
			m_instance = NULL;
			delete tmp;
		}
	}

	static CServerSocket* m_instance;
	class CHelper {
	public:
		CHelper() {
			CServerSocket::GetInstance();
		}
		~CHelper() {
			CServerSocket::releaseInstance();
		}
	};
	static CHelper m_helper;
};