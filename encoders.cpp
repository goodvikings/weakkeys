#include <cstdlib>
#include "encoders.h"

static const unsigned char hexEncodingList[] = "0123456789abcdef";
static unsigned char encoding_table[] = {'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H',
                                'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P',
                                'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X',
                                'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f',
                                'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n',
                                'o', 'p', 'q', 'r', 's', 't', 'u', 'v',
                                'w', 'x', 'y', 'z', '0', '1', '2', '3',
                                '4', '5', '6', '7', '8', '9', '+', '/'};
static unsigned char decoding_table[64] = "";
static int mod_table[] = {0, 2, 1};

unsigned char convertHexToByte(const unsigned char a, const unsigned char b)
{
	unsigned char retVal = 0;
	if (a >= '0' && a <= '9')
	{
		retVal = (a - '0') << 4;
	}
	else if (a >= 'a' && a <= 'f')
	{
		retVal = (a - 'a'  + 10) << 4;
	}
	else if (a >= 'A' && a <= 'F')
	{
		retVal = (a - 'A' + 10) << 4;
	} else {
		return -1;
	}

	if (b >= '0' && b <= '9')
	{
		retVal += b - '0';
	}
	else if (b >= 'a' && b <= 'f')
	{
		retVal += b - 'a' + 10;
	}
	else if (b >= 'A' && b <= 'F')
	{
		retVal += b - 'A' + 10;
	} else {
		return -1;
	}
	return retVal;
}

void build_decoding_table()
{
    for (int i = 0; i < 64; i++)
        decoding_table[(unsigned char) encoding_table[i]] = i;
}

void to_hex(const unsigned char* source, unsigned char** dest, const unsigned int sourceLen, unsigned int* destLen)
{
	*destLen = sourceLen * 2;
	*dest = new unsigned char[*destLen + 1];

	for (unsigned int i = 0; i < sourceLen; i++)
	{
		(*dest)[i * 2] = hexEncodingList[source[i] >> 4];
		(*dest)[i * 2 + 1] = hexEncodingList[source[i] & 0x0f];
	}
	(*dest)[*destLen] = 0;
}

void from_hex(const unsigned char* source, unsigned char** dest, const unsigned int sourceLen, unsigned int* destLen)
{
	*destLen = sourceLen / 2;
	*dest = new unsigned char[*destLen];

	for (unsigned int i = 0; i < sourceLen; i += 2)
	{
		(*dest)[i / 2] = convertHexToByte(source[i], source[i + 1]);
	}
}

void to_base64(const unsigned char* source, unsigned char** dest, const unsigned int sourceLen, unsigned int* destLen)
{
	*destLen = 4 * ((sourceLen + 2) / 3);
	*dest = new unsigned char[*destLen];

	for (unsigned int i = 0, j = 0; i < sourceLen;)
	{
		unsigned int octet_a = i < sourceLen ? (unsigned char)source[i++] : 0;
        	unsigned int octet_b = i < sourceLen ? (unsigned char)source[i++] : 0;
        	unsigned int octet_c = i < sourceLen ? (unsigned char)source[i++] : 0;

        	unsigned int triple = (octet_a << 0x10) + (octet_b << 0x08) + octet_c;

        	(*dest)[j++] = encoding_table[(triple >> 3 * 6) & 0x3F];
        	(*dest)[j++] = encoding_table[(triple >> 2 * 6) & 0x3F];
        	(*dest)[j++] = encoding_table[(triple >> 1 * 6) & 0x3F];
        	(*dest)[j++] = encoding_table[(triple >> 0 * 6) & 0x3F];
	}
	for (int i = 0; i < mod_table[sourceLen % 3]; i++)
        	(*dest)[*destLen - 1 - i] = '=';
}

void from_base64(const unsigned char* source, unsigned char** dest, const unsigned int sourceLen, unsigned int* destLen)
{
	if (decoding_table[0] == 0) build_decoding_table();

	*destLen = sourceLen / 4 * 3;
	if (source[sourceLen - 1] == '=') (*destLen)--;
	if (source[sourceLen - 2] == '=') (*destLen)--;

	*dest = new unsigned char[*destLen];

	for (unsigned int i = 0, j = 0; i < sourceLen;)
	{
		unsigned int sextet_a = source[i] == '=' ? 0 & i++ : decoding_table[source[i++]];
		unsigned int sextet_b = source[i] == '=' ? 0 & i++ : decoding_table[source[i++]];
		unsigned int sextet_c = source[i] == '=' ? 0 & i++ : decoding_table[source[i++]];
		unsigned int sextet_d = source[i] == '=' ? 0 & i++ : decoding_table[source[i++]];

		unsigned int triple = (sextet_a << 3 * 6) + (sextet_b << 2 * 6) + (sextet_c << 1 * 6) + (sextet_d << 0 * 6);

		if (j < *destLen) (*dest)[j++] = (triple >> 2 * 8) & 0xFF;
		if (j < *destLen) (*dest)[j++] = (triple >> 1 * 8) & 0xFF;
		if (j < *destLen) (*dest)[j++] = (triple >> 0 * 8) & 0xFF;
	}
}
