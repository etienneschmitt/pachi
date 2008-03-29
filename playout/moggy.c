#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include "board.h"
#include "debug.h"
#include "playout.h"
#include "playout/moggy.h"
#include "random.h"

#define PLDEBUGL(n) DEBUGL_(p->debug_level, n)


/* Is this ladder breaker friendly for the one who catches ladder. */
static bool
ladder_catcher(struct board *b, int x, int y, enum stone laddered)
{
	enum stone breaker = board_atxy(b, x, y);
	return breaker == stone_other(laddered) || breaker == S_OFFBOARD;
}

static bool
ladder_catches(struct playout_policy *p, struct board *b, coord_t coord, group_t laddered)
{
	/* This is very trivial and gets a lot of corner cases wrong.
	 * We need this to be just very fast. */
	//fprintf(stderr, "ladder check\n");

	enum stone lcolor = board_at(b, laddered);

	/* Figure out the ladder direction */
	int x = coord_x(coord, b), y = coord_y(coord, b);
	int xd, yd;
	xd = board_atxy(b, x + 1, y) == S_NONE ? 1 : board_atxy(b, x - 1, y) == S_NONE ? -1 : 0;
	yd = board_atxy(b, x, y + 1) == S_NONE ? 1 : board_atxy(b, x, y - 1) == S_NONE ? -1 : 0;

	/* We do only tight ladders, not loose ladders. Furthermore,
	 * the ladders need to be simple:
	 * . X .             . . X
	 * c O X supported   . c O unsupported
	 * X # #             X O #
	 */

	/* For given (xd,yd), we have two possibilities where to move
	 * next. Consider (-1,1):
	 * n X .   n c X
	 * c O X   X O #
	 * X # #   . X #
	 */
	if (!xd || !yd || !(ladder_catcher(b, x - xd, y, lcolor) ^ ladder_catcher(b, x, y - yd, lcolor))) {
		/* Silly situation, probably non-simple ladder or suicide. */
		/* TODO: In case of basic non-simple ladder, play out both variants. */
		if (PLDEBUGL(5))
			fprintf(stderr, "non-simple ladder\n");
		return false;
	}

#define ladder_check(xd1_, yd1_, xd2_, yd2_)	\
	if (board_atxy(b, x, y) != S_NONE) { \
		/* Did we hit a stone when playing out ladder? */ \
		if (ladder_catcher(b, x, y, lcolor)) \
			return true; /* ladder works */ \
		if (board_group_info(b, group_atxy(b, x, y)).lib[0] > 0) \
			return false; /* friend that's not in atari himself */ \
	} else { \
		/* No. So we are at new position. \
		 * We need to check indirect ladder breakers. */ \
		/* . 2 x . . \
		 * . x o O 1 <- only at O we can check for o at 2 \
		 * x o o x .    otherwise x at O would be still deadly \
		 * o o x . . \
		 * We check for o and x at 1, these are vital. \
		 * We check only for o at 2; x at 2 would mean we \
		 * need to fork (one step earlier). */ \
		enum stone s1 = board_atxy(b, x + (xd1_), y + (yd1_)); \
		if (s1 == lcolor) return false; \
		if (s1 == stone_other(lcolor)) return true; \
		enum stone s2 = board_atxy(b, x + (xd2_), y + (yd2_)); \
		if (s2 == lcolor) return false; \
	}
#define ladder_horiz	do { if (PLDEBUGL(6)) fprintf(stderr, "%d,%d horiz step %d\n", x, y, xd); x += xd; ladder_check(xd, 0, -2 * xd, yd); } while (0)
#define ladder_vert	do { if (PLDEBUGL(6)) fprintf(stderr, "%d,%d vert step %d\n", x, y, yd); y += yd; ladder_check(0, yd, xd, -2 * yd); } while (0)

	if (ladder_catcher(b, x - xd, y, lcolor))
		ladder_horiz;
	do {
		ladder_vert;
		ladder_horiz;
	} while (1);
}


static coord_t
group_atari_check(struct playout_policy *p, struct board *b, group_t group)
{
	enum stone color = board_at(b, group);
	coord_t lib = board_group_info(b, group).lib[0];
	if (board_at(b, group) == S_OFFBOARD) {
		/* Bogus group. */
		return pass;
	}
	if (PLDEBUGL(4))
		fprintf(stderr, "atariiiiiiiii %s of color %d\n", coord2sstr(lib, b), color);
	assert(board_at(b, lib) == S_NONE);

	/* Do not suicide... */
	if (!valid_escape_route(b, color, lib))
		return pass;
	if (PLDEBUGL(4))
		fprintf(stderr, "...escape route valid\n");
	
	/* ...or play out ladders. */
	if (ladder_catches(p, b, lib, group))
		return pass;
	if (PLDEBUGL(4))
		fprintf(stderr, "...no ladder\n");

	return lib;
}

static coord_t
global_atari_check(struct playout_policy *p, struct board *b)
{
	if (b->clen == 0)
		return pass;

	int g_base = fast_random(b->clen);
	for (int g = g_base; g < b->clen; g++) {
		coord_t c = group_atari_check(p, b, b->c[g]);
		if (!is_pass(c))
			return c;
	}
	for (int g = 0; g < g_base; g++) {
		coord_t c = group_atari_check(p, b, b->c[g]);
		if (!is_pass(c))
			return c;
	}
	return pass;
}

coord_t
playout_moggy_choose(struct playout_policy *p, struct board *b, enum stone our_real_color)
{
	coord_t c;
	if (PLDEBUGL(4))
		board_print(b, stderr);

	/* Local checks */

	if (!is_pass(b->last_move.coord)) {
		// TODO
	}

	/* Global checks */

	/* Any groups in atari? */
	c = global_atari_check(p, b);
	if (!is_pass(c))
		return c;

	return pass;
}

float
playout_moggy_assess(struct playout_policy *p, struct board *b, struct move *m)
{
	if (is_pass(m->coord))
		return NAN;

	if (PLDEBUGL(4))
		board_print(b, stderr);

	/* Are we dealing with atari? */
	foreach_neighbor(b, m->coord, {
		if (board_group_info(b, group_at(b, c)).libs == 1
		    && group_atari_check(p, b, group_at(b, c)) == m->coord)
			return 1.0;
	});

	return NAN;
}


struct playout_policy *
playout_moggy_init(char *arg)
{
	struct playout_policy *p = calloc(1, sizeof(*p));
	p->choose = playout_moggy_choose;
	p->assess = playout_moggy_assess;

	if (arg) {
		char *optspec, *next = arg;
		while (*next) {
			optspec = next;
			next += strcspn(next, ":");
			if (*next) { *next++ = 0; } else { *next = 0; }

			char *optname = optspec;
			char *optval = strchr(optspec, '=');
			if (optval) *optval++ = 0;

			if (0) {
				// No parameters yet.
			} else {
				fprintf(stderr, "playout-moggy: Invalid policy argument %s or missing value\n", optname);
			}
		}
	}

	return p;
}
