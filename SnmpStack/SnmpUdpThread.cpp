/* 
 * Copyright (C) 2012 Yee Young Han <websearch@naver.com> (http://blog.naver.com/websearch)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA 
 */

#include "SipPlatformDefine.h"
#include "AutoRelease.h"
#include "SnmpThread.h"
#include "AsnNull.h"
#include "ServerUtility.h"
#include "Log.h"
#include "MemoryDebug.h"

static bool gbStop = false;
static bool gbRun = false;

/**
 * @ingroup SnmpStack
 * @brief SNMP 응답 메시지 수신에 대한 callback 처리를 한다.
 * @param pclsSnmpStack SNMP stack 객체
 * @param pszPacket			패킷
 * @param iPacketSize		패킷 길이
 * @param pszIp					발신자 IP 주소
 * @param sPort					발신자 포트 번호
 */
static void SnmpRecvProcess( CSnmpStack * pclsSnmpStack, char * pszPacket, int iPacketSize, char * pszIp, uint16_t sPort )
{
	CSnmpMessage clsMessage;
	uint32_t iRequestId;

	if( clsMessage.ParsePacket( pszPacket, iPacketSize ) == -1 )
	{
		CLog::Print( LOG_ERROR, "%s clsMessage.ParsePacket() error", __FUNCTION__ );
		return;
	}

	if( clsMessage.m_cVersion == SNMP_VERSION_3 )
	{
		// SNMPv3 에서 PDU 가 암호화되면 m_iRequestId 가 아직 저장되어 있지 않으므로 m_iMsgId 를 사용한다.
		iRequestId = clsMessage.m_iMsgId;
	}
	else
	{
		iRequestId = clsMessage.m_iRequestId;
	}

	{
		CAutoRelease< CSnmpTransactionList, CSnmpTransaction > clsData( pclsSnmpStack->m_clsTransactionList );

		if( pclsSnmpStack->m_clsTransactionList.Select( iRequestId, &clsData.m_pclsData ) )
		{
			if( clsData.m_pclsData->m_pclsRequest->m_cMsgFlags == SNMP_MSG_FLAG_REPORT )
			{
				CSnmpMessage * pclsRequest = CSnmpMessage::Create( clsData.m_pclsData->m_pclsRequest );
				if( pclsRequest )
				{
					pclsRequest->m_iMsgId = pclsSnmpStack->GetNextRequestId();
					pclsRequest->m_iRequestId = pclsRequest->m_iMsgId;
					pclsRequest->m_strMsgAuthEngineId = clsMessage.m_strMsgAuthEngineId;
					pclsRequest->m_iMsgAuthEngineBoots = clsMessage.m_iMsgAuthEngineBoots;
					pclsRequest->m_iMsgAuthEngineTime = clsMessage.m_iMsgAuthEngineTime;
					pclsRequest->m_strMsgUserName = pclsRequest->m_strUserId;
					pclsRequest->m_strContextEngineId = clsMessage.m_strContextEngineId;

					pclsRequest->SetPrivParams( );
					pclsRequest->SetAuthParams( );

					pclsSnmpStack->SendRequest( pclsRequest );
				}
			}
			else
			{
				clsMessage.m_strAuthPassWord = clsData.m_pclsData->m_pclsRequest->m_strAuthPassWord;

				if( clsMessage.CheckAuth() == false )
				{
					pclsSnmpStack->m_pclsCallBack->RecvResponse( clsData.m_pclsData->m_pclsRequest, NULL );
				}
				else
				{
					if( clsMessage.m_strEncryptedPdu.empty() == false )
					{
						clsMessage.m_strPrivPassWord = clsData.m_pclsData->m_pclsRequest->m_strPrivPassWord;
						clsMessage.ParseEncryptedPdu( );
					}

					pclsSnmpStack->m_pclsCallBack->RecvResponse( clsData.m_pclsData->m_pclsRequest, &clsMessage );
				}
			}
		}
	}

	pclsSnmpStack->m_clsTransactionList.Delete( &clsMessage );
}

/**
 * @ingroup SnmpStack
 * @brief SNMP 메시지 수신 쓰레드
 * @param lpParameter SNMP stack 객체
 * @returns 0 을 리턴한다.
 */
THREAD_API SnmpUdpThread( LPVOID lpParameter )
{
	CSnmpStack * pclsSnmpStack = (CSnmpStack *)lpParameter;
	struct pollfd arrPoll[1];
	int		n, iPacketSize;
	char	szPacket[SNMP_MAX_PACKET_SIZE], szIp[16];
	uint16_t sPort;

	gbRun = true;

	TcpSetPollIn( arrPoll[0], pclsSnmpStack->m_hSocket );

	while( gbStop == false )
	{
		n = poll( arrPoll, 1, 1000 );
		if( n > 0 )
		{
			iPacketSize = sizeof(szPacket);
			if( UdpRecv( pclsSnmpStack->m_hSocket, szPacket, &iPacketSize, szIp, sizeof(szIp), &sPort ) )
			{
				SnmpRecvProcess( pclsSnmpStack, szPacket, iPacketSize, szIp, sPort );
			}
		}
	}

	gbRun = false;

	return 0;
}

/**
 * @ingroup SnmpStack
 * @brief SNMP 메시지 수신 쓰레드를 시작한다.
 * @param pclsSnmpStack SNMP stack 객체
 * @returns 성공하면 true 를 리턴하고 그렇지 않으면 false 를 리턴한다.
 */
bool StartSnmpUdpThread( CSnmpStack * pclsSnmpStack )
{
	gbStop = false;

	return StartThread( "SnmpUdpThread", SnmpUdpThread, pclsSnmpStack );
}

/**
 * @ingroup SnmpStack
 * @brief SNMP 메시지 수신 쓰레드를 중지한다.
 */
void StopSnmpUdpThread( )
{
	gbStop = true;
}

/**
 * @ingroup SnmpStack
 * @brief SNMP 메시지 수신 쓰레드가 실행 중인지 검사한다.
 * @returns SNMP 메시지 수신 쓰레드가 실행 중이면 true 를 리턴하고 그렇지 않으면 false 를 리턴한다.
 */
bool IsSnmpUdpThreadRun( )
{
	return gbRun;
}
