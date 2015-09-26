
#include <stdio.h>

#include <BearLibTerminal.h>

#include <bzzd.h>

char lookup[] = {
	'#', '.'
};

void print_park(struct bzzd_park *park)
{
	int x, y, spot;
	for (y = 0; y < bzzd_get_park_height(park); ++y) {
		for (x = 0; x < bzzd_get_park_width(park); ++x) {
			spot = bzzd_get_spot(park, x, y);
			terminal_put(x, y, lookup[spot]);
		}
	}
	terminal_refresh();
}

int generate_park(struct bzzd_park *park)
{
	struct bzzd_guy *guy;
	guy = bzzd_binge(park);

	bzzd_pee_everywhere(guy, 0);
	bzzd_dry_fresh(park);

	bzzd_wakeup_random(guy);
	bzzd_target_random(guy);

	while (!bzzd_is_on_target(guy)) {
		bzzd_pee_plus(guy, 1);
		bzzd_stagger_to_target(guy, 0.6);
	}

	bzzd_blackout(guy);

	return 1;
}

int main(void)
{
	struct bzzd_park *park;

	park = bzzd_new_park(80, 25);

	generate_park(park);

	terminal_open();

	print_park(park);

	while (terminal_read() != TK_CLOSE)
		;

	terminal_close();

	bzzd_close_park(park);
	return 0;
}
