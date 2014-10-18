﻿/* 
   Copyright 2013 KLab Inc.

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.
*/
//
//  CSockReadStream.cpp
//
#include <Windows.h>
#include <WinSock.h>

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <iostream>

#include "CPFInterface.h"
#include "CSockReadStream.h"
#include "CSockWriteStream.h"


CSockReadStream::CSockReadStream() : m_writeStream(0), m_eStat(NORMAL), m_fd(0), m_lastPos(0), m_getPos(0) {}
CSockReadStream::~CSockReadStream()
{
    // 生成時に作成した書き込み用ストリームオブジェクトを破棄
    if(m_writeStream) delete m_writeStream;
    // 破棄時にソケットをクローズする
    if(m_fd) closesocket(m_fd);    
}

/*!
 \param  hostname   IPアドレス表記またはFQDN表記のホスト指定
 \param  port       接続する TCP/IP の port 番号

 \return 接続成功ならば true, 失敗すれば false
 */
int
CSockReadStream::sock_connect(const char *hostname, int port)
{
    int dstSocket;
    struct sockaddr_in dstAddr;

    m_fd = 0;
    memset(&dstAddr, 0, sizeof(dstAddr));
    dstAddr.sin_port = htons((u16)port);
    dstAddr.sin_family = AF_INET;
    //dstAddr.sin_addr.s_addr = inet_addr(hostname);
	dstAddr.sin_addr.S_un.S_addr = inet_addr(hostname);

    if(dstAddr.sin_addr.S_un.S_addr == 0xffffffff) {
        struct hostent *host;
        host = gethostbyname(hostname);
        if(!host) return -1;
        dstAddr.sin_addr.s_addr = *(unsigned int *)host->h_addr_list[0];
    }
    dstSocket = socket(AF_INET, SOCK_STREAM, 0);

    if(0 > connect(dstSocket, (struct sockaddr *)&dstAddr, sizeof(dstAddr))) {
		closesocket(dstSocket);	// クローズを忘れるとディスクリプタを食いつぶす
        return -1;
	}

    // 取得したディスクリプタは、ノンブロックモードにて運用する。
    //int flags = fcntl(dstSocket, F_GETFL, 0);
    //fcntl(dstSocket, F_SETFL, flags | O_NONBLOCK);
	u_long val = 1;
	ioctlsocket(dstSocket, FIONBIO, &val);

    return dstSocket;
}

int	CSockReadStream::sock_listen(unsigned port)
{
	SOCKET        sListen,sClient;
	int           iAddrSize;
	struct sockaddr_in local,client;

	sListen = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
	if (sListen == SOCKET_ERROR)
	{
		//printf("socket() failed: %d\n", WSAGetLastError());
		return SOCKET_ERROR;
	}

	local.sin_addr.s_addr = htonl(INADDR_ANY);
	local.sin_family = AF_INET;
	local.sin_port = htons(port);

	if (bind(sListen, (struct sockaddr *)&local,sizeof(local)) == SOCKET_ERROR)
	{
		//printf("bind() failed: %d\n", WSAGetLastError());
		return SOCKET_ERROR;
	}
	::listen(sListen, 8);

	u_long NonBlock;
	while (1)
	{
		iAddrSize = sizeof(client);
		sClient = accept(sListen, (struct sockaddr *)&client,&iAddrSize);
		if (sClient == INVALID_SOCKET)
		{
			//printf("accept() failed: %d\n", WSAGetLastError());
			return SOCKET_ERROR;
		}
		//printf("Accepted client: %s:%d\n",
		//	inet_ntoa(client.sin_addr), ntohs(client.sin_port));
		NonBlock = 1;
		if (ioctlsocket(sClient, FIONBIO, &NonBlock) == SOCKET_ERROR)
		{
			//printf("ioctlsocket() failed with error %d\n", WSAGetLastError());
			return SOCKET_ERROR;
		}
		return sClient;

	}

}

bool
CSockReadStream::setStatus()
{
	int result = WSAGetLastError();
    switch(result)
    {
        default:
			m_eStat = CLOSED;
			if(m_fd > 0) {
				closesocket(m_fd);
				m_fd = 0;
			}
            CPFInterface::getInstance().platform().logging("errno = %d", errno);
            break;
        case WSAEWOULDBLOCK:
            m_eStat = NOT_AVAILABLE;
      //      CPFInterface::getInstance().platform().logging("errno = EAGAIN");
            break;
        case WSAETIMEDOUT:
            m_eStat = CLOSED;
            if(m_fd > 0) {
                closesocket(m_fd);
                m_fd = 0;
            }
            CPFInterface::getInstance().platform().logging("errno = ETIMEDOUT");
            break;
    }
    return false;   // 必ず false を返す
}
/*!
 \return  リングバッファ読み込みが正常終了していれば true, 失敗ならば false
 
 受信データをリングバッファに蓄積します。
 */
