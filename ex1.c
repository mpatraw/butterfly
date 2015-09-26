
#include <stdio.h>

#include <bzzd.h>

void print_park(struct bzzd_park *park)
{
	int x, y;
	for (y = 0; y < bzzd_get_park_height(park); ++y) {
		for (x = 0; x < bzzd_get_park_width(park); ++x) {
			printf("%d", bzzd_get_spot(park, x, y));
		}
		printf("\n");
	}
}

int main(void)
{
	struct bzzd_park *park;
	struct bzzd_guy *guy;

	park = bzzd_new_park(80, 24);
	guy = bzzd_binge(park);

	bzzd_pee_everywhere(guy, 0);
	bzzd_dry_fresh(park);

	bzzd_wakeup_random(guy);
	bzzd_target_random(guy);

	while (!bzzd_is_on_target(guy)) {
		bzzd_pee_plus(guy, 1);
		bzzd_stagger_to_target(guy, 0.6);
	}

	print_park(park);

	bzzd_blackout(guy);
	bzzd_close_park(park);
	return 0;
}
