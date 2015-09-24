
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define MIN(a, b) ((a) < (b) ? (a) : (b))

#define FLAG(n)	(1 << (n))

enum {
	BZZD_CONNECT_FROM_LEFT =	FLAG(0),
	BZZD_CONNECT_FROM_RIGHT =	FLAG(1),
	BZZD_CONNECT_FROM_TOP =		FLAG(2),
	BZZD_CONNECT_FROM_BOTTOM =	FLAG(3),
	BZZD_CONNECT_ON_FRONT =		FLAG(4),
	BZZD_CONNECT_ON_DIAGONALS =	FLAG(5),
	BZZD_CONNECT_ON_SIDES =		FLAG(6),
	BZZD_CONNECT_ON_BACK =		FLAG(7),
	BZZD_CONNECT_ONLY_OUTERMOSTS =	FLAG(8),
	BZZD_FLIPS_VERTICALLY =		FLAG(9),
	BZZD_FLIPS_HORIZONTALLY =	FLAG(10),

	BZZD_CONNECT_FROM_ALL_SIDES =
		BZZD_CONNECT_FROM_LEFT | BZZD_CONNECT_FROM_RIGHT |
		BZZD_CONNECT_FROM_TOP | BZZD_CONNECT_FROM_BOTTOM,
	BZZD_CONNECT_ON_ALL_SIDES =
		BZZD_CONNECT_ON_FRONT | BZZD_CONNECT_ON_BACK |
		BZZD_CONNECT_ON_DIAGONALS | BZZD_CONNECT_ON_SIDES,
	BZZD_CONNECT_ON_FRONT_SIDES =
		BZZD_CONNECT_ON_FRONT | BZZD_CONNECT_ON_SIDES,
	BZZD_FLIPS =
		BZZD_FLIPS_VERTICALLY | BZZD_FLIPS_HORIZONTALLY
};

/* Turns a side (LEFT, RIGHT, TOP, BOTTOM) into a FROM_ flag. */
#define CONN_FROM(side) (1 << (side))

enum {
	LEFT_CONN =		FLAG(0),
	RIGHT_CONN =		FLAG(1),
	TOP_CONN =		FLAG(2),
	BOTTOM_CONN =		FLAG(3),
	TOP_LEFT_CONN =		FLAG(4),
	TOP_RIGHT_CONN =	FLAG(5),
	BOTTOM_LEFT_CONN =	FLAG(6),
	BOTTOM_RIGHT_CONN =	FLAG(7)
};

#define CONN(side) (1 << (side))

enum {
	LEFT, RIGHT, TOP, BOTTOM,
	NSIDES
};

struct bzzd_pattern {
	int		startx;
	int		starty;
	int		endx;
	int		endy;
	int		dx;
	int		dy;

	int		flags;
	int		width;
	int		height;
	int		*cells;
	unsigned char	*conns;
	unsigned char	*outercells;
	unsigned char	*outermosts;
};

static int starts_from_0(int side)
{
	return (side % 2) == 0;
}

static int opposite_side(int side)
{
	if (side == LEFT) { return RIGHT; }
	if (side == RIGHT) { return LEFT; }
	if (side == TOP) { return BOTTOM; }
	if (side == BOTTOM) { return TOP; }
	return side;
}

static void find_outers(struct bzzd_pattern *patt)
{
	unsigned char mostval[NSIDES];
	for (int s = 0; s < NSIDES; ++s) {
		if (starts_from_0(s)) {
			mostval[s] = 0xff;
		} else {
			mostval[s] = 0x00;
		}
	}

	int lside = MAX(patt->width, patt->height);

	unsigned char *outers[NSIDES] = {
		&patt->outercells[0 * lside],
		&patt->outercells[1 * lside],
		&patt->outercells[2 * lside],
		&patt->outercells[3 * lside]
	};
	unsigned char *outermost[NSIDES] = {
		&patt->outermosts[0 * lside],
		&patt->outermosts[1 * lside],
		&patt->outermosts[2 * lside],
		&patt->outermosts[3 * lside]
	};

	for (int y = 0; y < patt->height; ++y) {
		for (int x = 0; x < patt->width; ++x) {
			int cell = patt->cells[y * patt->width + x];
			if (!cell) {
				continue;
			}

			outers[LEFT][y] = MIN(outers[LEFT][y], x);
			outers[RIGHT][y] = MAX(outers[RIGHT][y], x);
			outers[TOP][x] = MIN(outers[TOP][x], y);
			outers[BOTTOM][x] = MAX(outers[BOTTOM][x], y);

			mostval[LEFT] = MIN(mostval[LEFT], outers[LEFT][y]);
			mostval[RIGHT] = MAX(mostval[RIGHT], outers[RIGHT][y]);
			mostval[TOP] = MIN(mostval[TOP], outers[TOP][x]);
			mostval[BOTTOM] = MAX(
				mostval[BOTTOM], outers[BOTTOM][x]);
		}
	}

	for (int y = 0; y < patt->height; ++y) {
		for (int x = 0; x < patt->width; ++x) {
			outermost[LEFT][y] =
				mostval[LEFT] == outers[LEFT][y];
			outermost[RIGHT][y] =
				mostval[RIGHT] == outers[RIGHT][y];
			outermost[TOP][x] =
				mostval[TOP] == outers[TOP][x];
			outermost[BOTTOM][x] =
				mostval[BOTTOM] == outers[BOTTOM][x];
		}
	}
}

