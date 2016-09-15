
#include "butterfly.h"
#include "internal.h"

void goal(
	struct butterfly *bf,
	struct bf_farm *farm,
	struct bf_instinct *instinct)
{
	struct bf_instinct inst;
	struct point point;

	switch (instinct->action) {
	case BF_GOAL_FIXED_SPOT:
		bf->goal_x = instinct->args[0];
		bf->goal_y = instinct->args[1];
		break;

	case BF_GOAL_RANDOM_SPOT:
		bf->goal_x = random_next_index(farm->rng_state, farm->width);
		bf->goal_y = random_next_index(farm->rng_state, farm->height);
		break;

	case BF_GOAL_RANDOM_SAFE_SPOT:
		point = ps_rnd(farm->safe_spots, farm->rng_state);
		bf->goal_x = point.x;
		bf->goal_y = point.y;
		break;

	case BF_GOAL_RANDOM_DANGEROUS_SPOT:
		point = ps_rnd(farm->dangerous_spots, farm->rng_state);
		bf->goal_x = point.x;
		bf->goal_y = point.y;
		break;

	case BF_GOAL_RANDOM_WEST_SPOT:
		bf->goal_x = random_next_index(
			farm->rng_state, farm->width / 2);
		bf->goal_y = random_next_index(farm->rng_state, farm->height);
		break;

	case BF_GOAL_RANDOM_EAST_SPOT:
		bf->goal_x = random_next_index(
			farm->rng_state, farm->width / 2);
		bf->goal_y = random_next_index(farm->rng_state, farm->height);
		bf->goal_x += farm->width / 2;
		break;

	case BF_GOAL_RANDOM_NORTH_SPOT:
		bf->goal_x = random_next_index(farm->rng_state, farm->width);
		bf->goal_y = random_next_index(
			farm->rng_state, farm->height / 2);
		break;

	case BF_GOAL_RANDOM_SOUTH_SPOT:
		bf->goal_x = random_next_index(farm->rng_state, farm->width);
		bf->goal_y = random_next_index(
			farm->rng_state, farm->height / 2);
		bf->goal_y += farm->height / 2;
		break;

	case BF_GOAL_RANDOM_WEST_EDGE_SPOT:
		bf->goal_x = 0;
		bf->goal_y = random_next_index(farm->rng_state, farm->height);
		break;

	case BF_GOAL_RANDOM_EAST_EDGE_SPOT:
		bf->goal_x = farm->width - 1;
		bf->goal_y = random_next_index(farm->rng_state, farm->height);
		break;

	case BF_GOAL_RANDOM_NORTH_EDGE_SPOT:
		bf->goal_x = random_next_index(farm->rng_state, farm->width);
		bf->goal_y = 0;
		break;

	case BF_GOAL_RANDOM_SOUTH_EDGE_SPOT:
		bf->goal_x = random_next_index(farm->rng_state, farm->width);
		bf->goal_y = farm->height - 1;
		break;

	case BF_GOAL_RANDOM_WESTEAST_EDGE_SPOT:
		if (random_next_unit(farm->rng_state) < 0.5) {
			inst.action = BF_GOAL_RANDOM_WEST_EDGE_SPOT;
			goal(bf, farm, &inst);
		} else {
			inst.action = BF_GOAL_RANDOM_EAST_EDGE_SPOT;
			goal(bf, farm, &inst);
		}
		break;

	case BF_GOAL_RANDOM_NORTHSOUTH_EDGE_SPOT:
		if (random_next_unit(farm->rng_state) < 0.5) {
			inst.action = BF_GOAL_RANDOM_NORTH_EDGE_SPOT;
			goal(bf, farm, &inst);
		} else {
			inst.action = BF_GOAL_RANDOM_SOUTH_EDGE_SPOT;
			goal(bf, farm, &inst);
		}
		break;

	case BF_GOAL_RANDOM_EDGE_SPOT:
		if (random_next_unit(farm->rng_state) < 0.5) {
			inst.action = BF_GOAL_RANDOM_WESTEAST_EDGE_SPOT;
			goal(bf, farm, &inst);
		} else {
			inst.action = BF_GOAL_RANDOM_NORTHSOUTH_EDGE_SPOT;
			goal(bf, farm, &inst);
		}
		break;

	default:
		/* this means there is no goal spot */
		bf->goal_x = -1;
		bf->goal_y = -1;
		break;
	}
}
