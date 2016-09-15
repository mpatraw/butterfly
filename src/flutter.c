
#include <stdbool.h>
#include <stdlib.h>

#include "butterfly.h"
#include "internal.h"

static void move_by_caged(
	struct butterfly *bf,
	struct bf_farm *farm,
	int dx, int dy)
{
	int nx = bf->x + dx;
	int ny = bf->y + dy;
	if (nx < 0 || nx >= farm->width || ny < 0 || ny >= farm->height) {
		return;
	}
	bf->x = nx;
	bf->y = ny;
}

enum {NONE, HORIZONTAL, VERTICAL};

struct tunnel_path {
	int first;
	int second;
};

static bool flutter_tunnel(struct butterfly *bf)
{
	struct tunnel_path *data = bf->path_data;

	if (!data->first) {
		return true;
	}

	if (data->first == HORIZONTAL) {
		bf->x += (bf->goal_x - bf->x) < 0 ? -1 : 1;

		if (bf->x == bf->goal_x) {
			data->first = data->second;
			data->second = NONE;
		}
	} else if (data->first == VERTICAL) {
		bf->y += (bf->goal_y - bf->y) < 0 ? -1 : 1;

		if (bf->y == bf->goal_y) {
			data->first = data->second;
			data->second = NONE;
		}
	}

	return false;
}

struct line_path {
	int dx;
	int sx;
	int dy;
	int sy;
	int err;
	int e2;
};

static bool flutter_line(struct butterfly *bf)
{
	struct line_path *data = (void *)bf->path_data;

	if (bf->x == bf->goal_x && bf->y == bf->goal_y) {
		return true;
	}

	data->e2 = data->err;

	if (data->e2 > -data->dx) {
		data->err -= data->dy;
		bf->x += data->sx;
	}
	if (data->e2 < data->dy) {
		data->err += data->dx;
		bf->y += data->sy;
	}

	return false;
}

void flutter(
	struct butterfly *bf,
	struct bf_farm *farm,
	struct bf_instinct *instinct)
{
	int dirs4[4][2] = {
		{-1,  0},
		{ 0, -1},
		{ 0,  1},
		{ 1,  0},
	};
	int dirs8[8][2] = {
		{-1, -1},
		{-1,  0},
		{-1,  1},
		{ 0, -1},
		{ 0,  1},
		{ 1, -1},
		{ 1,  0},
		{ 1,  1},
	};

	int *dir;
	int dx, dy;
	double r, weight;
	int *dpz, *dpnz;

