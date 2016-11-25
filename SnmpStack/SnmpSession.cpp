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
#include "SnmpSession.h"
#include "AsnNull.h"
#include "Log.h"
#include "SnmpDebug.h"
#include "StringUtility.h"
#include "ServerUtility.h"
#include "MemoryDebug.h"

#include "SnmpSessionComm.hpp"
#include "SnmpSessionThread.hpp"

CSnmpSession::CSnmpSession() : m_bStop(false), m_hSocket(INVALID_SOCKET)
	, m_iPort(161), m_iIp(0), m_sPort(0)
	, m_iMiliTimeout(1000), m_iReSendCount(5), m_iRequestId(0)
	, m_bTcp(false), m_bTrapOnly(false)
	, m_bDebug(false)
{
}

CSnmpSession::~CSnmpSession()
{
	Close();
}

/**
 * @ingroup SnmpStack
 * @brief SNMP ��û �޽����� ������ SNMP Agent �ּҸ� �����Ѵ�.
 * @param pszIp	IP �ּ�
 * @param iPort ��Ʈ ��ȣ
 * @param bTcp	TCP ��� ����
 * @param bTrapOnly	TRAP �޽����� �����ϴ°�?
 * @returns �����ϸ� true �� �����ϰ� �׷��� ������ false �� �����Ѵ�.
 */
bool CSnmpSession::SetDestination( const char * pszIp, int iPort, bool bTcp, bool bTrapOnly )
{
	if( pszIp == NULL || strlen(pszIp) == 0 ) return false;
	if( iPort <= 0 || iPort > 65535 ) return false;

	m_strIp = pszIp;
	m_iPort = iPort;
	m_bTcp = bTcp;
	m_bTrapOnly = bTrapOnly;

	m_iIp = inet_addr( pszIp );
	m_sPort = htons( m_iPort );

	if( m_bTcp )
	{
		m_iMiliTimeout = 5000;
	}

	return true;
}

/**
 * @ingroup SnmpStack
 * @brief SNMPv2 �� ���� SNMP community ���ڿ��� �����Ѵ�.
 * @param pszCommunity SNMP community ���ڿ�
 * @returns �����ϸ� true �� �����ϰ� �׷��� ������ false �� �����Ѵ�.
 */
bool CSnmpSession::SetSnmpv2( const char * pszCommunity )
{
	if( pszCommunity == NULL ) return false;

	m_strCommunity = pszCommunity;

	return true;
}

/**
 * @ingroup SnmpStack
 * @brief SNMPv3 �� ���� ����� ���̵� �� ��й�ȣ�� �����Ѵ�.
 * @param pszUserName			����� ���̵�
 * @param pszAuthPassWord ���� ��й�ȣ
 * @param pszPrivPassWord ��ȣȭ ��й�ȣ
 * @param pszAuthEngineId	���� ���̵� ( SNMP TRAP ���۽ÿ� �Է��Ѵ�. )
 * @returns �����ϸ� true �� �����ϰ� �׷��� ������ false �� �����Ѵ�.
 */
bool CSnmpSession::SetSnmpv3( const char * pszUserName, const char * pszAuthPassWord, const char * pszPrivPassWord, const char * pszAuthEngineId )
{
	if( pszUserName == NULL ) return false;
	if( pszAuthPassWord == NULL ) return false;

	m_strUserName = pszUserName;
	m_strAuthPassWord = pszAuthPassWord;

	if( pszPrivPassWord )
	{
		m_strPrivPassWord = pszPrivPassWord;
	}

	if( pszAuthEngineId )
	{
		HexToString( pszAuthEngineId, m_strAuthEngineId );
	}

	return true;
}

/**
 * @ingroup SnmpStack
 * @brief �������� ���� ���� ��� timeout �ð��� �����Ѵ�.
 * @param iMiliSecond milisecond timeout �ð�
 * @returns �����ϸ� true �� �����ϰ� �׷��� ������ false �� �����Ѵ�.
 */
