
#include <stdio.h>

#include <butterfly.h>

enum {
	WALL,
	FLOOR
};

static void draw(int spots[24][80])
{
	for (int y = 0; y < 24; ++y) {
		for (int x = 0; x < 80; ++x) {
			printf("%c", spots[y][x] ? '.' : '#');
		}
		printf("\n");
	}
}

int main(void)
{
	/* This can be created any way you'd like. This is an 80x24 map filled
	 * with walls (0).
	 */
	int spots[24][80] = {{WALL}};
	/* The only three members that are required when using Butterfly. */
	struct bf_farm farm = {
		.spots = (void *)spots,
		.width = 80,
		.height = 24
	};
	/* This is to "seed" the map with a safe spot to travel. This ensures
	 * everything is connected if we always connect to a safe spot to
	 * flutter.
	 *
	 * Building the instincts (instructions).
	 * - First is the morph instinct which is to start anywhere on the
	 *   map.
	 * - Look is how we dig, this says just at each step dig a 1 FLOOR
	 *   tile.
	 * - And finally how to die, which is right after fluttering and
	 *   looking just once.
	 */
	struct bf_instinct carve_start[] = {
		{.event = BF_MORPH, .action = BF_MORPH_AT_RANDOM_SPOT},
		{.event = BF_LOOK, .action = BF_LOOK_1_AREA, .args = {FLOOR}},
		{.event = BF_DIE, .action = BF_DIE_AFTER_N, {1}}
	};
	/* The actual carving instincts. We build by always finding our way
	 * back to a safe spot.
	 *
	 * Building the instincts (instructions).
	 * - First is the morph instinct which is to start anywhere on the
	 *   map.
	 * - Next is the goal, which is any safe spot on the map that was
	 *   commited.
	 * - Flutter randomly with a 60% weighted walk to the goal.
	 * - Look in a plus symbol. I find this digs really nice caves.
	 * - And finally how to die, by arriving at any safe spot.
	 */
	struct bf_instinct carve[] = {
		{.event = BF_MORPH, .action = BF_MORPH_AT_RANDOM_SPOT},
		{.event = BF_GOAL, .action = BF_GOAL_RANDOM_SAFE_SPOT},
		{.event = BF_FLUTTER, .action = BF_FLUTTER_RANDOMLY_TO_GOAL, {60}},
		{.event = BF_LOOK, .action = BF_LOOK_PLUS_AREA, .args = {FLOOR}},
		{.event = BF_DIE, .action = BF_DIE_AT_SAFE_SPOT},
	};
	/* Run the instincts on our map. */
	BF_SPAWN_ARR(&farm, carve_start, NULL);
	/* Commit these changes. */
	bf_commit(&farm);

	/* Now carve out the rest. Commiting after each loop so the new spawns
	 * can find the new safe spots.
	 */
	for (int i = 0; i < 20; ++i) {
		BF_SPAWN_ARR(&farm, carve, NULL);
		bf_commit(&farm);
	}

	draw(spots);
	return 0;
}
