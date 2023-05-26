#pragma once

#ifndef _MAIN_H
#define _MAIN_H

#include <stdlib.h>
#include <Winsock2.h>
#include <Windows.h> 
#include <fstream> 
#include <detours.h>
#include <tchar.h>
#include <conio.h>
#include <stdio.h>
#include <iostream> 
#include <string.h>
#include <string>
#include <list>

#pragma comment(lib, "detours.lib") 
#pragma comment(lib, "ws2_32.lib") 

typedef struct _INIT_STRUCT {
	char* data;
	int length;
} INIT_STRUCT, *PINIT_STRUCT;
#endif

using namespace std; 


int ( WINAPI * Real_send)(SOCKET a0,   CONST char* a1,   int a2,   int a3)   = send;  
int (WINAPI* Real_recv)(SOCKET socket, LPWSABUF lpBuffers, DWORD dwBufferCount, LPDWORD lpNumberOfBytesRecvd, LPDWORD lpFlags, LPWSAOVERLAPPED lpOverlapped, LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine) = WSARecv;
int ( WINAPI *Real_connect )(SOCKET s,const struct sockaddr *name, int namelen) = connect;


//typedef int (WINAPI* SendPtr)(SOCKET s, const char* buf, int len, int flags);
//HMODULE hLib = LoadLibrary("wsock32.dll");
//SendPtr pRecv = (SendPtr)GetProcAddress(hLib, "recv");



int WINAPI custom_send(SOCKET sock, char* buf, int len, int flags); 
int WINAPI custom_recv(SOCKET socket, LPWSABUF lpBuffers, DWORD dwBufferCount, LPDWORD lpNumberOfBytesRecvd, LPDWORD lpFlags, LPWSAOVERLAPPED lpOverlapped, LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine);
//int WINAPI custom_recv2(SOCKET s, char* buf, int len, int flags);
int WINAPI custom_connect(SOCKET s,const struct sockaddr *name, int namelen); 

HANDLE hPIPE;
HANDLE NamedPipe;

char* TextToHex( char* text) 
{ 
    if (!text) return 0; 

    int len = strlen(text); 
	char asd[6] = "asd";


    char *buffer = new char[2*len]+1; 
    if (!buffer) return 0; 
	
    for (int i=0; i<len; i++) 
        sprintf( buffer+2*i, "%x", (unsigned int) text[i]); 

    return buffer; 
} 
char* getText(char* text, DWORD length)
{
    if (!text) return 0;

    int len = strlen(text);


	char * str;
	string str2 = text;
	string str3;
	str3 = str2.substr(0,length);
	str = new char[str3.length()+1];
	strcpy(str,str3.c_str());
	int test = str3.length();

    return str; 
} 
void WriteLog(char FAR* buf, int len) 
{
	BOOL bWrite = false;
	DWORD BYTESWRITTEN = 0;
	if (hPIPE != INVALID_HANDLE_VALUE)
	{
		//Write char array "buffer" to the pipe handle held in hPIPE
		bWrite = WriteFile(hPIPE, buf, len, &BYTESWRITTEN, NULL);
	}
}

bool isUsing = false;
SOCKET mysock;

int WINAPI custom_send(SOCKET sock, char* buf, int len, int flags)
{ 
	if(mysock == NULL)
	{
		mysock = sock;
	}
	while(isUsing == true)
	{
		Sleep(1);
	}
	isUsing = true;
    int returnValue =  Real_send(sock, buf, len, flags); 
	//WriteLog(buf, len);
	isUsing = false;
	return returnValue;
}
int WINAPI custom_recv(SOCKET socket, LPWSABUF lpBuffers, DWORD dwBufferCount, LPDWORD lpNumberOfBytesRecvd, LPDWORD lpFlags, LPWSAOVERLAPPED lpOverlapped, LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine)
{ 
	if(mysock == NULL)
	{
		mysock = socket;
	}
	//MessageBox(NULL, buf, NULL, NULL);
	
    int a = Real_recv(socket, lpBuffers, dwBufferCount, lpNumberOfBytesRecvd, lpFlags, lpOverlapped, lpCompletionRoutine);
	
	WriteLog(lpBuffers->buf, lpBuffers->len);
	return a;
} 

//int WINAPI custom_recv2(SOCKET s, char* buf, int len, int flags)
//{
//	WriteLog(buf, len);
//	return pRecv(s, buf, len, flags); //you need to call recv first
//}



