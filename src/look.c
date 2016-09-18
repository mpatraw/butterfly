
#include <math.h>
#include <stdlib.h>

#include "butterfly.h"
#include "internal.h"

static void set_new_spot(
	struct bf_farm *farm,
	int x, int y,
	int to)
{
	static int dir8[8][2] = {
		{-1, -1},
		{-1,  0},
		{-1,  1},
		{ 0, -1},
		{ 0,  1},
		{ 1, -1},
		{ 1,  0},
		{ 1,  1},
	};
	static int dir4p[4][2] = {
		{-1,  0},
		{ 0, -1},
		{ 0,  1},
		{ 1,  0}
	};
	static int dir4x[4][2] = {
		{-1, -1},
		{ 1, -1},
		{-1,  1},
		{ 1, -1}
	};
	struct butterfly *bf;
	int d, dx, dy, s;

	bf = farm->butterfly;

	if (!IN_BOUNDS(farm, x, y)) {
		if (bf->config && bf->config->error_on_looking_outside_farm) {
			farm->error = BF_ERROR_CANCEL;
		}
		return;
	}

	if (	bf->config && bf->config->error_on_looking_at_safe &&
		IS_SAFE_AT(farm, x, y)) {
		farm->error = BF_ERROR_CANCEL;
	}

	if (bf->config && bf->config->error_on_looking_at_safe_neighbor_4p) {
		for (d = 0; d < 4; ++d) {
			dx = x + dir4p[d][0];
			dy = y + dir4p[d][1];
			if (!IN_BOUNDS(farm, dx, dy)) {
				continue;
			}
			s = SPOT_AT(farm->spots, farm->width, dx, dy);
			if (IS_SAFE(farm, s)) {
				farm->error = BF_ERROR_CANCEL;
			}
		}
	}

	if (bf->config && bf->config->error_on_looking_at_safe_neighbor_4x) {
		for (d = 0; d < 4; ++d) {
			dx = x + dir4x[d][0];
			dy = y + dir4x[d][1];
			if (!IN_BOUNDS(farm, dx, dy)) {
				continue;
			}
			s = SPOT_AT(farm->spots, farm->width, dx, dy);
			if (IS_SAFE(farm, s)) {
				farm->error = BF_ERROR_CANCEL;
			}
		}
	}

	if (bf->config && bf->config->error_on_looking_at_safe_neighbor_8) {
		for (d = 0; d < 8; ++d) {
			dx = x + dir8[d][0];
			dy = y + dir8[d][1];
			if (!IN_BOUNDS(farm, dx, dy)) {
				continue;
			}
			s = SPOT_AT(farm->spots, farm->width, dx, dy);
			if (IS_SAFE(farm, s)) {
				farm->error = BF_ERROR_CANCEL;
			}
		}
	}

	SPOT_AT(bf->new_spots, farm->width, x, y) = to;

	if (bf->config && bf->config->enable_neighbor_look_8) {
		for (d = 0; d < 8; ++d) {
			dx = x + dir8[d][0];
			dy = y + dir8[d][1];
			if (!IN_BOUNDS(farm, dx, dy)) {
				continue;
			}
			s = SPOT_AT(bf->new_spots, farm->width, dx, dy);
			if (!IS_SAFE(farm, s)) {
				SPOT_AT(bf->new_spots, farm->width, dx, dy) =
					bf->config->neighbor_look_8;
			}
		}
	}

	if (bf->config && bf->config->enable_neighbor_look_4) {
		for (d = 0; d < 4; ++d) {
			dx = x + dir4p[d][0];
			dy = y + dir4p[d][1];
			if (!IN_BOUNDS(farm, dx, dy)) {
				continue;
			}
			s = SPOT_AT(bf->new_spots, farm->width, dx, dy);
			if (!IS_SAFE(farm, s)) {
				SPOT_AT(bf->new_spots, farm->width, dx, dy) =
					bf->config->neighbor_look_4;
			}
		}
	}
}

void look(
	struct butterfly *bf,
	struct bf_farm *farm,
	struct bf_instinct *instinct)
{
	struct bf_instinct temp;
	int x, y;
	int min, max;
	int w, h;
	int r;

