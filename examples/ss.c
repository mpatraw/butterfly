#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include <libtcod.h>
#include <libtcod_int.h>

#include <butterfly.h>

#define WIDTH   160
#define HEIGHT  80

struct tile_type {
	int face;
	bool blocks_movement, blocks_light;
	TCOD_color_t lit, unlit;
};

enum {
	STONE_WALL,
	DIRT_WALL,
	ALIEN_WALL,
	BLOODY_WALL,
	STONE_FLOOR,
	DIRT_FLOOR,
	ALIEN_FLOOR,
	BLOODY_FLOOR,
	WOOD_DOOR, WOOD_TABLE,
	WATER_FLOWING,
	NTILES
};

static const struct tile_type tiles[NTILES] = {
	{'#', true, true, {TCOD_WHITE}, {TCOD_DARKEST_GREY}},
	{'#', true, true, {TCOD_ORANGE}, {TCOD_DARKEST_ORANGE}},
	{'%', true, true, {TCOD_PURPLE}, {TCOD_DARKEST_PURPLE}},
	{'#', true, true, {TCOD_RED}, {TCOD_DARKEST_RED}},
	{'.', false, false, {TCOD_WHITE}, {TCOD_DARKEST_GREY}},
	{'.', false, false, {TCOD_ORANGE}, {TCOD_DARKEST_ORANGE}},
	{'.', false, false, {TCOD_PURPLE}, {TCOD_DARKEST_PURPLE}},
	{'.', false, false, {TCOD_RED}, {TCOD_DARKEST_RED}},
	{'+', false, true, {TCOD_YELLOW}, {TCOD_DARKEST_YELLOW}},
	{194, false, false, {TCOD_ORANGE}, {TCOD_DARKEST_ORANGE}},
	{'~', false, false, {TCOD_BLUE}, {TCOD_DARKEST_BLUE}},
};

static unsigned map[HEIGHT][WIDTH] = {{STONE_WALL}};

static void draw_tile(int x, int y, unsigned tile, bool vis)
{
	if (vis) {
		TCOD_console_set_default_foreground(NULL, tiles[tile].lit);
	} else {
		TCOD_console_set_default_foreground(NULL, tiles[tile].unlit);
	}
	TCOD_console_put_char(NULL, x, y, tiles[tile].face, TCOD_BKGND_NONE);
}

static void carve_seed(struct bf_farm *farm)
{
	static struct bf_instinct seed[] = {
		{.event = BF_MORPH, .action = BF_MORPH_AT_FIXED_SPOT, .args = {42, 42}},
		{.event = BF_LOOK, .action = BF_LOOK_1_AREA, .args = {STONE_FLOOR}},
		{.event = BF_DIE, .action = BF_DIE_AFTER_N, {1}}
	};
	/* Run the instincts on our map. */
	BF_SPAWN_ARR(farm, seed, NULL);
	/* Commit these changes. */
	bf_commit(farm);
}

static void carve_randomly(struct bf_farm *farm)
{
	static struct bf_config cave_config = {
		.enable_neighbor_look_8 = 1,
		.neighbor_look_8 = DIRT_WALL,
	};
	static struct bf_instinct cave[] = {
		{.event = BF_MORPH, .action = BF_MORPH_AT_RANDOM_SPOT},
		{.event = BF_GOAL, .action = BF_GOAL_RANDOM_SAFE_SPOT},
		{.event = BF_FLUTTER, .action = BF_FLUTTER_WEIGHTED_4, {51}},
		{.event = BF_LOOK, .action = BF_LOOK_PLUS_AREA, .args = {DIRT_FLOOR}},
		{.event = BF_DIE, .action = BF_DIE_AT_SAFE_SPOT}
	};
	BF_SPAWN_ARR(farm, cave, &cave_config);
	bf_commit(farm);
}

static void carve_room_and_corridor(struct bf_farm *farm)
{
	static struct bf_config room_config = {
		.error_on_looking_at_safe = 1
	};
	static struct bf_instinct room[] = {
		{.event = BF_MORPH, .action = BF_MORPH_AT_RANDOM_SPOT},
		{.event = BF_LOOK, .action = BF_LOOK_SHRINKING_RECT_AREA, .args = {STONE_FLOOR, 3, 8}},
		{.event = BF_LOOK, .action = BF_LOOK_SHRINKING_CIRCLE_AREA, .args = {STONE_FLOOR, 3, 8}},
		{.event = BF_DIE, .action = BF_DIE_AFTER_N, {1}}
	};
	static struct bf_instinct tunnel[] = {
		{.event = BF_MORPH, .action = BF_MORPH_AT_LAST_DEATH_SPOT},
		{.event = BF_GOAL, .action = BF_GOAL_RANDOM_SAFE_SPOT},
		{.event = BF_FLUTTER, .action = BF_FLUTTER_TUNNEL},
		{.event = BF_LOOK, .action = BF_LOOK_1_AREA, .args = {STONE_FLOOR}},
		{.event = BF_DIE, .action = BF_DIE_AT_SAFE_SPOT},
	};
	if (BF_SPAWN_ARR(farm, room, &room_config)) {
		/* failed to generate. */
		return;
	}
	BF_SPAWN_ARR(farm, tunnel, NULL);
	bf_commit(farm);
}

