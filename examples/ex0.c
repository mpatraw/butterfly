
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
	/* This can be any way you'd like. This is an 80x24 map filled with
	 * walls (0).
	 */
	int spots[24][80] = {{WALL}};
	/* The only three members that are required when using Butterfly. */
	struct bf_farm farm = {
		.spots = (void *)spots,
		.width = 80,
		.height = 24
	};
	/* Building the instincts (instructions).
	 * - First is the morph instinct which is to start anywhere on the
	 *   map.
	 * - Next is the goal, which is another random spot.
	 * - Flutter is how it moves somewhere, in this case, just a line.
	 * - Look is how we dig, this says just at each step dig a 1 FLOOR
	 *   tile.
	 * - And finally how to die, by arriving at our goal (flutter
	 *   guarantees this).
	 */
	struct bf_instinct carve[] = {
		{.event = BF_MORPH, .action = BF_MORPH_AT_RANDOM_SPOT},
		{.event = BF_GOAL, .action = BF_GOAL_RANDOM_SPOT},
		{.event = BF_FLUTTER, .action = BF_FLUTTER_LINE_TO_GOAL},
		{.event = BF_LOOK, .action = BF_LOOK_1_AREA, .args = {FLOOR}},
		{.event = BF_DIE, .action = BF_DIE_AT_GOAL},
	};
	/* Run the instincts on our map. */
	BF_SPAWN_ARR(&farm, carve, NULL);
	/* Commit these changes. */
	bf_commit(&farm);

	draw(spots);
	return 0;
}
