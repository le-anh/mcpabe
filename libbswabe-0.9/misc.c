#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <glib.h>
#include <mcl/bn_c384_256.h>
#include <stdio.h>

#include "bswabe.h"
#include "private.h"

#define FrByteSize 386
#define FpByteSize 386
#define G1ByteSize 770
#define G2ByteSize 1540
#define GTByteSize 4619

void serialize_uint32(GByteArray *b, uint32_t k)
{
	int i;
	guint8 byte;

	for (i = 3; i >= 0; i--)
	{
		byte = (k & 0xff << (i * 8)) >> (i * 8);
		g_byte_array_append(b, &byte, 1);
	}
}

uint32_t unserialize_uint32(GByteArray *b, int *offset)
{
	int i;
	uint32_t r;

	r = 0;
	for (i = 3; i >= 0; i--)
		r |= (b->data[(*offset)++]) << (i * 8);

	return r;
}

void serialize_element(GByteArray *b, mclBnGT e)
{
	unsigned char *buf;
	serialize_uint32(b, GTByteSize);
	buf = (unsigned char *)malloc(GTByteSize);
	mclBnGT_serialize(buf, sizeof(buf), &e);
	g_byte_array_append(b, buf, GTByteSize);
	free(buf);
}

void serialize_G1(GByteArray *b, mclBnG1 e)
{
	unsigned char *buf;
	serialize_uint32(b, G1ByteSize);
	buf = (unsigned char *)malloc(G1ByteSize);
	mclBnG1_serialize(buf, G1ByteSize, &e);
	g_byte_array_append(b, buf, G1ByteSize);
	free(buf);
}

void serialize_G2(GByteArray *b, mclBnG2 e)
{
	unsigned char *buf;
	serialize_uint32(b, G2ByteSize);
	buf = (unsigned char *)malloc(G2ByteSize);
	mclBnG2_serialize(buf, G2ByteSize, &e);
	g_byte_array_append(b, buf, G2ByteSize);
	free(buf);
}

void serialize_GT(GByteArray *b, mclBnGT e)
{
	unsigned char *buf;
	serialize_uint32(b, GTByteSize);
	buf = (unsigned char *)malloc(GTByteSize);
	mclBnGT_serialize(buf, GTByteSize, &e);
	g_byte_array_append(b, buf, GTByteSize);
	free(buf);
}

void serialize_Fr(GByteArray *b, mclBnFr e)
{
	unsigned char *buf;
	serialize_uint32(b, FrByteSize);
	buf = (unsigned char *)malloc(FrByteSize);
	mclBnFr_serialize(buf, FrByteSize, &e);
	g_byte_array_append(b, buf, FrByteSize);
	free(buf);
}

void unserialize_element(GByteArray *b, int *offset, mclBnGT *e)
{
	uint32_t len;
	unsigned char *buf;

	len = unserialize_uint32(b, offset);
	buf = (unsigned char *)malloc(len);
	memcpy(buf, b->data + *offset, len);
	*offset += len;

	mclBnGT_deserialize(e, buf, len);

	free(buf);
}

void unserialize_G1(GByteArray *b, int *offset, mclBnG1 *e)
{
	uint32_t len;
	unsigned char *buf;

	len = unserialize_uint32(b, offset);
	buf = (unsigned char *)malloc(len);
	memcpy(buf, b->data + *offset, len);
	*offset += len;

	mclBnG1_deserialize(e, buf, len);

	free(buf);
}

void unserialize_G2(GByteArray *b, int *offset, mclBnG2 *e)
{
	uint32_t len;
	unsigned char *buf;

	len = unserialize_uint32(b, offset);
	buf = (unsigned char *)malloc(len);
	memcpy(buf, b->data + *offset, len);
	*offset += len;

	mclBnG2_deserialize(e, buf, len);

	free(buf);
}

void unserialize_GT(GByteArray *b, int *offset, mclBnGT *e)
{
	uint32_t len;
	unsigned char *buf;

	len = unserialize_uint32(b, offset);
	buf = (unsigned char *)malloc(len);
	memcpy(buf, b->data + *offset, len);
	*offset += len;

	mclBnGT_deserialize(e, buf, len);

	free(buf);
}

