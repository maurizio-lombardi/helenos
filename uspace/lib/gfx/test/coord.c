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

#include <gfx/coord.h>
#include <pcut/pcut.h>

PCUT_INIT;

PCUT_TEST_SUITE(coord);

extern void gfx_coord2_add(gfx_coord2_t *, gfx_coord2_t *, gfx_coord2_t *);
extern void gfx_coord2_subtract(gfx_coord2_t *, gfx_coord2_t *, gfx_coord2_t *);
extern void gfx_rect_translate(gfx_coord2_t *, gfx_rect_t *, gfx_rect_t *);

/** gfx_coord2_add should add two coordinate vectors */
PCUT_TEST(coord2_add)
{
	gfx_coord2_t a, b;
	gfx_coord2_t d;

	a.x = 10;
	a.y = 11;
	b.x = 20;
	b.y = 22;

	gfx_coord2_add(&a, &b, &d);

	PCUT_ASSERT_EQUALS(a.x + b.x, d.x);
	PCUT_ASSERT_EQUALS(a.y + b.y, d.y);
}

/** gfx_coord2_subtract should subtract two coordinate vectors */
PCUT_TEST(coord2_subtract)
{
	gfx_coord2_t a, b;
	gfx_coord2_t d;

	a.x = 10;
	a.y = 11;
	b.x = 20;
	b.y = 22;

	gfx_coord2_subtract(&a, &b, &d);

	PCUT_ASSERT_EQUALS(a.x - b.x, d.x);
	PCUT_ASSERT_EQUALS(a.y - b.y, d.y);
}

/** gfx_rect_translate should translate rectangle */
PCUT_TEST(rect_translate)
{
	gfx_coord2_t offs;
	gfx_rect_t srect;
	gfx_rect_t drect;

	offs.x = 5;
	offs.y = 6;

	srect.p0.x = 10;
	srect.p0.y = 11;
	srect.p1.x = 20;
	srect.p1.y = 22;

	gfx_rect_translate(&offs, &srect, &drect);

	PCUT_ASSERT_EQUALS(offs.x + srect.p0.x, drect.p0.x);
	PCUT_ASSERT_EQUALS(offs.y + srect.p0.y, drect.p0.y);
	PCUT_ASSERT_EQUALS(offs.x + srect.p1.x, drect.p1.x);
	PCUT_ASSERT_EQUALS(offs.y + srect.p1.y, drect.p1.y);
}

/** Sorting span with lower start and higher end point results in the same span. */
PCUT_TEST(span_points_sort_asc)
{
	gfx_coord_t a, b;

	gfx_span_points_sort(1, 2, &a, &b);
	PCUT_ASSERT_EQUALS(1, a);
	PCUT_ASSERT_EQUALS(2, b);
}

/** Sorting span with same start and end point results in the same span. */
PCUT_TEST(span_points_sort_equal)
{
	gfx_coord_t a, b;

	gfx_span_points_sort(1, 1, &a, &b);
	PCUT_ASSERT_EQUALS(1, a);
	PCUT_ASSERT_EQUALS(1, b);
}

/** Sorting span with hight start and lower end point results in transposed span. */
PCUT_TEST(span_points_sort_decs)
{
	gfx_coord_t a, b;

	gfx_span_points_sort(1, 0, &a, &b);
	PCUT_ASSERT_EQUALS(1, a);
	PCUT_ASSERT_EQUALS(2, b);
}

/** Rectangle envelope with first rectangle empty should return the second rectangle. */
PCUT_TEST(rect_envelope_a_empty)
{
	gfx_rect_t a;
	gfx_rect_t b;
	gfx_rect_t e;

	a.p0.x = 0;
	a.p0.y = 0;
	a.p1.x = 0;
	a.p1.y = 0;

	b.p0.x = 1;
	b.p0.y = 2;
	b.p1.x = 3;
	b.p1.y = 4;

	gfx_rect_envelope(&a, &b, &e);
	PCUT_ASSERT_EQUALS(1, e.p0.x);
	PCUT_ASSERT_EQUALS(2, e.p0.y);
	PCUT_ASSERT_EQUALS(3, e.p1.x);
	PCUT_ASSERT_EQUALS(4, e.p1.y);
}

/** Rectangle envelope with second rectangle empty should return the first rectangle. */
PCUT_TEST(rect_envelope_b_empty)
{
	gfx_rect_t a;
	gfx_rect_t b;
	gfx_rect_t e;

	a.p0.x = 1;
	a.p0.y = 2;
	a.p1.x = 3;
	a.p1.y = 4;

	b.p0.x = 0;
	b.p0.y = 0;
	b.p1.x = 0;
	b.p1.y = 0;

	gfx_rect_envelope(&a, &b, &e);
	PCUT_ASSERT_EQUALS(1, e.p0.x);
	PCUT_ASSERT_EQUALS(2, e.p0.y);
	PCUT_ASSERT_EQUALS(3, e.p1.x);
	PCUT_ASSERT_EQUALS(4, e.p1.y);
}

/** Rectangle envelope, a has both coordinates lower than b */
PCUT_TEST(rect_envelope_nonempty_a_lt_b)
{
	gfx_rect_t a;
	gfx_rect_t b;
	gfx_rect_t e;

	a.p0.x = 1;
	a.p0.y = 2;
	a.p1.x = 3;
	a.p1.y = 4;

	b.p0.x = 5;
	b.p0.y = 6;
	b.p1.x = 7;
	b.p1.y = 8;

	gfx_rect_envelope(&a, &b, &e);
	PCUT_ASSERT_EQUALS(1, e.p0.x);
	PCUT_ASSERT_EQUALS(2, e.p0.y);
	PCUT_ASSERT_EQUALS(7, e.p1.x);
	PCUT_ASSERT_EQUALS(8, e.p1.y);
}

