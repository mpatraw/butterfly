
#include <limits.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "bzzd.h"

/*******************************************************************************

some top level declarations

*/


/* 
 * very simple way to get size specific types, should work on most platforms.
 */
#if UCHAR_MAX == 0xff
typedef unsigned char u8;
#endif
#if SCHAR_MAX == 0x7f
typedef signed char s8;
#else
#error "system does not support 8-bit integers"
#endif

#if UINT_MAX == 0xffff
typedef unsigned int u16;
#elif USHRT_MAX == 0xffff
typedef unsigned short u16;
#else
#error "system does not support 16-bit integers"
#endif
#if SINT_MAX == 0x7fff
typedef signed int s16;
#elif SHRT_MAX == 0x7fff
typedef signed short s16;
#else
#error "system does not support 16-bit integers"
#endif

#if ULONG_MAX == 0xfffffffful
typedef unsigned long u32;
#elif UINT_MAX == 0xfffffffful
typedef unsigned int u32;
#elif USHRT_MAX == 0xfffffffful
typedef unsigned short u32;
#else
#error "system does not support 32-bit integers"
#endif
#if LONG_MAX == 0x7fffffffl
typedef signed long s32;
#elif INT_MAX == 0x7fffffffl
typedef signed int s32;
#elif SHRT_MAX == 0x7fffffffl
typedef signed short s32;
#else
#error "system does not support 32-bit integers"
#endif


/*******************************************************************************

random number generator using xor128

*/

/* fast speed, lw memory, period = 2^128 - 1. */
enum {XOR128_K = 4};

struct xor128_state {
	u32 q[XOR128_K];
};

static void xor128_seed(struct xor128_state *x, u32 seed)
{
	int i;

	srand(seed);
	for (i = 0; i < XOR128_K; ++i) {
		x->q[i] = rand();
	}
}

static u32 xor128_next(struct xor128_state *x)
{
	u32 t;
	t = (x->q[0] ^ (x->q[0] << 11));
	x->q[0] = x->q[1];
	x->q[1] = x->q[2];
	x->q[2] = x->q[3];
	return x->q[3] = x->q[3] ^ (x->q[3] >> 19) ^ (t ^ (t >> 8));
}

/* [0.0, 1.0) */
static double xor128_next_unit(struct xor128_state *x)
{
	return xor128_next(x) * 2.3283064365386963e-10;
}

/* [0, sz) */
static u32 xor128_next_index(struct xor128_state *x, u32 sz)
{
	return floor(xor128_next_unit(x) * sz);
}

/* [min, max] */
static u32 xor128_next_range(struct xor128_state *x, u32 min, u32 max)
{
	return xor128_next_index(x, max - min + 1) + min;
}

/*******************************************************************************

pointset. used internally for keeping track of visited locations.

*/

struct point {
	int x;
	int y;
};

struct point make_point(int x, int y)
{
	struct point p;
	p.x = x;
	p.y = y;
	return p;
}

#define INDEX1(arr, i) ((arr)[(i)])
#define INDEX2(arr, x, y, w) INDEX1(arr, (y) * (w) + (x))

#define PS_FOR(ps, i, p) \
	for ((i) = 0, (p) = &(ps)->arr[(i)]; \
		(i) < (ps)->length; \
		(i)++, (p)++)

struct pointset {
	struct point *arr;
	int length;

	int *map;
	int width, height;
};


static int ps_init(struct pointset *ps, int width, int height)
{
	memset(ps, 0, sizeof(*ps));

	ps->arr = malloc(sizeof(*ps->arr) * width * height);
	if (!ps->arr) {
		goto alloc_failure;
	}
	memset(ps->arr, 0, sizeof(*ps->arr) * width * height);

	ps->map = malloc(sizeof(*ps->map) * width * height);
	if (!ps->map) {
		goto alloc_failure;
	}
	memset(ps->map, 0, sizeof(*ps->map) * width * height);

	ps->length = 0;
	ps->width = width;
	ps->height = height;

	return 0;

alloc_failure:
	if (ps->map)
		free(ps->map);
	if (ps->arr)
		free(ps->arr);
	return -1;
}