static int should_conn(struct bzzd_pattern *patt, int side, int x, int y)
{
	int *idx = NULL, *val = NULL;
	if (side == LEFT || side == RIGHT) {
		idx = &y;
		val = &x;
	} else if (side == TOP || side == BOTTOM) {
		idx = &x;
		val = &y;
	} else {
		return 0;
	}

	int lside = MAX(patt->width, patt->height);

	int flag = patt->flags & CONN_FROM(side);
	int is_outercell = patt->outercells[side * lside + *idx] == *val;
	int is_outermost = patt->outermosts[side * lside + *idx];
	int only_outermost = patt->flags & BZZD_CONNECT_ONLY_OUTERMOSTS;
	int check = (only_outermost && is_outermost) || !only_outermost;

	return flag && is_outercell && check;
}

static unsigned char sides_from(int side)
{
	if (side == LEFT || side == RIGHT) { return TOP_CONN | BOTTOM_CONN; }
	if (side == TOP || side == BOTTOM) { return LEFT_CONN | RIGHT_CONN; }
	return 0;
}

static unsigned char diagonals_from(int side)
{
	if (side == LEFT) { return TOP_LEFT_CONN | BOTTOM_LEFT_CONN; }
	if (side == RIGHT) { return TOP_RIGHT_CONN | BOTTOM_RIGHT_CONN; }
	if (side == TOP) { return TOP_RIGHT_CONN | TOP_LEFT_CONN; }
	if (side == BOTTOM) { return BOTTOM_RIGHT_CONN | BOTTOM_LEFT_CONN; }
	return 0;
}

static unsigned char back_from(int side)
{
	return CONN(opposite_side(side));
}

static unsigned char get_conns(struct bzzd_pattern *patt, int side)
{
	unsigned char conns = 0;
	if (patt->flags & BZZD_CONNECT_ON_FRONT) {
		conns |= CONN(side);
	}
	if (patt->flags & BZZD_CONNECT_ON_SIDES) {
		conns |= sides_from(side);
	}
	if (patt->flags & BZZD_CONNECT_ON_DIAGONALS) {
		conns |= diagonals_from(side);
	}
	if (patt->flags & BZZD_CONNECT_ON_BACK) {
		if (patt->flags & BZZD_CONNECT_ON_DIAGONALS) {
			int opp = opposite_side(side);
			conns |= diagonals_from(opp);
		}
		conns |= back_from(side);
	}
	return conns;
}

static void add_conns(struct bzzd_pattern *patt)
{
	find_outers(patt);

	for (int y = 0; y < patt->width; ++y) {
		for (int x = 0; x < patt->height; ++x) {
			unsigned char *conn = &patt->conns[y * patt->width + x];

			if (should_conn(patt, LEFT, x, y)) {
				*conn |= get_conns(patt, LEFT);
			}

			if (should_conn(patt, RIGHT, x, y)) {
				*conn |= get_conns(patt, RIGHT);
			}

			if (should_conn(patt, TOP, x, y)) {
				*conn |= get_conns(patt, TOP);
			}

			if (should_conn(patt, BOTTOM, x, y)) {
				*conn |= get_conns(patt, BOTTOM);
			}
		}
	}
}

