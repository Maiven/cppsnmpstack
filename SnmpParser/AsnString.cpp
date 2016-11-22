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

#include "AsnString.h"
#include "SnmpUdp.h"
#include "Log.h"
#include "MemoryDebug.h"

CAsnString::CAsnString()
{
	m_cType = ASN_TYPE_OCTET_STR;
}

CAsnString::CAsnString( uint8_t cType )
{
	m_cType = cType;
}

CAsnString::CAsnString( const char * pszValue )
{
	m_cType = ASN_TYPE_OCTET_STR;

	if( pszValue )
	{
		m_strValue = pszValue;
	}
}

CAsnString::CAsnString( uint8_t cType, const char * pszValue, int iValueLen )
{
	m_cType = cType;

	if( pszValue )
	{
		m_strValue.append( pszValue, iValueLen );
	}
}

CAsnString::~CAsnString()
{
}

/**
 * @ingroup SnmpParser
 * @brief ��Ŷ�� �Ľ��Ͽ��� ���� ������ ��Ŷ �����͸� �����Ѵ�.
 * @param pszPacket		��Ŷ
 * @param iPacketLen	��Ŷ ����
 * @returns �����ϸ� �Ľ��� ��Ŷ ���̸� �����ϰ� �����ϸ� -1 �� �����Ѵ�.
 */
int CAsnString::ParsePacket( const char * pszPacket, int iPacketLen )
{
	int			iPos = 0;

	m_strValue.clear();

	iPos = ParseHeader( pszPacket, iPacketLen );
	if( iPos == -1 ) return -1;

	m_strValue.append( pszPacket + iPos, m_iLen );

	iPos += m_iLen;

	return iPos;
}

/**
 * @ingroup SnmpParser
 * @brief ���� ������ ��Ŷ�� �����Ѵ�.
 * @param pszPacket		��Ŷ
 * @param iPacketSize ��Ŷ ũ��
 * @returns �����ϸ� ����� ��Ŷ ���̸� �����ϰ� �����ϸ� -1 �� �����Ѵ�.
 */
int CAsnString::MakePacket( char * pszPacket, int iPacketSize )
{
	int				iPos = 0;
	uint32_t	iLength = m_strValue.length();

	pszPacket[iPos++] = m_cType;

	if( iLength <= 127 )
	{
		pszPacket[iPos++] = iLength;
	}
	else
	{
		int n = SetInt( (uint8_t *)pszPacket + 1, iPacketSize - 1, iLength );
		if( n == -1 ) return -1;

		iPos += n;
	}

	memcpy( pszPacket + iPos, m_strValue.c_str(), iLength );
	iPos += iLength;

	return iPos;
}

/**
 * @ingroup SnmpParser
 * @brief ������ ��ü�� �����Ѵ�.
 * @returns ������ ��ü�� �����Ѵ�.
 */
CAsnType * CAsnString::Copy( )
{
	CAsnString * pclsValue = new CAsnString();
	if( pclsValue == NULL ) return NULL;

	pclsValue->m_cType = m_cType;
	pclsValue->m_strValue = m_strValue;

	return pclsValue;
}

/**
 * @ingroup SnmpParser
 * @brief ���ڿ��� �����´�.
 * @param strValue ���ڿ��� ������ ����
 * @returns �����ϸ� true �� �����ϰ� �����ϸ� false �� �����Ѵ�.
 */
bool CAsnString::GetString( std::string & strValue )
{
	if( m_cType == ASN_TYPE_IP_ADDRESS )
	{
		if( m_strValue.length() != 4 )
		{
			CLog::Print( LOG_ERROR, "%s value type is not IP", __FUNCTION__ );
			return false;
		}

		uint32_t iValue;
		char szIp[21];

		memcpy( &iValue, m_strValue.c_str(), 4 );
		inet_ntop( AF_INET, &iValue, szIp, sizeof(szIp) );
		strValue = szIp;
	}
	else
	{
	strValue = m_strValue;
	}

	return true;
}
