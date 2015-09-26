
#include <stdio.h>

#include <BearLibTerminal.h>

#include <bzzd.h>

#define WIDTH 80
#define HEIGHT 25

struct tile
{
	int face;
	int blocks_movement, blocks_light;
	const char *lit, *unlit;
};

enum {
	STONE_WALL,
	DIRT_FLOOR,
	NTILES
};

static struct tile tiles[NTILES] = {
	{'#', 1, 1, "white", "darkest grey"},
	{'.', 0, 0, "orange", "darkest orange"},
};

static unsigned map[HEIGHT][WIDTH] = {{STONE_WALL}};

static void draw_tile(int x, int y, int tile, int vis)
{
	terminal_bkcolor(color_from_name("black"));
	if (vis) {
		terminal_color(color_from_name(tiles[tile].lit));
	} else {
		terminal_color(color_from_name(tiles[tile].unlit));
	}
	terminal_put(x, y, tiles[tile].face);
}

static void print_park(struct bzzd_park *park)
{
	int x, y, spot;
	for (y = 0; y < bzzd_get_park_height(park); ++y) {
		for (x = 0; x < bzzd_get_park_width(park); ++x) {
			spot = bzzd_get_spot(park, x, y);
			draw_tile(x, y, spot, 1);
		}
	}
	terminal_refresh();
}

static void stagger_randomly(struct bzzd_park *park, struct bzzd_guy *guy)
{
	bzzd_wakeup_random(guy);
	bzzd_target_random_marked(guy);

	while (!bzzd_is_on_marked(guy))
	{
		bzzd_pee_plus(guy, DIRT_FLOOR);
		bzzd_stagger_to_target(guy, 0.51);
	}

	bzzd_dry_fresh(park);
}

static int generate_park(struct bzzd_park *park)
{
	struct bzzd_guy *guy;
	guy = bzzd_binge(park);

	bzzd_pee_everywhere(guy, STONE_WALL);
	bzzd_dry_fresh(park);

	bzzd_wakeup_random(guy);
	bzzd_pee_plus(guy, DIRT_FLOOR);
	bzzd_dry_fresh(park);

    int tries = HEIGHT * HEIGHT;
    while (bzzd_percent_park_marked(park) < 0.55 && tries --> 0) {
        stagger_randomly(park, guy);
    }

	bzzd_blackout(guy);

	return 1;
}

int main(void)
{
	struct bzzd_park *park;

	park = bzzd_open_park((void *)map, 80, 25);

	generate_park(park);

	terminal_open();

	print_park(park);

	while (terminal_read() != TK_CLOSE)
		;

	terminal_close();

	bzzd_close_park(park);
	return 0;
}
