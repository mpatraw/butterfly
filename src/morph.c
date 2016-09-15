
#include <assert.h>

#include "butterfly.h"
#include "internal.h"

void morph(
	struct butterfly *bf,
	struct bf_farm *farm,
	struct bf_instinct *instinct)
{
	struct bf_instinct inst;
	struct point point;

	switch (instinct->action) {
	case BF_MORPH_AT_LAST_DEATH_SPOT:
		bf->x = bf->last_death_x;
		bf->y = bf->last_death_y;
		break;

	case BF_MORPH_AT_LAST_MORPH_SPOT:
		bf->x = bf->last_morph_x;
		bf->y = bf->last_morph_y;
		break;

	case BF_MORPH_AT_FIXED_SPOT:
		bf->x = instinct->args[0];
		bf->y = instinct->args[1];
		break;

	case BF_MORPH_AT_RANDOM_SPOT:
		bf->x = random_next_index(farm->rng_state, farm->width);
		bf->y = random_next_index(farm->rng_state, farm->height);
		break;

	case BF_MORPH_AT_RANDOM_SAFE_SPOT:
		point = ps_rnd(farm->safe_spots, farm->rng_state);
		bf->x = point.x;
		bf->y = point.y;
		break;

	case BF_MORPH_AT_RANDOM_DANGEROUS_SPOT:
		point = ps_rnd(farm->dangerous_spots, farm->rng_state);
		bf->x = point.x;
		bf->y = point.y;
		break;

	case BF_MORPH_AT_RANDOM_WEST_SPOT:
		bf->x = random_next_index(farm->rng_state, farm->width / 2);
		bf->y = random_next_index(farm->rng_state, farm->height);
		break;

	case BF_MORPH_AT_RANDOM_EAST_SPOT:
		bf->x = random_next_index(farm->rng_state, farm->width / 2);
		bf->y = random_next_index(farm->rng_state, farm->height);
		bf->x += farm->width / 2;
		break;

	case BF_MORPH_AT_RANDOM_NORTH_SPOT:
		bf->x = random_next_index(farm->rng_state, farm->width);
		bf->y = random_next_index(farm->rng_state, farm->height / 2);
		break;

	case BF_MORPH_AT_RANDOM_SOUTH_SPOT:
		bf->x = random_next_index(farm->rng_state, farm->width);
		bf->y = random_next_index(farm->rng_state, farm->height / 2);
		bf->y += farm->height / 2;
		break;

	case BF_MORPH_AT_RANDOM_WEST_EDGE_SPOT:
		bf->x = 0;
		bf->y = random_next_index(farm->rng_state, farm->height);
		break;

	case BF_MORPH_AT_RANDOM_EAST_EDGE_SPOT:
		bf->x = farm->width - 1;
		bf->y = random_next_index(farm->rng_state, farm->height);
		break;

	case BF_MORPH_AT_RANDOM_NORTH_EDGE_SPOT:
		bf->x = random_next_index(farm->rng_state, farm->width);
		bf->y = 0;
		break;

	case BF_MORPH_AT_RANDOM_SOUTH_EDGE_SPOT:
		bf->x = random_next_index(farm->rng_state, farm->width);
		bf->y = farm->height - 1;
		break;

	case BF_MORPH_AT_RANDOM_WESTEAST_EDGE_SPOT:
		if (random_next_unit(farm->rng_state) < 0.5) {
			inst.action = BF_MORPH_AT_RANDOM_WEST_EDGE_SPOT;
			morph(bf, farm, &inst);
		} else {
			inst.action = BF_MORPH_AT_RANDOM_EAST_EDGE_SPOT;
			morph(bf, farm, &inst);
		}
		break;

	case BF_MORPH_AT_RANDOM_NORTHSOUTH_EDGE_SPOT:
		if (random_next_unit(farm->rng_state) < 0.5) {
			inst.action = BF_MORPH_AT_RANDOM_NORTH_EDGE_SPOT;
			morph(bf, farm, &inst);
		} else {
			inst.action = BF_MORPH_AT_RANDOM_SOUTH_EDGE_SPOT;
			morph(bf, farm, &inst);
		}
		break;

	case BF_MORPH_AT_RANDOM_EDGE_SPOT:
		if (random_next_unit(farm->rng_state) < 0.5) {
			inst.action = BF_MORPH_AT_RANDOM_WESTEAST_EDGE_SPOT;
			morph(bf, farm, &inst);
		} else {
			inst.action = BF_MORPH_AT_RANDOM_NORTHSOUTH_EDGE_SPOT;
			morph(bf, farm, &inst);
		}
		break;

	default:
		break;
	}
}
