
#include <stdio.h>

#include <butterfly.h>

enum {
	WALL,
	FLOOR
};

int main(void)
{
	int spots[24][80] = {{WALL}};
	struct bf_farm farm = {
		.spots = (void *)spots,
		.width = 80,
		.height = 24
	};
	struct bf_instinct instincts[] = {
		{.event = BF_MORPH, .action = BF_MORPH_AT_RANDOM_SPOT},
		{.event = BF_GOAL, .action = BF_GOAL_RANDOM_SPOT},
		{.event = BF_FLUTTER, .action = BF_FLUTTER_TUNNEL_TO_GOAL},
		{.event = BF_LOOK, .action = BF_LOOK_1_AREA, .args = {FLOOR}},
		{.event = BF_DIE, .action = BF_DIE_AT_GOAL},
	};
	struct bf_instinct instincts2[] = {
		{.event = BF_MORPH, .action = BF_MORPH_AT_LAST_DEATH_SPOT},
		{.event = BF_LOOK, .action = BF_LOOK_RECT_AREA, .args = {FLOOR, 2, 2}},
		{.event = BF_DIE, .action = BF_DIE_AFTER_N, .args = {1}},
	};
	BF_SPAWN_ARR(&farm, instincts, 1);
	BF_SPAWN_ARR(&farm, instincts2, 1);

	for (int y = 0; y < 24; ++y) {
		for (int x = 0; x < 80; ++x) {
			printf("%c", spots[y][x] ? '.' : '#');
		}
		printf("\n");
	}
	return 0;
}
