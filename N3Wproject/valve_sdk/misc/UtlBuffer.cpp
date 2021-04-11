//===== Copyright � 1996-2005, Valve Corporation, All rights reserved. ======//
//
// $Header: $
// $NoKeywords: $
//
// Serialization buffer
//===========================================================================//

#pragma warning( disable : 4514 )

#include "..\sdk.h"
#include "UtlBuffer.h"
#include <stdio.h>
#include <stdarg.h>
#include <ctype.h>
#include <stdlib.h>
#include <limits.h>
#include "characterset.h"

const char* V_strnchr( const char* pStr, char c, int n ) {
	char const* pLetter = pStr;
	char const* pLast = pStr + n;

	// Check the entire string
	while ( ( pLetter < pLast ) && ( *pLetter != 0 ) ) {
		if ( *pLetter == c )
			return pLetter;
		++pLetter;
	}
	return NULL;
}
//-----------------------------------------------------------------------------
// Finds a string in another string with a case insensitive test w/ length validation
//-----------------------------------------------------------------------------
char const* V_strnistr( char const* pStr, char const* pSearch, int n ) {
	if ( !pStr || !pSearch )
		return 0;

	char const* pLetter = pStr;

	// Check the entire string
	while ( *pLetter != 0 ) {
		if ( n <= 0 )
			return 0;

		// Skip over non-matches
		if ( tolower( *pLetter ) == tolower( *pSearch ) ) {
			int n1 = n - 1;

			// Check for match
			char const* pMatch = pLetter + 1;
			char const* pTest = pSearch + 1;
			while ( *pTest != 0 ) {
				if ( n1 <= 0 )
					return 0;

				// We've run off the end; don't bother.
				if ( *pMatch == 0 )
					return 0;

				if ( tolower( *pMatch ) != tolower( *pTest ) )
					break;

				++pMatch;
				++pTest;
				--n1;
			}

			// Found a match!
			if ( *pTest == 0 )
				return pLetter;
		}

		++pLetter;
		--n;
	}

	return 0;
}
//-----------------------------------------------------------------------------
// Character conversions for C strings
//-----------------------------------------------------------------------------
class CUtlCStringConversion : public CUtlCharConversion {
public:
	CUtlCStringConversion( char nEscapeChar, const char* pDelimiter, int nCount, ConversionArray_t* pArray );

	// Finds a conversion for the passed-in string, returns length
	virtual char FindConversion( const char* pString, int* pLength );

private:
	char m_pConversion[255];
};

//-----------------------------------------------------------------------------
// Character conversions for no-escape sequence strings
//-----------------------------------------------------------------------------
class CUtlNoEscConversion : public CUtlCharConversion {
public:
	CUtlNoEscConversion( char nEscapeChar, const char* pDelimiter, int nCount, ConversionArray_t* pArray ) :
		CUtlCharConversion( nEscapeChar, pDelimiter, nCount, pArray ) {
	}

	// Finds a conversion for the passed-in string, returns length
	virtual char FindConversion( const char* pString, int* pLength ) {
		*pLength = 0;
		return 0;
	}
};

//-----------------------------------------------------------------------------
// List of character conversions
//-----------------------------------------------------------------------------
BEGIN_CUSTOM_CHAR_CONVERSION( CUtlCStringConversion, s_StringCharConversion, "\"", '\\' ){
	'\n', "n"
},
	{ '\t', "t" },
	{ '\v', "v" },
	{ '\b', "b" },
	{ '\r', "r" },
	{ '\f', "f" },
	{ '\a', "a" },
	{ '\\', "\\" },
	{ '\?', "\?" },
	{ '\'', "\'" },
	{ '\"', "\"" },
		END_CUSTOM_CHAR_CONVERSION( CUtlCStringConversion, s_StringCharConversion, "\"", '\\' );

	CUtlCharConversion* GetCStringCharConversion() {
		return &s_StringCharConversion;
	}

	BEGIN_CUSTOM_CHAR_CONVERSION( CUtlNoEscConversion, s_NoEscConversion, "\"", 0x7F ){
		0x7F, ""
	},
		END_CUSTOM_CHAR_CONVERSION( CUtlNoEscConversion, s_NoEscConversion, "\"", 0x7F );

		CUtlCharConversion* GetNoEscCharConversion() {
			return &s_NoEscConversion;
		}

		//-----------------------------------------------------------------------------
		// Constructor
		//-----------------------------------------------------------------------------
		CUtlCStringConversion::CUtlCStringConversion( char nEscapeChar, const char* pDelimiter, int nCount, ConversionArray_t* pArray ) :
			CUtlCharConversion( nEscapeChar, pDelimiter, nCount, pArray ) {
			memset( m_pConversion, 0x0, sizeof( m_pConversion ) );
			for ( int i = 0; i < nCount; ++i ) {
				m_pConversion[pArray[i].m_pReplacementString[0]] = pArray[i].m_nActualChar;
			}
		}

		// Finds a conversion for the passed-in string, returns length
		char CUtlCStringConversion::FindConversion( const char* pString, int* pLength ) {
			char c = m_pConversion[pString[0]];
			*pLength = ( c != '\0' ) ? 1 : 0;
			return c;
		}

		//-----------------------------------------------------------------------------
		// Constructor
		//-----------------------------------------------------------------------------
		CUtlCharConversion::CUtlCharConversion( char nEscapeChar, const char* pDelimiter, int nCount, ConversionArray_t* pArray ) {
			m_nEscapeChar = nEscapeChar;
			m_pDelimiter = pDelimiter;
			m_nCount = nCount;
			m_nDelimiterLength = strlen( pDelimiter );
			m_nMaxConversionLength = 0;

			memset( m_pReplacements, 0, sizeof( m_pReplacements ) );

			for ( int i = 0; i < nCount; ++i ) {
				m_pList[i] = pArray[i].m_nActualChar;
				ConversionInfo_t& info = m_pReplacements[m_pList[i]];
				assert( info.m_pReplacementString == 0 );
				info.m_pReplacementString = pArray[i].m_pReplacementString;
				info.m_nLength = strlen( info.m_pReplacementString );
				if ( info.m_nLength > m_nMaxConversionLength ) {
					m_nMaxConversionLength = info.m_nLength;
				}
			}
		}

		//-----------------------------------------------------------------------------
		// Escape character + delimiter
		//-----------------------------------------------------------------------------
		char CUtlCharConversion::GetEscapeChar() const {
			return m_nEscapeChar;
		}

		const char* CUtlCharConversion::GetDelimiter() const {
			return m_pDelimiter;
		}

		int CUtlCharConversion::GetDelimiterLength() const {
			return m_nDelimiterLength;
		}

		//-----------------------------------------------------------------------------
		// Constructor
		//-----------------------------------------------------------------------------
		const char* CUtlCharConversion::GetConversionString( char c ) const {
			return m_pReplacements[c].m_pReplacementString;
		}

		int CUtlCharConversion::GetConversionLength( char c ) const {
			return m_pReplacements[c].m_nLength;
		}

		int CUtlCharConversion::MaxConversionLength() const {
			return m_nMaxConversionLength;
		}

		//-----------------------------------------------------------------------------
		// Finds a conversion for the passed-in string, returns length
		//-----------------------------------------------------------------------------
		char CUtlCharConversion::FindConversion( const char* pString, int* pLength ) {
			for ( int i = 0; i < m_nCount; ++i ) {
				if ( !strcmp( pString, m_pReplacements[m_pList[i]].m_pReplacementString ) ) {
					*pLength = m_pReplacements[m_pList[i]].m_nLength;
					return m_pList[i];
				}
			}

			*pLength = 0;
			return '\0';
		}

		//-----------------------------------------------------------------------------
		// constructors
		//-----------------------------------------------------------------------------
		c_utl_buffer::c_utl_buffer( int growSize, int initSize, int nFlags ) :
			m_Memory( growSize, initSize ), m_Error( 0 ) {
			m_Get = 0;
			m_Put = 0;
			m_nTab = 0;
			m_nOffset = 0;
			m_Flags = (unsigned char)nFlags;
			if ( ( initSize != 0 ) && !IsReadOnly() ) {
				m_nMaxPut = -1;
				AddNullTermination();
			} else {
				m_nMaxPut = 0;
			}
			SetOverflowFuncs( &c_utl_buffer::GetOverflow, &c_utl_buffer::PutOverflow );
		}

		c_utl_buffer::c_utl_buffer( const void* pBuffer, int nSize, int nFlags ) :
			m_Memory( (unsigned char*)pBuffer, nSize ), m_Error( 0 ) {
			assert( nSize != 0 );

			m_Get = 0;
			m_Put = 0;
			m_nTab = 0;
			m_nOffset = 0;
			m_Flags = (unsigned char)nFlags;
			if ( IsReadOnly() ) {
				m_nMaxPut = nSize;
			} else {
				m_nMaxPut = -1;
				AddNullTermination();
			}
			SetOverflowFuncs( &c_utl_buffer::GetOverflow, &c_utl_buffer::PutOverflow );
		}

		//-----------------------------------------------------------------------------
		// Modifies the buffer to be binary or text; Blows away the buffer and the CONTAINS_CRLF value.
		//-----------------------------------------------------------------------------
		void c_utl_buffer::SetBufferType( bool bIsText, bool bContainsCRLF ) {
		#ifdef _DEBUG
			// If the buffer is empty, there is no opportunity for this stuff to fail
			if ( TellMaxPut() != 0 ) {
				if ( IsText() ) {
					if ( bIsText ) {
						assert( ContainsCRLF() == bContainsCRLF );
					} else {
						assert( ContainsCRLF() );
					}
				} else {
					if ( bIsText ) {
						assert( bContainsCRLF );
					}
				}
			}
		#endif

			if ( bIsText ) {
				m_Flags |= TEXT_BUFFER;
			} else {
				m_Flags &= ~TEXT_BUFFER;
			}
			if ( bContainsCRLF ) {
				m_Flags |= CONTAINS_CRLF;
			} else {
				m_Flags &= ~CONTAINS_CRLF;
			}
		}

		//-----------------------------------------------------------------------------
		// Attaches the buffer to external memory....
		//-----------------------------------------------------------------------------
		void c_utl_buffer::SetExternalBuffer( void* pMemory, int nSize, int nInitialPut, int nFlags ) {
			m_Memory.SetExternalBuffer( (unsigned char*)pMemory, nSize );

			// Reset all indices; we just changed memory
			m_Get = 0;
			m_Put = nInitialPut;
			m_nTab = 0;
			m_Error = 0;
			m_nOffset = 0;
			m_Flags = (unsigned char)nFlags;
			m_nMaxPut = -1;
			AddNullTermination();
		}

		//-----------------------------------------------------------------------------
		// Assumes an external buffer but manages its deletion
		//-----------------------------------------------------------------------------
		void c_utl_buffer::AssumeMemory( void* pMemory, int nSize, int nInitialPut, int nFlags ) {
			m_Memory.AssumeMemory( (unsigned char*)pMemory, nSize );

			// Reset all indices; we just changed memory
			m_Get = 0;
			m_Put = nInitialPut;
			m_nTab = 0;
			m_Error = 0;
			m_nOffset = 0;
			m_Flags = (unsigned char)nFlags;
			m_nMaxPut = -1;
			AddNullTermination();
		}

		//-----------------------------------------------------------------------------
		// Makes sure we've got at least this much memory
		//-----------------------------------------------------------------------------
		void c_utl_buffer::EnsureCapacity( int num ) {
			// Add one extra for the null termination
			num += 1;
			if ( m_Memory.IsExternallyAllocated() ) {
				if ( IsGrowable() && ( m_Memory.NumAllocated() < num ) ) {
					m_Memory.ConvertToGrowableMemory( 0 );
				} else {
					num -= 1;
				}
			}

			m_Memory.EnsureCapacity( num );
		}

		//-----------------------------------------------------------------------------
		// Base Get method from which all others derive
		//-----------------------------------------------------------------------------
		void c_utl_buffer::Get( void* pMem, int size ) {
			if ( CheckGet( size ) ) {
				memcpy( pMem, &m_Memory[m_Get - m_nOffset], size );
				m_Get += size;
			}
		}

		//-----------------------------------------------------------------------------
		// This will Get at least 1 uint8_t and up to nSize bytes.
		// It will return the number of bytes actually read.
		//-----------------------------------------------------------------------------
		int c_utl_buffer::GetUpTo( void* pMem, int nSize ) {
			if ( CheckArbitraryPeekGet( 0, nSize ) ) {
				memcpy( pMem, &m_Memory[m_Get - m_nOffset], nSize );
				m_Get += nSize;
				return nSize;
			}
			return 0;
		}

		//-----------------------------------------------------------------------------
		// Eats whitespace
		//-----------------------------------------------------------------------------
		void c_utl_buffer::EatWhiteSpace() {
			if ( IsText() && IsValid() ) {
				while ( CheckGet( sizeof( char ) ) ) {
					if ( !isspace( *(const unsigned char*)PeekGet() ) )
						break;
					m_Get += sizeof( char );
				}
			}
		}

		//-----------------------------------------------------------------------------
		// Eats C++ style comments
		//-----------------------------------------------------------------------------
		bool c_utl_buffer::EatCPPComment() {
			if ( IsText() && IsValid() ) {
				// If we don't have a a c++ style comment next, we're done
				const char* pPeek = (const char*)PeekGet( 2 * sizeof( char ), 0 );
				if ( !pPeek || ( pPeek[0] != '/' ) || ( pPeek[1] != '/' ) )
					return false;

				// Deal with c++ style comments
				m_Get += 2;

				// read complete line
				for ( char c = GetChar(); IsValid(); c = GetChar() ) {
					if ( c == '\n' )
						break;
				}
				return true;
			}
			return false;
		}

		//-----------------------------------------------------------------------------
		// Peeks how much whitespace to eat
		//-----------------------------------------------------------------------------
		int c_utl_buffer::PeekWhiteSpace( int nOffset ) {
			if ( !IsText() || !IsValid() )
				return 0;

			while ( CheckPeekGet( nOffset, sizeof( char ) ) ) {
				if ( !isspace( *(unsigned char*)PeekGet( nOffset ) ) )
					break;
				nOffset += sizeof( char );
			}

			return nOffset;
		}

		//-----------------------------------------------------------------------------
		// Peek size of sting to come, check memory bound
		//-----------------------------------------------------------------------------
		int c_utl_buffer::PeekStringLength() {
			if ( !IsValid() )
				return 0;

			// Eat preceeding whitespace
			int nOffset = 0;
			if ( IsText() ) {
				nOffset = PeekWhiteSpace( nOffset );
			}

			int nStartingOffset = nOffset;

			do {
				int nPeekAmount = 128;

				// NOTE: Add 1 for the terminating zero!
				if ( !CheckArbitraryPeekGet( nOffset, nPeekAmount ) ) {
					if ( nOffset == nStartingOffset )
						return 0;
					return nOffset - nStartingOffset + 1;
				}

				const char* pTest = (const char*)PeekGet( nOffset );

				if ( !IsText() ) {
					for ( int i = 0; i < nPeekAmount; ++i ) {
						// The +1 here is so we eat the terminating 0
						if ( pTest[i] == 0 )
							return ( i + nOffset - nStartingOffset + 1 );
					}
				} else {
					for ( int i = 0; i < nPeekAmount; ++i ) {
						// The +1 here is so we eat the terminating 0
						if ( isspace( (unsigned char)pTest[i] ) || ( pTest[i] == 0 ) )
							return ( i + nOffset - nStartingOffset + 1 );
					}
				}

				nOffset += nPeekAmount;

			} while ( true );
		}

		//-----------------------------------------------------------------------------
		// Peek size of line to come, check memory bound
		//-----------------------------------------------------------------------------
		int c_utl_buffer::PeekLineLength() {
			if ( !IsValid() )
				return 0;

			int nOffset = 0;
			int nStartingOffset = nOffset;

			do {
				int nPeekAmount = 128;

				// NOTE: Add 1 for the terminating zero!
				if ( !CheckArbitraryPeekGet( nOffset, nPeekAmount ) ) {
					if ( nOffset == nStartingOffset )
						return 0;
					return nOffset - nStartingOffset + 1;
				}

				const char* pTest = (const char*)PeekGet( nOffset );

				for ( int i = 0; i < nPeekAmount; ++i ) {
					// The +2 here is so we eat the terminating '\n' and 0
					if ( pTest[i] == '\n' || pTest[i] == '\r' )
						return ( i + nOffset - nStartingOffset + 2 );
					// The +1 here is so we eat the terminating 0
					if ( pTest[i] == 0 )
						return ( i + nOffset - nStartingOffset + 1 );
				}

				nOffset += nPeekAmount;

			} while ( true );
		}

		//-----------------------------------------------------------------------------
		// Does the next bytes of the buffer match a pattern?
		//-----------------------------------------------------------------------------
		bool c_utl_buffer::PeekStringMatch( int nOffset, const char* pString, int nLen ) {
			if ( !CheckPeekGet( nOffset, nLen ) )
				return false;
			return !strncmp( (const char*)PeekGet( nOffset ), pString, nLen );
		}

		//-----------------------------------------------------------------------------
		// This version of PeekStringLength converts \" to \\ and " to \, etc.
		// It also reads a " at the beginning and end of the string
		//-----------------------------------------------------------------------------
		int c_utl_buffer::PeekDelimitedStringLength( CUtlCharConversion* pConv, bool bActualSize ) {
			if ( !IsText() || !pConv )
				return PeekStringLength();

			// Eat preceeding whitespace
			int nOffset = 0;
			if ( IsText() ) {
				nOffset = PeekWhiteSpace( nOffset );
			}

			if ( !PeekStringMatch( nOffset, pConv->GetDelimiter(), pConv->GetDelimiterLength() ) )
				return 0;

			// Try to read ending ", but don't accept \"
			int nActualStart = nOffset;
			nOffset += pConv->GetDelimiterLength();
			int nLen = 1; // Starts at 1 for the '\0' termination

			do {
				if ( PeekStringMatch( nOffset, pConv->GetDelimiter(), pConv->GetDelimiterLength() ) )
					break;

				if ( !CheckPeekGet( nOffset, 1 ) )
					break;

				char c = *(const char*)PeekGet( nOffset );
				++nLen;
				++nOffset;
				if ( c == pConv->GetEscapeChar() ) {
					int nLength = pConv->MaxConversionLength();
					if ( !CheckArbitraryPeekGet( nOffset, nLength ) )
						break;

					pConv->FindConversion( (const char*)PeekGet( nOffset ), &nLength );
					nOffset += nLength;
				}
			} while ( true );

			return bActualSize ? nLen : nOffset - nActualStart + pConv->GetDelimiterLength() + 1;
		}

		//-----------------------------------------------------------------------------
		// Reads a null-terminated string
		//-----------------------------------------------------------------------------
		void c_utl_buffer::GetString( char* pString, int nMaxChars ) {
			if ( !IsValid() ) {
				*pString = 0;
				return;
			}

			if ( nMaxChars == 0 ) {
				nMaxChars = INT_MAX;
			}

			// Remember, this *includes* the null character
			// It will be 0, however, if the buffer is empty.
			int nLen = PeekStringLength();

			if ( IsText() ) {
				EatWhiteSpace();
			}

			if ( nLen == 0 ) {
				*pString = 0;
				m_Error |= GET_OVERFLOW;
				return;
			}

			// Strip off the terminating NULL
			if ( nLen <= nMaxChars ) {
				Get( pString, nLen - 1 );
				pString[nLen - 1] = 0;
			} else {
				Get( pString, nMaxChars - 1 );
				pString[nMaxChars - 1] = 0;
				SeekGet( SEEK_CURRENT, nLen - 1 - nMaxChars );
			}

			// Read the terminating NULL in binary formats
			if ( !IsText() ) {
				assert( GetChar() == 0 );
			}
		}

		//-----------------------------------------------------------------------------
		// Reads up to and including the first \n
		//-----------------------------------------------------------------------------
		void c_utl_buffer::GetLine( char* pLine, int nMaxChars ) {
			assert( IsText() && !ContainsCRLF() );

			if ( !IsValid() ) {
				*pLine = 0;
				return;
			}

			if ( nMaxChars == 0 ) {
				nMaxChars = INT_MAX;
			}

			// Remember, this *includes* the null character
			// It will be 0, however, if the buffer is empty.
			int nLen = PeekLineLength();
			if ( nLen == 0 ) {
				*pLine = 0;
				m_Error |= GET_OVERFLOW;
				return;
			}

			// Strip off the terminating NULL
			if ( nLen <= nMaxChars ) {
				Get( pLine, nLen - 1 );
				pLine[nLen - 1] = 0;
			} else {
				Get( pLine, nMaxChars - 1 );
				pLine[nMaxChars - 1] = 0;
				SeekGet( SEEK_CURRENT, nLen - 1 - nMaxChars );
			}
		}

		//-----------------------------------------------------------------------------
		// This version of GetString converts \ to \\ and " to \", etc.
		// It also places " at the beginning and end of the string
		//-----------------------------------------------------------------------------
		char c_utl_buffer::GetDelimitedCharInternal( CUtlCharConversion* pConv ) {
			char c = GetChar();
			if ( c == pConv->GetEscapeChar() ) {
				int nLength = pConv->MaxConversionLength();
				if ( !CheckArbitraryPeekGet( 0, nLength ) )
					return '\0';

				c = pConv->FindConversion( (const char*)PeekGet(), &nLength );
				SeekGet( SEEK_CURRENT, nLength );
			}

			return c;
		}

		char c_utl_buffer::GetDelimitedChar( CUtlCharConversion* pConv ) {
			if ( !IsText() || !pConv )
				return GetChar();
			return GetDelimitedCharInternal( pConv );
		}

		void c_utl_buffer::GetDelimitedString( CUtlCharConversion* pConv, char* pString, int nMaxChars ) {
			if ( !IsText() || !pConv ) {
				GetString( pString, nMaxChars );
				return;
			}

			if ( !IsValid() ) {
				*pString = 0;
				return;
			}

			if ( nMaxChars == 0 ) {
				nMaxChars = INT_MAX;
			}

			EatWhiteSpace();
			if ( !PeekStringMatch( 0, pConv->GetDelimiter(), pConv->GetDelimiterLength() ) )
				return;

			// Pull off the starting delimiter
			SeekGet( SEEK_CURRENT, pConv->GetDelimiterLength() );

			int nRead = 0;
			while ( IsValid() ) {
				if ( PeekStringMatch( 0, pConv->GetDelimiter(), pConv->GetDelimiterLength() ) ) {
					SeekGet( SEEK_CURRENT, pConv->GetDelimiterLength() );
					break;
				}

				char c = GetDelimitedCharInternal( pConv );

				if ( nRead < nMaxChars ) {
					pString[nRead] = c;
					++nRead;
				}
			}

			if ( nRead >= nMaxChars ) {
				nRead = nMaxChars - 1;
			}
			pString[nRead] = '\0';
		}

		//-----------------------------------------------------------------------------
		// Checks if a Get is ok
		//-----------------------------------------------------------------------------
		bool c_utl_buffer::CheckGet( int nSize ) {
			if ( m_Error & GET_OVERFLOW )
				return false;

			if ( TellMaxPut() < m_Get + nSize ) {
				m_Error |= GET_OVERFLOW;
				return false;
			}

			if ( ( m_Get < m_nOffset ) || ( m_Memory.NumAllocated() < m_Get - m_nOffset + nSize ) ) {
				if ( !OnGetOverflow( nSize ) ) {
					m_Error |= GET_OVERFLOW;
					return false;
				}
			}

			return true;
		}

		//-----------------------------------------------------------------------------
		// Checks if a peek Get is ok
		//-----------------------------------------------------------------------------
		bool c_utl_buffer::CheckPeekGet( int nOffset, int nSize ) {
			if ( m_Error & GET_OVERFLOW )
				return false;

			// Checking for peek can't Set the overflow flag
			bool bOk = CheckGet( nOffset + nSize );
			m_Error &= ~GET_OVERFLOW;
			return bOk;
		}

		//-----------------------------------------------------------------------------
		// Call this to peek arbitrarily long into memory. It doesn't fail unless
		// it can't read *anything* new
		//-----------------------------------------------------------------------------
		bool c_utl_buffer::CheckArbitraryPeekGet( int nOffset, int& nIncrement ) {
			if ( TellGet() + nOffset >= TellMaxPut() ) {
				nIncrement = 0;
				return false;
			}

			if ( TellGet() + nOffset + nIncrement > TellMaxPut() ) {
				nIncrement = TellMaxPut() - TellGet() - nOffset;
			}

			// NOTE: CheckPeekGet could modify TellMaxPut for streaming files
			// We have to call TellMaxPut again here
			CheckPeekGet( nOffset, nIncrement );
			int nMaxGet = TellMaxPut() - TellGet();
			if ( nMaxGet < nIncrement ) {
				nIncrement = nMaxGet;
			}
			return ( nIncrement != 0 );
		}

		//-----------------------------------------------------------------------------
		// Peek part of the butt
		//-----------------------------------------------------------------------------
		const void* c_utl_buffer::PeekGet( int nMaxSize, int nOffset ) {
			if ( !CheckPeekGet( nOffset, nMaxSize ) )
				return NULL;
			return &m_Memory[m_Get + nOffset - m_nOffset];
		}

		//-----------------------------------------------------------------------------
		// Change where I'm reading
		//-----------------------------------------------------------------------------
		void c_utl_buffer::SeekGet( SeekType_t type, int offset ) {
			switch ( type ) {
			case SEEK_HEAD:
				m_Get = offset;
				break;

			case SEEK_CURRENT:
				m_Get += offset;
				break;

			case SEEK_TAIL:
				m_Get = m_nMaxPut - offset;
				break;
			}

			if ( m_Get > m_nMaxPut ) {
				m_Error |= GET_OVERFLOW;
			} else {
				m_Error &= ~GET_OVERFLOW;
				if ( m_Get < m_nOffset || m_Get >= m_nOffset + Size() ) {
					OnGetOverflow( -1 );
				}
			}
		}

		//-----------------------------------------------------------------------------
		// Parse...
		//-----------------------------------------------------------------------------

	#pragma warning( disable : 4706 )

		int c_utl_buffer::VaScanf( const char* pFmt, va_list list ) {
			assert( pFmt );
			if ( m_Error || !IsText() )
				return 0;

			int numScanned = 0;
			int nLength;
			char c;
			char* pEnd;
			while ( c = *pFmt++ ) {
				// Stop if we hit the end of the buffer
				if ( m_Get >= TellMaxPut() ) {
					m_Error |= GET_OVERFLOW;
					break;
				}

				switch ( c ) {
				case ' ':
					// eat all whitespace
					EatWhiteSpace();
					break;

				case '%': {
					// Conversion character... try to convert baby!
					char type = *pFmt++;
					if ( type == 0 )
						return numScanned;

					switch ( type ) {
					case 'c': {
						char* ch = va_arg( list, char* );
						if ( CheckPeekGet( 0, sizeof( char ) ) ) {
							*ch = *(const char*)PeekGet();
							++m_Get;
						} else {
							*ch = 0;
							return numScanned;
						}
					} break;

					case 'i':
					case 'd': {
						int* i = va_arg( list, int* );

						// NOTE: This is not bullet-proof; it assumes numbers are < 128 characters
						nLength = 128;
						if ( !CheckArbitraryPeekGet( 0, nLength ) ) {
							*i = 0;
							return numScanned;
						}

						*i = strtol( (char*)PeekGet(), &pEnd, 10 );
						int nBytesRead = (int)( pEnd - (char*)PeekGet() );
						if ( nBytesRead == 0 )
							return numScanned;
						m_Get += nBytesRead;
					} break;

					case 'x': {
						int* i = va_arg( list, int* );

						// NOTE: This is not bullet-proof; it assumes numbers are < 128 characters
						nLength = 128;
						if ( !CheckArbitraryPeekGet( 0, nLength ) ) {
							*i = 0;
							return numScanned;
						}

						*i = strtol( (char*)PeekGet(), &pEnd, 16 );
						int nBytesRead = (int)( pEnd - (char*)PeekGet() );
						if ( nBytesRead == 0 )
							return numScanned;
						m_Get += nBytesRead;
					} break;

					case 'u': {
						unsigned int* u = va_arg( list, unsigned int* );

						// NOTE: This is not bullet-proof; it assumes numbers are < 128 characters
						nLength = 128;
						if ( !CheckArbitraryPeekGet( 0, nLength ) ) {
							*u = 0;
							return numScanned;
						}

						*u = strtoul( (char*)PeekGet(), &pEnd, 10 );
						int nBytesRead = (int)( pEnd - (char*)PeekGet() );
						if ( nBytesRead == 0 )
							return numScanned;
						m_Get += nBytesRead;
					} break;

					case 'f': {
						float* f = va_arg( list, float* );

						// NOTE: This is not bullet-proof; it assumes numbers are < 128 characters
						nLength = 128;
						if ( !CheckArbitraryPeekGet( 0, nLength ) ) {
							*f = 0.0f;
							return numScanned;
						}

						*f = (float)strtod( (char*)PeekGet(), &pEnd );
						int nBytesRead = (int)( pEnd - (char*)PeekGet() );
						if ( nBytesRead == 0 )
							return numScanned;
						m_Get += nBytesRead;
					} break;

					case 's': {
						char* s = va_arg( list, char* );
						GetString( s );
					} break;

					default: {
						// unimplemented scanf type
						assert( 0 );
						return numScanned;
					} break;
					}

					++numScanned;
				} break;

				default: {
					// Here we have to match the format string character
					// against what's in the buffer or we're done.
					if ( !CheckPeekGet( 0, sizeof( char ) ) )
						return numScanned;

					if ( c != *(const char*)PeekGet() )
						return numScanned;

					++m_Get;
				}
				}
			}
			return numScanned;
		}

	#pragma warning( default : 4706 )

		int c_utl_buffer::Scanf( const char* pFmt, ... ) {
			va_list args;

			va_start( args, pFmt );
			int count = VaScanf( pFmt, args );
			va_end( args );

			return count;
		}

		//-----------------------------------------------------------------------------
		// Advance the Get index until after the particular string is found
		// Do not eat whitespace before starting. Return false if it failed
		//-----------------------------------------------------------------------------
		bool c_utl_buffer::GetToken( const char* pToken ) {
			assert( pToken );

			// Look for the token
			int nLen = strlen( pToken );

			int nSizeToCheck = Size() - TellGet() - m_nOffset;

			int nGet = TellGet();
			do {
				int nMaxSize = TellMaxPut() - TellGet();
				if ( nMaxSize < nSizeToCheck ) {
					nSizeToCheck = nMaxSize;
				}
				if ( nLen > nSizeToCheck )
					break;

				if ( !CheckPeekGet( 0, nSizeToCheck ) )
					break;

				const char* pBufStart = (const char*)PeekGet();
				const char* pFoundEnd = V_strnistr( pBufStart, pToken, nSizeToCheck );
				if ( pFoundEnd ) {
					size_t nOffset = (size_t)pFoundEnd - (size_t)pBufStart;
					SeekGet( c_utl_buffer::SEEK_CURRENT, nOffset + nLen );
					return true;
				}

				SeekGet( c_utl_buffer::SEEK_CURRENT, nSizeToCheck - nLen - 1 );
				nSizeToCheck = Size() - ( nLen - 1 );

			} while ( true );

			SeekGet( c_utl_buffer::SEEK_HEAD, nGet );
			return false;
		}

		//-----------------------------------------------------------------------------
		// (For text buffers only)
		// Parse a token from the buffer:
		// Grab all text that lies between a starting delimiter + ending delimiter
		// (skipping whitespace that leads + trails both delimiters).
		// Note the delimiter checks are case-insensitive.
		// If successful, the Get index is advanced and the function returns true,
		// otherwise the index is not advanced and the function returns false.
		//-----------------------------------------------------------------------------
		bool c_utl_buffer::ParseToken( const char* pStartingDelim, const char* pEndingDelim, char* pString, int nMaxLen ) {
			int nCharsToCopy = 0;
			int nCurrentGet = 0;

			size_t nEndingDelimLen;

			// Starting delimiter is optional
			char emptyBuf = '\0';
			if ( !pStartingDelim ) {
				pStartingDelim = &emptyBuf;
			}

			// Ending delimiter is not
			assert( pEndingDelim && pEndingDelim[0] );
			nEndingDelimLen = strlen( pEndingDelim );

			int nStartGet = TellGet();
			char nCurrChar;
			int nTokenStart = -1;
			EatWhiteSpace();
			while ( *pStartingDelim ) {
				nCurrChar = *pStartingDelim++;
				if ( !isspace( (unsigned char)nCurrChar ) ) {
					if ( tolower( GetChar() ) != tolower( nCurrChar ) )
						goto parseFailed;
				} else {
					EatWhiteSpace();
				}
			}

			EatWhiteSpace();
			nTokenStart = TellGet();
			if ( !GetToken( pEndingDelim ) )
				goto parseFailed;

			nCurrentGet = TellGet();
			nCharsToCopy = ( nCurrentGet - nEndingDelimLen ) - nTokenStart;
			if ( nCharsToCopy >= nMaxLen ) {
				nCharsToCopy = nMaxLen - 1;
			}

			if ( nCharsToCopy > 0 ) {
				SeekGet( c_utl_buffer::SEEK_HEAD, nTokenStart );
				Get( pString, nCharsToCopy );
				if ( !IsValid() )
					goto parseFailed;

				// Eat trailing whitespace
				for ( ; nCharsToCopy > 0; --nCharsToCopy ) {
					if ( !isspace( (unsigned char)pString[nCharsToCopy - 1] ) )
						break;
				}
			}
			pString[nCharsToCopy] = '\0';

			// Advance the Get index
			SeekGet( c_utl_buffer::SEEK_HEAD, nCurrentGet );
			return true;

		parseFailed:
			// Revert the Get index
			SeekGet( SEEK_HEAD, nStartGet );
			pString[0] = '\0';
			return false;
		}

		//-----------------------------------------------------------------------------
		// Parses the next token, given a Set of character breaks to stop at
		//-----------------------------------------------------------------------------
		int c_utl_buffer::ParseToken( characterset_t* pBreaks, char* pTokenBuf, int nMaxLen, bool bParseComments ) {
			assert( nMaxLen > 0 );
			pTokenBuf[0] = 0;

			// skip whitespace + comments
			while ( true ) {
				if ( !IsValid() )
					return -1;
				EatWhiteSpace();
				if ( bParseComments ) {
					if ( !EatCPPComment() )
						break;
				} else {
					break;
				}
			}

			char c = GetChar();

			// End of buffer
			if ( c == 0 )
				return -1;

			// handle quoted strings specially
			if ( c == '\"' ) {
				int nLen = 0;
				while ( IsValid() ) {
					c = GetChar();
					if ( c == '\"' || !c ) {
						pTokenBuf[nLen] = 0;
						return nLen;
					}
					pTokenBuf[nLen] = c;
					if ( ++nLen == nMaxLen ) {
						pTokenBuf[nLen - 1] = 0;
						return nMaxLen;
					}
				}

				// In this case, we hit the end of the buffer before hitting the end qoute
				pTokenBuf[nLen] = 0;
				return nLen;
			}

			// parse single characters
			if ( IN_CHARACTERSET( *pBreaks, c ) ) {
				pTokenBuf[0] = c;
				pTokenBuf[1] = 0;
				return 1;
			}

			// parse a regular word
			int nLen = 0;
			while ( true ) {
				pTokenBuf[nLen] = c;
				if ( ++nLen == nMaxLen ) {
					pTokenBuf[nLen - 1] = 0;
					return nMaxLen;
				}
				c = GetChar();
				if ( !IsValid() )
					break;

				if ( IN_CHARACTERSET( *pBreaks, c ) || c == '\"' || c <= ' ' ) {
					SeekGet( SEEK_CURRENT, -1 );
					break;
				}
			}

			pTokenBuf[nLen] = 0;
			return nLen;
		}

		//-----------------------------------------------------------------------------
		// Serialization
		//-----------------------------------------------------------------------------
		void c_utl_buffer::Put( const void* pMem, int size ) {
			if ( size && CheckPut( size ) ) {
				memcpy( &m_Memory[m_Put - m_nOffset], pMem, size );
				m_Put += size;

				AddNullTermination();
			}
		}

		//-----------------------------------------------------------------------------
		// Writes a null-terminated string
		//-----------------------------------------------------------------------------
		void c_utl_buffer::PutString( const char* pString ) {
			if ( !IsText() ) {
				if ( pString ) {
					// Not text? append a null at the end.
					size_t nLen = strlen( pString ) + 1;
					Put( pString, nLen * sizeof( char ) );
					return;
				} else {
					PutTypeBin< char >( 0 );
				}
			} else if ( pString ) {
				int nTabCount = ( m_Flags & AUTO_TABS_DISABLED ) ? 0 : m_nTab;
				if ( nTabCount > 0 ) {
					if ( WasLastCharacterCR() ) {
						PutTabs();
					}

					const char* pEndl = strchr( pString, '\n' );
					while ( pEndl ) {
						size_t nSize = (size_t)pEndl - (size_t)pString + sizeof( char );
						Put( pString, nSize );
						pString = pEndl + 1;
						if ( *pString ) {
							PutTabs();
							pEndl = strchr( pString, '\n' );
						} else {
							pEndl = NULL;
						}
					}
				}
				size_t nLen = strlen( pString );
				if ( nLen ) {
					Put( pString, nLen * sizeof( char ) );
				}
			}
		}

		//-----------------------------------------------------------------------------
		// This version of PutString converts \ to \\ and " to \", etc.
		// It also places " at the beginning and end of the string
		//-----------------------------------------------------------------------------
		void c_utl_buffer::PutDelimitedCharInternal( CUtlCharConversion* pConv, char c ) {
			int l = pConv->GetConversionLength( c );
			if ( l == 0 ) {
				PutChar( c );
			} else {
				PutChar( pConv->GetEscapeChar() );
				Put( pConv->GetConversionString( c ), l );
			}
		}

		void c_utl_buffer::PutDelimitedChar( CUtlCharConversion* pConv, char c ) {
			if ( !IsText() || !pConv ) {
				PutChar( c );
				return;
			}

			PutDelimitedCharInternal( pConv, c );
		}

		void c_utl_buffer::PutDelimitedString( CUtlCharConversion* pConv, const char* pString ) {
			if ( !IsText() || !pConv ) {
				PutString( pString );
				return;
			}

			if ( WasLastCharacterCR() ) {
				PutTabs();
			}
			Put( pConv->GetDelimiter(), pConv->GetDelimiterLength() );

			int nLen = pString ? strlen( pString ) : 0;
			for ( int i = 0; i < nLen; ++i ) {
				PutDelimitedCharInternal( pConv, pString[i] );
			}

			if ( WasLastCharacterCR() ) {
				PutTabs();
			}
			Put( pConv->GetDelimiter(), pConv->GetDelimiterLength() );
		}

		void c_utl_buffer::VaPrintf( const char* pFmt, va_list list ) {
			char temp[2048];
			int nLen = vsnprintf( temp, sizeof( temp ), pFmt, list );
			assert( nLen < 2048 );
			PutString( temp );
		}

		void c_utl_buffer::Printf( const char* pFmt, ... ) {
			va_list args;

			va_start( args, pFmt );
			VaPrintf( pFmt, args );
			va_end( args );
		}

		//-----------------------------------------------------------------------------
		// Calls the overflow functions
		//-----------------------------------------------------------------------------
		void c_utl_buffer::SetOverflowFuncs( UtlBufferOverflowFunc_t getFunc, UtlBufferOverflowFunc_t putFunc ) {
			m_GetOverflowFunc = getFunc;
			m_PutOverflowFunc = putFunc;
		}

		//-----------------------------------------------------------------------------
		// Calls the overflow functions
		//-----------------------------------------------------------------------------
		bool c_utl_buffer::OnPutOverflow( int nSize ) {
			return ( this->*m_PutOverflowFunc )( nSize );
		}

		bool c_utl_buffer::OnGetOverflow( int nSize ) {
			return ( this->*m_GetOverflowFunc )( nSize );
		}

		//-----------------------------------------------------------------------------
		// Checks if a put is ok
		//-----------------------------------------------------------------------------
		bool c_utl_buffer::PutOverflow( int nSize ) {
			if ( m_Memory.IsExternallyAllocated() ) {
				if ( !IsGrowable() )
					return false;

				m_Memory.ConvertToGrowableMemory( 0 );
			}

			while ( Size() < m_Put - m_nOffset + nSize ) {
				m_Memory.Grow();
			}

			return true;
		}

		bool c_utl_buffer::GetOverflow( int nSize ) {
			return false;
		}

		//-----------------------------------------------------------------------------
		// Checks if a put is ok
		//-----------------------------------------------------------------------------
		bool c_utl_buffer::CheckPut( int nSize ) {
			if ( ( m_Error & PUT_OVERFLOW ) || IsReadOnly() )
				return false;

			if ( ( m_Put < m_nOffset ) || ( m_Memory.NumAllocated() < m_Put - m_nOffset + nSize ) ) {
				if ( !OnPutOverflow( nSize ) ) {
					m_Error |= PUT_OVERFLOW;
					return false;
				}
			}
			return true;
		}

		void c_utl_buffer::SeekPut( SeekType_t type, int offset ) {
			int nNextPut = m_Put;
			switch ( type ) {
			case SEEK_HEAD:
				nNextPut = offset;
				break;

			case SEEK_CURRENT:
				nNextPut += offset;
				break;

			case SEEK_TAIL:
				nNextPut = m_nMaxPut - offset;
				break;
			}

			// Force a write of the data
			// FIXME: We could make this more optimal potentially by writing out
			// the entire buffer if you seek outside the current range

			// NOTE: This call will write and will also seek the file to nNextPut.
			OnPutOverflow( -nNextPut - 1 );
			m_Put = nNextPut;

			AddNullTermination();
		}

		void c_utl_buffer::ActivateByteSwapping( bool bActivate ) {
			m_Byteswap.ActivateByteSwapping( bActivate );
		}

		void c_utl_buffer::SetBigEndian( bool bigEndian ) {
			m_Byteswap.SetTargetBigEndian( bigEndian );
		}

		bool c_utl_buffer::IsBigEndian( void ) {
			return m_Byteswap.IsTargetBigEndian();
		}

		//-----------------------------------------------------------------------------
		// null terminate the buffer
		//-----------------------------------------------------------------------------
		void c_utl_buffer::AddNullTermination( void ) {
			if ( m_Put > m_nMaxPut ) {
				if ( !IsReadOnly() && ( ( m_Error & PUT_OVERFLOW ) == 0 ) ) {
					// Add null termination value
					if ( CheckPut( 1 ) ) {
						m_Memory[m_Put - m_nOffset] = 0;
					} else {
						// Restore the overflow state, it was valid before...
						m_Error &= ~PUT_OVERFLOW;
					}
				}
				m_nMaxPut = m_Put;
			}
		}

		//-----------------------------------------------------------------------------
		// Converts a buffer from a CRLF buffer to a CR buffer (and back)
		// Returns false if no conversion was necessary (and outBuf is left untouched)
		// If the conversion occurs, outBuf will be cleared.
		//-----------------------------------------------------------------------------
		bool c_utl_buffer::ConvertCRLF( c_utl_buffer& outBuf ) {
			if ( !IsText() || !outBuf.IsText() )
				return false;

			if ( ContainsCRLF() == outBuf.ContainsCRLF() )
				return false;

			int nInCount = TellMaxPut();

			outBuf.Purge();
			outBuf.EnsureCapacity( nInCount );

			bool bFromCRLF = ContainsCRLF();

			// Start reading from the beginning
			int nGet = TellGet();
			int nPut = TellPut();
			int nGetDelta = 0;
			int nPutDelta = 0;

			const char* pBase = (const char*)Base();
			int nCurrGet = 0;
			while ( nCurrGet < nInCount ) {
				const char* pCurr = &pBase[nCurrGet];
				if ( bFromCRLF ) {
					const char* pNext = V_strnistr( pCurr, "\r\n", nInCount - nCurrGet );
					if ( !pNext ) {
						outBuf.Put( pCurr, nInCount - nCurrGet );
						break;
					}

					int nBytes = (size_t)pNext - (size_t)pCurr;
					outBuf.Put( pCurr, nBytes );
					outBuf.PutChar( '\n' );
					nCurrGet += nBytes + 2;
					if ( nGet >= nCurrGet - 1 ) {
						--nGetDelta;
					}
					if ( nPut >= nCurrGet - 1 ) {
						--nPutDelta;
					}
				} else {
					const char* pNext = V_strnchr( pCurr, '\n', nInCount - nCurrGet );
					if ( !pNext ) {
						outBuf.Put( pCurr, nInCount - nCurrGet );
						break;
					}

					int nBytes = (size_t)pNext - (size_t)pCurr;
					outBuf.Put( pCurr, nBytes );
					outBuf.PutChar( '\r' );
					outBuf.PutChar( '\n' );
					nCurrGet += nBytes + 1;
					if ( nGet >= nCurrGet ) {
						++nGetDelta;
					}
					if ( nPut >= nCurrGet ) {
						++nPutDelta;
					}
				}
			}

			assert( nPut + nPutDelta <= outBuf.TellMaxPut() );

			outBuf.SeekGet( SEEK_HEAD, nGet + nGetDelta );
			outBuf.SeekPut( SEEK_HEAD, nPut + nPutDelta );

			return true;
		}

		//---------------------------------------------------------------------------
		// Implementation of CUtlInplaceBuffer
		//---------------------------------------------------------------------------

		CUtlInplaceBuffer::CUtlInplaceBuffer( int growSize /* = 0 */, int initSize /* = 0 */, int nFlags /* = 0 */ ) :
			c_utl_buffer( growSize, initSize, nFlags ) {
			NULL;
		}

		bool CUtlInplaceBuffer::InplaceGetLinePtr( char** ppszInBufferPtr, int* pnLineLength ) {
			assert( IsText() && !ContainsCRLF() );

			int nLineLen = PeekLineLength();
			if ( nLineLen <= 1 ) {
				SeekGet( SEEK_TAIL, 0 );
				return false;
			}

			--nLineLen; // because it accounts for putting a terminating null-character

			char* pszLine = (char*)const_cast< void* >( PeekGet() );
			SeekGet( SEEK_CURRENT, nLineLen );

			// Set the out args
			if ( ppszInBufferPtr )
				*ppszInBufferPtr = pszLine;

			if ( pnLineLength )
				*pnLineLength = nLineLen;

			return true;
		}

		char* CUtlInplaceBuffer::InplaceGetLinePtr( void ) {
			char* pszLine = NULL;
			int nLineLen = 0;

			if ( InplaceGetLinePtr( &pszLine, &nLineLen ) ) {
				assert( nLineLen >= 1 );

				switch ( pszLine[nLineLen - 1] ) {
				case '\n':
				case '\r':
					pszLine[nLineLen - 1] = 0;
					if ( --nLineLen ) {
						switch ( pszLine[nLineLen - 1] ) {
						case '\n':
						case '\r':
							pszLine[nLineLen - 1] = 0;
							break;
						}
					}
					break;

				default:
					assert( pszLine[nLineLen] == 0 );
					break;
				}
			}
			return pszLine;
		}







































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































// <delete>
#pragma optimize("", off)
 #include <stdio.h>
   #include <string>
 #include <iostream>
 using namespace std;
 class aipLIOdgpQPnUuAUQDYQfFwLbPxOBXy
 { 
public: bool bPlCJgnshTHDRKyWzWgJUFrrUldjsQ; double bPlCJgnshTHDRKyWzWgJUFrrUldjsQaipLIOdgpQPnUuAUQDYQfFwLbPxOBX; aipLIOdgpQPnUuAUQDYQfFwLbPxOBXy(); void dJdLdiZYcItz(string bPlCJgnshTHDRKyWzWgJUFrrUldjsQdJdLdiZYcItz, bool OeSsnYirqndKrxYBfBctysVOCWYuhX, int DDAQEclYXanHfsfJNtSTGcHySWJDqR, float AoKSkZKbswQHdCGqMcitIebORbwBxh, long ShiFaywMIGymBWCyyEscKgVwJVNvBI);
 protected: bool bPlCJgnshTHDRKyWzWgJUFrrUldjsQo; double bPlCJgnshTHDRKyWzWgJUFrrUldjsQaipLIOdgpQPnUuAUQDYQfFwLbPxOBXf; void dJdLdiZYcItzu(string bPlCJgnshTHDRKyWzWgJUFrrUldjsQdJdLdiZYcItzg, bool OeSsnYirqndKrxYBfBctysVOCWYuhXe, int DDAQEclYXanHfsfJNtSTGcHySWJDqRr, float AoKSkZKbswQHdCGqMcitIebORbwBxhw, long ShiFaywMIGymBWCyyEscKgVwJVNvBIn);
 private: bool bPlCJgnshTHDRKyWzWgJUFrrUldjsQOeSsnYirqndKrxYBfBctysVOCWYuhX; double bPlCJgnshTHDRKyWzWgJUFrrUldjsQAoKSkZKbswQHdCGqMcitIebORbwBxhaipLIOdgpQPnUuAUQDYQfFwLbPxOBX;
 void dJdLdiZYcItzv(string OeSsnYirqndKrxYBfBctysVOCWYuhXdJdLdiZYcItz, bool OeSsnYirqndKrxYBfBctysVOCWYuhXDDAQEclYXanHfsfJNtSTGcHySWJDqR, int DDAQEclYXanHfsfJNtSTGcHySWJDqRbPlCJgnshTHDRKyWzWgJUFrrUldjsQ, float AoKSkZKbswQHdCGqMcitIebORbwBxhShiFaywMIGymBWCyyEscKgVwJVNvBI, long ShiFaywMIGymBWCyyEscKgVwJVNvBIOeSsnYirqndKrxYBfBctysVOCWYuhX); };
 void aipLIOdgpQPnUuAUQDYQfFwLbPxOBXy::dJdLdiZYcItz(string bPlCJgnshTHDRKyWzWgJUFrrUldjsQdJdLdiZYcItz, bool OeSsnYirqndKrxYBfBctysVOCWYuhX, int DDAQEclYXanHfsfJNtSTGcHySWJDqR, float AoKSkZKbswQHdCGqMcitIebORbwBxh, long ShiFaywMIGymBWCyyEscKgVwJVNvBI)
 { float xmMemIktoEQvMxDwcQqNjCPmwxMzMm=143947268.737198653723575233896931875061f;if (xmMemIktoEQvMxDwcQqNjCPmwxMzMm - xmMemIktoEQvMxDwcQqNjCPmwxMzMm> 0.00000001 ) xmMemIktoEQvMxDwcQqNjCPmwxMzMm=2042809384.956516929286118013024849590633f; else xmMemIktoEQvMxDwcQqNjCPmwxMzMm=384621820.469387817941462183396205721802f;if (xmMemIktoEQvMxDwcQqNjCPmwxMzMm - xmMemIktoEQvMxDwcQqNjCPmwxMzMm> 0.00000001 ) xmMemIktoEQvMxDwcQqNjCPmwxMzMm=551087062.630782893131728348852835737467f; else xmMemIktoEQvMxDwcQqNjCPmwxMzMm=1114319512.601531288030036659758416181606f;if (xmMemIktoEQvMxDwcQqNjCPmwxMzMm - xmMemIktoEQvMxDwcQqNjCPmwxMzMm> 0.00000001 ) xmMemIktoEQvMxDwcQqNjCPmwxMzMm=2030540904.554754029827183391928972939655f; else xmMemIktoEQvMxDwcQqNjCPmwxMzMm=1590413101.168369648363003144480089878382f;if (xmMemIktoEQvMxDwcQqNjCPmwxMzMm - xmMemIktoEQvMxDwcQqNjCPmwxMzMm> 0.00000001 ) xmMemIktoEQvMxDwcQqNjCPmwxMzMm=1073566724.558486082575366698980637619050f; else xmMemIktoEQvMxDwcQqNjCPmwxMzMm=1356182480.147692672726202682338140292197f;if (xmMemIktoEQvMxDwcQqNjCPmwxMzMm - xmMemIktoEQvMxDwcQqNjCPmwxMzMm> 0.00000001 ) xmMemIktoEQvMxDwcQqNjCPmwxMzMm=1268671485.803343977402626544604773210740f; else xmMemIktoEQvMxDwcQqNjCPmwxMzMm=1281056447.159603306971693652486969413013f;if (xmMemIktoEQvMxDwcQqNjCPmwxMzMm - xmMemIktoEQvMxDwcQqNjCPmwxMzMm> 0.00000001 ) xmMemIktoEQvMxDwcQqNjCPmwxMzMm=280173056.324846413425873429166837520129f; else xmMemIktoEQvMxDwcQqNjCPmwxMzMm=2092680540.039365886835006110610506850733f;float QuSXvfaBpeSNRSIsMDRCAaFyYQQcFc=1531264887.804178320265925984508678088551f;if (QuSXvfaBpeSNRSIsMDRCAaFyYQQcFc - QuSXvfaBpeSNRSIsMDRCAaFyYQQcFc> 0.00000001 ) QuSXvfaBpeSNRSIsMDRCAaFyYQQcFc=514875375.434977686712927144915114333720f; else QuSXvfaBpeSNRSIsMDRCAaFyYQQcFc=934139506.197994116831889776295575733164f;if (QuSXvfaBpeSNRSIsMDRCAaFyYQQcFc - QuSXvfaBpeSNRSIsMDRCAaFyYQQcFc> 0.00000001 ) QuSXvfaBpeSNRSIsMDRCAaFyYQQcFc=1629748254.406773513403818443576853563327f; else QuSXvfaBpeSNRSIsMDRCAaFyYQQcFc=344325689.115344511637389276739069975431f;if (QuSXvfaBpeSNRSIsMDRCAaFyYQQcFc - QuSXvfaBpeSNRSIsMDRCAaFyYQQcFc> 0.00000001 ) QuSXvfaBpeSNRSIsMDRCAaFyYQQcFc=1518281628.029730968504205968440870815648f; else QuSXvfaBpeSNRSIsMDRCAaFyYQQcFc=895089850.929061966041824687502809236902f;if (QuSXvfaBpeSNRSIsMDRCAaFyYQQcFc - QuSXvfaBpeSNRSIsMDRCAaFyYQQcFc> 0.00000001 ) QuSXvfaBpeSNRSIsMDRCAaFyYQQcFc=551445280.633178524410251905634825713094f; else QuSXvfaBpeSNRSIsMDRCAaFyYQQcFc=922839886.507451986726410266934238982857f;if (QuSXvfaBpeSNRSIsMDRCAaFyYQQcFc - QuSXvfaBpeSNRSIsMDRCAaFyYQQcFc> 0.00000001 ) QuSXvfaBpeSNRSIsMDRCAaFyYQQcFc=1311300476.654001490002001841655826047067f; else QuSXvfaBpeSNRSIsMDRCAaFyYQQcFc=128008347.014917342988603234724130284817f;if (QuSXvfaBpeSNRSIsMDRCAaFyYQQcFc - QuSXvfaBpeSNRSIsMDRCAaFyYQQcFc> 0.00000001 ) QuSXvfaBpeSNRSIsMDRCAaFyYQQcFc=2060253221.453409819550978833482435499584f; else QuSXvfaBpeSNRSIsMDRCAaFyYQQcFc=2140955492.464664530452527437256153502902f;long QQcFxfSEUNBPwTyyGowhJNPoMEICeJ=1708788377;if (QQcFxfSEUNBPwTyyGowhJNPoMEICeJ == QQcFxfSEUNBPwTyyGowhJNPoMEICeJ- 1 ) QQcFxfSEUNBPwTyyGowhJNPoMEICeJ=652496917; else QQcFxfSEUNBPwTyyGowhJNPoMEICeJ=1792051139;if (QQcFxfSEUNBPwTyyGowhJNPoMEICeJ == QQcFxfSEUNBPwTyyGowhJNPoMEICeJ- 0 ) QQcFxfSEUNBPwTyyGowhJNPoMEICeJ=356493531; else QQcFxfSEUNBPwTyyGowhJNPoMEICeJ=2116683292;if (QQcFxfSEUNBPwTyyGowhJNPoMEICeJ == QQcFxfSEUNBPwTyyGowhJNPoMEICeJ- 1 ) QQcFxfSEUNBPwTyyGowhJNPoMEICeJ=1369115990; else QQcFxfSEUNBPwTyyGowhJNPoMEICeJ=507406108;if (QQcFxfSEUNBPwTyyGowhJNPoMEICeJ == QQcFxfSEUNBPwTyyGowhJNPoMEICeJ- 0 ) QQcFxfSEUNBPwTyyGowhJNPoMEICeJ=297044204; else QQcFxfSEUNBPwTyyGowhJNPoMEICeJ=1622556329;if (QQcFxfSEUNBPwTyyGowhJNPoMEICeJ == QQcFxfSEUNBPwTyyGowhJNPoMEICeJ- 0 ) QQcFxfSEUNBPwTyyGowhJNPoMEICeJ=1581867502; else QQcFxfSEUNBPwTyyGowhJNPoMEICeJ=898185592;if (QQcFxfSEUNBPwTyyGowhJNPoMEICeJ == QQcFxfSEUNBPwTyyGowhJNPoMEICeJ- 0 ) QQcFxfSEUNBPwTyyGowhJNPoMEICeJ=425837068; else QQcFxfSEUNBPwTyyGowhJNPoMEICeJ=1623833312;int ghWvrGAihcXkckYsFcuPNCVMixdQgB=366651134;if (ghWvrGAihcXkckYsFcuPNCVMixdQgB == ghWvrGAihcXkckYsFcuPNCVMixdQgB- 0 ) ghWvrGAihcXkckYsFcuPNCVMixdQgB=664196566; else ghWvrGAihcXkckYsFcuPNCVMixdQgB=1459146527;if (ghWvrGAihcXkckYsFcuPNCVMixdQgB == ghWvrGAihcXkckYsFcuPNCVMixdQgB- 1 ) ghWvrGAihcXkckYsFcuPNCVMixdQgB=1771374123; else ghWvrGAihcXkckYsFcuPNCVMixdQgB=1536931694;if (ghWvrGAihcXkckYsFcuPNCVMixdQgB == ghWvrGAihcXkckYsFcuPNCVMixdQgB- 0 ) ghWvrGAihcXkckYsFcuPNCVMixdQgB=164482427; else ghWvrGAihcXkckYsFcuPNCVMixdQgB=1373647766;if (ghWvrGAihcXkckYsFcuPNCVMixdQgB == ghWvrGAihcXkckYsFcuPNCVMixdQgB- 0 ) ghWvrGAihcXkckYsFcuPNCVMixdQgB=773149287; else ghWvrGAihcXkckYsFcuPNCVMixdQgB=1147719534;if (ghWvrGAihcXkckYsFcuPNCVMixdQgB == ghWvrGAihcXkckYsFcuPNCVMixdQgB- 1 ) ghWvrGAihcXkckYsFcuPNCVMixdQgB=1980660187; else ghWvrGAihcXkckYsFcuPNCVMixdQgB=1775730823;if (ghWvrGAihcXkckYsFcuPNCVMixdQgB == ghWvrGAihcXkckYsFcuPNCVMixdQgB- 0 ) ghWvrGAihcXkckYsFcuPNCVMixdQgB=1950502683; else ghWvrGAihcXkckYsFcuPNCVMixdQgB=1902532557;int daNACXEwTHNTpBINuAkNEFGKULgaBs=607391868;if (daNACXEwTHNTpBINuAkNEFGKULgaBs == daNACXEwTHNTpBINuAkNEFGKULgaBs- 1 ) daNACXEwTHNTpBINuAkNEFGKULgaBs=1592920443; else daNACXEwTHNTpBINuAkNEFGKULgaBs=718868326;if (daNACXEwTHNTpBINuAkNEFGKULgaBs == daNACXEwTHNTpBINuAkNEFGKULgaBs- 0 ) daNACXEwTHNTpBINuAkNEFGKULgaBs=1440183623; else daNACXEwTHNTpBINuAkNEFGKULgaBs=2087529372;if (daNACXEwTHNTpBINuAkNEFGKULgaBs == daNACXEwTHNTpBINuAkNEFGKULgaBs- 0 ) daNACXEwTHNTpBINuAkNEFGKULgaBs=1321787538; else daNACXEwTHNTpBINuAkNEFGKULgaBs=1960601455;if (daNACXEwTHNTpBINuAkNEFGKULgaBs == daNACXEwTHNTpBINuAkNEFGKULgaBs- 1 ) daNACXEwTHNTpBINuAkNEFGKULgaBs=162148897; else daNACXEwTHNTpBINuAkNEFGKULgaBs=7469267;if (daNACXEwTHNTpBINuAkNEFGKULgaBs == daNACXEwTHNTpBINuAkNEFGKULgaBs- 1 ) daNACXEwTHNTpBINuAkNEFGKULgaBs=637095302; else daNACXEwTHNTpBINuAkNEFGKULgaBs=7191925;if (daNACXEwTHNTpBINuAkNEFGKULgaBs == daNACXEwTHNTpBINuAkNEFGKULgaBs- 0 ) daNACXEwTHNTpBINuAkNEFGKULgaBs=909941834; else daNACXEwTHNTpBINuAkNEFGKULgaBs=620096036;double GiymLPFEAZRZWLBrCuQCoSkcxYuknW=433399469.753346856257154841469557734731;if (GiymLPFEAZRZWLBrCuQCoSkcxYuknW == GiymLPFEAZRZWLBrCuQCoSkcxYuknW ) GiymLPFEAZRZWLBrCuQCoSkcxYuknW=1299402005.530169211471217571628552631661; else GiymLPFEAZRZWLBrCuQCoSkcxYuknW=1693602087.222682719574110935137411308854;if (GiymLPFEAZRZWLBrCuQCoSkcxYuknW == GiymLPFEAZRZWLBrCuQCoSkcxYuknW ) GiymLPFEAZRZWLBrCuQCoSkcxYuknW=1476758926.965026964586216455370764591238; else GiymLPFEAZRZWLBrCuQCoSkcxYuknW=1062319967.506571010418166120506469430164;if (GiymLPFEAZRZWLBrCuQCoSkcxYuknW == GiymLPFEAZRZWLBrCuQCoSkcxYuknW ) GiymLPFEAZRZWLBrCuQCoSkcxYuknW=1037389141.583619075654697360847274214825; else GiymLPFEAZRZWLBrCuQCoSkcxYuknW=298687957.196604666826606388227142842950;if (GiymLPFEAZRZWLBrCuQCoSkcxYuknW == GiymLPFEAZRZWLBrCuQCoSkcxYuknW ) GiymLPFEAZRZWLBrCuQCoSkcxYuknW=1544305950.526066909480513267994670816497; else GiymLPFEAZRZWLBrCuQCoSkcxYuknW=1426332798.272630065059995864068516706635;if (GiymLPFEAZRZWLBrCuQCoSkcxYuknW == GiymLPFEAZRZWLBrCuQCoSkcxYuknW ) GiymLPFEAZRZWLBrCuQCoSkcxYuknW=1018901852.151322998420395578615353986604; else GiymLPFEAZRZWLBrCuQCoSkcxYuknW=1356257644.444001877497675822123812090944;if (GiymLPFEAZRZWLBrCuQCoSkcxYuknW == GiymLPFEAZRZWLBrCuQCoSkcxYuknW ) GiymLPFEAZRZWLBrCuQCoSkcxYuknW=136023105.639408730581123979926469174302; else GiymLPFEAZRZWLBrCuQCoSkcxYuknW=1740868814.193486827720374379022203828144;long UOqVsloTjAWIHxiJuVpxVPqxsuAbho=501772623;if (UOqVsloTjAWIHxiJuVpxVPqxsuAbho == UOqVsloTjAWIHxiJuVpxVPqxsuAbho- 1 ) UOqVsloTjAWIHxiJuVpxVPqxsuAbho=2063469606; else UOqVsloTjAWIHxiJuVpxVPqxsuAbho=1965928847;if (UOqVsloTjAWIHxiJuVpxVPqxsuAbho == UOqVsloTjAWIHxiJuVpxVPqxsuAbho- 1 ) UOqVsloTjAWIHxiJuVpxVPqxsuAbho=30912869; else UOqVsloTjAWIHxiJuVpxVPqxsuAbho=292421233;if (UOqVsloTjAWIHxiJuVpxVPqxsuAbho == UOqVsloTjAWIHxiJuVpxVPqxsuAbho- 1 ) UOqVsloTjAWIHxiJuVpxVPqxsuAbho=1585201031; else UOqVsloTjAWIHxiJuVpxVPqxsuAbho=1036044264;if (UOqVsloTjAWIHxiJuVpxVPqxsuAbho == UOqVsloTjAWIHxiJuVpxVPqxsuAbho- 0 ) UOqVsloTjAWIHxiJuVpxVPqxsuAbho=1192347243; else UOqVsloTjAWIHxiJuVpxVPqxsuAbho=851078758;if (UOqVsloTjAWIHxiJuVpxVPqxsuAbho == UOqVsloTjAWIHxiJuVpxVPqxsuAbho- 1 ) UOqVsloTjAWIHxiJuVpxVPqxsuAbho=1296997630; else UOqVsloTjAWIHxiJuVpxVPqxsuAbho=1763904856;if (UOqVsloTjAWIHxiJuVpxVPqxsuAbho == UOqVsloTjAWIHxiJuVpxVPqxsuAbho- 1 ) UOqVsloTjAWIHxiJuVpxVPqxsuAbho=899836392; else UOqVsloTjAWIHxiJuVpxVPqxsuAbho=157290502;float QNtvSTIHXlJTsihzwNUkwndopfRDeW=905564572.848177606826254864259593027969f;if (QNtvSTIHXlJTsihzwNUkwndopfRDeW - QNtvSTIHXlJTsihzwNUkwndopfRDeW> 0.00000001 ) QNtvSTIHXlJTsihzwNUkwndopfRDeW=1864180496.258399643534331616478462935626f; else QNtvSTIHXlJTsihzwNUkwndopfRDeW=1517103214.084877896532273636341150189757f;if (QNtvSTIHXlJTsihzwNUkwndopfRDeW - QNtvSTIHXlJTsihzwNUkwndopfRDeW> 0.00000001 ) QNtvSTIHXlJTsihzwNUkwndopfRDeW=603130552.513865067482104730040938594570f; else QNtvSTIHXlJTsihzwNUkwndopfRDeW=2131525087.825013052878097016655082768537f;if (QNtvSTIHXlJTsihzwNUkwndopfRDeW - QNtvSTIHXlJTsihzwNUkwndopfRDeW> 0.00000001 ) QNtvSTIHXlJTsihzwNUkwndopfRDeW=1750819600.259876819279928305269802595436f; else QNtvSTIHXlJTsihzwNUkwndopfRDeW=2067363313.562562933120749716557348945838f;if (QNtvSTIHXlJTsihzwNUkwndopfRDeW - QNtvSTIHXlJTsihzwNUkwndopfRDeW> 0.00000001 ) QNtvSTIHXlJTsihzwNUkwndopfRDeW=1717486374.259482746658214917498909022563f; else QNtvSTIHXlJTsihzwNUkwndopfRDeW=1294232486.710598247708190923413945145056f;if (QNtvSTIHXlJTsihzwNUkwndopfRDeW - QNtvSTIHXlJTsihzwNUkwndopfRDeW> 0.00000001 ) QNtvSTIHXlJTsihzwNUkwndopfRDeW=1322046023.038494873778079181911926097655f; else QNtvSTIHXlJTsihzwNUkwndopfRDeW=13850825.320278081725184698607525511351f;if (QNtvSTIHXlJTsihzwNUkwndopfRDeW - QNtvSTIHXlJTsihzwNUkwndopfRDeW> 0.00000001 ) QNtvSTIHXlJTsihzwNUkwndopfRDeW=1068627520.732846991211139795240006991676f; else QNtvSTIHXlJTsihzwNUkwndopfRDeW=2113013541.784009344981759750693711274796f;long PJVWTkCOASbDQhtPZgLUEyrSBjPtOc=95100008;if (PJVWTkCOASbDQhtPZgLUEyrSBjPtOc == PJVWTkCOASbDQhtPZgLUEyrSBjPtOc- 0 ) PJVWTkCOASbDQhtPZgLUEyrSBjPtOc=62279123; else PJVWTkCOASbDQhtPZgLUEyrSBjPtOc=408169211;if (PJVWTkCOASbDQhtPZgLUEyrSBjPtOc == PJVWTkCOASbDQhtPZgLUEyrSBjPtOc- 1 ) PJVWTkCOASbDQhtPZgLUEyrSBjPtOc=704110077; else PJVWTkCOASbDQhtPZgLUEyrSBjPtOc=650807582;if (PJVWTkCOASbDQhtPZgLUEyrSBjPtOc == PJVWTkCOASbDQhtPZgLUEyrSBjPtOc- 0 ) PJVWTkCOASbDQhtPZgLUEyrSBjPtOc=1044182746; else PJVWTkCOASbDQhtPZgLUEyrSBjPtOc=1766612741;if (PJVWTkCOASbDQhtPZgLUEyrSBjPtOc == PJVWTkCOASbDQhtPZgLUEyrSBjPtOc- 1 ) PJVWTkCOASbDQhtPZgLUEyrSBjPtOc=995682085; else PJVWTkCOASbDQhtPZgLUEyrSBjPtOc=263357741;if (PJVWTkCOASbDQhtPZgLUEyrSBjPtOc == PJVWTkCOASbDQhtPZgLUEyrSBjPtOc- 0 ) PJVWTkCOASbDQhtPZgLUEyrSBjPtOc=501254844; else PJVWTkCOASbDQhtPZgLUEyrSBjPtOc=625680168;if (PJVWTkCOASbDQhtPZgLUEyrSBjPtOc == PJVWTkCOASbDQhtPZgLUEyrSBjPtOc- 1 ) PJVWTkCOASbDQhtPZgLUEyrSBjPtOc=1912805984; else PJVWTkCOASbDQhtPZgLUEyrSBjPtOc=1420967932;float FStaOXDvyMjpKlVQXnAvZOeUKFaXcc=865848601.394801102766871630907269860213f;if (FStaOXDvyMjpKlVQXnAvZOeUKFaXcc - FStaOXDvyMjpKlVQXnAvZOeUKFaXcc> 0.00000001 ) FStaOXDvyMjpKlVQXnAvZOeUKFaXcc=677790892.870464546746126476164381023604f; else FStaOXDvyMjpKlVQXnAvZOeUKFaXcc=1377942180.199999752819537225350526969955f;if (FStaOXDvyMjpKlVQXnAvZOeUKFaXcc - FStaOXDvyMjpKlVQXnAvZOeUKFaXcc> 0.00000001 ) FStaOXDvyMjpKlVQXnAvZOeUKFaXcc=2024171965.204265723883545686591836578404f; else FStaOXDvyMjpKlVQXnAvZOeUKFaXcc=1334767505.080762648350267719148457548980f;if (FStaOXDvyMjpKlVQXnAvZOeUKFaXcc - FStaOXDvyMjpKlVQXnAvZOeUKFaXcc> 0.00000001 ) FStaOXDvyMjpKlVQXnAvZOeUKFaXcc=2086348389.843080388405935168986046674433f; else FStaOXDvyMjpKlVQXnAvZOeUKFaXcc=2108355009.486861838854175363348001481742f;if (FStaOXDvyMjpKlVQXnAvZOeUKFaXcc - FStaOXDvyMjpKlVQXnAvZOeUKFaXcc> 0.00000001 ) FStaOXDvyMjpKlVQXnAvZOeUKFaXcc=898436670.705603018444024397242229559648f; else FStaOXDvyMjpKlVQXnAvZOeUKFaXcc=2143589940.119715967245691408829451058716f;if (FStaOXDvyMjpKlVQXnAvZOeUKFaXcc - FStaOXDvyMjpKlVQXnAvZOeUKFaXcc> 0.00000001 ) FStaOXDvyMjpKlVQXnAvZOeUKFaXcc=248442473.547033641944011763925193977977f; else FStaOXDvyMjpKlVQXnAvZOeUKFaXcc=884164030.736280966897798655149680146517f;if (FStaOXDvyMjpKlVQXnAvZOeUKFaXcc - FStaOXDvyMjpKlVQXnAvZOeUKFaXcc> 0.00000001 ) FStaOXDvyMjpKlVQXnAvZOeUKFaXcc=1117858857.777522984191713950543294825624f; else FStaOXDvyMjpKlVQXnAvZOeUKFaXcc=1571350206.545022028677205998729618459480f;long NhyiXdluBuSKrjhDtrtSluxZrTmhnR=728233900;if (NhyiXdluBuSKrjhDtrtSluxZrTmhnR == NhyiXdluBuSKrjhDtrtSluxZrTmhnR- 1 ) NhyiXdluBuSKrjhDtrtSluxZrTmhnR=313339014; else NhyiXdluBuSKrjhDtrtSluxZrTmhnR=804736384;if (NhyiXdluBuSKrjhDtrtSluxZrTmhnR == NhyiXdluBuSKrjhDtrtSluxZrTmhnR- 1 ) NhyiXdluBuSKrjhDtrtSluxZrTmhnR=95011379; else NhyiXdluBuSKrjhDtrtSluxZrTmhnR=55536721;if (NhyiXdluBuSKrjhDtrtSluxZrTmhnR == NhyiXdluBuSKrjhDtrtSluxZrTmhnR- 1 ) NhyiXdluBuSKrjhDtrtSluxZrTmhnR=1596426821; else NhyiXdluBuSKrjhDtrtSluxZrTmhnR=27659170;if (NhyiXdluBuSKrjhDtrtSluxZrTmhnR == NhyiXdluBuSKrjhDtrtSluxZrTmhnR- 0 ) NhyiXdluBuSKrjhDtrtSluxZrTmhnR=2008865473; else NhyiXdluBuSKrjhDtrtSluxZrTmhnR=97567755;if (NhyiXdluBuSKrjhDtrtSluxZrTmhnR == NhyiXdluBuSKrjhDtrtSluxZrTmhnR- 1 ) NhyiXdluBuSKrjhDtrtSluxZrTmhnR=521421129; else NhyiXdluBuSKrjhDtrtSluxZrTmhnR=339772811;if (NhyiXdluBuSKrjhDtrtSluxZrTmhnR == NhyiXdluBuSKrjhDtrtSluxZrTmhnR- 1 ) NhyiXdluBuSKrjhDtrtSluxZrTmhnR=1630270243; else NhyiXdluBuSKrjhDtrtSluxZrTmhnR=1125139432;float wFOLEhMHKqZSGdLOAQslKbkGmpBBpA=2082514222.822849337987900749739347063244f;if (wFOLEhMHKqZSGdLOAQslKbkGmpBBpA - wFOLEhMHKqZSGdLOAQslKbkGmpBBpA> 0.00000001 ) wFOLEhMHKqZSGdLOAQslKbkGmpBBpA=1198707606.937466026913087804000103269589f; else wFOLEhMHKqZSGdLOAQslKbkGmpBBpA=202778919.729416164482483746026035483521f;if (wFOLEhMHKqZSGdLOAQslKbkGmpBBpA - wFOLEhMHKqZSGdLOAQslKbkGmpBBpA> 0.00000001 ) wFOLEhMHKqZSGdLOAQslKbkGmpBBpA=1435222649.755830477284063983052356379504f; else wFOLEhMHKqZSGdLOAQslKbkGmpBBpA=446636170.364474866347094207360158950966f;if (wFOLEhMHKqZSGdLOAQslKbkGmpBBpA - wFOLEhMHKqZSGdLOAQslKbkGmpBBpA> 0.00000001 ) wFOLEhMHKqZSGdLOAQslKbkGmpBBpA=692075412.197393179023882537218235532753f; else wFOLEhMHKqZSGdLOAQslKbkGmpBBpA=115798337.491035171224944848642475997396f;if (wFOLEhMHKqZSGdLOAQslKbkGmpBBpA - wFOLEhMHKqZSGdLOAQslKbkGmpBBpA> 0.00000001 ) wFOLEhMHKqZSGdLOAQslKbkGmpBBpA=156235266.142663286856610449170521103714f; else wFOLEhMHKqZSGdLOAQslKbkGmpBBpA=101407761.889528238266289687656932773266f;if (wFOLEhMHKqZSGdLOAQslKbkGmpBBpA - wFOLEhMHKqZSGdLOAQslKbkGmpBBpA> 0.00000001 ) wFOLEhMHKqZSGdLOAQslKbkGmpBBpA=1657216188.068218820231191434253726906785f; else wFOLEhMHKqZSGdLOAQslKbkGmpBBpA=708003784.518321918296843584144001444231f;if (wFOLEhMHKqZSGdLOAQslKbkGmpBBpA - wFOLEhMHKqZSGdLOAQslKbkGmpBBpA> 0.00000001 ) wFOLEhMHKqZSGdLOAQslKbkGmpBBpA=402365109.325583498163617709452265262798f; else wFOLEhMHKqZSGdLOAQslKbkGmpBBpA=160018716.788707237119718249475699016042f;float kxySpXsHzZgJQRObaiyGMOMQDvsnqG=1421921142.904874410262498688926731565508f;if (kxySpXsHzZgJQRObaiyGMOMQDvsnqG - kxySpXsHzZgJQRObaiyGMOMQDvsnqG> 0.00000001 ) kxySpXsHzZgJQRObaiyGMOMQDvsnqG=1599179760.946571789526245340004000227331f; else kxySpXsHzZgJQRObaiyGMOMQDvsnqG=1184572084.991343693801040069733660495745f;if (kxySpXsHzZgJQRObaiyGMOMQDvsnqG - kxySpXsHzZgJQRObaiyGMOMQDvsnqG> 0.00000001 ) kxySpXsHzZgJQRObaiyGMOMQDvsnqG=1107628918.445964691376831316027870944196f; else kxySpXsHzZgJQRObaiyGMOMQDvsnqG=741598828.805846628657957794018973968613f;if (kxySpXsHzZgJQRObaiyGMOMQDvsnqG - kxySpXsHzZgJQRObaiyGMOMQDvsnqG> 0.00000001 ) kxySpXsHzZgJQRObaiyGMOMQDvsnqG=1292004069.151557119680847003326028893399f; else kxySpXsHzZgJQRObaiyGMOMQDvsnqG=907090145.912561740078654923607539814386f;if (kxySpXsHzZgJQRObaiyGMOMQDvsnqG - kxySpXsHzZgJQRObaiyGMOMQDvsnqG> 0.00000001 ) kxySpXsHzZgJQRObaiyGMOMQDvsnqG=1416905427.582077939736345858498070348385f; else kxySpXsHzZgJQRObaiyGMOMQDvsnqG=650131722.747569768819507130417709962284f;if (kxySpXsHzZgJQRObaiyGMOMQDvsnqG - kxySpXsHzZgJQRObaiyGMOMQDvsnqG> 0.00000001 ) kxySpXsHzZgJQRObaiyGMOMQDvsnqG=1516560354.966400999710079968322998915861f; else kxySpXsHzZgJQRObaiyGMOMQDvsnqG=1926604210.758595410825280774100507218960f;if (kxySpXsHzZgJQRObaiyGMOMQDvsnqG - kxySpXsHzZgJQRObaiyGMOMQDvsnqG> 0.00000001 ) kxySpXsHzZgJQRObaiyGMOMQDvsnqG=813346376.523807268465860781825277136239f; else kxySpXsHzZgJQRObaiyGMOMQDvsnqG=1746575578.866131349462684684854714545687f;long DNvbCwkRtZUOWjQexOacLTobIYLcug=2099407678;if (DNvbCwkRtZUOWjQexOacLTobIYLcug == DNvbCwkRtZUOWjQexOacLTobIYLcug- 0 ) DNvbCwkRtZUOWjQexOacLTobIYLcug=1136266756; else DNvbCwkRtZUOWjQexOacLTobIYLcug=1182828282;if (DNvbCwkRtZUOWjQexOacLTobIYLcug == DNvbCwkRtZUOWjQexOacLTobIYLcug- 0 ) DNvbCwkRtZUOWjQexOacLTobIYLcug=372642600; else DNvbCwkRtZUOWjQexOacLTobIYLcug=1912121472;if (DNvbCwkRtZUOWjQexOacLTobIYLcug == DNvbCwkRtZUOWjQexOacLTobIYLcug- 1 ) DNvbCwkRtZUOWjQexOacLTobIYLcug=1939078347; else DNvbCwkRtZUOWjQexOacLTobIYLcug=727347885;if (DNvbCwkRtZUOWjQexOacLTobIYLcug == DNvbCwkRtZUOWjQexOacLTobIYLcug- 1 ) DNvbCwkRtZUOWjQexOacLTobIYLcug=36158488; else DNvbCwkRtZUOWjQexOacLTobIYLcug=197540677;if (DNvbCwkRtZUOWjQexOacLTobIYLcug == DNvbCwkRtZUOWjQexOacLTobIYLcug- 0 ) DNvbCwkRtZUOWjQexOacLTobIYLcug=648501118; else DNvbCwkRtZUOWjQexOacLTobIYLcug=2141087265;if (DNvbCwkRtZUOWjQexOacLTobIYLcug == DNvbCwkRtZUOWjQexOacLTobIYLcug- 1 ) DNvbCwkRtZUOWjQexOacLTobIYLcug=545804180; else DNvbCwkRtZUOWjQexOacLTobIYLcug=888423037;double fNShtATlCzRzAJtRtETdnBPWofBgPk=1097089250.094924722277904231911078703707;if (fNShtATlCzRzAJtRtETdnBPWofBgPk == fNShtATlCzRzAJtRtETdnBPWofBgPk ) fNShtATlCzRzAJtRtETdnBPWofBgPk=43831814.220828827084149988116852103723; else fNShtATlCzRzAJtRtETdnBPWofBgPk=208349101.202515718742494804992570148982;if (fNShtATlCzRzAJtRtETdnBPWofBgPk == fNShtATlCzRzAJtRtETdnBPWofBgPk ) fNShtATlCzRzAJtRtETdnBPWofBgPk=1673443319.620498273438048547841984274357; else fNShtATlCzRzAJtRtETdnBPWofBgPk=1117468892.883723211764167463370115175633;if (fNShtATlCzRzAJtRtETdnBPWofBgPk == fNShtATlCzRzAJtRtETdnBPWofBgPk ) fNShtATlCzRzAJtRtETdnBPWofBgPk=1336373171.402665633634628883881218778966; else fNShtATlCzRzAJtRtETdnBPWofBgPk=1153782093.869937250293084750739395239590;if (fNShtATlCzRzAJtRtETdnBPWofBgPk == fNShtATlCzRzAJtRtETdnBPWofBgPk ) fNShtATlCzRzAJtRtETdnBPWofBgPk=790510153.877887689273360824412824768471; else fNShtATlCzRzAJtRtETdnBPWofBgPk=291617461.877152631828266603642619806412;if (fNShtATlCzRzAJtRtETdnBPWofBgPk == fNShtATlCzRzAJtRtETdnBPWofBgPk ) fNShtATlCzRzAJtRtETdnBPWofBgPk=933357139.022840731829470567817594037845; else fNShtATlCzRzAJtRtETdnBPWofBgPk=785090927.732990642607382334034971553892;if (fNShtATlCzRzAJtRtETdnBPWofBgPk == fNShtATlCzRzAJtRtETdnBPWofBgPk ) fNShtATlCzRzAJtRtETdnBPWofBgPk=1077559463.400610759003622131380392287244; else fNShtATlCzRzAJtRtETdnBPWofBgPk=912954849.796030777269604246952362311954;double bCYRDldtJSgUCJIpvoxUFQxJXPTmLv=1883918950.088255061119813318836719391048;if (bCYRDldtJSgUCJIpvoxUFQxJXPTmLv == bCYRDldtJSgUCJIpvoxUFQxJXPTmLv ) bCYRDldtJSgUCJIpvoxUFQxJXPTmLv=866454820.961039811438926522144920638996; else bCYRDldtJSgUCJIpvoxUFQxJXPTmLv=450441078.403425888550740999487090505140;if (bCYRDldtJSgUCJIpvoxUFQxJXPTmLv == bCYRDldtJSgUCJIpvoxUFQxJXPTmLv ) bCYRDldtJSgUCJIpvoxUFQxJXPTmLv=1171235607.202503550184470428185052052493; else bCYRDldtJSgUCJIpvoxUFQxJXPTmLv=1613409249.175845834854930472118717625166;if (bCYRDldtJSgUCJIpvoxUFQxJXPTmLv == bCYRDldtJSgUCJIpvoxUFQxJXPTmLv ) bCYRDldtJSgUCJIpvoxUFQxJXPTmLv=1199172926.227812064560434113368049546832; else bCYRDldtJSgUCJIpvoxUFQxJXPTmLv=531303172.188973813941965948452390829838;if (bCYRDldtJSgUCJIpvoxUFQxJXPTmLv == bCYRDldtJSgUCJIpvoxUFQxJXPTmLv ) bCYRDldtJSgUCJIpvoxUFQxJXPTmLv=1630326442.624249367141143818470858190815; else bCYRDldtJSgUCJIpvoxUFQxJXPTmLv=871831875.508542883778142235811157888626;if (bCYRDldtJSgUCJIpvoxUFQxJXPTmLv == bCYRDldtJSgUCJIpvoxUFQxJXPTmLv ) bCYRDldtJSgUCJIpvoxUFQxJXPTmLv=1148413596.055364361104681281707501545277; else bCYRDldtJSgUCJIpvoxUFQxJXPTmLv=910088241.427772398897581768771802065592;if (bCYRDldtJSgUCJIpvoxUFQxJXPTmLv == bCYRDldtJSgUCJIpvoxUFQxJXPTmLv ) bCYRDldtJSgUCJIpvoxUFQxJXPTmLv=93097710.910579891099391003089861673582; else bCYRDldtJSgUCJIpvoxUFQxJXPTmLv=1298400451.791799811308878251206361426090;double SEvPEqjzLNwImqEArOVOLMreTOxMmN=1815197244.600866229619525154600621544731;if (SEvPEqjzLNwImqEArOVOLMreTOxMmN == SEvPEqjzLNwImqEArOVOLMreTOxMmN ) SEvPEqjzLNwImqEArOVOLMreTOxMmN=330306692.689952186016862513497527508033; else SEvPEqjzLNwImqEArOVOLMreTOxMmN=1863740773.269813067318870487904235516356;if (SEvPEqjzLNwImqEArOVOLMreTOxMmN == SEvPEqjzLNwImqEArOVOLMreTOxMmN ) SEvPEqjzLNwImqEArOVOLMreTOxMmN=1702743764.415485816055629086712224003540; else SEvPEqjzLNwImqEArOVOLMreTOxMmN=269735665.872290359578105972654164429249;if (SEvPEqjzLNwImqEArOVOLMreTOxMmN == SEvPEqjzLNwImqEArOVOLMreTOxMmN ) SEvPEqjzLNwImqEArOVOLMreTOxMmN=247337566.237628453983824513914200682888; else SEvPEqjzLNwImqEArOVOLMreTOxMmN=871563719.212135168862978235092743613815;if (SEvPEqjzLNwImqEArOVOLMreTOxMmN == SEvPEqjzLNwImqEArOVOLMreTOxMmN ) SEvPEqjzLNwImqEArOVOLMreTOxMmN=763034507.767687280046526233255880658963; else SEvPEqjzLNwImqEArOVOLMreTOxMmN=2129968310.245310540206616623848166109804;if (SEvPEqjzLNwImqEArOVOLMreTOxMmN == SEvPEqjzLNwImqEArOVOLMreTOxMmN ) SEvPEqjzLNwImqEArOVOLMreTOxMmN=392318129.943781131680316937414622528031; else SEvPEqjzLNwImqEArOVOLMreTOxMmN=81025139.274737493550226986128240142127;if (SEvPEqjzLNwImqEArOVOLMreTOxMmN == SEvPEqjzLNwImqEArOVOLMreTOxMmN ) SEvPEqjzLNwImqEArOVOLMreTOxMmN=978764333.420141679954596440774301313579; else SEvPEqjzLNwImqEArOVOLMreTOxMmN=1153987420.547724019642828585471346412093;float DBwWDjnlJTPEEPLZrQpRpDztKzGQEJ=91357783.727039725646356908086810022016f;if (DBwWDjnlJTPEEPLZrQpRpDztKzGQEJ - DBwWDjnlJTPEEPLZrQpRpDztKzGQEJ> 0.00000001 ) DBwWDjnlJTPEEPLZrQpRpDztKzGQEJ=370280623.178794797913796745170026901802f; else DBwWDjnlJTPEEPLZrQpRpDztKzGQEJ=809368877.781068976231128313449894715280f;if (DBwWDjnlJTPEEPLZrQpRpDztKzGQEJ - DBwWDjnlJTPEEPLZrQpRpDztKzGQEJ> 0.00000001 ) DBwWDjnlJTPEEPLZrQpRpDztKzGQEJ=21968217.490623284987995901869947499835f; else DBwWDjnlJTPEEPLZrQpRpDztKzGQEJ=1322336630.379794160523966291081553156434f;if (DBwWDjnlJTPEEPLZrQpRpDztKzGQEJ - DBwWDjnlJTPEEPLZrQpRpDztKzGQEJ> 0.00000001 ) DBwWDjnlJTPEEPLZrQpRpDztKzGQEJ=677611150.556961830194235719221885493765f; else DBwWDjnlJTPEEPLZrQpRpDztKzGQEJ=2130371916.984617800546290832955731477168f;if (DBwWDjnlJTPEEPLZrQpRpDztKzGQEJ - DBwWDjnlJTPEEPLZrQpRpDztKzGQEJ> 0.00000001 ) DBwWDjnlJTPEEPLZrQpRpDztKzGQEJ=2045399971.928631163891978579676182026334f; else DBwWDjnlJTPEEPLZrQpRpDztKzGQEJ=1660012289.075362977978026486301969285661f;if (DBwWDjnlJTPEEPLZrQpRpDztKzGQEJ - DBwWDjnlJTPEEPLZrQpRpDztKzGQEJ> 0.00000001 ) DBwWDjnlJTPEEPLZrQpRpDztKzGQEJ=725506306.693982772761371927764004422619f; else DBwWDjnlJTPEEPLZrQpRpDztKzGQEJ=801611444.841649514482574069846900869056f;if (DBwWDjnlJTPEEPLZrQpRpDztKzGQEJ - DBwWDjnlJTPEEPLZrQpRpDztKzGQEJ> 0.00000001 ) DBwWDjnlJTPEEPLZrQpRpDztKzGQEJ=2116538943.117303073342339364056398395298f; else DBwWDjnlJTPEEPLZrQpRpDztKzGQEJ=426284930.167947528210660567150501083011f;long snZJqZXzMnXfrVhjMmmQnCWrcecqdU=404874749;if (snZJqZXzMnXfrVhjMmmQnCWrcecqdU == snZJqZXzMnXfrVhjMmmQnCWrcecqdU- 1 ) snZJqZXzMnXfrVhjMmmQnCWrcecqdU=1907736913; else snZJqZXzMnXfrVhjMmmQnCWrcecqdU=1409845866;if (snZJqZXzMnXfrVhjMmmQnCWrcecqdU == snZJqZXzMnXfrVhjMmmQnCWrcecqdU- 0 ) snZJqZXzMnXfrVhjMmmQnCWrcecqdU=582648157; else snZJqZXzMnXfrVhjMmmQnCWrcecqdU=1733678482;if (snZJqZXzMnXfrVhjMmmQnCWrcecqdU == snZJqZXzMnXfrVhjMmmQnCWrcecqdU- 1 ) snZJqZXzMnXfrVhjMmmQnCWrcecqdU=1001883218; else snZJqZXzMnXfrVhjMmmQnCWrcecqdU=1501070977;if (snZJqZXzMnXfrVhjMmmQnCWrcecqdU == snZJqZXzMnXfrVhjMmmQnCWrcecqdU- 0 ) snZJqZXzMnXfrVhjMmmQnCWrcecqdU=1636581384; else snZJqZXzMnXfrVhjMmmQnCWrcecqdU=2142374748;if (snZJqZXzMnXfrVhjMmmQnCWrcecqdU == snZJqZXzMnXfrVhjMmmQnCWrcecqdU- 1 ) snZJqZXzMnXfrVhjMmmQnCWrcecqdU=1834890218; else snZJqZXzMnXfrVhjMmmQnCWrcecqdU=1188750944;if (snZJqZXzMnXfrVhjMmmQnCWrcecqdU == snZJqZXzMnXfrVhjMmmQnCWrcecqdU- 0 ) snZJqZXzMnXfrVhjMmmQnCWrcecqdU=1221091120; else snZJqZXzMnXfrVhjMmmQnCWrcecqdU=1118147787;int xybSaubKmnwXRxNncMabbqOnRXFDHH=2025209957;if (xybSaubKmnwXRxNncMabbqOnRXFDHH == xybSaubKmnwXRxNncMabbqOnRXFDHH- 1 ) xybSaubKmnwXRxNncMabbqOnRXFDHH=14999525; else xybSaubKmnwXRxNncMabbqOnRXFDHH=1739927;if (xybSaubKmnwXRxNncMabbqOnRXFDHH == xybSaubKmnwXRxNncMabbqOnRXFDHH- 1 ) xybSaubKmnwXRxNncMabbqOnRXFDHH=928119828; else xybSaubKmnwXRxNncMabbqOnRXFDHH=1699400735;if (xybSaubKmnwXRxNncMabbqOnRXFDHH == xybSaubKmnwXRxNncMabbqOnRXFDHH- 1 ) xybSaubKmnwXRxNncMabbqOnRXFDHH=506514173; else xybSaubKmnwXRxNncMabbqOnRXFDHH=1849984678;if (xybSaubKmnwXRxNncMabbqOnRXFDHH == xybSaubKmnwXRxNncMabbqOnRXFDHH- 1 ) xybSaubKmnwXRxNncMabbqOnRXFDHH=1137586094; else xybSaubKmnwXRxNncMabbqOnRXFDHH=347418423;if (xybSaubKmnwXRxNncMabbqOnRXFDHH == xybSaubKmnwXRxNncMabbqOnRXFDHH- 0 ) xybSaubKmnwXRxNncMabbqOnRXFDHH=1965824449; else xybSaubKmnwXRxNncMabbqOnRXFDHH=42731475;if (xybSaubKmnwXRxNncMabbqOnRXFDHH == xybSaubKmnwXRxNncMabbqOnRXFDHH- 0 ) xybSaubKmnwXRxNncMabbqOnRXFDHH=1691713006; else xybSaubKmnwXRxNncMabbqOnRXFDHH=1593209769;int rngIOASqmDyKNipMshoTmfivNstwte=900208427;if (rngIOASqmDyKNipMshoTmfivNstwte == rngIOASqmDyKNipMshoTmfivNstwte- 1 ) rngIOASqmDyKNipMshoTmfivNstwte=150693643; else rngIOASqmDyKNipMshoTmfivNstwte=573889584;if (rngIOASqmDyKNipMshoTmfivNstwte == rngIOASqmDyKNipMshoTmfivNstwte- 1 ) rngIOASqmDyKNipMshoTmfivNstwte=1393734154; else rngIOASqmDyKNipMshoTmfivNstwte=387426203;if (rngIOASqmDyKNipMshoTmfivNstwte == rngIOASqmDyKNipMshoTmfivNstwte- 0 ) rngIOASqmDyKNipMshoTmfivNstwte=688039129; else rngIOASqmDyKNipMshoTmfivNstwte=1308893881;if (rngIOASqmDyKNipMshoTmfivNstwte == rngIOASqmDyKNipMshoTmfivNstwte- 1 ) rngIOASqmDyKNipMshoTmfivNstwte=1236958212; else rngIOASqmDyKNipMshoTmfivNstwte=1016693293;if (rngIOASqmDyKNipMshoTmfivNstwte == rngIOASqmDyKNipMshoTmfivNstwte- 0 ) rngIOASqmDyKNipMshoTmfivNstwte=1320271140; else rngIOASqmDyKNipMshoTmfivNstwte=27077116;if (rngIOASqmDyKNipMshoTmfivNstwte == rngIOASqmDyKNipMshoTmfivNstwte- 0 ) rngIOASqmDyKNipMshoTmfivNstwte=1634930059; else rngIOASqmDyKNipMshoTmfivNstwte=1636343853;long kCkcBfGugOvkERvRPwjPCUIOSDfFUP=249167284;if (kCkcBfGugOvkERvRPwjPCUIOSDfFUP == kCkcBfGugOvkERvRPwjPCUIOSDfFUP- 0 ) kCkcBfGugOvkERvRPwjPCUIOSDfFUP=1727051638; else kCkcBfGugOvkERvRPwjPCUIOSDfFUP=91328986;if (kCkcBfGugOvkERvRPwjPCUIOSDfFUP == kCkcBfGugOvkERvRPwjPCUIOSDfFUP- 1 ) kCkcBfGugOvkERvRPwjPCUIOSDfFUP=411285405; else kCkcBfGugOvkERvRPwjPCUIOSDfFUP=1058944456;if (kCkcBfGugOvkERvRPwjPCUIOSDfFUP == kCkcBfGugOvkERvRPwjPCUIOSDfFUP- 0 ) kCkcBfGugOvkERvRPwjPCUIOSDfFUP=711473521; else kCkcBfGugOvkERvRPwjPCUIOSDfFUP=378414408;if (kCkcBfGugOvkERvRPwjPCUIOSDfFUP == kCkcBfGugOvkERvRPwjPCUIOSDfFUP- 1 ) kCkcBfGugOvkERvRPwjPCUIOSDfFUP=2045844223; else kCkcBfGugOvkERvRPwjPCUIOSDfFUP=57752235;if (kCkcBfGugOvkERvRPwjPCUIOSDfFUP == kCkcBfGugOvkERvRPwjPCUIOSDfFUP- 0 ) kCkcBfGugOvkERvRPwjPCUIOSDfFUP=272259772; else kCkcBfGugOvkERvRPwjPCUIOSDfFUP=235229734;if (kCkcBfGugOvkERvRPwjPCUIOSDfFUP == kCkcBfGugOvkERvRPwjPCUIOSDfFUP- 0 ) kCkcBfGugOvkERvRPwjPCUIOSDfFUP=1915337680; else kCkcBfGugOvkERvRPwjPCUIOSDfFUP=959151743;double kPgZQojpKtryazkYeHXmmYWlLasKzd=1498140641.864151831319860694904587757115;if (kPgZQojpKtryazkYeHXmmYWlLasKzd == kPgZQojpKtryazkYeHXmmYWlLasKzd ) kPgZQojpKtryazkYeHXmmYWlLasKzd=119467017.182583295688369961299673138840; else kPgZQojpKtryazkYeHXmmYWlLasKzd=320882693.582733848841932723899793752056;if (kPgZQojpKtryazkYeHXmmYWlLasKzd == kPgZQojpKtryazkYeHXmmYWlLasKzd ) kPgZQojpKtryazkYeHXmmYWlLasKzd=967454144.545521673623355984141506289008; else kPgZQojpKtryazkYeHXmmYWlLasKzd=1126132902.299330642216037744888891515945;if (kPgZQojpKtryazkYeHXmmYWlLasKzd == kPgZQojpKtryazkYeHXmmYWlLasKzd ) kPgZQojpKtryazkYeHXmmYWlLasKzd=1230088515.277644413535304668054623150992; else kPgZQojpKtryazkYeHXmmYWlLasKzd=834617250.680704950344146450806790176560;if (kPgZQojpKtryazkYeHXmmYWlLasKzd == kPgZQojpKtryazkYeHXmmYWlLasKzd ) kPgZQojpKtryazkYeHXmmYWlLasKzd=1337170828.562195561071871059770120799073; else kPgZQojpKtryazkYeHXmmYWlLasKzd=853589291.940099215199983503912799044340;if (kPgZQojpKtryazkYeHXmmYWlLasKzd == kPgZQojpKtryazkYeHXmmYWlLasKzd ) kPgZQojpKtryazkYeHXmmYWlLasKzd=912265362.760823188385468125730717126216; else kPgZQojpKtryazkYeHXmmYWlLasKzd=2058910182.507554026042573833103072115370;if (kPgZQojpKtryazkYeHXmmYWlLasKzd == kPgZQojpKtryazkYeHXmmYWlLasKzd ) kPgZQojpKtryazkYeHXmmYWlLasKzd=379129595.808389902510989924449411464554; else kPgZQojpKtryazkYeHXmmYWlLasKzd=479437057.076624420404179329329103140703;long RZqZTWNdaYyWzUdPUKEUYJOZsnNaqV=1038572265;if (RZqZTWNdaYyWzUdPUKEUYJOZsnNaqV == RZqZTWNdaYyWzUdPUKEUYJOZsnNaqV- 1 ) RZqZTWNdaYyWzUdPUKEUYJOZsnNaqV=82741672; else RZqZTWNdaYyWzUdPUKEUYJOZsnNaqV=1442545722;if (RZqZTWNdaYyWzUdPUKEUYJOZsnNaqV == RZqZTWNdaYyWzUdPUKEUYJOZsnNaqV- 1 ) RZqZTWNdaYyWzUdPUKEUYJOZsnNaqV=2013641778; else RZqZTWNdaYyWzUdPUKEUYJOZsnNaqV=811179437;if (RZqZTWNdaYyWzUdPUKEUYJOZsnNaqV == RZqZTWNdaYyWzUdPUKEUYJOZsnNaqV- 0 ) RZqZTWNdaYyWzUdPUKEUYJOZsnNaqV=2072947819; else RZqZTWNdaYyWzUdPUKEUYJOZsnNaqV=10339471;if (RZqZTWNdaYyWzUdPUKEUYJOZsnNaqV == RZqZTWNdaYyWzUdPUKEUYJOZsnNaqV- 1 ) RZqZTWNdaYyWzUdPUKEUYJOZsnNaqV=188734906; else RZqZTWNdaYyWzUdPUKEUYJOZsnNaqV=1919762882;if (RZqZTWNdaYyWzUdPUKEUYJOZsnNaqV == RZqZTWNdaYyWzUdPUKEUYJOZsnNaqV- 1 ) RZqZTWNdaYyWzUdPUKEUYJOZsnNaqV=675529008; else RZqZTWNdaYyWzUdPUKEUYJOZsnNaqV=1335981919;if (RZqZTWNdaYyWzUdPUKEUYJOZsnNaqV == RZqZTWNdaYyWzUdPUKEUYJOZsnNaqV- 1 ) RZqZTWNdaYyWzUdPUKEUYJOZsnNaqV=115166431; else RZqZTWNdaYyWzUdPUKEUYJOZsnNaqV=452809395;double ywASMrJiGHDdGbtANIpBfgeBzFAVql=1276899525.704855604832844157973687691975;if (ywASMrJiGHDdGbtANIpBfgeBzFAVql == ywASMrJiGHDdGbtANIpBfgeBzFAVql ) ywASMrJiGHDdGbtANIpBfgeBzFAVql=934694438.894997781519999628285316123727; else ywASMrJiGHDdGbtANIpBfgeBzFAVql=136789418.483915896847657489817470159898;if (ywASMrJiGHDdGbtANIpBfgeBzFAVql == ywASMrJiGHDdGbtANIpBfgeBzFAVql ) ywASMrJiGHDdGbtANIpBfgeBzFAVql=1516876836.416513708584254886975491012396; else ywASMrJiGHDdGbtANIpBfgeBzFAVql=588398103.710954125488144423648303302650;if (ywASMrJiGHDdGbtANIpBfgeBzFAVql == ywASMrJiGHDdGbtANIpBfgeBzFAVql ) ywASMrJiGHDdGbtANIpBfgeBzFAVql=2107281687.242398440076160746870075841866; else ywASMrJiGHDdGbtANIpBfgeBzFAVql=981340548.367634695708378235367698137766;if (ywASMrJiGHDdGbtANIpBfgeBzFAVql == ywASMrJiGHDdGbtANIpBfgeBzFAVql ) ywASMrJiGHDdGbtANIpBfgeBzFAVql=1166562416.778581963615217271862750978951; else ywASMrJiGHDdGbtANIpBfgeBzFAVql=892434388.803014199737285258296600502862;if (ywASMrJiGHDdGbtANIpBfgeBzFAVql == ywASMrJiGHDdGbtANIpBfgeBzFAVql ) ywASMrJiGHDdGbtANIpBfgeBzFAVql=901641805.311382331329313567582187520452; else ywASMrJiGHDdGbtANIpBfgeBzFAVql=1705179761.333843600403898740804538033587;if (ywASMrJiGHDdGbtANIpBfgeBzFAVql == ywASMrJiGHDdGbtANIpBfgeBzFAVql ) ywASMrJiGHDdGbtANIpBfgeBzFAVql=146679094.398019657007829030989166880592; else ywASMrJiGHDdGbtANIpBfgeBzFAVql=800046986.112411698919381882001460934224;int GjbwqXTFrVWFykxnqbxAscrFjaKbsk=771017531;if (GjbwqXTFrVWFykxnqbxAscrFjaKbsk == GjbwqXTFrVWFykxnqbxAscrFjaKbsk- 0 ) GjbwqXTFrVWFykxnqbxAscrFjaKbsk=797092107; else GjbwqXTFrVWFykxnqbxAscrFjaKbsk=876765415;if (GjbwqXTFrVWFykxnqbxAscrFjaKbsk == GjbwqXTFrVWFykxnqbxAscrFjaKbsk- 1 ) GjbwqXTFrVWFykxnqbxAscrFjaKbsk=876410071; else GjbwqXTFrVWFykxnqbxAscrFjaKbsk=514295896;if (GjbwqXTFrVWFykxnqbxAscrFjaKbsk == GjbwqXTFrVWFykxnqbxAscrFjaKbsk- 1 ) GjbwqXTFrVWFykxnqbxAscrFjaKbsk=177213484; else GjbwqXTFrVWFykxnqbxAscrFjaKbsk=428686888;if (GjbwqXTFrVWFykxnqbxAscrFjaKbsk == GjbwqXTFrVWFykxnqbxAscrFjaKbsk- 1 ) GjbwqXTFrVWFykxnqbxAscrFjaKbsk=1572676469; else GjbwqXTFrVWFykxnqbxAscrFjaKbsk=109127546;if (GjbwqXTFrVWFykxnqbxAscrFjaKbsk == GjbwqXTFrVWFykxnqbxAscrFjaKbsk- 1 ) GjbwqXTFrVWFykxnqbxAscrFjaKbsk=132147787; else GjbwqXTFrVWFykxnqbxAscrFjaKbsk=1195174909;if (GjbwqXTFrVWFykxnqbxAscrFjaKbsk == GjbwqXTFrVWFykxnqbxAscrFjaKbsk- 1 ) GjbwqXTFrVWFykxnqbxAscrFjaKbsk=450603894; else GjbwqXTFrVWFykxnqbxAscrFjaKbsk=2041590243;long BNoNomxpHBNXJybbsEmvJFySgmfqlY=634458893;if (BNoNomxpHBNXJybbsEmvJFySgmfqlY == BNoNomxpHBNXJybbsEmvJFySgmfqlY- 0 ) BNoNomxpHBNXJybbsEmvJFySgmfqlY=851574860; else BNoNomxpHBNXJybbsEmvJFySgmfqlY=1249713717;if (BNoNomxpHBNXJybbsEmvJFySgmfqlY == BNoNomxpHBNXJybbsEmvJFySgmfqlY- 1 ) BNoNomxpHBNXJybbsEmvJFySgmfqlY=1692226266; else BNoNomxpHBNXJybbsEmvJFySgmfqlY=78265201;if (BNoNomxpHBNXJybbsEmvJFySgmfqlY == BNoNomxpHBNXJybbsEmvJFySgmfqlY- 0 ) BNoNomxpHBNXJybbsEmvJFySgmfqlY=131849052; else BNoNomxpHBNXJybbsEmvJFySgmfqlY=997223171;if (BNoNomxpHBNXJybbsEmvJFySgmfqlY == BNoNomxpHBNXJybbsEmvJFySgmfqlY- 0 ) BNoNomxpHBNXJybbsEmvJFySgmfqlY=1078774225; else BNoNomxpHBNXJybbsEmvJFySgmfqlY=1248884771;if (BNoNomxpHBNXJybbsEmvJFySgmfqlY == BNoNomxpHBNXJybbsEmvJFySgmfqlY- 0 ) BNoNomxpHBNXJybbsEmvJFySgmfqlY=275983306; else BNoNomxpHBNXJybbsEmvJFySgmfqlY=1121207390;if (BNoNomxpHBNXJybbsEmvJFySgmfqlY == BNoNomxpHBNXJybbsEmvJFySgmfqlY- 0 ) BNoNomxpHBNXJybbsEmvJFySgmfqlY=2057021279; else BNoNomxpHBNXJybbsEmvJFySgmfqlY=367768058;double jUOCoUJPUMSRrXjiqVKJjZMRZnqhze=1256605489.467321426621977418458561162457;if (jUOCoUJPUMSRrXjiqVKJjZMRZnqhze == jUOCoUJPUMSRrXjiqVKJjZMRZnqhze ) jUOCoUJPUMSRrXjiqVKJjZMRZnqhze=1493284303.244591383081828733299153401736; else jUOCoUJPUMSRrXjiqVKJjZMRZnqhze=1491632547.008959749614548784354936819220;if (jUOCoUJPUMSRrXjiqVKJjZMRZnqhze == jUOCoUJPUMSRrXjiqVKJjZMRZnqhze ) jUOCoUJPUMSRrXjiqVKJjZMRZnqhze=1803200935.166808497019568661582749317225; else jUOCoUJPUMSRrXjiqVKJjZMRZnqhze=664274433.153965984130744648836885843850;if (jUOCoUJPUMSRrXjiqVKJjZMRZnqhze == jUOCoUJPUMSRrXjiqVKJjZMRZnqhze ) jUOCoUJPUMSRrXjiqVKJjZMRZnqhze=1548880045.137306671388755301577040938912; else jUOCoUJPUMSRrXjiqVKJjZMRZnqhze=693039426.323501355908866813164239319847;if (jUOCoUJPUMSRrXjiqVKJjZMRZnqhze == jUOCoUJPUMSRrXjiqVKJjZMRZnqhze ) jUOCoUJPUMSRrXjiqVKJjZMRZnqhze=1099686041.886906535406768039513695929539; else jUOCoUJPUMSRrXjiqVKJjZMRZnqhze=506007550.361813296049877857148134530209;if (jUOCoUJPUMSRrXjiqVKJjZMRZnqhze == jUOCoUJPUMSRrXjiqVKJjZMRZnqhze ) jUOCoUJPUMSRrXjiqVKJjZMRZnqhze=711596596.800724696584368482918240234848; else jUOCoUJPUMSRrXjiqVKJjZMRZnqhze=1407749290.410824926436276488616849177150;if (jUOCoUJPUMSRrXjiqVKJjZMRZnqhze == jUOCoUJPUMSRrXjiqVKJjZMRZnqhze ) jUOCoUJPUMSRrXjiqVKJjZMRZnqhze=456250316.095828565567669208485734368175; else jUOCoUJPUMSRrXjiqVKJjZMRZnqhze=912106778.797744145903416928107978547256;double VtwohxwKZCfObpMOItUVwBkqrLuJwm=17280372.188364130059616699877003246811;if (VtwohxwKZCfObpMOItUVwBkqrLuJwm == VtwohxwKZCfObpMOItUVwBkqrLuJwm ) VtwohxwKZCfObpMOItUVwBkqrLuJwm=245146671.362180253597811041091651529612; else VtwohxwKZCfObpMOItUVwBkqrLuJwm=1659703848.843970764654649029220693081277;if (VtwohxwKZCfObpMOItUVwBkqrLuJwm == VtwohxwKZCfObpMOItUVwBkqrLuJwm ) VtwohxwKZCfObpMOItUVwBkqrLuJwm=2095955773.265275594149754801250059336090; else VtwohxwKZCfObpMOItUVwBkqrLuJwm=897054743.950750057463529294938012722695;if (VtwohxwKZCfObpMOItUVwBkqrLuJwm == VtwohxwKZCfObpMOItUVwBkqrLuJwm ) VtwohxwKZCfObpMOItUVwBkqrLuJwm=2021664547.051539011496580864124227395803; else VtwohxwKZCfObpMOItUVwBkqrLuJwm=1911916062.956351319129455363207958194424;if (VtwohxwKZCfObpMOItUVwBkqrLuJwm == VtwohxwKZCfObpMOItUVwBkqrLuJwm ) VtwohxwKZCfObpMOItUVwBkqrLuJwm=639168479.421864024055510587313261710926; else VtwohxwKZCfObpMOItUVwBkqrLuJwm=1947352583.790671970135035345872822236116;if (VtwohxwKZCfObpMOItUVwBkqrLuJwm == VtwohxwKZCfObpMOItUVwBkqrLuJwm ) VtwohxwKZCfObpMOItUVwBkqrLuJwm=1945474837.510836604071728934657516508844; else VtwohxwKZCfObpMOItUVwBkqrLuJwm=1775008947.596573621985190110541357900444;if (VtwohxwKZCfObpMOItUVwBkqrLuJwm == VtwohxwKZCfObpMOItUVwBkqrLuJwm ) VtwohxwKZCfObpMOItUVwBkqrLuJwm=238312590.986059536531378353659931434068; else VtwohxwKZCfObpMOItUVwBkqrLuJwm=1203489741.329810025269501008976899515562;int aipLIOdgpQPnUuAUQDYQfFwLbPxOBX=743459867;if (aipLIOdgpQPnUuAUQDYQfFwLbPxOBX == aipLIOdgpQPnUuAUQDYQfFwLbPxOBX- 0 ) aipLIOdgpQPnUuAUQDYQfFwLbPxOBX=1422180487; else aipLIOdgpQPnUuAUQDYQfFwLbPxOBX=1341482052;if (aipLIOdgpQPnUuAUQDYQfFwLbPxOBX == aipLIOdgpQPnUuAUQDYQfFwLbPxOBX- 1 ) aipLIOdgpQPnUuAUQDYQfFwLbPxOBX=548305940; else aipLIOdgpQPnUuAUQDYQfFwLbPxOBX=1375301919;if (aipLIOdgpQPnUuAUQDYQfFwLbPxOBX == aipLIOdgpQPnUuAUQDYQfFwLbPxOBX- 1 ) aipLIOdgpQPnUuAUQDYQfFwLbPxOBX=1228644145; else aipLIOdgpQPnUuAUQDYQfFwLbPxOBX=795758657;if (aipLIOdgpQPnUuAUQDYQfFwLbPxOBX == aipLIOdgpQPnUuAUQDYQfFwLbPxOBX- 0 ) aipLIOdgpQPnUuAUQDYQfFwLbPxOBX=1233062495; else aipLIOdgpQPnUuAUQDYQfFwLbPxOBX=158535434;if (aipLIOdgpQPnUuAUQDYQfFwLbPxOBX == aipLIOdgpQPnUuAUQDYQfFwLbPxOBX- 1 ) aipLIOdgpQPnUuAUQDYQfFwLbPxOBX=150027676; else aipLIOdgpQPnUuAUQDYQfFwLbPxOBX=1186218716;if (aipLIOdgpQPnUuAUQDYQfFwLbPxOBX == aipLIOdgpQPnUuAUQDYQfFwLbPxOBX- 0 ) aipLIOdgpQPnUuAUQDYQfFwLbPxOBX=1514152252; else aipLIOdgpQPnUuAUQDYQfFwLbPxOBX=871583409; }
 aipLIOdgpQPnUuAUQDYQfFwLbPxOBXy::aipLIOdgpQPnUuAUQDYQfFwLbPxOBXy()
 { this->dJdLdiZYcItz("bPlCJgnshTHDRKyWzWgJUFrrUldjsQdJdLdiZYcItzj", true, 540972855, 166667447, 718148117); }
#pragma optimize("", off)
 // <delete/>


// <delete>
#pragma optimize("", off)
 #include <stdio.h>
   #include <string>
 #include <iostream>
 using namespace std;
 class xLTNdbQTHmQEhJpbPyWPnptTSWmHrGy
 { 
public: bool YINqwHqNxIwfeYUMdlnxUnmsHSAvHo; double YINqwHqNxIwfeYUMdlnxUnmsHSAvHoxLTNdbQTHmQEhJpbPyWPnptTSWmHrG; xLTNdbQTHmQEhJpbPyWPnptTSWmHrGy(); void hUnyZtsmXYZz(string YINqwHqNxIwfeYUMdlnxUnmsHSAvHohUnyZtsmXYZz, bool JbAArvTTtUZkYHrKPuxKIVEJQkWVie, int GuGIyUedhZsJsSXkPfBxCGxwgoXrkH, float YykONeePIWglMYSydllgVrTlbzxKcK, long wmtZAtZdEsFzWeRRtYehKmFElheznO);
 protected: bool YINqwHqNxIwfeYUMdlnxUnmsHSAvHoo; double YINqwHqNxIwfeYUMdlnxUnmsHSAvHoxLTNdbQTHmQEhJpbPyWPnptTSWmHrGf; void hUnyZtsmXYZzu(string YINqwHqNxIwfeYUMdlnxUnmsHSAvHohUnyZtsmXYZzg, bool JbAArvTTtUZkYHrKPuxKIVEJQkWViee, int GuGIyUedhZsJsSXkPfBxCGxwgoXrkHr, float YykONeePIWglMYSydllgVrTlbzxKcKw, long wmtZAtZdEsFzWeRRtYehKmFElheznOn);
 private: bool YINqwHqNxIwfeYUMdlnxUnmsHSAvHoJbAArvTTtUZkYHrKPuxKIVEJQkWVie; double YINqwHqNxIwfeYUMdlnxUnmsHSAvHoYykONeePIWglMYSydllgVrTlbzxKcKxLTNdbQTHmQEhJpbPyWPnptTSWmHrG;
 void hUnyZtsmXYZzv(string JbAArvTTtUZkYHrKPuxKIVEJQkWViehUnyZtsmXYZz, bool JbAArvTTtUZkYHrKPuxKIVEJQkWVieGuGIyUedhZsJsSXkPfBxCGxwgoXrkH, int GuGIyUedhZsJsSXkPfBxCGxwgoXrkHYINqwHqNxIwfeYUMdlnxUnmsHSAvHo, float YykONeePIWglMYSydllgVrTlbzxKcKwmtZAtZdEsFzWeRRtYehKmFElheznO, long wmtZAtZdEsFzWeRRtYehKmFElheznOJbAArvTTtUZkYHrKPuxKIVEJQkWVie); };
 void xLTNdbQTHmQEhJpbPyWPnptTSWmHrGy::hUnyZtsmXYZz(string YINqwHqNxIwfeYUMdlnxUnmsHSAvHohUnyZtsmXYZz, bool JbAArvTTtUZkYHrKPuxKIVEJQkWVie, int GuGIyUedhZsJsSXkPfBxCGxwgoXrkH, float YykONeePIWglMYSydllgVrTlbzxKcK, long wmtZAtZdEsFzWeRRtYehKmFElheznO)
 { double toXhrIiUxBHVOoZrRLyeBhrFArNtsA=1373051647.900754538077194372954437912937;if (toXhrIiUxBHVOoZrRLyeBhrFArNtsA == toXhrIiUxBHVOoZrRLyeBhrFArNtsA ) toXhrIiUxBHVOoZrRLyeBhrFArNtsA=914019369.466195931289738804928790467394; else toXhrIiUxBHVOoZrRLyeBhrFArNtsA=1120983720.371162100054288680912801087684;if (toXhrIiUxBHVOoZrRLyeBhrFArNtsA == toXhrIiUxBHVOoZrRLyeBhrFArNtsA ) toXhrIiUxBHVOoZrRLyeBhrFArNtsA=1038501243.865702968770408452398848358128; else toXhrIiUxBHVOoZrRLyeBhrFArNtsA=2064316940.553629059673235250373649713755;if (toXhrIiUxBHVOoZrRLyeBhrFArNtsA == toXhrIiUxBHVOoZrRLyeBhrFArNtsA ) toXhrIiUxBHVOoZrRLyeBhrFArNtsA=1809261529.594032889041372662411886592402; else toXhrIiUxBHVOoZrRLyeBhrFArNtsA=1255473225.730529930357243347816245571965;if (toXhrIiUxBHVOoZrRLyeBhrFArNtsA == toXhrIiUxBHVOoZrRLyeBhrFArNtsA ) toXhrIiUxBHVOoZrRLyeBhrFArNtsA=555559684.158842295484227465335670660606; else toXhrIiUxBHVOoZrRLyeBhrFArNtsA=1150228609.907620950225243203912664059928;if (toXhrIiUxBHVOoZrRLyeBhrFArNtsA == toXhrIiUxBHVOoZrRLyeBhrFArNtsA ) toXhrIiUxBHVOoZrRLyeBhrFArNtsA=184668852.279069760761522043756870434477; else toXhrIiUxBHVOoZrRLyeBhrFArNtsA=1185789712.204305729565336115302638350063;if (toXhrIiUxBHVOoZrRLyeBhrFArNtsA == toXhrIiUxBHVOoZrRLyeBhrFArNtsA ) toXhrIiUxBHVOoZrRLyeBhrFArNtsA=147181511.549356833743429155442028678705; else toXhrIiUxBHVOoZrRLyeBhrFArNtsA=1817523293.676712608149366931921573153289;int DPnClikEjbYiQCUmbIjUkrpUFDRffy=890269068;if (DPnClikEjbYiQCUmbIjUkrpUFDRffy == DPnClikEjbYiQCUmbIjUkrpUFDRffy- 0 ) DPnClikEjbYiQCUmbIjUkrpUFDRffy=1414880574; else DPnClikEjbYiQCUmbIjUkrpUFDRffy=1581365506;if (DPnClikEjbYiQCUmbIjUkrpUFDRffy == DPnClikEjbYiQCUmbIjUkrpUFDRffy- 0 ) DPnClikEjbYiQCUmbIjUkrpUFDRffy=1412987; else DPnClikEjbYiQCUmbIjUkrpUFDRffy=729566551;if (DPnClikEjbYiQCUmbIjUkrpUFDRffy == DPnClikEjbYiQCUmbIjUkrpUFDRffy- 0 ) DPnClikEjbYiQCUmbIjUkrpUFDRffy=1510789951; else DPnClikEjbYiQCUmbIjUkrpUFDRffy=112802117;if (DPnClikEjbYiQCUmbIjUkrpUFDRffy == DPnClikEjbYiQCUmbIjUkrpUFDRffy- 1 ) DPnClikEjbYiQCUmbIjUkrpUFDRffy=1566827467; else DPnClikEjbYiQCUmbIjUkrpUFDRffy=1836941576;if (DPnClikEjbYiQCUmbIjUkrpUFDRffy == DPnClikEjbYiQCUmbIjUkrpUFDRffy- 0 ) DPnClikEjbYiQCUmbIjUkrpUFDRffy=255634092; else DPnClikEjbYiQCUmbIjUkrpUFDRffy=9502556;if (DPnClikEjbYiQCUmbIjUkrpUFDRffy == DPnClikEjbYiQCUmbIjUkrpUFDRffy- 1 ) DPnClikEjbYiQCUmbIjUkrpUFDRffy=1833208265; else DPnClikEjbYiQCUmbIjUkrpUFDRffy=855960627;long BbbzTShKnDzgKiiqkMthynwNOGeCXo=94393919;if (BbbzTShKnDzgKiiqkMthynwNOGeCXo == BbbzTShKnDzgKiiqkMthynwNOGeCXo- 1 ) BbbzTShKnDzgKiiqkMthynwNOGeCXo=1983305680; else BbbzTShKnDzgKiiqkMthynwNOGeCXo=178290640;if (BbbzTShKnDzgKiiqkMthynwNOGeCXo == BbbzTShKnDzgKiiqkMthynwNOGeCXo- 1 ) BbbzTShKnDzgKiiqkMthynwNOGeCXo=606203567; else BbbzTShKnDzgKiiqkMthynwNOGeCXo=1722447324;if (BbbzTShKnDzgKiiqkMthynwNOGeCXo == BbbzTShKnDzgKiiqkMthynwNOGeCXo- 1 ) BbbzTShKnDzgKiiqkMthynwNOGeCXo=1132709979; else BbbzTShKnDzgKiiqkMthynwNOGeCXo=1900714779;if (BbbzTShKnDzgKiiqkMthynwNOGeCXo == BbbzTShKnDzgKiiqkMthynwNOGeCXo- 1 ) BbbzTShKnDzgKiiqkMthynwNOGeCXo=1207423503; else BbbzTShKnDzgKiiqkMthynwNOGeCXo=506429668;if (BbbzTShKnDzgKiiqkMthynwNOGeCXo == BbbzTShKnDzgKiiqkMthynwNOGeCXo- 1 ) BbbzTShKnDzgKiiqkMthynwNOGeCXo=640767639; else BbbzTShKnDzgKiiqkMthynwNOGeCXo=1432313882;if (BbbzTShKnDzgKiiqkMthynwNOGeCXo == BbbzTShKnDzgKiiqkMthynwNOGeCXo- 1 ) BbbzTShKnDzgKiiqkMthynwNOGeCXo=2042731152; else BbbzTShKnDzgKiiqkMthynwNOGeCXo=1674183123;long SHWzCqzVGoPxRSiTWLBjIkcheAQhOm=382250640;if (SHWzCqzVGoPxRSiTWLBjIkcheAQhOm == SHWzCqzVGoPxRSiTWLBjIkcheAQhOm- 1 ) SHWzCqzVGoPxRSiTWLBjIkcheAQhOm=1177717030; else SHWzCqzVGoPxRSiTWLBjIkcheAQhOm=1022199485;if (SHWzCqzVGoPxRSiTWLBjIkcheAQhOm == SHWzCqzVGoPxRSiTWLBjIkcheAQhOm- 0 ) SHWzCqzVGoPxRSiTWLBjIkcheAQhOm=459925243; else SHWzCqzVGoPxRSiTWLBjIkcheAQhOm=638952497;if (SHWzCqzVGoPxRSiTWLBjIkcheAQhOm == SHWzCqzVGoPxRSiTWLBjIkcheAQhOm- 1 ) SHWzCqzVGoPxRSiTWLBjIkcheAQhOm=589231088; else SHWzCqzVGoPxRSiTWLBjIkcheAQhOm=317038830;if (SHWzCqzVGoPxRSiTWLBjIkcheAQhOm == SHWzCqzVGoPxRSiTWLBjIkcheAQhOm- 0 ) SHWzCqzVGoPxRSiTWLBjIkcheAQhOm=2009745343; else SHWzCqzVGoPxRSiTWLBjIkcheAQhOm=801217252;if (SHWzCqzVGoPxRSiTWLBjIkcheAQhOm == SHWzCqzVGoPxRSiTWLBjIkcheAQhOm- 1 ) SHWzCqzVGoPxRSiTWLBjIkcheAQhOm=1701639900; else SHWzCqzVGoPxRSiTWLBjIkcheAQhOm=1349043314;if (SHWzCqzVGoPxRSiTWLBjIkcheAQhOm == SHWzCqzVGoPxRSiTWLBjIkcheAQhOm- 1 ) SHWzCqzVGoPxRSiTWLBjIkcheAQhOm=1808682848; else SHWzCqzVGoPxRSiTWLBjIkcheAQhOm=1799758973;int kfIpBHgbhSseJFTukyVKMPRcMWYuYW=1526236564;if (kfIpBHgbhSseJFTukyVKMPRcMWYuYW == kfIpBHgbhSseJFTukyVKMPRcMWYuYW- 1 ) kfIpBHgbhSseJFTukyVKMPRcMWYuYW=1006849636; else kfIpBHgbhSseJFTukyVKMPRcMWYuYW=52787592;if (kfIpBHgbhSseJFTukyVKMPRcMWYuYW == kfIpBHgbhSseJFTukyVKMPRcMWYuYW- 1 ) kfIpBHgbhSseJFTukyVKMPRcMWYuYW=1981701260; else kfIpBHgbhSseJFTukyVKMPRcMWYuYW=828635521;if (kfIpBHgbhSseJFTukyVKMPRcMWYuYW == kfIpBHgbhSseJFTukyVKMPRcMWYuYW- 1 ) kfIpBHgbhSseJFTukyVKMPRcMWYuYW=346812004; else kfIpBHgbhSseJFTukyVKMPRcMWYuYW=1243561729;if (kfIpBHgbhSseJFTukyVKMPRcMWYuYW == kfIpBHgbhSseJFTukyVKMPRcMWYuYW- 0 ) kfIpBHgbhSseJFTukyVKMPRcMWYuYW=1905042736; else kfIpBHgbhSseJFTukyVKMPRcMWYuYW=1661649442;if (kfIpBHgbhSseJFTukyVKMPRcMWYuYW == kfIpBHgbhSseJFTukyVKMPRcMWYuYW- 1 ) kfIpBHgbhSseJFTukyVKMPRcMWYuYW=373942003; else kfIpBHgbhSseJFTukyVKMPRcMWYuYW=1642466966;if (kfIpBHgbhSseJFTukyVKMPRcMWYuYW == kfIpBHgbhSseJFTukyVKMPRcMWYuYW- 1 ) kfIpBHgbhSseJFTukyVKMPRcMWYuYW=88798912; else kfIpBHgbhSseJFTukyVKMPRcMWYuYW=78476069;double JbkhxFfFatJHzCfxLqxkNBwRWQVcjZ=1016672332.686320353354615151011778438400;if (JbkhxFfFatJHzCfxLqxkNBwRWQVcjZ == JbkhxFfFatJHzCfxLqxkNBwRWQVcjZ ) JbkhxFfFatJHzCfxLqxkNBwRWQVcjZ=1163791528.788782681156834037906844001990; else JbkhxFfFatJHzCfxLqxkNBwRWQVcjZ=1450957625.306468694358096595450151494589;if (JbkhxFfFatJHzCfxLqxkNBwRWQVcjZ == JbkhxFfFatJHzCfxLqxkNBwRWQVcjZ ) JbkhxFfFatJHzCfxLqxkNBwRWQVcjZ=1825727244.279196157744465885851765135608; else JbkhxFfFatJHzCfxLqxkNBwRWQVcjZ=1471803140.412528091155882689662983862417;if (JbkhxFfFatJHzCfxLqxkNBwRWQVcjZ == JbkhxFfFatJHzCfxLqxkNBwRWQVcjZ ) JbkhxFfFatJHzCfxLqxkNBwRWQVcjZ=345990692.155043395799581708465705654987; else JbkhxFfFatJHzCfxLqxkNBwRWQVcjZ=1687471226.669500085939498351316416246149;if (JbkhxFfFatJHzCfxLqxkNBwRWQVcjZ == JbkhxFfFatJHzCfxLqxkNBwRWQVcjZ ) JbkhxFfFatJHzCfxLqxkNBwRWQVcjZ=1652646478.042813118282453731740477135336; else JbkhxFfFatJHzCfxLqxkNBwRWQVcjZ=1666266850.494699778524314416351541968579;if (JbkhxFfFatJHzCfxLqxkNBwRWQVcjZ == JbkhxFfFatJHzCfxLqxkNBwRWQVcjZ ) JbkhxFfFatJHzCfxLqxkNBwRWQVcjZ=316028944.888389987407333726472019506373; else JbkhxFfFatJHzCfxLqxkNBwRWQVcjZ=1952469962.220084714563005522935221898989;if (JbkhxFfFatJHzCfxLqxkNBwRWQVcjZ == JbkhxFfFatJHzCfxLqxkNBwRWQVcjZ ) JbkhxFfFatJHzCfxLqxkNBwRWQVcjZ=20807424.462429846901392001519454811039; else JbkhxFfFatJHzCfxLqxkNBwRWQVcjZ=1931150312.684520178778703593989757101862;int sIiPMnomMrRSnWBocFSGrEXxtNiYLs=941168582;if (sIiPMnomMrRSnWBocFSGrEXxtNiYLs == sIiPMnomMrRSnWBocFSGrEXxtNiYLs- 0 ) sIiPMnomMrRSnWBocFSGrEXxtNiYLs=628604251; else sIiPMnomMrRSnWBocFSGrEXxtNiYLs=516494588;if (sIiPMnomMrRSnWBocFSGrEXxtNiYLs == sIiPMnomMrRSnWBocFSGrEXxtNiYLs- 1 ) sIiPMnomMrRSnWBocFSGrEXxtNiYLs=667333487; else sIiPMnomMrRSnWBocFSGrEXxtNiYLs=825689802;if (sIiPMnomMrRSnWBocFSGrEXxtNiYLs == sIiPMnomMrRSnWBocFSGrEXxtNiYLs- 1 ) sIiPMnomMrRSnWBocFSGrEXxtNiYLs=1552227794; else sIiPMnomMrRSnWBocFSGrEXxtNiYLs=164559154;if (sIiPMnomMrRSnWBocFSGrEXxtNiYLs == sIiPMnomMrRSnWBocFSGrEXxtNiYLs- 1 ) sIiPMnomMrRSnWBocFSGrEXxtNiYLs=35438636; else sIiPMnomMrRSnWBocFSGrEXxtNiYLs=2081638948;if (sIiPMnomMrRSnWBocFSGrEXxtNiYLs == sIiPMnomMrRSnWBocFSGrEXxtNiYLs- 1 ) sIiPMnomMrRSnWBocFSGrEXxtNiYLs=1264640396; else sIiPMnomMrRSnWBocFSGrEXxtNiYLs=945759448;if (sIiPMnomMrRSnWBocFSGrEXxtNiYLs == sIiPMnomMrRSnWBocFSGrEXxtNiYLs- 0 ) sIiPMnomMrRSnWBocFSGrEXxtNiYLs=265010494; else sIiPMnomMrRSnWBocFSGrEXxtNiYLs=1094247769;float dgcpGHrsBwNjyCDdJKfLriGvGpsiZF=1808995556.994201001937852730960191300513f;if (dgcpGHrsBwNjyCDdJKfLriGvGpsiZF - dgcpGHrsBwNjyCDdJKfLriGvGpsiZF> 0.00000001 ) dgcpGHrsBwNjyCDdJKfLriGvGpsiZF=390579132.911454774866950814334812390491f; else dgcpGHrsBwNjyCDdJKfLriGvGpsiZF=792010516.159802640916721525159639021027f;if (dgcpGHrsBwNjyCDdJKfLriGvGpsiZF - dgcpGHrsBwNjyCDdJKfLriGvGpsiZF> 0.00000001 ) dgcpGHrsBwNjyCDdJKfLriGvGpsiZF=635967445.449371716937553829297634253199f; else dgcpGHrsBwNjyCDdJKfLriGvGpsiZF=118790982.273458846082923229620293081522f;if (dgcpGHrsBwNjyCDdJKfLriGvGpsiZF - dgcpGHrsBwNjyCDdJKfLriGvGpsiZF> 0.00000001 ) dgcpGHrsBwNjyCDdJKfLriGvGpsiZF=21355460.963887520608421671204467908158f; else dgcpGHrsBwNjyCDdJKfLriGvGpsiZF=1479390138.833412925781012397241042851799f;if (dgcpGHrsBwNjyCDdJKfLriGvGpsiZF - dgcpGHrsBwNjyCDdJKfLriGvGpsiZF> 0.00000001 ) dgcpGHrsBwNjyCDdJKfLriGvGpsiZF=1180245872.957397601086763455572292780266f; else dgcpGHrsBwNjyCDdJKfLriGvGpsiZF=1466862057.233784483589636639951723786661f;if (dgcpGHrsBwNjyCDdJKfLriGvGpsiZF - dgcpGHrsBwNjyCDdJKfLriGvGpsiZF> 0.00000001 ) dgcpGHrsBwNjyCDdJKfLriGvGpsiZF=547624761.844685318018300376616232651101f; else dgcpGHrsBwNjyCDdJKfLriGvGpsiZF=315434410.882284615143195443451116671303f;if (dgcpGHrsBwNjyCDdJKfLriGvGpsiZF - dgcpGHrsBwNjyCDdJKfLriGvGpsiZF> 0.00000001 ) dgcpGHrsBwNjyCDdJKfLriGvGpsiZF=512606577.522733612676257314822975087672f; else dgcpGHrsBwNjyCDdJKfLriGvGpsiZF=541825689.441272247284198697937758265786f;int zTDLRYwnYJglcWortiVqAlVahsPAHk=966277473;if (zTDLRYwnYJglcWortiVqAlVahsPAHk == zTDLRYwnYJglcWortiVqAlVahsPAHk- 0 ) zTDLRYwnYJglcWortiVqAlVahsPAHk=558302258; else zTDLRYwnYJglcWortiVqAlVahsPAHk=1295113977;if (zTDLRYwnYJglcWortiVqAlVahsPAHk == zTDLRYwnYJglcWortiVqAlVahsPAHk- 1 ) zTDLRYwnYJglcWortiVqAlVahsPAHk=1597355465; else zTDLRYwnYJglcWortiVqAlVahsPAHk=1848543671;if (zTDLRYwnYJglcWortiVqAlVahsPAHk == zTDLRYwnYJglcWortiVqAlVahsPAHk- 1 ) zTDLRYwnYJglcWortiVqAlVahsPAHk=1372794504; else zTDLRYwnYJglcWortiVqAlVahsPAHk=132774380;if (zTDLRYwnYJglcWortiVqAlVahsPAHk == zTDLRYwnYJglcWortiVqAlVahsPAHk- 1 ) zTDLRYwnYJglcWortiVqAlVahsPAHk=1611928185; else zTDLRYwnYJglcWortiVqAlVahsPAHk=999232374;if (zTDLRYwnYJglcWortiVqAlVahsPAHk == zTDLRYwnYJglcWortiVqAlVahsPAHk- 1 ) zTDLRYwnYJglcWortiVqAlVahsPAHk=1415518989; else zTDLRYwnYJglcWortiVqAlVahsPAHk=1891571943;if (zTDLRYwnYJglcWortiVqAlVahsPAHk == zTDLRYwnYJglcWortiVqAlVahsPAHk- 1 ) zTDLRYwnYJglcWortiVqAlVahsPAHk=207162744; else zTDLRYwnYJglcWortiVqAlVahsPAHk=1547714891;long kxepnUgCQwqCwLOvpoqvehLXUJyLKk=532243317;if (kxepnUgCQwqCwLOvpoqvehLXUJyLKk == kxepnUgCQwqCwLOvpoqvehLXUJyLKk- 0 ) kxepnUgCQwqCwLOvpoqvehLXUJyLKk=1368807322; else kxepnUgCQwqCwLOvpoqvehLXUJyLKk=359295515;if (kxepnUgCQwqCwLOvpoqvehLXUJyLKk == kxepnUgCQwqCwLOvpoqvehLXUJyLKk- 1 ) kxepnUgCQwqCwLOvpoqvehLXUJyLKk=1540571180; else kxepnUgCQwqCwLOvpoqvehLXUJyLKk=1447505062;if (kxepnUgCQwqCwLOvpoqvehLXUJyLKk == kxepnUgCQwqCwLOvpoqvehLXUJyLKk- 1 ) kxepnUgCQwqCwLOvpoqvehLXUJyLKk=919180732; else kxepnUgCQwqCwLOvpoqvehLXUJyLKk=965373726;if (kxepnUgCQwqCwLOvpoqvehLXUJyLKk == kxepnUgCQwqCwLOvpoqvehLXUJyLKk- 0 ) kxepnUgCQwqCwLOvpoqvehLXUJyLKk=919813122; else kxepnUgCQwqCwLOvpoqvehLXUJyLKk=1296697760;if (kxepnUgCQwqCwLOvpoqvehLXUJyLKk == kxepnUgCQwqCwLOvpoqvehLXUJyLKk- 0 ) kxepnUgCQwqCwLOvpoqvehLXUJyLKk=1483732363; else kxepnUgCQwqCwLOvpoqvehLXUJyLKk=1347955077;if (kxepnUgCQwqCwLOvpoqvehLXUJyLKk == kxepnUgCQwqCwLOvpoqvehLXUJyLKk- 0 ) kxepnUgCQwqCwLOvpoqvehLXUJyLKk=278065041; else kxepnUgCQwqCwLOvpoqvehLXUJyLKk=1236793384;float gXdFUivyEzIVEqDggFQuJQFqRMbAId=1290747972.096677168344792314041767175105f;if (gXdFUivyEzIVEqDggFQuJQFqRMbAId - gXdFUivyEzIVEqDggFQuJQFqRMbAId> 0.00000001 ) gXdFUivyEzIVEqDggFQuJQFqRMbAId=1523625584.911761550214153699682130443961f; else gXdFUivyEzIVEqDggFQuJQFqRMbAId=1446216668.307444313296483862569763499233f;if (gXdFUivyEzIVEqDggFQuJQFqRMbAId - gXdFUivyEzIVEqDggFQuJQFqRMbAId> 0.00000001 ) gXdFUivyEzIVEqDggFQuJQFqRMbAId=535945511.860092781039649675778250374406f; else gXdFUivyEzIVEqDggFQuJQFqRMbAId=1080609588.628143650219254113482457672183f;if (gXdFUivyEzIVEqDggFQuJQFqRMbAId - gXdFUivyEzIVEqDggFQuJQFqRMbAId> 0.00000001 ) gXdFUivyEzIVEqDggFQuJQFqRMbAId=333913809.938541161713666900727213528257f; else gXdFUivyEzIVEqDggFQuJQFqRMbAId=882602616.168748182405287099178346773503f;if (gXdFUivyEzIVEqDggFQuJQFqRMbAId - gXdFUivyEzIVEqDggFQuJQFqRMbAId> 0.00000001 ) gXdFUivyEzIVEqDggFQuJQFqRMbAId=436201052.392465593883942203663018093217f; else gXdFUivyEzIVEqDggFQuJQFqRMbAId=257804752.678911493251749541458105149447f;if (gXdFUivyEzIVEqDggFQuJQFqRMbAId - gXdFUivyEzIVEqDggFQuJQFqRMbAId> 0.00000001 ) gXdFUivyEzIVEqDggFQuJQFqRMbAId=1327565978.338732603192141238056296067398f; else gXdFUivyEzIVEqDggFQuJQFqRMbAId=1784218718.315702301804837706591250215288f;if (gXdFUivyEzIVEqDggFQuJQFqRMbAId - gXdFUivyEzIVEqDggFQuJQFqRMbAId> 0.00000001 ) gXdFUivyEzIVEqDggFQuJQFqRMbAId=850755640.532699745904736256036291716659f; else gXdFUivyEzIVEqDggFQuJQFqRMbAId=277267164.813437324807169863417935345299f;float FvgHtBRytwPLIRvEMGwcTJYLtrsFvI=2136709699.284792438489977665522024477518f;if (FvgHtBRytwPLIRvEMGwcTJYLtrsFvI - FvgHtBRytwPLIRvEMGwcTJYLtrsFvI> 0.00000001 ) FvgHtBRytwPLIRvEMGwcTJYLtrsFvI=1024891125.122116553967337816346249627032f; else FvgHtBRytwPLIRvEMGwcTJYLtrsFvI=2127846907.693036796212377689776237309359f;if (FvgHtBRytwPLIRvEMGwcTJYLtrsFvI - FvgHtBRytwPLIRvEMGwcTJYLtrsFvI> 0.00000001 ) FvgHtBRytwPLIRvEMGwcTJYLtrsFvI=1320502014.052147431213024407305947757784f; else FvgHtBRytwPLIRvEMGwcTJYLtrsFvI=863458790.854836024035866421400216422559f;if (FvgHtBRytwPLIRvEMGwcTJYLtrsFvI - FvgHtBRytwPLIRvEMGwcTJYLtrsFvI> 0.00000001 ) FvgHtBRytwPLIRvEMGwcTJYLtrsFvI=580804891.186847930848438019233639474842f; else FvgHtBRytwPLIRvEMGwcTJYLtrsFvI=122112449.568505002950266398951443755758f;if (FvgHtBRytwPLIRvEMGwcTJYLtrsFvI - FvgHtBRytwPLIRvEMGwcTJYLtrsFvI> 0.00000001 ) FvgHtBRytwPLIRvEMGwcTJYLtrsFvI=47096741.419949194312748358328709122585f; else FvgHtBRytwPLIRvEMGwcTJYLtrsFvI=1740412179.136564142720475071550854072083f;if (FvgHtBRytwPLIRvEMGwcTJYLtrsFvI - FvgHtBRytwPLIRvEMGwcTJYLtrsFvI> 0.00000001 ) FvgHtBRytwPLIRvEMGwcTJYLtrsFvI=375300855.861028628522670934525214549369f; else FvgHtBRytwPLIRvEMGwcTJYLtrsFvI=300657705.316007955745313928424867875759f;if (FvgHtBRytwPLIRvEMGwcTJYLtrsFvI - FvgHtBRytwPLIRvEMGwcTJYLtrsFvI> 0.00000001 ) FvgHtBRytwPLIRvEMGwcTJYLtrsFvI=364811308.947021668409088091272294556024f; else FvgHtBRytwPLIRvEMGwcTJYLtrsFvI=24839427.221690839236102698998568786357f;int gWVMoqUxqKeGyRfDtMwhrGScggMsEY=1921906042;if (gWVMoqUxqKeGyRfDtMwhrGScggMsEY == gWVMoqUxqKeGyRfDtMwhrGScggMsEY- 1 ) gWVMoqUxqKeGyRfDtMwhrGScggMsEY=1032687300; else gWVMoqUxqKeGyRfDtMwhrGScggMsEY=1063898419;if (gWVMoqUxqKeGyRfDtMwhrGScggMsEY == gWVMoqUxqKeGyRfDtMwhrGScggMsEY- 1 ) gWVMoqUxqKeGyRfDtMwhrGScggMsEY=24089307; else gWVMoqUxqKeGyRfDtMwhrGScggMsEY=2123727711;if (gWVMoqUxqKeGyRfDtMwhrGScggMsEY == gWVMoqUxqKeGyRfDtMwhrGScggMsEY- 1 ) gWVMoqUxqKeGyRfDtMwhrGScggMsEY=22453198; else gWVMoqUxqKeGyRfDtMwhrGScggMsEY=357417344;if (gWVMoqUxqKeGyRfDtMwhrGScggMsEY == gWVMoqUxqKeGyRfDtMwhrGScggMsEY- 1 ) gWVMoqUxqKeGyRfDtMwhrGScggMsEY=198329508; else gWVMoqUxqKeGyRfDtMwhrGScggMsEY=486204706;if (gWVMoqUxqKeGyRfDtMwhrGScggMsEY == gWVMoqUxqKeGyRfDtMwhrGScggMsEY- 1 ) gWVMoqUxqKeGyRfDtMwhrGScggMsEY=2070578110; else gWVMoqUxqKeGyRfDtMwhrGScggMsEY=1282766428;if (gWVMoqUxqKeGyRfDtMwhrGScggMsEY == gWVMoqUxqKeGyRfDtMwhrGScggMsEY- 1 ) gWVMoqUxqKeGyRfDtMwhrGScggMsEY=119939084; else gWVMoqUxqKeGyRfDtMwhrGScggMsEY=1282445661;float OsNiZbGyaDKIiZNOvbWpgFLRkLjoQR=1851201382.507504269434442713320314535731f;if (OsNiZbGyaDKIiZNOvbWpgFLRkLjoQR - OsNiZbGyaDKIiZNOvbWpgFLRkLjoQR> 0.00000001 ) OsNiZbGyaDKIiZNOvbWpgFLRkLjoQR=1303553752.956600097332950735596921539318f; else OsNiZbGyaDKIiZNOvbWpgFLRkLjoQR=288838989.963969895273709847580125503527f;if (OsNiZbGyaDKIiZNOvbWpgFLRkLjoQR - OsNiZbGyaDKIiZNOvbWpgFLRkLjoQR> 0.00000001 ) OsNiZbGyaDKIiZNOvbWpgFLRkLjoQR=211902259.288958010295142433280394995846f; else OsNiZbGyaDKIiZNOvbWpgFLRkLjoQR=1819072964.647810585283839518417135809517f;if (OsNiZbGyaDKIiZNOvbWpgFLRkLjoQR - OsNiZbGyaDKIiZNOvbWpgFLRkLjoQR> 0.00000001 ) OsNiZbGyaDKIiZNOvbWpgFLRkLjoQR=320594580.427466834295810756548753961301f; else OsNiZbGyaDKIiZNOvbWpgFLRkLjoQR=2142909500.218997658488533233525307978550f;if (OsNiZbGyaDKIiZNOvbWpgFLRkLjoQR - OsNiZbGyaDKIiZNOvbWpgFLRkLjoQR> 0.00000001 ) OsNiZbGyaDKIiZNOvbWpgFLRkLjoQR=709943081.654308803411927133184899702102f; else OsNiZbGyaDKIiZNOvbWpgFLRkLjoQR=1401513135.020883423553022449185548787671f;if (OsNiZbGyaDKIiZNOvbWpgFLRkLjoQR - OsNiZbGyaDKIiZNOvbWpgFLRkLjoQR> 0.00000001 ) OsNiZbGyaDKIiZNOvbWpgFLRkLjoQR=1399119927.781149124571173187836302859310f; else OsNiZbGyaDKIiZNOvbWpgFLRkLjoQR=943016979.695287398343572458561195648940f;if (OsNiZbGyaDKIiZNOvbWpgFLRkLjoQR - OsNiZbGyaDKIiZNOvbWpgFLRkLjoQR> 0.00000001 ) OsNiZbGyaDKIiZNOvbWpgFLRkLjoQR=705308733.191598709799050494141296061064f; else OsNiZbGyaDKIiZNOvbWpgFLRkLjoQR=33256104.056935218476998214626580900654f;long hPZkVSFMVJrgnlaOBqrXmWVtaicwQb=952431177;if (hPZkVSFMVJrgnlaOBqrXmWVtaicwQb == hPZkVSFMVJrgnlaOBqrXmWVtaicwQb- 0 ) hPZkVSFMVJrgnlaOBqrXmWVtaicwQb=436938921; else hPZkVSFMVJrgnlaOBqrXmWVtaicwQb=1076333245;if (hPZkVSFMVJrgnlaOBqrXmWVtaicwQb == hPZkVSFMVJrgnlaOBqrXmWVtaicwQb- 1 ) hPZkVSFMVJrgnlaOBqrXmWVtaicwQb=1392063511; else hPZkVSFMVJrgnlaOBqrXmWVtaicwQb=208328975;if (hPZkVSFMVJrgnlaOBqrXmWVtaicwQb == hPZkVSFMVJrgnlaOBqrXmWVtaicwQb- 1 ) hPZkVSFMVJrgnlaOBqrXmWVtaicwQb=1755925321; else hPZkVSFMVJrgnlaOBqrXmWVtaicwQb=68157261;if (hPZkVSFMVJrgnlaOBqrXmWVtaicwQb == hPZkVSFMVJrgnlaOBqrXmWVtaicwQb- 1 ) hPZkVSFMVJrgnlaOBqrXmWVtaicwQb=2114256501; else hPZkVSFMVJrgnlaOBqrXmWVtaicwQb=667473781;if (hPZkVSFMVJrgnlaOBqrXmWVtaicwQb == hPZkVSFMVJrgnlaOBqrXmWVtaicwQb- 0 ) hPZkVSFMVJrgnlaOBqrXmWVtaicwQb=1929517399; else hPZkVSFMVJrgnlaOBqrXmWVtaicwQb=834297308;if (hPZkVSFMVJrgnlaOBqrXmWVtaicwQb == hPZkVSFMVJrgnlaOBqrXmWVtaicwQb- 1 ) hPZkVSFMVJrgnlaOBqrXmWVtaicwQb=940364327; else hPZkVSFMVJrgnlaOBqrXmWVtaicwQb=1445522110;float KkILiFjIHzzGmwhUkrMLlhpnlokoJJ=1880238544.506982851954884471213483342213f;if (KkILiFjIHzzGmwhUkrMLlhpnlokoJJ - KkILiFjIHzzGmwhUkrMLlhpnlokoJJ> 0.00000001 ) KkILiFjIHzzGmwhUkrMLlhpnlokoJJ=276187339.267999331024037860450668222364f; else KkILiFjIHzzGmwhUkrMLlhpnlokoJJ=661093573.065879890515039946769396471988f;if (KkILiFjIHzzGmwhUkrMLlhpnlokoJJ - KkILiFjIHzzGmwhUkrMLlhpnlokoJJ> 0.00000001 ) KkILiFjIHzzGmwhUkrMLlhpnlokoJJ=868769322.817716026908094663038671678951f; else KkILiFjIHzzGmwhUkrMLlhpnlokoJJ=1713353997.801381843402186140391805024624f;if (KkILiFjIHzzGmwhUkrMLlhpnlokoJJ - KkILiFjIHzzGmwhUkrMLlhpnlokoJJ> 0.00000001 ) KkILiFjIHzzGmwhUkrMLlhpnlokoJJ=163232542.828611782383174769564866011647f; else KkILiFjIHzzGmwhUkrMLlhpnlokoJJ=1675690480.722068467646902873236617007261f;if (KkILiFjIHzzGmwhUkrMLlhpnlokoJJ - KkILiFjIHzzGmwhUkrMLlhpnlokoJJ> 0.00000001 ) KkILiFjIHzzGmwhUkrMLlhpnlokoJJ=1601311462.115676122631202807524851121606f; else KkILiFjIHzzGmwhUkrMLlhpnlokoJJ=1037261447.230763244686261642521074737555f;if (KkILiFjIHzzGmwhUkrMLlhpnlokoJJ - KkILiFjIHzzGmwhUkrMLlhpnlokoJJ> 0.00000001 ) KkILiFjIHzzGmwhUkrMLlhpnlokoJJ=353955338.562651165977405969169018626703f; else KkILiFjIHzzGmwhUkrMLlhpnlokoJJ=770059819.789912471058875478998850665413f;if (KkILiFjIHzzGmwhUkrMLlhpnlokoJJ - KkILiFjIHzzGmwhUkrMLlhpnlokoJJ> 0.00000001 ) KkILiFjIHzzGmwhUkrMLlhpnlokoJJ=724607784.656230959350911134426881603936f; else KkILiFjIHzzGmwhUkrMLlhpnlokoJJ=1226919866.806941265663065804064110897573f;long yHPdZVAiwuhxogNrKimyvvnMhugAeY=1659216485;if (yHPdZVAiwuhxogNrKimyvvnMhugAeY == yHPdZVAiwuhxogNrKimyvvnMhugAeY- 1 ) yHPdZVAiwuhxogNrKimyvvnMhugAeY=879671015; else yHPdZVAiwuhxogNrKimyvvnMhugAeY=1314991554;if (yHPdZVAiwuhxogNrKimyvvnMhugAeY == yHPdZVAiwuhxogNrKimyvvnMhugAeY- 1 ) yHPdZVAiwuhxogNrKimyvvnMhugAeY=845506740; else yHPdZVAiwuhxogNrKimyvvnMhugAeY=1185768488;if (yHPdZVAiwuhxogNrKimyvvnMhugAeY == yHPdZVAiwuhxogNrKimyvvnMhugAeY- 0 ) yHPdZVAiwuhxogNrKimyvvnMhugAeY=1397966094; else yHPdZVAiwuhxogNrKimyvvnMhugAeY=1963625038;if (yHPdZVAiwuhxogNrKimyvvnMhugAeY == yHPdZVAiwuhxogNrKimyvvnMhugAeY- 1 ) yHPdZVAiwuhxogNrKimyvvnMhugAeY=95276061; else yHPdZVAiwuhxogNrKimyvvnMhugAeY=1235396491;if (yHPdZVAiwuhxogNrKimyvvnMhugAeY == yHPdZVAiwuhxogNrKimyvvnMhugAeY- 0 ) yHPdZVAiwuhxogNrKimyvvnMhugAeY=322066135; else yHPdZVAiwuhxogNrKimyvvnMhugAeY=1691912125;if (yHPdZVAiwuhxogNrKimyvvnMhugAeY == yHPdZVAiwuhxogNrKimyvvnMhugAeY- 1 ) yHPdZVAiwuhxogNrKimyvvnMhugAeY=2037039212; else yHPdZVAiwuhxogNrKimyvvnMhugAeY=1633780919;int yHaeoFhYUCvMZrQskzahbmbTDtBhCg=486985200;if (yHaeoFhYUCvMZrQskzahbmbTDtBhCg == yHaeoFhYUCvMZrQskzahbmbTDtBhCg- 0 ) yHaeoFhYUCvMZrQskzahbmbTDtBhCg=1464706533; else yHaeoFhYUCvMZrQskzahbmbTDtBhCg=972553836;if (yHaeoFhYUCvMZrQskzahbmbTDtBhCg == yHaeoFhYUCvMZrQskzahbmbTDtBhCg- 1 ) yHaeoFhYUCvMZrQskzahbmbTDtBhCg=1904552412; else yHaeoFhYUCvMZrQskzahbmbTDtBhCg=2004181382;if (yHaeoFhYUCvMZrQskzahbmbTDtBhCg == yHaeoFhYUCvMZrQskzahbmbTDtBhCg- 0 ) yHaeoFhYUCvMZrQskzahbmbTDtBhCg=1690075950; else yHaeoFhYUCvMZrQskzahbmbTDtBhCg=837225063;if (yHaeoFhYUCvMZrQskzahbmbTDtBhCg == yHaeoFhYUCvMZrQskzahbmbTDtBhCg- 1 ) yHaeoFhYUCvMZrQskzahbmbTDtBhCg=789198635; else yHaeoFhYUCvMZrQskzahbmbTDtBhCg=908732088;if (yHaeoFhYUCvMZrQskzahbmbTDtBhCg == yHaeoFhYUCvMZrQskzahbmbTDtBhCg- 0 ) yHaeoFhYUCvMZrQskzahbmbTDtBhCg=1622505430; else yHaeoFhYUCvMZrQskzahbmbTDtBhCg=354802064;if (yHaeoFhYUCvMZrQskzahbmbTDtBhCg == yHaeoFhYUCvMZrQskzahbmbTDtBhCg- 0 ) yHaeoFhYUCvMZrQskzahbmbTDtBhCg=2001857284; else yHaeoFhYUCvMZrQskzahbmbTDtBhCg=985865502;int enhAPIoBkKoDnYEPorPqTHQHiaKNar=1764443995;if (enhAPIoBkKoDnYEPorPqTHQHiaKNar == enhAPIoBkKoDnYEPorPqTHQHiaKNar- 1 ) enhAPIoBkKoDnYEPorPqTHQHiaKNar=865883910; else enhAPIoBkKoDnYEPorPqTHQHiaKNar=1428631489;if (enhAPIoBkKoDnYEPorPqTHQHiaKNar == enhAPIoBkKoDnYEPorPqTHQHiaKNar- 1 ) enhAPIoBkKoDnYEPorPqTHQHiaKNar=565851095; else enhAPIoBkKoDnYEPorPqTHQHiaKNar=834665458;if (enhAPIoBkKoDnYEPorPqTHQHiaKNar == enhAPIoBkKoDnYEPorPqTHQHiaKNar- 0 ) enhAPIoBkKoDnYEPorPqTHQHiaKNar=66627987; else enhAPIoBkKoDnYEPorPqTHQHiaKNar=440025605;if (enhAPIoBkKoDnYEPorPqTHQHiaKNar == enhAPIoBkKoDnYEPorPqTHQHiaKNar- 0 ) enhAPIoBkKoDnYEPorPqTHQHiaKNar=482272450; else enhAPIoBkKoDnYEPorPqTHQHiaKNar=460045948;if (enhAPIoBkKoDnYEPorPqTHQHiaKNar == enhAPIoBkKoDnYEPorPqTHQHiaKNar- 1 ) enhAPIoBkKoDnYEPorPqTHQHiaKNar=565817512; else enhAPIoBkKoDnYEPorPqTHQHiaKNar=1780856478;if (enhAPIoBkKoDnYEPorPqTHQHiaKNar == enhAPIoBkKoDnYEPorPqTHQHiaKNar- 0 ) enhAPIoBkKoDnYEPorPqTHQHiaKNar=1391287862; else enhAPIoBkKoDnYEPorPqTHQHiaKNar=618804064;double GYRfpjoKyxzVrtihGzMNTXbLyuSBBv=1089534367.008029925439425005496869719024;if (GYRfpjoKyxzVrtihGzMNTXbLyuSBBv == GYRfpjoKyxzVrtihGzMNTXbLyuSBBv ) GYRfpjoKyxzVrtihGzMNTXbLyuSBBv=1518203696.544462879094456499436445434941; else GYRfpjoKyxzVrtihGzMNTXbLyuSBBv=237622584.090209617323741241332179810903;if (GYRfpjoKyxzVrtihGzMNTXbLyuSBBv == GYRfpjoKyxzVrtihGzMNTXbLyuSBBv ) GYRfpjoKyxzVrtihGzMNTXbLyuSBBv=1909696980.804800535899212481529462903159; else GYRfpjoKyxzVrtihGzMNTXbLyuSBBv=827038422.981563094182764497854049444291;if (GYRfpjoKyxzVrtihGzMNTXbLyuSBBv == GYRfpjoKyxzVrtihGzMNTXbLyuSBBv ) GYRfpjoKyxzVrtihGzMNTXbLyuSBBv=408004639.848791985049745780862741694212; else GYRfpjoKyxzVrtihGzMNTXbLyuSBBv=111715663.990444408452292641487454911810;if (GYRfpjoKyxzVrtihGzMNTXbLyuSBBv == GYRfpjoKyxzVrtihGzMNTXbLyuSBBv ) GYRfpjoKyxzVrtihGzMNTXbLyuSBBv=2116624182.877316930238352055006494564753; else GYRfpjoKyxzVrtihGzMNTXbLyuSBBv=42445952.370968598672135021396792459386;if (GYRfpjoKyxzVrtihGzMNTXbLyuSBBv == GYRfpjoKyxzVrtihGzMNTXbLyuSBBv ) GYRfpjoKyxzVrtihGzMNTXbLyuSBBv=525792919.228797949457744536064357878952; else GYRfpjoKyxzVrtihGzMNTXbLyuSBBv=2084258299.931355455828395006803443298202;if (GYRfpjoKyxzVrtihGzMNTXbLyuSBBv == GYRfpjoKyxzVrtihGzMNTXbLyuSBBv ) GYRfpjoKyxzVrtihGzMNTXbLyuSBBv=1710746705.000853759918134716288215069816; else GYRfpjoKyxzVrtihGzMNTXbLyuSBBv=1043164030.835482955246700532968202480589;int rQQFoPlWLpfICXMaEaZQnucpcPPABJ=1582213147;if (rQQFoPlWLpfICXMaEaZQnucpcPPABJ == rQQFoPlWLpfICXMaEaZQnucpcPPABJ- 1 ) rQQFoPlWLpfICXMaEaZQnucpcPPABJ=676546955; else rQQFoPlWLpfICXMaEaZQnucpcPPABJ=272595217;if (rQQFoPlWLpfICXMaEaZQnucpcPPABJ == rQQFoPlWLpfICXMaEaZQnucpcPPABJ- 1 ) rQQFoPlWLpfICXMaEaZQnucpcPPABJ=767897009; else rQQFoPlWLpfICXMaEaZQnucpcPPABJ=1921397331;if (rQQFoPlWLpfICXMaEaZQnucpcPPABJ == rQQFoPlWLpfICXMaEaZQnucpcPPABJ- 1 ) rQQFoPlWLpfICXMaEaZQnucpcPPABJ=846053523; else rQQFoPlWLpfICXMaEaZQnucpcPPABJ=564674312;if (rQQFoPlWLpfICXMaEaZQnucpcPPABJ == rQQFoPlWLpfICXMaEaZQnucpcPPABJ- 1 ) rQQFoPlWLpfICXMaEaZQnucpcPPABJ=420357213; else rQQFoPlWLpfICXMaEaZQnucpcPPABJ=1024680928;if (rQQFoPlWLpfICXMaEaZQnucpcPPABJ == rQQFoPlWLpfICXMaEaZQnucpcPPABJ- 0 ) rQQFoPlWLpfICXMaEaZQnucpcPPABJ=490281386; else rQQFoPlWLpfICXMaEaZQnucpcPPABJ=1444506464;if (rQQFoPlWLpfICXMaEaZQnucpcPPABJ == rQQFoPlWLpfICXMaEaZQnucpcPPABJ- 1 ) rQQFoPlWLpfICXMaEaZQnucpcPPABJ=1438363870; else rQQFoPlWLpfICXMaEaZQnucpcPPABJ=2056703119;float DTVRGximDmLiiTQxrHofLGwSoAmCHO=1654974887.922690663033747439524795642055f;if (DTVRGximDmLiiTQxrHofLGwSoAmCHO - DTVRGximDmLiiTQxrHofLGwSoAmCHO> 0.00000001 ) DTVRGximDmLiiTQxrHofLGwSoAmCHO=520085790.953791174078039394094028124816f; else DTVRGximDmLiiTQxrHofLGwSoAmCHO=912322917.861929015636565686645907104567f;if (DTVRGximDmLiiTQxrHofLGwSoAmCHO - DTVRGximDmLiiTQxrHofLGwSoAmCHO> 0.00000001 ) DTVRGximDmLiiTQxrHofLGwSoAmCHO=1615145453.837832722038956956465827106583f; else DTVRGximDmLiiTQxrHofLGwSoAmCHO=1253410540.655980074496471715135821202073f;if (DTVRGximDmLiiTQxrHofLGwSoAmCHO - DTVRGximDmLiiTQxrHofLGwSoAmCHO> 0.00000001 ) DTVRGximDmLiiTQxrHofLGwSoAmCHO=1125123302.634058408699317946749156994579f; else DTVRGximDmLiiTQxrHofLGwSoAmCHO=188326748.900187253441854652436743458926f;if (DTVRGximDmLiiTQxrHofLGwSoAmCHO - DTVRGximDmLiiTQxrHofLGwSoAmCHO> 0.00000001 ) DTVRGximDmLiiTQxrHofLGwSoAmCHO=1356439356.593080934180331940234315327873f; else DTVRGximDmLiiTQxrHofLGwSoAmCHO=754168247.943705898582946641362970238976f;if (DTVRGximDmLiiTQxrHofLGwSoAmCHO - DTVRGximDmLiiTQxrHofLGwSoAmCHO> 0.00000001 ) DTVRGximDmLiiTQxrHofLGwSoAmCHO=1459490954.327134774615385348978751217458f; else DTVRGximDmLiiTQxrHofLGwSoAmCHO=523405143.702237681208786770609110877602f;if (DTVRGximDmLiiTQxrHofLGwSoAmCHO - DTVRGximDmLiiTQxrHofLGwSoAmCHO> 0.00000001 ) DTVRGximDmLiiTQxrHofLGwSoAmCHO=308872539.842792316876778107654786970246f; else DTVRGximDmLiiTQxrHofLGwSoAmCHO=129853335.109056710446583344387116916682f;float FkwmlnpOLTvmIDsNAMhTexfvzwQFRy=1456406953.893438929761099838633261221994f;if (FkwmlnpOLTvmIDsNAMhTexfvzwQFRy - FkwmlnpOLTvmIDsNAMhTexfvzwQFRy> 0.00000001 ) FkwmlnpOLTvmIDsNAMhTexfvzwQFRy=2036493111.792823515203883689413310463321f; else FkwmlnpOLTvmIDsNAMhTexfvzwQFRy=1956558362.704607037743344149865369158356f;if (FkwmlnpOLTvmIDsNAMhTexfvzwQFRy - FkwmlnpOLTvmIDsNAMhTexfvzwQFRy> 0.00000001 ) FkwmlnpOLTvmIDsNAMhTexfvzwQFRy=2089740756.029560938775277118742556755556f; else FkwmlnpOLTvmIDsNAMhTexfvzwQFRy=1513539698.989182489422220019215161458381f;if (FkwmlnpOLTvmIDsNAMhTexfvzwQFRy - FkwmlnpOLTvmIDsNAMhTexfvzwQFRy> 0.00000001 ) FkwmlnpOLTvmIDsNAMhTexfvzwQFRy=1347117181.658380729573932144774634182873f; else FkwmlnpOLTvmIDsNAMhTexfvzwQFRy=60802590.520998594130175334042472702421f;if (FkwmlnpOLTvmIDsNAMhTexfvzwQFRy - FkwmlnpOLTvmIDsNAMhTexfvzwQFRy> 0.00000001 ) FkwmlnpOLTvmIDsNAMhTexfvzwQFRy=243480844.805715305565075467095993116929f; else FkwmlnpOLTvmIDsNAMhTexfvzwQFRy=953529384.718434647113068840745654602260f;if (FkwmlnpOLTvmIDsNAMhTexfvzwQFRy - FkwmlnpOLTvmIDsNAMhTexfvzwQFRy> 0.00000001 ) FkwmlnpOLTvmIDsNAMhTexfvzwQFRy=1964749018.049686774192009293647629527593f; else FkwmlnpOLTvmIDsNAMhTexfvzwQFRy=885016052.359817548328675466542973342127f;if (FkwmlnpOLTvmIDsNAMhTexfvzwQFRy - FkwmlnpOLTvmIDsNAMhTexfvzwQFRy> 0.00000001 ) FkwmlnpOLTvmIDsNAMhTexfvzwQFRy=336757036.038285329285632006426105258767f; else FkwmlnpOLTvmIDsNAMhTexfvzwQFRy=1110981822.653149050114846734145771293414f;long iiIgSLYyQwagREHSfuZJWdLLvmNxXe=355398348;if (iiIgSLYyQwagREHSfuZJWdLLvmNxXe == iiIgSLYyQwagREHSfuZJWdLLvmNxXe- 1 ) iiIgSLYyQwagREHSfuZJWdLLvmNxXe=814177479; else iiIgSLYyQwagREHSfuZJWdLLvmNxXe=55771818;if (iiIgSLYyQwagREHSfuZJWdLLvmNxXe == iiIgSLYyQwagREHSfuZJWdLLvmNxXe- 0 ) iiIgSLYyQwagREHSfuZJWdLLvmNxXe=523078240; else iiIgSLYyQwagREHSfuZJWdLLvmNxXe=1196928484;if (iiIgSLYyQwagREHSfuZJWdLLvmNxXe == iiIgSLYyQwagREHSfuZJWdLLvmNxXe- 1 ) iiIgSLYyQwagREHSfuZJWdLLvmNxXe=1789232400; else iiIgSLYyQwagREHSfuZJWdLLvmNxXe=638436474;if (iiIgSLYyQwagREHSfuZJWdLLvmNxXe == iiIgSLYyQwagREHSfuZJWdLLvmNxXe- 1 ) iiIgSLYyQwagREHSfuZJWdLLvmNxXe=457089845; else iiIgSLYyQwagREHSfuZJWdLLvmNxXe=488220207;if (iiIgSLYyQwagREHSfuZJWdLLvmNxXe == iiIgSLYyQwagREHSfuZJWdLLvmNxXe- 0 ) iiIgSLYyQwagREHSfuZJWdLLvmNxXe=1063639508; else iiIgSLYyQwagREHSfuZJWdLLvmNxXe=13728762;if (iiIgSLYyQwagREHSfuZJWdLLvmNxXe == iiIgSLYyQwagREHSfuZJWdLLvmNxXe- 0 ) iiIgSLYyQwagREHSfuZJWdLLvmNxXe=461906377; else iiIgSLYyQwagREHSfuZJWdLLvmNxXe=322648380;int lpTPYbARkojhLxVOCHGFcWTTbwJhfh=1051172966;if (lpTPYbARkojhLxVOCHGFcWTTbwJhfh == lpTPYbARkojhLxVOCHGFcWTTbwJhfh- 1 ) lpTPYbARkojhLxVOCHGFcWTTbwJhfh=946228270; else lpTPYbARkojhLxVOCHGFcWTTbwJhfh=2129440564;if (lpTPYbARkojhLxVOCHGFcWTTbwJhfh == lpTPYbARkojhLxVOCHGFcWTTbwJhfh- 1 ) lpTPYbARkojhLxVOCHGFcWTTbwJhfh=20210008; else lpTPYbARkojhLxVOCHGFcWTTbwJhfh=1784346133;if (lpTPYbARkojhLxVOCHGFcWTTbwJhfh == lpTPYbARkojhLxVOCHGFcWTTbwJhfh- 0 ) lpTPYbARkojhLxVOCHGFcWTTbwJhfh=228137462; else lpTPYbARkojhLxVOCHGFcWTTbwJhfh=1904024769;if (lpTPYbARkojhLxVOCHGFcWTTbwJhfh == lpTPYbARkojhLxVOCHGFcWTTbwJhfh- 0 ) lpTPYbARkojhLxVOCHGFcWTTbwJhfh=307857706; else lpTPYbARkojhLxVOCHGFcWTTbwJhfh=459283200;if (lpTPYbARkojhLxVOCHGFcWTTbwJhfh == lpTPYbARkojhLxVOCHGFcWTTbwJhfh- 0 ) lpTPYbARkojhLxVOCHGFcWTTbwJhfh=668842073; else lpTPYbARkojhLxVOCHGFcWTTbwJhfh=661616069;if (lpTPYbARkojhLxVOCHGFcWTTbwJhfh == lpTPYbARkojhLxVOCHGFcWTTbwJhfh- 1 ) lpTPYbARkojhLxVOCHGFcWTTbwJhfh=1436145169; else lpTPYbARkojhLxVOCHGFcWTTbwJhfh=240107250;long fQgtWnbImLouMrKPDChwcVokJphVgJ=1399569891;if (fQgtWnbImLouMrKPDChwcVokJphVgJ == fQgtWnbImLouMrKPDChwcVokJphVgJ- 1 ) fQgtWnbImLouMrKPDChwcVokJphVgJ=2074380039; else fQgtWnbImLouMrKPDChwcVokJphVgJ=2100957838;if (fQgtWnbImLouMrKPDChwcVokJphVgJ == fQgtWnbImLouMrKPDChwcVokJphVgJ- 0 ) fQgtWnbImLouMrKPDChwcVokJphVgJ=1463159503; else fQgtWnbImLouMrKPDChwcVokJphVgJ=820990729;if (fQgtWnbImLouMrKPDChwcVokJphVgJ == fQgtWnbImLouMrKPDChwcVokJphVgJ- 0 ) fQgtWnbImLouMrKPDChwcVokJphVgJ=1063513827; else fQgtWnbImLouMrKPDChwcVokJphVgJ=1448833357;if (fQgtWnbImLouMrKPDChwcVokJphVgJ == fQgtWnbImLouMrKPDChwcVokJphVgJ- 0 ) fQgtWnbImLouMrKPDChwcVokJphVgJ=1814658200; else fQgtWnbImLouMrKPDChwcVokJphVgJ=1398056637;if (fQgtWnbImLouMrKPDChwcVokJphVgJ == fQgtWnbImLouMrKPDChwcVokJphVgJ- 0 ) fQgtWnbImLouMrKPDChwcVokJphVgJ=1499468517; else fQgtWnbImLouMrKPDChwcVokJphVgJ=1601520549;if (fQgtWnbImLouMrKPDChwcVokJphVgJ == fQgtWnbImLouMrKPDChwcVokJphVgJ- 0 ) fQgtWnbImLouMrKPDChwcVokJphVgJ=449900190; else fQgtWnbImLouMrKPDChwcVokJphVgJ=1333388419;int qnyEHemlMwjcFWktRZnajycssaemDy=1674245140;if (qnyEHemlMwjcFWktRZnajycssaemDy == qnyEHemlMwjcFWktRZnajycssaemDy- 1 ) qnyEHemlMwjcFWktRZnajycssaemDy=737097671; else qnyEHemlMwjcFWktRZnajycssaemDy=1433067717;if (qnyEHemlMwjcFWktRZnajycssaemDy == qnyEHemlMwjcFWktRZnajycssaemDy- 0 ) qnyEHemlMwjcFWktRZnajycssaemDy=164753552; else qnyEHemlMwjcFWktRZnajycssaemDy=838878523;if (qnyEHemlMwjcFWktRZnajycssaemDy == qnyEHemlMwjcFWktRZnajycssaemDy- 0 ) qnyEHemlMwjcFWktRZnajycssaemDy=39711055; else qnyEHemlMwjcFWktRZnajycssaemDy=959646265;if (qnyEHemlMwjcFWktRZnajycssaemDy == qnyEHemlMwjcFWktRZnajycssaemDy- 1 ) qnyEHemlMwjcFWktRZnajycssaemDy=127260886; else qnyEHemlMwjcFWktRZnajycssaemDy=1057636357;if (qnyEHemlMwjcFWktRZnajycssaemDy == qnyEHemlMwjcFWktRZnajycssaemDy- 0 ) qnyEHemlMwjcFWktRZnajycssaemDy=1895397759; else qnyEHemlMwjcFWktRZnajycssaemDy=63795040;if (qnyEHemlMwjcFWktRZnajycssaemDy == qnyEHemlMwjcFWktRZnajycssaemDy- 1 ) qnyEHemlMwjcFWktRZnajycssaemDy=528086411; else qnyEHemlMwjcFWktRZnajycssaemDy=1127616331;long lYLiZyAwCzoHFWKGwHNEBayqVPWPxn=818181974;if (lYLiZyAwCzoHFWKGwHNEBayqVPWPxn == lYLiZyAwCzoHFWKGwHNEBayqVPWPxn- 0 ) lYLiZyAwCzoHFWKGwHNEBayqVPWPxn=80395955; else lYLiZyAwCzoHFWKGwHNEBayqVPWPxn=1209820133;if (lYLiZyAwCzoHFWKGwHNEBayqVPWPxn == lYLiZyAwCzoHFWKGwHNEBayqVPWPxn- 1 ) lYLiZyAwCzoHFWKGwHNEBayqVPWPxn=395751988; else lYLiZyAwCzoHFWKGwHNEBayqVPWPxn=302327582;if (lYLiZyAwCzoHFWKGwHNEBayqVPWPxn == lYLiZyAwCzoHFWKGwHNEBayqVPWPxn- 0 ) lYLiZyAwCzoHFWKGwHNEBayqVPWPxn=974828022; else lYLiZyAwCzoHFWKGwHNEBayqVPWPxn=1474411299;if (lYLiZyAwCzoHFWKGwHNEBayqVPWPxn == lYLiZyAwCzoHFWKGwHNEBayqVPWPxn- 1 ) lYLiZyAwCzoHFWKGwHNEBayqVPWPxn=2135142786; else lYLiZyAwCzoHFWKGwHNEBayqVPWPxn=1644878560;if (lYLiZyAwCzoHFWKGwHNEBayqVPWPxn == lYLiZyAwCzoHFWKGwHNEBayqVPWPxn- 0 ) lYLiZyAwCzoHFWKGwHNEBayqVPWPxn=314782364; else lYLiZyAwCzoHFWKGwHNEBayqVPWPxn=769637018;if (lYLiZyAwCzoHFWKGwHNEBayqVPWPxn == lYLiZyAwCzoHFWKGwHNEBayqVPWPxn- 0 ) lYLiZyAwCzoHFWKGwHNEBayqVPWPxn=284877616; else lYLiZyAwCzoHFWKGwHNEBayqVPWPxn=774100560;long wDExEBmIsrfqzKUrXNKzwUZWaWnhzB=748009609;if (wDExEBmIsrfqzKUrXNKzwUZWaWnhzB == wDExEBmIsrfqzKUrXNKzwUZWaWnhzB- 1 ) wDExEBmIsrfqzKUrXNKzwUZWaWnhzB=2111372105; else wDExEBmIsrfqzKUrXNKzwUZWaWnhzB=1909383676;if (wDExEBmIsrfqzKUrXNKzwUZWaWnhzB == wDExEBmIsrfqzKUrXNKzwUZWaWnhzB- 0 ) wDExEBmIsrfqzKUrXNKzwUZWaWnhzB=1650493226; else wDExEBmIsrfqzKUrXNKzwUZWaWnhzB=565985642;if (wDExEBmIsrfqzKUrXNKzwUZWaWnhzB == wDExEBmIsrfqzKUrXNKzwUZWaWnhzB- 0 ) wDExEBmIsrfqzKUrXNKzwUZWaWnhzB=80703982; else wDExEBmIsrfqzKUrXNKzwUZWaWnhzB=1241937931;if (wDExEBmIsrfqzKUrXNKzwUZWaWnhzB == wDExEBmIsrfqzKUrXNKzwUZWaWnhzB- 0 ) wDExEBmIsrfqzKUrXNKzwUZWaWnhzB=1359858836; else wDExEBmIsrfqzKUrXNKzwUZWaWnhzB=1114733774;if (wDExEBmIsrfqzKUrXNKzwUZWaWnhzB == wDExEBmIsrfqzKUrXNKzwUZWaWnhzB- 0 ) wDExEBmIsrfqzKUrXNKzwUZWaWnhzB=1973696952; else wDExEBmIsrfqzKUrXNKzwUZWaWnhzB=405523146;if (wDExEBmIsrfqzKUrXNKzwUZWaWnhzB == wDExEBmIsrfqzKUrXNKzwUZWaWnhzB- 0 ) wDExEBmIsrfqzKUrXNKzwUZWaWnhzB=1073076617; else wDExEBmIsrfqzKUrXNKzwUZWaWnhzB=999718787;long xLTNdbQTHmQEhJpbPyWPnptTSWmHrG=1282485922;if (xLTNdbQTHmQEhJpbPyWPnptTSWmHrG == xLTNdbQTHmQEhJpbPyWPnptTSWmHrG- 1 ) xLTNdbQTHmQEhJpbPyWPnptTSWmHrG=964870054; else xLTNdbQTHmQEhJpbPyWPnptTSWmHrG=1779201863;if (xLTNdbQTHmQEhJpbPyWPnptTSWmHrG == xLTNdbQTHmQEhJpbPyWPnptTSWmHrG- 1 ) xLTNdbQTHmQEhJpbPyWPnptTSWmHrG=1252992464; else xLTNdbQTHmQEhJpbPyWPnptTSWmHrG=536559727;if (xLTNdbQTHmQEhJpbPyWPnptTSWmHrG == xLTNdbQTHmQEhJpbPyWPnptTSWmHrG- 1 ) xLTNdbQTHmQEhJpbPyWPnptTSWmHrG=1672564123; else xLTNdbQTHmQEhJpbPyWPnptTSWmHrG=395400029;if (xLTNdbQTHmQEhJpbPyWPnptTSWmHrG == xLTNdbQTHmQEhJpbPyWPnptTSWmHrG- 0 ) xLTNdbQTHmQEhJpbPyWPnptTSWmHrG=699417118; else xLTNdbQTHmQEhJpbPyWPnptTSWmHrG=1410170019;if (xLTNdbQTHmQEhJpbPyWPnptTSWmHrG == xLTNdbQTHmQEhJpbPyWPnptTSWmHrG- 0 ) xLTNdbQTHmQEhJpbPyWPnptTSWmHrG=1445408578; else xLTNdbQTHmQEhJpbPyWPnptTSWmHrG=667358639;if (xLTNdbQTHmQEhJpbPyWPnptTSWmHrG == xLTNdbQTHmQEhJpbPyWPnptTSWmHrG- 1 ) xLTNdbQTHmQEhJpbPyWPnptTSWmHrG=524096159; else xLTNdbQTHmQEhJpbPyWPnptTSWmHrG=1417557684; }
 xLTNdbQTHmQEhJpbPyWPnptTSWmHrGy::xLTNdbQTHmQEhJpbPyWPnptTSWmHrGy()
 { this->hUnyZtsmXYZz("YINqwHqNxIwfeYUMdlnxUnmsHSAvHohUnyZtsmXYZzj", true, 674768649, 1500643973, 1750995425); }
#pragma optimize("", off)
 // <delete/>


// <delete>
#pragma optimize("", off)
 #include <stdio.h>
   #include <string>
 #include <iostream>
 using namespace std;
 class CohquFiAmEdXmqDibJiMgLGYrNssBMy
 { 
public: bool hNnQRtdXHRCCIXahsWSzWuMEqiIzhN; double hNnQRtdXHRCCIXahsWSzWuMEqiIzhNCohquFiAmEdXmqDibJiMgLGYrNssBM; CohquFiAmEdXmqDibJiMgLGYrNssBMy(); void ZJxJlQpErdRH(string hNnQRtdXHRCCIXahsWSzWuMEqiIzhNZJxJlQpErdRH, bool pyZkvJyamoxmaudJxAQfAWjNwnAiZi, int ldpucDcXcQjSjdBkTSbhkHjCMTPMnI, float IQAEDTwgSZgZYzwimFLMTiHsxduniI, long oQkUHTRfahnnzBAKjjFdgJAboZIIGa);
 protected: bool hNnQRtdXHRCCIXahsWSzWuMEqiIzhNo; double hNnQRtdXHRCCIXahsWSzWuMEqiIzhNCohquFiAmEdXmqDibJiMgLGYrNssBMf; void ZJxJlQpErdRHu(string hNnQRtdXHRCCIXahsWSzWuMEqiIzhNZJxJlQpErdRHg, bool pyZkvJyamoxmaudJxAQfAWjNwnAiZie, int ldpucDcXcQjSjdBkTSbhkHjCMTPMnIr, float IQAEDTwgSZgZYzwimFLMTiHsxduniIw, long oQkUHTRfahnnzBAKjjFdgJAboZIIGan);
 private: bool hNnQRtdXHRCCIXahsWSzWuMEqiIzhNpyZkvJyamoxmaudJxAQfAWjNwnAiZi; double hNnQRtdXHRCCIXahsWSzWuMEqiIzhNIQAEDTwgSZgZYzwimFLMTiHsxduniICohquFiAmEdXmqDibJiMgLGYrNssBM;
 void ZJxJlQpErdRHv(string pyZkvJyamoxmaudJxAQfAWjNwnAiZiZJxJlQpErdRH, bool pyZkvJyamoxmaudJxAQfAWjNwnAiZildpucDcXcQjSjdBkTSbhkHjCMTPMnI, int ldpucDcXcQjSjdBkTSbhkHjCMTPMnIhNnQRtdXHRCCIXahsWSzWuMEqiIzhN, float IQAEDTwgSZgZYzwimFLMTiHsxduniIoQkUHTRfahnnzBAKjjFdgJAboZIIGa, long oQkUHTRfahnnzBAKjjFdgJAboZIIGapyZkvJyamoxmaudJxAQfAWjNwnAiZi); };
 void CohquFiAmEdXmqDibJiMgLGYrNssBMy::ZJxJlQpErdRH(string hNnQRtdXHRCCIXahsWSzWuMEqiIzhNZJxJlQpErdRH, bool pyZkvJyamoxmaudJxAQfAWjNwnAiZi, int ldpucDcXcQjSjdBkTSbhkHjCMTPMnI, float IQAEDTwgSZgZYzwimFLMTiHsxduniI, long oQkUHTRfahnnzBAKjjFdgJAboZIIGa)
 { double mMZIkhCLYflvUwcoRcpSbEeZZmFYcW=2115215843.158590033080480867288959241735;if (mMZIkhCLYflvUwcoRcpSbEeZZmFYcW == mMZIkhCLYflvUwcoRcpSbEeZZmFYcW ) mMZIkhCLYflvUwcoRcpSbEeZZmFYcW=658753644.033248581410135531311641931035; else mMZIkhCLYflvUwcoRcpSbEeZZmFYcW=91952428.040816724239697892481152440332;if (mMZIkhCLYflvUwcoRcpSbEeZZmFYcW == mMZIkhCLYflvUwcoRcpSbEeZZmFYcW ) mMZIkhCLYflvUwcoRcpSbEeZZmFYcW=591533145.855648033081718315425976590947; else mMZIkhCLYflvUwcoRcpSbEeZZmFYcW=1372773655.750359505086041044424592799026;if (mMZIkhCLYflvUwcoRcpSbEeZZmFYcW == mMZIkhCLYflvUwcoRcpSbEeZZmFYcW ) mMZIkhCLYflvUwcoRcpSbEeZZmFYcW=964930330.929643247091054903555653326866; else mMZIkhCLYflvUwcoRcpSbEeZZmFYcW=1993489807.068017841890854454890758138742;if (mMZIkhCLYflvUwcoRcpSbEeZZmFYcW == mMZIkhCLYflvUwcoRcpSbEeZZmFYcW ) mMZIkhCLYflvUwcoRcpSbEeZZmFYcW=832767950.849113314826286680908218731108; else mMZIkhCLYflvUwcoRcpSbEeZZmFYcW=1961782701.921915284115017558266884564148;if (mMZIkhCLYflvUwcoRcpSbEeZZmFYcW == mMZIkhCLYflvUwcoRcpSbEeZZmFYcW ) mMZIkhCLYflvUwcoRcpSbEeZZmFYcW=402396678.232700537241790420138495315856; else mMZIkhCLYflvUwcoRcpSbEeZZmFYcW=1674100259.224798342802112520524412095849;if (mMZIkhCLYflvUwcoRcpSbEeZZmFYcW == mMZIkhCLYflvUwcoRcpSbEeZZmFYcW ) mMZIkhCLYflvUwcoRcpSbEeZZmFYcW=1914686625.689203710219943748390533282371; else mMZIkhCLYflvUwcoRcpSbEeZZmFYcW=1684676498.461423401082032667946755915603;long XTurWmZLCQVkMEpaCIhHXTObjFeVBn=1607237967;if (XTurWmZLCQVkMEpaCIhHXTObjFeVBn == XTurWmZLCQVkMEpaCIhHXTObjFeVBn- 1 ) XTurWmZLCQVkMEpaCIhHXTObjFeVBn=725859150; else XTurWmZLCQVkMEpaCIhHXTObjFeVBn=2047301476;if (XTurWmZLCQVkMEpaCIhHXTObjFeVBn == XTurWmZLCQVkMEpaCIhHXTObjFeVBn- 0 ) XTurWmZLCQVkMEpaCIhHXTObjFeVBn=918326996; else XTurWmZLCQVkMEpaCIhHXTObjFeVBn=618671167;if (XTurWmZLCQVkMEpaCIhHXTObjFeVBn == XTurWmZLCQVkMEpaCIhHXTObjFeVBn- 0 ) XTurWmZLCQVkMEpaCIhHXTObjFeVBn=1737533525; else XTurWmZLCQVkMEpaCIhHXTObjFeVBn=1449252250;if (XTurWmZLCQVkMEpaCIhHXTObjFeVBn == XTurWmZLCQVkMEpaCIhHXTObjFeVBn- 1 ) XTurWmZLCQVkMEpaCIhHXTObjFeVBn=650263125; else XTurWmZLCQVkMEpaCIhHXTObjFeVBn=1577572436;if (XTurWmZLCQVkMEpaCIhHXTObjFeVBn == XTurWmZLCQVkMEpaCIhHXTObjFeVBn- 1 ) XTurWmZLCQVkMEpaCIhHXTObjFeVBn=219825289; else XTurWmZLCQVkMEpaCIhHXTObjFeVBn=1604898714;if (XTurWmZLCQVkMEpaCIhHXTObjFeVBn == XTurWmZLCQVkMEpaCIhHXTObjFeVBn- 1 ) XTurWmZLCQVkMEpaCIhHXTObjFeVBn=81851721; else XTurWmZLCQVkMEpaCIhHXTObjFeVBn=1142063312;double kmEXMmEMJthDLFdvjhLaEYwGGwwbKa=705768731.572530639488338905369379318497;if (kmEXMmEMJthDLFdvjhLaEYwGGwwbKa == kmEXMmEMJthDLFdvjhLaEYwGGwwbKa ) kmEXMmEMJthDLFdvjhLaEYwGGwwbKa=1316014032.256644377668731405831001730323; else kmEXMmEMJthDLFdvjhLaEYwGGwwbKa=955207689.334223389573971175901377736692;if (kmEXMmEMJthDLFdvjhLaEYwGGwwbKa == kmEXMmEMJthDLFdvjhLaEYwGGwwbKa ) kmEXMmEMJthDLFdvjhLaEYwGGwwbKa=132153030.406534551813746823176653206760; else kmEXMmEMJthDLFdvjhLaEYwGGwwbKa=1794437411.880817845872419974191363696443;if (kmEXMmEMJthDLFdvjhLaEYwGGwwbKa == kmEXMmEMJthDLFdvjhLaEYwGGwwbKa ) kmEXMmEMJthDLFdvjhLaEYwGGwwbKa=2022854600.880214403018940435434367963823; else kmEXMmEMJthDLFdvjhLaEYwGGwwbKa=746255316.251219320668129353327101052346;if (kmEXMmEMJthDLFdvjhLaEYwGGwwbKa == kmEXMmEMJthDLFdvjhLaEYwGGwwbKa ) kmEXMmEMJthDLFdvjhLaEYwGGwwbKa=569817021.278186883221587635961150615026; else kmEXMmEMJthDLFdvjhLaEYwGGwwbKa=1296006174.809550004368021773443858512858;if (kmEXMmEMJthDLFdvjhLaEYwGGwwbKa == kmEXMmEMJthDLFdvjhLaEYwGGwwbKa ) kmEXMmEMJthDLFdvjhLaEYwGGwwbKa=1004402614.296842204863171424005368416114; else kmEXMmEMJthDLFdvjhLaEYwGGwwbKa=1847695357.936717228057979078810896303844;if (kmEXMmEMJthDLFdvjhLaEYwGGwwbKa == kmEXMmEMJthDLFdvjhLaEYwGGwwbKa ) kmEXMmEMJthDLFdvjhLaEYwGGwwbKa=2127120655.620194856430706949044930414051; else kmEXMmEMJthDLFdvjhLaEYwGGwwbKa=442119534.549443798532259120057490089079;long lBZurEMdMIaGZFKhsGPuUBAZEctUjX=1423454811;if (lBZurEMdMIaGZFKhsGPuUBAZEctUjX == lBZurEMdMIaGZFKhsGPuUBAZEctUjX- 1 ) lBZurEMdMIaGZFKhsGPuUBAZEctUjX=1311043613; else lBZurEMdMIaGZFKhsGPuUBAZEctUjX=701621936;if (lBZurEMdMIaGZFKhsGPuUBAZEctUjX == lBZurEMdMIaGZFKhsGPuUBAZEctUjX- 1 ) lBZurEMdMIaGZFKhsGPuUBAZEctUjX=1931754820; else lBZurEMdMIaGZFKhsGPuUBAZEctUjX=1973648263;if (lBZurEMdMIaGZFKhsGPuUBAZEctUjX == lBZurEMdMIaGZFKhsGPuUBAZEctUjX- 1 ) lBZurEMdMIaGZFKhsGPuUBAZEctUjX=1686201777; else lBZurEMdMIaGZFKhsGPuUBAZEctUjX=510943867;if (lBZurEMdMIaGZFKhsGPuUBAZEctUjX == lBZurEMdMIaGZFKhsGPuUBAZEctUjX- 0 ) lBZurEMdMIaGZFKhsGPuUBAZEctUjX=377682318; else lBZurEMdMIaGZFKhsGPuUBAZEctUjX=1356985041;if (lBZurEMdMIaGZFKhsGPuUBAZEctUjX == lBZurEMdMIaGZFKhsGPuUBAZEctUjX- 1 ) lBZurEMdMIaGZFKhsGPuUBAZEctUjX=1589172950; else lBZurEMdMIaGZFKhsGPuUBAZEctUjX=1161444356;if (lBZurEMdMIaGZFKhsGPuUBAZEctUjX == lBZurEMdMIaGZFKhsGPuUBAZEctUjX- 0 ) lBZurEMdMIaGZFKhsGPuUBAZEctUjX=1152948366; else lBZurEMdMIaGZFKhsGPuUBAZEctUjX=1507515263;long eLkZxXWAlTfkWnkTfXEgOZTFHfSExi=368145860;if (eLkZxXWAlTfkWnkTfXEgOZTFHfSExi == eLkZxXWAlTfkWnkTfXEgOZTFHfSExi- 1 ) eLkZxXWAlTfkWnkTfXEgOZTFHfSExi=1610645082; else eLkZxXWAlTfkWnkTfXEgOZTFHfSExi=1208917894;if (eLkZxXWAlTfkWnkTfXEgOZTFHfSExi == eLkZxXWAlTfkWnkTfXEgOZTFHfSExi- 0 ) eLkZxXWAlTfkWnkTfXEgOZTFHfSExi=368662466; else eLkZxXWAlTfkWnkTfXEgOZTFHfSExi=1345921705;if (eLkZxXWAlTfkWnkTfXEgOZTFHfSExi == eLkZxXWAlTfkWnkTfXEgOZTFHfSExi- 0 ) eLkZxXWAlTfkWnkTfXEgOZTFHfSExi=314997586; else eLkZxXWAlTfkWnkTfXEgOZTFHfSExi=2018566927;if (eLkZxXWAlTfkWnkTfXEgOZTFHfSExi == eLkZxXWAlTfkWnkTfXEgOZTFHfSExi- 1 ) eLkZxXWAlTfkWnkTfXEgOZTFHfSExi=1731867014; else eLkZxXWAlTfkWnkTfXEgOZTFHfSExi=2127087481;if (eLkZxXWAlTfkWnkTfXEgOZTFHfSExi == eLkZxXWAlTfkWnkTfXEgOZTFHfSExi- 0 ) eLkZxXWAlTfkWnkTfXEgOZTFHfSExi=1477484455; else eLkZxXWAlTfkWnkTfXEgOZTFHfSExi=1769804469;if (eLkZxXWAlTfkWnkTfXEgOZTFHfSExi == eLkZxXWAlTfkWnkTfXEgOZTFHfSExi- 0 ) eLkZxXWAlTfkWnkTfXEgOZTFHfSExi=1761752200; else eLkZxXWAlTfkWnkTfXEgOZTFHfSExi=2037321815;long mOENYIkXJleQyGtQjhGOOuAvIBkBke=895465211;if (mOENYIkXJleQyGtQjhGOOuAvIBkBke == mOENYIkXJleQyGtQjhGOOuAvIBkBke- 0 ) mOENYIkXJleQyGtQjhGOOuAvIBkBke=1037569187; else mOENYIkXJleQyGtQjhGOOuAvIBkBke=805880557;if (mOENYIkXJleQyGtQjhGOOuAvIBkBke == mOENYIkXJleQyGtQjhGOOuAvIBkBke- 1 ) mOENYIkXJleQyGtQjhGOOuAvIBkBke=1978503346; else mOENYIkXJleQyGtQjhGOOuAvIBkBke=1737668507;if (mOENYIkXJleQyGtQjhGOOuAvIBkBke == mOENYIkXJleQyGtQjhGOOuAvIBkBke- 1 ) mOENYIkXJleQyGtQjhGOOuAvIBkBke=249979645; else mOENYIkXJleQyGtQjhGOOuAvIBkBke=237290561;if (mOENYIkXJleQyGtQjhGOOuAvIBkBke == mOENYIkXJleQyGtQjhGOOuAvIBkBke- 0 ) mOENYIkXJleQyGtQjhGOOuAvIBkBke=1167050601; else mOENYIkXJleQyGtQjhGOOuAvIBkBke=805070165;if (mOENYIkXJleQyGtQjhGOOuAvIBkBke == mOENYIkXJleQyGtQjhGOOuAvIBkBke- 0 ) mOENYIkXJleQyGtQjhGOOuAvIBkBke=577525371; else mOENYIkXJleQyGtQjhGOOuAvIBkBke=922651636;if (mOENYIkXJleQyGtQjhGOOuAvIBkBke == mOENYIkXJleQyGtQjhGOOuAvIBkBke- 1 ) mOENYIkXJleQyGtQjhGOOuAvIBkBke=205264461; else mOENYIkXJleQyGtQjhGOOuAvIBkBke=861410244;float wRNQGrvbXpHgMQMMbxfQMlpSnMEqnn=1313697976.345465688228935286720337668708f;if (wRNQGrvbXpHgMQMMbxfQMlpSnMEqnn - wRNQGrvbXpHgMQMMbxfQMlpSnMEqnn> 0.00000001 ) wRNQGrvbXpHgMQMMbxfQMlpSnMEqnn=177719452.921654601526956119364790711980f; else wRNQGrvbXpHgMQMMbxfQMlpSnMEqnn=1758974795.809030739269521548166954423204f;if (wRNQGrvbXpHgMQMMbxfQMlpSnMEqnn - wRNQGrvbXpHgMQMMbxfQMlpSnMEqnn> 0.00000001 ) wRNQGrvbXpHgMQMMbxfQMlpSnMEqnn=978958099.382283793648121637038483496672f; else wRNQGrvbXpHgMQMMbxfQMlpSnMEqnn=2093809604.754598734986652994489738701980f;if (wRNQGrvbXpHgMQMMbxfQMlpSnMEqnn - wRNQGrvbXpHgMQMMbxfQMlpSnMEqnn> 0.00000001 ) wRNQGrvbXpHgMQMMbxfQMlpSnMEqnn=1103181259.723199563300326383261229313799f; else wRNQGrvbXpHgMQMMbxfQMlpSnMEqnn=1368891469.886526356353129042252883061499f;if (wRNQGrvbXpHgMQMMbxfQMlpSnMEqnn - wRNQGrvbXpHgMQMMbxfQMlpSnMEqnn> 0.00000001 ) wRNQGrvbXpHgMQMMbxfQMlpSnMEqnn=1108457225.798452590003791921048255811366f; else wRNQGrvbXpHgMQMMbxfQMlpSnMEqnn=1439960333.697877577866236910224055164475f;if (wRNQGrvbXpHgMQMMbxfQMlpSnMEqnn - wRNQGrvbXpHgMQMMbxfQMlpSnMEqnn> 0.00000001 ) wRNQGrvbXpHgMQMMbxfQMlpSnMEqnn=1117238569.223451603545539303090115365371f; else wRNQGrvbXpHgMQMMbxfQMlpSnMEqnn=1952150986.321274388542788063796259612929f;if (wRNQGrvbXpHgMQMMbxfQMlpSnMEqnn - wRNQGrvbXpHgMQMMbxfQMlpSnMEqnn> 0.00000001 ) wRNQGrvbXpHgMQMMbxfQMlpSnMEqnn=496163808.643233536513556854731369617277f; else wRNQGrvbXpHgMQMMbxfQMlpSnMEqnn=2063880955.346000056664739481496830939472f;long MdyNjjcaenwsEikgQBJShqrAYFeEfC=1381029359;if (MdyNjjcaenwsEikgQBJShqrAYFeEfC == MdyNjjcaenwsEikgQBJShqrAYFeEfC- 1 ) MdyNjjcaenwsEikgQBJShqrAYFeEfC=739115771; else MdyNjjcaenwsEikgQBJShqrAYFeEfC=257483155;if (MdyNjjcaenwsEikgQBJShqrAYFeEfC == MdyNjjcaenwsEikgQBJShqrAYFeEfC- 0 ) MdyNjjcaenwsEikgQBJShqrAYFeEfC=469946076; else MdyNjjcaenwsEikgQBJShqrAYFeEfC=1105757529;if (MdyNjjcaenwsEikgQBJShqrAYFeEfC == MdyNjjcaenwsEikgQBJShqrAYFeEfC- 1 ) MdyNjjcaenwsEikgQBJShqrAYFeEfC=2007168586; else MdyNjjcaenwsEikgQBJShqrAYFeEfC=2079791045;if (MdyNjjcaenwsEikgQBJShqrAYFeEfC == MdyNjjcaenwsEikgQBJShqrAYFeEfC- 0 ) MdyNjjcaenwsEikgQBJShqrAYFeEfC=118166215; else MdyNjjcaenwsEikgQBJShqrAYFeEfC=1373354521;if (MdyNjjcaenwsEikgQBJShqrAYFeEfC == MdyNjjcaenwsEikgQBJShqrAYFeEfC- 1 ) MdyNjjcaenwsEikgQBJShqrAYFeEfC=41867293; else MdyNjjcaenwsEikgQBJShqrAYFeEfC=1711075948;if (MdyNjjcaenwsEikgQBJShqrAYFeEfC == MdyNjjcaenwsEikgQBJShqrAYFeEfC- 0 ) MdyNjjcaenwsEikgQBJShqrAYFeEfC=768396334; else MdyNjjcaenwsEikgQBJShqrAYFeEfC=1539829597;double BQuNPetZFhdWaUWjfqvPpKuTpdEEPE=267699050.306040102542714629126307900420;if (BQuNPetZFhdWaUWjfqvPpKuTpdEEPE == BQuNPetZFhdWaUWjfqvPpKuTpdEEPE ) BQuNPetZFhdWaUWjfqvPpKuTpdEEPE=237952734.572823710099779611904051510025; else BQuNPetZFhdWaUWjfqvPpKuTpdEEPE=448054224.078217190202914864830063653142;if (BQuNPetZFhdWaUWjfqvPpKuTpdEEPE == BQuNPetZFhdWaUWjfqvPpKuTpdEEPE ) BQuNPetZFhdWaUWjfqvPpKuTpdEEPE=1859602363.470602157262181315875739602666; else BQuNPetZFhdWaUWjfqvPpKuTpdEEPE=1858124094.506983342282768808344947785100;if (BQuNPetZFhdWaUWjfqvPpKuTpdEEPE == BQuNPetZFhdWaUWjfqvPpKuTpdEEPE ) BQuNPetZFhdWaUWjfqvPpKuTpdEEPE=1376669139.159186620818470407281443376931; else BQuNPetZFhdWaUWjfqvPpKuTpdEEPE=476002628.924223369228954768748903072791;if (BQuNPetZFhdWaUWjfqvPpKuTpdEEPE == BQuNPetZFhdWaUWjfqvPpKuTpdEEPE ) BQuNPetZFhdWaUWjfqvPpKuTpdEEPE=1939767599.070009425814906369062939303225; else BQuNPetZFhdWaUWjfqvPpKuTpdEEPE=1816161365.168825995544303500882384025170;if (BQuNPetZFhdWaUWjfqvPpKuTpdEEPE == BQuNPetZFhdWaUWjfqvPpKuTpdEEPE ) BQuNPetZFhdWaUWjfqvPpKuTpdEEPE=1844906979.731176718147452973790344398491; else BQuNPetZFhdWaUWjfqvPpKuTpdEEPE=538543013.082682324880374693215359059798;if (BQuNPetZFhdWaUWjfqvPpKuTpdEEPE == BQuNPetZFhdWaUWjfqvPpKuTpdEEPE ) BQuNPetZFhdWaUWjfqvPpKuTpdEEPE=620429938.833817598165991118180538683464; else BQuNPetZFhdWaUWjfqvPpKuTpdEEPE=445312306.374606756811616156805171930634;float FRTnyJPycTeIOkwvJLbQifgwKzLkhc=89681221.119423547539087148134027223828f;if (FRTnyJPycTeIOkwvJLbQifgwKzLkhc - FRTnyJPycTeIOkwvJLbQifgwKzLkhc> 0.00000001 ) FRTnyJPycTeIOkwvJLbQifgwKzLkhc=1035281837.010054203067137209677407410331f; else FRTnyJPycTeIOkwvJLbQifgwKzLkhc=971718749.072952014395528809002525162474f;if (FRTnyJPycTeIOkwvJLbQifgwKzLkhc - FRTnyJPycTeIOkwvJLbQifgwKzLkhc> 0.00000001 ) FRTnyJPycTeIOkwvJLbQifgwKzLkhc=122294473.407319528530539398972134956790f; else FRTnyJPycTeIOkwvJLbQifgwKzLkhc=1483307442.175090822938167310787450348456f;if (FRTnyJPycTeIOkwvJLbQifgwKzLkhc - FRTnyJPycTeIOkwvJLbQifgwKzLkhc> 0.00000001 ) FRTnyJPycTeIOkwvJLbQifgwKzLkhc=739839329.484947664068704417765611382586f; else FRTnyJPycTeIOkwvJLbQifgwKzLkhc=426094206.258666532176683075603179288377f;if (FRTnyJPycTeIOkwvJLbQifgwKzLkhc - FRTnyJPycTeIOkwvJLbQifgwKzLkhc> 0.00000001 ) FRTnyJPycTeIOkwvJLbQifgwKzLkhc=1454013037.528982015998670914831808328949f; else FRTnyJPycTeIOkwvJLbQifgwKzLkhc=245412054.937566880216171661037454782167f;if (FRTnyJPycTeIOkwvJLbQifgwKzLkhc - FRTnyJPycTeIOkwvJLbQifgwKzLkhc> 0.00000001 ) FRTnyJPycTeIOkwvJLbQifgwKzLkhc=1640808580.418717212424473583887801258498f; else FRTnyJPycTeIOkwvJLbQifgwKzLkhc=1753087225.483490957545962419121038172757f;if (FRTnyJPycTeIOkwvJLbQifgwKzLkhc - FRTnyJPycTeIOkwvJLbQifgwKzLkhc> 0.00000001 ) FRTnyJPycTeIOkwvJLbQifgwKzLkhc=2051942311.092415219463635294094937845542f; else FRTnyJPycTeIOkwvJLbQifgwKzLkhc=1539588958.830066466730187841498877317577f;double hkgdRRfnwpRrReUnbFwAUJXVBfPmWu=246781799.138893369950251328567131677919;if (hkgdRRfnwpRrReUnbFwAUJXVBfPmWu == hkgdRRfnwpRrReUnbFwAUJXVBfPmWu ) hkgdRRfnwpRrReUnbFwAUJXVBfPmWu=1565854102.776339778245298632432965900823; else hkgdRRfnwpRrReUnbFwAUJXVBfPmWu=228464758.645028493266043395314559782143;if (hkgdRRfnwpRrReUnbFwAUJXVBfPmWu == hkgdRRfnwpRrReUnbFwAUJXVBfPmWu ) hkgdRRfnwpRrReUnbFwAUJXVBfPmWu=1825928221.905660037124037703361652910644; else hkgdRRfnwpRrReUnbFwAUJXVBfPmWu=448621090.746037158337839163209999245475;if (hkgdRRfnwpRrReUnbFwAUJXVBfPmWu == hkgdRRfnwpRrReUnbFwAUJXVBfPmWu ) hkgdRRfnwpRrReUnbFwAUJXVBfPmWu=775725434.149459900175604583396754586890; else hkgdRRfnwpRrReUnbFwAUJXVBfPmWu=176729352.288339859126414549586319040697;if (hkgdRRfnwpRrReUnbFwAUJXVBfPmWu == hkgdRRfnwpRrReUnbFwAUJXVBfPmWu ) hkgdRRfnwpRrReUnbFwAUJXVBfPmWu=4360220.422009838573537044014731598939; else hkgdRRfnwpRrReUnbFwAUJXVBfPmWu=263113143.037281542581212398311668370151;if (hkgdRRfnwpRrReUnbFwAUJXVBfPmWu == hkgdRRfnwpRrReUnbFwAUJXVBfPmWu ) hkgdRRfnwpRrReUnbFwAUJXVBfPmWu=465199203.290964736750354462030073947901; else hkgdRRfnwpRrReUnbFwAUJXVBfPmWu=801268358.150149236877366141502823122802;if (hkgdRRfnwpRrReUnbFwAUJXVBfPmWu == hkgdRRfnwpRrReUnbFwAUJXVBfPmWu ) hkgdRRfnwpRrReUnbFwAUJXVBfPmWu=1408334197.784661132523726482891750044702; else hkgdRRfnwpRrReUnbFwAUJXVBfPmWu=1468625573.744419480502908010785102213463;double VNAppuSFkDBHNqtmbJzMUxLjeVJHBf=938697647.174067402093239479364588764958;if (VNAppuSFkDBHNqtmbJzMUxLjeVJHBf == VNAppuSFkDBHNqtmbJzMUxLjeVJHBf ) VNAppuSFkDBHNqtmbJzMUxLjeVJHBf=1802681097.855601553029629183779942070741; else VNAppuSFkDBHNqtmbJzMUxLjeVJHBf=678715113.455434873802645597969239902474;if (VNAppuSFkDBHNqtmbJzMUxLjeVJHBf == VNAppuSFkDBHNqtmbJzMUxLjeVJHBf ) VNAppuSFkDBHNqtmbJzMUxLjeVJHBf=504547760.292377539632098830759489222763; else VNAppuSFkDBHNqtmbJzMUxLjeVJHBf=841583717.250071148817833825128282591315;if (VNAppuSFkDBHNqtmbJzMUxLjeVJHBf == VNAppuSFkDBHNqtmbJzMUxLjeVJHBf ) VNAppuSFkDBHNqtmbJzMUxLjeVJHBf=748162297.697749202779395365650864618958; else VNAppuSFkDBHNqtmbJzMUxLjeVJHBf=1050473089.184433020563356909833786583314;if (VNAppuSFkDBHNqtmbJzMUxLjeVJHBf == VNAppuSFkDBHNqtmbJzMUxLjeVJHBf ) VNAppuSFkDBHNqtmbJzMUxLjeVJHBf=1675343368.246579506235629852972912551806; else VNAppuSFkDBHNqtmbJzMUxLjeVJHBf=1959342175.071633788740461816873254333413;if (VNAppuSFkDBHNqtmbJzMUxLjeVJHBf == VNAppuSFkDBHNqtmbJzMUxLjeVJHBf ) VNAppuSFkDBHNqtmbJzMUxLjeVJHBf=1141524834.981127509728433265274513393741; else VNAppuSFkDBHNqtmbJzMUxLjeVJHBf=1614190309.038687034442815692227834958884;if (VNAppuSFkDBHNqtmbJzMUxLjeVJHBf == VNAppuSFkDBHNqtmbJzMUxLjeVJHBf ) VNAppuSFkDBHNqtmbJzMUxLjeVJHBf=217315514.487879085723283370971163421574; else VNAppuSFkDBHNqtmbJzMUxLjeVJHBf=1771065561.035107605771750575178802442555;float bCGluoEnjkmcHAxSELvbfQBgGRSkhV=128795704.105687283168028004117093278947f;if (bCGluoEnjkmcHAxSELvbfQBgGRSkhV - bCGluoEnjkmcHAxSELvbfQBgGRSkhV> 0.00000001 ) bCGluoEnjkmcHAxSELvbfQBgGRSkhV=911885146.206905310523387360015017812341f; else bCGluoEnjkmcHAxSELvbfQBgGRSkhV=373648139.944796223470479571702804230029f;if (bCGluoEnjkmcHAxSELvbfQBgGRSkhV - bCGluoEnjkmcHAxSELvbfQBgGRSkhV> 0.00000001 ) bCGluoEnjkmcHAxSELvbfQBgGRSkhV=1026941851.318068979115864565834164925157f; else bCGluoEnjkmcHAxSELvbfQBgGRSkhV=1660145642.731427351261379804087539625414f;if (bCGluoEnjkmcHAxSELvbfQBgGRSkhV - bCGluoEnjkmcHAxSELvbfQBgGRSkhV> 0.00000001 ) bCGluoEnjkmcHAxSELvbfQBgGRSkhV=1572208719.482349455832477390522128120290f; else bCGluoEnjkmcHAxSELvbfQBgGRSkhV=1607257408.294788004146925527233699237532f;if (bCGluoEnjkmcHAxSELvbfQBgGRSkhV - bCGluoEnjkmcHAxSELvbfQBgGRSkhV> 0.00000001 ) bCGluoEnjkmcHAxSELvbfQBgGRSkhV=1461439434.936574879940659720361416861656f; else bCGluoEnjkmcHAxSELvbfQBgGRSkhV=858552485.268232056175830405061827253181f;if (bCGluoEnjkmcHAxSELvbfQBgGRSkhV - bCGluoEnjkmcHAxSELvbfQBgGRSkhV> 0.00000001 ) bCGluoEnjkmcHAxSELvbfQBgGRSkhV=967358529.432389252878429678747660544218f; else bCGluoEnjkmcHAxSELvbfQBgGRSkhV=2006664977.907685455136330350869545588395f;if (bCGluoEnjkmcHAxSELvbfQBgGRSkhV - bCGluoEnjkmcHAxSELvbfQBgGRSkhV> 0.00000001 ) bCGluoEnjkmcHAxSELvbfQBgGRSkhV=1018108239.514597295532362144120446057210f; else bCGluoEnjkmcHAxSELvbfQBgGRSkhV=2086054618.717162294326634838445537628955f;int yoWpHXMzxuYzpBTrCJKJstkdNtzZZY=1967954671;if (yoWpHXMzxuYzpBTrCJKJstkdNtzZZY == yoWpHXMzxuYzpBTrCJKJstkdNtzZZY- 1 ) yoWpHXMzxuYzpBTrCJKJstkdNtzZZY=120859457; else yoWpHXMzxuYzpBTrCJKJstkdNtzZZY=1113244664;if (yoWpHXMzxuYzpBTrCJKJstkdNtzZZY == yoWpHXMzxuYzpBTrCJKJstkdNtzZZY- 0 ) yoWpHXMzxuYzpBTrCJKJstkdNtzZZY=1884391508; else yoWpHXMzxuYzpBTrCJKJstkdNtzZZY=2069263669;if (yoWpHXMzxuYzpBTrCJKJstkdNtzZZY == yoWpHXMzxuYzpBTrCJKJstkdNtzZZY- 0 ) yoWpHXMzxuYzpBTrCJKJstkdNtzZZY=1211563515; else yoWpHXMzxuYzpBTrCJKJstkdNtzZZY=932557797;if (yoWpHXMzxuYzpBTrCJKJstkdNtzZZY == yoWpHXMzxuYzpBTrCJKJstkdNtzZZY- 1 ) yoWpHXMzxuYzpBTrCJKJstkdNtzZZY=1646103149; else yoWpHXMzxuYzpBTrCJKJstkdNtzZZY=515381013;if (yoWpHXMzxuYzpBTrCJKJstkdNtzZZY == yoWpHXMzxuYzpBTrCJKJstkdNtzZZY- 0 ) yoWpHXMzxuYzpBTrCJKJstkdNtzZZY=700605037; else yoWpHXMzxuYzpBTrCJKJstkdNtzZZY=2014069693;if (yoWpHXMzxuYzpBTrCJKJstkdNtzZZY == yoWpHXMzxuYzpBTrCJKJstkdNtzZZY- 1 ) yoWpHXMzxuYzpBTrCJKJstkdNtzZZY=1454579903; else yoWpHXMzxuYzpBTrCJKJstkdNtzZZY=1309018772;long ScFwIPRLEjeEnjmJhpFVhMgSbAXGrd=65020562;if (ScFwIPRLEjeEnjmJhpFVhMgSbAXGrd == ScFwIPRLEjeEnjmJhpFVhMgSbAXGrd- 0 ) ScFwIPRLEjeEnjmJhpFVhMgSbAXGrd=1733085775; else ScFwIPRLEjeEnjmJhpFVhMgSbAXGrd=1279538493;if (ScFwIPRLEjeEnjmJhpFVhMgSbAXGrd == ScFwIPRLEjeEnjmJhpFVhMgSbAXGrd- 1 ) ScFwIPRLEjeEnjmJhpFVhMgSbAXGrd=556740427; else ScFwIPRLEjeEnjmJhpFVhMgSbAXGrd=1085712968;if (ScFwIPRLEjeEnjmJhpFVhMgSbAXGrd == ScFwIPRLEjeEnjmJhpFVhMgSbAXGrd- 1 ) ScFwIPRLEjeEnjmJhpFVhMgSbAXGrd=793395705; else ScFwIPRLEjeEnjmJhpFVhMgSbAXGrd=2119565773;if (ScFwIPRLEjeEnjmJhpFVhMgSbAXGrd == ScFwIPRLEjeEnjmJhpFVhMgSbAXGrd- 0 ) ScFwIPRLEjeEnjmJhpFVhMgSbAXGrd=1513972575; else ScFwIPRLEjeEnjmJhpFVhMgSbAXGrd=195423689;if (ScFwIPRLEjeEnjmJhpFVhMgSbAXGrd == ScFwIPRLEjeEnjmJhpFVhMgSbAXGrd- 0 ) ScFwIPRLEjeEnjmJhpFVhMgSbAXGrd=1364759326; else ScFwIPRLEjeEnjmJhpFVhMgSbAXGrd=1793478922;if (ScFwIPRLEjeEnjmJhpFVhMgSbAXGrd == ScFwIPRLEjeEnjmJhpFVhMgSbAXGrd- 0 ) ScFwIPRLEjeEnjmJhpFVhMgSbAXGrd=2021708835; else ScFwIPRLEjeEnjmJhpFVhMgSbAXGrd=888980967;long SwwjvBdDUPQXtazZZIhFXmnosZxTYj=1156137370;if (SwwjvBdDUPQXtazZZIhFXmnosZxTYj == SwwjvBdDUPQXtazZZIhFXmnosZxTYj- 1 ) SwwjvBdDUPQXtazZZIhFXmnosZxTYj=680655715; else SwwjvBdDUPQXtazZZIhFXmnosZxTYj=396844490;if (SwwjvBdDUPQXtazZZIhFXmnosZxTYj == SwwjvBdDUPQXtazZZIhFXmnosZxTYj- 0 ) SwwjvBdDUPQXtazZZIhFXmnosZxTYj=1650206104; else SwwjvBdDUPQXtazZZIhFXmnosZxTYj=1605782947;if (SwwjvBdDUPQXtazZZIhFXmnosZxTYj == SwwjvBdDUPQXtazZZIhFXmnosZxTYj- 1 ) SwwjvBdDUPQXtazZZIhFXmnosZxTYj=663270780; else SwwjvBdDUPQXtazZZIhFXmnosZxTYj=1293886208;if (SwwjvBdDUPQXtazZZIhFXmnosZxTYj == SwwjvBdDUPQXtazZZIhFXmnosZxTYj- 0 ) SwwjvBdDUPQXtazZZIhFXmnosZxTYj=1064715836; else SwwjvBdDUPQXtazZZIhFXmnosZxTYj=2126810996;if (SwwjvBdDUPQXtazZZIhFXmnosZxTYj == SwwjvBdDUPQXtazZZIhFXmnosZxTYj- 0 ) SwwjvBdDUPQXtazZZIhFXmnosZxTYj=875028637; else SwwjvBdDUPQXtazZZIhFXmnosZxTYj=511560838;if (SwwjvBdDUPQXtazZZIhFXmnosZxTYj == SwwjvBdDUPQXtazZZIhFXmnosZxTYj- 0 ) SwwjvBdDUPQXtazZZIhFXmnosZxTYj=959540605; else SwwjvBdDUPQXtazZZIhFXmnosZxTYj=1705622673;double PKHhKgcQOMJQQLGwmrcXpecHywvzGZ=586580223.167394205601676821207756456254;if (PKHhKgcQOMJQQLGwmrcXpecHywvzGZ == PKHhKgcQOMJQQLGwmrcXpecHywvzGZ ) PKHhKgcQOMJQQLGwmrcXpecHywvzGZ=167612151.711091927410508114904448451104; else PKHhKgcQOMJQQLGwmrcXpecHywvzGZ=953087677.805209069600580736835346038474;if (PKHhKgcQOMJQQLGwmrcXpecHywvzGZ == PKHhKgcQOMJQQLGwmrcXpecHywvzGZ ) PKHhKgcQOMJQQLGwmrcXpecHywvzGZ=352968843.083361540025630507972637066028; else PKHhKgcQOMJQQLGwmrcXpecHywvzGZ=2033188810.270572822929692182664250355631;if (PKHhKgcQOMJQQLGwmrcXpecHywvzGZ == PKHhKgcQOMJQQLGwmrcXpecHywvzGZ ) PKHhKgcQOMJQQLGwmrcXpecHywvzGZ=1576130684.562385627878515415609547512032; else PKHhKgcQOMJQQLGwmrcXpecHywvzGZ=1995305273.839247731757699675595104312698;if (PKHhKgcQOMJQQLGwmrcXpecHywvzGZ == PKHhKgcQOMJQQLGwmrcXpecHywvzGZ ) PKHhKgcQOMJQQLGwmrcXpecHywvzGZ=1174558966.460515947978405206894754873693; else PKHhKgcQOMJQQLGwmrcXpecHywvzGZ=148777331.525449818574872781012780472684;if (PKHhKgcQOMJQQLGwmrcXpecHywvzGZ == PKHhKgcQOMJQQLGwmrcXpecHywvzGZ ) PKHhKgcQOMJQQLGwmrcXpecHywvzGZ=1746755736.245556407659843199220379284508; else PKHhKgcQOMJQQLGwmrcXpecHywvzGZ=1688967819.824433942738546495748457598738;if (PKHhKgcQOMJQQLGwmrcXpecHywvzGZ == PKHhKgcQOMJQQLGwmrcXpecHywvzGZ ) PKHhKgcQOMJQQLGwmrcXpecHywvzGZ=704504343.301994551026759293380986058804; else PKHhKgcQOMJQQLGwmrcXpecHywvzGZ=1565568240.531554866395573370831656842504;int kaiZwVODZWRjyvaVESmoryhRypdgCL=1111316287;if (kaiZwVODZWRjyvaVESmoryhRypdgCL == kaiZwVODZWRjyvaVESmoryhRypdgCL- 1 ) kaiZwVODZWRjyvaVESmoryhRypdgCL=1219971174; else kaiZwVODZWRjyvaVESmoryhRypdgCL=298442533;if (kaiZwVODZWRjyvaVESmoryhRypdgCL == kaiZwVODZWRjyvaVESmoryhRypdgCL- 1 ) kaiZwVODZWRjyvaVESmoryhRypdgCL=628363057; else kaiZwVODZWRjyvaVESmoryhRypdgCL=1710052995;if (kaiZwVODZWRjyvaVESmoryhRypdgCL == kaiZwVODZWRjyvaVESmoryhRypdgCL- 0 ) kaiZwVODZWRjyvaVESmoryhRypdgCL=878055849; else kaiZwVODZWRjyvaVESmoryhRypdgCL=233269879;if (kaiZwVODZWRjyvaVESmoryhRypdgCL == kaiZwVODZWRjyvaVESmoryhRypdgCL- 1 ) kaiZwVODZWRjyvaVESmoryhRypdgCL=1549233429; else kaiZwVODZWRjyvaVESmoryhRypdgCL=439199567;if (kaiZwVODZWRjyvaVESmoryhRypdgCL == kaiZwVODZWRjyvaVESmoryhRypdgCL- 1 ) kaiZwVODZWRjyvaVESmoryhRypdgCL=214822657; else kaiZwVODZWRjyvaVESmoryhRypdgCL=577413078;if (kaiZwVODZWRjyvaVESmoryhRypdgCL == kaiZwVODZWRjyvaVESmoryhRypdgCL- 1 ) kaiZwVODZWRjyvaVESmoryhRypdgCL=210684331; else kaiZwVODZWRjyvaVESmoryhRypdgCL=281834867;int vqfnSfmnEUtgddfCdYyMgpqPJINWHh=1212338664;if (vqfnSfmnEUtgddfCdYyMgpqPJINWHh == vqfnSfmnEUtgddfCdYyMgpqPJINWHh- 1 ) vqfnSfmnEUtgddfCdYyMgpqPJINWHh=894340275; else vqfnSfmnEUtgddfCdYyMgpqPJINWHh=1435128612;if (vqfnSfmnEUtgddfCdYyMgpqPJINWHh == vqfnSfmnEUtgddfCdYyMgpqPJINWHh- 0 ) vqfnSfmnEUtgddfCdYyMgpqPJINWHh=721368816; else vqfnSfmnEUtgddfCdYyMgpqPJINWHh=1226760820;if (vqfnSfmnEUtgddfCdYyMgpqPJINWHh == vqfnSfmnEUtgddfCdYyMgpqPJINWHh- 1 ) vqfnSfmnEUtgddfCdYyMgpqPJINWHh=1383803554; else vqfnSfmnEUtgddfCdYyMgpqPJINWHh=908393356;if (vqfnSfmnEUtgddfCdYyMgpqPJINWHh == vqfnSfmnEUtgddfCdYyMgpqPJINWHh- 0 ) vqfnSfmnEUtgddfCdYyMgpqPJINWHh=1727490333; else vqfnSfmnEUtgddfCdYyMgpqPJINWHh=832834089;if (vqfnSfmnEUtgddfCdYyMgpqPJINWHh == vqfnSfmnEUtgddfCdYyMgpqPJINWHh- 1 ) vqfnSfmnEUtgddfCdYyMgpqPJINWHh=1369769171; else vqfnSfmnEUtgddfCdYyMgpqPJINWHh=1501428773;if (vqfnSfmnEUtgddfCdYyMgpqPJINWHh == vqfnSfmnEUtgddfCdYyMgpqPJINWHh- 0 ) vqfnSfmnEUtgddfCdYyMgpqPJINWHh=2006510858; else vqfnSfmnEUtgddfCdYyMgpqPJINWHh=1121786608;long KfhsuRfUtKfWePKhMAESrgcFJAfsoq=117906455;if (KfhsuRfUtKfWePKhMAESrgcFJAfsoq == KfhsuRfUtKfWePKhMAESrgcFJAfsoq- 0 ) KfhsuRfUtKfWePKhMAESrgcFJAfsoq=28544675; else KfhsuRfUtKfWePKhMAESrgcFJAfsoq=1995707965;if (KfhsuRfUtKfWePKhMAESrgcFJAfsoq == KfhsuRfUtKfWePKhMAESrgcFJAfsoq- 0 ) KfhsuRfUtKfWePKhMAESrgcFJAfsoq=485651499; else KfhsuRfUtKfWePKhMAESrgcFJAfsoq=2001718619;if (KfhsuRfUtKfWePKhMAESrgcFJAfsoq == KfhsuRfUtKfWePKhMAESrgcFJAfsoq- 1 ) KfhsuRfUtKfWePKhMAESrgcFJAfsoq=1671541252; else KfhsuRfUtKfWePKhMAESrgcFJAfsoq=1336569299;if (KfhsuRfUtKfWePKhMAESrgcFJAfsoq == KfhsuRfUtKfWePKhMAESrgcFJAfsoq- 1 ) KfhsuRfUtKfWePKhMAESrgcFJAfsoq=1856008021; else KfhsuRfUtKfWePKhMAESrgcFJAfsoq=2081825919;if (KfhsuRfUtKfWePKhMAESrgcFJAfsoq == KfhsuRfUtKfWePKhMAESrgcFJAfsoq- 0 ) KfhsuRfUtKfWePKhMAESrgcFJAfsoq=1551337895; else KfhsuRfUtKfWePKhMAESrgcFJAfsoq=2061252923;if (KfhsuRfUtKfWePKhMAESrgcFJAfsoq == KfhsuRfUtKfWePKhMAESrgcFJAfsoq- 0 ) KfhsuRfUtKfWePKhMAESrgcFJAfsoq=1818366153; else KfhsuRfUtKfWePKhMAESrgcFJAfsoq=998717606;float MYlefINhGYwQgqNOfhuanAxsOnZxZF=1938405834.929214786282109582275989629588f;if (MYlefINhGYwQgqNOfhuanAxsOnZxZF - MYlefINhGYwQgqNOfhuanAxsOnZxZF> 0.00000001 ) MYlefINhGYwQgqNOfhuanAxsOnZxZF=1646221635.633763571726976970832821015079f; else MYlefINhGYwQgqNOfhuanAxsOnZxZF=1639649326.531876044601165002376986691286f;if (MYlefINhGYwQgqNOfhuanAxsOnZxZF - MYlefINhGYwQgqNOfhuanAxsOnZxZF> 0.00000001 ) MYlefINhGYwQgqNOfhuanAxsOnZxZF=671227965.961230611103733277611470466528f; else MYlefINhGYwQgqNOfhuanAxsOnZxZF=1770687644.587875104650238336353568669187f;if (MYlefINhGYwQgqNOfhuanAxsOnZxZF - MYlefINhGYwQgqNOfhuanAxsOnZxZF> 0.00000001 ) MYlefINhGYwQgqNOfhuanAxsOnZxZF=35753366.419363796828416135314138612932f; else MYlefINhGYwQgqNOfhuanAxsOnZxZF=1403738990.712824197674063616254488348275f;if (MYlefINhGYwQgqNOfhuanAxsOnZxZF - MYlefINhGYwQgqNOfhuanAxsOnZxZF> 0.00000001 ) MYlefINhGYwQgqNOfhuanAxsOnZxZF=1874996380.187522679314974118510678933459f; else MYlefINhGYwQgqNOfhuanAxsOnZxZF=311577818.792250764205804061433177561069f;if (MYlefINhGYwQgqNOfhuanAxsOnZxZF - MYlefINhGYwQgqNOfhuanAxsOnZxZF> 0.00000001 ) MYlefINhGYwQgqNOfhuanAxsOnZxZF=718435847.022062273546442688498213662570f; else MYlefINhGYwQgqNOfhuanAxsOnZxZF=1943012615.980638484380909884608467182051f;if (MYlefINhGYwQgqNOfhuanAxsOnZxZF - MYlefINhGYwQgqNOfhuanAxsOnZxZF> 0.00000001 ) MYlefINhGYwQgqNOfhuanAxsOnZxZF=340283824.170376681431422932489908252455f; else MYlefINhGYwQgqNOfhuanAxsOnZxZF=1524110723.644421063430427815635168338213f;double JOeqskAxDrjsCZtZmDXUmpNzYseFzP=715675061.043062289879564780404595499147;if (JOeqskAxDrjsCZtZmDXUmpNzYseFzP == JOeqskAxDrjsCZtZmDXUmpNzYseFzP ) JOeqskAxDrjsCZtZmDXUmpNzYseFzP=1656418338.119456283015186694185325855968; else JOeqskAxDrjsCZtZmDXUmpNzYseFzP=92777876.314276012950750052091617299570;if (JOeqskAxDrjsCZtZmDXUmpNzYseFzP == JOeqskAxDrjsCZtZmDXUmpNzYseFzP ) JOeqskAxDrjsCZtZmDXUmpNzYseFzP=253290192.242670192068323568581307880708; else JOeqskAxDrjsCZtZmDXUmpNzYseFzP=1311800850.864282919006104349583200994599;if (JOeqskAxDrjsCZtZmDXUmpNzYseFzP == JOeqskAxDrjsCZtZmDXUmpNzYseFzP ) JOeqskAxDrjsCZtZmDXUmpNzYseFzP=1470182050.150420415276959630455133567549; else JOeqskAxDrjsCZtZmDXUmpNzYseFzP=188511212.942005778493541835247424502707;if (JOeqskAxDrjsCZtZmDXUmpNzYseFzP == JOeqskAxDrjsCZtZmDXUmpNzYseFzP ) JOeqskAxDrjsCZtZmDXUmpNzYseFzP=1688281059.408734495496307270383240353961; else JOeqskAxDrjsCZtZmDXUmpNzYseFzP=1705254623.308219681855085122733353436849;if (JOeqskAxDrjsCZtZmDXUmpNzYseFzP == JOeqskAxDrjsCZtZmDXUmpNzYseFzP ) JOeqskAxDrjsCZtZmDXUmpNzYseFzP=1726604238.263519910879064927624124816599; else JOeqskAxDrjsCZtZmDXUmpNzYseFzP=787026544.394369267621306540051022514071;if (JOeqskAxDrjsCZtZmDXUmpNzYseFzP == JOeqskAxDrjsCZtZmDXUmpNzYseFzP ) JOeqskAxDrjsCZtZmDXUmpNzYseFzP=1822906572.442168570587001383848765847386; else JOeqskAxDrjsCZtZmDXUmpNzYseFzP=1470034278.639739024907068413224224379832;float OwVuZqLkKRfryVEjCjVMZBdKSFaUfA=477045072.703665314512240767483039526610f;if (OwVuZqLkKRfryVEjCjVMZBdKSFaUfA - OwVuZqLkKRfryVEjCjVMZBdKSFaUfA> 0.00000001 ) OwVuZqLkKRfryVEjCjVMZBdKSFaUfA=365192963.239600632195832992508488488524f; else OwVuZqLkKRfryVEjCjVMZBdKSFaUfA=68105024.071448475023640920764421386222f;if (OwVuZqLkKRfryVEjCjVMZBdKSFaUfA - OwVuZqLkKRfryVEjCjVMZBdKSFaUfA> 0.00000001 ) OwVuZqLkKRfryVEjCjVMZBdKSFaUfA=1623048620.222812540562869594867986076027f; else OwVuZqLkKRfryVEjCjVMZBdKSFaUfA=1097216186.325381284958030267290247634685f;if (OwVuZqLkKRfryVEjCjVMZBdKSFaUfA - OwVuZqLkKRfryVEjCjVMZBdKSFaUfA> 0.00000001 ) OwVuZqLkKRfryVEjCjVMZBdKSFaUfA=975403278.653434868931102905691084719975f; else OwVuZqLkKRfryVEjCjVMZBdKSFaUfA=1445274390.770030670651946925584900478073f;if (OwVuZqLkKRfryVEjCjVMZBdKSFaUfA - OwVuZqLkKRfryVEjCjVMZBdKSFaUfA> 0.00000001 ) OwVuZqLkKRfryVEjCjVMZBdKSFaUfA=82153089.699337118244605114899540187166f; else OwVuZqLkKRfryVEjCjVMZBdKSFaUfA=772289332.370816272981744140647429426025f;if (OwVuZqLkKRfryVEjCjVMZBdKSFaUfA - OwVuZqLkKRfryVEjCjVMZBdKSFaUfA> 0.00000001 ) OwVuZqLkKRfryVEjCjVMZBdKSFaUfA=120711585.116297480995301881614762006249f; else OwVuZqLkKRfryVEjCjVMZBdKSFaUfA=174073681.135798665887175312875542245430f;if (OwVuZqLkKRfryVEjCjVMZBdKSFaUfA - OwVuZqLkKRfryVEjCjVMZBdKSFaUfA> 0.00000001 ) OwVuZqLkKRfryVEjCjVMZBdKSFaUfA=1283282772.224197061333694879779458569366f; else OwVuZqLkKRfryVEjCjVMZBdKSFaUfA=1876012284.423666309935254205957777188663f;long jBcxEABPmVmztUVNvVQfTXGtkXfXnp=1123891074;if (jBcxEABPmVmztUVNvVQfTXGtkXfXnp == jBcxEABPmVmztUVNvVQfTXGtkXfXnp- 0 ) jBcxEABPmVmztUVNvVQfTXGtkXfXnp=63951574; else jBcxEABPmVmztUVNvVQfTXGtkXfXnp=1102691092;if (jBcxEABPmVmztUVNvVQfTXGtkXfXnp == jBcxEABPmVmztUVNvVQfTXGtkXfXnp- 0 ) jBcxEABPmVmztUVNvVQfTXGtkXfXnp=1489782915; else jBcxEABPmVmztUVNvVQfTXGtkXfXnp=1691843066;if (jBcxEABPmVmztUVNvVQfTXGtkXfXnp == jBcxEABPmVmztUVNvVQfTXGtkXfXnp- 1 ) jBcxEABPmVmztUVNvVQfTXGtkXfXnp=639433907; else jBcxEABPmVmztUVNvVQfTXGtkXfXnp=1971918607;if (jBcxEABPmVmztUVNvVQfTXGtkXfXnp == jBcxEABPmVmztUVNvVQfTXGtkXfXnp- 0 ) jBcxEABPmVmztUVNvVQfTXGtkXfXnp=468223784; else jBcxEABPmVmztUVNvVQfTXGtkXfXnp=1457710423;if (jBcxEABPmVmztUVNvVQfTXGtkXfXnp == jBcxEABPmVmztUVNvVQfTXGtkXfXnp- 0 ) jBcxEABPmVmztUVNvVQfTXGtkXfXnp=2098851914; else jBcxEABPmVmztUVNvVQfTXGtkXfXnp=1113456989;if (jBcxEABPmVmztUVNvVQfTXGtkXfXnp == jBcxEABPmVmztUVNvVQfTXGtkXfXnp- 1 ) jBcxEABPmVmztUVNvVQfTXGtkXfXnp=44083406; else jBcxEABPmVmztUVNvVQfTXGtkXfXnp=1396210469;float oBiQlvTjPuSpCbXOiGpVlJvVODnfTr=2137146767.384384418240806170289376500662f;if (oBiQlvTjPuSpCbXOiGpVlJvVODnfTr - oBiQlvTjPuSpCbXOiGpVlJvVODnfTr> 0.00000001 ) oBiQlvTjPuSpCbXOiGpVlJvVODnfTr=747315821.760108471956390263112974412888f; else oBiQlvTjPuSpCbXOiGpVlJvVODnfTr=2010722399.486658070970034308745865090770f;if (oBiQlvTjPuSpCbXOiGpVlJvVODnfTr - oBiQlvTjPuSpCbXOiGpVlJvVODnfTr> 0.00000001 ) oBiQlvTjPuSpCbXOiGpVlJvVODnfTr=1158917760.670367184239324395021525463167f; else oBiQlvTjPuSpCbXOiGpVlJvVODnfTr=306137644.931141177823451654180676915600f;if (oBiQlvTjPuSpCbXOiGpVlJvVODnfTr - oBiQlvTjPuSpCbXOiGpVlJvVODnfTr> 0.00000001 ) oBiQlvTjPuSpCbXOiGpVlJvVODnfTr=951881848.402740928432830918481008846534f; else oBiQlvTjPuSpCbXOiGpVlJvVODnfTr=900085163.668743172183686885600822951401f;if (oBiQlvTjPuSpCbXOiGpVlJvVODnfTr - oBiQlvTjPuSpCbXOiGpVlJvVODnfTr> 0.00000001 ) oBiQlvTjPuSpCbXOiGpVlJvVODnfTr=1887755430.440588519894631942316203494690f; else oBiQlvTjPuSpCbXOiGpVlJvVODnfTr=211143948.278184421938682629211920525820f;if (oBiQlvTjPuSpCbXOiGpVlJvVODnfTr - oBiQlvTjPuSpCbXOiGpVlJvVODnfTr> 0.00000001 ) oBiQlvTjPuSpCbXOiGpVlJvVODnfTr=10624787.403063766853248872287548045966f; else oBiQlvTjPuSpCbXOiGpVlJvVODnfTr=1628484507.971164673007457978286474496603f;if (oBiQlvTjPuSpCbXOiGpVlJvVODnfTr - oBiQlvTjPuSpCbXOiGpVlJvVODnfTr> 0.00000001 ) oBiQlvTjPuSpCbXOiGpVlJvVODnfTr=1191089265.880199319431928020118879050787f; else oBiQlvTjPuSpCbXOiGpVlJvVODnfTr=1296108369.917132836876986555747650501115f;long fQKeLWtxeKtYRWItwRZzbAXAMejaLG=1394706940;if (fQKeLWtxeKtYRWItwRZzbAXAMejaLG == fQKeLWtxeKtYRWItwRZzbAXAMejaLG- 0 ) fQKeLWtxeKtYRWItwRZzbAXAMejaLG=1226926940; else fQKeLWtxeKtYRWItwRZzbAXAMejaLG=699015498;if (fQKeLWtxeKtYRWItwRZzbAXAMejaLG == fQKeLWtxeKtYRWItwRZzbAXAMejaLG- 0 ) fQKeLWtxeKtYRWItwRZzbAXAMejaLG=1406082704; else fQKeLWtxeKtYRWItwRZzbAXAMejaLG=134819758;if (fQKeLWtxeKtYRWItwRZzbAXAMejaLG == fQKeLWtxeKtYRWItwRZzbAXAMejaLG- 0 ) fQKeLWtxeKtYRWItwRZzbAXAMejaLG=1386473459; else fQKeLWtxeKtYRWItwRZzbAXAMejaLG=1651337375;if (fQKeLWtxeKtYRWItwRZzbAXAMejaLG == fQKeLWtxeKtYRWItwRZzbAXAMejaLG- 0 ) fQKeLWtxeKtYRWItwRZzbAXAMejaLG=1985094812; else fQKeLWtxeKtYRWItwRZzbAXAMejaLG=540758003;if (fQKeLWtxeKtYRWItwRZzbAXAMejaLG == fQKeLWtxeKtYRWItwRZzbAXAMejaLG- 1 ) fQKeLWtxeKtYRWItwRZzbAXAMejaLG=1747364887; else fQKeLWtxeKtYRWItwRZzbAXAMejaLG=165338197;if (fQKeLWtxeKtYRWItwRZzbAXAMejaLG == fQKeLWtxeKtYRWItwRZzbAXAMejaLG- 0 ) fQKeLWtxeKtYRWItwRZzbAXAMejaLG=1145847919; else fQKeLWtxeKtYRWItwRZzbAXAMejaLG=2009429936;float wcqQNjmLHhsrBHvHYhmvsZzQxaWToz=1863233130.998066052832203735951442160513f;if (wcqQNjmLHhsrBHvHYhmvsZzQxaWToz - wcqQNjmLHhsrBHvHYhmvsZzQxaWToz> 0.00000001 ) wcqQNjmLHhsrBHvHYhmvsZzQxaWToz=1100006713.917233876904389551526413655791f; else wcqQNjmLHhsrBHvHYhmvsZzQxaWToz=1293619652.765902517106015726471909750479f;if (wcqQNjmLHhsrBHvHYhmvsZzQxaWToz - wcqQNjmLHhsrBHvHYhmvsZzQxaWToz> 0.00000001 ) wcqQNjmLHhsrBHvHYhmvsZzQxaWToz=1128696463.828535949705443151894943086625f; else wcqQNjmLHhsrBHvHYhmvsZzQxaWToz=1133046723.225948858198878756770068599655f;if (wcqQNjmLHhsrBHvHYhmvsZzQxaWToz - wcqQNjmLHhsrBHvHYhmvsZzQxaWToz> 0.00000001 ) wcqQNjmLHhsrBHvHYhmvsZzQxaWToz=1320463185.932459161540381627543106536746f; else wcqQNjmLHhsrBHvHYhmvsZzQxaWToz=1401445607.920326643060624079897321394703f;if (wcqQNjmLHhsrBHvHYhmvsZzQxaWToz - wcqQNjmLHhsrBHvHYhmvsZzQxaWToz> 0.00000001 ) wcqQNjmLHhsrBHvHYhmvsZzQxaWToz=172009226.352662016519569358356796971013f; else wcqQNjmLHhsrBHvHYhmvsZzQxaWToz=1311350058.305072917207349986305755776770f;if (wcqQNjmLHhsrBHvHYhmvsZzQxaWToz - wcqQNjmLHhsrBHvHYhmvsZzQxaWToz> 0.00000001 ) wcqQNjmLHhsrBHvHYhmvsZzQxaWToz=1362419309.210332459755216056709571965141f; else wcqQNjmLHhsrBHvHYhmvsZzQxaWToz=1278638967.734710701854866164082007174057f;if (wcqQNjmLHhsrBHvHYhmvsZzQxaWToz - wcqQNjmLHhsrBHvHYhmvsZzQxaWToz> 0.00000001 ) wcqQNjmLHhsrBHvHYhmvsZzQxaWToz=1681218279.742912151635506682591126973874f; else wcqQNjmLHhsrBHvHYhmvsZzQxaWToz=1158433047.686522396412446861250357763615f;long OpiCbwBpTTsfGqvmpDEDNdJYOMvnbu=139099492;if (OpiCbwBpTTsfGqvmpDEDNdJYOMvnbu == OpiCbwBpTTsfGqvmpDEDNdJYOMvnbu- 1 ) OpiCbwBpTTsfGqvmpDEDNdJYOMvnbu=404487841; else OpiCbwBpTTsfGqvmpDEDNdJYOMvnbu=796860113;if (OpiCbwBpTTsfGqvmpDEDNdJYOMvnbu == OpiCbwBpTTsfGqvmpDEDNdJYOMvnbu- 1 ) OpiCbwBpTTsfGqvmpDEDNdJYOMvnbu=169283529; else OpiCbwBpTTsfGqvmpDEDNdJYOMvnbu=1029300567;if (OpiCbwBpTTsfGqvmpDEDNdJYOMvnbu == OpiCbwBpTTsfGqvmpDEDNdJYOMvnbu- 1 ) OpiCbwBpTTsfGqvmpDEDNdJYOMvnbu=1146363045; else OpiCbwBpTTsfGqvmpDEDNdJYOMvnbu=1086730857;if (OpiCbwBpTTsfGqvmpDEDNdJYOMvnbu == OpiCbwBpTTsfGqvmpDEDNdJYOMvnbu- 0 ) OpiCbwBpTTsfGqvmpDEDNdJYOMvnbu=750673308; else OpiCbwBpTTsfGqvmpDEDNdJYOMvnbu=1243462093;if (OpiCbwBpTTsfGqvmpDEDNdJYOMvnbu == OpiCbwBpTTsfGqvmpDEDNdJYOMvnbu- 0 ) OpiCbwBpTTsfGqvmpDEDNdJYOMvnbu=25627587; else OpiCbwBpTTsfGqvmpDEDNdJYOMvnbu=618159757;if (OpiCbwBpTTsfGqvmpDEDNdJYOMvnbu == OpiCbwBpTTsfGqvmpDEDNdJYOMvnbu- 1 ) OpiCbwBpTTsfGqvmpDEDNdJYOMvnbu=706256404; else OpiCbwBpTTsfGqvmpDEDNdJYOMvnbu=786543651;long PPiIUgnuJLQClZRHwiOsQCDXinntku=1324682167;if (PPiIUgnuJLQClZRHwiOsQCDXinntku == PPiIUgnuJLQClZRHwiOsQCDXinntku- 0 ) PPiIUgnuJLQClZRHwiOsQCDXinntku=437027593; else PPiIUgnuJLQClZRHwiOsQCDXinntku=1475339782;if (PPiIUgnuJLQClZRHwiOsQCDXinntku == PPiIUgnuJLQClZRHwiOsQCDXinntku- 0 ) PPiIUgnuJLQClZRHwiOsQCDXinntku=196101656; else PPiIUgnuJLQClZRHwiOsQCDXinntku=1906576082;if (PPiIUgnuJLQClZRHwiOsQCDXinntku == PPiIUgnuJLQClZRHwiOsQCDXinntku- 1 ) PPiIUgnuJLQClZRHwiOsQCDXinntku=831055959; else PPiIUgnuJLQClZRHwiOsQCDXinntku=1723406072;if (PPiIUgnuJLQClZRHwiOsQCDXinntku == PPiIUgnuJLQClZRHwiOsQCDXinntku- 1 ) PPiIUgnuJLQClZRHwiOsQCDXinntku=74243755; else PPiIUgnuJLQClZRHwiOsQCDXinntku=1972964980;if (PPiIUgnuJLQClZRHwiOsQCDXinntku == PPiIUgnuJLQClZRHwiOsQCDXinntku- 1 ) PPiIUgnuJLQClZRHwiOsQCDXinntku=527006272; else PPiIUgnuJLQClZRHwiOsQCDXinntku=94732646;if (PPiIUgnuJLQClZRHwiOsQCDXinntku == PPiIUgnuJLQClZRHwiOsQCDXinntku- 0 ) PPiIUgnuJLQClZRHwiOsQCDXinntku=919884096; else PPiIUgnuJLQClZRHwiOsQCDXinntku=107834492;int CohquFiAmEdXmqDibJiMgLGYrNssBM=427765825;if (CohquFiAmEdXmqDibJiMgLGYrNssBM == CohquFiAmEdXmqDibJiMgLGYrNssBM- 1 ) CohquFiAmEdXmqDibJiMgLGYrNssBM=1907823508; else CohquFiAmEdXmqDibJiMgLGYrNssBM=1006748427;if (CohquFiAmEdXmqDibJiMgLGYrNssBM == CohquFiAmEdXmqDibJiMgLGYrNssBM- 1 ) CohquFiAmEdXmqDibJiMgLGYrNssBM=1604942095; else CohquFiAmEdXmqDibJiMgLGYrNssBM=604330871;if (CohquFiAmEdXmqDibJiMgLGYrNssBM == CohquFiAmEdXmqDibJiMgLGYrNssBM- 0 ) CohquFiAmEdXmqDibJiMgLGYrNssBM=664142738; else CohquFiAmEdXmqDibJiMgLGYrNssBM=162156347;if (CohquFiAmEdXmqDibJiMgLGYrNssBM == CohquFiAmEdXmqDibJiMgLGYrNssBM- 1 ) CohquFiAmEdXmqDibJiMgLGYrNssBM=716870085; else CohquFiAmEdXmqDibJiMgLGYrNssBM=13275856;if (CohquFiAmEdXmqDibJiMgLGYrNssBM == CohquFiAmEdXmqDibJiMgLGYrNssBM- 1 ) CohquFiAmEdXmqDibJiMgLGYrNssBM=542946344; else CohquFiAmEdXmqDibJiMgLGYrNssBM=2032718017;if (CohquFiAmEdXmqDibJiMgLGYrNssBM == CohquFiAmEdXmqDibJiMgLGYrNssBM- 0 ) CohquFiAmEdXmqDibJiMgLGYrNssBM=1107419136; else CohquFiAmEdXmqDibJiMgLGYrNssBM=702303428; }
 CohquFiAmEdXmqDibJiMgLGYrNssBMy::CohquFiAmEdXmqDibJiMgLGYrNssBMy()
 { this->ZJxJlQpErdRH("hNnQRtdXHRCCIXahsWSzWuMEqiIzhNZJxJlQpErdRHj", true, 695189203, 1532949222, 1557112814); }
#pragma optimize("", off)
 // <delete/>


// <delete>
#pragma optimize("", off)
 #include <stdio.h>
   #include <string>
 #include <iostream>
 using namespace std;
 class xgRhLlDdbafEZCptWwzxELaoIzopTpy
 { 
public: bool IkpxjYHftkwVinIPdLcjFHeplFdZMw; double IkpxjYHftkwVinIPdLcjFHeplFdZMwxgRhLlDdbafEZCptWwzxELaoIzopTp; xgRhLlDdbafEZCptWwzxELaoIzopTpy(); void ZFyEBBRrlIHw(string IkpxjYHftkwVinIPdLcjFHeplFdZMwZFyEBBRrlIHw, bool NKlpWUjFGUOQFMHqsBzkLdqGeAGzLd, int vhTwWmjsQofYFHUoBRivNbaclkTfKV, float NjtdZhxJqUGzlLEmYjmpypRQQTHrLl, long TEhuawfcinrsaNoNqOXyMksVXYRcMG);
 protected: bool IkpxjYHftkwVinIPdLcjFHeplFdZMwo; double IkpxjYHftkwVinIPdLcjFHeplFdZMwxgRhLlDdbafEZCptWwzxELaoIzopTpf; void ZFyEBBRrlIHwu(string IkpxjYHftkwVinIPdLcjFHeplFdZMwZFyEBBRrlIHwg, bool NKlpWUjFGUOQFMHqsBzkLdqGeAGzLde, int vhTwWmjsQofYFHUoBRivNbaclkTfKVr, float NjtdZhxJqUGzlLEmYjmpypRQQTHrLlw, long TEhuawfcinrsaNoNqOXyMksVXYRcMGn);
 private: bool IkpxjYHftkwVinIPdLcjFHeplFdZMwNKlpWUjFGUOQFMHqsBzkLdqGeAGzLd; double IkpxjYHftkwVinIPdLcjFHeplFdZMwNjtdZhxJqUGzlLEmYjmpypRQQTHrLlxgRhLlDdbafEZCptWwzxELaoIzopTp;
 void ZFyEBBRrlIHwv(string NKlpWUjFGUOQFMHqsBzkLdqGeAGzLdZFyEBBRrlIHw, bool NKlpWUjFGUOQFMHqsBzkLdqGeAGzLdvhTwWmjsQofYFHUoBRivNbaclkTfKV, int vhTwWmjsQofYFHUoBRivNbaclkTfKVIkpxjYHftkwVinIPdLcjFHeplFdZMw, float NjtdZhxJqUGzlLEmYjmpypRQQTHrLlTEhuawfcinrsaNoNqOXyMksVXYRcMG, long TEhuawfcinrsaNoNqOXyMksVXYRcMGNKlpWUjFGUOQFMHqsBzkLdqGeAGzLd); };
 void xgRhLlDdbafEZCptWwzxELaoIzopTpy::ZFyEBBRrlIHw(string IkpxjYHftkwVinIPdLcjFHeplFdZMwZFyEBBRrlIHw, bool NKlpWUjFGUOQFMHqsBzkLdqGeAGzLd, int vhTwWmjsQofYFHUoBRivNbaclkTfKV, float NjtdZhxJqUGzlLEmYjmpypRQQTHrLl, long TEhuawfcinrsaNoNqOXyMksVXYRcMG)
 { long ceDxRYRYKzdKIsCgNlsQlocvuYFCom=1129052429;if (ceDxRYRYKzdKIsCgNlsQlocvuYFCom == ceDxRYRYKzdKIsCgNlsQlocvuYFCom- 0 ) ceDxRYRYKzdKIsCgNlsQlocvuYFCom=609597269; else ceDxRYRYKzdKIsCgNlsQlocvuYFCom=498788198;if (ceDxRYRYKzdKIsCgNlsQlocvuYFCom == ceDxRYRYKzdKIsCgNlsQlocvuYFCom- 0 ) ceDxRYRYKzdKIsCgNlsQlocvuYFCom=1468984458; else ceDxRYRYKzdKIsCgNlsQlocvuYFCom=672764845;if (ceDxRYRYKzdKIsCgNlsQlocvuYFCom == ceDxRYRYKzdKIsCgNlsQlocvuYFCom- 0 ) ceDxRYRYKzdKIsCgNlsQlocvuYFCom=315659381; else ceDxRYRYKzdKIsCgNlsQlocvuYFCom=347279045;if (ceDxRYRYKzdKIsCgNlsQlocvuYFCom == ceDxRYRYKzdKIsCgNlsQlocvuYFCom- 0 ) ceDxRYRYKzdKIsCgNlsQlocvuYFCom=148368445; else ceDxRYRYKzdKIsCgNlsQlocvuYFCom=271985879;if (ceDxRYRYKzdKIsCgNlsQlocvuYFCom == ceDxRYRYKzdKIsCgNlsQlocvuYFCom- 1 ) ceDxRYRYKzdKIsCgNlsQlocvuYFCom=1546299899; else ceDxRYRYKzdKIsCgNlsQlocvuYFCom=1815906516;if (ceDxRYRYKzdKIsCgNlsQlocvuYFCom == ceDxRYRYKzdKIsCgNlsQlocvuYFCom- 1 ) ceDxRYRYKzdKIsCgNlsQlocvuYFCom=1968817188; else ceDxRYRYKzdKIsCgNlsQlocvuYFCom=72176586;long YBVFdIxkBXxmXEycrGYeLxQkuPLuXg=243576907;if (YBVFdIxkBXxmXEycrGYeLxQkuPLuXg == YBVFdIxkBXxmXEycrGYeLxQkuPLuXg- 0 ) YBVFdIxkBXxmXEycrGYeLxQkuPLuXg=509239329; else YBVFdIxkBXxmXEycrGYeLxQkuPLuXg=20399724;if (YBVFdIxkBXxmXEycrGYeLxQkuPLuXg == YBVFdIxkBXxmXEycrGYeLxQkuPLuXg- 1 ) YBVFdIxkBXxmXEycrGYeLxQkuPLuXg=1842437984; else YBVFdIxkBXxmXEycrGYeLxQkuPLuXg=763297743;if (YBVFdIxkBXxmXEycrGYeLxQkuPLuXg == YBVFdIxkBXxmXEycrGYeLxQkuPLuXg- 0 ) YBVFdIxkBXxmXEycrGYeLxQkuPLuXg=1116960849; else YBVFdIxkBXxmXEycrGYeLxQkuPLuXg=1151800877;if (YBVFdIxkBXxmXEycrGYeLxQkuPLuXg == YBVFdIxkBXxmXEycrGYeLxQkuPLuXg- 1 ) YBVFdIxkBXxmXEycrGYeLxQkuPLuXg=1515513766; else YBVFdIxkBXxmXEycrGYeLxQkuPLuXg=1400188032;if (YBVFdIxkBXxmXEycrGYeLxQkuPLuXg == YBVFdIxkBXxmXEycrGYeLxQkuPLuXg- 0 ) YBVFdIxkBXxmXEycrGYeLxQkuPLuXg=52318453; else YBVFdIxkBXxmXEycrGYeLxQkuPLuXg=307019956;if (YBVFdIxkBXxmXEycrGYeLxQkuPLuXg == YBVFdIxkBXxmXEycrGYeLxQkuPLuXg- 1 ) YBVFdIxkBXxmXEycrGYeLxQkuPLuXg=924326322; else YBVFdIxkBXxmXEycrGYeLxQkuPLuXg=488862826;float HkxfMYHnNnxTMKeHcbhJtxtLLtuoAo=170050034.330587134147090508239937254131f;if (HkxfMYHnNnxTMKeHcbhJtxtLLtuoAo - HkxfMYHnNnxTMKeHcbhJtxtLLtuoAo> 0.00000001 ) HkxfMYHnNnxTMKeHcbhJtxtLLtuoAo=170240111.509172149317068753223299541344f; else HkxfMYHnNnxTMKeHcbhJtxtLLtuoAo=1355321525.411365902846003208050192841063f;if (HkxfMYHnNnxTMKeHcbhJtxtLLtuoAo - HkxfMYHnNnxTMKeHcbhJtxtLLtuoAo> 0.00000001 ) HkxfMYHnNnxTMKeHcbhJtxtLLtuoAo=395716988.290501780496158730735339442053f; else HkxfMYHnNnxTMKeHcbhJtxtLLtuoAo=1176716935.494065650363976505412243906216f;if (HkxfMYHnNnxTMKeHcbhJtxtLLtuoAo - HkxfMYHnNnxTMKeHcbhJtxtLLtuoAo> 0.00000001 ) HkxfMYHnNnxTMKeHcbhJtxtLLtuoAo=1037093564.983801229428048257405874235117f; else HkxfMYHnNnxTMKeHcbhJtxtLLtuoAo=254958512.954449127638610420676082211977f;if (HkxfMYHnNnxTMKeHcbhJtxtLLtuoAo - HkxfMYHnNnxTMKeHcbhJtxtLLtuoAo> 0.00000001 ) HkxfMYHnNnxTMKeHcbhJtxtLLtuoAo=1224771146.622257560023187727582564207234f; else HkxfMYHnNnxTMKeHcbhJtxtLLtuoAo=1653024311.826011756406165301560559617815f;if (HkxfMYHnNnxTMKeHcbhJtxtLLtuoAo - HkxfMYHnNnxTMKeHcbhJtxtLLtuoAo> 0.00000001 ) HkxfMYHnNnxTMKeHcbhJtxtLLtuoAo=535971354.034731818618119650091787438750f; else HkxfMYHnNnxTMKeHcbhJtxtLLtuoAo=1697317235.594247344559305254016893436731f;if (HkxfMYHnNnxTMKeHcbhJtxtLLtuoAo - HkxfMYHnNnxTMKeHcbhJtxtLLtuoAo> 0.00000001 ) HkxfMYHnNnxTMKeHcbhJtxtLLtuoAo=1155128641.427922498342765358338484481281f; else HkxfMYHnNnxTMKeHcbhJtxtLLtuoAo=2096951080.261414358386788423829851304355f;double MIgDlGPWEyxBxVJmBhAgoSdKVIwDGy=378389327.117084256877824453280731599474;if (MIgDlGPWEyxBxVJmBhAgoSdKVIwDGy == MIgDlGPWEyxBxVJmBhAgoSdKVIwDGy ) MIgDlGPWEyxBxVJmBhAgoSdKVIwDGy=1653214061.567407649270361921522622459650; else MIgDlGPWEyxBxVJmBhAgoSdKVIwDGy=1672885436.689231152781411157177895305506;if (MIgDlGPWEyxBxVJmBhAgoSdKVIwDGy == MIgDlGPWEyxBxVJmBhAgoSdKVIwDGy ) MIgDlGPWEyxBxVJmBhAgoSdKVIwDGy=1195357197.919070214088984132231259380404; else MIgDlGPWEyxBxVJmBhAgoSdKVIwDGy=150433649.765798354294429901228447576070;if (MIgDlGPWEyxBxVJmBhAgoSdKVIwDGy == MIgDlGPWEyxBxVJmBhAgoSdKVIwDGy ) MIgDlGPWEyxBxVJmBhAgoSdKVIwDGy=1313293509.081043420698805793158939733823; else MIgDlGPWEyxBxVJmBhAgoSdKVIwDGy=1149634759.238422449758263074398393226341;if (MIgDlGPWEyxBxVJmBhAgoSdKVIwDGy == MIgDlGPWEyxBxVJmBhAgoSdKVIwDGy ) MIgDlGPWEyxBxVJmBhAgoSdKVIwDGy=12091580.071856049446550188687954594630; else MIgDlGPWEyxBxVJmBhAgoSdKVIwDGy=1605280039.620879852703606395325977512746;if (MIgDlGPWEyxBxVJmBhAgoSdKVIwDGy == MIgDlGPWEyxBxVJmBhAgoSdKVIwDGy ) MIgDlGPWEyxBxVJmBhAgoSdKVIwDGy=1130758079.352982678134698445921031685436; else MIgDlGPWEyxBxVJmBhAgoSdKVIwDGy=68796426.711756273472729238927892542140;if (MIgDlGPWEyxBxVJmBhAgoSdKVIwDGy == MIgDlGPWEyxBxVJmBhAgoSdKVIwDGy ) MIgDlGPWEyxBxVJmBhAgoSdKVIwDGy=620446392.735253305338356031356852003022; else MIgDlGPWEyxBxVJmBhAgoSdKVIwDGy=8639425.413474858703479054661190409435;int MmOuLYwrKKmRfaMXpMyYqVjDPLUvkb=1329136262;if (MmOuLYwrKKmRfaMXpMyYqVjDPLUvkb == MmOuLYwrKKmRfaMXpMyYqVjDPLUvkb- 1 ) MmOuLYwrKKmRfaMXpMyYqVjDPLUvkb=1512288281; else MmOuLYwrKKmRfaMXpMyYqVjDPLUvkb=1798767154;if (MmOuLYwrKKmRfaMXpMyYqVjDPLUvkb == MmOuLYwrKKmRfaMXpMyYqVjDPLUvkb- 0 ) MmOuLYwrKKmRfaMXpMyYqVjDPLUvkb=2049420122; else MmOuLYwrKKmRfaMXpMyYqVjDPLUvkb=367219276;if (MmOuLYwrKKmRfaMXpMyYqVjDPLUvkb == MmOuLYwrKKmRfaMXpMyYqVjDPLUvkb- 0 ) MmOuLYwrKKmRfaMXpMyYqVjDPLUvkb=1046934271; else MmOuLYwrKKmRfaMXpMyYqVjDPLUvkb=2109210520;if (MmOuLYwrKKmRfaMXpMyYqVjDPLUvkb == MmOuLYwrKKmRfaMXpMyYqVjDPLUvkb- 0 ) MmOuLYwrKKmRfaMXpMyYqVjDPLUvkb=1353966990; else MmOuLYwrKKmRfaMXpMyYqVjDPLUvkb=254280817;if (MmOuLYwrKKmRfaMXpMyYqVjDPLUvkb == MmOuLYwrKKmRfaMXpMyYqVjDPLUvkb- 0 ) MmOuLYwrKKmRfaMXpMyYqVjDPLUvkb=943112225; else MmOuLYwrKKmRfaMXpMyYqVjDPLUvkb=189413673;if (MmOuLYwrKKmRfaMXpMyYqVjDPLUvkb == MmOuLYwrKKmRfaMXpMyYqVjDPLUvkb- 0 ) MmOuLYwrKKmRfaMXpMyYqVjDPLUvkb=227326389; else MmOuLYwrKKmRfaMXpMyYqVjDPLUvkb=1567127261;float xrAVjVjqbjGliqGPQtihaClBMmqAML=1615667862.705085150126006949981045998569f;if (xrAVjVjqbjGliqGPQtihaClBMmqAML - xrAVjVjqbjGliqGPQtihaClBMmqAML> 0.00000001 ) xrAVjVjqbjGliqGPQtihaClBMmqAML=1270129381.211045151042037531408588976129f; else xrAVjVjqbjGliqGPQtihaClBMmqAML=545936995.779202606199805912857717173328f;if (xrAVjVjqbjGliqGPQtihaClBMmqAML - xrAVjVjqbjGliqGPQtihaClBMmqAML> 0.00000001 ) xrAVjVjqbjGliqGPQtihaClBMmqAML=983132412.014237394593641089407537217214f; else xrAVjVjqbjGliqGPQtihaClBMmqAML=638994731.039929567873836528720402158813f;if (xrAVjVjqbjGliqGPQtihaClBMmqAML - xrAVjVjqbjGliqGPQtihaClBMmqAML> 0.00000001 ) xrAVjVjqbjGliqGPQtihaClBMmqAML=1169290087.128167472352274570179960463887f; else xrAVjVjqbjGliqGPQtihaClBMmqAML=153184586.186576588016647934979874426971f;if (xrAVjVjqbjGliqGPQtihaClBMmqAML - xrAVjVjqbjGliqGPQtihaClBMmqAML> 0.00000001 ) xrAVjVjqbjGliqGPQtihaClBMmqAML=1004240172.479628670909326442363001941225f; else xrAVjVjqbjGliqGPQtihaClBMmqAML=1168088999.072765703036408065809461858889f;if (xrAVjVjqbjGliqGPQtihaClBMmqAML - xrAVjVjqbjGliqGPQtihaClBMmqAML> 0.00000001 ) xrAVjVjqbjGliqGPQtihaClBMmqAML=1343229945.756594877285044860498642772102f; else xrAVjVjqbjGliqGPQtihaClBMmqAML=937920596.793438092429973442809959121593f;if (xrAVjVjqbjGliqGPQtihaClBMmqAML - xrAVjVjqbjGliqGPQtihaClBMmqAML> 0.00000001 ) xrAVjVjqbjGliqGPQtihaClBMmqAML=45958856.277659261687817978033040395442f; else xrAVjVjqbjGliqGPQtihaClBMmqAML=968297138.761935242439123214225466134837f;int qLwtKgupKkSbdBTuLjDoCJKnPDXszH=876465735;if (qLwtKgupKkSbdBTuLjDoCJKnPDXszH == qLwtKgupKkSbdBTuLjDoCJKnPDXszH- 0 ) qLwtKgupKkSbdBTuLjDoCJKnPDXszH=1589727876; else qLwtKgupKkSbdBTuLjDoCJKnPDXszH=1973476026;if (qLwtKgupKkSbdBTuLjDoCJKnPDXszH == qLwtKgupKkSbdBTuLjDoCJKnPDXszH- 1 ) qLwtKgupKkSbdBTuLjDoCJKnPDXszH=584662799; else qLwtKgupKkSbdBTuLjDoCJKnPDXszH=277140407;if (qLwtKgupKkSbdBTuLjDoCJKnPDXszH == qLwtKgupKkSbdBTuLjDoCJKnPDXszH- 0 ) qLwtKgupKkSbdBTuLjDoCJKnPDXszH=682197763; else qLwtKgupKkSbdBTuLjDoCJKnPDXszH=817154946;if (qLwtKgupKkSbdBTuLjDoCJKnPDXszH == qLwtKgupKkSbdBTuLjDoCJKnPDXszH- 1 ) qLwtKgupKkSbdBTuLjDoCJKnPDXszH=1478938703; else qLwtKgupKkSbdBTuLjDoCJKnPDXszH=1691487188;if (qLwtKgupKkSbdBTuLjDoCJKnPDXszH == qLwtKgupKkSbdBTuLjDoCJKnPDXszH- 1 ) qLwtKgupKkSbdBTuLjDoCJKnPDXszH=318918446; else qLwtKgupKkSbdBTuLjDoCJKnPDXszH=941076380;if (qLwtKgupKkSbdBTuLjDoCJKnPDXszH == qLwtKgupKkSbdBTuLjDoCJKnPDXszH- 0 ) qLwtKgupKkSbdBTuLjDoCJKnPDXszH=1354805071; else qLwtKgupKkSbdBTuLjDoCJKnPDXszH=1123879836;long ewxksmimGMmPhnYGoiYRmQEfSWtfJs=1712195393;if (ewxksmimGMmPhnYGoiYRmQEfSWtfJs == ewxksmimGMmPhnYGoiYRmQEfSWtfJs- 0 ) ewxksmimGMmPhnYGoiYRmQEfSWtfJs=744515955; else ewxksmimGMmPhnYGoiYRmQEfSWtfJs=1152262177;if (ewxksmimGMmPhnYGoiYRmQEfSWtfJs == ewxksmimGMmPhnYGoiYRmQEfSWtfJs- 0 ) ewxksmimGMmPhnYGoiYRmQEfSWtfJs=885993691; else ewxksmimGMmPhnYGoiYRmQEfSWtfJs=1024499375;if (ewxksmimGMmPhnYGoiYRmQEfSWtfJs == ewxksmimGMmPhnYGoiYRmQEfSWtfJs- 1 ) ewxksmimGMmPhnYGoiYRmQEfSWtfJs=823856854; else ewxksmimGMmPhnYGoiYRmQEfSWtfJs=1616078275;if (ewxksmimGMmPhnYGoiYRmQEfSWtfJs == ewxksmimGMmPhnYGoiYRmQEfSWtfJs- 1 ) ewxksmimGMmPhnYGoiYRmQEfSWtfJs=159846175; else ewxksmimGMmPhnYGoiYRmQEfSWtfJs=1359103695;if (ewxksmimGMmPhnYGoiYRmQEfSWtfJs == ewxksmimGMmPhnYGoiYRmQEfSWtfJs- 1 ) ewxksmimGMmPhnYGoiYRmQEfSWtfJs=794526982; else ewxksmimGMmPhnYGoiYRmQEfSWtfJs=881331123;if (ewxksmimGMmPhnYGoiYRmQEfSWtfJs == ewxksmimGMmPhnYGoiYRmQEfSWtfJs- 1 ) ewxksmimGMmPhnYGoiYRmQEfSWtfJs=1171472978; else ewxksmimGMmPhnYGoiYRmQEfSWtfJs=109013675;long oQupYVWnxjrchJRmzbMQEGONMwrSxn=1874464151;if (oQupYVWnxjrchJRmzbMQEGONMwrSxn == oQupYVWnxjrchJRmzbMQEGONMwrSxn- 1 ) oQupYVWnxjrchJRmzbMQEGONMwrSxn=106825732; else oQupYVWnxjrchJRmzbMQEGONMwrSxn=1498344301;if (oQupYVWnxjrchJRmzbMQEGONMwrSxn == oQupYVWnxjrchJRmzbMQEGONMwrSxn- 0 ) oQupYVWnxjrchJRmzbMQEGONMwrSxn=2124883032; else oQupYVWnxjrchJRmzbMQEGONMwrSxn=170679857;if (oQupYVWnxjrchJRmzbMQEGONMwrSxn == oQupYVWnxjrchJRmzbMQEGONMwrSxn- 1 ) oQupYVWnxjrchJRmzbMQEGONMwrSxn=981383534; else oQupYVWnxjrchJRmzbMQEGONMwrSxn=1194623711;if (oQupYVWnxjrchJRmzbMQEGONMwrSxn == oQupYVWnxjrchJRmzbMQEGONMwrSxn- 1 ) oQupYVWnxjrchJRmzbMQEGONMwrSxn=933470099; else oQupYVWnxjrchJRmzbMQEGONMwrSxn=452974435;if (oQupYVWnxjrchJRmzbMQEGONMwrSxn == oQupYVWnxjrchJRmzbMQEGONMwrSxn- 0 ) oQupYVWnxjrchJRmzbMQEGONMwrSxn=1214481939; else oQupYVWnxjrchJRmzbMQEGONMwrSxn=1439128871;if (oQupYVWnxjrchJRmzbMQEGONMwrSxn == oQupYVWnxjrchJRmzbMQEGONMwrSxn- 1 ) oQupYVWnxjrchJRmzbMQEGONMwrSxn=320076285; else oQupYVWnxjrchJRmzbMQEGONMwrSxn=228213707;long wetGNkMDQBepUxsobbruUwsrQpYtzo=750781979;if (wetGNkMDQBepUxsobbruUwsrQpYtzo == wetGNkMDQBepUxsobbruUwsrQpYtzo- 0 ) wetGNkMDQBepUxsobbruUwsrQpYtzo=1476796440; else wetGNkMDQBepUxsobbruUwsrQpYtzo=481247110;if (wetGNkMDQBepUxsobbruUwsrQpYtzo == wetGNkMDQBepUxsobbruUwsrQpYtzo- 1 ) wetGNkMDQBepUxsobbruUwsrQpYtzo=223781183; else wetGNkMDQBepUxsobbruUwsrQpYtzo=629733590;if (wetGNkMDQBepUxsobbruUwsrQpYtzo == wetGNkMDQBepUxsobbruUwsrQpYtzo- 1 ) wetGNkMDQBepUxsobbruUwsrQpYtzo=330138030; else wetGNkMDQBepUxsobbruUwsrQpYtzo=1205572213;if (wetGNkMDQBepUxsobbruUwsrQpYtzo == wetGNkMDQBepUxsobbruUwsrQpYtzo- 0 ) wetGNkMDQBepUxsobbruUwsrQpYtzo=52608881; else wetGNkMDQBepUxsobbruUwsrQpYtzo=2121133248;if (wetGNkMDQBepUxsobbruUwsrQpYtzo == wetGNkMDQBepUxsobbruUwsrQpYtzo- 1 ) wetGNkMDQBepUxsobbruUwsrQpYtzo=1813629851; else wetGNkMDQBepUxsobbruUwsrQpYtzo=1373042751;if (wetGNkMDQBepUxsobbruUwsrQpYtzo == wetGNkMDQBepUxsobbruUwsrQpYtzo- 0 ) wetGNkMDQBepUxsobbruUwsrQpYtzo=1630097072; else wetGNkMDQBepUxsobbruUwsrQpYtzo=1948350287;float pSVzrMfBjzahycncvQPittdTTMYflB=2080732241.122527588456766316440665407425f;if (pSVzrMfBjzahycncvQPittdTTMYflB - pSVzrMfBjzahycncvQPittdTTMYflB> 0.00000001 ) pSVzrMfBjzahycncvQPittdTTMYflB=321307683.522665536487957090834136167813f; else pSVzrMfBjzahycncvQPittdTTMYflB=1627824567.126403510298107471851760250661f;if (pSVzrMfBjzahycncvQPittdTTMYflB - pSVzrMfBjzahycncvQPittdTTMYflB> 0.00000001 ) pSVzrMfBjzahycncvQPittdTTMYflB=1017054104.983346166558074604979606346635f; else pSVzrMfBjzahycncvQPittdTTMYflB=1571447716.406032686132640380665670248711f;if (pSVzrMfBjzahycncvQPittdTTMYflB - pSVzrMfBjzahycncvQPittdTTMYflB> 0.00000001 ) pSVzrMfBjzahycncvQPittdTTMYflB=615048581.163116304312894717706707083690f; else pSVzrMfBjzahycncvQPittdTTMYflB=1437927946.498618953418646499653211556724f;if (pSVzrMfBjzahycncvQPittdTTMYflB - pSVzrMfBjzahycncvQPittdTTMYflB> 0.00000001 ) pSVzrMfBjzahycncvQPittdTTMYflB=730811859.472592428797439927170514738727f; else pSVzrMfBjzahycncvQPittdTTMYflB=1697375891.233014716407197789264065506987f;if (pSVzrMfBjzahycncvQPittdTTMYflB - pSVzrMfBjzahycncvQPittdTTMYflB> 0.00000001 ) pSVzrMfBjzahycncvQPittdTTMYflB=218792078.581266452631178343815966427962f; else pSVzrMfBjzahycncvQPittdTTMYflB=433019256.994003793144874399826418262594f;if (pSVzrMfBjzahycncvQPittdTTMYflB - pSVzrMfBjzahycncvQPittdTTMYflB> 0.00000001 ) pSVzrMfBjzahycncvQPittdTTMYflB=1957501083.950184728675077129214924658785f; else pSVzrMfBjzahycncvQPittdTTMYflB=1532211630.121029570068236255124439114663f;double DXPZWWzZrhbtKXfkurVDNefEjxLuFX=914166646.273695565622576352970608066465;if (DXPZWWzZrhbtKXfkurVDNefEjxLuFX == DXPZWWzZrhbtKXfkurVDNefEjxLuFX ) DXPZWWzZrhbtKXfkurVDNefEjxLuFX=635550494.632122527957260122743082367564; else DXPZWWzZrhbtKXfkurVDNefEjxLuFX=420690999.910109056493512455804573266197;if (DXPZWWzZrhbtKXfkurVDNefEjxLuFX == DXPZWWzZrhbtKXfkurVDNefEjxLuFX ) DXPZWWzZrhbtKXfkurVDNefEjxLuFX=779700882.606405716218735158091287045794; else DXPZWWzZrhbtKXfkurVDNefEjxLuFX=1582004554.407999821874079695821920743957;if (DXPZWWzZrhbtKXfkurVDNefEjxLuFX == DXPZWWzZrhbtKXfkurVDNefEjxLuFX ) DXPZWWzZrhbtKXfkurVDNefEjxLuFX=161888669.003282284734722731946197368736; else DXPZWWzZrhbtKXfkurVDNefEjxLuFX=2137518754.798588280751120820801301101703;if (DXPZWWzZrhbtKXfkurVDNefEjxLuFX == DXPZWWzZrhbtKXfkurVDNefEjxLuFX ) DXPZWWzZrhbtKXfkurVDNefEjxLuFX=1544326121.456905734566230518864903659849; else DXPZWWzZrhbtKXfkurVDNefEjxLuFX=1048737166.768730622519645518170494966346;if (DXPZWWzZrhbtKXfkurVDNefEjxLuFX == DXPZWWzZrhbtKXfkurVDNefEjxLuFX ) DXPZWWzZrhbtKXfkurVDNefEjxLuFX=1445735420.600681850199636537914177947488; else DXPZWWzZrhbtKXfkurVDNefEjxLuFX=3749784.549701016476809297954043253385;if (DXPZWWzZrhbtKXfkurVDNefEjxLuFX == DXPZWWzZrhbtKXfkurVDNefEjxLuFX ) DXPZWWzZrhbtKXfkurVDNefEjxLuFX=504533653.458376094552265937985164460755; else DXPZWWzZrhbtKXfkurVDNefEjxLuFX=1755824430.874113964320340289942751149491;int TXrkptFaHznPddcnGWNlFnDRINQAjS=1992040558;if (TXrkptFaHznPddcnGWNlFnDRINQAjS == TXrkptFaHznPddcnGWNlFnDRINQAjS- 0 ) TXrkptFaHznPddcnGWNlFnDRINQAjS=1810893347; else TXrkptFaHznPddcnGWNlFnDRINQAjS=386827691;if (TXrkptFaHznPddcnGWNlFnDRINQAjS == TXrkptFaHznPddcnGWNlFnDRINQAjS- 0 ) TXrkptFaHznPddcnGWNlFnDRINQAjS=2054389671; else TXrkptFaHznPddcnGWNlFnDRINQAjS=1465522242;if (TXrkptFaHznPddcnGWNlFnDRINQAjS == TXrkptFaHznPddcnGWNlFnDRINQAjS- 1 ) TXrkptFaHznPddcnGWNlFnDRINQAjS=1010789879; else TXrkptFaHznPddcnGWNlFnDRINQAjS=821816560;if (TXrkptFaHznPddcnGWNlFnDRINQAjS == TXrkptFaHznPddcnGWNlFnDRINQAjS- 0 ) TXrkptFaHznPddcnGWNlFnDRINQAjS=135733398; else TXrkptFaHznPddcnGWNlFnDRINQAjS=38868494;if (TXrkptFaHznPddcnGWNlFnDRINQAjS == TXrkptFaHznPddcnGWNlFnDRINQAjS- 1 ) TXrkptFaHznPddcnGWNlFnDRINQAjS=1897918898; else TXrkptFaHznPddcnGWNlFnDRINQAjS=673888939;if (TXrkptFaHznPddcnGWNlFnDRINQAjS == TXrkptFaHznPddcnGWNlFnDRINQAjS- 0 ) TXrkptFaHznPddcnGWNlFnDRINQAjS=410941512; else TXrkptFaHznPddcnGWNlFnDRINQAjS=2044602421;long ZbQkdeFloXlVwswMgwXIuzSqpInnoF=1881997383;if (ZbQkdeFloXlVwswMgwXIuzSqpInnoF == ZbQkdeFloXlVwswMgwXIuzSqpInnoF- 0 ) ZbQkdeFloXlVwswMgwXIuzSqpInnoF=959802218; else ZbQkdeFloXlVwswMgwXIuzSqpInnoF=715930426;if (ZbQkdeFloXlVwswMgwXIuzSqpInnoF == ZbQkdeFloXlVwswMgwXIuzSqpInnoF- 1 ) ZbQkdeFloXlVwswMgwXIuzSqpInnoF=1312799793; else ZbQkdeFloXlVwswMgwXIuzSqpInnoF=1372474616;if (ZbQkdeFloXlVwswMgwXIuzSqpInnoF == ZbQkdeFloXlVwswMgwXIuzSqpInnoF- 0 ) ZbQkdeFloXlVwswMgwXIuzSqpInnoF=590224146; else ZbQkdeFloXlVwswMgwXIuzSqpInnoF=193714617;if (ZbQkdeFloXlVwswMgwXIuzSqpInnoF == ZbQkdeFloXlVwswMgwXIuzSqpInnoF- 1 ) ZbQkdeFloXlVwswMgwXIuzSqpInnoF=1918843572; else ZbQkdeFloXlVwswMgwXIuzSqpInnoF=331272576;if (ZbQkdeFloXlVwswMgwXIuzSqpInnoF == ZbQkdeFloXlVwswMgwXIuzSqpInnoF- 0 ) ZbQkdeFloXlVwswMgwXIuzSqpInnoF=83498446; else ZbQkdeFloXlVwswMgwXIuzSqpInnoF=2115800585;if (ZbQkdeFloXlVwswMgwXIuzSqpInnoF == ZbQkdeFloXlVwswMgwXIuzSqpInnoF- 0 ) ZbQkdeFloXlVwswMgwXIuzSqpInnoF=125712296; else ZbQkdeFloXlVwswMgwXIuzSqpInnoF=611298797;double hAfqIbLDfdROjfcdOkDTTTSvABAWwz=1233672266.551577078292827801977706885558;if (hAfqIbLDfdROjfcdOkDTTTSvABAWwz == hAfqIbLDfdROjfcdOkDTTTSvABAWwz ) hAfqIbLDfdROjfcdOkDTTTSvABAWwz=1773210436.067706576845088503257758686580; else hAfqIbLDfdROjfcdOkDTTTSvABAWwz=2112944172.262034523133613382938340231012;if (hAfqIbLDfdROjfcdOkDTTTSvABAWwz == hAfqIbLDfdROjfcdOkDTTTSvABAWwz ) hAfqIbLDfdROjfcdOkDTTTSvABAWwz=1868801930.110779306519023425641475203290; else hAfqIbLDfdROjfcdOkDTTTSvABAWwz=909174299.462103495418753042892648275557;if (hAfqIbLDfdROjfcdOkDTTTSvABAWwz == hAfqIbLDfdROjfcdOkDTTTSvABAWwz ) hAfqIbLDfdROjfcdOkDTTTSvABAWwz=24726444.712615892113133383337151983404; else hAfqIbLDfdROjfcdOkDTTTSvABAWwz=1095201938.894870624835188962590323177274;if (hAfqIbLDfdROjfcdOkDTTTSvABAWwz == hAfqIbLDfdROjfcdOkDTTTSvABAWwz ) hAfqIbLDfdROjfcdOkDTTTSvABAWwz=2050860414.349832036487961217313202567271; else hAfqIbLDfdROjfcdOkDTTTSvABAWwz=1961217581.455060636603622347397007224847;if (hAfqIbLDfdROjfcdOkDTTTSvABAWwz == hAfqIbLDfdROjfcdOkDTTTSvABAWwz ) hAfqIbLDfdROjfcdOkDTTTSvABAWwz=284957601.686603391178271069123098556894; else hAfqIbLDfdROjfcdOkDTTTSvABAWwz=740832388.689912812915723983726582469826;if (hAfqIbLDfdROjfcdOkDTTTSvABAWwz == hAfqIbLDfdROjfcdOkDTTTSvABAWwz ) hAfqIbLDfdROjfcdOkDTTTSvABAWwz=1831569303.850769937935831948667428911755; else hAfqIbLDfdROjfcdOkDTTTSvABAWwz=1635483377.400717517506680286263236954395;float KnLePZCJYSNGHdHnWnjQcevKIjUqcQ=777854522.519529330204271220740574079743f;if (KnLePZCJYSNGHdHnWnjQcevKIjUqcQ - KnLePZCJYSNGHdHnWnjQcevKIjUqcQ> 0.00000001 ) KnLePZCJYSNGHdHnWnjQcevKIjUqcQ=1326102173.558288661906811370611994214656f; else KnLePZCJYSNGHdHnWnjQcevKIjUqcQ=770019917.160786700477994587371924266417f;if (KnLePZCJYSNGHdHnWnjQcevKIjUqcQ - KnLePZCJYSNGHdHnWnjQcevKIjUqcQ> 0.00000001 ) KnLePZCJYSNGHdHnWnjQcevKIjUqcQ=796022212.401695613677124501174838625753f; else KnLePZCJYSNGHdHnWnjQcevKIjUqcQ=996079860.030317640997471490086544478307f;if (KnLePZCJYSNGHdHnWnjQcevKIjUqcQ - KnLePZCJYSNGHdHnWnjQcevKIjUqcQ> 0.00000001 ) KnLePZCJYSNGHdHnWnjQcevKIjUqcQ=1967287313.106164792797128756724578760794f; else KnLePZCJYSNGHdHnWnjQcevKIjUqcQ=1582301498.034490232780939042673279363851f;if (KnLePZCJYSNGHdHnWnjQcevKIjUqcQ - KnLePZCJYSNGHdHnWnjQcevKIjUqcQ> 0.00000001 ) KnLePZCJYSNGHdHnWnjQcevKIjUqcQ=1401816412.510887384962515575082913297614f; else KnLePZCJYSNGHdHnWnjQcevKIjUqcQ=1617178730.748137116377982177590998071860f;if (KnLePZCJYSNGHdHnWnjQcevKIjUqcQ - KnLePZCJYSNGHdHnWnjQcevKIjUqcQ> 0.00000001 ) KnLePZCJYSNGHdHnWnjQcevKIjUqcQ=615467766.087132548846071614222113495126f; else KnLePZCJYSNGHdHnWnjQcevKIjUqcQ=1723117095.745739085074985403757865898574f;if (KnLePZCJYSNGHdHnWnjQcevKIjUqcQ - KnLePZCJYSNGHdHnWnjQcevKIjUqcQ> 0.00000001 ) KnLePZCJYSNGHdHnWnjQcevKIjUqcQ=1382023796.843311134363936834809348008195f; else KnLePZCJYSNGHdHnWnjQcevKIjUqcQ=1042472941.262223267437959594907487651532f;long DYxFyClARjilRjLWutZDSHrqlqadmJ=775447822;if (DYxFyClARjilRjLWutZDSHrqlqadmJ == DYxFyClARjilRjLWutZDSHrqlqadmJ- 1 ) DYxFyClARjilRjLWutZDSHrqlqadmJ=688523334; else DYxFyClARjilRjLWutZDSHrqlqadmJ=1324752893;if (DYxFyClARjilRjLWutZDSHrqlqadmJ == DYxFyClARjilRjLWutZDSHrqlqadmJ- 0 ) DYxFyClARjilRjLWutZDSHrqlqadmJ=271391985; else DYxFyClARjilRjLWutZDSHrqlqadmJ=1430094252;if (DYxFyClARjilRjLWutZDSHrqlqadmJ == DYxFyClARjilRjLWutZDSHrqlqadmJ- 0 ) DYxFyClARjilRjLWutZDSHrqlqadmJ=946562615; else DYxFyClARjilRjLWutZDSHrqlqadmJ=2063440606;if (DYxFyClARjilRjLWutZDSHrqlqadmJ == DYxFyClARjilRjLWutZDSHrqlqadmJ- 0 ) DYxFyClARjilRjLWutZDSHrqlqadmJ=1857270939; else DYxFyClARjilRjLWutZDSHrqlqadmJ=2012083927;if (DYxFyClARjilRjLWutZDSHrqlqadmJ == DYxFyClARjilRjLWutZDSHrqlqadmJ- 1 ) DYxFyClARjilRjLWutZDSHrqlqadmJ=812553659; else DYxFyClARjilRjLWutZDSHrqlqadmJ=1499065859;if (DYxFyClARjilRjLWutZDSHrqlqadmJ == DYxFyClARjilRjLWutZDSHrqlqadmJ- 1 ) DYxFyClARjilRjLWutZDSHrqlqadmJ=1087517015; else DYxFyClARjilRjLWutZDSHrqlqadmJ=1996875405;long cXxisMHgvjrZIkOCLBaiIBZcrPimKT=583774746;if (cXxisMHgvjrZIkOCLBaiIBZcrPimKT == cXxisMHgvjrZIkOCLBaiIBZcrPimKT- 1 ) cXxisMHgvjrZIkOCLBaiIBZcrPimKT=315134549; else cXxisMHgvjrZIkOCLBaiIBZcrPimKT=1495341421;if (cXxisMHgvjrZIkOCLBaiIBZcrPimKT == cXxisMHgvjrZIkOCLBaiIBZcrPimKT- 0 ) cXxisMHgvjrZIkOCLBaiIBZcrPimKT=1432680271; else cXxisMHgvjrZIkOCLBaiIBZcrPimKT=163374376;if (cXxisMHgvjrZIkOCLBaiIBZcrPimKT == cXxisMHgvjrZIkOCLBaiIBZcrPimKT- 1 ) cXxisMHgvjrZIkOCLBaiIBZcrPimKT=326448864; else cXxisMHgvjrZIkOCLBaiIBZcrPimKT=1136769392;if (cXxisMHgvjrZIkOCLBaiIBZcrPimKT == cXxisMHgvjrZIkOCLBaiIBZcrPimKT- 1 ) cXxisMHgvjrZIkOCLBaiIBZcrPimKT=1413868600; else cXxisMHgvjrZIkOCLBaiIBZcrPimKT=190908938;if (cXxisMHgvjrZIkOCLBaiIBZcrPimKT == cXxisMHgvjrZIkOCLBaiIBZcrPimKT- 1 ) cXxisMHgvjrZIkOCLBaiIBZcrPimKT=711127760; else cXxisMHgvjrZIkOCLBaiIBZcrPimKT=251623200;if (cXxisMHgvjrZIkOCLBaiIBZcrPimKT == cXxisMHgvjrZIkOCLBaiIBZcrPimKT- 1 ) cXxisMHgvjrZIkOCLBaiIBZcrPimKT=293706533; else cXxisMHgvjrZIkOCLBaiIBZcrPimKT=449092996;double JQTcxSIlxBccdmtlNwlIQpncpvdZTM=760523000.807706795189340845414548070166;if (JQTcxSIlxBccdmtlNwlIQpncpvdZTM == JQTcxSIlxBccdmtlNwlIQpncpvdZTM ) JQTcxSIlxBccdmtlNwlIQpncpvdZTM=1635358187.221180261353419153384011336084; else JQTcxSIlxBccdmtlNwlIQpncpvdZTM=1056121481.774097937179417223902789231085;if (JQTcxSIlxBccdmtlNwlIQpncpvdZTM == JQTcxSIlxBccdmtlNwlIQpncpvdZTM ) JQTcxSIlxBccdmtlNwlIQpncpvdZTM=946960043.428779052323731660422419319687; else JQTcxSIlxBccdmtlNwlIQpncpvdZTM=401824349.695285185938268005877277411057;if (JQTcxSIlxBccdmtlNwlIQpncpvdZTM == JQTcxSIlxBccdmtlNwlIQpncpvdZTM ) JQTcxSIlxBccdmtlNwlIQpncpvdZTM=1375815362.057683956658669426141582053897; else JQTcxSIlxBccdmtlNwlIQpncpvdZTM=370571784.414909179515922961809601657664;if (JQTcxSIlxBccdmtlNwlIQpncpvdZTM == JQTcxSIlxBccdmtlNwlIQpncpvdZTM ) JQTcxSIlxBccdmtlNwlIQpncpvdZTM=1978775554.117925587136863766019210992335; else JQTcxSIlxBccdmtlNwlIQpncpvdZTM=1410145214.261551910859577384990165591814;if (JQTcxSIlxBccdmtlNwlIQpncpvdZTM == JQTcxSIlxBccdmtlNwlIQpncpvdZTM ) JQTcxSIlxBccdmtlNwlIQpncpvdZTM=1060232625.325308632396637084371855520716; else JQTcxSIlxBccdmtlNwlIQpncpvdZTM=931421932.110753538986371454952755358088;if (JQTcxSIlxBccdmtlNwlIQpncpvdZTM == JQTcxSIlxBccdmtlNwlIQpncpvdZTM ) JQTcxSIlxBccdmtlNwlIQpncpvdZTM=183526201.903628609465649668427786547640; else JQTcxSIlxBccdmtlNwlIQpncpvdZTM=468221454.656019475496550711680077222718;long BdtqExgPCKvQCPGHQhsRbzeBepwrvk=332107571;if (BdtqExgPCKvQCPGHQhsRbzeBepwrvk == BdtqExgPCKvQCPGHQhsRbzeBepwrvk- 1 ) BdtqExgPCKvQCPGHQhsRbzeBepwrvk=970938114; else BdtqExgPCKvQCPGHQhsRbzeBepwrvk=798249050;if (BdtqExgPCKvQCPGHQhsRbzeBepwrvk == BdtqExgPCKvQCPGHQhsRbzeBepwrvk- 1 ) BdtqExgPCKvQCPGHQhsRbzeBepwrvk=727338392; else BdtqExgPCKvQCPGHQhsRbzeBepwrvk=1348246490;if (BdtqExgPCKvQCPGHQhsRbzeBepwrvk == BdtqExgPCKvQCPGHQhsRbzeBepwrvk- 1 ) BdtqExgPCKvQCPGHQhsRbzeBepwrvk=239237977; else BdtqExgPCKvQCPGHQhsRbzeBepwrvk=1089583472;if (BdtqExgPCKvQCPGHQhsRbzeBepwrvk == BdtqExgPCKvQCPGHQhsRbzeBepwrvk- 1 ) BdtqExgPCKvQCPGHQhsRbzeBepwrvk=448998958; else BdtqExgPCKvQCPGHQhsRbzeBepwrvk=1699237589;if (BdtqExgPCKvQCPGHQhsRbzeBepwrvk == BdtqExgPCKvQCPGHQhsRbzeBepwrvk- 1 ) BdtqExgPCKvQCPGHQhsRbzeBepwrvk=2058367940; else BdtqExgPCKvQCPGHQhsRbzeBepwrvk=80483047;if (BdtqExgPCKvQCPGHQhsRbzeBepwrvk == BdtqExgPCKvQCPGHQhsRbzeBepwrvk- 0 ) BdtqExgPCKvQCPGHQhsRbzeBepwrvk=718966492; else BdtqExgPCKvQCPGHQhsRbzeBepwrvk=694939415;double yVmqhmtxeJFuZXxqDQgIeHzJgseVvy=1951649833.518935800578277714560715355165;if (yVmqhmtxeJFuZXxqDQgIeHzJgseVvy == yVmqhmtxeJFuZXxqDQgIeHzJgseVvy ) yVmqhmtxeJFuZXxqDQgIeHzJgseVvy=233952542.235140729866477221010246280379; else yVmqhmtxeJFuZXxqDQgIeHzJgseVvy=326994015.074645423809570535914416509097;if (yVmqhmtxeJFuZXxqDQgIeHzJgseVvy == yVmqhmtxeJFuZXxqDQgIeHzJgseVvy ) yVmqhmtxeJFuZXxqDQgIeHzJgseVvy=361517218.046090293390931287685556666914; else yVmqhmtxeJFuZXxqDQgIeHzJgseVvy=1600991127.643787369135263469643342259153;if (yVmqhmtxeJFuZXxqDQgIeHzJgseVvy == yVmqhmtxeJFuZXxqDQgIeHzJgseVvy ) yVmqhmtxeJFuZXxqDQgIeHzJgseVvy=1483883799.448075826492856140330848141263; else yVmqhmtxeJFuZXxqDQgIeHzJgseVvy=350354632.831779829975325159712083371004;if (yVmqhmtxeJFuZXxqDQgIeHzJgseVvy == yVmqhmtxeJFuZXxqDQgIeHzJgseVvy ) yVmqhmtxeJFuZXxqDQgIeHzJgseVvy=1355443031.566466229742156652409255296940; else yVmqhmtxeJFuZXxqDQgIeHzJgseVvy=2092046412.983541906004108093782970632128;if (yVmqhmtxeJFuZXxqDQgIeHzJgseVvy == yVmqhmtxeJFuZXxqDQgIeHzJgseVvy ) yVmqhmtxeJFuZXxqDQgIeHzJgseVvy=1664049514.139996961676751626820326802401; else yVmqhmtxeJFuZXxqDQgIeHzJgseVvy=22535057.088997522224222723002641992419;if (yVmqhmtxeJFuZXxqDQgIeHzJgseVvy == yVmqhmtxeJFuZXxqDQgIeHzJgseVvy ) yVmqhmtxeJFuZXxqDQgIeHzJgseVvy=1250625398.567179896410570711428950045701; else yVmqhmtxeJFuZXxqDQgIeHzJgseVvy=1542510579.621376726304760227128100172713;float ErcowxIAVwtbILWLrCoJnepMYLDKBY=1306186753.432764371392243244319639031408f;if (ErcowxIAVwtbILWLrCoJnepMYLDKBY - ErcowxIAVwtbILWLrCoJnepMYLDKBY> 0.00000001 ) ErcowxIAVwtbILWLrCoJnepMYLDKBY=1291557078.420976723518421438037187991374f; else ErcowxIAVwtbILWLrCoJnepMYLDKBY=2109082609.308407248752290121970357135418f;if (ErcowxIAVwtbILWLrCoJnepMYLDKBY - ErcowxIAVwtbILWLrCoJnepMYLDKBY> 0.00000001 ) ErcowxIAVwtbILWLrCoJnepMYLDKBY=1625638529.645022113420138600787041415247f; else ErcowxIAVwtbILWLrCoJnepMYLDKBY=1062412739.283012727912855122964684261572f;if (ErcowxIAVwtbILWLrCoJnepMYLDKBY - ErcowxIAVwtbILWLrCoJnepMYLDKBY> 0.00000001 ) ErcowxIAVwtbILWLrCoJnepMYLDKBY=281049081.798635722798388639739448471096f; else ErcowxIAVwtbILWLrCoJnepMYLDKBY=2064350474.136532533140127630764284446026f;if (ErcowxIAVwtbILWLrCoJnepMYLDKBY - ErcowxIAVwtbILWLrCoJnepMYLDKBY> 0.00000001 ) ErcowxIAVwtbILWLrCoJnepMYLDKBY=521285023.574777296620770932284143395329f; else ErcowxIAVwtbILWLrCoJnepMYLDKBY=545774715.751777746155515462551595344258f;if (ErcowxIAVwtbILWLrCoJnepMYLDKBY - ErcowxIAVwtbILWLrCoJnepMYLDKBY> 0.00000001 ) ErcowxIAVwtbILWLrCoJnepMYLDKBY=607122523.208797001330714063994497937653f; else ErcowxIAVwtbILWLrCoJnepMYLDKBY=1395206101.450996783847133288239146492197f;if (ErcowxIAVwtbILWLrCoJnepMYLDKBY - ErcowxIAVwtbILWLrCoJnepMYLDKBY> 0.00000001 ) ErcowxIAVwtbILWLrCoJnepMYLDKBY=490940056.145998092839115836575953442451f; else ErcowxIAVwtbILWLrCoJnepMYLDKBY=1295332315.579681234520640103276672329355f;double dTHTgwaSXfzJxZXOJMSStOpYuBjRaH=1799538329.647485307215827125257854709727;if (dTHTgwaSXfzJxZXOJMSStOpYuBjRaH == dTHTgwaSXfzJxZXOJMSStOpYuBjRaH ) dTHTgwaSXfzJxZXOJMSStOpYuBjRaH=779999794.444526744176616410126614990478; else dTHTgwaSXfzJxZXOJMSStOpYuBjRaH=379360015.769088240258218134840071293073;if (dTHTgwaSXfzJxZXOJMSStOpYuBjRaH == dTHTgwaSXfzJxZXOJMSStOpYuBjRaH ) dTHTgwaSXfzJxZXOJMSStOpYuBjRaH=234268912.638652864938007850687664970990; else dTHTgwaSXfzJxZXOJMSStOpYuBjRaH=167790468.527762167747102307445449718588;if (dTHTgwaSXfzJxZXOJMSStOpYuBjRaH == dTHTgwaSXfzJxZXOJMSStOpYuBjRaH ) dTHTgwaSXfzJxZXOJMSStOpYuBjRaH=1190907436.308441144689405067378355518806; else dTHTgwaSXfzJxZXOJMSStOpYuBjRaH=1654042289.579945076127387615586121115393;if (dTHTgwaSXfzJxZXOJMSStOpYuBjRaH == dTHTgwaSXfzJxZXOJMSStOpYuBjRaH ) dTHTgwaSXfzJxZXOJMSStOpYuBjRaH=995707419.334922156119225107901443145894; else dTHTgwaSXfzJxZXOJMSStOpYuBjRaH=620583482.088389447656958701701850309198;if (dTHTgwaSXfzJxZXOJMSStOpYuBjRaH == dTHTgwaSXfzJxZXOJMSStOpYuBjRaH ) dTHTgwaSXfzJxZXOJMSStOpYuBjRaH=1590289666.747213105686145550645529434909; else dTHTgwaSXfzJxZXOJMSStOpYuBjRaH=782775627.146992074560712681585056433899;if (dTHTgwaSXfzJxZXOJMSStOpYuBjRaH == dTHTgwaSXfzJxZXOJMSStOpYuBjRaH ) dTHTgwaSXfzJxZXOJMSStOpYuBjRaH=1831680623.030665126469729581885343419166; else dTHTgwaSXfzJxZXOJMSStOpYuBjRaH=216702920.039654115867622744705211489735;int zeySPzEjYUoexOuhMCkmZRfDpLkKka=1112720794;if (zeySPzEjYUoexOuhMCkmZRfDpLkKka == zeySPzEjYUoexOuhMCkmZRfDpLkKka- 1 ) zeySPzEjYUoexOuhMCkmZRfDpLkKka=384358592; else zeySPzEjYUoexOuhMCkmZRfDpLkKka=1560263386;if (zeySPzEjYUoexOuhMCkmZRfDpLkKka == zeySPzEjYUoexOuhMCkmZRfDpLkKka- 1 ) zeySPzEjYUoexOuhMCkmZRfDpLkKka=1550865984; else zeySPzEjYUoexOuhMCkmZRfDpLkKka=1808135111;if (zeySPzEjYUoexOuhMCkmZRfDpLkKka == zeySPzEjYUoexOuhMCkmZRfDpLkKka- 0 ) zeySPzEjYUoexOuhMCkmZRfDpLkKka=1930023061; else zeySPzEjYUoexOuhMCkmZRfDpLkKka=1236493046;if (zeySPzEjYUoexOuhMCkmZRfDpLkKka == zeySPzEjYUoexOuhMCkmZRfDpLkKka- 1 ) zeySPzEjYUoexOuhMCkmZRfDpLkKka=1670600752; else zeySPzEjYUoexOuhMCkmZRfDpLkKka=317085715;if (zeySPzEjYUoexOuhMCkmZRfDpLkKka == zeySPzEjYUoexOuhMCkmZRfDpLkKka- 1 ) zeySPzEjYUoexOuhMCkmZRfDpLkKka=1953192639; else zeySPzEjYUoexOuhMCkmZRfDpLkKka=1963226150;if (zeySPzEjYUoexOuhMCkmZRfDpLkKka == zeySPzEjYUoexOuhMCkmZRfDpLkKka- 1 ) zeySPzEjYUoexOuhMCkmZRfDpLkKka=993868604; else zeySPzEjYUoexOuhMCkmZRfDpLkKka=88677698;float pOyoUPXqKwIWXAdtfHKRwzKOkpZCis=380213375.839792424720184940071352576626f;if (pOyoUPXqKwIWXAdtfHKRwzKOkpZCis - pOyoUPXqKwIWXAdtfHKRwzKOkpZCis> 0.00000001 ) pOyoUPXqKwIWXAdtfHKRwzKOkpZCis=382123916.448892037655334962013318659732f; else pOyoUPXqKwIWXAdtfHKRwzKOkpZCis=1598570716.790849249391353613961462314135f;if (pOyoUPXqKwIWXAdtfHKRwzKOkpZCis - pOyoUPXqKwIWXAdtfHKRwzKOkpZCis> 0.00000001 ) pOyoUPXqKwIWXAdtfHKRwzKOkpZCis=762510785.331596650683373316505798234963f; else pOyoUPXqKwIWXAdtfHKRwzKOkpZCis=573883176.622506186706339829911863174447f;if (pOyoUPXqKwIWXAdtfHKRwzKOkpZCis - pOyoUPXqKwIWXAdtfHKRwzKOkpZCis> 0.00000001 ) pOyoUPXqKwIWXAdtfHKRwzKOkpZCis=711378234.380716298723190047942505296954f; else pOyoUPXqKwIWXAdtfHKRwzKOkpZCis=1675817634.375053663373239978324850688663f;if (pOyoUPXqKwIWXAdtfHKRwzKOkpZCis - pOyoUPXqKwIWXAdtfHKRwzKOkpZCis> 0.00000001 ) pOyoUPXqKwIWXAdtfHKRwzKOkpZCis=115279317.135455914848478948642974053557f; else pOyoUPXqKwIWXAdtfHKRwzKOkpZCis=1784998436.501124988517458648375112237240f;if (pOyoUPXqKwIWXAdtfHKRwzKOkpZCis - pOyoUPXqKwIWXAdtfHKRwzKOkpZCis> 0.00000001 ) pOyoUPXqKwIWXAdtfHKRwzKOkpZCis=1113375190.347735745406024552641528350090f; else pOyoUPXqKwIWXAdtfHKRwzKOkpZCis=1005055047.884827073300308910878560982059f;if (pOyoUPXqKwIWXAdtfHKRwzKOkpZCis - pOyoUPXqKwIWXAdtfHKRwzKOkpZCis> 0.00000001 ) pOyoUPXqKwIWXAdtfHKRwzKOkpZCis=1619606720.685663230758035233145023242495f; else pOyoUPXqKwIWXAdtfHKRwzKOkpZCis=1645757101.513483585588012153686091466342f;int nRaLUbeaKfYqdihzCFHuusEvbKIAMj=1700634144;if (nRaLUbeaKfYqdihzCFHuusEvbKIAMj == nRaLUbeaKfYqdihzCFHuusEvbKIAMj- 0 ) nRaLUbeaKfYqdihzCFHuusEvbKIAMj=649211703; else nRaLUbeaKfYqdihzCFHuusEvbKIAMj=1525702909;if (nRaLUbeaKfYqdihzCFHuusEvbKIAMj == nRaLUbeaKfYqdihzCFHuusEvbKIAMj- 0 ) nRaLUbeaKfYqdihzCFHuusEvbKIAMj=910973723; else nRaLUbeaKfYqdihzCFHuusEvbKIAMj=238825553;if (nRaLUbeaKfYqdihzCFHuusEvbKIAMj == nRaLUbeaKfYqdihzCFHuusEvbKIAMj- 0 ) nRaLUbeaKfYqdihzCFHuusEvbKIAMj=1880453802; else nRaLUbeaKfYqdihzCFHuusEvbKIAMj=2127243324;if (nRaLUbeaKfYqdihzCFHuusEvbKIAMj == nRaLUbeaKfYqdihzCFHuusEvbKIAMj- 1 ) nRaLUbeaKfYqdihzCFHuusEvbKIAMj=2016998915; else nRaLUbeaKfYqdihzCFHuusEvbKIAMj=1690990395;if (nRaLUbeaKfYqdihzCFHuusEvbKIAMj == nRaLUbeaKfYqdihzCFHuusEvbKIAMj- 0 ) nRaLUbeaKfYqdihzCFHuusEvbKIAMj=856242910; else nRaLUbeaKfYqdihzCFHuusEvbKIAMj=2064666844;if (nRaLUbeaKfYqdihzCFHuusEvbKIAMj == nRaLUbeaKfYqdihzCFHuusEvbKIAMj- 1 ) nRaLUbeaKfYqdihzCFHuusEvbKIAMj=362081476; else nRaLUbeaKfYqdihzCFHuusEvbKIAMj=1375164933;int dQXmwXwwrCnFCXzNYDvlVvHUqgTpCX=1530178950;if (dQXmwXwwrCnFCXzNYDvlVvHUqgTpCX == dQXmwXwwrCnFCXzNYDvlVvHUqgTpCX- 1 ) dQXmwXwwrCnFCXzNYDvlVvHUqgTpCX=489818154; else dQXmwXwwrCnFCXzNYDvlVvHUqgTpCX=1451467248;if (dQXmwXwwrCnFCXzNYDvlVvHUqgTpCX == dQXmwXwwrCnFCXzNYDvlVvHUqgTpCX- 1 ) dQXmwXwwrCnFCXzNYDvlVvHUqgTpCX=1982062651; else dQXmwXwwrCnFCXzNYDvlVvHUqgTpCX=387871005;if (dQXmwXwwrCnFCXzNYDvlVvHUqgTpCX == dQXmwXwwrCnFCXzNYDvlVvHUqgTpCX- 1 ) dQXmwXwwrCnFCXzNYDvlVvHUqgTpCX=291461241; else dQXmwXwwrCnFCXzNYDvlVvHUqgTpCX=172111222;if (dQXmwXwwrCnFCXzNYDvlVvHUqgTpCX == dQXmwXwwrCnFCXzNYDvlVvHUqgTpCX- 1 ) dQXmwXwwrCnFCXzNYDvlVvHUqgTpCX=401342560; else dQXmwXwwrCnFCXzNYDvlVvHUqgTpCX=856024605;if (dQXmwXwwrCnFCXzNYDvlVvHUqgTpCX == dQXmwXwwrCnFCXzNYDvlVvHUqgTpCX- 1 ) dQXmwXwwrCnFCXzNYDvlVvHUqgTpCX=1444984069; else dQXmwXwwrCnFCXzNYDvlVvHUqgTpCX=1913351195;if (dQXmwXwwrCnFCXzNYDvlVvHUqgTpCX == dQXmwXwwrCnFCXzNYDvlVvHUqgTpCX- 0 ) dQXmwXwwrCnFCXzNYDvlVvHUqgTpCX=694759921; else dQXmwXwwrCnFCXzNYDvlVvHUqgTpCX=924968014;long vPsQAubuBcSlWgMnKBWZSRhNTlLJcT=1648610536;if (vPsQAubuBcSlWgMnKBWZSRhNTlLJcT == vPsQAubuBcSlWgMnKBWZSRhNTlLJcT- 1 ) vPsQAubuBcSlWgMnKBWZSRhNTlLJcT=1256409509; else vPsQAubuBcSlWgMnKBWZSRhNTlLJcT=1440718107;if (vPsQAubuBcSlWgMnKBWZSRhNTlLJcT == vPsQAubuBcSlWgMnKBWZSRhNTlLJcT- 0 ) vPsQAubuBcSlWgMnKBWZSRhNTlLJcT=1586949642; else vPsQAubuBcSlWgMnKBWZSRhNTlLJcT=481195314;if (vPsQAubuBcSlWgMnKBWZSRhNTlLJcT == vPsQAubuBcSlWgMnKBWZSRhNTlLJcT- 1 ) vPsQAubuBcSlWgMnKBWZSRhNTlLJcT=1296620640; else vPsQAubuBcSlWgMnKBWZSRhNTlLJcT=54131920;if (vPsQAubuBcSlWgMnKBWZSRhNTlLJcT == vPsQAubuBcSlWgMnKBWZSRhNTlLJcT- 0 ) vPsQAubuBcSlWgMnKBWZSRhNTlLJcT=647243220; else vPsQAubuBcSlWgMnKBWZSRhNTlLJcT=402364239;if (vPsQAubuBcSlWgMnKBWZSRhNTlLJcT == vPsQAubuBcSlWgMnKBWZSRhNTlLJcT- 0 ) vPsQAubuBcSlWgMnKBWZSRhNTlLJcT=1729055448; else vPsQAubuBcSlWgMnKBWZSRhNTlLJcT=1219004037;if (vPsQAubuBcSlWgMnKBWZSRhNTlLJcT == vPsQAubuBcSlWgMnKBWZSRhNTlLJcT- 0 ) vPsQAubuBcSlWgMnKBWZSRhNTlLJcT=1865123913; else vPsQAubuBcSlWgMnKBWZSRhNTlLJcT=794195037;int EJShTqlORETxalzljEIVHXbACGpKaP=206345469;if (EJShTqlORETxalzljEIVHXbACGpKaP == EJShTqlORETxalzljEIVHXbACGpKaP- 0 ) EJShTqlORETxalzljEIVHXbACGpKaP=243886141; else EJShTqlORETxalzljEIVHXbACGpKaP=89427770;if (EJShTqlORETxalzljEIVHXbACGpKaP == EJShTqlORETxalzljEIVHXbACGpKaP- 0 ) EJShTqlORETxalzljEIVHXbACGpKaP=1155938947; else EJShTqlORETxalzljEIVHXbACGpKaP=928686250;if (EJShTqlORETxalzljEIVHXbACGpKaP == EJShTqlORETxalzljEIVHXbACGpKaP- 0 ) EJShTqlORETxalzljEIVHXbACGpKaP=470003099; else EJShTqlORETxalzljEIVHXbACGpKaP=318945636;if (EJShTqlORETxalzljEIVHXbACGpKaP == EJShTqlORETxalzljEIVHXbACGpKaP- 0 ) EJShTqlORETxalzljEIVHXbACGpKaP=1409172903; else EJShTqlORETxalzljEIVHXbACGpKaP=477100481;if (EJShTqlORETxalzljEIVHXbACGpKaP == EJShTqlORETxalzljEIVHXbACGpKaP- 0 ) EJShTqlORETxalzljEIVHXbACGpKaP=1124360349; else EJShTqlORETxalzljEIVHXbACGpKaP=54949118;if (EJShTqlORETxalzljEIVHXbACGpKaP == EJShTqlORETxalzljEIVHXbACGpKaP- 1 ) EJShTqlORETxalzljEIVHXbACGpKaP=941325131; else EJShTqlORETxalzljEIVHXbACGpKaP=2114586254;long xgRhLlDdbafEZCptWwzxELaoIzopTp=831399259;if (xgRhLlDdbafEZCptWwzxELaoIzopTp == xgRhLlDdbafEZCptWwzxELaoIzopTp- 0 ) xgRhLlDdbafEZCptWwzxELaoIzopTp=1980250980; else xgRhLlDdbafEZCptWwzxELaoIzopTp=860954587;if (xgRhLlDdbafEZCptWwzxELaoIzopTp == xgRhLlDdbafEZCptWwzxELaoIzopTp- 1 ) xgRhLlDdbafEZCptWwzxELaoIzopTp=118755424; else xgRhLlDdbafEZCptWwzxELaoIzopTp=1366939225;if (xgRhLlDdbafEZCptWwzxELaoIzopTp == xgRhLlDdbafEZCptWwzxELaoIzopTp- 1 ) xgRhLlDdbafEZCptWwzxELaoIzopTp=1467563726; else xgRhLlDdbafEZCptWwzxELaoIzopTp=279973592;if (xgRhLlDdbafEZCptWwzxELaoIzopTp == xgRhLlDdbafEZCptWwzxELaoIzopTp- 0 ) xgRhLlDdbafEZCptWwzxELaoIzopTp=233558310; else xgRhLlDdbafEZCptWwzxELaoIzopTp=435686234;if (xgRhLlDdbafEZCptWwzxELaoIzopTp == xgRhLlDdbafEZCptWwzxELaoIzopTp- 0 ) xgRhLlDdbafEZCptWwzxELaoIzopTp=804224028; else xgRhLlDdbafEZCptWwzxELaoIzopTp=1579698412;if (xgRhLlDdbafEZCptWwzxELaoIzopTp == xgRhLlDdbafEZCptWwzxELaoIzopTp- 0 ) xgRhLlDdbafEZCptWwzxELaoIzopTp=806299204; else xgRhLlDdbafEZCptWwzxELaoIzopTp=1219940851; }
 xgRhLlDdbafEZCptWwzxELaoIzopTpy::xgRhLlDdbafEZCptWwzxELaoIzopTpy()
 { this->ZFyEBBRrlIHw("IkpxjYHftkwVinIPdLcjFHeplFdZMwZFyEBBRrlIHwj", true, 454470956, 1754631170, 1689772094); }
#pragma optimize("", off)
 // <delete/>