bool
CSockReadStream::readRingBuf()
{
    if((m_lastPos > m_getPos) && (m_lastPos < READ_BUFSIZ)) {
        int result = recv(m_fd, (char *)m_readBuf + m_lastPos, READ_BUFSIZ - m_lastPos, 0);
        if(result < 0) { return false; }
        m_lastPos += result;
    }
    if((m_lastPos == READ_BUFSIZ) && (m_getPos > 0)) {
        int result = recv(m_fd, (char *)m_readBuf, m_getPos, 0);
        if(result < 0) { return false; }
        if(result > 0) { m_lastPos = result; }
    } else if(m_lastPos < m_getPos) {
		int result = recv(m_fd, (char *)m_readBuf + m_lastPos, m_getPos - m_lastPos, 0);
        if(result < 0) { return false; }
        m_lastPos += result;
    } else if(m_getPos == 0 && m_lastPos == 0) {
        // バッファが完全に空の状態
        int result = recv(m_fd, (char *)m_readBuf, READ_BUFSIZ, 0);
        if(result < 0) { return false; }
        m_lastPos += result;
    }
	// m_lastPos == m_getPos && m_lastPos > 0 のときはバッファがいっぱい
	// m_lastPos == READ_BUFSIZE && m_getPos == 0 のときも同様

    return true;
}


/*!
 \param buf     データを受け取るバッファ
 \param size    要求サイズ

 読み込みバッファを経由して、指定サイズのデータを取得する。
 ネットワークの読み込みが追いつかない場合、読み込みポインタが更新されない。
 */
bool
CSockReadStream::requestData(unsigned char * buf, size_t reqSize)
{
    // 最初にステータスを NORMAL にする。途中で問題があれば書きかわる。
    // 正常系を通れば問題なくNORMALを維持できる。
    m_eStat = NORMAL;

    // リングバッファへの読み込みを試みる。
    if(!readRingBuf()) {
		setStatus();
	}

    // リングバッファの残りサイズが要求サイズ以上であるか確認し、サイズが足りなければ false を返す。
    size_t leftSize = left_size();
    if(!reqSize || leftSize < reqSize) {
        // m_eStat = NOT_AVAILABLE;
        return false;
    }
    // リングバッファから指定のバッファに指定サイズを転送する
    int rdsiz = ((size_t)(READ_BUFSIZ - m_getPos) >= reqSize) ? reqSize : (READ_BUFSIZ - m_getPos);
    memcpy(buf, m_readBuf + m_getPos, rdsiz);
    m_getPos += rdsiz;
    if(m_getPos == READ_BUFSIZ) {
		m_getPos = 0;
        if(m_lastPos == READ_BUFSIZ) { m_lastPos = 0; }
	}
    if(0 < (reqSize - rdsiz)) {
        memcpy(buf + rdsiz, m_readBuf, reqSize - rdsiz);
        m_getPos += reqSize - rdsiz;
    }

    // 読み込み済みデータに追いついたら、一旦バッファを仕切り直す。
    if(m_lastPos == m_getPos) { m_lastPos = m_getPos = 0; }
    
    return true;
}

CSockReadStream *
CSockReadStream::openStream(const char * sockName)
{
    char * strHost = NULL;
    char * strPort = NULL;
    CSockReadStream * pStream = NULL;
    try {
        pStream = new CSockReadStream();

        // 渡されたホスト:portを分解する
        for(int i = 0; sockName[i]; i++) {
            if(sockName[i] == ':') {
                strHost = new char [ i + 1 ];
                strncpy(strHost, sockName, i);
                strHost[i] = 0;
                int plen = strlen(sockName + i + 1);
                strPort = new char [ plen + 1 ];
                strcpy(strPort, sockName + i + 1);
                break;
            }
        }
        if(!strHost || !strPort) {
            // 与えられた名称の書式が不正
            pStream->m_eStat = NOT_FOUND;
            if( strPort ) { delete [] strPort; }
			if( strHost ) { delete [] strHost; }
            return pStream;
        }

        // port を数値に変換する。
        // 数列であることを確認。
        for(int i = 0; strPort[i]; i++) {
            if(strPort[i] < '0' || strPort[i] > '9') {
                // 数字以外が含まれている場合は接続先指定として不正
                pStream->m_eStat = NOT_FOUND;
                if( strPort ) { delete [] strPort; }
				if( strHost ) { delete [] strHost; }
                return pStream;
            }
        }
        // 特に問題ないので、port を数値に変換する
        int port = atoi(strPort);

        // 接続を試みる
        int fd = pStream->sock_connect(strHost, port);
        if(fd < 0) {
            // 接続に失敗した場合
            pStream->m_eStat = NOT_FOUND;
            if( strPort ) { delete [] strPort; }
			if( strHost ) { delete [] strHost; }
            return pStream;
        }
        pStream->m_fd = fd;
        // 同時に書き込み用クラスを生成
        pStream->m_writeStream = new CSockWriteStream(*pStream);

        pStream->m_eStat = NORMAL;
        
    } catch(...) {
        if( strPort ) { delete [] strPort; }
		if( strHost ) { delete [] strHost; }
        delete pStream;
        return 0;
    }
    
    if( strPort ) { delete [] strPort; }
	if( strHost ) { delete [] strHost; }

    return pStream;
}

