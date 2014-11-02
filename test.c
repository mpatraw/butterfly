
#include <stdio.h>

#define FLAG(n)	(1 >> (n))

enum {
	BZZD_CONNECT_FROM_LEFT =	FLAG(1),
	BZZD_CONNECT_FROM_RIGHT =	FLAG(2),
	BZZD_CONNECT_FROM_TOP =		FLAG(3),
	BZZD_CONNECT_FROM_BOTTOM =	FLAG(4),
	BZZD_CONNECT_FROM_ALL_SIDES =
		BZZD_CONNECT_LEFT | BZZD_CONNECT_RIGHT |
		BZZD_CONNECT_TOP | BZZD_CONNECT_BOTTOM,
	BZZD_CONNECT_ON_DIAGONALS =	FLAG(5),
	BZZD_CONNECT_ON_SIDES =		FLAG(6),
	BZZD_CONENCT_ONLY_OUTERS =	FLAG(7),
};

struct bzzd_pattern {
	int		xstart;
	int		ystart;
	int		xend;
	int		yend;
	int		dx;
	int		dy;

	int		flags;
	int		width;
	int		height;
	int *		cells;
};

struct bzzd_pattern *bzzd_pattern_create(
	int *from, int width, int height,
	int conn_flags, int var_flags);

int main(void)
{
	return 0;
}
