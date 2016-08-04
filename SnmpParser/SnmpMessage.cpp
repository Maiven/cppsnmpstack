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

#include "SnmpMessage.h"
#include "SnmpAuth.h"
#include "AsnInt.h"
#include "AsnOid.h"
#include "AsnNull.h"
#include "Log.h"
#include "TimeUtility.h"
#include <stdlib.h>
#include "MemoryDebug.h"

#include "SnmpMessageMake.hpp"
#include "SnmpMessageParse.hpp"
#include "SnmpMessagev3.hpp"

CSnmpMessage::CSnmpMessage() : m_cVersion(SNMP_VERSION_2C), m_cCommand(SNMP_CMD_GET)
	, m_iRequestId(0), m_iErrorStatus(0), m_iErrorIndex(0), m_pclsOidValueList(NULL)
	, m_iMsgId(0), m_iMsgMaxSize(0), m_cMsgFlags(0), m_iMsgSecurityModel(0)
	, m_iMsgAuthEngineBoots(0), m_iMsgAuthEngineTime(0)
	, m_pszPacket(NULL), m_iPacketLen(0)
{
}

CSnmpMessage::~CSnmpMessage()
{
	Clear();
}

/**
 * @ingroup SnmpParser
 * @brief ���� ������ �ʱ�ȭ��Ų��.
 */
void CSnmpMessage::Clear()
{
	m_iErrorStatus = 0;
	m_iErrorIndex = 0;

	m_iMsgId = 0;
	m_iMsgMaxSize = SNMP_MAX_PACKET_SIZE;
	m_cMsgFlags = 0;
	m_iMsgSecurityModel = SNMP_SECURITY_MODEL_USM;

	m_iMsgAuthEngineBoots = 0;
	m_iMsgAuthEngineTime = 0;

	m_strMsgAuthEngineId.clear();
	m_strMsgUserName.clear();
	m_strMsgAuthParams.clear();
	m_strMsgPrivParams.clear();

	m_strContextEngineId.clear();
	m_strContextName.clear();

	m_strEncryptedPdu.clear();

	if( m_pclsOidValueList )
	{
		delete m_pclsOidValueList;
		m_pclsOidValueList = NULL;
	}

	if( m_pszPacket )
	{
		free( m_pszPacket );
		m_pszPacket = NULL;
	}
}

/**
 * @ingroup SnmpParser
 * @brief SNMPv2 GET ��û �޽����� �����Ѵ�.
 * @param pszCommunity	community ���ڿ�
 * @param iRequestId		��û ���̵�
 * @param pszOid				OID ���ڿ�
 * @returns �����ϸ� true �� �����ϰ� �����ϸ� false �� �����Ѵ�.
 */
bool CSnmpMessage::MakeGetRequest( const char * pszCommunity, uint32_t iRequestId, const char * pszOid )
{
	if( pszCommunity == NULL || pszOid == NULL ) return false;

	Clear();

	m_cVersion = SNMP_VERSION_2C;
	m_strCommunity = pszCommunity;
	m_cCommand = SNMP_CMD_GET;
	m_iRequestId = iRequestId;

	CAsnNull clsValue;

	if( AddOidValue( pszOid, &clsValue ) == false ) return false;

	return true;
}

/**
 * @ingroup SnmpParser
 * @brief SNMPv2 GET NEXT ��û �޽����� �����Ѵ�.
 * @param pszCommunity	community ���ڿ�
 * @param iRequestId		��û ���̵�
 * @param pszOid				OID ���ڿ�
 * @returns �����ϸ� true �� �����ϰ� �����ϸ� false �� �����Ѵ�.
 */
bool CSnmpMessage::MakeGetNextRequest( const char * pszCommunity, uint32_t iRequestId, const char * pszOid )
{
	if( MakeGetRequest( pszCommunity, iRequestId, pszOid ) == false ) return false;

	m_cCommand = SNMP_CMD_GET_NEXT;

	return true;
}

/**
 * @ingroup SnmpParser
 * @brief SNMPv2 TRAP �޽����� �����Ѵ�. �� �޼ҵ带 ȣ���� ��, AddOidValue �޼ҵ带 ȣ���Ͽ��� TRAP ���� ������ OID �� OID �� ���� �����ؾ� �Ѵ�.
 * @param pszTrapOid		TRAP OID ���ڿ�
 * @returns �����ϸ� true �� �����ϰ� �����ϸ� false �� �����Ѵ�.
 */
bool CSnmpMessage::MakeTrapRequest( const char * pszTrapOid )
{
	Clear();

	m_cVersion = SNMP_VERSION_2C;
	m_cCommand = SNMP_CMD_TRAP;

	AddOidValue( "1.3.6.1.2.1.1.3.0", time(NULL) );
	AddOidValueOid( "1.3.6.1.6.3.1.1.4.1.0", pszTrapOid );

	return true;
}

