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

class CAsnOid : public CAsnType
{
public:
	CAsnOid();
	~CAsnOid();

	virtual int ParsePacket( const char * pszPacket, int iPacketLen );
	virtual int MakePacket( char * pszPacket, int iPacketSize );

	std::string m_strValue;
};

#endif