static void ps_uninit(struct pointset *ps)
{
	free(ps->arr);
	free(ps->map);
}

static int ps_has(struct pointset *ps, struct point p)
{
	if (INDEX2(ps->map, p.x, p.y, ps->width))
		return 1;
	return 0;
}

static void ps_add(struct pointset *ps, struct point p)
{
	int had = ps_has(ps, p);
	
	INDEX2(ps->map, p.x, p.y, ps->width) = 1;
	/* we just added 1 to the length. */
	ps->arr[ps->length] = p;

	if (!had) {
		ps->length++;
	}
}

static void ps_rem(struct pointset *ps, struct point p)
{
	int i;
	int had;

	had = ps_has(ps, p);

	INDEX2(ps->map, p.x, p.y, ps->width) = 0;
	for (i = 0; i < ps->length; ++i) {
		if (ps->arr[i].x == p.x && ps->arr[i].y == p.y) {
			break;
		}
	}
	memmove(&ps->arr[i], &ps->arr[i + 1],
		(ps->length - i) * sizeof(*ps->arr));

	if (had) {
		ps->length--;
	}
}

static struct point ps_rnd(struct pointset *ps, struct xor128_state *x)
{
	return ps->arr[xor128_next_index(x, ps->length - 1)];
}

static void ps_clr(struct pointset *ps)
{
	ps->length = 0;
	memset(ps->map, 0, sizeof *ps->map * ps->width * ps->height);
}

/*******************************************************************************

park functions.

*/

#define MARKED_THRESHOLD 1

struct bzzd_park {
	struct pointset *markedset;
	struct pointset *markingset;
	struct xor128_state *rng;
	u32 seed;

	int *spots;
	int width, height;

	int fence;

	int owns_spots;
};

struct bzzd_park *bzzd_open_park(int *spots, int w, int h)
{
	struct bzzd_park *park = malloc(sizeof(*park));
	if (!park) {
    	goto alloc_failure;
	}
	memset(park, 0, sizeof(*park));

	park->markingset = malloc(sizeof(*park->markingset));
	if (!park->markingset) {
		goto alloc_failure;
	}

	park->markedset = malloc(sizeof(*park->markedset));
	if (!park->markedset) {
		goto alloc_failure;
	}

	park->rng = malloc(sizeof(*park->rng));
	if (!park->rng) {
		goto alloc_failure;
	}

	if (ps_init(park->markingset, w, h)) {
		goto markingset_init_failure;
	}

	if (ps_init(park->markedset, w, h)) {
		goto markedset_init_failure;
	}

	park->seed = time(NULL);
	xor128_seed(park->rng, park->seed);

	park->spots = spots;
	park->width = w;
	park->height = h;

	park->fence = 0;

	park->owns_spots = 0;

	return park;

markedset_init_failure:
	ps_uninit(park->markingset);
markingset_init_failure:
alloc_failure:
	if (park->rng) {
		free(park->rng);
	}
	if (park->markedset) {
		free(park->markedset);
	}
	if (park->markingset) {
		free(park->markingset);
	}
	if (park) {
		free(park);
	}
	return NULL;
}

struct bzzd_park *bzzd_new_park(int w, int h)
{
	struct bzzd_park *park;
	int *spots;

	spots = malloc(sizeof(*spots) * w * h);
	if (!spots) {
		goto alloc_failure;
	}
	memset(spots, 0, sizeof(*spots) * w * h);

	park = bzzd_open_park(spots, w, h);
	if (!park) {
		goto alloc_failure;
	}

	park->owns_spots = 1;
	return park;

alloc_failure:
	if (park) {
		bzzd_close_park(park);
	}

	if (spots) {
		free(spots);
	}

	return NULL;
}

void bzzd_close_park(struct bzzd_park *park)
{
	ps_uninit(park->markingset);
	ps_uninit(park->markedset);
	free(park->rng);
	free(park->markedset);
	free(park->markingset);
	free(park->spots);
	free(park);
}