/**
 * @ingroup SnmpParser
 * @brief SNMPv3 GET ��û �޽����� �����Ѵ�.
 * @param pszUserName	����� ���̵�
 * @param pszAuthPassWord	msgAuthenticationParameters ������ ���� ��й�ȣ
 * @param pszPrivPassWord msgPrivacyParameters ������ ���� ��й�ȣ
 * @param iRequestId	��û ���̵�
 * @param pszOid			OID ���ڿ�
 * @returns �����ϸ� true �� �����ϰ� �����ϸ� false �� �����Ѵ�.
 */
bool CSnmpMessage::MakeGetRequest( const char * pszUserName, const char * pszAuthPassWord, const char * pszPrivPassWord, uint32_t iRequestId, const char * pszOid )
{
	if( pszUserName == NULL || pszOid == NULL ) return false;

	Clear();

	m_cVersion = SNMP_VERSION_3;
	
	m_iMsgId = iRequestId;

	m_cCommand = SNMP_CMD_GET;
	m_iRequestId = iRequestId;

	m_cMsgFlags = SNMP_MSG_FLAG_REPORT;

	// �ι�° SNMP �޽��� ������ ���ؼ� �����Ѵ�.
	m_strReqOid = pszOid;

	if( pszUserName )
	{
		m_strUserId = pszUserName;
	}

	if( pszAuthPassWord )
	{
		m_strAuthPassWord = pszAuthPassWord;
	}

	if( pszPrivPassWord )
	{
		m_strPrivPassWord = pszPrivPassWord;
	}

	CAsnNull clsValue;

	if( AddOidValue( pszOid, &clsValue ) == false ) return false;

	return true;
}

/**
 * @ingroup SnmpParser
 * @brief SNMPv3 GET ��û �޽����� �����Ѵ�.
 * @param pszUserName	����� ���̵�
 * @param pszAuthPassWord	msgAuthenticationParameters ������ ���� ��й�ȣ
 * @param pszPrivPassWord msgPrivacyParameters ������ ���� ��й�ȣ
 * @param iRequestId	��û ���̵�
 * @param pszOid			OID ���ڿ�
 * @returns �����ϸ� true �� �����ϰ� �����ϸ� false �� �����Ѵ�.
 */
bool CSnmpMessage::MakeGetNextRequest( const char * pszUserName, const char * pszAuthPassWord, const char * pszPrivPassWord, uint32_t iRequestId, const char * pszOid )
{
	if( MakeGetRequest( pszUserName, pszAuthPassWord, pszPrivPassWord, iRequestId, pszOid ) == false ) return false;

	m_cCommand = SNMP_CMD_GET_NEXT;

	return true;
}

/**
 * @ingroup SnmpParser
 * @brief ù��° OID �� �����´�.
 * @returns ù��° OID �� �����ϸ� ù��° OID �� �����ϰ� �׷��� ������ NULL �� �����Ѵ�.
 */
const char * CSnmpMessage::GetOid()
{
	if( m_pclsOidValueList == NULL ) return NULL;

	CSnmpOidValue * pclsOidValue = m_pclsOidValueList->GetFirst();
	if( pclsOidValue == NULL ) return NULL;

	return pclsOidValue->m_strOid.c_str();
}

/**
 * @ingroup SnmpParser
 * @brief OID �� OID �� ���� �����Ѵ�.
 * @param pszOid		OID
 * @param pclsValue OID �� ��
 * @returns �����ϸ� true �� �����ϰ� �׷��� ������ false �� �����Ѵ�.
 */
bool CSnmpMessage::AddOidValue( const char * pszOid, CAsnType * pclsValue )
{
	if( m_pclsOidValueList == NULL )
	{
		m_pclsOidValueList = new CSnmpOidValueList();
		if( m_pclsOidValueList == NULL ) return false;
	}

	m_pclsOidValueList->Add( pszOid, pclsValue );

	return true;
}

/**
 * @ingroup SnmpParser
 * @brief OID �� ������ �����Ѵ�.
 * @param pszOid	OID
 * @param iValue	���� ( OID �� �� )
 * @returns �����ϸ� true �� �����ϰ� �׷��� ������ false �� �����Ѵ�.
 */
bool CSnmpMessage::AddOidValue( const char * pszOid, uint32_t iValue )
{
	CAsnInt clsValue( ASN_TYPE_INT, iValue );
	
	return AddOidValue( pszOid, &clsValue );
}

/**
 * @ingroup SnmpParser
 * @brief OID �� ������ �����Ѵ�.
 * @param pszOid	OID
 * @param iValue	���� ( OID �� �� )
 * @returns �����ϸ� true �� �����ϰ� �׷��� ������ false �� �����Ѵ�.
 */
