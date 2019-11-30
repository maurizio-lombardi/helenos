/*
 * Copyright (c) 2019 Jiri Svoboda
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

/** @addtogroup libgfx
 * @{
 */
/**
 * @file Coordinates
 */

#include <gfx/coord.h>
#include <macros.h>
#include <stdbool.h>

/** Add two vectors.
 *
 * @param a First vector
 * @param b Second vector
 * @param d Destination
 */
void gfx_coord2_add(gfx_coord2_t *a, gfx_coord2_t *b, gfx_coord2_t *d)
{
	d->x = a->x + b->x;
	d->y = a->y + b->y;
}

/** Subtract two vectors.
 *
 * @param a First vector
 * @param b Second vector
 * @param d Destination
 */
void gfx_coord2_subtract(gfx_coord2_t *a, gfx_coord2_t *b, gfx_coord2_t *d)
{
	d->x = a->x - b->x;
	d->y = a->y - b->y;
}

/** Sort points of a span.
 *
 * Sort the begin and end points so that the begin point has the lower
 * coordinate (i.e. if needed, the span is transposed, if not, it is simply
 * copied).
 *
 * @param s0 Source span start point
 * @param s1 Source span end point
 * @param d0 Destination span start point
 * @param d1 Destination span end point
 */
void gfx_span_points_sort(gfx_coord_t s0, gfx_coord_t s1, gfx_coord_t *d0,
    gfx_coord_t *d1)
{
	if (s0 <= s1) {
		*d0 = s0;
		*d1 = s1;
	} else {
		*d0 = s1 + 1;
		*d1 = s0 + 1;
	}
}

/** Move (translate) rectangle.
 *
 * @param trans Translation
 * @param src Source rectangle
 * @param dest Destination rectangle
 */
void gfx_rect_translate(gfx_coord2_t *trans, gfx_rect_t *src, gfx_rect_t *dest)
{
	gfx_coord2_add(trans, &src->p0, &dest->p0);
	gfx_coord2_add(trans, &src->p1, &dest->p1);
}

/** Compute envelope of two rectangles.
 *
 * Envelope is the minimal rectangle covering all pixels of both rectangles.
 */
void gfx_rect_envelope(gfx_rect_t *a, gfx_rect_t *b, gfx_rect_t *dest)
{
	gfx_rect_t sa, sb;

	if (gfx_rect_is_empty(a)) {
		*dest = *b;
		return;
	}

	if (gfx_rect_is_empty(b)) {
		*dest = *a;
		return;
	}

	/* a and b are both non-empty */

	gfx_rect_points_sort(a, &sa);
	gfx_rect_points_sort(b, &sb);

	dest->p0.x = min(sa.p0.x, sb.p0.x);
	dest->p0.y = min(sa.p0.y, sb.p0.y);
	dest->p1.x = max(sa.p1.x, sb.p1.x);
	dest->p1.y = max(sa.p1.y, sb.p1.y);
}

/** Sort points of a rectangle.
 *
 * Shuffle around coordinates of a rectangle so that p0.x < p1.x and
 * p0.y < p0.y.
 *
 * @param src Source rectangle
 * @param dest Destination (sorted) rectangle
 */
void gfx_rect_points_sort(gfx_rect_t *src, gfx_rect_t *dest)
{
	gfx_span_points_sort(src->p0.x, src->p1.x, &dest->p0.x, &dest->p1.x);
	gfx_span_points_sort(src->p0.y, src->p1.y, &dest->p0.y, &dest->p1.y);
}

/** Determine if rectangle contains no pixels
 *
 * @param rect Rectangle
 * @return @c true iff rectangle contains no pixels
 */
bool gfx_rect_is_empty(gfx_rect_t *rect)
{
	return rect->p0.x == rect->p1.x || rect->p0.y == rect->p1.y;
}

/** @}
 */