bool CSnmpSession::SetTimeout( int iMiliSecond )
{
	if( iMiliSecond <= 0 ) return false;

	m_iMiliTimeout = iMiliSecond;

	return true;
}

/**
 * @ingroup SnmpStack
 * @brief timeout �Ǿ��� ���� �������ϴ� ������ �����Ѵ�.
 * @param iReSendCount timeout �Ǿ��� ���� �������ϴ� ����
 * @returns �����ϸ� true �� �����ϰ� �׷��� ������ false �� �����Ѵ�.
 */
bool CSnmpSession::SetReSendCount( int iReSendCount )
{
	if( iReSendCount < 0 ) return false;

	m_iReSendCount = iReSendCount;

	return true;
}

/**
 * @ingroup SnmpStack
 * @brief SNMP ��� ����� ��带 �����Ѵ�.
 * @param bDebug ����� ���
 */
void CSnmpSession::SetDebug( bool bDebug )
{
	m_bDebug = bDebug;
}

/**
 * @ingroup SnmpStack
 * @brief SNMP ����� ���� UDP ������ �����Ѵ�.
 * @returns �����ϸ� true �� �����ϰ� �׷��� ������ false �� �����Ѵ�.
 */
bool CSnmpSession::Open()
{
	if( m_hSocket != INVALID_SOCKET ) return false;

	m_bStop = false;

	if( m_bTcp )
	{
		if( m_strIp.empty() && m_iPort <= 0 )
		{
			CLog::Print( LOG_ERROR, "%s m_strIp(%s) or m_iPort(%d) is not correct", __FUNCTION__, m_strIp.c_str(), m_iPort );
			return false;
		}

		m_hSocket = TcpConnect( m_strIp.c_str(), m_iPort, m_iMiliTimeout / 1000 );
		if( m_hSocket == INVALID_SOCKET )
		{
			CLog::Print( LOG_ERROR, "%s TcpConnect(%s:%d) error(%d)", __FUNCTION__, m_strIp.c_str(), m_iPort, GetError() );
			return false;
		}

		if( m_bTrapOnly )
		{
			if( StartThread( "SnmpSessionThread", SnmpSessionThread, this ) == false )
			{
				CLog::Print( LOG_ERROR, "%s SnmpSessionThread start error(%d)", __FUNCTION__, GetError() );
			}
		}
	}
	else
	{
	m_hSocket = UdpSocket();
		if( m_hSocket == INVALID_SOCKET )
		{
			CLog::Print( LOG_ERROR, "%s UdpSocket() error(%d)", __FUNCTION__, GetError() );
			return false;
		}
	}

	return true;
}

/**
 * @ingroup SnmpStack
 * @brief SNMP ����� ���� UDP ������ �����Ѵ�.
 */
void CSnmpSession::Close()
{
	m_bStop = true;

	if( m_hSocket != INVALID_SOCKET )
	{
		closesocket( m_hSocket );
		m_hSocket = INVALID_SOCKET;
	}
}

/**
 * @ingroup SnmpStack
 * @brief ������ ��ȿ���� �˻��Ѵ�.
 * @returns ������ ��ȿ�ϸ� true �� �����ϰ� �׷��� ������ false �� �����Ѵ�.
 */
bool CSnmpSession::Check()
{
	if( m_strIp.empty() )
	{
		CLog::Print( LOG_ERROR, "%s ip is not defined", __FUNCTION__ );
		return false;
	}

	if( m_hSocket == INVALID_SOCKET )
	{
		CLog::Print( LOG_ERROR, "%s socket is invalid", __FUNCTION__ );
		return false;
	}

	return true;
}

/**
 * @ingroup SnmpStack
 * @brief destination IP �ּҸ� �����Ѵ�.
 * @returns destination IP �ּҸ� �����Ѵ�.
 */
const char * CSnmpSession::GetIp()
{
	return m_strIp.c_str();
}