/** Rectangle envelope, a has both coordinates higher than b */
PCUT_TEST(rect_envelope_nonempty_a_gt_b)
{
	gfx_rect_t a;
	gfx_rect_t b;
	gfx_rect_t e;

	a.p0.x = 5;
	a.p0.y = 6;
	a.p1.x = 7;
	a.p1.y = 8;

	b.p0.x = 1;
	b.p0.y = 2;
	b.p1.x = 3;
	b.p1.y = 4;

	gfx_rect_envelope(&a, &b, &e);
	PCUT_ASSERT_EQUALS(1, e.p0.x);
	PCUT_ASSERT_EQUALS(2, e.p0.y);
	PCUT_ASSERT_EQUALS(7, e.p1.x);
	PCUT_ASSERT_EQUALS(8, e.p1.y);
}

/** Rectangle envelope, a is inside b */
PCUT_TEST(rect_envelope_nonempty_a_inside_b)
{
	gfx_rect_t a;
	gfx_rect_t b;
	gfx_rect_t e;

	a.p0.x = 1;
	a.p0.y = 2;
	a.p1.x = 7;
	a.p1.y = 8;

	b.p0.x = 3;
	b.p0.y = 4;
	b.p1.x = 5;
	b.p1.y = 6;

	gfx_rect_envelope(&a, &b, &e);
	PCUT_ASSERT_EQUALS(1, e.p0.x);
	PCUT_ASSERT_EQUALS(2, e.p0.y);
	PCUT_ASSERT_EQUALS(7, e.p1.x);
	PCUT_ASSERT_EQUALS(8, e.p1.y);
}

/** Rectangle envelope, b is inside a*/
PCUT_TEST(rect_envelope_nonempty_b_inside_a)
{
	gfx_rect_t a;
	gfx_rect_t b;
	gfx_rect_t e;

	a.p0.x = 3;
	a.p0.y = 4;
	a.p1.x = 5;
	a.p1.y = 6;

	b.p0.x = 1;
	b.p0.y = 2;
	b.p1.x = 7;
	b.p1.y = 8;

	gfx_rect_envelope(&a, &b, &e);
	PCUT_ASSERT_EQUALS(1, e.p0.x);
	PCUT_ASSERT_EQUALS(2, e.p0.y);
	PCUT_ASSERT_EQUALS(7, e.p1.x);
	PCUT_ASSERT_EQUALS(8, e.p1.y);
}

/** Rectangle envelope, a and b cross */
PCUT_TEST(rect_envelope_nonempty_a_crosses_b)
{
	gfx_rect_t a;
	gfx_rect_t b;
	gfx_rect_t e;

	a.p0.x = 1;
	a.p0.y = 2;
	a.p1.x = 4;
	a.p1.y = 3;

	b.p0.x = 2;
	b.p0.y = 1;
	b.p1.x = 3;
	b.p1.y = 4;

	gfx_rect_envelope(&a, &b, &e);
	PCUT_ASSERT_EQUALS(1, e.p0.x);
	PCUT_ASSERT_EQUALS(1, e.p0.y);
	PCUT_ASSERT_EQUALS(4, e.p1.x);
	PCUT_ASSERT_EQUALS(4, e.p1.y);
}

/** Sort span points that are already sorted should produde indentical points */
PCUT_TEST(rect_points_sort_sorted)
{
	gfx_coord_t s0, s1;

	gfx_span_points_sort(1, 2, &s0, &s1);
	PCUT_ASSERT_EQUALS(1, s0);
	PCUT_ASSERT_EQUALS(2, s1);
}

/** Sort span points that are reversed should transpose them */
PCUT_TEST(rect_points_sort_reversed)
{
	gfx_coord_t s0, s1;

	gfx_span_points_sort(2, 1, &s0, &s1);
	PCUT_ASSERT_EQUALS(2, s0);
	PCUT_ASSERT_EQUALS(3, s1);
}

/** gfx_rect_is_empty for straight rectangle with zero columns returns true */
PCUT_TEST(rect_is_empty_pos_x)
{
	gfx_rect_t rect;

	rect.p0.x = 1;
	rect.p0.y = 2;
	rect.p1.x = 1;
	rect.p1.y = 3;
	PCUT_ASSERT_TRUE(gfx_rect_is_empty(&rect));
}

/** gfx_rect_is_empty for straight rectangle with zero rows returns true */
PCUT_TEST(rect_is_empty_pos_y)
{
	gfx_rect_t rect;

	rect.p0.x = 1;
	rect.p0.y = 2;
	rect.p1.x = 2;
	rect.p1.y = 2;
	PCUT_ASSERT_TRUE(gfx_rect_is_empty(&rect));
}

/** gfx_rect_is_empty for staright non-empty rectangle returns false */
PCUT_TEST(rect_is_empty_neg)
{
	gfx_rect_t rect;

	rect.p0.x = 1;
	rect.p0.y = 2;
	rect.p1.x = 2;
	rect.p1.y = 3;
	PCUT_ASSERT_FALSE(gfx_rect_is_empty(&rect));
}

/** gfx_rect_is_empty for reverse non-empty rectangle returns false */
PCUT_TEST(rect_is_empty_reverse_neg)
{
	gfx_rect_t rect;

	rect.p0.x = 1;
	rect.p0.y = 2;
	rect.p1.x = 0;
	rect.p1.y = 1;
	PCUT_ASSERT_FALSE(gfx_rect_is_empty(&rect));
}

PCUT_EXPORT(coord);