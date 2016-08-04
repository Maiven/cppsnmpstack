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

#include "SnmpPlatformDefine.h"
#include "StringUtility.h"
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include "Log.h"
#include "MemoryDebug.h"

/**
 * @ingroup SipPlatform
 * @brief ���ڿ��� ���Ե� ���ڿ��� �����Ѵ�.
 * @param strCallId	���ڿ�
 * @param pszBefore ���� ��� ���ڿ�
 * @param pszAfter	���� ��� ���ڿ��� ������ ���ڿ�
 */
void ReplaceString( std::string & strCallId, const char * pszBefore, const char * pszAfter )
{
	size_t iPos = strCallId.find( pszBefore );
	size_t iBeforeLen = strlen( pszBefore );
	size_t iAfterLen = strlen( pszAfter );

	while( iPos < std::string::npos )
	{
		strCallId.replace( iPos, iBeforeLen, pszAfter );
		iPos = strCallId.find( pszBefore, iPos + iAfterLen );
	}
}

/**
 * @ingroup SipPlatform
 * @brief ���ڿ��� ���Ե� �� ó�� ���ڿ��� �����Ѵ�.
 * @param strCallId	���ڿ�
 * @param pszBefore ���� ��� ���ڿ�
 * @param pszAfter	���� ��� ���ڿ��� ������ ���ڿ�
 */
void ReplaceStringFirst( std::string & strCallId, const char * pszBefore, const char * pszAfter )
{
	size_t iPos = strCallId.find( pszBefore );
	size_t iBeforeLen = strlen( pszBefore );

	if( iPos < std::string::npos )
	{
		strCallId.replace( iPos, iBeforeLen, pszAfter );
	}
}

/**
 * @ingroup SipPlatform
 * @brief ���ڿ��� ���Ե� Ű�� ���� �����Ѵ�. 
 *				"app=36;msg=36;hotline=46;presence=36; broadcast=46" ���ڿ����� 
 *				app �� ���� �����ϰ� ������ pszKey �� "app=" �� �Է��ϰ� cSep �� ';' �� �Է��ϸ� �ȴ�.
 * @param strText		���ڿ�
 * @param pszKey		Ű
 * @param cSep			������
 * @param strValue	Ű�� ���� ������ ����
 * @returns �����ϸ� true �� �����ϰ� �����ϸ� false �� �����Ѵ�.
 */
bool SearchValue( std::string & strText, const char * pszKey, char cSep, std::string & strValue )
{
	strValue.clear();

	size_t iPos = strText.find( pszKey );
	if( iPos < std::string::npos )
	{
		size_t iKeyLen = strlen( pszKey );
		size_t iEndPos = strText.find( cSep, iPos + iKeyLen );

		if( iEndPos < std::string::npos )
		{
			strValue = strText.substr( iPos + iKeyLen, iEndPos - ( iPos + iKeyLen ) );
		}
		else
		{
			strValue = strText.substr( iPos + iKeyLen );
		}

		return true;
	}

	return false;
}

/**
 * @ingroup SipPlatform
 * @brief ���ڿ��� ���Ե� Ű�� ���� �����Ѵ�. 
 *				"app=36;msg=36;hotline=46;presence=36; broadcast=46" ���ڿ����� 
 *				app �� ���� �����ϰ� ������ pszKey �� "app=" �� �Է��ϰ� cSep �� ';' �� �Է��ϸ� �ȴ�.
 * @param strText		���ڿ�
 * @param pszKey		Ű
 * @param cSep			������
 * @param iValue		Ű�� ���� ������ ����
 * @returns �����ϸ� true �� �����ϰ� �����ϸ� false �� �����Ѵ�.
 */
bool SearchValue( std::string & strText, const char * pszKey, char cSep, int & iValue )
{
	std::string	strValue;

	if( SearchValue( strText, pszKey, cSep, strValue ) )
	{
		iValue = atoi( strValue.c_str() );

		return true;
	}

	return false;
}

/**
 * @ingroup SipPlatform
 * @brief ���ڿ��� �Էµ� ���� �����ϴ��� �˻��Ѵ�.
 * @param strText		���ڿ�
 * @param pszValue	�˻� ���ڿ�
 * @returns ���ڿ��� �Էµ� ���� �����ϸ� true �� �����ϰ� �׷��� ������ false �� �����Ѵ�.
 */
bool SearchString( std::string & strText, const char * pszValue )
{
	size_t iPos = strText.find( pszValue );
	if( iPos < std::string::npos )
	{
		return true;
	}

	return false;
}

/**
 * @ingroup SipPlatform
 * @brief ���ڿ��� �Էµ� ���� �����ϴ��� �˻��Ѵ�.
 * @param strText		���ڿ�
 * @param cValue		�˻� ����
 * @returns ���ڿ��� �Էµ� ���� �����ϸ� true �� �����ϰ� �׷��� ������ false �� �����Ѵ�.
 */