/**
 * @ingroup SnmpStack
 * @brief SNMP GET �޽����� �����Ͽ��� �̿� ���� ������ �����Ѵ�.
 * @param pszOid				MIB
 * @param ppclsAsnType	���� �޽����� ASN Ÿ�� ���� ����
 * @returns �����ϸ� true �� �����ϰ� �׷��� ������ false �� �����Ѵ�.
 */
bool CSnmpSession::SendGetRequest( const char * pszOid, CAsnType ** ppclsAsnType )
{
	CSnmpMessage clsRequest;
	uint32_t iRequestId = ++m_iRequestId;

	if( m_strUserName.empty() )
	{
		// SNMPv2
		if( clsRequest.MakeGetRequest( m_strCommunity.c_str(), iRequestId, pszOid ) == false )
		{
			CLog::Print( LOG_ERROR, "%s MakeGetRequest SNMPv2 error", __FUNCTION__ );
			return false;
		}
	}
	else
	{
		// SNMPv3
		if( clsRequest.MakeGetRequest( m_strUserName.c_str(), m_strAuthPassWord.c_str(), m_strPrivPassWord.c_str(), iRequestId, pszOid ) == false )
		{
			CLog::Print( LOG_ERROR, "%s MakeGetRequest SNMPv2 error", __FUNCTION__ );
			return false;
		}
	}

	if( SendRequest( &clsRequest, &m_clsResponse ) == false ) return false;
	if( m_clsResponse.m_pclsOidValueList == NULL ) return false;

	CSnmpOidValue * pclsOidValue = m_clsResponse.m_pclsOidValueList->GetFirst();
	if( pclsOidValue == NULL ) return false;

	*ppclsAsnType = pclsOidValue->m_pclsValue;

	return true;
}

/**
 * @ingroup SnmpStack
 * @brief SNMP GET NEXT �޽����� �����Ͽ��� �̿� ���� ������ �����Ѵ�.
 * @param pszOid						MIB
 * @param ppstrResponseOid	���� �޽����� OID ���� ����
 * @param ppclsAsnType			���� �޽����� ASN Ÿ�� ���� ����
 * @returns �����ϸ� true �� �����ϰ� �׷��� ������ false �� �����Ѵ�.
 */
bool CSnmpSession::SendGetNextRequest( const char * pszOid, std::string ** ppstrResponseOid, CAsnType ** ppclsAsnType )
{
	CSnmpMessage clsRequest;
	uint32_t iRequestId = ++m_iRequestId;

	if( m_strUserName.empty() )
	{
		// SNMPv2
		if( clsRequest.MakeGetNextRequest( m_strCommunity.c_str(), iRequestId, pszOid ) == false )
		{
			CLog::Print( LOG_ERROR, "%s MakeGetRequest SNMPv2 error", __FUNCTION__ );
			return false;
		}
	}
	else
	{
		// SNMPv3
		if( clsRequest.MakeGetNextRequest( m_strUserName.c_str(), m_strAuthPassWord.c_str(), m_strPrivPassWord.c_str(), iRequestId, pszOid ) == false )
		{
			CLog::Print( LOG_ERROR, "%s MakeGetRequest SNMPv2 error", __FUNCTION__ );
			return false;
		}
	}

	if( SendRequest( &clsRequest, &m_clsResponse ) == false ) return false;
	if( m_clsResponse.m_pclsOidValueList == NULL ) return false;

	CSnmpOidValue * pclsOidValue = m_clsResponse.m_pclsOidValueList->GetFirst();
	if( pclsOidValue == NULL ) return false;

	*ppstrResponseOid = &pclsOidValue->m_strOid;
	*ppclsAsnType = pclsOidValue->m_pclsValue;

	return true;
}

/**
 * @ingroup SnmpStack
 * @brief SNMP GET �޽����� �����Ͽ��� �̿� ���� ������ �����Ѵ�.
 * @param pszOid		MIB
 * @param strValue	���� �� ���� ����
 * @returns �����ϸ� true �� �����ϰ� �׷��� ������ false �� �����Ѵ�.
 */
