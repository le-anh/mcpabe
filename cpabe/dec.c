// gcc ../symmetric/pairing.c ../symmetric/miller.c ../symmetric/efp12_symmetric.c utilities.c hash_map.c tree.c setup.c key_gen.c encrypt.c decrypt.c test.c -o test.out -lgmp -lelips -I/opt/ssl/include/ -L/opt/ssl/lib/ -lcrypto

// gcc ./cpabe/dec.c ./cpabe/common.c ./cpabe/policy_lang.c ./libbswabe-0.9/core.c ./libbswabe-0.9/misc.c -o dec.out `pkg-config --cflags glib-2.0` `pkg-config --libs glib-2.0` -lgmp /usr/local/include/mcl/libmcl.a -lcrypto -lstdc++ -O3 -w

// ./dec.out pub_key lhanh_priv_key plaintext.txt.cpabe

#include <assert.h>
#include <string.h>
#include <unistd.h>
#include <glib.h>
// #include <pbc.h>
// #include <pbc_random.h>
#include <mcl/bn_c384_256.h>

#include "bswabe.h"
#include "common.h"

#ifndef PACKAGE_NAME
#define PACKAGE_NAME "cpabe"
#endif

#ifndef PACKAGE_VERSION
#define PACKAGE_VERSION "0.1"
#endif

char *usage =
	"Usage: cpabe-dec [OPTION ...] PUB_KEY PRIV_KEY FILE\n"
	"\n"
	"Decrypt FILE using private key PRIV_KEY and assuming public key\n"
	"PUB_KEY. If the name of FILE is X.cpabe, the decrypted file will\n"
	"be written as X and FILE will be removed. Otherwise the file will be\n"
	"decrypted in place. Use of the -o option overrides this\n"
	"behavior.\n"
	"\n"
	"Mandatory arguments to long options are mandatory for short options too.\n\n"
	" -h, --help               print this message\n\n"
	" -v, --version            print version information\n\n"
	" -k, --keep-input-file    don't delete original file\n\n"
	" -o, --output FILE        write output to FILE\n\n"
	" -d, --deterministic      use deterministic \"random\" numbers\n"
	"                          (only for debugging)\n\n"
	/* " -s, --no-opt-sat         pick an arbitrary way of satisfying the policy\n" */
	/* "                          (only for performance comparison)\n\n" */
	/* " -n, --naive-dec          use slower decryption algorithm\n" */
	/* "                          (only for performance comparison)\n\n" */
	/* " -f, --flatten            use slightly different decryption algorithm\n" */
	/* "                          (may result in higher or lower performance)\n\n" */
	/* " -r, --report-ops         report numbers of group operations\n" */
	/* "                          (only for performance evaluation)\n\n" */
	"";

/* enum { */
/* 	DEC_NAIVE, */
/* 	DEC_FLATTEN, */
/* 	DEC_MERGE, */
/* } dec_strategy = DEC_MERGE;		 */

char *pub_file = 0;
char *prv_file = 0;
char *in_file = 0;
char *out_file = 0;
/* int   no_opt_sat = 0; */
/* int   report_ops = 0; */
int keep = 0;

/* int num_pairings = 0; */
/* int num_exps     = 0; */
/* int num_muls     = 0; */

void parse_args(int argc, char **argv)
{
	int i;

	for (i = 1; i < argc; i++)
		if (!strcmp(argv[i], "-h") || !strcmp(argv[i], "--help"))
		{
			printf("%s", usage);
			exit(0);
		}
		else if (!strcmp(argv[i], "-v") || !strcmp(argv[i], "--version"))
		{
			printf(CPABE_VERSION, "-dec");
			exit(0);
		}
		else if (!strcmp(argv[i], "-k") || !strcmp(argv[i], "--keep-input-file"))
		{
			keep = 1;
		}
		else if (!strcmp(argv[i], "-o") || !strcmp(argv[i], "--output"))
		{
			if (++i >= argc)
				die(usage);
			else
				out_file = argv[i];
		}
		else if (!strcmp(argv[i], "-d") || !strcmp(argv[i], "--deterministic"))
		{
			// pbc_random_set_deterministic(0);
		}
		/* 		else if( !strcmp(argv[i], "-s") || !strcmp(argv[i], "--no-opt-sat") ) */
		/* 		{ */
		/* 			no_opt_sat = 1; */
		/* 		} */
		/* 		else if( !strcmp(argv[i], "-n") || !strcmp(argv[i], "--naive-dec") ) */
		/* 		{ */
		/* 			dec_strategy = DEC_NAIVE; */
		/* 		} */
		/* 		else if( !strcmp(argv[i], "-f") || !strcmp(argv[i], "--flatten") ) */
		/* 		{ */
		/* 			dec_strategy = DEC_FLATTEN; */
		/* 		} */
		/* 		else if( !strcmp(argv[i], "-r") || !strcmp(argv[i], "--report-ops") ) */
		/* 		{ */
		/* 			report_ops = 1; */
		/* 		} */
		else if (!pub_file)
		{
			pub_file = argv[i];
		}
		else if (!prv_file)
		{
			prv_file = argv[i];
		}
		else if (!in_file)
		{
			in_file = argv[i];
		}
		else
			die(usage);

	if (!pub_file || !prv_file || !in_file)
		die(usage);

	if (!out_file)
	{
		if (strlen(in_file) > 6 &&
			!strcmp(in_file + strlen(in_file) - 6, ".cpabe"))
			out_file = g_strndup(in_file, strlen(in_file) - 6);
		else
			out_file = strdup(in_file);
	}

	if (keep && !strcmp(in_file, out_file))
		die("cannot keep input file when decrypting file in place (try -o)\n");
}