void unserialize_Fr(GByteArray *b, int *offset, mclBnFr *e)
{
	uint32_t len;
	unsigned char *buf;

	len = unserialize_uint32(b, offset);
	buf = (unsigned char *)malloc(len);
	memcpy(buf, b->data + *offset, len);
	*offset += len;

	mclBnFr_deserialize(e, buf, len);

	free(buf);
}

void serialize_string(GByteArray *b, char *s)
{
	g_byte_array_append(b, (unsigned char *)s, strlen(s) + 1);
}

char *unserialize_string(GByteArray *b, int *offset)
{
	GString *s;
	char *r;
	char c;

	s = g_string_sized_new(32);
	while (1)
	{
		c = b->data[(*offset)++];
		if (c && c != EOF)
			g_string_append_c(s, c);
		else
			break;
	}

	r = s->str;
	g_string_free(s, 0);

	return r;
}

GByteArray *bswabe_pub_serialize(bswabe_pub_t *pub)
{
	GByteArray *b;

	b = g_byte_array_new();

	serialize_G1(b, pub->g);
	serialize_G1(b, pub->h);
	serialize_G2(b, pub->gp);
	serialize_GT(b, pub->g_hat_alpha);

	return b;
}

bswabe_pub_t *bswabe_pub_unserialize(GByteArray *b, int free)
{
	bswabe_pub_t *pub;
	int offset;

	pub = (bswabe_pub_t *)malloc(sizeof(bswabe_pub_t));
	offset = 0;

	unserialize_G1(b, &offset, &pub->g);
	unserialize_G1(b, &offset, &pub->h);
	unserialize_G2(b, &offset, &pub->gp);
	unserialize_GT(b, &offset, &pub->g_hat_alpha);

	if (free)
		g_byte_array_free(b, 1);

	return pub;
}

GByteArray *bswabe_msk_serialize(bswabe_msk_t *msk)
{
	GByteArray *b;

	b = g_byte_array_new();
	serialize_Fr(b, msk->beta);
	serialize_G2(b, msk->g_alpha);

	return b;
}

bswabe_msk_t *bswabe_msk_unserialize(bswabe_pub_t *pub, GByteArray *b, int free)
{
	bswabe_msk_t *msk;
	int offset;

	msk = (bswabe_msk_t *)malloc(sizeof(bswabe_msk_t));
	offset = 0;

	unserialize_Fr(b, &offset, &msk->beta);
	unserialize_G2(b, &offset, &msk->g_alpha);

	if (free)
		g_byte_array_free(b, 1);

	return msk;
}

GByteArray *bswabe_prv_serialize(bswabe_prv_t *prv)
{
	GByteArray *b;
	int i;

	b = g_byte_array_new();

	serialize_G2(b, prv->d);
	serialize_uint32(b, prv->comps->len);

	bswabe_prv_comp_t c;
	for (i = 0; i < prv->comps->len; i++)
	{
		c = g_array_index(prv->comps, bswabe_prv_comp_t, i);
		serialize_string(b, c.attr);
		serialize_G2(b, c.d);
		serialize_G1(b, c.dp);

		// serialize_string(b, g_array_index(prv->comps, bswabe_prv_comp_t, i).attr);
		// serialize_G2(b, g_array_index(prv->comps, bswabe_prv_comp_t, i).d);
		// serialize_G1(b, g_array_index(prv->comps, bswabe_prv_comp_t, i).dp);
	}
	// free(c.attr);
	mclBnG2_clear(&c.d);
	mclBnG1_clear(&c.dp);

	return b;
}

bswabe_prv_t *bswabe_prv_unserialize(bswabe_pub_t *pub, GByteArray *b, int free)
{
	bswabe_prv_t *prv;
	int i;
	int len;
	int offset;

	prv = (bswabe_prv_t *)malloc(sizeof(bswabe_prv_t));
	offset = 0;

	unserialize_G2(b, &offset, &prv->d);

	prv->comps = g_array_new(0, 1, sizeof(bswabe_prv_comp_t));
	len = unserialize_uint32(b, &offset);

	for (i = 0; i < len; i++)
	{
		bswabe_prv_comp_t c;

		c.attr = unserialize_string(b, &offset);
		unserialize_G2(b, &offset, &c.d);
		unserialize_G1(b, &offset, &c.dp);

		g_array_append_val(prv->comps, c);
	}

	if (free)
		g_byte_array_free(b, 1);

	return prv;
}