bool CSnmpSession::SendGetRequest( const char * pszOid, std::string & strValue )
{
	CAsnType * pclsType;

	if( SendGetRequest( pszOid, &pclsType ) == false )
	{
		CLog::Print( LOG_ERROR, "%s(%s) ip(%s) error", __FUNCTION__, pszOid, m_strIp.c_str() );
		return false;
	}

	if( pclsType->GetString( strValue ) == false )
	{
		CLog::Print( LOG_ERROR, "%s(%s) ip(%s) value type is not string", __FUNCTION__, pszOid, m_strIp.c_str() );
		return false;
	}

	return true;
}

/**
 * @ingroup SnmpStack
 * @brief SNMP GET �޽����� �����Ͽ��� �̿� ���� ������ �����Ѵ�.
 * @param pszOid	MIB
 * @param iValue	���� �� ���� ����
 * @returns �����ϸ� true �� �����ϰ� �׷��� ������ false �� �����Ѵ�.
 */
bool CSnmpSession::SendGetRequest( const char * pszOid, uint32_t & iValue )
{
	CAsnType * pclsType;

	if( SendGetRequest( pszOid, &pclsType ) == false )
	{
		CLog::Print( LOG_ERROR, "%s(%s) ip(%s) error", __FUNCTION__, pszOid, m_strIp.c_str() );
		return false;
	}

	if( pclsType->GetInt( iValue ) == false )
	{
		CLog::Print( LOG_ERROR, "%s(%s) ip(%s) value type is not int", __FUNCTION__, pszOid, m_strIp.c_str() );
		return false;
	}

	return true;
}

/**
 * @ingroup SnmpStack
 * @brief SNMP GET NEXT �޽����� �����Ͽ��� �̿� ���� ������ �����Ѵ�.
 * @param pszOid						MIB
 * @param ppstrResponseOid	���� �޽����� OID ���� ����
 * @param strValue						���� �� ���� ����
 * @returns �����ϸ� true �� �����ϰ� �׷��� ������ false �� �����Ѵ�.
 */
bool CSnmpSession::SendGetNextRequest( const char * pszOid, std::string ** ppstrResponseOid, std::string & strValue )
{
	CAsnType * pclsType;

	if( SendGetNextRequest( pszOid, ppstrResponseOid, &pclsType ) == false )
	{
		CLog::Print( LOG_ERROR, "%s(%s) ip(%s) error", __FUNCTION__, pszOid, m_strIp.c_str() );
		return false;
	}

	if( pclsType->GetString( strValue ) == false )
	{
		// GET NEXT �̹Ƿ� �� Ÿ���� Ʋ�� �� �ִ�.
		strValue.clear();
	}

	return true;
}

/**
 * @ingroup SnmpStack
 * @brief SNMP GET NEXT �޽����� �����Ͽ��� �̿� ���� ������ �����Ѵ�.
 * @param pszOid						MIB
 * @param ppstrResponseOid	���� �޽����� OID ���� ����
 * @param iValue						���� �� ���� ����
 * @returns �����ϸ� true �� �����ϰ� �׷��� ������ false �� �����Ѵ�.
 */
bool CSnmpSession::SendGetNextRequest( const char * pszOid, std::string ** ppstrResponseOid, uint32_t & iValue )
{
	CAsnType * pclsType;

	if( SendGetNextRequest( pszOid, ppstrResponseOid, &pclsType ) == false )
	{
		CLog::Print( LOG_ERROR, "%s(%s) ip(%s) error", __FUNCTION__, pszOid, m_strIp.c_str() );
		return false;
	}

	if( pclsType->GetInt( iValue ) == false )
	{
		// GET NEXT �̹Ƿ� �� Ÿ���� Ʋ�� �� �ִ�.
		iValue = 0;
	}

	return true;
}
