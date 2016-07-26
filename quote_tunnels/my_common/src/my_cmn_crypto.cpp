/*
 *  RFC 1521 base64 encoding/decoding
 *
 */

#include "my_cmn_crypto.h"

#if defined(_MSC_VER) && !defined(EFIX64) && !defined(EFI32)
#include <basetsd.h>
typedef UINT32 uint32_t;
#else
#include <inttypes.h>
#endif

#include <openssl/evp.h>
#include <openssl/aes.h>

#define AES256 256
#define OPENSSL_AES_BUFFBLOCK 64

static const unsigned char base64_enc_map[64] =
{
	'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J',
	'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T',
	'U', 'V', 'W', 'X', 'Y', 'Z', 'a', 'b', 'c', 'd',
	'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n',
	'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x',
	'y', 'z', '0', '1', '2', '3', '4', '5', '6', '7',
	'8', '9', '+', '/'
};

static const unsigned char base64_dec_map[128] =
{
	127, 127, 127, 127, 127, 127, 127, 127, 127, 127,
	127, 127, 127, 127, 127, 127, 127, 127, 127, 127,
	127, 127, 127, 127, 127, 127, 127, 127, 127, 127,
	127, 127, 127, 127, 127, 127, 127, 127, 127, 127,
	127, 127, 127,  62, 127, 127, 127,  63,  52,  53,
	 54,  55,  56,  57,  58,  59,  60,  61, 127, 127,
	127,  64, 127, 127, 127,   0,   1,   2,   3,   4,
	  5,   6,   7,   8,   9,  10,  11,  12,  13,  14,
	 15,  16,  17,  18,  19,  20,  21,  22,  23,  24,
	 25, 127, 127, 127, 127, 127, 127,  26,  27,  28,
	 29,  30,  31,  32,  33,  34,  35,  36,  37,  38,
	 39,  40,  41,  42,  43,  44,  45,  46,  47,  48,
	 49,  50,  51, 127, 127, 127, 127, 127
};

static unsigned char key[32] = {2,12,33,87,64,173,9,101};

/*
 * Encode a buffer into base64 format
 */
int
base64_encode(unsigned char *dst, size_t *dlen,
              unsigned char *src, size_t slen)
{
	size_t i, n;
	int C1, C2, C3;
	unsigned char *p;

	if (slen == 0) {
		return 0;
	}

	n = (slen << 3) / 6;

	switch ((slen << 3) - (n * 6)) {
		case  2:
			n += 3;
			break;

		case  4:
			n += 2;
			break;

		default:
			break;
	}

	if (*dlen < n + 1) {
		*dlen = n + 1;
		return ERR_BASE64_BUFFER_TOO_SMALL;
	}

	n = (slen / 3) * 3;

	for (i = 0, p = dst; i < n; i += 3) {
		C1 = *src++;
		C2 = *src++;
		C3 = *src++;

		*p++ = base64_enc_map[(C1 >> 2) & 0x3F];
		*p++ = base64_enc_map[(((C1 &  3) << 4) + (C2 >> 4)) & 0x3F];
		*p++ = base64_enc_map[(((C2 & 15) << 2) + (C3 >> 6)) & 0x3F];
		*p++ = base64_enc_map[C3 & 0x3F];
	}

	if (i < slen) {
		C1 = *src++;
		C2 = ((i + 1) < slen) ? *src++ : 0;

		*p++ = base64_enc_map[(C1 >> 2) & 0x3F];
		*p++ = base64_enc_map[(((C1 & 3) << 4) + (C2 >> 4)) & 0x3F];

		if ((i + 1) < slen) {
			*p++ = base64_enc_map[((C2 & 15) << 2) & 0x3F];
		} else {
			*p++ = '=';
		}

		*p++ = '=';
	}

	*dlen = p - dst;
	*p = 0;

	return 0;
}

/*
 * Decode a base64-formatted buffer
 */
int
base64_decode(unsigned char *dst, size_t *dlen,
              unsigned char *src, size_t slen)
{
	size_t i, n;
	uint32_t j, x;
	unsigned char *p;

	for (i = n = j = 0; i < slen; i++) {
		if ((slen - i) >= 2 && src[i] == '\r' && src[i + 1] == '\n') {
			continue;
		}

		if (src[i] == '\n') {
			continue;
		}

		if (src[i] == '=' && ++j > 2) {
			return ERR_BASE64_INVALID_CHARACTER;
		}

		if (src[i] > 127 || base64_dec_map[src[i]] == 127) {
			return ERR_BASE64_INVALID_CHARACTER;
		}

		if (base64_dec_map[src[i]] < 64 && j != 0) {
			return ERR_BASE64_INVALID_CHARACTER;
		}

		n++;
	}

	if (n == 0) {
		return 0;
	}

	n = ((n * 6) + 7) >> 3;

	if (dst == NULL || *dlen < n) {
		*dlen = n;
		return ERR_BASE64_BUFFER_TOO_SMALL;
	}

	for (j = 3, n = x = 0, p = dst; i > 0; i--, src++) {
		if (*src == '\r' || *src == '\n') {
			continue;
		}

		j -= (base64_dec_map[*src] == 64);
		x  = (x << 6) | ( base64_dec_map[*src] & 0x3F );

		if (++n == 4) {
			n = 0;
			if (j > 0) {
				*p++ = (unsigned char)(x >> 16);
			}

			if (j > 1) {
				*p++ = (unsigned char)(x >>  8);
			}

			if (j > 2) {
				*p++ = (unsigned char)(x);
			}
		}
	}

	*dlen = p - dst;

	return 0;
}