bool CSnmpMessage::AddOidValue( const char * pszOid, int32_t iValue )
{
	CAsnInt clsValue( ASN_TYPE_TIMETICKS, (uint32_t)iValue );
	
	return AddOidValue( pszOid, &clsValue );
}

/**
 * @ingroup SnmpParser
 * @brief OID �� �ð��� �����Ѵ�.
 * @param pszOid	OID
 * @param iValue	�ð� ( OID �� �� )
 * @returns �����ϸ� true �� �����ϰ� �׷��� ������ false �� �����Ѵ�.
 */
bool CSnmpMessage::AddOidValue( const char * pszOid, time_t iValue )
{
	CAsnInt clsValue( ASN_TYPE_TIMETICKS, (uint32_t)iValue );
	
	return AddOidValue( pszOid, &clsValue );
}

/**
 * @ingroup SnmpParser
 * @brief OID �� ���ڿ��� �����Ѵ�.
 * @param pszOid		OID
 * @param pszValue	���ڿ� ( OID �� �� )
 * @returns �����ϸ� true �� �����ϰ� �׷��� ������ false �� �����Ѵ�.
 */
bool CSnmpMessage::AddOidValue( const char * pszOid, const char * pszValue )
{
	CAsnString clsValue( pszValue );

	return AddOidValue( pszOid, &clsValue );
}

/**
 * @ingroup SnmpParser
 * @brief OID �� OID �� �����Ѵ�.
 * @param pszOid		OID
 * @param pszValue	OID ( OID �� �� )
 * @returns �����ϸ� true �� �����ϰ� �׷��� ������ false �� �����Ѵ�.
 */
bool CSnmpMessage::AddOidValueOid( const char * pszOid, const char * pszValue )
{
	CAsnOid clsValue( pszValue );

	return AddOidValue( pszOid, &clsValue );
}

/**
 * @ingroup SnmpParser
 * @brief OID �� ī��Ʈ�� �����Ѵ�.
 * @param pszOid OID
 * @param iValue ���� ( OID �� �� )
 * @returns �����ϸ� true �� �����ϰ� �׷��� ������ false �� �����Ѵ�.
 */
bool CSnmpMessage::AddOidValueCounter( const char * pszOid, uint32_t iValue )
{
	CAsnInt clsValue( ASN_TYPE_COUNTER_32, (uint32_t)iValue );
	
	return AddOidValue( pszOid, &clsValue );
}

/**
 * @ingroup SnmpParser
 * @brief SNMP ���� �޽����� �����Ѵ�.
 * @returns SNMP ���� �޽����� �����Ѵ�.
 */
CSnmpMessage * CSnmpMessage::CreateResponse()
{
	CSnmpMessage * pclsResponse = new CSnmpMessage();
	if( pclsResponse == NULL ) return NULL;
	
	pclsResponse->m_cVersion = m_cVersion;
	pclsResponse->m_strCommunity = m_strCommunity;
	pclsResponse->m_cCommand = SNMP_CMD_RESPONSE;
	pclsResponse->m_iRequestId = m_iRequestId;
	pclsResponse->m_iMsgId = m_iMsgId;

	if( m_cVersion == SNMP_VERSION_3 )
	{
		pclsResponse->m_iMsgSecurityModel = m_iMsgSecurityModel;

		pclsResponse->m_strMsgAuthEngineId = m_strMsgAuthEngineId;
		pclsResponse->m_strContextEngineId = m_strContextEngineId;
		pclsResponse->m_iMsgMaxSize = m_iMsgMaxSize;
		pclsResponse->m_iMsgAuthEngineBoots = m_iMsgAuthEngineBoots;
		pclsResponse->m_iMsgAuthEngineTime = m_iMsgAuthEngineTime;
	}

	return pclsResponse;
}

/**
 * @ingroup SnmpParser
 * @brief SNMP �޽����� ������ ��, �Էµ� SNMP �޽����� �����Ѵ�.
 * @param pclsMessage SNMP �޽���
 * @returns �����ϸ� ����� SNMP �޽����� �����ϰ� �׷��� ������ NULL �� �����Ѵ�.
 */
CSnmpMessage * CSnmpMessage::Create( CSnmpMessage * pclsMessage )
{
	CSnmpMessage * pclsCopy = new CSnmpMessage();
	if( pclsCopy == NULL ) return NULL;

	*pclsCopy = *pclsMessage;

	if( pclsMessage->m_pclsOidValueList )
	{
		pclsCopy->m_pclsOidValueList = pclsMessage->m_pclsOidValueList->Copy();
	}

	pclsCopy->m_pszPacket = NULL;
	pclsCopy->m_iPacketLen = 0;

	return pclsCopy;
}
