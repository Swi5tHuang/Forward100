﻿/*
* Author:
*
*        Huang Hanjie (h.forward100@gmail.com)
*
* History: VS2017
*
*         Jun.19, 2018 Nanjing (created)
*
* Notes:
*
*         Keep forging ahead!
*
*/








#include "stdafx.h"
#include <iostream>
#include <winsock.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>
#pragma comment(lib, "wsock32")
using namespace std;


/*============================================================================*/
/////////////////////////////////////////////////////////////////////////////
//global variable
/////////////////////////////////////////////////////////////////////////////
SOCKET m_sock = INVALID_SOCKET;//connection socket 
char ipaddr[1024] = { 0 };//ip address
char m_svraddr[1024] = { 0 };//server domain
unsigned short port = 110;//POP3 port

char UserName[1024] = { "aaaaa@163.com" };//email address
char PassWord[1024] = { "xxxxxx" };//email password


								   ////////////////////////////////////////////////////////////////
								   //send function
								   ////////////////////////////////////////////////////////////////
int Send(char* buf, int len, int flags)
{
	int bytes = 0;
	int count = 0;

	while (count<len)
	{
		bytes = send(m_sock, buf + count, len - count, flags);
		if ((bytes == -1) || (bytes == 0))
		{
			return -1;
		}
		count = count + bytes;
	}
	return count;
}


////////////////////////////////////////////////////////////////
//receive message from server
////////////////////////////////////////////////////////////////
int Recv(char* buf, int len, int flags)
{
	cout << "++++++++++++++++++" << endl << endl;
	return(recv(m_sock, buf, len, flags));
}



////////////////////////////////////////////////////////////////////////////
//connect to server
///////////////////////////////////////////////////////////////////////////
int EmailServerConnect()
{

	//******************************************
	//create socket
	//******************************************
	m_sock = socket(AF_INET, SOCK_STREAM, 0);
	if (m_sock == INVALID_SOCKET)
	{
		//if create socket failed,it return -1
		return -1;
	}


	//*******************************************************
	//get the address of email server
	//*******************************************************
	struct hostent* p;
	char domain[1024] = { "pop.163.com" };//the url depends on 
	memset(ipaddr, 0, sizeof(ipaddr));
	//if((p=gethostbyname(m_svraddr))==NULL)
	if ((p = gethostbyname(domain)) == NULL)
	{
		//get ip address of server，return -2
		return -2;
	}
	else
	{
		//if you get IP address success，IP address will store in array
		sprintf(ipaddr, "%u.%u.%u.%u", (unsigned char)p->h_addr_list[0][0], (unsigned char)p->h_addr_list[0][1], (unsigned char)p->h_addr_list[0][2], (unsigned char)p->h_addr_list[0][3]);
		cout << "服务器IP地址：" << ipaddr << endl << endl;
	}


	//**********************************************************************************
	//bind socket(if you don't know socket,you should to look through network programming）
	//**********************************************************************************
	struct sockaddr_in svraddr;
	svraddr.sin_family = AF_INET;
	svraddr.sin_addr.s_addr = inet_addr(ipaddr);
	svraddr.sin_port = htons(port);

	//connect to pop3 server
	int isConnect = 0;
	isConnect = connect(m_sock, (struct sockaddr*)&svraddr, sizeof(svraddr));
	if (isConnect == SOCKET_ERROR)
	{
		//if connect failed,return -3
		return -3;
	}


	//*********************************************************************
	//connected succcessful,server should send "welcome information" to you
	//*********************************************************************
	char buf[65536] = { 0 };
	memset(buf, 0, sizeof(buf));

	int recvCount = 0;
	recvCount = Recv(buf, sizeof(buf), 0);

	if ((recvCount <= 0) || (strncmp(buf, "+OK", 3) != 0))
	{
		//then this is receive "success information failed
		return -4;
	}

	buf[recvCount] = '\0';

	//show information which server give you
	cout << "Recv POP3 Resp: " << buf << endl << endl;

	return 1;
}