bool SearchString( std::string & strText, char cValue )
{
	size_t iPos = strText.find( cValue );
	if( iPos < std::string::npos )
	{
		return true;
	}

	return false;
}

/**
 * @ingroup SipPlatform
 * @brief ���ڿ��� ���� ������ �����Ѵ�.
 * @param strText ���ڿ�
 */
void LeftTrimString( std::string & strText )
{
	int iIndex;
	int iLen = (int)strText.length();
	for( iIndex = 0; iIndex < iLen; ++iIndex )
	{
		char c = strText.at(iIndex);
		if( c == ' ' || c == '\t' ) continue;

		strText.erase( 0, iIndex );
		break;
	}

	if( iIndex == iLen )
	{
		strText.clear();
	}
}

/**
 * @ingroup SipPlatform
 * @brief ���ڿ��� ������ ������ �����Ѵ�.
 * @param strText ���ڿ�
 */
void RightTrimString( std::string & strText )
{
	int iIndex;
	int iLen = (int)strText.length();
	for( iIndex = iLen - 1; iIndex >= 0; --iIndex )
	{
		char c = strText.at(iIndex);
		if( c == ' ' || c == '\t' ) continue;

		if( iIndex != ( iLen - 1 ) )
		{
			strText.erase( iIndex + 1 );
		}

		break;
	}

	if( iIndex == -1 )
	{
		strText.clear();
	}
}

/**
 * @ingroup SipPlatform
 * @brief ���ڿ��� ����, ������ ������ �����Ѵ�.
 * @param strText ���ڿ�
 */
void TrimString( std::string & strText )
{
	LeftTrimString( strText );
	RightTrimString( strText );
}

/**
 * @ingroup SipPlatform
 * @brief ���α׷� ���� ���ڿ��� �Ľ��Ͽ��� ���ڿ� �迭�� �����Ѵ�.
 * @param pszCommand ���α׷� ���� ���ڿ�
 * @returns �����ϸ� ���ڿ� �迭�� �����ϰ� �����ϸ� NULL �� �����Ѵ�.
 */
char ** ParseCommandString( const char * pszCommand )
{
	char ** ppszArg = NULL;
	STRING_LIST clsList;
	int iStartPos = 0;
	bool bDoubleQuotes = false, bSingleQuotes = false, bStart = false;

	for( int i = 0; pszCommand[i]; ++i )
	{
		if( bStart == false )
		{
			if( isspace( pszCommand[i] ) == 0 )
			{
				bDoubleQuotes = false;
				bSingleQuotes = false;
				bStart = true;
				iStartPos = i;

				if( pszCommand[i] == '"' )
				{
					bDoubleQuotes = true;
					iStartPos = i + 1;
				}
				else if( pszCommand[i] == '\'' )
				{
					bSingleQuotes = true;
					iStartPos = i + 1;
				}
			}
		}
		else
		{
			if( bDoubleQuotes )
			{
				if( pszCommand[i] == '"' )
				{
					std::string strArg;

					strArg.append( pszCommand + iStartPos, i - iStartPos );
					clsList.push_back( strArg );
					bStart = false;
				}
			}
			else if( bSingleQuotes )
			{
				if( pszCommand[i] == '\'' )
				{
					std::string strArg;

					strArg.append( pszCommand + iStartPos, i - iStartPos );
					clsList.push_back( strArg );
					bStart = false;
				}
			}
			else
			{
				if( isspace( pszCommand[i] ) )
				{
					std::string strArg;

					strArg.append( pszCommand + iStartPos, i - iStartPos );
					clsList.push_back( strArg );
					bStart = false;
				}
			}
		}
	}

	if( bStart )
	{
		std::string strArg;

		strArg.append( pszCommand + iStartPos );
		clsList.push_back( strArg );
	}

	int iCount = (int)clsList.size();

	if( iCount > 0 )
	{
		++iCount;
		ppszArg = (char **)malloc( sizeof(char *) * iCount );
		if( ppszArg )
		{
			memset( ppszArg, 0, sizeof(char *) * iCount );

			STRING_LIST::iterator	itList;
			int iPos = 0;

			for( itList = clsList.begin(); itList != clsList.end(); ++itList )
			{
				ppszArg[iPos] = (char *)malloc( itList->length() + 1 );
				if( ppszArg[iPos] == NULL ) break;

				sprintf( ppszArg[iPos], "%s", itList->c_str() );
				++iPos;
			}
		}
	}

	return ppszArg;
}

/**
 * @ingroup SipPlatform
 * @brief ParseCommandString �� ������ �迭�� �����Ѵ�.
 * @param ppszArg ParseCommandString �� ������ �迭
 */
