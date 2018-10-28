/*
 * Copyright (c) 2018 Manuele Conti
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * - Redistributions of source code must retain the above copyright
 *   notice, this list of conditions and the following disclaimer.
 * - Redistributions in binary form must reproduce the above copyright
 *   notice, this list of conditions and the following disclaimer in the
 *   documentation and/or other materials provided with the distribution.
 * - The name of the author may not be used to endorse or promote products
 *   derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/** @addtogroup libmath
 * @{
 */
/** @file sqrt mathematical function
 */

#include <math.h>
#include <stdint.h>

#include "internal.h"

static const double tiny = 1.0e-300;

double sqrt(double x)
{
	double z;
	int32_t sign = (int)0x80000000;
	int32_t ix0,s0,q,m,t,i;
	uint32_t r,t1,s1,ix1,q1;

	EXTRACT_WORDS(ix0, ix1, x);

	/* take care of Inf and NaN */
	if ((ix0&0x7ff00000) == 0x7ff00000) {
		return x*x + x;  /* sqrt(NaN)=NaN, sqrt(+inf)=+inf, sqrt(-inf)=sNaN */
	}
	/* take care of zero */
	if (ix0 <= 0) {
		if (((ix0&~sign)|ix1) == 0)
			return x;  /* sqrt(+-0) = +-0 */
		if (ix0 < 0)
			return (x-x)/(x-x);  /* sqrt(-ve) = sNaN */
	}
	/* normalize x */
	m = ix0>>20;
	if (m == 0) {  /* subnormal x */
		while (ix0 == 0) {
			m -= 21;
			ix0 |= (ix1>>11);
			ix1 <<= 21;
		}
		for (i=0; (ix0&0x00100000) == 0; i++)
			ix0<<=1;
		m -= i - 1;
		ix0 |= ix1>>(32-i);
		ix1 <<= i;
	}
	m -= 1023;    /* unbias exponent */
	ix0 = (ix0&0x000fffff)|0x00100000;
	if (m & 1) {  /* odd m, double x to make it even */
		ix0 += ix0 + ((ix1&sign)>>31);
		ix1 += ix1;
	}
	m >>= 1;      /* m = [m/2] */

	/* generate sqrt(x) bit by bit */
	ix0 += ix0 + ((ix1&sign)>>31);
	ix1 += ix1;
	q = q1 = s0 = s1 = 0;  /* [q,q1] = sqrt(x) */
	r = 0x00200000;        /* r = moving bit from right to left */

	while (r != 0) {
		t = s0 + r;
		if (t <= ix0) {
			s0   = t + r;
			ix0 -= t;
			q   += r;
		}
		ix0 += ix0 + ((ix1&sign)>>31);
		ix1 += ix1;
		r >>= 1;
	}

	r = sign;
	while (r != 0) {
		t1 = s1 + r;
		t  = s0;
		if (t < ix0 || (t == ix0 && t1 <= ix1)) {
			s1 = t1 + r;
			if ((t1&sign) == (uint32_t)sign && (s1&sign) == 0)
				s0++;
			ix0 -= t;
			if (ix1 < t1)
				ix0--;
			ix1 -= t1;
			q1 += r;
		}
		ix0 += ix0 + ((ix1&sign)>>31);
		ix1 += ix1;
		r >>= 1;
	}

	/* use floating add to find out rounding direction */
	if ((ix0|ix1) != 0) {
		z = 1.0 - tiny; /* raise inexact flag */
		if (z >= 1.0) {
			z = 1.0 + tiny;
			if (q1 == (uint32_t)0xffffffff) {
				q1 = 0;
				q++;
			} else if (z > 1.0) {
				if (q1 == (uint32_t)0xfffffffe)
					q++;
				q1 += 2;
			} else
				q1 += q1 & 1;
		}
	}
	ix0 = (q>>1) + 0x3fe00000;
	ix1 = q1>>1;
	if (q&1)
		ix1 |= sign;
	ix0 += m << 20;
	INSERT_WORDS(z, ix0, ix1);
	return z;
}

/** @}
 */