void serialize_policy(GByteArray *b, bswabe_policy_t *p)
{
	int i;

	serialize_uint32(b, (uint32_t)p->k);
	serialize_uint32(b, (uint32_t)p->children->len);

	if (p->children->len == 0)
	{
		serialize_string(b, p->attr);
		serialize_G1(b, p->c);
		serialize_G2(b, p->cp);
	}
	else
		for (i = 0; i < p->children->len; i++)
			serialize_policy(b, g_ptr_array_index(p->children, i));
}

bswabe_policy_t *unserialize_policy(bswabe_pub_t *pub, GByteArray *b, int *offset)
{
	int i;
	int n;
	bswabe_policy_t *p;

	p = (bswabe_policy_t *)malloc(sizeof(bswabe_policy_t));

	p->k = (int)unserialize_uint32(b, offset);
	p->attr = 0;
	p->children = g_ptr_array_new();
	n = unserialize_uint32(b, offset);

	if (n == 0)
	{
		p->attr = unserialize_string(b, offset);
		unserialize_G1(b, offset, &p->c);
		unserialize_G2(b, offset, &p->cp);
	}
	else
		for (i = 0; i < n; i++)
			g_ptr_array_add(p->children, unserialize_policy(pub, b, offset));

	return p;
}

GByteArray *bswabe_cph_serialize(bswabe_cph_t *cph)
{
	GByteArray *b;

	b = g_byte_array_new();
	serialize_GT(b, cph->cs);
	serialize_G1(b, cph->c);
	serialize_policy(b, cph->p);

	return b;
}

bswabe_cph_t *bswabe_cph_unserialize(bswabe_pub_t *pub, GByteArray *b, int free)
{
	bswabe_cph_t *cph;
	int offset;

	cph = (bswabe_cph_t *)malloc(sizeof(bswabe_cph_t));
	offset = 0;

	unserialize_GT(b, &offset, &cph->cs);
	unserialize_G1(b, &offset, &cph->c);

	cph->p = unserialize_policy(pub, b, &offset);

	if (free)
		g_byte_array_free(b, 1);

	return cph;
}

void bswabe_pub_free(bswabe_pub_t *pub)
{
	mclBnG1_clear(&pub->g);
	mclBnG1_clear(&pub->h);
	mclBnG2_clear(&pub->gp);
	mclBnGT_clear(&pub->g_hat_alpha);
	mclBnGT_clear(&pub->p);
	free(pub);
}

void bswabe_msk_free(bswabe_msk_t *msk)
{
	mclBnFr_clear(&msk->beta);
	mclBnG2_clear(&msk->g_alpha);
	free(msk);
}

void bswabe_prv_free(bswabe_prv_t *prv)
{
	int i;

	mclBnG2_clear(&prv->d);

	for (i = 0; i < prv->comps->len; i++)
	{
		bswabe_prv_comp_t c;

		c = g_array_index(prv->comps, bswabe_prv_comp_t, i);
		free(c.attr);
		mclBnG2_clear(&c.d);
		mclBnG1_clear(&c.dp);
	}

	g_array_free(prv->comps, 1);

	free(prv);
}

void bswabe_policy_free(bswabe_policy_t *p)
{
	int i;

	if (p->attr)
	{
		free(p->attr);
		mclBnG1_clear(&p->c);
		mclBnG2_clear(&p->cp);
	}

	for (i = 0; i < p->children->len; i++)
		bswabe_policy_free(g_ptr_array_index(p->children, i));

	g_ptr_array_free(p->children, 1);

	free(p);
}

void bswabe_cph_free(bswabe_cph_t *cph)
{
	mclBnGT_clear(&cph->cs);
	mclBnG1_clear(&cph->c);
	bswabe_policy_free(cph->p);
}