void FreeCommandArg( char ** ppszArg )
{
	if( ppszArg )
	{
		for( int i = 0; ppszArg[i]; ++i )
		{
			free( ppszArg[i] );
		}

		free( ppszArg );
	}
}

/**
 * @ingroup SipPlatform
 * @brief ���ڿ��� ��� 0 ~ 9 ���ڷθ� �����Ǿ� �ִ°�?
 * @param strText ���ڿ�
 * @returns ���ڿ��� ��� 0 ~ 9 ���ڷθ� �����Ǿ� ������ true �� �����ϰ� �׷��� ������ false �� �����Ѵ�.
 */
bool IsStringAllDigit( std::string & strText )
{
	int iLen = (int)strText.length();
	const char * pszText = strText.c_str();

	for( int i = 0; i < iLen; ++i )
	{
		if( pszText[i] < '0' || pszText[i] > '9' ) return false;
	}

	return true;
}

#ifdef WIN32

#include <windows.h>

/**
 * @ingroup SipPlatform
 * @brief UTF8 ���ڿ��� ANSI ���ڿ��� ��ȯ�Ѵ�.
 * @param pszUtf8		UTF8 ���ڿ� (input)
 * @param strOutput ANSI ���ڿ� (output)
 * @returns �����ϸ� true �� �����ϰ� �����ϸ� false �� �����Ѵ�.
 */
bool Utf8ToAnsi( const char * pszUtf8, std::string & strOutput )
{
	BSTR    strWide = NULL;
	char*   pszAnsi = NULL;
	int     iLength;
	int			iUtf8Length = (int)strlen(pszUtf8) + 1;
	
	iLength = MultiByteToWideChar( CP_UTF8, 0, pszUtf8, iUtf8Length, NULL, NULL );
	if( iLength == 0 )
	{
		CLog::Print( LOG_ERROR, "%s MultiByteToWideChar error(%d)", __FUNCTION__, GetLastError() );
		return false;
	}

	strWide = SysAllocStringLen( NULL, iLength );
	if( strWide == NULL )
	{
		CLog::Print( LOG_ERROR, "%s SysAllocStringLen error(%d)", __FUNCTION__, GetLastError() );
		return false;
	}

	MultiByteToWideChar( CP_UTF8, 0, pszUtf8, iUtf8Length, strWide, iLength );

	iLength = WideCharToMultiByte( CP_ACP, 0, strWide, -1, NULL, 0, NULL, NULL );
	if( iLength == 0 )
	{
		SysFreeString( strWide );
		CLog::Print( LOG_ERROR, "%s WideCharToMultiByte error(%d)", __FUNCTION__, GetLastError() );
		return false;
	}

	pszAnsi = new char[iLength];
	if( pszAnsi == NULL )
	{
		SysFreeString( strWide );
		CLog::Print( LOG_ERROR, "%s new error(%d)", __FUNCTION__, GetLastError() );
		return false;
	}

	WideCharToMultiByte( CP_ACP, 0, strWide, -1, pszAnsi, iLength, NULL, NULL );
	strOutput = pszAnsi;
	
	SysFreeString( strWide );
	delete [] pszAnsi;
	
	return true;
}

/**
 * @ingroup SipPlatform
 * @brief ANSI ���ڿ��� UTF-8 ���ڿ��� ��ȯ�Ѵ�. EUC-KR ���ڿ��� UTF-8 ���ڿ��� ��ȯ�Ѵ�.
 * @param pszAnsi		ANSI ���ڿ�
 * @param strOutput UTF-8 ���ڿ��� ������ ����
 * @returns �����ϸ� true �� �����ϰ� �����ϸ� false �� �����Ѵ�.
 */
bool AnsiToUtf8( const char * pszAnsi, std::string & strOutput )
{
	BSTR    strWide = NULL;
	char*   pszUtf8 = NULL;
	int     iLength;
	int			iAnsiLength = (int)strlen(pszAnsi) + 1;
	
	iLength = MultiByteToWideChar( CP_ACP, 0, pszAnsi, iAnsiLength, NULL, NULL );
	if( iLength == 0 )
	{
		CLog::Print( LOG_ERROR, "%s MultiByteToWideChar error(%d)", __FUNCTION__, GetLastError() );
		return false;
	}

	strWide = SysAllocStringLen( NULL, iLength );
	if( strWide == NULL )
	{
		CLog::Print( LOG_ERROR, "%s SysAllocStringLen error(%d)", __FUNCTION__, GetLastError() );
		return false;
	}

	MultiByteToWideChar( CP_ACP, 0, pszAnsi, iAnsiLength, strWide, iLength );

	iLength = WideCharToMultiByte( CP_UTF8, 0, strWide, -1, NULL, 0, NULL, NULL );
	if( iLength == 0 )
	{
		SysFreeString( strWide );
		CLog::Print( LOG_ERROR, "%s WideCharToMultiByte error(%d)", __FUNCTION__, GetLastError() );
		return false;
	}

	pszUtf8 = new char[iLength];
	if( pszUtf8 == NULL )
	{
		SysFreeString( strWide );
		CLog::Print( LOG_ERROR, "%s new error(%d)", __FUNCTION__, GetLastError() );
		return false;
	}

	WideCharToMultiByte( CP_UTF8, 0, strWide, -1, pszUtf8, iLength, NULL, NULL );
	strOutput = pszUtf8;
	
	SysFreeString( strWide );
	delete [] pszUtf8;
	
	return true;
}

