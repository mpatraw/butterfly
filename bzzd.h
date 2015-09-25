
#ifndef BZZD_H
#define BZZD_H

struct bzzd_park;
struct bzzd_guy;

#ifdef __cplusplus
extern "C" {
#endif

#ifdef BZZD_SLURRED
#define WATSH_DER		bzzd_spot_at
#define THISH_DER		bzzd_set_spot
#endif

struct bzzd_park *bzzd_open_park(int *spots, int w, int h);
struct bzzd_park *bzzd_new_park(int w, int h);
void bzzd_close_park(struct bzzd_park *park);
int bzzd_get_seed(struct bzzd_park *park);
void bzzd_set_seed(struct bzzd_park *park, int seed);
int bzzd_get_park_west(struct bzzd_park *park);
int bzzd_get_park_north(struct bzzd_park *park);
int bzzd_get_park_east(struct bzzd_park *park);
int bzzd_get_park_south(struct bzzd_park *park);
int bzzd_get_park_width(struct bzzd_park *park);
int bzzd_get_park_height(struct bzzd_park *park);
int bzzd_get_spot(struct bzzd_park *park, int x, int y);
void bzzd_set_spot(struct bzzd_park *park, int x, int y, int to);
int bzzd_is_inside_park(struct bzzd_park *park, int x, int y);
int bzzd_is_marked_spot(struct bzzd_park *park, int x, int y);
int bzzd_is_fresh_spot(struct bzzd_park *park, int x, int y);
void bzzd_pee(struct bzzd_park *park, int x, int y, int pee);
void bzzd_dry_fresh(struct bzzd_park *park);
void bzzd_set_fence_size(struct bzzd_park *park, int size);
int bzzd_get_fence_size(struct bzzd_park *park);
int bzzd_count_marked(struct bzzd_park *park);
double bzzd_percent_park_marked(struct bzzd_park *park);
void bzzd_find_random_marked_spot(struct bzzd_park *park, int *x, int *y);

struct bzzd_guy *bzzd_binge(struct bzzd_park *park);
void bzzd_blackout(struct bzzd_guy *guy);
void bzzd_get_coords(struct bzzd_guy *guy, int *x, int *y);
void bzzd_get_target(struct bzzd_guy *guy, int *x, int *y);
void bzzd_set_coords(struct bzzd_guy *guy, int x, int y);
void bzzd_set_target(struct bzzd_guy *guy, int x, int y);

void bzzd_wakeup_fixed(struct bzzd_guy *guy, int x, int y);
void bzzd_wakeup_random(struct bzzd_guy *guy);
void bzzd_wakeup_random_west(struct bzzd_guy *guy);
void bzzd_wakeup_random_east(struct bzzd_guy *guy);
void bzzd_wakeup_random_north(struct bzzd_guy *guy);
void bzzd_wakeup_random_south(struct bzzd_guy *guy);
void bzzd_wakeup_random_west_edge(struct bzzd_guy *guy);
void bzzd_wakeup_random_east_edge(struct bzzd_guy *guy);
void bzzd_wakeup_random_north_edge(struct bzzd_guy *guy);
void bzzd_wakeup_random_south_edge(struct bzzd_guy *guy);
void bzzd_wakeup_random_westeast_edge(struct bzzd_guy *guy);
void bzzd_wakeup_random_northsouth_edge(struct bzzd_guy *guy);
void bzzd_wakeup_random_edge(struct bzzd_guy *guy);
void bzzd_wakeup_random_marked(struct bzzd_guy *guy);
void bzzd_wakeup_random_fresh(struct bzzd_guy *guy);

void bzzd_pee_everywhere(struct bzzd_guy *guy, int pee);
void bzzd_pee_1(struct bzzd_guy *guy, int pee);
void bzzd_pee_plus(struct bzzd_guy *guy, int pee);
void bzzd_pee_x(struct bzzd_guy *guy, int pee);
void bzzd_pee_rect(struct bzzd_guy *guy, int hw, int hh, int pee);
void bzzd_pee_circle(struct bzzd_guy *guy, int r, int pee);

#ifdef __cplusplus
}
#endif

#endif
