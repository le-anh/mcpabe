#include <stdlib.h>
#include <string.h>
#ifndef BSWABE_DEBUG
#define NDEBUG
#endif
#include <assert.h>
#include <openssl/sha.h>
#include <glib.h>
#include <mcl/bn_c384_256.h>
#include "bswabe.h"
#include "private.h"

char last_error[256];

char *
bswabe_error()
{
	return last_error;
}

void raise_error(char *fmt, ...)
{
	va_list args;

#ifdef BSWABE_DEBUG
	va_start(args, fmt);
	vfprintf(stderr, fmt, args);
	va_end(args);
	exit(1);
#else
	va_start(args, fmt);
	vsnprintf(last_error, 256, fmt, args);
	va_end(args);
#endif
}

char *random_string(int length)
{
	char *str = (char *)malloc(length + 1);
	if (str == NULL)
	{
		return NULL;
	}

	const char charset[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
	int charset_size = strlen(charset);

	for (int i = 0; i < length; i++)
	{
		str[i] = charset[rand() % charset_size];
	}
	str[length] = '\0';

	return str;
}

void random_mclBnG1(mclBnG1 *P)
{
	mclBnFp fp_tmp;
	mclBnFp_setByCSPRNG(&fp_tmp);
	mclBnFp_mapToG1(P, &fp_tmp);
	mclBnFp_clear(&fp_tmp);
}

void random_mclBnG2(mclBnG2 *Q)
{
	mclBnG2_hashAndMapTo(Q, random_string(24), 24);
}

void random_mclBnGT(mclBnGT *e)
{
	mclBnG1 P;
	mclBnG2 Q;

	random_mclBnG1(&P);
	random_mclBnG2(&Q);

	mclBn_pairing(e, &P, &Q);

	mclBnG1_clear(&P);
	mclBnG2_clear(&Q);
}

void bswabe_setup(bswabe_pub_t **pub, bswabe_msk_t **msk)
{
	mclBnFr alpha;
	/* initialize */
	*pub = malloc(sizeof(bswabe_pub_t));
	*msk = malloc(sizeof(bswabe_msk_t));

	mclBnFr_setByCSPRNG(&alpha);
	mclBnFr_setByCSPRNG(&(*msk)->beta);
	random_mclBnG1(&(*pub)->g);
	random_mclBnG2(&(*pub)->gp);

	mclBnG2_mul(&(*msk)->g_alpha, &(*pub)->gp, &alpha);
	mclBnG1_mul(&(*pub)->h, &(*pub)->g, &(*msk)->beta);
	mclBn_pairing(&(*pub)->g_hat_alpha, &(*pub)->g, &(*msk)->g_alpha);
}

bswabe_prv_t *bswabe_keygen(bswabe_pub_t *pub, bswabe_msk_t *msk, char **attributes)
{
	bswabe_prv_t *prv;
	mclBnG2 g_r;
	mclBnFr r;
	mclBnFr beta_inv;

	/* initialize */
	prv = malloc(sizeof(bswabe_prv_t));

	prv->comps = g_array_new(0, 1, sizeof(bswabe_prv_comp_t));

	/* compute */
	mclBnFr_setByCSPRNG(&r);
	mclBnG2_mul(&g_r, &pub->gp, &r);
	mclBnG2_add(&prv->d, &msk->g_alpha, &g_r);
	mclBnFr_inv(&beta_inv, &msk->beta);
	mclBnG2_mul(&prv->d, &prv->d, &beta_inv);
	int i = 0;
	while (*attributes)
	{
		bswabe_prv_comp_t c;
		mclBnG2 h_rp;
		mclBnFr rp;

		c.attr = *(attributes++);

		mclBnG2_hashAndMapTo(&h_rp, c.attr, strlen(c.attr));
		mclBnFr_setByCSPRNG(&rp);
		mclBnG2_mul(&h_rp, &h_rp, &rp);
		mclBnG2_add(&c.d, &h_rp, &g_r);
		mclBnG1_mul(&c.dp, &pub->g, &rp);

		mclBnG2_clear(&h_rp);
		mclBnFr_clear(&rp);

		g_array_append_val(prv->comps, c);
	}

	return prv;
}

bswabe_policy_t *base_node(int k, char *s)
{
	bswabe_policy_t *p;

	p = (bswabe_policy_t *)malloc(sizeof(bswabe_policy_t));
	p->k = k;
	p->attr = s ? strdup(s) : 0;
	p->children = g_ptr_array_new();
	p->q = 0;

	return p;
}

/*
	TODO convert this to use a GScanner and handle quotes and / or
	escapes to allow attributes with whitespace or = signs in them
*/

bswabe_policy_t *parse_policy_postfix(char *s)
{
	char **toks;
	char **cur_toks;
	char *tok;
	GPtrArray *stack; /* pointers to bswabe_policy_t's */
	bswabe_policy_t *root;

	toks = g_strsplit(s, " ", 0);
	cur_toks = toks;
	stack = g_ptr_array_new();

	while (*cur_toks)
	{
		int i, k, n;

		tok = *(cur_toks++);

		if (!*tok)
			continue;

		if (sscanf(tok, "%dof%d", &k, &n) != 2)
			/* push leaf token */
			g_ptr_array_add(stack, base_node(1, tok));
		else
		{
			bswabe_policy_t *node;

			/* parse "kofn" operator */

			if (k < 1)
			{
				raise_error("error parsing \"%s\": trivially satisfied operator \"%s\"\n", s, tok);
				return 0;
			}
			else if (k > n)
			{
				raise_error("error parsing \"%s\": unsatisfiable operator \"%s\"\n", s, tok);
				return 0;
			}
			else if (n == 1)
			{
				raise_error("error parsing \"%s\": identity operator \"%s\"\n", s, tok);
				return 0;
			}
			else if (n > stack->len)
			{
				raise_error("error parsing \"%s\": stack underflow at \"%s\"\n", s, tok);
				return 0;
			}

			/* pop n things and fill in children */
			node = base_node(k, 0);
			g_ptr_array_set_size(node->children, n);
			for (i = n - 1; i >= 0; i--)
				node->children->pdata[i] = g_ptr_array_remove_index(stack, stack->len - 1);

			/* push result */
			g_ptr_array_add(stack, node);
		}
	}

	if (stack->len > 1)
	{
		raise_error("error parsing \"%s\": extra tokens left on stack\n", s);
		return 0;
	}
	else if (stack->len < 1)
	{
		raise_error("error parsing \"%s\": empty policy\n", s);
		return 0;
	}

	root = g_ptr_array_index(stack, 0);

	g_strfreev(toks);
	g_ptr_array_free(stack, 0);

	return root;
}

bswabe_polynomial_t *rand_poly(int deg, mclBnFr zero_val)
{
	int i;
	bswabe_polynomial_t *q;

	q = (bswabe_polynomial_t *)malloc(sizeof(bswabe_polynomial_t));
	q->deg = deg;
	q->coef = (mclBnFr *)malloc(sizeof(mclBnFr) * (deg + 1));

	q->coef[0] = zero_val;

	for (i = 1; i < q->deg + 1; i++)
		mclBnFr_setByCSPRNG(&q->coef[i]);

	return q;
}

void eval_poly(mclBnFr *r, bswabe_polynomial_t *q, mclBnFr x)
{
	int i;
	mclBnFr s;
	mclBnFr t;

	mclBnFr_setInt(r, 0);
	mclBnFr_setInt(&t, 1);

	for (i = 0; i < q->deg + 1; i++)
	{
		mclBnFr_mul(&s, &q->coef[i], &t);
		mclBnFr_add(r, r, &s);

		/* t *= x */
		mclBnFr_mul(&t, &t, &x);
	}

	mclBnFr_clear(&s);
	mclBnFr_clear(&t);
}

void fill_policy(bswabe_policy_t *p, bswabe_pub_t *pub, mclBnFr e)
{
	int i;
	mclBnFr r;
	mclBnFr t;
	mclBnG2 h;

	p->q = rand_poly(p->k - 1, e);

	if (p->children->len == 0)
	{
		mclBnG2_hashAndMapTo(&h, p->attr, strlen(p->attr));
		mclBnG1_mul(&p->c, &pub->g, &p->q->coef[0]);
		mclBnG2_mul(&p->cp, &h, &p->q->coef[0]);
	}
	else
		for (i = 0; i < p->children->len; i++)
		{
			mclBnFr_setInt(&r, i + 1);
			eval_poly(&t, p->q, r);
			fill_policy(g_ptr_array_index(p->children, i), pub, t);
		}

	mclBnFr_clear(&r);
	mclBnFr_clear(&t);
	mclBnG2_clear(&h);
}

bswabe_cph_t *bswabe_enc(bswabe_pub_t *pub, mclBnGT *m, char *policy)
{
	bswabe_cph_t *cph;
	mclBnFr s;

	/* initialize */
	cph = malloc(sizeof(bswabe_cph_t));

	cph->p = parse_policy_postfix(policy);

	/* compute */
	random_mclBnGT(m);
	mclBnFr_setByCSPRNG(&s);

	mclBnGT_pow(&cph->cs, &pub->g_hat_alpha, &s);
	mclBnGT_mul(&cph->cs, &cph->cs, m);
	mclBnG1_mul(&cph->c, &pub->h, &s);

	fill_policy(cph->p, pub, s);

	return cph;
}

void check_sat(bswabe_policy_t *p, bswabe_prv_t *prv)
{
	int i, l;

	p->satisfiable = 0;
	if (p->children->len == 0)
	{
		for (i = 0; i < prv->comps->len; i++)
			if (!strcmp(g_array_index(prv->comps, bswabe_prv_comp_t, i).attr, p->attr))
			{
				p->satisfiable = 1;
				p->attri = i;
				break;
			}
	}
	else
	{
		for (i = 0; i < p->children->len; i++)
			check_sat(g_ptr_array_index(p->children, i), prv);
		l = 0;
		for (i = 0; i < p->children->len; i++)
			if (((bswabe_policy_t *)g_ptr_array_index(p->children, i))->satisfiable)
				l++;

		if (l >= p->k)
			p->satisfiable = 1;
	}
}

void pick_sat_naive(bswabe_policy_t *p, bswabe_prv_t *prv)
{
	int i, k, l;

	assert(p->satisfiable == 1);

	if (p->children->len == 0)
		return;

	p->satl = g_array_new(0, 0, sizeof(int));

	l = 0;
	for (i = 0; i < p->children->len && l < p->k; i++)
		if (((bswabe_policy_t *)g_ptr_array_index(p->children, i))->satisfiable)
		{
			pick_sat_naive(g_ptr_array_index(p->children, i), prv);
			l++;
			k = i + 1;
			g_array_append_val(p->satl, k);
		}
}

/* TODO there should be a better way of doing this */
bswabe_policy_t *cur_comp_pol;
int cmp_int(const void *a, const void *b)
{
	int k, l;

	k = ((bswabe_policy_t *)g_ptr_array_index(cur_comp_pol->children, *((int *)a)))->min_leaves;
	l = ((bswabe_policy_t *)g_ptr_array_index(cur_comp_pol->children, *((int *)b)))->min_leaves;

	return k < l ? -1 : k == l ? 0
							   : 1;
}

void pick_sat_min_leaves(bswabe_policy_t *p, bswabe_prv_t *prv)
{
	int i, k, l;
	int *c;

	assert(p->satisfiable == 1);

	if (p->children->len == 0)
		p->min_leaves = 1;
	else
	{
		for (i = 0; i < p->children->len; i++)
			if (((bswabe_policy_t *)g_ptr_array_index(p->children, i))->satisfiable)
				pick_sat_min_leaves(g_ptr_array_index(p->children, i), prv);

		c = alloca(sizeof(int) * p->children->len);
		for (i = 0; i < p->children->len; i++)
			c[i] = i;

		cur_comp_pol = p;
		qsort(c, p->children->len, sizeof(int), cmp_int);

		p->satl = g_array_new(0, 0, sizeof(int));
		p->min_leaves = 0;
		l = 0;

		for (i = 0; i < p->children->len && l < p->k; i++)
			if (((bswabe_policy_t *)g_ptr_array_index(p->children, c[i]))->satisfiable)
			{
				l++;
				p->min_leaves += ((bswabe_policy_t *)g_ptr_array_index(p->children, c[i]))->min_leaves;
				k = c[i] + 1;
				g_array_append_val(p->satl, k);
			}
		assert(l == p->k);
	}
}

void lagrange_coef(mclBnFr *r, GArray *s, int i)
{
	int j, k;
	mclBnFr t;
	mclBnFr_setInt(r, 1);

	for (k = 0; k < s->len; k++)
	{
		j = g_array_index(s, int, k);
		if (j == i)
			continue;
		mclBnFr_setInt(&t, -j);
		mclBnFr_mul(r, r, &t);
		mclBnFr_setInt(&t, i - j);
		mclBnFr_inv(&t, &t);
		mclBnFr_mul(r, r, &t);
	}

	mclBnFr_clear(&t);
}

void dec_leaf_naive(mclBnGT *r, bswabe_policy_t *p, bswabe_prv_t *prv, bswabe_pub_t *pub)
{
	bswabe_prv_comp_t *c;
	mclBnGT s;
	c = &(g_array_index(prv->comps, bswabe_prv_comp_t, p->attri));
	mclBn_pairing(r, &p->c, &c->d);
	mclBn_pairing(&s, &p->cp, &c->dp);
	mclBnGT_inv(&s, &s);
	mclBnGT_mul(r, r, &s);
	mclBnGT_clear(&s);
}

void dec_node_naive(mclBnGT *r, bswabe_policy_t *p, bswabe_prv_t *prv, bswabe_pub_t *pub);

void dec_internal_naive(mclBnGT *r, bswabe_policy_t *p, bswabe_prv_t *prv, bswabe_pub_t *pub)
{
	int i;
	mclBnGT s;
	mclBnFr t;
	mclBnGT_setInt(r, 1);

	for (i = 0; i < p->satl->len; i++)
	{
		dec_node_naive(&s, g_ptr_array_index(p->children, g_array_index(p->satl, int, i) - 1), prv, pub);
		lagrange_coef(&t, p->satl, g_array_index(p->satl, int, i));
		mclBnGT_pow(&s, &s, &t);
		mclBnGT_mul(r, r, &s);
	}

	mclBnGT_clear(&s);
	mclBnFr_clear(&t);
}

void dec_node_naive(mclBnGT *r, bswabe_policy_t *p, bswabe_prv_t *prv, bswabe_pub_t *pub)
{
	assert(p->satisfiable);
	if (p->children->len == 0)
		dec_leaf_naive(&r, p, prv, pub);
	else
		dec_internal_naive(&r, p, prv, pub);
}

void dec_naive(mclBnGT *r, bswabe_policy_t *p, bswabe_prv_t *prv, bswabe_pub_t *pub)
{
	dec_node_naive(&r, p, prv, pub);
}

void dec_leaf_merge(mclBnFr exp, bswabe_policy_t *p, bswabe_prv_t *prv, bswabe_pub_t *pub)
{
	bswabe_prv_comp_t *c;
	mclBnG1 s;

	c = &(g_array_index(prv->comps, bswabe_prv_comp_t, p->attri));

	if (!c->used)
	{
		c->used = 1;
		mclBnFp one;
		mclBnFp_setInt(&one, 1);
		mclBnFp_mapToG1(&c->z, &one);
		mclBnFp_mapToG1(&c->zp, &one);
		mclBnFp_clear(&one);
	}

	mclBnG1_mul(&s, &p->c, &exp);
	mclBnG1_add(&c->z, &c->z, &s);
	mclBnG1_mul(&s, &p->cp, &exp);
	mclBnG1_add(&c->zp, &c->zp, &s);
	mclBnG1_clear(&s);
}

void dec_node_merge(mclBnFr exp, bswabe_policy_t *p, bswabe_prv_t *prv, bswabe_pub_t *pub);

void dec_internal_merge(mclBnFr exp, bswabe_policy_t *p, bswabe_prv_t *prv, bswabe_pub_t *pub)
{
	int i;
	mclBnFr t;
	mclBnFr expnew;

	for (i = 0; i < p->satl->len; i++)
	{
		lagrange_coef(&t, p->satl, g_array_index(p->satl, int, i));
		mclBnFr_mul(&expnew, &exp, &t);
		dec_node_merge(expnew, g_ptr_array_index(p->children, g_array_index(p->satl, int, i) - 1), prv, pub);
	}

	mclBnFr_clear(&t);
	mclBnFr_clear(&expnew);
}

void dec_node_merge(mclBnFr exp, bswabe_policy_t *p, bswabe_prv_t *prv, bswabe_pub_t *pub)
{
	assert(p->satisfiable);
	if (p->children->len == 0)
		dec_leaf_merge(exp, p, prv, pub);
	else
		dec_internal_merge(exp, p, prv, pub);
}

void dec_merge(mclBnGT *r, bswabe_policy_t *p, bswabe_prv_t *prv, bswabe_pub_t *pub)
{
	int i;
	mclBnFr one;
	mclBnGT s;

	/* first mark all attributes as unused */
	for (i = 0; i < prv->comps->len; i++)
		g_array_index(prv->comps, bswabe_prv_comp_t, i).used = 0;

	/* now fill in the z's and zp's */
	mclBnFr_setInt(&one, 1);
	dec_node_merge(one, p, prv, pub);
	mclBnFr_clear(&one);

	/* now do all the pairings and multiply everything together */
	mclBnGT_setInt(r, 1);

	for (i = 0; i < prv->comps->len; i++)
		if (g_array_index(prv->comps, bswabe_prv_comp_t, i).used)
		{
			bswabe_prv_comp_t *c = &(g_array_index(prv->comps, bswabe_prv_comp_t, i));
			mclBn_pairing(&s, &c->z, &c->d);
			mclBnGT_mul(r, r, &s);
			mclBn_pairing(&s, &c->zp, &c->dp);
			mclBnGT_inv(&s, &s);
			mclBnGT_mul(r, r, &s);
		}

	mclBnGT_clear(&s);
}

void dec_leaf_flatten(mclBnGT *r, mclBnFr exp, bswabe_policy_t *p, bswabe_prv_t *prv, bswabe_pub_t *pub)
{
	bswabe_prv_comp_t *c;
	mclBnGT s;
	mclBnGT t;

	c = &(g_array_index(prv->comps, bswabe_prv_comp_t, p->attri));

	mclBn_pairing(&s, &p->c, &c->d);
	mclBn_pairing(&t, &c->dp, &p->cp);

	mclBnGT_inv(&t, &t);
	mclBnGT_mul(&s, &s, &t);
	mclBnGT_pow(&s, &s, &exp);
	mclBnGT_mul(r, r, &s);

	mclBnGT_clear(&s);
	mclBnGT_clear(&t);
}

void dec_node_flatten(mclBnGT *r, mclBnFr exp, bswabe_policy_t *p, bswabe_prv_t *prv, bswabe_pub_t *pub);

void dec_internal_flatten(mclBnGT *r, mclBnFr exp, bswabe_policy_t *p, bswabe_prv_t *prv, bswabe_pub_t *pub)
{
	int i;
	mclBnFr t;
	mclBnFr expnew;

	for (i = 0; i < p->satl->len; i++)
	{
		lagrange_coef(&t, p->satl, g_array_index(p->satl, int, i));
		mclBnFr_mul(&expnew, &exp, &t);
		dec_node_flatten(r, expnew, g_ptr_array_index(p->children, g_array_index(p->satl, int, i) - 1), prv, pub);
	}

	mclBnFr_clear(&t);
	mclBnFr_clear(&expnew);
}

void dec_node_flatten(mclBnGT *r, mclBnFr exp, bswabe_policy_t *p, bswabe_prv_t *prv, bswabe_pub_t *pub)
{
	assert(p->satisfiable);
	if (p->children->len == 0)
		dec_leaf_flatten(r, exp, p, prv, pub);
	else
		dec_internal_flatten(r, exp, p, prv, pub);
}

void dec_flatten(mclBnGT *r, bswabe_policy_t *p, bswabe_prv_t *prv, bswabe_pub_t *pub)
{
	mclBnFr one;
	mclBnFr_setInt(&one, 1);
	mclBnGT_setInt(r, 1);

	dec_node_flatten(r, one, p, prv, pub);

	mclBnFr_clear(&one);
}

int bswabe_dec(bswabe_pub_t *pub, bswabe_prv_t *prv, bswabe_cph_t *cph, mclBnGT *m)
{
	mclBnGT t;

	check_sat(cph->p, prv);
	if (!cph->p->satisfiable)
	{
		raise_error("cannot decrypt, attributes in key do not satisfy policy\n");
		return 0;
	}

	/* 	if( no_opt_sat ) */
	/* 		pick_sat_naive(cph->p, prv); */
	/* 	else */
	pick_sat_min_leaves(cph->p, prv);

	/* 	if( dec_strategy == DEC_NAIVE ) */
	/* 		dec_naive(t, cph->p, prv, pub); */
	/* 	else if( dec_strategy == DEC_FLATTEN ) */
	dec_flatten(&t, cph->p, prv, pub);
	/* 	else */
	/* 		dec_merge(t, cph->p, prv, pub); */

	mclBnGT_mul(m, &cph->cs, &t);
	mclBn_pairing(&t, &cph->c, &prv->d);
	mclBnGT_inv(&t, &t);

	mclBnGT_mul(m, m, &t);

	return 1;
}