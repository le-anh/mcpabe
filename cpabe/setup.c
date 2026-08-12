// gcc setup.c common.c ../libbswabe-0.9/core.c ../libbswabe-0.9/misc.c -o setup.out `pkg-config --cflags glib-2.0` `pkg-config --libs glib-2.0` -lgmp /usr/local/include/mcl/libmcl.a -lcrypto -lstdc++ -O3 -w

#include <stdio.h>
#include <stdlib.h>
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

char* usage =
"Usage: cpabe-setup [OPTION ...]\n"
"\n"
"Generate system parameters, a public key, and a master secret key\n"
"for use with cpabe-keygen, cpabe-enc, and cpabe-dec.\n"
"\n"
"Output will be written to the files \"pub_key\" and \"master_key\"\n"
"unless the --output-public-key or --output-master-key options are\n"
"used.\n"
"\n"
"Mandatory arguments to long options are mandatory for short options too.\n\n"
" -h, --help                    print this message\n\n"
" -v, --version                 print version information\n\n"
" -p, --output-public-key FILE  write public key to FILE\n\n"
" -m, --output-master-key FILE  write master secret key to FILE\n\n"
" -d, --deterministic           use deterministic \"random\" numbers\n"
"                               (only for debugging)\n\n"
"";

char* pub_file = "pub_key";
char* msk_file = "master_key";

void
parse_args( int argc, char** argv )
{
	int i;

	for( i = 1; i < argc; i++ )
		if(      !strcmp(argv[i], "-h") || !strcmp(argv[i], "--help") )
		{
			printf("%s", usage);
			exit(0);
		}
		else if( !strcmp(argv[i], "-v") || !strcmp(argv[i], "--version") )
		{
			printf(CPABE_VERSION, "-setup");
			exit(0);
		}
		else if( !strcmp(argv[i], "-p") || !strcmp(argv[i], "--output-public-key") )
		{
			if( ++i >= argc )
				die(usage);
			else
				pub_file = argv[i];
		}
		else if( !strcmp(argv[i], "-m") || !strcmp(argv[i], "--output-master-key") )
		{
			if( ++i >= argc )
				die(usage);
			else
				msk_file = argv[i];
		}
		else if( !strcmp(argv[i], "-d") || !strcmp(argv[i], "--deterministic") )
		{
			// pbc_random_set_deterministic(0);
			int ret = mclBn_init(MCL_BLS12_381, MCLBN_COMPILED_TIME_VAR);
			printf("\nmclBn_init ret=%d\n", ret);
			if (ret != 0) {
				printf("err ret=%d\n", ret);
				exit(1);
			}
		}
		else
			die(usage);
}

int
main( int argc, char** argv )
{
	bswabe_pub_t* pub;
	bswabe_msk_t* msk;

	parse_args(argc, argv);

	int ret = mclBn_init(MCL_BLS12_381, MCLBN_COMPILED_TIME_VAR);
	if (ret != 0) {
		printf("err ret=%d\n", ret);
		exit(1);
	}

	bswabe_setup(&pub, &msk);
	spit_file(pub_file, bswabe_pub_serialize(pub), 1);
	spit_file(msk_file, bswabe_msk_serialize(msk), 1);

	return 0;
}
