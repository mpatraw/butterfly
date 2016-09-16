
#ifndef BUTTERFLY_H
#define BUTTERFLY_H

#include <stddef.h>

enum {
	BF_NARGS = 5
};

/*
 * Life events for a butterfly.
 */
enum {
	/* Where to spawn (optional). Default is last death location.) */
	BF_MORPH,
	/* Where to go (optional). Default is no goal. */
	BF_GOAL,
	/* How to get to the goal (optional). Default is no movement. */
	BF_FLUTTER,
	/* Pattern to dig out at each step (optional). */
	BF_LOOK,
	/* How and when to die (required). */
	BF_DIE
};

enum {
	/* Morph where the last butterfly died.
	 *
	 * Default if a morph instinct is not specified.
	 */
	BF_MORPH_AT_LAST_DEATH_SPOT,
	/* Morph where the last butterfly morphed.
	 */
	BF_MORPH_AT_LAST_MORPH_SPOT,
	/* Morph at a fixed location.
	 * args[0] = x location
	 * args[1] = y location
	 */
	BF_MORPH_AT_FIXED_SPOT,
	/* Morph at any random spot.
	 */
	BF_MORPH_AT_RANDOM_SPOT,
	/* Morph at a random safe spot. Undefined behavior if there
	 * are no safe spots.
	 */
	BF_MORPH_AT_RANDOM_SAFE_SPOT,
	/* Morph at a random dangerous spot. Undefined behavior if there
	 * are no dangerous spots.
	 */
	BF_MORPH_AT_RANDOM_DANGEROUS_SPOT,
	/* Morph at a random spot on the west half.
	 */
	BF_MORPH_AT_RANDOM_WEST_SPOT,
	/* Morph at a random spot on the east half.
	 */
	BF_MORPH_AT_RANDOM_EAST_SPOT,
	/* Morph at a random spot on the north half.
	 */
	BF_MORPH_AT_RANDOM_NORTH_SPOT,
	/* Morph at a random spot on the south half.
	 */
	BF_MORPH_AT_RANDOM_SOUTH_SPOT,
	/* Morph at a random spot on the west edge.
	 */
	BF_MORPH_AT_RANDOM_WEST_EDGE_SPOT,
	/* Morph at a random spot on the east edge.
	 */
	BF_MORPH_AT_RANDOM_EAST_EDGE_SPOT,
	/* Morph at a random spot on the north edge.
	 */
	BF_MORPH_AT_RANDOM_NORTH_EDGE_SPOT,
	/* Morph at a random spot on the south edge.
	 */
	BF_MORPH_AT_RANDOM_SOUTH_EDGE_SPOT,
	/* Morph at a random spot on a west oreast edge.
	 */
	BF_MORPH_AT_RANDOM_WESTEAST_EDGE_SPOT,
	/* Morph at a random spot on a north or south edge.
	 */
	BF_MORPH_AT_RANDOM_NORTHSOUTH_EDGE_SPOT,
	/* Morph at a random spot on an edge.
	 */
	BF_MORPH_AT_RANDOM_EDGE_SPOT
};

enum {
	/* Set goal to a fixed location.
	 * args[0] = x location
	 * args[1] = y location
	 */
	BF_GOAL_FIXED_SPOT,
	/* Set goal to a random spot.
	 */
	BF_GOAL_RANDOM_SPOT,
	/* Set goal to a random safe spot spot. Undefined behavior if there
	 * are no safe spots.
	 */
	BF_GOAL_RANDOM_SAFE_SPOT,
	/* Set goal to a random dangerous spot spot. Undefined behavior if there
	 * are no dangerous spots.
	 */
	BF_GOAL_RANDOM_DANGEROUS_SPOT,
	/* Set goal to a random spot on the west half.
	 */
	BF_GOAL_RANDOM_WEST_SPOT,
	/* Set goal to a random spot on the east half.
	 */
	BF_GOAL_RANDOM_EAST_SPOT,
	/* Set goal to a random spot on the north half.
	 */
	BF_GOAL_RANDOM_NORTH_SPOT,
	/* Set goal to a random spot on the south half.
	 */
	BF_GOAL_RANDOM_SOUTH_SPOT,
	/* Set goal to a random spot on the west edge.
	 */
	BF_GOAL_RANDOM_WEST_EDGE_SPOT,
	/* Set goal to a random spot on the east edge.
	 */
	BF_GOAL_RANDOM_EAST_EDGE_SPOT,
	/* Set goal to a random spot on the north edge.
	 */
	BF_GOAL_RANDOM_NORTH_EDGE_SPOT,
	/* Set goal to a random spot on the south edge.
	 */
	BF_GOAL_RANDOM_SOUTH_EDGE_SPOT,
	/* Set goal to a random spot on the west or east edge.
	 */
	BF_GOAL_RANDOM_WESTEAST_EDGE_SPOT,
	/* Set goal to a random spot on the north or south edge.
	 */
	BF_GOAL_RANDOM_NORTHSOUTH_EDGE_SPOT,
	/* Set goal to a random spot on an edge.
	 */
	BF_GOAL_RANDOM_EDGE_SPOT
};

