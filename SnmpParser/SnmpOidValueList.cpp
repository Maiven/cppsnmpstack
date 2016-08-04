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

#include "SnmpOidValueList.h"
#include "AsnString.h"

CSnmpOidValue::CSnmpOidValue() : m_pclsValue(NULL)
{
}

CSnmpOidValue::~CSnmpOidValue()
{
	Clear();
}

/**
 * @ingroup SnmpParser
 * @brief ���� ������ �ʱ�ȭ ��Ų��.
 */
void CSnmpOidValue::Clear()
{
	m_strOid.clear();

	if( m_pclsValue )
	{
		delete m_pclsValue;
		m_pclsValue = NULL;
	}
}

/**
 * @ingroup SnmpParser
 * @brief �ڽ��� ���� ������ ���� ������ ��ü�� �����Ͽ��� �����Ѵ�.
 * @returns �����ϸ� �ڽ��� ���� ������ ���� ������ ��ü�� �����Ͽ��� �����ϰ� �׷��� ������ NULL �� �����Ѵ�.
 */
CSnmpOidValue * CSnmpOidValue::Copy()
{
	CSnmpOidValue * pclsNew = new CSnmpOidValue();
	if( pclsNew == NULL ) return NULL;

	pclsNew->m_strOid = m_strOid;

	if( m_pclsValue )
	{
		pclsNew->m_pclsValue = m_pclsValue->Copy();
		if( pclsNew->m_pclsValue == NULL )
		{
			delete pclsNew;
			return NULL;
		}
	}

	return pclsNew;
}

CSnmpOidValueList::CSnmpOidValueList()
{
}

CSnmpOidValueList::~CSnmpOidValueList()
{
	Clear();
}

/**
 * @ingroup SnmpParser
 * @brief OID �� OID �� ���� �����Ѵ�.
 * @param pszOid		OID
 * @param pclsValue OID �� ��
 * @returns �����ϸ� true �� �����ϰ� �����ϸ� false �� �����Ѵ�.
 */
bool CSnmpOidValueList::Add( const char * pszOid, CAsnType * pclsValue )
{
	CSnmpOidValue * pclsOidValue = new CSnmpOidValue();
	if( pclsOidValue == NULL ) return false;
	
	pclsOidValue->m_strOid = pszOid;
	pclsOidValue->m_pclsValue = pclsValue->Copy();
	if( pclsOidValue->m_pclsValue == NULL )
	{
		delete pclsOidValue;
		return false;
	}

	m_clsList.push_back( pclsOidValue );

	return true;
}

/**
 * @ingroup SnmpParser
 * @brief ���� ������ �ʱ�ȭ ��Ų��.
 */
void CSnmpOidValueList::Clear()
{
	SNMP_OID_VALUE_LIST::iterator itOL;

	for( itOL = m_clsList.begin(); itOL != m_clsList.end(); ++itOL )
	{
		delete *itOL;
	}

	m_clsList.clear();
}

/**
 * @ingroup SnmpParser
 * @brief ����Ʈ�� ù��° OID �� OID ���� ������ ��ü�� �����´�.
 * @returns ����Ʈ�� ù��°�� �����ϸ� OID �� OID ���� ������ ��ü�� �����ϰ� �׷��� ������ NULL �� �����Ѵ�.
 */
CSnmpOidValue * CSnmpOidValueList::GetFirst()
{
	SNMP_OID_VALUE_LIST::iterator itOL = m_clsList.begin();
	if( itOL == m_clsList.end() ) return NULL;

	return *itOL;
}

/**
 * @ingroup SnmpParser
 * @brief �ڽ��� ���� ������ ���� ������ ��ü�� �����Ͽ��� �����Ѵ�.
 * @returns �����ϸ� �ڽ��� ���� ������ ���� ������ ��ü�� �����Ͽ��� �����ϰ� �׷��� ������ NULL �� �����Ѵ�.
 */
CSnmpOidValueList * CSnmpOidValueList::Copy()
{
	CSnmpOidValueList * pclsList = new CSnmpOidValueList();
	if( pclsList == NULL ) return NULL;

	SNMP_OID_VALUE_LIST::iterator itOL;

	for( itOL = m_clsList.begin(); itOL != m_clsList.end(); ++itOL )
	{
		CSnmpOidValue * pclsOidValue = (*itOL)->Copy();
		if( pclsOidValue == NULL )
		{
			delete pclsList;
			return NULL;
		}

		pclsList->m_clsList.push_back( pclsOidValue );
	}

	return pclsList;
}

/**
 * @ingroup SnmpParser
 * @brief OID �� OID �� ���� CAsnComplex �� �����Ͽ��� �����Ѵ�.
 * @returns �����ϸ� OID �� OID �� ���� ������ CAsnComplex ��ü�� �����ϰ� �����ϸ� NULL �� �����Ѵ�.
 */
CAsnComplex * CSnmpOidValueList::GetComplex()
{
	SNMP_OID_VALUE_LIST::iterator itOL;
	CAsnType * pclsValue;
	CAsnComplex * pclsBodyFrame = new CAsnComplex();
	if( pclsBodyFrame == NULL )  goto FUNC_ERROR;

	for( itOL = m_clsList.begin(); itOL != m_clsList.end(); ++itOL )
	{
		CAsnComplex * pclsBody = new CAsnComplex();
		if( pclsBody == NULL ) goto FUNC_ERROR;

		if( pclsBody->AddOid( (*itOL)->m_strOid.c_str() ) == false ) goto FUNC_ERROR;

		pclsValue = (*itOL)->m_pclsValue->Copy();
		if( pclsValue == NULL ) goto FUNC_ERROR;
		if( pclsBody->AddValue( pclsValue ) == false ) goto FUNC_ERROR;

		pclsBodyFrame->AddComplex( pclsBody );
	}

	return pclsBodyFrame;

FUNC_ERROR:
	if( pclsBodyFrame ) delete pclsBodyFrame;

	return NULL;
}
