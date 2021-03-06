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

#ifndef _ASN_OID_H_
#define _ASN_OID_H_

#include "AsnType.h"
#include <string>

/**
 * @ingroup SnmpParser
 * @brief ASN OID 타입 클래스
 */
class CAsnOid : public CAsnType
{
public:
	CAsnOid();
	CAsnOid( const char * pszValue );
	virtual ~CAsnOid();

	virtual int ParsePacket( const char * pszPacket, int iPacketLen );
	virtual int MakePacket( char * pszPacket, int iPacketSize );
	virtual CAsnType * Copy( );

	virtual bool GetString( std::string & strValue );

	/** OID */
	std::string m_strValue;

private:
	static void SetOidEntry( char * pszPacket, int iPacketSize, uint64_t iValue, int & iPos );
};

#endif