	switch (instinct->action) {
	case BF_LOOK_NOWHERE:
		break;

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
		set_new_spot(farm, bf->x - 1, bf->y    , instinct->args[0]);
		set_new_spot(farm, bf->x    , bf->y - 1, instinct->args[0]);
		set_new_spot(farm, bf->x    , bf->y    , instinct->args[0]);
		set_new_spot(farm, bf->x    , bf->y + 1, instinct->args[0]);
		set_new_spot(farm, bf->x + 1, bf->y    , instinct->args[0]);
		break;

	case BF_LOOK_BIG_PLUS_AREA:
		set_new_spot(farm, bf->x - 1, bf->y    , instinct->args[0]);
		set_new_spot(farm, bf->x    , bf->y - 1, instinct->args[0]);
		set_new_spot(farm, bf->x    , bf->y    , instinct->args[0]);
		set_new_spot(farm, bf->x    , bf->y + 1, instinct->args[0]);
		set_new_spot(farm, bf->x + 1, bf->y    , instinct->args[0]);

		set_new_spot(farm, bf->x - 2, bf->y    , instinct->args[0]);
		set_new_spot(farm, bf->x    , bf->y - 2, instinct->args[0]);
		set_new_spot(farm, bf->x    , bf->y    , instinct->args[0]);
		set_new_spot(farm, bf->x    , bf->y + 2, instinct->args[0]);
		set_new_spot(farm, bf->x + 2, bf->y    , instinct->args[0]);
		break;

	case BF_LOOK_X_AREA:
		set_new_spot(farm, bf->x - 1, bf->y - 1, instinct->args[0]);
		set_new_spot(farm, bf->x - 1, bf->y + 1, instinct->args[0]);
		set_new_spot(farm, bf->x    , bf->y    , instinct->args[0]);
		set_new_spot(farm, bf->x + 1, bf->y - 1, instinct->args[0]);
		set_new_spot(farm, bf->x + 1, bf->y + 1, instinct->args[0]);
		break;

	case BF_LOOK_BIG_X_AREA:
		set_new_spot(farm, bf->x - 1, bf->y - 1, instinct->args[0]);
		set_new_spot(farm, bf->x - 1, bf->y + 1, instinct->args[0]);
		set_new_spot(farm, bf->x    , bf->y    , instinct->args[0]);
		set_new_spot(farm, bf->x + 1, bf->y - 1, instinct->args[0]);
		set_new_spot(farm, bf->x + 1, bf->y + 1, instinct->args[0]);

		set_new_spot(farm, bf->x - 2, bf->y - 2, instinct->args[0]);
		set_new_spot(farm, bf->x - 2, bf->y + 2, instinct->args[0]);
		set_new_spot(farm, bf->x    , bf->y    , instinct->args[0]);
		set_new_spot(farm, bf->x + 2, bf->y - 2, instinct->args[0]);
		set_new_spot(farm, bf->x + 2, bf->y + 2, instinct->args[0]);
		break;

	case BF_LOOK_RECT_AREA:
		w = instinct->args[1];
		h = instinct->args[2];
		for (x = bf->x - w; x <= bf->x + w; ++x) {
			for (y = bf->y - h; y <= bf->y + h; ++y) {
				set_new_spot(
					farm,
					x, y,
					instinct->args[0]);
			}
		}
		break;

	case BF_LOOK_SHRINKING_RECT_AREA:
		min = instinct->args[1];
		max = instinct->args[2];
		do {
			farm->error = BF_ERROR_NONE;
			temp = (struct bf_instinct){
				.action = BF_LOOK_RECT_AREA,
				.args = {instinct->args[0], max / 2, max / 2}
			};
			look(bf, farm, &temp);
		} while (farm->error && --max >= min);
		break;

	case BF_LOOK_CIRCLE_AREA:
		r = instinct->args[1];
		for (x = -r; x <= r; ++x) {
			h = floor(sqrt(r * r - x * x));
			for (y = -h; y <= h; ++y) {
				set_new_spot(
					farm,
					bf->x + x, bf->y + y,
					instinct->args[0]);
			}
		}
		break;

	case BF_LOOK_SHRINKING_CIRCLE_AREA:
		min = instinct->args[1];
		max = instinct->args[2];
		do {
			farm->error = BF_ERROR_NONE;
			temp = (struct bf_instinct){
				.action = BF_LOOK_CIRCLE_AREA,
				.args = {instinct->args[0], max / 2}
			};
			look(bf, farm, &temp);
		} while (farm->error && --max >= min);
		break;

	case BF_LOOK_DIAMOND_AREA:
		r = instinct->args[1];
		for (x = -r; x <= r; ++x) {
			h = floor(sqrt(r * r - x * x));
			for (y = -h; y <= h; ++y) {
				if (abs(x) + abs(y) > r) {
					continue;
				}
				set_new_spot(
					farm,
					bf->x + x, bf->y + y,
					instinct->args[0]);
			}
		}
		break;

	case BF_LOOK_SHRINKING_DIAMOND_AREA:
		min = instinct->args[1];
		max = instinct->args[2];
		do {
			farm->error = BF_ERROR_NONE;
			temp = (struct bf_instinct){
				.action = BF_LOOK_DIAMOND_AREA,
				.args = {instinct->args[0], max / 2}
			};
			look(bf, farm, &temp);
		} while (farm->error && --max >= min);
		break;

	default:
		break;
	}
}
