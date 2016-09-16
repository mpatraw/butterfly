
#ifndef BUTTERFLY_H
#define BUTTERFLY_H

#include <stddef.h>

enum {
	BF_NARGS = 3
};

/* life events */
enum {
	BF_MORPH,
	BF_GOAL,
	BF_FLUTTER,
	BF_LOOK,
	BF_DIE
};

enum {
	/* default if no morph spot specified */
	BF_MORPH_AT_LAST_DEATH_SPOT,
	BF_MORPH_AT_LAST_MORPH_SPOT,
	BF_MORPH_AT_FIXED_SPOT,
	BF_MORPH_AT_RANDOM_SPOT,
	BF_MORPH_AT_RANDOM_SAFE_SPOT,
	BF_MORPH_AT_RANDOM_DANGEROUS_SPOT,
	BF_MORPH_AT_RANDOM_WEST_SPOT,
	BF_MORPH_AT_RANDOM_EAST_SPOT,
	BF_MORPH_AT_RANDOM_NORTH_SPOT,
	BF_MORPH_AT_RANDOM_SOUTH_SPOT,
	BF_MORPH_AT_RANDOM_WEST_EDGE_SPOT,
	BF_MORPH_AT_RANDOM_EAST_EDGE_SPOT,
	BF_MORPH_AT_RANDOM_NORTH_EDGE_SPOT,
	BF_MORPH_AT_RANDOM_SOUTH_EDGE_SPOT,
	BF_MORPH_AT_RANDOM_WESTEAST_EDGE_SPOT,
	BF_MORPH_AT_RANDOM_NORTHSOUTH_EDGE_SPOT,
	BF_MORPH_AT_RANDOM_EDGE_SPOT
};

enum {
	BF_GOAL_FIXED_SPOT,
	BF_GOAL_RANDOM_SPOT,
	BF_GOAL_RANDOM_SAFE_SPOT,
	BF_GOAL_RANDOM_DANGEROUS_SPOT,
	BF_GOAL_RANDOM_WEST_SPOT,
	BF_GOAL_RANDOM_EAST_SPOT,
	BF_GOAL_RANDOM_NORTH_SPOT,
	BF_GOAL_RANDOM_SOUTH_SPOT,
	BF_GOAL_RANDOM_WEST_EDGE_SPOT,
	BF_GOAL_RANDOM_EAST_EDGE_SPOT,
	BF_GOAL_RANDOM_NORTH_EDGE_SPOT,
	BF_GOAL_RANDOM_SOUTH_EDGE_SPOT,
	BF_GOAL_RANDOM_WESTEAST_EDGE_SPOT,
	BF_GOAL_RANDOM_NORTHSOUTH_EDGE_SPOT,
	BF_GOAL_RANDOM_EDGE_SPOT,
	BF_GOAL_RANDOM_MARKED_SPOT,
	BF_GOAL_RANDOM_PEED_SPOT,
};

enum {
	BF_FLUTTER_RANDOMLY_4,
	BF_FLUTTER_RANDOMLY_8,
	BF_FLUTTER_RANDOMLY_TO_GOAL,
	BF_FLUTTER_STRAIGHT_TO_GOAL,
	BF_FLUTTER_TUNNEL_TO_GOAL,
	BF_FLUTTER_LINE_TO_GOAL
};

enum {
	BF_LOOK_EVERYWHERE,
	BF_LOOK_1_AREA,
	BF_LOOK_PLUS_AREA,
	BF_LOOK_BIG_PLUS_AREA,
	BF_LOOK_X_AREA,
	BF_LOOK_BIG_X_AREA,
	BF_LOOK_RECT_AREA,
	BF_LOOK_CIRCLE_AREA,
};

enum {
	BF_DIE_AT_SPOT,
	BF_DIE_AT_SAFE_SPOT,
	BF_DIE_AT_DANGEROUS_SPOT,
	BF_DIE_AT_GOAL,
	BF_DIE_AFTER_N
};

enum {
	BF_ERROR_NONE,
	BF_ERROR_CANCEL,
	BF_ERROR_NO_MEM
};

struct bf_farm {
	int *spots;
	int width;
	int height;
	int seed;

	/* 0 if no error */
	int error;

	/* internal */
	int is_init;
	void *rng_state;
	void *safe_spots;
	void *dangerous_spots;
	void *butterfly;
};

struct bf_instinct {
	int event;
	int action;
	int args[BF_NARGS];
};

struct bf_config {
	/* this will make the butterflies cancel when they
	 * look at a safe location (to prevent overwrites)
	 * default = false
	 */
	int error_on_looking_at_safe;

	/* this will make the butterflies stop when they look
	 * outside the map.
	 * default = false
	 */
	int error_on_looking_outside_farm;

	/* this cycles through all the looking instincts instead of
	 * randomly selecting one.
	 * default = false
	 */
	int cycle_looking;
};

#ifdef __cplusplus
extern "C" {
#endif

int bf_spawn(
	struct bf_farm *farm,
	struct bf_instinct *instincts,
	size_t count,
	struct bf_config *config);
void bf_commit(struct bf_farm *farm);
void bf_cleanup(struct bf_farm *farm);

#ifdef __cplusplus
}
#endif

#define BF_SPAWN_ARR(farm, inst, config)	\
	bf_spawn(farm, inst, sizeof(inst) / sizeof(*inst), config)


#endif