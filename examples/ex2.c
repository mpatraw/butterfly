
#include <stdio.h>

#include <butterfly.h>

enum {
	WALL,
	FLOOR
};

static void draw(int spots[200][100])
{
	for (int y = 0; y < 200; ++y) {
		for (int x = 0; x < 100; ++x) {
			printf("%c", spots[y][x] ? '.' : '#');
		}
		printf("\n");
	}
}

int main(void)
{
	int spots[200][100] = {{WALL}};
	struct bf_farm farm = {
		.spots = (void *)spots,
		.width = 100,
		.height = 200
	};
	struct bf_instinct carve_start[] = {
		{.event = BF_MORPH, .action = BF_MORPH_AT_RANDOM_SPOT},
		{.event = BF_LOOK, .action = BF_LOOK_1_AREA, .args = {FLOOR}},
		{.event = BF_DIE, .action = BF_DIE_AFTER_N, {1}}
	};
	struct bf_instinct carve[] = {
		{.event = BF_MORPH, .action = BF_MORPH_AT_RANDOM_SPOT},
		{.event = BF_GOAL, .action = BF_GOAL_RANDOM_SAFE_SPOT},
		{.event = BF_FLUTTER, .action = BF_FLUTTER_WEIGHTED_4, {60}},
		{.event = BF_LOOK, .action = BF_LOOK_PLUS_AREA, .args = {FLOOR}},
		{.event = BF_DIE, .action = BF_DIE_AT_SAFE_SPOT},
	};
	BF_SPAWN_ARR(&farm, carve_start, NULL);
	bf_commit(&farm);

	for (int i = 0; i < 50; ++i) {
		BF_SPAWN_ARR(&farm, carve, NULL);
		bf_commit(&farm);
	}

	draw(spots);
	return 0;
}
