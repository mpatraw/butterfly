
#include <math.h>

#include "butterfly.h"
#include "internal.h"

static void set_new_spot(
	struct bf_farm *farm,
	int x, int y,
	int to)
{
	struct butterfly *bf;

	if (x < 0 || x >= farm->width || y < 0 || y >= farm->height) {
		return;
	}

	bf = farm->butterfly;

	if (	bf->config && bf->config->cancel_on_looking_at_safe &&
		SPOT_AT(farm->spots, farm->width, x, y) > 0) {
		farm->error = BF_CANCEL;
	}

	SPOT_AT(bf->new_spots, farm->width, x, y) = to;
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
				set_new_spot(
					farm,
					x, y,
					instinct->args[0]);
			}
		}
		break;

	case BF_LOOK_1_AREA:
		set_new_spot(
			farm,
			bf->x, bf->y,
			instinct->args[0]);
		break;

	case BF_LOOK_PLUS_AREA:
		set_new_spot(
			farm,
			bf->x - 1, bf->y,
			instinct->args[0]);
		set_new_spot(
			farm,
			bf->x, bf->y - 1,
			instinct->args[0]);
		set_new_spot(
			farm,
			bf->x, bf->y,
			instinct->args[0]);
		set_new_spot(
			farm,
			bf->x, bf->y + 1,
			instinct->args[0]);
		set_new_spot(
			farm,
			bf->x + 1, bf->y,
			instinct->args[0]);
		break;

	case BF_LOOK_X_AREA:
		set_new_spot(
			farm,
			bf->x - 1, bf->y - 1,
			instinct->args[0]);
		set_new_spot(
			farm,
			bf->x - 1, bf->y + 1,
			instinct->args[0]);
		set_new_spot(
			farm,
			bf->x, bf->y,
			instinct->args[0]);
		set_new_spot(
			farm,
			bf->x + 1, bf->y - 1,
			instinct->args[0]);
		set_new_spot(
			farm,
			bf->x + 1, bf->y + 1,
			instinct->args[0]);
		break;

	case BF_LOOK_RECT_AREA:
		w = instinct->args[1];
		h = instinct->args[2];
		for (	x = bf->x - w;
			x <= bf->x + w;
			++x) {
			for (	y = bf->y - h;
				y <= bf->y + h;
				++y) {
				set_new_spot(
					farm,
					x, y,
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
					farm,
					bf->x + x, bf->y + y,
					instinct->args[0]);
			}
		}
		break;

	default:
		break;
	}
}
