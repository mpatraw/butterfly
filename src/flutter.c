
#include "butterfly.h"
#include "internal.h"

static void move_by(
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

void flutter(
	struct butterfly *bf,
	struct bf_farm *farm,
	struct bf_instinct *instinct)
{
	int dirs[4][2] = {
		{-1,  0},
		{ 0, -1},
		{ 0,  1},
		{ 1,  0},
	};

	int *dir;
	int dx, dy;
	double r, weight;
	int *dpz, *dpnz;

	switch (instinct->action) {
	case BF_FLUTTER_RANDOMLY:
		dir = dirs[random_next_index(farm->rng_state, 4)];
		dx = dir[0];
		dy = dir[1];
		move_by(bf, farm, dx, dy);
		break;

/* Okay, so here's the weighted walk algorith. Very sloppy, but I foresee it
 * changing a lot so I'm not too worried. The algorithm is hard coded and
 * not optimized... for now.
 *
 * The two cases you have in a four directional weighted walk is where the
 * walker is ON an axis that the target is on, and when the walker is not...
 * Each case is handled differently.
 *
 * === On a same axis ===
 *
 * With this weighted walk I desire a spread of 25% for each direction when
 * weight = 50%. That's right in the middle. In the same axis case there is only
 * one direction that heads directly to the target, so in order to give it the
 * proper weight, it's chance of occuring is weight / 2, so at weight = 50%, the
 * exact chance to go directly towards the target is 25%. What do we do with
 * the remainding percent and the other three directions? Well the remaining
 * weight is divided evenly, so each of the three directions gets a 3rd of it
 * or a (6th of the total since the direct direction took half already). They
 * also get the left over anti-weight (1 - weight), which is divided evenly so
 * they get a 3rd of it.
 *
 * (X is target, @ is location, W is weight):
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
 * (X is target, @ is location, W is weight):
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
			/* On target, do nothing. */
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

		move_by(bf, farm, dx, dy);
		break;

	case BF_FLUTTER_STRAIGHT_TO_GOAL:
		break;

	default:
		break;
	}
}
