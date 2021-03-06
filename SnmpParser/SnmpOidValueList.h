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

#ifndef _SNMP_OID_VALUE_LIST_H_
#define _SNMP_OID_VALUE_LIST_H_

#include "AsnComplex.h"
#include <list>

/**
 * @ingroup SnmpParser
 * @brief OID 와 OID 의 값을 저장하는 클래스
 */
class CSnmpOidValue
{
public:
	CSnmpOidValue();
	~CSnmpOidValue();

	void Clear();
	CSnmpOidValue * Copy();

	/** OID */
	std::string	m_strOid;

	/** OID 의 값 */
	CAsnType    * m_pclsValue;
};

typedef std::list< CSnmpOidValue * > SNMP_OID_VALUE_LIST;

/**
 * @ingroup SnmpParser
 * @brief OID 와 OID 의 값들을 리스트에 저장하여서 관리하는 클래스
 */
class CSnmpOidValueList
{
public:
	CSnmpOidValueList();
	~CSnmpOidValueList();

	bool Add( const char * pszOid, CAsnType * pclsValue );
	void Clear();
	CSnmpOidValue * GetFirst();

	CSnmpOidValueList * Copy();
	CAsnComplex * GetComplex();

	SNMP_OID_VALUE_LIST m_clsList;
};

#endif
