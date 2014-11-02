
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define FLAG(n)	(1 >> (n))

enum {
	BZZD_CONNECT_FROM_LEFT =	FLAG(1),
	BZZD_CONNECT_FROM_RIGHT =	FLAG(2),
	BZZD_CONNECT_FROM_TOP =		FLAG(3),
	BZZD_CONNECT_FROM_BOTTOM =	FLAG(4),
	BZZD_CONNECT_ON_DIAGONALS =	FLAG(5),
	BZZD_CONNECT_ON_SIDES =		FLAG(6),
	BZZD_CONNECT_ON_BACK =		FLAG(7),
	BZZD_CONENCT_ONLY_OUTERS =	FLAG(8),
	BZZD_FLIPS_VERTICALLY =		FLAG(9),
	BZZD_FLIPS_HORIZONTALLY =	FLAG(10),
	BZZD_ROTATES_RIGHT =		FLAG(11),
	BZZD_ROTATES_LEFT =		FLAG(12),

	BZZD_CONNECT_FROM_ALL_SIDES =
		BZZD_CONNECT_FROM_LEFT | BZZD_CONNECT_FROM_RIGHT |
		BZZD_CONNECT_FROM_TOP | BZZD_CONNECT_FROM_BOTTOM,
	BZZD_CONNECT_ON_ALL_SIDES =
		BZZD_CONNECT_ON_DIAGONALS | BZZD_CONNECT_ON_SIDES |
		BZZD_CONNECT_ON_BACK,
	BZZD_FLIPS =
		BZZD_FLIPS_VERTICALLY | BZZD_FLIPS_HORIZONTALLY,
	BZZD_ROTATES =
		BZZD_ROTATES_RIGHT | BZZD_ROTATES_LEFT,
};

enum {
	LEFT_CONN =		FLAG(1),
	RIGHT_CONN =		FLAG(2),
	TOP_CONN =		FLAG(3),
	BOTTOM_CONN =		FLAG(4),
	TOP_LEFT_CONN =		FLAG(5),
	TOP_RIGHT_CONN =	FLAG(6),
	BOTTOM_LEFT_CONN =	FLAG(7),
	BOTTOM_RIGHT_CONN =	FLAG(8)
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
};

static void add_conns(struct bzzd_pattern *patt)
{
	char outers[patt->width > patt->height ? patt->width : patt->height];
	memset(outers, 0, sizeof(outers) / sizeof(outers[0]));

	int only_outers = patt->flags & BZZD_CONENCT_ONLY_OUTERS;
	int do_left = patt->flags & BZZD_CONNECT_FROM_LEFT;
	int do_right = patt->flags & BZZD_CONNECT_FROM_RIGHT;
	int do_top = patt->flags & BZZD_CONNECT_FROM_TOP;
	int do_bottom = patt->flags & BZZD_CONNECT_FROM_BOTTOM;
	// go down each flagged side
	// if only_outers, scan and tag each coord that's the most outer
	// if found > 0, add the flags to conns array based on flags
	if (do_left) {

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

	memcpy(patt->cells, from, width * height * sizeof(*patt->cells));
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

void bzzd_debug_pattern(struct bzzd_pattern *patt)
{
	char display[patt->height * 3][patt->width * 3];
	memset(display, ' ', patt->width * patt->height * 9 * sizeof(char));

	for (int y = patt->starty; y != patt->endy; y += patt->dy) {
		for (int x = patt->startx; x != patt->endx; x += patt->dx) {
			int cell = patt->cells[y * patt->width + x];
			int conn = patt->conns[y * patt->width + x];
			char *disp[3] = {
				&display[y * 3][x * 3],
				&display[y * 3 + 1][x * 3 + 1],
				&display[y * 3 + 2][x * 3 + 2],
			};

			if (cell) {
				disp[1][1] = 'O';
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
		0, 1, 0,
		1, 1, 1,
		0, 1, 0
	};

	patt = bzzd_build_pattern(blueprint, 3, 3, BZZD_CONNECT_FROM_ALL_SIDES);
	if (!patt) {
		fprintf(stderr, "failed to build patter\n");
		return -1;
	}

	bzzd_debug_pattern(patt);

	bzzd_demolish_pattern(patt);
	return 0;
}