////////////////////////////////////////////////////////////////////////////
//login mail server
///////////////////////////////////////////////////////////////////////////
int LoginEmailServer()
{
	char sendbuf[1024] = { 0 };
	char recvbuf[65536] = { 0 };

	//int bytes=0;
	//int count=0;
	//int sendbufLen=0;


	/////////////////////////////////////////////////////////////////////////
	//send UserName[1024] to server
	/////////////////////////////////////////////////////////////////////////
	memset(sendbuf, 0, sizeof(sendbuf));
	sprintf(sendbuf, "USER %s\r\n", UserName);

	Send(sendbuf, strlen(sendbuf), 0);

	memset(recvbuf, 0, sizeof(recvbuf));
	int rs = Recv(recvbuf, sizeof(recvbuf), 0);
	if ((rs <= 0) || (strncmp(recvbuf, "+OK", 3) != 0))
	{
		//reiceve "serever get username" information failed,then return -2
		return -2;
	}
	recvbuf[rs] = '\0';
	cout << "Recv USER Resp: " << recvbuf << endl << endl;

	/////////////////////////////////////////////////////////////////////////
	//send password to server
	/////////////////////////////////////////////////////////////////////////
	memset(sendbuf, 0, sizeof(sendbuf));
	sprintf(sendbuf, "PASS %s\r\n", PassWord);

	Send(sendbuf, strlen(sendbuf), 0);

	memset(recvbuf, 0, sizeof(recvbuf));
	rs = Recv(recvbuf, sizeof(recvbuf), 0);
	if ((rs <= 0) || (strncmp(recvbuf, "+OK", 3) != 0))
	{
		//reiceve "get password feedback" failed，return -4
		return -4;
	}
	recvbuf[rs] = '\0';
	cout << "Recv PASS Resp: " << recvbuf << endl << endl;

	//if you login successful,return 1
	return 1;
}



////////////////////////////////////////////////////////////////
//get count of email
////////////////////////////////////////////////////////////////
int GetMailSum(char* buf)
{
	int sum = 0;
	char* p = strstr(buf, "\r\n");
	if (p == NULL)
	{
		return sum;
	}
	p = strstr(p + 2, "\r\n");
	if (p == NULL)
	{
		return sum;
	}
	while ((p = strstr(p + 2, "\r\n")) != NULL)
	{
		sum++;
	}
	return sum;
}
////////////////////////////////////////////////////////////////
//POP3Recv
////////////////////////////////////////////////////////////////
int POP3Recv(char *buf, int len, int flags)
{
	int rs = 0;
	int offset = 0;

	do
	{

		if (offset>len - 2)
		{
			return offset;
		}

		rs = Recv(buf + offset, len - offset, flags);

		if (rs<0)
		{
			return -1;
		}
		offset = offset + rs;
		buf[offset] = '\0';

	} while (strstr(buf, "\r\n.\r\n") == (char*)NULL);


	return offset;
}


///////////////////////////////////////////////////////////////////////////
//Determine if there is an email to receive
///////////////////////////////////////////////////////////////////////////
int GetCountOfEmail()
{
	char sendbuf[1024] = { 0 };
	char recvbuf[65536] = { 0 };


	memset(sendbuf, 0, sizeof(sendbuf));
	sprintf(sendbuf, "LIST \r\n");




	Send(sendbuf, strlen(sendbuf), 0);



	memset(recvbuf, 0, sizeof(recvbuf));
	int rs = POP3Recv(recvbuf, sizeof(recvbuf), 0);

	//cout<<"********************"<<endl;
	if ((rs <= 0) || (strncmp(recvbuf, "+OK", 3) != 0))
	{
		//Failed to receive feedback from mailing list requests，return -1
		return -1;
	}
	recvbuf[rs] = '\0';
	cout << "Recv LIST Resp: " << recvbuf << endl << endl;

	int count = 0;//Record the number of emails
	count = GetMailSum(recvbuf);//get number of email

	cout << "Email's number:" << count << endl << endl;
	return count;
}

//decode base64
unsigned char Decode(char cTemp)
{
	if ((cTemp >= 'A') && (cTemp <= 'Z'))
	{
		return cTemp - 'A';
	}
	if ((cTemp >= 'a') && (cTemp <= 'z'))
	{
		return cTemp - 'a' + 26;
	}
	if ((cTemp >= '0') && (cTemp <= '9'))
	{
		return cTemp - '0' + 52;
	}
	if (cTemp == '+') return 62;
	if (cTemp == '/') return 63;
	if (cTemp == '=') return 0;
}

