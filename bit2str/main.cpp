#include <intrin.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include <iostream>
#include <string>
#include <typeinfo>
#include <vector>

/*
型名、16進文字列、2進文字列を表示

int        : 00000000 : 00000000-00000000-00000000-00000000
int        : 00000001 : 00000000-00000000-00000000-00000001
int        : 000000FF : 00000000-00000000-00000000-11111111
int        : 0000FFFF : 00000000-00000000-11111111-11111111
int        : FFFFFFFF : 11111111-11111111-11111111-11111111
int        : 00004321 : 00000000-00000000-01000011-00100001
int        : 87654321 : 10000111-01100101-01000011-00100001
int        : AA550011 : 10101010-01010101-00000000-00010001
__int64    : 00000000-87654321 :
00000000-00000000-00000000-00000000-10000111-01100101-01000011-00100001
__int64    : 00000000-87654321 :
00000000-00000000-00000000-00000000-10000111-01100101-01000011-00100001
__int64    : 00000000-FFFFFFFE :
00000000-00000000-00000000-00000000-11111111-11111111-11111111-11111110
__int64    : FFFFFFFF-FFFFFFFF :
11111111-11111111-11111111-11111111-11111111-11111111-11111111-11111111 union
__m256i :
FFFF1234-A511FFFE-00008001-12345678-FFFFFFFF-84218421-11223344-00000001 :
11111111-11111111-00010010-00110100-10100101-00010001-11111111-11111110-00000000-00000000-10000000-00000001-00010010-00110100-01010110-01111000-11111111-11111111-11111111-11111111-10000100-00100001-10000100-00100001-00010001-00100010-00110011-01000100-00000000-00000000-00000000-00000001

 */
/**
 * 任意の型の値を2進数文字列へへんかんする
 */
template <typename T>
std::string reg_to_bit_string( T reg, const char *splitter = "-" ) {
	int splitter_length = strlen( splitter );
	int buffer_length =
		sizeof( T ) * 8 + 1 + ( ( sizeof( T ) * 8 - 1 ) / 8 ) * splitter_length;
	char *const buffer = static_cast<char *>( _alloca( buffer_length ) );
	buffer[ buffer_length - 1 ] = '\0';
	char *pbuffer = buffer;

	const int64_t mask = 0x01010101'01010101;
	const uint8_t *preg = reinterpret_cast<uint8_t *>( &reg ) + sizeof( T ) - 1;

	bool flag_first = true;
	for ( size_t i = 0; i < sizeof( T ); i++ ) {
		if ( flag_first ) {
			flag_first = false;
		} else {
			strcpy( pbuffer, splitter );
			pbuffer += splitter_length;
		}
		uint64_t v = *preg;
		preg--;

		int64_t b = _pdep_u64( v, mask );
		b += 0x30303030'30303030;
		b = _byteswap_uint64( b );
		// memcpy( pbuffer, &b, sizeof( b ) );
		*reinterpret_cast<int64_t*>(pbuffer) = b;   // memcpy() を使わず、*pbuffer へ b を書き込むことによって b をレジスタのみで完結させる
		pbuffer += sizeof( b );
	}
	return std::string( buffer );
}

/**
 * 任意の値を16進文字列に変換する
 */
template <typename T>
std::string reg_to_hex_string( T reg, int split_spacing = 4,
							   const char *splitter = "-",
							   bool flag_big_endian = true ) {
	const int bytenum = sizeof( T );
	const int splitter_length =
		strlen( reinterpret_cast<const char *>( splitter ) );
	if ( split_spacing > sizeof( T ) ) {
		split_spacing = 0;
	}
	const size_t buffersize =
		( bytenum * 2 ) + 1 +
		( split_spacing > 0
			  ? ( ( bytenum - 1 ) / split_spacing ) * splitter_length
			  : 0 );

	char *const buffer = static_cast<char *>( alloca( buffersize ) );
	char *pbuffer = buffer;

	const uint8_t *p = reinterpret_cast<const uint8_t *>( &reg );
	int pointer_proceeding = 1;
	if ( flag_big_endian ) {
		p += sizeof( T ) - 1;
		pointer_proceeding = -1;
	}

	bool flag_first = true;
	for ( size_t i = 0; i < sizeof( T ); i++ ) {
		uint8_t c = *p;
		p += pointer_proceeding;
		if ( flag_first ) {
			flag_first = false;
		} else {
			if ( split_spacing > 0 && i % split_spacing == 0 ) {
				strcpy( pbuffer, reinterpret_cast<const char *>( splitter ) );
				pbuffer += splitter_length;
			}
		}
		uint8_t a = c >> 4;
		uint8_t base = a < 10 ? '0' : ( 'A' - 10 );
		*pbuffer++ = base + a;
		a = c & 0xf;
		base = a < 10 ? '0' : ( 'A' - 10 );
		*pbuffer++ = base + a;
	}
	*pbuffer = '\0';
	return std::string( buffer );
}

/**
 * 型名、16進文字列、2進文字列を表示
 */
template <typename T>
void print_value_by_bit( T v ) {
	std::string bit_v = reg_to_bit_string( v );
	std::string hex_v = reg_to_hex_string( v );
	printf( "%-10s : %s : %s \n", typeid( v ).name(), hex_v.c_str(), bit_v.c_str() );
}

int main() {
	print_value_by_bit( (char)0 );
	print_value_by_bit( (char)1 );
	print_value_by_bit( (short)1 );
	print_value_by_bit( (int)0 );
	print_value_by_bit( (int)1 );
	print_value_by_bit( (int)0xFF );
	print_value_by_bit( (int)0xFFFF );
	print_value_by_bit( (int)0xFFFF'FFFF );
	print_value_by_bit( (int)0x4321 );
	print_value_by_bit( (int)0x8765'4321 );
	print_value_by_bit( (int)0xAA55'0011 );
	print_value_by_bit( (long long)0x87654321 );
	print_value_by_bit( (long long)0x87654321 );
	print_value_by_bit( (long long)0xFFFFFFFE );
	print_value_by_bit( (long long)0xFFFFFFFF'FFFFFFFF );
	__m256i m =
		_mm256_set_epi32( 0xFFFF'1234, 0xA511'FFFE, 0x0000'8001, 0x1234'5678,
						  -1, 0x8421'8421, 0x1122'3344, 0x0000'00001 );
	print_value_by_bit( m );
}