static void carve_rest(struct bf_farm *farm)
{
	if (bf_random(farm) < 0.3) {
		carve_randomly(farm);
	} else {
		carve_room_and_corridor(farm);
	}
}

static void generate_dungeon(struct bf_farm *farm)
{
	int tries = WIDTH * HEIGHT;
	memset(map, 0, sizeof(map));
	carve_seed(farm);
	while (bf_query(farm, BF_QUERY_SAFE_PERCENTAGE) < 65 && tries-- > 0) {
		carve_rest(farm);
	}
}

static void generate_fov(TCOD_map_t fov, bool explored[HEIGHT][WIDTH])
{
	struct tile_type t;
	int x, y;
	for (x = 0; x < WIDTH; ++x) {
		for (y = 0; y < HEIGHT; ++y) {
			t = tiles[map[y][x]];
			TCOD_map_set_properties(fov, x, y, !t.blocks_light, !t.blocks_movement);
			explored[y][x] = false;
		}
	}
}

int main(int argc, char *argv[])
{
	int x, y, tile;
	int px = 42, py = 42;
	struct bf_farm farm;

	farm = (struct bf_farm){
		.spots = (void *)map,
		.width = WIDTH,
		.height = HEIGHT,
		.last_dangerous = BLOODY_WALL
	};

	if (argc == 2) {
		farm.seed = atoi(argv[1]);
	} else {
		farm.seed = time(NULL);
	}
	printf("seed: %u\n", farm.seed);

	bool explored[HEIGHT][WIDTH] = {{false}};
	TCOD_map_t fov_map = TCOD_map_new(WIDTH, HEIGHT);

	TCOD_console_init_root(WIDTH, HEIGHT, "butterfly", false, TCOD_RENDERER_SDL);
	TCOD_sys_set_fps(30);

	generate_dungeon(&farm);
	generate_fov(fov_map, explored);

	while (!TCOD_console_is_window_closed()) {
		TCOD_key_t key = TCOD_console_check_for_keypress(TCOD_KEY_PRESSED);

		switch (key.vk) {
		case TCODK_LEFT:
			if (TCOD_map_is_walkable(fov_map, px - 1, py)) {
				px--;
			}
			printf("%d, %d\n", px, py);
			break;

		case TCODK_RIGHT:
			if (TCOD_map_is_walkable(fov_map, px + 1, py)) {
				px++;
			}
			printf("%d, %d\n", px, py);
			break;

		case TCODK_UP:
			if (TCOD_map_is_walkable(fov_map, px, py - 1)) {
				py--;
			}
			printf("%d, %d\n", px, py);
			break;

		case TCODK_DOWN:
			if (TCOD_map_is_walkable(fov_map, px, py + 1)) {
				py++;
			}
			printf("%d, %d\n", px, py);
			break;

		case TCODK_CHAR:
			if (key.c == 'r') {
				bf_cleanup(&farm);
				farm.seed = time(NULL);
				printf("seed: %u\n", farm.seed);
				generate_dungeon(&farm);
				px = 42, py = 42;
				generate_fov(fov_map, explored);
			} else if (key.c == 's') {
				for (x = 0; x < WIDTH; ++x) {
					for (y = 0; y < HEIGHT; ++y) {
						explored[y][x] = true;
					}
				}
			}
			break;

		default:
			break;
		}

		TCOD_map_compute_fov(fov_map, px, py, 20, true, FOV_BASIC);

		TCOD_console_clear(NULL);

		for (x = 0; x < WIDTH; ++x) {
			for (y = 0; y < HEIGHT; ++y) {
				tile = map[y][x];
				if (TCOD_map_is_in_fov(fov_map, x, y)) {
					draw_tile(x, y, tile, true);
					explored[y][x] = true;
				} else if (explored[y][x]) {
					draw_tile(x, y, tile, false);
				} else {
					continue;
				}
			}
		}

		TCOD_console_set_default_foreground(NULL, TCOD_red);
		TCOD_console_put_char(NULL, px, py, '@', TCOD_BKGND_NONE);

		TCOD_console_flush(false);
	}

	TCOD_map_delete(fov_map);

	return EXIT_SUCCESS;
}