//subject:email's subject
//num:email's id
void Base64Decode(char* subject)
{

	FILE* pOutFile;
	int iLenght, iLoop;
	char ucBuffer[4096];
	unsigned long ulTemp = 0;

	int i = 0, j = 0, k = 0;
	char result[4096];
	memset((char*)result, 0, sizeof(result));
	memset((char*)ucBuffer, 0, sizeof(ucBuffer));


	char tempFileName[128] = { 0 };
	char intToStr[10] = { 0 };

	strcpy(ucBuffer, subject);//Store the mail subject in ucbuffer

	iLenght = strlen((char*)ucBuffer);
	if (ucBuffer[iLenght - 1] == '\n')
	{
		iLenght--;
	}
	for (iLoop = 0; (iLoop + 4) <= iLenght; iLoop += 4)
	{
		ulTemp = Decode(*(ucBuffer + iLoop));
		ulTemp = (ulTemp << 6) + Decode(*(ucBuffer + iLoop + 1));
		ulTemp = (ulTemp << 6) + Decode(*(ucBuffer + iLoop + 2));
		ulTemp = (ulTemp << 6) + Decode(*(ucBuffer + iLoop + 3));
		*(ucBuffer) = (ulTemp >> 16) & 0xff;
		*(ucBuffer + 1) = (ulTemp >> 8) & 0xff;
		*(ucBuffer + 2) = ulTemp & 0xff;
		/*
		result[i]=*(ucBuffer);
		result[i+1]=*(ucBuffer+1);
		result[i+2]=*(ucBuffer+2);
		i=i+3;
		*/
	}
	printf("%s", ucBuffer);
	return;
}

///////////////////////////////////////////////////////////////////////////
//GetSubject
///////////////////////////////////////////////////////////////////////////
bool GetSubject(char* subject, char* buf)
{
	char subBuf[1024] = { 0 };

	char* pStart = strstr(buf, "Subject: ");//Find the starting location of the mail topic
	if (pStart == NULL)
	{
		return false;
	}

	//pStart=pStart+9;
	char* pEnd = strstr(pStart, "?=");//Find the end location of the mail topic
	if (pEnd == NULL)
	{
		printf("false\n");
		return false;
	}

	//Find the true starting position of the subject based on different encoding methods, for intercepting filename
	char* pCode = strstr(pStart, "gb2312?B?");
	if (pCode == NULL)
	{
		if ((pCode = strstr(pStart, "UTF-8?B?")) == NULL)
		{
			return false;
		}
		else
		{
			pCode = pCode + 8;
		}
	}
	else
	{
		pCode = pCode + 9;
	}
	/*
	for(int i=0;i<32;i++)
	{
	if((pStart[i]=='\r')||(pStart[i]=='\n'))
	{
	subject[i]='\0';
	break;
	}
	subject[i]=pStart[i];
	}
	*/
	strncpy(subBuf, pStart, (pEnd - buf) - (pStart - buf));
	strncpy(subject, pCode, pEnd - pCode);
	Base64Decode(subject);
	//Output message subject information (in coding form)

	printf("%s\n", subject);
	return true;
}

//Get sender and receiver information
void GetSenderAndReceiver(char* buf)
{
	char* pStart = strstr(buf, "From: ");//Find the sender's starting location
	if (pStart == NULL)
	{
		printf("No Sender.\n");
		return;
	}

	char* pEnd = strstr(pStart, "Message-ID: ");
	if (pEnd == NULL)
	{
		printf("No Receiver.\n");
		return;
	}

	char sendAndRecvBuf[2048] = { 0 };
	strncpy(sendAndRecvBuf, pStart, pEnd - pStart);

	printf("%s\n", sendAndRecvBuf);
	return;
}


///////////////////////////////////////////////////////////////////////////
//Get mail content
///////////////////////////////////////////////////////////////////////////
bool GetEmailContents(SOCKET connection, int nMsg, int nLine)
{
	const int RESPONSE_BUFFER_SIZE = 1024;
	char sRetr[100] = { 0 };
	sprintf(sRetr, "RETR %d\r\n", nMsg);

	char response_buf[RESPONSE_BUFFER_SIZE * 10] = { 0 };
	send(connection, sRetr, strlen(sRetr), 0);
	Sleep(1000);     //Delays must be added in order to receive the data fully
	if (recv(connection, response_buf, RESPONSE_BUFFER_SIZE * 10, 0) == SOCKET_ERROR)
		return FALSE;

	char *ret; //Location starting point
	char *findEndLine;//Location cut-off point
	char subject[8] = "Subject";
	char EndLine[3] = "\n";

	char textPart[40] = "Content-Type: text/plain; charset=";
	char outPutSubject[128] = { 0 };
	char sysdictate[128] = { 0 };
	ret = strstr(response_buf, subject) + 9;//Locate the starting point of the topic
	findEndLine = strstr(ret, EndLine);//Locating the end of topic

	unsigned n = findEndLine - ret;//Get the target length
	strncpy(outPutSubject, ret, n);

	ret = strstr(ret, textPart) + 41;
	//The instruction content ends with a “+”
	findEndLine = strstr(ret, "+");
	n = findEndLine - ret;
	strncpy(sysdictate, ret, n);

	FILE *fpt;//if you don't need store to file you can delete this part.
	if ((fpt = fopen("subject.txt", "w")) == NULL) //Save the topic content to a file
	{
		printf("open file failed!\n");
		exit(0);
	}
	fprintf(fpt, "%s", outPutSubject);
	fclose(fpt);

	printf("%s\n", outPutSubject);  //output subject of email


									//printf("%s\n", sysdictate);  //output indicate of email

									//system(sysdictate);   //do system dictate

	printf("%s\n", response_buf);  //output all of email





	return TRUE;
}





