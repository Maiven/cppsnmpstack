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

#ifndef _ASN_INT_H_
#define _ASN_INT_H_

#include "AsnType.h"

/**
 * @ingroup SnmpParser
 * @brief ASN 정수 타입 클래스
 */
class CAsnInt : public CAsnType
{
public:
	CAsnInt();
	CAsnInt( uint8_t cType );
	CAsnInt( uint8_t cType, uint32_t iValue );
	virtual ~CAsnInt();

	virtual int ParsePacket( const char * pszPacket, int iPacketLen );
	virtual int MakePacket( char * pszPacket, int iPacketSize );
	virtual CAsnType * Copy( );

	virtual bool GetInt( uint32_t & iValue );

	/** ASN 값 */
	uint32_t	m_iValue;
};

#endif
