/***************************************************************************
 *   Copyright (C) 2004-2005 by Naresh [Kamil Klimek]                      *
 *   naresh@tlen.pl                                                        *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#include "auth.h"

#ifdef _MSC_VER
#define snprintf _snprintf
#endif

//-----------------------------------------------------
// Kod zaczerpniety z libtlen
//-----------------------------------------------------

static void shaHashBlock (j_SHA_CTX * ctx);

void shaInit (j_SHA_CTX * ctx) {
	int i;

	ctx->lenW = 0;
	ctx->sizeHi = ctx->sizeLo = 0;

	/* Initialize H with the magic constants (see FIPS180 for constants)
	 */
	ctx->H[0] = 0x67452301L;
	ctx->H[1] = 0xefcdab89L;
	ctx->H[2] = 0x98badcfeL;
	ctx->H[3] = 0x10325476L;
	ctx->H[4] = 0xc3d2e1f0L;

	for (i = 0; i < 80; i++)
		ctx->W[i] = 0;
}


void shaUpdate (j_SHA_CTX * ctx, unsigned char *dataIn, int len) {
	int i;

	// Read the data into W and process blocks as they get full
	for (i = 0; i < len; i++) {
		ctx->W[ctx->lenW / 4] <<= 8;
		ctx->W[ctx->lenW / 4] |= (unsigned long) dataIn[i];
		if ((++ctx->lenW) % 64 == 0) {
			shaHashBlock (ctx);
			ctx->lenW = 0;
		}
		ctx->sizeLo += 8;
		ctx->sizeHi += (ctx->sizeLo < 8);
	}
}


void shaFinal (j_SHA_CTX * ctx, unsigned char hashout[20]) {
	unsigned char pad0x80 = 0x80;
	unsigned char pad0x00 = 0x00;
	unsigned char padlen[8];
	int i;

	// Pad with a binary 1 (e.g. 0x80), then zeroes, then length
	padlen[0] = (unsigned char) ((ctx->sizeHi >> 24) & 255);
	padlen[1] = (unsigned char) ((ctx->sizeHi >> 16) & 255);
	padlen[2] = (unsigned char) ((ctx->sizeHi >> 8) & 255);
	padlen[3] = (unsigned char) ((ctx->sizeHi >> 0) & 255);
	padlen[4] = (unsigned char) ((ctx->sizeLo >> 24) & 255);
	padlen[5] = (unsigned char) ((ctx->sizeLo >> 16) & 255);
	padlen[6] = (unsigned char) ((ctx->sizeLo >> 8) & 255);
	padlen[7] = (unsigned char) ((ctx->sizeLo >> 0) & 255);
	shaUpdate (ctx, &pad0x80, 1);
	while (ctx->lenW != 56)
		shaUpdate (ctx, &pad0x00, 1);
	shaUpdate (ctx, padlen, 8);

	// Output hash
	for (i = 0; i < 20; i++) {
		hashout[i] = (unsigned char) (ctx->H[i / 4] >> 24);
		ctx->H[i / 4] <<= 8;
	}

	//  Re-initialize the context (also zeroizes contents)
	shaInit (ctx);
}


void shaBlock (unsigned char *dataIn, int len, unsigned char hashout[20]) {
	j_SHA_CTX ctx;

	shaInit (&ctx);
	shaUpdate (&ctx, dataIn, len);
	shaFinal (&ctx, hashout);
}

#define SHA_ROTL(X,n) ((((X) << (n)) | ((X) >> (32-(n)))) & 0xffffffffL)

