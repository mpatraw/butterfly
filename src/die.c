
#include "butterfly.h"
#include "internal.h"

bool die(
	struct butterfly *bf,
	struct bf_farm *farm,
	struct bf_instinct *instinct)
{
	struct point point;

	switch (instinct->action) {
	case BF_DIE_AT_SPOT:
		return	bf->x == instinct->args[0] &&
			bf->y == instinct->args[1];

	case BF_DIE_AT_SAFE_SPOT:
		point = (struct point){bf->x, bf->y};
		return ps_has(farm->safe_spots, point);

	case BF_DIE_AT_DANGEROUS_SPOT:
		point = (struct point){bf->x, bf->y};
		return ps_has(farm->dangerous_spots, point);

	case BF_DIE_AT_GOAL:
		return bf->x == bf->goal_x && bf->y == bf->goal_y;

	case BF_DIE_AFTER_N:
		if (bf->death_timer == instinct->args[0]) {
			return true;
		}
		bf->death_timer++;
		break;

	default:
		break;
	}

	return false;
}
