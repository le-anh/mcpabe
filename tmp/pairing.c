#include <stdio.h>
#include <string.h>
#include <time.h>
#include <mcl/bn_c384_256.h>

#define G1ByteSize 770
#define G2ByteSize 1540
#define GTByteSize 4619

int num_loop = 1;
int g_err = 0;
#define ASSERT(x)                                      \
	{                                                  \
		if (!(x))                                      \
		{                                              \
			printf("err %s:%d\n", __FILE__, __LINE__); \
			g_err++;                                   \
		}                                              \
	}

long double getCurrentTime()
{
	struct timespec ts;
	clock_gettime(CLOCK_MONOTONIC, &ts);
	return (long double)ts.tv_sec + (long double)ts.tv_nsec / 1000000000.0;
}

int main()
{
	// long double start1, end1, start2, end2, start3, end3, start4, end4, elapsed_MillerLoop = 0.0L, elapsed_FinalExp = 0.0L;

	char buf[1600];
	// const char *aStr = "123";
	// const char *bStr = "456";
	int ret = mclBn_init(MCL_BLS12_381, MCLBN_COMPILED_TIME_VAR);
	if (ret != 0)
	{
		printf("err ret=%d\n", ret);
		return 1;
	}
	mclBnFr a, b, ab;
	mclBnG1 P, aP;
	mclBnG2 Q, bQ;
	mclBnGT e, e1, e2, e3, e4;
	// mclBnFr_setStr(&a, aStr, strlen(aStr), 10);
	// mclBnFr_setStr(&b, bStr, strlen(bStr), 10);
	// mclBnFr_mul(&ab, &a, &b);
	// mclBnFr_getStr(buf, sizeof(buf), &ab, 10);
	// printf("%s x %s = %s\n", aStr, bStr, buf);
	// mclBnFr_sub(&a, &a, &b);
	// mclBnFr_getStr(buf, sizeof(buf), &a, 10);
	// printf("%s - %s = %s\n", aStr, bStr, buf);

	ASSERT(!mclBnG1_hashAndMapTo(&P, "this", 4));
	ASSERT(!mclBnG2_hashAndMapTo(&Q, "that", 4));
	// ASSERT(mclBnG1_getStr(buf, sizeof(buf), &P, 16));
	// printf("P = %s\n", buf);
	// ASSERT(mclBnG2_getStr(buf, sizeof(buf), &Q, 16));
	// printf("Q = %s\n", buf);

	// mclBnG1_mul(&aP, &P, &a);
	// mclBnG2_mul(&bQ, &Q, &b);

	mclBn_pairing(&e, &P, &Q);
	// ASSERT(mclBnGT_getStr(buf, sizeof(buf), &e, 16));
	// printf("e = %s\n", buf);
	// mclBnGT_pow(&e1, &e, &a);
	// start1 = getCurrentTime();
	// for(int i = 0; i < num_loop; i++) {
	// 	mclBn_pairing(&e2, &aP, &Q);
	// }
	// end1 = getCurrentTime();

	// ASSERT(mclBnGT_isEqual(&e1, &e2));

	// mclBnGT_pow(&e1, &e, &b);
	// start2 = getCurrentTime();
	// for(int i = 0; i < num_loop; i++) {
	// 	mclBn_pairing(&e2, &P, &bQ);
	// }
	// end2 = getCurrentTime();

	// ASSERT(mclBnGT_isEqual(&e1, &e2));
	// if (g_err) {
	// 	printf("e1 vs e2: err %d\n", g_err);
	// 	// return 1;
	// } else {
	// 	printf("e1 vs e2: no err\n");
	// 	// return 0;
	// }

	// start3 = getCurrentTime();
	// for(int i = 0; i < num_loop; i++) {
	// 	start4 = getCurrentTime();
	// 	mclBn_millerLoop(&e3, &P, &bQ);
	// 	end4 = getCurrentTime();
	// 	elapsed_MillerLoop += end4 - start4;

	// 	start4 = getCurrentTime();
	// 	mclBn_finalExp(&e3, &e3);
	// 	end4 = getCurrentTime();
	// 	elapsed_FinalExp += end4 - start4;
	// }
	// end3 = getCurrentTime();

	// ASSERT(mclBnGT_isEqual(&e3, &e2));
	// if (g_err) {
	// 	printf("e3 vs e2: err %d\n", g_err);
	// 	// return 1;
	// } else {
	// 	printf("e3 vs e2: no err\n");
	// 	// return 0;
	// }

	// printf("pairing %d times: %Lf sec\n", num_loop, end1 - start1);
	// printf("pairing %d times: %Lf sec\n", num_loop, end2 - start2);
	// printf("millerLoop+finalExp %d times: %Lf sec\n", num_loop, end3 - start3);
	// printf("\tMillerLoop: %Lf sec\n", elapsed_MillerLoop);
	// printf("\tFinalExp: %Lf sec\n", elapsed_FinalExp);

	// mclBn_millerLoop(&e3, &P, &bQ);
	// mclBn_millerLoop(&e4, &P, &bQ);
	// mclBnGT_mul(&e3, &e3, &e4);
	// mclBn_finalExp(&e3, &e3);
	// mclBnGT_mul(&e1, &e1, &e2);

	// ASSERT(mclBnGT_isEqual(&e1, &e3));
	// if (g_err) {
	// 	printf("e1 vs e3: err %d\n", g_err);
	// 	// return 1;
	// } else {
	// 	printf("e1 vs e3: no err\n");
	// 	// return 0;
	// }

	// Test serisialize and
	// int G1ByteSize = mclBn_getFpByteSize() * 8 * 2 + 2;
	// int G2ByteSize = mclBn_getFpByteSize() * 8 * 4 + 4;
	// int GTByteSize = mclBn_getFpByteSize() * 8 * 12 + 11;

	printf("G1ByteSize: %d\n", G1ByteSize);
	printf("G2ByteSize: %d\n", G2ByteSize);
	printf("GTByteSize: %d\n", GTByteSize);

	ASSERT(mclBnG1_getStr(buf, G1ByteSize, &P, 16));
	printf("P = %s\n", buf);
	ASSERT(mclBnG1_serialize(buf, G1ByteSize, &P));
	printf("serialize(P): %s\n", buf);
	mclBnG1_clear(&P);
	ASSERT(mclBnG1_deserialize(&P, buf, G1ByteSize));
	ASSERT(mclBnG1_getStr(buf, sizeof(buf), &P, 16));
	printf("deserialize(P): %s\n", buf);

	ASSERT(mclBnG2_getStr(buf, G2ByteSize, &Q, 16));
	printf("Q = %s\n", buf);
	ASSERT(mclBnG2_serialize(buf, G2ByteSize, &Q));
	printf("serialize(Q): %s\n", buf);
	mclBnG2_clear(&Q);
	ASSERT(mclBnG2_deserialize(&Q, buf, G2ByteSize));
	ASSERT(mclBnG2_getStr(buf, sizeof(buf), &Q, 16));
	printf("deserialize(Q): %s\n", buf);

	ASSERT(mclBnGT_getStr(buf, GTByteSize, &e, 16));
	printf("e = %s\n", buf);
	ASSERT(mclBnGT_serialize(buf, GTByteSize, &e));
	printf("serialize(e): %s\n", buf);
	mclBnGT_clear(&e);
	ASSERT(mclBnGT_deserialize(&e, buf, GTByteSize));
	ASSERT(mclBnGT_getStr(buf, sizeof(buf), &e, 16));
	printf("deserialize(e): %s\n", buf);

	printf(" mclBn_getG1ByteSize: %d\n mclBn_getG2ByteSize:%d\n mclBn_getFpByteSize: %d\n", mclBn_getG1ByteSize(), mclBn_getG2ByteSize(), mclBn_getFpByteSize());
}