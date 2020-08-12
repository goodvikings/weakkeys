#include <gmp.h>
#include <iostream>
#include <libtasn1.h>
#include <stdio.h>
#include "encoders.h"
using namespace std;

#define E 0x10001

void randomProbablePrime(mpz_t* rop, gmp_randstate_t* rand_state, int bitLength);
void initRand(gmp_randstate_t* rand_state, unsigned long seed_ul);
void printASN1(mpz_t* modulus, mpz_t* pubExp, mpz_t* privExp, mpz_t* p, mpz_t* q, mpz_t* exp1, mpz_t* exp2, mpz_t* coeff);

const asn1_static_node asn1Structure_asn1_tab[] = {
  { "RSAPrivateKey", 536870917, NULL },
  { "version", 1073741827, NULL },
  { "modulus", 1073741827, NULL },
  { "publicExponent", 1073741827, NULL },
  { "privateExponent", 1073741827, NULL },
  { "prime1", 1073741827, NULL },
  { "prime2", 1073741827, NULL },
  { "exponent1", 1073741827, NULL },
  { "exponent2", 1073741827, NULL },
  { "coefficient", 3, NULL },
  { NULL, 0, NULL }
};

int main()
{
	mpz_t p, q, modulus, phi, p_sub_1, q_sub_1, e, d, ex1, ex2, coeff;
	gmp_randstate_t rand_state;

	initRand(&rand_state, time(0));
	
	mpz_init(p);
	mpz_init(q);
	mpz_init(p_sub_1);
	mpz_init(q_sub_1);
	mpz_init(modulus);
	mpz_init(phi);
	mpz_init(d);
	mpz_init(ex1);
	mpz_init(ex2);
	mpz_init(coeff);
	mpz_init_set_ui(e, E);

	randomProbablePrime(&p, &rand_state, 512);
	randomProbablePrime(&q, &rand_state, 512);

	mpz_mul(modulus, p, q);
	mpz_sub_ui(p_sub_1, p, 1);
	mpz_sub_ui(q_sub_1, q, 1);
	mpz_lcm(phi, p_sub_1, q_sub_1);
	mpz_invert(d, e, phi);
	mpz_mod(ex1, d, p_sub_1);
	mpz_mod(ex2, d, q_sub_1);
	mpz_invert(coeff, q, p);

	printASN1(&modulus, &e, &d, &p, &q, &ex1, &ex2, &coeff);

	mpz_clear(p);
	mpz_clear(q);
	mpz_clear(p_sub_1);
	mpz_clear(q_sub_1);
	mpz_clear(modulus);
	mpz_clear(phi);
	mpz_clear(e);
	mpz_clear(d);
	mpz_clear(ex1);
	mpz_clear(ex2);
	mpz_clear(coeff);
	gmp_randclear(rand_state);

	return 0;
}

void initRand(gmp_randstate_t* rand_state, unsigned long seed_ul)
{
	mpz_t seed;
	mpz_init(seed);

	gmp_randinit_mt(*rand_state);
	mpz_import(seed, sizeof(unsigned long), 0, 1, 0, 0, &seed_ul);
	gmp_randseed(*rand_state, seed);

	mpz_clear(seed);
}

void randomProbablePrime(mpz_t* rop, gmp_randstate_t* rand_state, int bitLength)
{
	while (true)
	{
		mpz_urandomb(*rop, *rand_state, bitLength);
		if (mpz_probab_prime_p(*rop, 50))
			break;
	}
}

void printASN1(mpz_t* modulus, mpz_t* pubExp, mpz_t* privExp, mpz_t* p, mpz_t* q, mpz_t* exp1, mpz_t* exp2, mpz_t* coeff)
{
	ASN1_TYPE node = ASN1_TYPE_EMPTY;
	unsigned char* buff = new unsigned char[1024 * 1024];
	size_t len = 0;

	asn1_array2tree(asn1Structure_asn1_tab, &node, NULL);
	
	asn1_write_value(node, "RSAPrivateKey.version", &len, 1);

	mpz_export(buff, &len, 0, 1, 0, 0, *modulus);
	asn1_write_value(node, "RSAPrivateKey.modulus", buff, len);

	mpz_export(buff, &len, 0, 1, 0, 0, *pubExp);
	asn1_write_value(node, "RSAPrivateKey.publicExponent", buff, len);

	mpz_export(buff, &len, 0, 1, 0, 0, *privExp);
	asn1_write_value(node, "RSAPrivateKey.privateExponent", buff, len);

	mpz_export(buff, &len, 0, 1, 0, 0, *p);
	asn1_write_value(node, "RSAPrivateKey.prime1", buff, len);

	mpz_export(buff, &len, 0, 1, 0, 0, *q);
	asn1_write_value(node, "RSAPrivateKey.prime2", buff, len);

	mpz_export(buff, &len, 0, 1, 0, 0, *exp1);
	asn1_write_value(node, "RSAPrivateKey.exponent1", buff, len);

	mpz_export(buff, &len, 0, 1, 0, 0, *exp2);
	asn1_write_value(node, "RSAPrivateKey.exponent2", buff, len);

	mpz_export(buff, &len, 0, 1, 0, 0, *coeff);
	asn1_write_value(node, "RSAPrivateKey.coefficient", buff, len);

	int derLen = 1024 * 1024;
	char errorText[ASN1_MAX_ERROR_DESCRIPTION_SIZE];
	int result = asn1_der_coding(node, "RSAPrivateKey", buff, &derLen, errorText);

	if (result)
	{
		cerr << errorText << endl;
		return;
	}

	unsigned char* b64;
	unsigned int b64Len;

	to_base64(buff, &b64, derLen, &b64Len);

	cout << "-----BEGIN RSA PRIVATE KEY-----\n";
	for (int i = 0; i < b64Len; i++)
	{
		cout << b64[i];
		if (!((i + 1) % 64)) cout << "\n";
	}
	cout << "\n-----END RSA PRIVATE KEY-----" << endl;
	
	delete [] b64;
	delete [] buff;
	asn1_delete_structure(&node);
}
