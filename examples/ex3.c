
#include <stdio.h>

#include <butterfly.h>

enum {
	WALL,
	FLOOR
};

static void draw(int spots[10][20])
{
	for (int y = 0; y < 10; ++y) {
		for (int x = 0; x < 20; ++x) {
			printf("%c", spots[y][x] ? '.' : '#');
		}
		printf("\n");
	}
}

int main(void)
{
	int spots[10][20] = {{WALL}};
	struct bf_farm farm = {
		.spots = (void *)spots,
		.width = 20,
		.height = 10
	};
	struct bf_instinct carve_start[] = {
		{.action = BF_MORPH_AT_RANDOM_SPOT},
		{.action = BF_LOOK_1_AREA, .args = {FLOOR}},
		{.action = BF_DIE_AFTER_N, {1}}
	};
	struct bf_instinct carve[] = {
		{.action = BF_MORPH_AT_RANDOM_SPOT},
		{.action = BF_GOAL_RANDOM_SAFE_SPOT},
		{.action = BF_FLUTTER_WEIGHTED_4, {60}},
		{.action = BF_LOOK_PLUS_AREA, .args = {FLOOR}},
		{.action = BF_DIE_AT_SAFE_SPOT},
	};
	BF_SPAWN_ARR(&farm, carve_start, NULL);
	bf_commit(&farm);

	for (int i = 0; i < 2; ++i) {
		BF_SPAWN_ARR(&farm, carve, NULL);
		bf_commit(&farm);
	}

	draw(spots);
	return 0;
}