int bzzd_get_seed(struct bzzd_park *park)
{
	return park->seed;
}

void bzzd_set_seed(struct bzzd_park *park, int seed)
{
	park->seed = seed;
	xor128_seed(park->rng, park->seed);
}

int bzzd_get_park_width(struct bzzd_park *park)
{
	return park->width;
}

int bzzd_get_park_height(struct bzzd_park *park)
{
	return park->height;
}

int bzzd_get_spot(struct bzzd_park *park, int x, int y)
{
	return park->spots[y * park->width + x];
}

void bzzd_set_spot(struct bzzd_park *park, int x, int y, int to)
{
	park->spots[y * park->width + x] = to;
}

int bzzd_is_inside_park(struct bzzd_park *park, int x, int y)
{
	int in_x = x >= park->fence && x < park->width - park->fence;
	int in_y = y >= park->fence && y < park->height - park->fence;
	return in_x && in_y;
}

int bzzd_is_marked_spot(struct bzzd_park *park, int x, int y)
{
	return ps_has(park->markedset, make_point(x, y));
}

int bzzd_is_marking_spot(struct bzzd_park *park, int x, int y)
{
	return ps_has(park->markingset, make_point(x, y));
}

void bzzd_marking(struct bzzd_park *park, int x, int y, int with)
{
	if (bzzd_is_inside_park(park, x, y)) {
		if (with >= 1) {
			ps_add(park->markingset, make_point(x, y));
		} else if (with <= -1) {
			ps_rem(park->markingset, make_point(x, y));
			ps_rem(park->markedset, make_point(x, y));
		}
		bzzd_set_spot(park, x, y, with);
	}
}

void bzzd_flush_markings(struct bzzd_park *park)
{
	int i;
	struct point *pi;

	PS_FOR(park->markingset, i, pi) {
		ps_add(park->markedset, *pi);
	}

	ps_clr(park->markingset);
}

void bzzd_set_fence_size(struct bzzd_park *park, int size)
{
	park->fence = size;
}

int bzzd_count_marked(struct bzzd_park *park)
{
	return park->markedset->length;
}

double bzzd_percent_park_marked(struct bzzd_park *park)
{
	int size = park->width * park->height;
	return (double)bzzd_count_marked(park) / (double)size;
}

void bzzd_find_random_marked_spot(struct bzzd_park *park, int *x, int *y)
{
	struct point p = ps_rnd(park->markedset, park->rng);
	*x = p.x;
	*y = p.y;
}

/*******************************************************************************

buzzed guy.

*/

struct bzzd_guy {
	struct bzzd_park *park;

	int x, y;
	int target_x, target_y;

	unsigned char path_data[256];
	int (*pathing_function) (struct bzzd_guy *);
};

struct bzzd_guy *bzzd_binge(struct bzzd_park *park)
{
	struct bzzd_guy *guy = malloc(sizeof(*guy));
	if (!guy) {
		goto alloc_failure;
	}
	memset(guy, 0, sizeof(*guy));

	guy->park = park;

	return guy;

alloc_failure:
	return NULL;
}

void bzzd_blackout(struct bzzd_guy *guy)
{
	free(guy);
}

void bzzd_get_coords(struct bzzd_guy *guy, int *x, int *y)
{
	if (x) *x = guy->x;
	if (y) *y = guy->y;
}

void bzzd_get_target(struct bzzd_guy *guy, int *x, int *y)
{
	if (x) *x = guy->target_x;
	if (y) *y = guy->target_y;
}

void bzzd_set_coords(struct bzzd_guy *guy, int x, int y)
{
	guy->x = x;
	guy->y = y;
}

void bzzd_set_target(struct bzzd_guy *guy, int x, int y)
{
	guy->target_x = x;
	guy->target_y = y;
}

void bzzd_wakeup_random(struct bzzd_guy *guy)
{
	int w = guy->park->width;
	int h = guy->park->height;
	int f = guy->park->fence;
	guy->x = xor128_next_range(guy->park->rng, f, w - f - 1);
	guy->y = xor128_next_range(guy->park->rng, f, h - f - 1);
}

