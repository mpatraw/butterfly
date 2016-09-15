
#include <stdio.h>

#include <butterfly.h>

enum {
	WALL,
	FLOOR
};

int main(void)
{
	int spots[24][80] = {{WALL}};
	struct bf_farm farm = {(void *)spots, 80, 24, 0};
	struct bf_instinct instincts[] = {
		{BF_MORPH, BF_MORPH_AT_RANDOM_SPOT, {0}},
		{BF_FLUTTER, BF_FLUTTER_RANDOMLY, {0}},
		{BF_LOOK, BF_LOOK_PLUS_AREA, {FLOOR}},
		{BF_DIE, BF_DIE_AFTER_N_FLUTTERS, {1000}},
	};
	bf_spawn(&farm, instincts, sizeof(instincts) / sizeof(*instincts));

	for (int y = 0; y < 24; ++y) {
		for (int x = 0; x < 80; ++x) {
			printf("%c", spots[y][x] ? '.' : '#');
		}
		printf("\n");
	}
	return 0;
}