CSockReadStream * CSockReadStream::listen(unsigned port)
{
	char * strHost = NULL;
	CSockReadStream * pStream = NULL;
	pStream = new CSockReadStream();
	int fd = pStream->sock_listen(port);
	if (fd < 0)
	{
		pStream->m_eStat = NOT_FOUND;
		return pStream;
	}

	pStream->m_fd = fd;
	pStream->m_writeStream = new CSockWriteStream(*pStream);
	pStream->m_eStat = NORMAL;


	return pStream;
}

s32 CSockReadStream::getHostIp(char *ipaddr)
{
	char   myname[128];
	gethostname(myname, 128);
	hostent *hp = gethostbyname(myname);
	if (hp == NULL)
		return(-1);
	for (unsigned i = 0; hp->h_addr_list[i]; i++)
	{
		char* ip = inet_ntoa(*(struct in_addr*)hp->h_addr_list[i]);
		sprintf(ipaddr, "%s", ip);
		return 0;
	}

}


s32  
CSockReadStream::getSize()
{

    // 最初にステータスを NORMAL にする。途中で問題があれば書きかわる。
    // 正常系を通れば問題なくNORMALを維持できる。
    m_eStat = NORMAL;

    // リングバッファへの読み込みを試みる。
    if(!readRingBuf()) {
		setStatus();
//		return 0;
	}

	// リングバッファ中の有効サイズを返す
    size_t leftSize = left_size();
	return (s32)leftSize;
}

s32
CSockReadStream::getPosition()
{
    // ネットワークソケットは位置を取得できない
    return 0;
}

/*
 ネットワークストリームの場合、全ての読み込みメソッドにおいて
 *必ずしも読み込みが成功するわけではない* ことに注意する必要がある。
 相手ホストがデータを送信していない場合、たとえ8bitのデータでも読み込みに失敗することがある。
 
 readBlock() 以外は読み込みの成否を返すようには出来ていないため、
 実行後に必ず getStatus() でステータスを確認し、NOT_AVAILABLE である場合は
 戻り値に意味が無いものとして破棄する必要がある。
*/

u8
CSockReadStream::readU8()
{
    u8 buf;
    if(!requestData(&buf, 1)) { return 0xff; }
    return buf;
}


u16
CSockReadStream::readU16()
{
    u8 buf[2];
    if(!requestData(buf, 2)) { return 0xffff; }
    return (u16)buf[0] << 8 | (u16)buf[1];
}

u32
CSockReadStream::readU32()
{
    u8 buf[4];
    if(!requestData(buf, 4)) { return 0xffff; }
    return ((u32)buf[0] << 24) | ((u32)buf[1] << 16) | ((u32)buf[2] << 8) | (u32)buf[3];
    
}
float
CSockReadStream::readFloat()
{
    float buf;
    if(!requestData((unsigned char *)&buf, sizeof(float))) { return -6e24f; }
    return buf;
}

/*!
 指定サイズ読み込み失敗時に読み込み位置を更新しないリングバッファ方式のため、
 byteSize にはリングバッファのサイズ以上を指定できない。
 */
bool
CSockReadStream::readBlock(void * buffer, u32 byteSize)
{
    if(!requestData((unsigned char *)buffer, byteSize)) { return false; }
    return true;
}

CSockReadStream::ESTATUS
CSockReadStream::getStatus()
{
    return m_eStat;
}

// この Stream から書き込み(送出)可能なストリームを生成する
IWriteStream *
CSockReadStream::getWriteStream()
{
    return m_writeStream;
}

int
CSockReadStream::readU16arr(u16 *pBufferU16, int items)
{
    // リングバッファに指定サイズを要求するので、全て取得できるか、全く取れないかのどちらかになる。
    if(!requestData((unsigned char *)pBufferU16, items * sizeof(u16))) { return 0; }
    
    // iOS および Android/ARM では元から big endian なのでバイトオーダー入れ替えは発生しない。

    return items;
}

int
CSockReadStream::readU32arr(u32 *pBufferU32, int items)
{
    // リングバッファに指定サイズを要求するので、全て取得できるか、全く取れないかのどちらかになる。
    if(!requestData((unsigned char *)pBufferU32, items * sizeof(u32))) { return 0; }
    
    // iOS および Android/ARM では元から big endian なのでバイトオーダー入れ替えは発生しない。
    
    return items;
}