	switch (instinct->action) {
	case BF_FLUTTER_RANDOMLY_4:
		dir = dirs4[random_next_index(farm->rng_state, 4)];
		move_by_caged(bf, farm, dir[0], dir[1]);
		break;

	case BF_FLUTTER_RANDOMLY_8:
		dir = dirs8[random_next_index(farm->rng_state, 8)];
		move_by_caged(bf, farm, dir[0], dir[1]);
		break;

/* Okay, so here's the weighted walk algorith. Very sloppy, but I foresee it
 * changing a lot so I'm not too worried. The algorithm is hard coded and
 * not optimized... for now.
 *
 * The two cases you have in a four directional weighted walk is where the
 * walker is ON an axis that the goal is on, and when the walker is not...
 * Each case is handled differently.
 *
 * === On a same axis ===
 *
 * With this weighted walk I desire a spread of 25% for each direction when
 * weight = 50%. That's right in the middle. In the same axis case there is only
 * one direction that heads directly to the goal, so in order to give it the
 * proper weight, it's chance of occuring is weight / 2, so at weight = 50%, the
 * exact chance to go directly towards the goal is 25%. What do we do with
 * the remainding percent and the other three directions? Well the remaining
 * weight is divided evenly, so each of the three directions gets a 3rd of it
 * or a (6th of the total since the direct direction took half already). They
 * also get the left over anti-weight (1 - weight), which is divided evenly so
 * they get a 3rd of it.
 *
 * (X is goal, @ is location, W is weight):
 *                  (W / 6 + (1 - W) / 3)
 *                          |
 *                          |
 * (W / 6 + (1 - W) / 3) ---@--- X (W / 2)
 *                          |
 *                          |
 *                  (W / 6 + (1 - W) / 3)
 *
 * For example, when W = 0.75:
 *         ~20.83%
 *            |
 *            |
 * ~20.83% ---@--- X 37.5%
 *            |
 *            |
 *         ~20.83%
 *
 * When W = 1.0:
 *         ~16.66%
 *            |
 *            |
 * ~16.66% ---@--- X 50%
 *            |
 *            |
 *         ~16.66%
 *
 * === Not on a same axis ===
 *
 * The same axis is easier, the formula is:
 *
 * (X is goal, @ is location, W is weight):
 *             (W / 2)
 *                |
 *                |  X
 * (1 - W) / 2 ---@--- (W / 2)
 *                |
 *                |
 *           (1 - W) / 2
 *
 * For example, when W = 0.75:
 *        37.5%
 *          |
 *          |  X
 * 12.5% ---@--- 37.5%
 *          |
 *          |
 *        12.5%
 */

	case BF_FLUTTER_RANDOMLY_TO_GOAL:
		weight = instinct->args[0] / 100.f;
		dx = bf->goal_x - bf->x;
		dy = bf->goal_y - bf->y;

		if (dx >  1) { dx =  1; }
		if (dx < -1) { dx = -1; }
		if (dy >  1) { dy =  1; }
		if (dy < -1) { dy = -1; }

		r = random_next_unit(farm->rng_state);

		if (dx == 0 && dy == 0) {
			/* On goal, do nothing. */
		} else if (dx == 0 || dy == 0) {
			if (dx == 0) {
				dpz = &dx;
				dpnz = &dy;
			} else {
				dpz = &dy;
				dpnz = &dx;
			}

			if (r >= (weight * 0.5)) {
				r -= weight * 0.5;
				if (r < weight * (1.0 / 6) + (1 - weight) * (1.0 / 3)) {
					*dpz = -1;
					*dpnz = 0;
				} else if (r < weight * (2.0 / 6) + (1 - weight) * (2.0 / 3)) {
					*dpz = 1;
					*dpnz = 0;
				} else {
					*dpz = 0;
					*dpnz *= -1;
				}
			}
		} else {
			if (r < weight * 0.5) {
				dy = 0;
			} else if (r < weight) {
				dx = 0;
			} else if (r < weight + (1 - weight) * 0.5) {
				dx *= -1;
				dy = 0;
			} else {
				dx = 0;
				dy *= -1;
			}
		}

		bf->x += dx;
		bf->y += dy;
		break;

	case BF_FLUTTER_STRAIGHT_TO_GOAL:
		break;

	case BF_FLUTTER_TUNNEL_TO_GOAL: {
		if (!bf->path_data) {
			bf->path_data = malloc(sizeof(struct tunnel_path));
			if (!bf->path_data) {
				farm->error = BF_ERROR_NO_MEM;
				return;
			}
			struct tunnel_path *data = (void *)bf->path_data;

			int dx = bf->goal_x - bf->x;
			int dy = bf->goal_y - bf->y;

			if (dx != 0 && dy != 0) {
				if (random_next_unit(farm->rng_state) < 0.5) {
					data->first = HORIZONTAL;
					data->second = VERTICAL;
				} else {
					data->first = VERTICAL;
					data->second = HORIZONTAL;
				}
			} else if (dx != 0) {
				data->first = HORIZONTAL;
				data->second = NONE;
			} else if (dy != 0) {
				data->first = VERTICAL;
				data->second = NONE;
			}
		}
		flutter_tunnel(bf);

		break;
	}

	case BF_FLUTTER_LINE_TO_GOAL: {
		if (!bf->path_data) {
			bf->path_data = malloc(sizeof(struct line_path));
			if (!bf->path_data) {
				farm->error = BF_ERROR_NO_MEM;
				return;
			}
			struct line_path *data = bf->path_data;

			data->dx = abs(bf->goal_x - bf->x);
			data->sx = bf->x < bf->goal_x ? 1 : -1;

			data->dy = abs(bf->goal_y - bf->y);
			data->sy = bf->y < bf->goal_y ? 1 : -1;

			data->err = (data->dx > data->dy ? data->dx : -data->dy) / 2;

		}
		flutter_line(bf);

		break;
	}

	default:
		break;
	}
}