///////////////////////////////////////////////////////////////////////////
//quit from email server
///////////////////////////////////////////////////////////////////////////
bool QuitFromPOP3ReceiveClient()
{
	char sendbuf[1024] = { 0 };
	char recvbuf[65536] = { 0 };

	/*Send Quit Command*/
	memset(sendbuf, 0, sizeof(sendbuf));
	sprintf(sendbuf, "QUIT\r\n");
	Send(sendbuf, strlen(sendbuf), 0);

	memset(recvbuf, 0, sizeof(recvbuf));
	int rs = Recv(recvbuf, sizeof(recvbuf), 0);
	if ((rs <= 0) || (strncmp(recvbuf, "+OK", 3)) != 0)
	{
		return false;
	}

	recvbuf[rs] = '\0';
	cout << "Recv QUIT Resp: " << recvbuf << endl << endl;

	closesocket(m_sock);
	return true;
}





//////////////////////////////////////////////////////////
//get username and password information
/////////////////////////////////////////////////////////
void InputEmailInfo(char *info)
{
	gets_s(info, 65535);
}

//////////////////////////////////////////////////////////
//main function
/////////////////////////////////////////////////////////
int main()
{

	char Email[128] = { 0 };//username of email
	char PassWord[128] = { 0 };//password of email

	int count = 0;//number of emails
	int i = 0;


	cout << "************************************************************************" << endl;
	cout << "********************Start email program**************************" << endl;
	cout << "************************************************************************" << endl << endl << endl;

	WSADATA wsaData;
	int ret = WSAStartup(MAKEWORD(2, 0), &wsaData);
	if (ret)
	{
		cout << "init winsock failed!" << endl << endl;
	}


	int isConnect = 0;
	isConnect = EmailServerConnect();
	if (isConnect == -1)
	{
		cout << "create failed!" << endl << endl;
		system("pause");
		return 0;
	}
	if (isConnect == -2)
	{
		cout << "get pop3 address failed!" << endl << endl;
		system("pause");
		return 0;
	}
	if (isConnect == -3)
	{
		cout << "connect to pop3 server failed!" << endl << endl;
		system("pause");
		return 0;
	}
	if (isConnect == -4)
	{
		cout << "reiceve server feedback failed!" << endl << endl;
		system("pause");
		return 0;
	}
	else if (isConnect == 1)
	{
		cout << "connect server successful!" << endl << endl;
		//system("pause");
	}





	////////////////////////////////////////////////////////////////////////
	//login up server
	////////////////////////////////////////////////////////////////////////
	int isLogin = 0;
	isLogin = LoginEmailServer();
	if (isLogin == -1)
	{
		cout << "send username failed,can't login!" << endl << endl;
		system("pause");
		return 0;
	}
	if (isLogin == -2)
	{
		cout << "reiceve server username feedback failed!" << endl << endl;
		system("pause");
		return 0;
	}
	if (isLogin == -3)
	{
		cout << "send password failed!" << endl << endl;
		system("pause");
		return 0;
	}
	if (isLogin == -4)
	{
		cout << "receive server password feedback failed!" << endl << endl;
		system("pause");
		return 0;
	}
	if (isLogin == 1)
	{
		cout << "login successful!" << endl << endl;
	}




	///////////////////////////////////////////////////////////////////////////
	//get number of email
	//////////////////////////////////////////////////////////////////////////
	count = 0;
	count = GetCountOfEmail();



	if (count<0)//if you email is empty,it will tell you that.
	{
		//cout << "email is empty!" << endl;
		return -1;
	}
	else//get email
	{
		cout << "there are" << count << "emails need to recv" << endl << endl;
		for (i = 1; i <= count; i++)
		{
			printf("\n");
			printf("\n");
			cout << "get [" << i << "] th(st/sd) email content：" << endl;

			////////////////////////////
			//get content
			////////////////////////////
			GetEmailContents(m_sock, i, 100);
			system("pause");


			cout << endl << endl;
		}
	}


	/////////////////////////////////////////////////////////////////////////////////////
	//quit from server
	/////////////////////////////////////////////////////////////////////////////////////
	QuitFromPOP3ReceiveClient();

	system("pause");
	return 0;

}
