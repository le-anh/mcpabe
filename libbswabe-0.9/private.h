/*
	Include glib.h, pbc.h ↦ mcl/bn_c384_256.h, and bswabe.h before including this file.
*/

struct bswabe_pub_s
{
	// PBC
	// char* pairing_desc;
	// pairing_t p;
	// element_t g;           /* G_1 */
	// element_t h;           /* G_1 */
	// element_t gp;          /* G_2 */
	// element_t g_hat_alpha; /* G_T */

	// MCL
	char* pairing_desc;
	mclBnGT p;			/* G_T */
	mclBnG1 g;			/* G_1 */
	mclBnG1 h;			/* G_1 */
	mclBnG2 gp;			/* G_2 */
	mclBnGT g_hat_alpha;	/* G_T */
};

struct bswabe_msk_s
{
	// PBC
	// element_t beta;    /* Z_r */
	// element_t g_alpha; /* G_2 */

	// MCL
	mclBnFr beta;    /* Z_r */
	mclBnG2 g_alpha; /* G_2 */
};

typedef struct
{
	/* these actually get serialized */
	char* attr;
	// PBC
	// element_t d;  /* G_2 */
	// element_t dp; /* G_2 */

	// MCL
	mclBnG2 d;  /* G_2 */
	mclBnG1 dp; /* G_1 */

	/* only used during dec (only by dec_merge) */
	int used;
	// PBC
	// element_t z;  /* G_1 */
	// element_t zp; /* G_1 */

	// MCL
	mclBnG1 z;  /* G_1 */
	mclBnG1 zp; /* G_1 */
}
bswabe_prv_comp_t;

struct bswabe_prv_s
{
	// PBC
	// element_t d;   /* G_2 */

	// MCL
	mclBnG2 d;   /* G_2 */

	GArray* comps; /* bswabe_prv_comp_t's */
};

typedef struct
{
	int deg;
	/* coefficients from [0] x^0 to [deg] x^deg */
	// PBC
	// element_t* coef; /* G_T (of length deg + 1) */

	// MCL
	mclBnFr* coef; /* Fr (of length deg + 1) */
}
bswabe_polynomial_t;

typedef struct
{
	/* serialized */
	int k;            /* one if leaf, otherwise threshold */
	char* attr;       /* attribute string if leaf, otherwise null */
	// PBC
	// element_t c;      /* G_1, only for leaves */
	// element_t cp;     /* G_1, only for leaves */
	// MCL
	mclBnG1 c;      /* G_1, only for leaves */
	mclBnG1 cp;     /* G_1, only for leaves */

	GPtrArray* children; /* pointers to bswabe_policy_t's, len == 0 for leaves */

	/* only used during encryption */
	bswabe_polynomial_t* q;

	/* only used during decryption */
	int satisfiable;
	int min_leaves;
	int attri;
	GArray* satl;
}
bswabe_policy_t;

struct bswabe_cph_s
{
	// PBC
	// element_t cs; /* G_T */
	// element_t c;  /* G_1 */
	// bswabe_policy_t* p;

	// MCL
	mclBnGT cs; /* G_T */
	mclBnG1 c;  /* G_1 */
	bswabe_policy_t* p;
};
