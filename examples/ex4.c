
#include <stdio.h>

#include <BearLibTerminal.h>
#include <butterfly.h>

enum {
	WALL,
	FLOOR
};

struct tile_def {
	char glyph;
	const char *fg;
};

static struct tile_def tile_defs[] = {
	{'#', "grey"},
	{'.', "grey"},
};

int main(void)
{
	int spots[25][80] = {{WALL}};
	struct bf_config room_config = {
		.error_on_looking_at_safe = 1
	};
	struct bf_config tunnel_config = {
		.cycle_looking = 1
	};
	struct bf_farm farm = {
		.spots = (void *)spots,
		.width = 80,
		.height = 25,
		.seed = 0
	};
	struct bf_instinct room[] = {
		{.event = BF_MORPH, .action = BF_MORPH_AT_RANDOM_SPOT},
		{.event = BF_LOOK, .action = BF_LOOK_RECT_AREA, .args = {FLOOR, 2, 2}},
		{.event = BF_LOOK, .action = BF_LOOK_RECT_AREA, .args = {FLOOR, 3, 3}},
		{.event = BF_LOOK, .action = BF_LOOK_CIRCLE_AREA, .args = {FLOOR, 3}},
		{.event = BF_LOOK, .action = BF_LOOK_CIRCLE_AREA, .args = {FLOOR, 4}},
		{.event = BF_LOOK, .action = BF_LOOK_CIRCLE_AREA, .args = {FLOOR, 6}},
		{.event = BF_DIE, .action = BF_DIE_AFTER_N, .args = {1}},
	};
	struct bf_instinct tunnel[] = {
		{.event = BF_MORPH, .action = BF_MORPH_AT_LAST_DEATH_SPOT},
		{.event = BF_GOAL, .action = BF_GOAL_RANDOM_SAFE_SPOT},
		{.event = BF_FLUTTER, .action = BF_FLUTTER_TUNNEL_TO_GOAL},
		{.event = BF_LOOK, .action = BF_LOOK_1_AREA, .args = {FLOOR}},
		{.event = BF_LOOK, .action = BF_LOOK_1_AREA, .args = {FLOOR}},
		{.event = BF_LOOK, .action = BF_LOOK_BIG_PLUS_AREA, .args = {FLOOR}},
		{.event = BF_DIE, .action = BF_DIE_AT_SAFE_SPOT},
	};

	BF_SPAWN_ARR(&farm, room, NULL);
	bf_commit(&farm);
	printf("seed: %d\n", farm.seed);
	for (int i = 0; i < 15; ++i) {
		if (BF_SPAWN_ARR(&farm, room, &room_config)) {
			continue;
		}
		BF_SPAWN_ARR(&farm, tunnel, &tunnel_config);
		bf_commit(&farm);
	}

	terminal_open();


	for (int y = 0; y < 25; ++y) {
		for (int x = 0; x < 80; ++x) {
			struct tile_def t = tile_defs[spots[y][x]];
			terminal_color(color_from_name(t.fg));
			terminal_put(x, y, t.glyph);
		}
	}

	terminal_refresh();
	while (terminal_read() != TK_CLOSE) {
		terminal_refresh();
	}

	terminal_close();
	return 0;
}
