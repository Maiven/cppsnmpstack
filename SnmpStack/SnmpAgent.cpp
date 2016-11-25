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
#include "SnmpAgent.h"
#include "Log.h"
#include "TimeUtility.h"
#include "AsnInt.h"
#include "MemoryDebug.h"

CSnmpAgent::CSnmpAgent() : m_hSocket(INVALID_SOCKET), m_iDestIp(0), m_sDestPort(0), m_cMsgFlags(0)
{
}

CSnmpAgent::~CSnmpAgent()
{
}

/**
 * @ingroup SnmpStack
 * @brief SNMP ��Ŷ ������ ���� UDP ��Ʈ�� ����.
 * @param iUdpPort SNMP ��Ŷ ������ ���� UDP ��Ʈ ��ȣ
 * @param pszCommunity		SNMPv2 community ���ڿ�
 * @param pszUserName			SNMPv3 ����� ���̵�
 * @param pszAuthPassWord SNMPv3 ���� ��й�ȣ
 * @param pszPrivPassWord SNMPv3 ��ȣȭ ��й�ȣ
 * @returns �����ϸ� true �� �����ϰ� �����ϸ� false �� �����Ѵ�.
 */
bool CSnmpAgent::Open( int iUdpPort, const char * pszCommunity, const char * pszUserName, const char * pszAuthPassWord, const char * pszPrivPassWord )
{
	if( pszCommunity == NULL && pszUserName == NULL )
	{
		CLog::Print( LOG_ERROR, "%s pszCommunity and pszUserName is NULL", __FUNCTION__ );
		return false;
	}

	if( m_hSocket != INVALID_SOCKET )
	{
		CLog::Print( LOG_ERROR, "%s already open", __FUNCTION__ );
		return false;
	}

	if( pszCommunity ) m_strCommunity = pszCommunity;
	if( pszUserName ) m_strUserName = pszUserName;
	if( pszAuthPassWord ) m_strAuthPassWord = pszAuthPassWord;
	if( pszPrivPassWord ) m_strPrivPassWord = pszPrivPassWord;

	m_hSocket = UdpListen( iUdpPort, NULL );
	if( m_hSocket == INVALID_SOCKET )
	{
		CLog::Print( LOG_ERROR, "%s UdpListen(%d) error(%d)", __FUNCTION__, iUdpPort, GetError() );
		return false;
	}

	return true;
}

/**
 * @ingroup SnmpStack
 * @brief SNMP ��Ŷ ������ ���� UDP ��Ʈ�� �ݴ´�.
 */
void CSnmpAgent::Close()
{
	if( m_hSocket != INVALID_SOCKET )
	{
		closesocket( m_hSocket );
		m_hSocket = INVALID_SOCKET;
	}
}

/**
 * @ingroup SnmpStack
 * @brief SNMP ��û �޽��� ���� ��� & �����Ѵ�.
 * @param pclsRequest	���ŵ� SNMP ��û �޽����� ������ ����
 * @param iTimeout		��� �ð� ( ms ���� )
 * @returns �����ϸ� true �� �����ϰ� �����ϸ� false �� �����Ѵ�.
 */
