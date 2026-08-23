/*
	Include glib.h, mcl/bn_c384_256.h, and bswabe.h before including this file.
*/

struct bswabe_pub_s
{
	mclBnGT p;			 /* G_T */
	mclBnG1 g;			 /* G_1 */
	mclBnG1 h;			 /* G_1 */
	mclBnG2 gp;			 /* G_2 */
	mclBnGT g_hat_alpha; /* G_T */
};

struct bswabe_msk_s
{
	mclBnFr beta;	 /* F_r */
	mclBnG2 g_alpha; /* G_2 */
};

typedef struct
{
	/* these actually get serialized */
	char *attr;
	mclBnG2 d;	/* G_2 */
	mclBnG1 dp; /* G_1 */

	/* only used during dec (only by dec_merge) */
	int used;
	mclBnG1 z;	/* G_1 */
	mclBnG1 zp; /* G_1 */
} bswabe_prv_comp_t;

struct bswabe_prv_s
{
	mclBnG2 d; /* G_2 */

	GArray *comps; /* bswabe_prv_comp_t's */
};

typedef struct
{
	int deg;
	/* coefficients from [0] x^0 to [deg] x^deg */
	mclBnFr *coef; /* Fr (of length deg + 1) */
} bswabe_polynomial_t;

typedef struct
{
	/* serialized */
	int k;		/* one if leaf, otherwise threshold */
	char *attr; /* attribute string if leaf, otherwise null */
	mclBnG1 c;
	mclBnG2 cp;

	GPtrArray *children; /* pointers to bswabe_policy_t's, len == 0 for leaves */

	/* only used during encryption */
	bswabe_polynomial_t *q;

	/* only used during decryption */
	int satisfiable;
	int min_leaves;
	int attri;
	GArray *satl;
} bswabe_policy_t;

struct bswabe_cph_s
{
	mclBnGT cs; /* G_T */
	mclBnG1 c;	/* G_1 */
	bswabe_policy_t *p;
};