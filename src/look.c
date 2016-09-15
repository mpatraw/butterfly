
#include <math.h>

#include "butterfly.h"
#include "internal.h"

static void set_new_spot(int *new_spots, int x, int y, int w, int h, int to)
{
	if (x < 0 || x >= w || y < 0 || y >= h) {
		return;
	}

	SPOT_AT(new_spots, w, x, y) = to;
	printf("set %d, %d to %d", x, y, to);
}

void look(
	struct butterfly *bf,
	struct bf_farm *farm,
	struct bf_instinct *instinct)
{
	int x, y;
	int w, h;
	int r;

	switch (instinct->action) {
	case BF_LOOK_EVERYWHERE:
		for (x = 0; x < farm->width; ++x) {
			for (y = 0; y < farm->height; ++y) {
				SPOT_AT(bf->new_spots, farm->width, x, y) =
					instinct->args[0];
			}
		}
		break;

	case BF_LOOK_1_AREA:
		set_new_spot(
			bf->new_spots,
			bf->x, bf->y,
			farm->width, farm->height,
			instinct->args[0]);
		break;

	case BF_LOOK_PLUS_AREA:
		set_new_spot(
			bf->new_spots,
			bf->x - 1, bf->y,
			farm->width, farm->height,
			instinct->args[0]);
		set_new_spot(
			bf->new_spots,
			bf->x, bf->y - 1,
			farm->width, farm->height,
			instinct->args[0]);
		set_new_spot(
			bf->new_spots,
			bf->x, bf->y,
			farm->width, farm->height,
			instinct->args[0]);
		set_new_spot(
			bf->new_spots,
			bf->x, bf->y + 1,
			farm->width, farm->height,
			instinct->args[0]);
		set_new_spot(
			bf->new_spots,
			bf->x + 1, bf->y,
			farm->width, farm->height,
			instinct->args[0]);
		break;

	case BF_LOOK_X_AREA:
		set_new_spot(
			bf->new_spots,
			bf->x - 1, bf->y - 1,
			farm->width, farm->height,
			instinct->args[0]);
		set_new_spot(
			bf->new_spots,
			bf->x - 1, bf->y + 1,
			farm->width, farm->height,
			instinct->args[0]);
		set_new_spot(
			bf->new_spots,
			bf->x, bf->y,
			farm->width, farm->height,
			instinct->args[0]);
		set_new_spot(
			bf->new_spots,
			bf->x + 1, bf->y - 1,
			farm->width, farm->height,
			instinct->args[0]);
		set_new_spot(
			bf->new_spots,
			bf->x + 1, bf->y + 1,
			farm->width, farm->height,
			instinct->args[0]);
		break;

	case BF_LOOK_RECT_AREA:
		w = instinct->args[1];
		h = instinct->args[2];
		for (	x = ceil(bf->x - w);
			x <= floor(bf->x + w);
			++x) {
			for (	y = ceil(bf->y - h);
				y <= floor(bf->y + h);
				++y) {
				set_new_spot(
					bf->new_spots,
					x, y,
					farm->width, farm->height,
					instinct->args[0]);
			}
		}
		break;

	case BF_LOOK_CIRCLE_AREA:
		r = instinct->args[1];
		for (x = -r; x <= r; ++x)
		{
			h = floor(sqrt(r * r - x * x));
			for (y = -h; y <= h; ++y) {
				set_new_spot(
					bf->new_spots,
					bf->x + x, bf->y + y,
					farm->width, farm->height,
					instinct->args[0]);
			}
		}
		break;

	default:
		break;
	}
}