bool CSnmpAgent::RecvRequest( CSnmpMessage * pclsRequest, int iTimeout )
{
	char	szPacket[SNMP_MAX_PACKET_SIZE];
	bool	bRes = false;
	struct pollfd arrPoll[1];

	if( pclsRequest == NULL ) return false;

	pclsRequest->Clear();

	TcpSetPollIn( arrPoll[0], m_hSocket );

	while( 1 )
	{
		int n = poll( arrPoll, 1, iTimeout );
		if( n > 0 )
		{
			int		iPacketLen = sizeof(szPacket);
			
			if( UdpRecv( m_hSocket, szPacket, &iPacketLen, &m_iDestIp, &m_sDestPort ) )
			{
				if( pclsRequest->ParsePacket( szPacket, iPacketLen ) != -1 )
				{
					if( pclsRequest->m_cVersion == SNMP_VERSION_3 )
					{
						if( m_strUserName.empty() )
						{
							// SNMPv3 �� �������� �ʴ´�.
							continue;
						}

						if( pclsRequest->m_cMsgFlags & SNMP_MSG_FLAG_ENCRYPT )
						{
							if( m_strPrivPassWord.empty() )
							{
								// ��ȣȭ ��й�ȣ�� �������� �����Ƿ� �������� �ʴ´�.
								continue;
							}
						}

						if( pclsRequest->m_cMsgFlags & SNMP_MSG_FLAG_AUTH )
						{
							if( m_strAuthPassWord.empty() )
							{
								// ���� ��й�ȣ�� �������� �����Ƿ� �������� �ʴ´�.
								continue;
							}
						}

						if( pclsRequest->m_iMsgAuthEngineBoots == 0 && pclsRequest->m_strMsgAuthParams.empty() )
						{
							CSnmpMessage * pclsResponse = pclsRequest->CreateResponse();
							if( pclsResponse )
							{
								time_t iTime;

								time( &iTime );

								static uint8_t szEngineId[] = { 0x80, 0x00, 0x00, 0x04, 0x80, 0x00 };

								pclsResponse->m_cMsgFlags = 0;
								pclsResponse->m_strMsgAuthEngineId.append( (char *)szEngineId, sizeof(szEngineId) );
								pclsResponse->m_strContextEngineId = pclsResponse->m_strMsgAuthEngineId;
								pclsResponse->m_iMsgMaxSize = SNMP_MAX_MSG_SIZE;
								pclsResponse->m_iMsgAuthEngineBoots = (uint32_t)iTime;
								pclsResponse->m_iMsgAuthEngineTime = pclsResponse->m_iMsgAuthEngineBoots;
								pclsResponse->m_cCommand = SNMP_CMD_REPORT;

								pclsResponse->AddOidValueCounter( "1.3.6.1.6.3.15.1.1.4.0", 10 );

								iPacketLen = pclsResponse->MakePacket( szPacket, sizeof(szPacket) );
								if( iPacketLen != -1 )
								{
									UdpSend( m_hSocket, szPacket, iPacketLen, m_iDestIp, m_sDestPort );
								}

								delete pclsResponse;
							}

							continue;
						}

						if( strcmp( pclsRequest->m_strMsgUserName.c_str(), m_strUserName.c_str() ) )
						{
							CLog::Print( LOG_ERROR, "%s userId(%s) != userName(%s)", __FUNCTION__, pclsRequest->m_strMsgUserName.c_str(), m_strUserName.c_str() );
							continue;
						}

						pclsRequest->m_strUserId = m_strUserName;
						pclsRequest->m_strAuthPassWord = m_strAuthPassWord;
						pclsRequest->m_strPrivPassWord = m_strPrivPassWord;

						if( pclsRequest->m_strEncryptedPdu.empty() == false )
						{
							pclsRequest->ParseEncryptedPdu( );
						}

						if( pclsRequest->CheckAuth() == false )
						{
							continue;
						}

						m_cMsgFlags = pclsRequest->m_cMsgFlags;
					}
					else
					{
						if( strcmp( pclsRequest->m_strCommunity.c_str(), m_strCommunity.c_str() ) )
						{
							CLog::Print( LOG_ERROR, "%s community(%s) != setup community(%s)", __FUNCTION__, pclsRequest->m_strCommunity.c_str(), m_strCommunity.c_str() );
							continue;
						}
					}

					bRes = true;
				}
			}
		}

		break;
	}

	return bRes;
}

/**
 * @ingroup SnmpStack
 * @brief SNMP ���� �޽����� �����Ѵ�.
 * @param pclsResponse SNMP ���� �޽���
 * @returns �����ϸ� true �� �����ϰ� �����ϸ� false �� �����Ѵ�.
 */
bool CSnmpAgent::SendResponse( CSnmpMessage * pclsResponse )
{
	char	szPacket[SNMP_MAX_PACKET_SIZE];
	int		iPacketLen;

	if( pclsResponse == NULL ) return false;

	if( m_strUserName.empty() == false )
	{
		pclsResponse->m_strMsgUserName = m_strUserName;
		pclsResponse->m_strAuthPassWord = m_strAuthPassWord;
		pclsResponse->m_strPrivPassWord = m_strPrivPassWord;

		if( m_cMsgFlags & SNMP_MSG_FLAG_ENCRYPT )
		{
			pclsResponse->SetPrivParams( );
		}

		if( m_cMsgFlags & SNMP_MSG_FLAG_AUTH )
		{
			pclsResponse->SetAuthParams( );
		}
	}

	iPacketLen = pclsResponse->MakePacket( szPacket, sizeof(szPacket) );
	if( iPacketLen == -1 ) return false;

	UdpSend( m_hSocket, szPacket, iPacketLen, m_iDestIp, m_sDestPort );
	
	return true;
}