static void shaHashBlock (j_SHA_CTX * ctx) {
	int t;
	unsigned long A, B, C, D, E, TEMP;

	for (t = 16; t <= 79; t++)
		ctx->W[t] =
			SHA_ROTL (ctx->W[t - 3] ^ ctx->W[t - 8] ^ ctx->
				  W[t - 14] ^ ctx->W[t - 16], 1);

	A = ctx->H[0];
	B = ctx->H[1];
	C = ctx->H[2];
	D = ctx->H[3];
	E = ctx->H[4];

	for (t = 0; t <= 19; t++) {
		TEMP = (SHA_ROTL (A, 5) + (((C ^ D) & B) ^ D) + E +
			ctx->W[t] + 0x5a827999L) & 0xffffffffL;
		E = D;
		D = C;
		C = SHA_ROTL (B, 30);
		B = A;
		A = TEMP;
	}
	for (t = 20; t <= 39; t++) {
		TEMP = (SHA_ROTL (A, 5) + (B ^ C ^ D) + E + ctx->W[t] +
			0x6ed9eba1L) & 0xffffffffL;
		E = D;
		D = C;
		C = SHA_ROTL (B, 30);
		B = A;
		A = TEMP;
	}
	for (t = 40; t <= 59; t++) {
		TEMP = (SHA_ROTL (A, 5) + ((B & C) | (D & (B | C))) + E +
			ctx->W[t] + 0x8f1bbcdcL) & 0xffffffffL;
		E = D;
		D = C;
		C = SHA_ROTL (B, 30);
		B = A;
		A = TEMP;
	}
	for (t = 60; t <= 79; t++) {
		TEMP = (SHA_ROTL (A, 5) + (B ^ C ^ D) + E + ctx->W[t] +
			0xca62c1d6L) & 0xffffffffL;
		E = D;
		D = C;
		C = SHA_ROTL (B, 30);
		B = A;
		A = TEMP;
	}

	ctx->H[0] += A;
	ctx->H[1] += B;
	ctx->H[2] += C;
	ctx->H[3] += D;
	ctx->H[4] += E;
}

/*----------------------------------------------------------------------------
 *
 * This code added by Thomas "temas" Muldowney for Jabber compatability
 *
 *---------------------------------------------------------------------------*/
char* shahash (char *str) {
	static char final[41];
	char *pos;
	unsigned char hashval[20];
	int x;

	if (!str || strlen (str) == 0)
		return NULL;

	shaBlock ((unsigned char *) str, strlen (str), hashval);

	pos = final;
	for (x = 0; x < 20; x++) {
		snprintf (pos, 3, "%02x", hashval[x]);
		pos += 2;
	}
	return (char *) final;
}

void shahash_r (const char *str, char hashbuf[41]) {
	int x;
	char *pos;
	unsigned char hashval[20];

	if (!str || strlen (str) == 0)
		return;

	shaBlock ((unsigned char *) str, strlen (str), hashval);

	pos = hashbuf;
	for (x = 0; x < 20; x++) {
		snprintf (pos, 3, "%02x", hashval[x]);
		pos += 2;
	}

	return;
}

/*********************************************/
// Ten kod dodany przeze mnie i Piotra Paw�owa

void calc_passcode (const char *pass, char *code) {
	int magic1 = 0x50305735, magic2 = 0x12345671, sum = 7;
	char z;
	while ((z = *pass++) != 0) {
		if (z == ' ')
			continue;
		if (z == '\t')
			continue;
		magic1 ^= (((magic1 & 0x3f) + sum) * z) + (magic1 << 8);
		magic2 += (magic2 << 8) ^ magic1;
		sum += z;
	}
	magic1 &= 0x7fffffff;
	magic2 &= 0x7fffffff;
	sprintf (code, "%08x%08x", magic1, magic2);
}

/*
 * tlen_hash()
 *
 * Generuje hash potrzebny do zalogowania si�
 *
 * - pass - has�o
 * - id - identyfikator sesji
 *
 * Funkcja wewn�trzna na potrzeby tlen_authorize
 *
 */

char *tlen_hash (const char *pass, const char *id) {
	char passcode[17];
	char dohasha[25];
	char *hash;
	hash = (char *) malloc (41);
	calc_passcode (pass, passcode);
	strcpy (dohasha, id);
	strcat (dohasha, passcode);
        dohasha[24]=0;
	shahash_r (dohasha, hash);
	return (char *) hash;
}