std::string my_cmn::MYEncrypt(const std::string& text) {
	unsigned char m_to[OPENSSL_AES_BUFFBLOCK];
	AES_KEY m_key;

	int len = text.size();
    unsigned char *buf_tmp = new unsigned char[len + 1];

    memset(buf_tmp, 0, len + 1);
    memcpy(buf_tmp, text.data(), len);
    memset(m_to, 0, sizeof(m_to));

    AES_set_encrypt_key(key, AES256, &m_key);
    AES_encrypt((unsigned char *)buf_tmp,m_to,&m_key);

    unsigned char encoded[128];
    memset(encoded, 0, sizeof(encoded));
    size_t dlen = sizeof(encoded);

    base64_encode(encoded, &dlen, m_to, sizeof(m_to));

    return std::string((char*)encoded);

}

std::string my_cmn::MYDecrypt(const std::string& cypher_text) {
	int len = cypher_text.size();
	unsigned char decoded[128];
	memset(decoded, 0, sizeof(decoded));
	size_t dlen = sizeof(decoded);

	base64_decode(decoded, &dlen,
			(unsigned char *) (cypher_text.c_str()), len);
	unsigned char *encrypt_pswd = new unsigned char[len + 1];
	memset(encrypt_pswd, 0, len + 1);
	memcpy(encrypt_pswd, decoded, len);

	unsigned char buffer_tmp[256];
	AES_KEY m_key;
	AES_set_decrypt_key(key, AES256, &m_key);
	AES_decrypt(encrypt_pswd, (unsigned char *) buffer_tmp, &m_key);

	return std::string((char *) (buffer_tmp));
}

#if defined(SELF_TEST)

#include <string.h>
#include <stdio.h>

static const unsigned char base64_test_dec[64] =
{
	0x24, 0x48, 0x6E, 0x56, 0x87, 0x62, 0x5A, 0xBD,
	0xBF, 0x17, 0xD9, 0xA2, 0xC4, 0x17, 0x1A, 0x01,
	0x94, 0xED, 0x8F, 0x1E, 0x11, 0xB3, 0xD7, 0x09,
	0x0C, 0xB6, 0xE9, 0x10, 0x6F, 0x22, 0xEE, 0x13,
	0xCA, 0xB3, 0x07, 0x05, 0x76, 0xC9, 0xFA, 0x31,
	0x6C, 0x08, 0x34, 0xFF, 0x8D, 0xC2, 0x6C, 0x38,
	0x00, 0x43, 0xE9, 0x54, 0x97, 0xAF, 0x50, 0x4B,
	0xD1, 0x41, 0xBA, 0x95, 0x31, 0x5A, 0x0B, 0x97
};

static const unsigned char base64_test_enc[] =
    "JEhuVodiWr2/F9mixBcaAZTtjx4Rs9cJDLbpEG8i7hPK"
    "swcFdsn6MWwINP+Nwmw4AEPpVJevUEvRQbqVMVoLlw==";

/*
 * Checkup routine
 */
int
base64_self_test(int verbose)
{
	size_t len;
	const unsigned char *src;
	unsigned char buffer[128];

	if (verbose != 0) {
		printf("  Base64 encoding test: ");
	}

	len = sizeof(buffer);
	src = base64_test_dec;

	if (base64_encode(buffer, &len, src, 64) != 0 ||
	    memcmp(base64_test_enc, buffer, 88) != 0) {
		if( verbose != 0 ) {
			printf("failed\n");
		}

		return 1;
	}

	if (verbose != 0) {
		printf("passed\n  Base64 decoding test: ");
	}

	len = sizeof(buffer);
	src = base64_test_enc;

	if (base64_decode(buffer, &len, src, 88) != 0 ||
	    memcmp(base64_test_dec, buffer, 64) != 0) {
		if (verbose != 0) {
			printf("failed\n");
		}

		return 1;
	}

	if (verbose != 0) {
		printf("passed\n\n");
	}

	return 0;
}

int
main(int argc, char *argv[])
{
	base64_self_test(1);

	return 0;
}

#endif

