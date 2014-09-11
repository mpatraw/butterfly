
#include <stdio.h>

#include <bzzd.h>

int main(void)
{
	struct bzzd_park *park;
	struct bzzd_guy *guy;
	int x;
	int y;

	park = bzzd_new_park(10, 10);
	guy = bzzd_start(park);

	for (y = 0; y < bzzd_get_park_height(park); ++y) {
		for (x = 0; x < bzzd_get_park_width(park); ++x) {
			printf("%d", bzzd_get_spot(park, x, y));
		}
		printf("\n");
	}

	bzzd_stop(guy);
	bzzd_close_park(park);
	return 0;
}