enum {
	/* Don't move.
	 */
	BF_FLUTTER_STILL,
	/* Move to the goal with a weighted random walk. If no goal specified,
	 * this is completely random.
	 * args[0] = weight (0-100), 0 means away from, 100 means towards.
	 */
	BF_FLUTTER_WEIGHTED_4,
	/* Move to the goal with a weighted random walk. If no goal specified,
	 * this is completely random.
	 * args[0] = weight (0-100), 0 means away from, 100 means towards.
	 * XXX: not implemented.
	 */
	BF_FLUTTER_WEIGHTED_8,
	/* Move to the goal in a tunnel path. First goes horizontally or
	 * vertically, then the opposite to reach the goal. If no goal specified,
	 * does nothing.
	 */
	BF_FLUTTER_TUNNEL,
	/* Move to the goal in a line path. This uses Bresenham's line alogirithm.
	 * If no goal specified, does nothing.
	 */
	BF_FLUTTER_LINE
};

enum {
	/* No op. Doesn't change any spots.
	 */
	BF_LOOK_NOWHERE,
	/* Changes every spot on the farm.
	 * args[0] = tile
	 */
	BF_LOOK_EVERYWHERE,
	/* Changes the spot where the butterfly is.
	 * args[0] = tile
	 */
	BF_LOOK_1_AREA,
	/* Changes the spots in a plus pattern centered on the butterfly.
	 * #.#
	 * ...
	 * #.#
	 * args[0] = tile
	 */
	BF_LOOK_PLUS_AREA,
	/* Changes the spots in a big plus pattern centered on the butterfly.
	 * ##.##
	 * ##.##
	 * .....
	 * ##.##
	 * ##.##
	 * args[0] = tile
	 */
	BF_LOOK_BIG_PLUS_AREA,
	/* Changes the spots in an X pattern centered on the butterfly.
	 * .#.
	 * #.#
	 * .#.
	 * args[0] = tile
	 */
	BF_LOOK_X_AREA,
	/* Changes the spots in a big X pattern centered on the butterfly.
	 * .###.
	 * #.#.#
	 * ##.##
	 * #.#.#
	 * .###.
	 * args[0] = tile
	 */
	BF_LOOK_BIG_X_AREA,
	/* Changes the spots in a rect pattern centered on the butterfly.
	 * args[0] = tile
	 * args[1] = half width
	 * args[2] = half height
	 */
	BF_LOOK_RECT_AREA,
	/* Changes the spots in a rect pattern centered on the butterfly. This
	 * only works if the config option: .error_on_looking_at_safe is set.
	 * args[0] = tile
	 * args[1] = minimum width
	 * args[2] = minimum height
	 * XXX: not implemented correctly.
	 */
	BF_LOOK_SHRINKING_RECT_AREA,
	/* Changes the spots in a circle pattern centered on the butterfly.
	 * args[0] = tile
	 * args[1] = radius
	 */
	BF_LOOK_CIRCLE_AREA,
	/* Changes the spots in a circle pattern centered on the butterfly. This
	 * only works if the config option: .error_on_looking_at_safe is set.
	 * args[0] = tile
	 * args[1] = radius
	 * args[2] = maximum radius
	 * XXX: not implemented correctly.
	 */
	BF_LOOK_SHRINKING_CIRCLE_AREA,
	/* Changes the spots in a diamond pattern centered on the butterfly.
	 * args[0] = tile
	 * args[1] = radius
	 * XXX: not implemented correctly.
	 */
	BF_LOOK_DIAMOND_AREA,
	/* Changes the spots in a diamond pattern centered on the butterfly. This
	 * only works if the config option: .error_on_looking_at_safe is set.
	 * args[0] = tile
	 * args[1] = minimum radius
	 * args[2] = maximum radius
	 * XXX: not implemented correctly.
	 */
	BF_LOOK_SHRINKING_DIAMOND_AREA,
};

enum {
	/* The butterfly will die when landing on a specific coordinate.
	 * args[0] = x
	 * args[1] = y
	 */
	BF_DIE_AT_FIXED_SPOT,
	/* The butterfly will die when it flutters to any safe spot.
	 */
	BF_DIE_AT_SAFE_SPOT,
	/* The butterfly will die when it flutters to any dangerous spot.
	 */
	BF_DIE_AT_DANGEROUS_SPOT,
	/* The butterfly will die when it flutters to the goal location. If there
	 * is no goal, this will never kill the butterfly.
	 */
	BF_DIE_AT_GOAL,
	/* The butterfly will die after N flutters and looks. Even if no looks or
	 * flutters.
	 */
	BF_DIE_AFTER_N,
	/* The butterfly has a one in N chance to die every flutter and look.
	 */
	BF_DIE_ONE_IN
};

enum {
	BF_QUERY_SAFE_PERCENTAGE,
	BF_QUERY_DANGEROUS_PERCENTAGE
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

	/* the last dangerous numbers. any number higher than this
	 * is considered "safe.""
	 * default 0
	 */
	int last_dangerous;

	/* internal */
	int is_init;
	void *rng_state;
	void *safe_spots;
	void *dangerous_spots;
	void *butterfly;
	/* 0 if no error */
	int error;
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

	/* this makes it so any tile touched, all the neighbors are imbued with
	 * this spot. this will only overwrite dangerous spots.
	 * **only works for dangerous spots**
	 */
	int enable_neighbor_look_8;
	int neighbor_look_8;

	/* this makes it so any tile touched, all the neighbors are imbued with
	 * this spot.  this will only overwrite dangerous spots.
	 * **only works for dangerous spots**
	 */
	int enable_neighbor_look_4;
	int neighbor_look_4;
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
double bf_random(struct bf_farm *farm);
int bf_query(struct bf_farm *farm, int query);
void bf_cleanup(struct bf_farm *farm);

#ifdef __cplusplus
}
#endif

#define BF_SPAWN_ARR(farm, inst, config)	\
	bf_spawn(farm, inst, sizeof(inst) / sizeof(*inst), config)


#endif