struct bzzd_pattern *bzzd_build_pattern(
	int *from, int width, int height, int flags)
{
	struct bzzd_pattern *patt;

	patt = calloc(1, sizeof(*patt));
	if (!patt) {
		goto patt_alloc_failure;
	}

	patt->cells = calloc(width * height, sizeof(*patt->cells));
	if (!patt->cells) {
		goto cells_alloc_failure;
	}

	patt->conns = calloc(width * height, sizeof(*patt->conns));
	if (!patt->conns) {
		goto conns_alloc_failure;
	}

	int lside = MAX(width, height);

	patt->outercells = calloc(NSIDES * lside, sizeof(*patt->outercells));
	if (!patt->outercells) {
		goto outercells_alloc_failure;
	}

	patt->outermosts = calloc(NSIDES * lside, sizeof(*patt->outermosts));
	if (!patt->outermosts) {
		goto outermosts_alloc_failure;
	}

	memcpy(patt->cells, from, width * height * sizeof(*patt->cells));

	for (int s = 0; s < NSIDES; ++s) {
		for (int xy = 0; xy < lside; ++xy) {
			if (starts_from_0(s)) {
				patt->outercells[s * lside + xy] = 0xff;
			} else {
				patt->outercells[s * lside + xy] = 0x00;
			}
		}
	}

	memset(patt->outermosts, 0, NSIDES * lside * sizeof(*patt->outermosts));

	patt->width = width;
	patt->height = height;
	patt->flags = flags;

	patt->startx = 0;
	patt->starty = 0;
	patt->endx = patt->width;
	patt->endy = patt->height;
	patt->dx = 1;
	patt->dy = 1;

	add_conns(patt);

	return patt;

outermosts_alloc_failure:
	free(patt->outercells);
outercells_alloc_failure:
	free(patt->conns);
conns_alloc_failure:
	free(patt->cells);
cells_alloc_failure:
	free(patt);
	patt = NULL;
patt_alloc_failure:
	return patt;
}

void bzzd_demolish_pattern(struct bzzd_pattern *patt)
{
	free(patt->conns);
	free(patt->cells);
	free(patt);
}

int bzzd_get_pattern_cell(struct bzzd_pattern *patt, int x, int y)
{
	x = abs(patt->startx - x);
	y = abs(patt->starty - y);
	return patt->cells[y * patt->width + x];
}

unsigned char bzzd_get_pattern_conns(struct bzzd_pattern *patt, int x, int y)
{
	x = abs(patt->startx - x);
	y = abs(patt->starty - y);
	return patt->conns[y * patt->width + x];
}

void bzzd_flipv(struct bzzd_pattern *patt)
{
	int tmp = patt->endy;
	patt->endy = patt->starty - patt->dy;
	patt->starty = tmp - patt->dy;
	patt->dy *= -1;
}

void bzzd_flipx(struct bzzd_pattern *patt)
{
	int tmp = patt->endx;
	patt->endx = patt->startx - patt->dx;
	patt->startx = tmp - patt->dx;
	patt->dx *= -1;
}

void bzzd_debug_pattern(struct bzzd_pattern *patt)
{
	unsigned char display[patt->height * 3][patt->width * 3];
	memset(display, ' ', patt->width * patt->height * 9 * sizeof(unsigned char));

	for (int y = patt->starty; y != patt->endy; y += patt->dy) {
		for (int x = patt->startx; x != patt->endx; x += patt->dx) {
			int cell = bzzd_get_pattern_cell(patt, x, y);
			unsigned char conn = bzzd_get_pattern_conns(patt, x, y);
			unsigned char *disp[3] = {
				&display[y * 3 + 0][x * 3],
				&display[y * 3 + 1][x * 3],
				&display[y * 3 + 2][x * 3],
			};

			if (cell) {
				disp[1][1] = 'O';
			}

			if (conn & LEFT_CONN) {
				disp[1][0] = '-';
			}

			if (conn & RIGHT_CONN) {
				disp[1][2] = '-';
			}

			if (conn & TOP_CONN) {
				disp[0][1] = '|';
			}

			if (conn & BOTTOM_CONN) {
				disp[2][1] = '|';
			}

			if (conn & TOP_LEFT_CONN) {
				disp[0][0] = '\\';
			}

			if (conn & TOP_RIGHT_CONN) {
				disp[0][2] = '/';
			}

			if (conn & BOTTOM_LEFT_CONN) {
				disp[2][0] = '/';
			}

			if (conn & BOTTOM_RIGHT_CONN) {
				disp[2][2] = '\\';
			}
		}
	}

	for (int y = 0; y < patt->height * 3; ++y) {
		for (int x = 0; x < patt->width * 3; ++x) {
			printf("%c", display[y][x]);
		}
		printf("\n");
	}
}

int main(void)
{
	struct bzzd_pattern *patt;
	int blueprint[] = {
		0, 1, 0, 1,
		1, 1, 1, 1,
		0, 1, 0, 1,
		1, 1, 1, 1
	};

	patt = bzzd_build_pattern(blueprint, 4, 4,
		BZZD_CONNECT_FROM_ALL_SIDES | BZZD_CONNECT_ON_FRONT |
		BZZD_CONNECT_ONLY_OUTERMOSTS | BZZD_CONNECT_ON_DIAGONALS);
	if (!patt) {
		fprintf(stderr, "failed to build pattern\n");
		return -1;
	}

	bzzd_debug_pattern(patt);
	bzzd_flipv(patt);
	printf("#########################\n");
	bzzd_debug_pattern(patt);

	bzzd_demolish_pattern(patt);
	return 0;
}
