
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
	BF_FLUTTER_RANDOMLY,
	BF_FLUTTER_RANDOMLY_TO_GOAL,
	BF_FLUTTER_STRAIGHT_TO_GOAL,
};

enum {
	BF_LOOK_EVERYWHERE,
	BF_LOOK_1_AREA,
	BF_LOOK_PLUS_AREA,
	BF_LOOK_X_AREA,
	BF_LOOK_RECT_AREA,
	BF_LOOK_CIRCLE_AREA,
};

enum {
	BF_DIE_AT_SPOT,
	BF_DIE_AT_SAFE_SPOT,
	BF_DIE_AT_DANGEROUS_SPOT,
	BF_DIE_AT_GOAL,
	BF_DIE_AFTER_N_FLUTTERS
};

struct bf_farm {
	int *spots;
	int width;
	int height;
	int seed;

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

#ifdef __cplusplus
extern "C" {
#endif

int bf_spawn(
	struct bf_farm *farm,
	struct bf_instinct *instincts,
	size_t count);
void bf_cleanup(struct bf_farm *farm);

#ifdef __cplusplus
}
#endif

#define BF_SPAWN_ARR(farm, inst, ntimes) 					\
do {														\
	int i;													\
	for (i = 0; i < (ntimes); ++i) {						\
		bf_spawn(farm, inst, sizeof(inst) / sizeof(*inst));	\
	}														\
} while (0)

#define BF_SPAWN_SZ(farm, inst, sz, ntimes) 				\
do {														\
	int i;													\
	for (i = 0; i < (ntimes); ++i) {						\
		bf_spawn(farm, inst, sz);							\
	}														\
} while (0)


#endif