int main(int argc, char **argv)
{
	bswabe_pub_t *pub;
	bswabe_prv_t *prv;
	int file_len;
	GByteArray *aes_buf;
	GByteArray *plt;
	GByteArray *cph_buf;
	bswabe_cph_t *cph;
	mclBnGT m;

	parse_args(argc, argv);

	int ret = mclBn_init(MCL_BLS12_381, MCLBN_COMPILED_TIME_VAR);
	if (ret != 0)
	{
		printf("err ret=%d\n", ret);
		exit(1);
	}

	pub = bswabe_pub_unserialize(suck_file(pub_file), 1);
	prv = bswabe_prv_unserialize(pub, suck_file(prv_file), 1);

	read_cpabe_file(in_file, &cph_buf, &file_len, &aes_buf);

	cph = bswabe_cph_unserialize(pub, cph_buf, 1);
	if (!bswabe_dec(pub, prv, cph, &m))
		die("%s", bswabe_error());
	bswabe_cph_free(cph);

	char buf[1600];
	// strcpy(buf, "155b3a33ef5dcb41b23d535d5af275384866424abc54b66b95749089ce44f453b9bc57846042a2c74b8fb4089f5e1c6 1053f1f68e51a994f0bb217ca8c942343f4985f408bb7bb17366a1741feedb714bc716e78b92c883a9f6eb26a1649f82 1426639b7f354214c4957cf2a47093e3675990d8311486d506d2fea79e351dca89f8667e3a729d101feb555ab202dfc7 8be727f3f05cf85a832d049faa17d2c386205ce9c7b1b457bc725602c8c41547629ea6ced0770334abe0763c3bfcf34 13a1a63a1377191d93930a8a01c327942f6f27a89d4b667839fe259c392a9c9655a9f23ddea4e28e16807bcc12f67d6d 13e4b8ad2d179e69ff26b647adb288b8a27664b446719a8da5cfa0c896c374a904b56dc7af8a8917711c71a1c8286e89 1175c23f09147f9a094b6c26866b11a1b58e60a7689d02170dde797883b611ebeedab83b28f970a28ef161059eb97e3f 17312c904b92bee139ccaf3236bb8a33bb1752f26be4a467d54a07b166eeb04a2f858d247ff4bd495ee63fed44c46eee 4d0841c02659554759c09adc2a86fa9ecc4d92c51b6e7a9b1cd5ca3852901a8a477c582271a4e08bd3f44d220fe8beb 96af794f5e05cbc44affda7635798f3a3130d2139337edfe2d9b39def6875408082db7888112cbb8e4d7c3213e8b8bb 1986323ba82012f929602b378548d2fae632519e6ccce69ce9faf91fe85fdda64d2fbdaaf7cfb553858b5b7c841d2f60 5045784dac7a1143d4d2d47f05058a206177ceb50152a2ca83a00f067892d7426f7db5261e02a70a8f284896655528d ");
	// printf(" dec.c -> aes_128_cbc_decrypt -> buf Str(setStr): %s\n", buf);
	// mclBnGT_clear(&m);
	// mclBnGT_setStr(&m, buf, 1600, 16);
	// mclBnGT_setInt(&m, 16);
	mclBnGT_getStr(buf, 1600, &m, 16);
	printf(" dec.c -> aes_128_cbc_decrypt -> m: %s\n", buf);

	plt = aes_128_cbc_decrypt(aes_buf, m);
	g_byte_array_set_size(plt, file_len);
	g_byte_array_free(aes_buf, 1);

	spit_file(out_file, plt, 1);

	// if (!keep)
	// 	unlink(in_file);

	/* report ops if necessary */
	/* 	if( report_ops ) */
	/* 		printf("pairings:        %5d\n" */
	/* 					 "exponentiations: %5d\n" */
	/* 					 "multiplications: %5d\n", num_pairings, num_exps, num_muls); */

	return 0;
}