int WINAPI custom_connect(SOCKET s, const struct sockaddr *name, int namelen) 
{ 
	mysock = s;
    return Real_connect(s, name, namelen); 
} 

int  WINAPI NamedPipeClient()//der erstellte Thread
{
	
	BOOL bWrite = false;
	DWORD BYTESWRITTEN = 0;	
	char buffer[64];	
	//copy string into buffer and fill with terminating null characters
	strcpy( buffer, " Hello " ); 
	
	if ( hPIPE != INVALID_HANDLE_VALUE )
	{
		//Write char array "buffer" to the pipe handle held in hPIPE
		bWrite = WriteFile( hPIPE, buffer, sizeof(buffer), &BYTESWRITTEN, NULL );
		if ( bWrite == FALSE )
		{
			MessageBox(NULL, "WriteFile() Error!", "ERROR", MB_OK);
		}
	}
	else 
	{
		MessageBox(NULL, "CREATEFILE FAILED!", "ERROR!", MB_OK);
	}	
	while(1)
	{	
		Sleep(1);
	}
	return 0;
}
std::string GetLastErrorStdStr()
{
	DWORD error = GetLastError();
	if (error)
	{
		LPVOID lpMsgBuf;
		DWORD bufLen = FormatMessage(
			FORMAT_MESSAGE_ALLOCATE_BUFFER |
			FORMAT_MESSAGE_FROM_SYSTEM |
			FORMAT_MESSAGE_IGNORE_INSERTS,
			NULL,
			error,
			MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
			(LPTSTR)&lpMsgBuf,
			0, NULL);
		if (bufLen)
		{
			LPCSTR lpMsgStr = (LPCSTR)lpMsgBuf;
			std::string result(lpMsgStr, lpMsgStr + bufLen);

			LocalFree(lpMsgBuf);

			return result;
		}
	}
	return std::string();
}
void openPipe()
{
	LPTSTR lpszPipename = "\\\\.\\pipe\\myRosePipe"; 
	DWORD currPID = GetCurrentProcessId();
	char bufa[15];
	wsprintf(bufa,"%d",currPID);		
	char *pipeName = new char[strlen(bufa)+strlen(lpszPipename)+1];
	strcpy(pipeName,lpszPipename);
	strcat(pipeName,bufa);
	cout << GetLastError();
	hPIPE = CreateFile(pipeName, GENERIC_WRITE,0, NULL, OPEN_EXISTING,0, NULL);
	if (hPIPE == INVALID_HANDLE_VALUE)
	{
		MessageBox(NULL, GetLastErrorStdStr().c_str(), "openPipe failed!", MB_OK);
	}
}


BOOL WINAPI DllMain(HMODULE hModule, DWORD reason, LPVOID reserved) 
{ 
    if ( reason == DLL_PROCESS_ATTACH ) 
    { 
		DisableThreadLibraryCalls( hModule );
		DetourTransactionBegin();
		DetourUpdateThread( GetCurrentThread() );
		DetourAttach( &(PVOID&)Real_send, custom_send );
		DetourAttach( &(PVOID&)Real_recv, custom_recv );
		//DetourAttach(&(PVOID&)pRecv, custom_recv2);
		DetourAttach( &(PVOID&)Real_connect, custom_connect );
		DetourTransactionCommit();
		openPipe();
		NamedPipe = CreateThread(0, 0, (LPTHREAD_START_ROUTINE) NamedPipeClient, 0, 0, 0);
		//return 0;
    } 
    else if ( reason == DLL_PROCESS_DETACH ) 
    {
		DetourTransactionBegin();
		DetourUpdateThread( GetCurrentThread() );
		DetourDetach( &(PVOID&)Real_send, custom_send );
		DetourDetach( &(PVOID&)Real_recv, custom_recv );
		//DetourDetach( &(PVOID&)pRecv, custom_recv2);
		DetourDetach( &(PVOID&)Real_connect, custom_connect );
		
		DetourTransactionCommit();
		TerminateThread( NamedPipe, -1000 );
		CloseHandle( NamedPipe );
		NamedPipe = NULL;
		CloseHandle(hPIPE);
		hPIPE= NULL;
    }     
    return true; 
} 


extern "C" __declspec(dllexport) void sendData(PVOID message) {

	PINIT_STRUCT messageStruct = reinterpret_cast<PINIT_STRUCT>(message);
	int lpc_len = messageStruct->length;
	LPCSTR lpc_data = messageStruct->data;
	char *p = const_cast<char*>(lpc_data);
	custom_send(mysock, p, lpc_len, 0);
}

