
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
int bzzd_get_park_width(struct bzzd_park *park);
int bzzd_get_park_height(struct bzzd_park *park);
int bzzd_get_spot(struct bzzd_park *park, int x, int y);
void bzzd_set_spot(struct bzzd_park *park, int x, int y, int to);
int bzzd_is_inside_park(struct bzzd_park *park, int x, int y);
int bzzd_is_opened_spot(struct bzzd_park *park, int x, int y);
int bzzd_is_marked_spot(struct bzzd_park *park, int x, int y);
void bzzd_mark(struct bzzd_park *park, int x, int y, int with);
void bzzd_flush_marks(struct bzzd_park *park);
void bzzd_set_fence_size(struct bzzd_park *park, int size);
int bzzd_count_opened(struct bzzd_park *park);
double bzzd_percent_park_opened(struct bzzd_park *park);
void bzzd_find_random_spot(struct bzzd_park *park, unsigned *x, unsigned *y);

struct bzzd_guy *bzzd_start(struct bzzd_park *park);
void bzzd_stop(struct bzzd_guy *guy);

#ifdef __cplusplus
}
#endif

#endif