#endif

/**
 * @ingroup SipPlatform
 * @brief ���ڿ��� unsigned int �� ��ȯ�Ѵ�.
 * @param pszText ���ڿ�
 * @returns unsigned int �� �����Ѵ�.
 */
uint32_t GetUInt32( const char * pszText )
{
	if( pszText == NULL ) return 0;

	int iRadix = 10;

	// 16���� ó��
	if( pszText[0] == '0' && pszText[1] == 'x' )
	{
		iRadix = 16;
		pszText += 2;
	}

	return strtoul( pszText, NULL, iRadix );
}

/**
 * @ingroup SipPlatform
 * @brief ���ڿ��� unsigned long long ���� ��ȯ�Ѵ�.
 * @param pszText ���ڿ�
 * @returns unsigned long long �� �����Ѵ�.
 */
uint64_t GetUInt64( const char * pszText )
{
	if( pszText == NULL ) return 0;

	int iRadix = 10;

	// 16���� ó��
	if( pszText[0] == '0' && pszText[1] == 'x' )
	{
		iRadix = 16;
		pszText += 2;
	}

#ifdef WIN32
	return _strtoui64( pszText, NULL, iRadix );
#else
	return strtoull( pszText, NULL, iRadix );
#endif
}

/**
 * @ingroup SipPlatform
 * @brief ������ ���̸�ŭ�� ���ڿ��� ���ڷ� ��ȯ�Ѵ�.
 * @param pszText		���� ���ڿ�
 * @param iTextLen	���ڿ� ����
 * @returns �����ϸ� ���ϴ� ���ڰ� ���ϵǰ� �����ϸ� 0 �� ���ϵȴ�.
 */
int GetInt( const char * pszText, int iTextLen )
{
	char szNum[11];

	if( iTextLen > 10 || iTextLen <= 0 ) return 0;

	memcpy( szNum, pszText, iTextLen );
	szNum[iTextLen] = '\0';

	return atoi( szNum );
}

static inline char HexToChar( char c )
{
  if( c >= '0' && c <= '9' )
  {
		return c - '0';
  }
  else if( c >= 'a' && c <= 'f' )
  {
		return c + 10 - 'a';
  }
  else if( c >= 'A' && c <= 'F' )
  {
		return c + 10 - 'A';
  }

  return -1;
}

/**
 * @ingroup SipPlatform
 * @brief HEX �� ����� ���ڿ��� ���ڷ� ��ȯ�� ���ڿ��� �����Ѵ�.
 * @param pszInput	HEX �� ����� ���ڿ�
 * @param strOutput ���ڷ� ��ȯ�� ���ڿ� ����� ����
 * @returns �����ϸ� true �� �����ϰ� �����ϸ� false �� �����Ѵ�.
 */
bool HexToString( const char * pszInput, std::string & strOutput )
{
	int iLen = (int)strlen( pszInput );
	int iValue;

	strOutput.clear();

	if( iLen >= 2 )
	{
		if( pszInput[0] == '0' && pszInput[1] == 'x' )
		{
			pszInput += 2;
			iLen -= 2;
		}
	}

	if( iLen == 0 || iLen % 2 == 1 ) return false;

	for( int i = 0; i < iLen; i += 2 )
	{
		sscanf( pszInput + i, "%02x", &iValue );
		strOutput.push_back( (char)iValue );
	}

	return true;
}

/**
 * @ingroup SipPlatform
 * @brief ���ڿ��� �Է��� ���ڿ��� �߰��Ѵ�.
 * @param strBuf	[out] ���ڿ�
 * @param fmt			[in] ���� ���ڿ�
 * @param ...			[in] fmt ���˿� �Է��� ���ڵ�
 */
void StringAppend( std::string & strBuf, const char * fmt, ... )
{
	va_list		ap;
	char		szBuf[1024];

	va_start( ap, fmt );
	vsnprintf( szBuf, sizeof(szBuf)-1, fmt, ap );
	va_end( ap );

	strBuf.append( szBuf );
}
