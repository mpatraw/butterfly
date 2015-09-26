
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
	struct pointset *freshset;
	struct xor128_state *rng;
	u32 seed;

	int *spots;
	int width, height;

	int fence;
	int threshold;

	int owns_spots;
};

struct bzzd_park *bzzd_open_park(int *spots, int w, int h)
{
	struct bzzd_park *park = malloc(sizeof(*park));
	if (!park) {
		goto alloc_failure;
	}
	memset(park, 0, sizeof(*park));

	park->freshset = malloc(sizeof(*park->freshset));
	if (!park->freshset) {
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

	if (ps_init(park->freshset, w, h)) {
		goto freshset_init_failure;
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
	park->threshold = 0;

	park->owns_spots = 0;

	return park;

markedset_init_failure:
	ps_uninit(park->freshset);
freshset_init_failure:
alloc_failure:
	if (park->rng) {
		free(park->rng);
	}
	if (park->markedset) {
		free(park->markedset);
	}
	if (park->freshset) {
		free(park->freshset);
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
	ps_uninit(park->freshset);
	ps_uninit(park->markedset);
	free(park->rng);
	free(park->markedset);
	free(park->freshset);
	if (park->owns_spots) {
		free(park->spots);
	}
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

int bzzd_get_park_west(struct bzzd_park *park)
{
	return park->fence;
}

int bzzd_get_park_north(struct bzzd_park *park)
{
	return park->fence;
}

int bzzd_get_park_east(struct bzzd_park *park)
{
	return park->width - park->fence - 1;
}

int bzzd_get_park_south(struct bzzd_park *park)
{
	return park->height - park->fence - 1;
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

int bzzd_is_fresh_spot(struct bzzd_park *park, int x, int y)
{
	return ps_has(park->freshset, make_point(x, y));
}

int bzzd_get_pee_threshold(struct bzzd_park *park)
{
	return park->threshold;
}

void bzzd_set_pee_threshold(struct bzzd_park *park, int pee)
{
	park->threshold = pee;
}

void bzzd_pee(struct bzzd_park *park, int x, int y, int pee)
{
	if (bzzd_is_inside_park(park, x, y)) {
		if (pee >= 1) {
			ps_add(park->freshset, make_point(x, y));
		} else if (pee <= park->threshold) {
			ps_rem(park->freshset, make_point(x, y));
			ps_rem(park->markedset, make_point(x, y));
		}
		bzzd_set_spot(park, x, y, pee);
	}
}

void bzzd_dry_fresh(struct bzzd_park *park)
{
	int i;
	struct point *pi;

	PS_FOR(park->freshset, i, pi) {
		ps_add(park->markedset, *pi);
	}

	ps_clr(park->freshset);
}

void bzzd_set_fence_size(struct bzzd_park *park, int size)
{
	park->fence = size;
}

int bzzd_get_fence_size(struct bzzd_park *park)
{
	return park->fence;
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

void bzzd_find_random_fresh_spot(struct bzzd_park *park, int *x, int *y)
{
	struct point p = ps_rnd(park->freshset, park->rng);
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

/*
wakeup functions.
*/

void bzzd_wakeup_fixed(struct bzzd_guy *guy, int x, int y)
{
	guy->x = x;
	guy->y = y;
}

void bzzd_wakeup_random(struct bzzd_guy *guy)
{
	int w, n, e, s;
	w = bzzd_get_park_west(guy->park);
	n = bzzd_get_park_north(guy->park);
	e = bzzd_get_park_east(guy->park);
	s = bzzd_get_park_south(guy->park);
	guy->x = xor128_next_range(guy->park->rng, w, e);
	guy->y = xor128_next_range(guy->park->rng, n, s);
}

void bzzd_wakeup_random_west(struct bzzd_guy *guy)
{
	int w, n, s;
	w = bzzd_get_park_west(guy->park);
	n = bzzd_get_park_north(guy->park);
	s = bzzd_get_park_south(guy->park);
	guy->x = xor128_next_range(guy->park->rng, w, guy->park->width / 2);
	guy->y = xor128_next_range(guy->park->rng, n, s);
}

void bzzd_wakeup_random_east(struct bzzd_guy *guy)
{
	int n, e, s;
	n = bzzd_get_park_north(guy->park);
	e = bzzd_get_park_east(guy->park);
	s = bzzd_get_park_south(guy->park);
	guy->x = xor128_next_range(guy->park->rng, guy->park->width / 2, e);
	guy->y = xor128_next_range(guy->park->rng, n, s);
}

void bzzd_wakeup_random_north(struct bzzd_guy *guy)
{
	int w, n, e;
	w = bzzd_get_park_west(guy->park);
	n = bzzd_get_park_north(guy->park);
	e = bzzd_get_park_east(guy->park);
	guy->x = xor128_next_range(guy->park->rng, w, e);
	guy->y = xor128_next_range(guy->park->rng, n, guy->park->height / 2);
}

void bzzd_wakeup_random_south(struct bzzd_guy *guy)
{
	int w, e, s;
	w = bzzd_get_park_west(guy->park);
	e = bzzd_get_park_east(guy->park);
	s = bzzd_get_park_south(guy->park);
	guy->x = xor128_next_range(guy->park->rng, w, e);
	guy->y = xor128_next_range(guy->park->rng, guy->park->height / 2, s);
}

void bzzd_wakeup_random_west_edge(struct bzzd_guy *guy)
{
	int w, n, s;
	w = bzzd_get_park_west(guy->park);
	n = bzzd_get_park_north(guy->park);
	s = bzzd_get_park_south(guy->park);
	guy->x = w;
	guy->y = xor128_next_range(guy->park->rng, n, s);
}

void bzzd_wakeup_random_east_edge(struct bzzd_guy *guy)
{
	int n, e, s;
	n = bzzd_get_park_north(guy->park);
	e = bzzd_get_park_east(guy->park);
	s = bzzd_get_park_south(guy->park);
	guy->x = e;
	guy->y = xor128_next_range(guy->park->rng, n, s);
}

void bzzd_wakeup_random_north_edge(struct bzzd_guy *guy)
{
	int w, n, e;
	w = bzzd_get_park_west(guy->park);
	n = bzzd_get_park_north(guy->park);
	e = bzzd_get_park_east(guy->park);
	guy->x = xor128_next_range(guy->park->rng, w, e);
	guy->y = n;
}

void bzzd_wakeup_random_south_edge(struct bzzd_guy *guy)
{
	int w, e, s;
	w = bzzd_get_park_west(guy->park);
	e = bzzd_get_park_east(guy->park);
	s = bzzd_get_park_south(guy->park);
	guy->x = xor128_next_range(guy->park->rng, w, e);
	guy->y = s;
}

void bzzd_wakeup_random_westeast_edge(struct bzzd_guy *guy)
{
	if (xor128_next_unit(guy->park->rng) < 0.5) {
		bzzd_wakeup_random_west_edge(guy);
	} else {
		bzzd_wakeup_random_east_edge(guy);
	}
}

void bzzd_wakeup_random_northsouth_edge(struct bzzd_guy *guy)
{
	if (xor128_next_unit(guy->park->rng) < 0.5) {
		bzzd_wakeup_random_north_edge(guy);
	} else {
		bzzd_wakeup_random_south_edge(guy);
	}
}

void bzzd_wakeup_random_edge(struct bzzd_guy *guy)
{
	if (xor128_next_unit(guy->park->rng) < 0.5) {
		bzzd_wakeup_random_westeast_edge(guy);
	} else {
		bzzd_wakeup_random_northsouth_edge(guy);
	}
}

void bzzd_wakeup_random_marked(struct bzzd_guy *guy)
{
	struct point p = ps_rnd(guy->park->markedset, guy->park->rng);
	guy->x = p.x;
	guy->y = p.y;
}

void bzzd_wakeup_random_fresh(struct bzzd_guy *guy)
{
	struct point p = ps_rnd(guy->park->freshset, guy->park->rng);
	guy->x = p.x;
	guy->y = p.y;
}

/*
target functions.
*/

void bzzd_target_fixed(struct bzzd_guy *guy, int x, int y)
{
	guy->target_x = x;
	guy->target_y = y;
}

void bzzd_target_random(struct bzzd_guy *guy)
{
	int w, n, e, s;
	w = bzzd_get_park_west(guy->park);
	n = bzzd_get_park_north(guy->park);
	e = bzzd_get_park_east(guy->park);
	s = bzzd_get_park_south(guy->park);
	guy->target_x = xor128_next_range(guy->park->rng, w, e);
	guy->target_y = xor128_next_range(guy->park->rng, n, s);
}

void bzzd_target_random_west(struct bzzd_guy *guy)
{
	int w, n, s;
	w = bzzd_get_park_west(guy->park);
	n = bzzd_get_park_north(guy->park);
	s = bzzd_get_park_south(guy->park);
	guy->target_x = xor128_next_range(guy->park->rng, w, guy->park->width / 2);
	guy->target_y = xor128_next_range(guy->park->rng, n, s);
}

void bzzd_target_random_east(struct bzzd_guy *guy)
{
	int n, e, s;
	n = bzzd_get_park_north(guy->park);
	e = bzzd_get_park_east(guy->park);
	s = bzzd_get_park_south(guy->park);
	guy->target_x = xor128_next_range(guy->park->rng, guy->park->width / 2, e);
	guy->target_y = xor128_next_range(guy->park->rng, n, s);
}

void bzzd_target_random_north(struct bzzd_guy *guy)
{
	int w, n, e;
	w = bzzd_get_park_west(guy->park);
	n = bzzd_get_park_north(guy->park);
	e = bzzd_get_park_east(guy->park);
	guy->target_x = xor128_next_range(guy->park->rng, w, e);
	guy->target_y = xor128_next_range(guy->park->rng, n, guy->park->height / 2);
}

void bzzd_target_random_south(struct bzzd_guy *guy)
{
	int w, e, s;
	w = bzzd_get_park_west(guy->park);
	e = bzzd_get_park_east(guy->park);
	s = bzzd_get_park_south(guy->park);
	guy->target_x = xor128_next_range(guy->park->rng, w, e);
	guy->target_y = xor128_next_range(guy->park->rng, guy->park->height / 2, s);
}

void bzzd_target_random_west_edge(struct bzzd_guy *guy)
{
	int w, n, s;
	w = bzzd_get_park_west(guy->park);
	n = bzzd_get_park_north(guy->park);
	s = bzzd_get_park_south(guy->park);
	guy->target_x = w;
	guy->target_y = xor128_next_range(guy->park->rng, n, s);
}

void bzzd_target_random_east_edge(struct bzzd_guy *guy)
{
	int n, e, s;
	n = bzzd_get_park_north(guy->park);
	e = bzzd_get_park_east(guy->park);
	s = bzzd_get_park_south(guy->park);
	guy->target_x = e;
	guy->target_y = xor128_next_range(guy->park->rng, n, s);
}

void bzzd_target_random_north_edge(struct bzzd_guy *guy)
{
	int w, n, e;
	w = bzzd_get_park_west(guy->park);
	n = bzzd_get_park_north(guy->park);
	e = bzzd_get_park_east(guy->park);
	guy->target_x = xor128_next_range(guy->park->rng, w, e);
	guy->target_y = n;
}

void bzzd_target_random_south_edge(struct bzzd_guy *guy)
{
	int w, e, s;
	w = bzzd_get_park_west(guy->park);
	e = bzzd_get_park_east(guy->park);
	s = bzzd_get_park_south(guy->park);
	guy->target_x = xor128_next_range(guy->park->rng, w, e);
	guy->target_y = s;
}

void bzzd_target_random_westeast_edge(struct bzzd_guy *guy)
{
	if (xor128_next_unit(guy->park->rng) < 0.5) {
		bzzd_target_random_west_edge(guy);
	} else {
		bzzd_target_random_east_edge(guy);
	}
}

void bzzd_target_random_northsouth_edge(struct bzzd_guy *guy)
{
	if (xor128_next_unit(guy->park->rng) < 0.5) {
		bzzd_target_random_north_edge(guy);
	} else {
		bzzd_target_random_south_edge(guy);
	}
}

void bzzd_target_random_edge(struct bzzd_guy *guy)
{
	if (xor128_next_unit(guy->park->rng) < 0.5) {
		bzzd_target_random_westeast_edge(guy);
	} else {
		bzzd_target_random_northsouth_edge(guy);
	}
}

void bzzd_target_random_marked(struct bzzd_guy *guy)
{
	struct point p = ps_rnd(guy->park->markedset, guy->park->rng);
	guy->target_x = p.x;
	guy->target_y = p.y;
}

void bzzd_target_random_fresh(struct bzzd_guy *guy)
{
	struct point p = ps_rnd(guy->park->freshset, guy->park->rng);
	guy->target_x = p.x;
	guy->target_y = p.y;
}

/*
peeing functions.
*/

void bzzd_pee_everywhere(struct bzzd_guy *guy, int pee)
{
	int x, y;
	int w, n, e, s;
	w = bzzd_get_park_west(guy->park);
	n = bzzd_get_park_north(guy->park);
	e = bzzd_get_park_east(guy->park);
	s = bzzd_get_park_south(guy->park);
	for (y = n; y <= s; ++y) {
		for (x = w; x <= e; ++x) {
			bzzd_pee(guy->park, x, y, pee);
		}
	}
}

void bzzd_pee_1(struct bzzd_guy *guy, int pee)
{
	bzzd_pee(guy->park, guy->x, guy->y, pee);
}

void bzzd_pee_plus(struct bzzd_guy *guy, int pee)
{
	bzzd_pee(guy->park, guy->x, guy->y, pee);
	bzzd_pee(guy->park, guy->x - 1, guy->y, pee);
	bzzd_pee(guy->park, guy->x, guy->y - 1, pee);
	bzzd_pee(guy->park, guy->x, guy->y + 1, pee);
	bzzd_pee(guy->park, guy->x + 1, guy->y, pee);
}

void bzzd_pee_x(struct bzzd_guy *guy, int pee)
{
	bzzd_pee(guy->park, guy->x, guy->y, pee);
	bzzd_pee(guy->park, guy->x - 1, guy->y - 1, pee);
	bzzd_pee(guy->park, guy->x - 1, guy->y + 1, pee);
	bzzd_pee(guy->park, guy->x + 1, guy->y - 1, pee);
	bzzd_pee(guy->park, guy->x + 1, guy->y + 1, pee);
}

void bzzd_pee_rect(struct bzzd_guy *guy, int hw, int hh, int pee)
{
	int x, y;
	for (x = ceil(guy->x - hw); x <= floor(guy->x + hw); ++x) {
		for (y = ceil(guy->y - hh); y <= floor(guy->y + hh); ++y) {
			bzzd_pee(guy->park, x, y, pee);
		}
	}
}

void bzzd_pee_circle(struct bzzd_guy *guy, int r, int pee)
{
	int dx, dy, h, x, y;

	for (dx = -r; dx <= r; ++dx)
	{
		h = floor(sqrt(r * r - dx * dx));
		for (dy = -h; dy <= h; ++dy)
		{
			x = guy->x + dx;
			y = guy->y + dy;
			bzzd_pee(guy->park, x, y, pee);
		}
	}
}

/*
stagger functions.
*/

void bzzd_stagger_by(struct bzzd_guy *guy, int dx, int dy)
{
	int in_bounds, target_in_bounds;
	in_bounds = bzzd_is_inside_park(guy->park, guy->x, guy->y);
	target_in_bounds = bzzd_is_inside_park(guy->park, guy->x + dx, guy->y + dy);
	if (target_in_bounds || (!in_bounds && !target_in_bounds)) {
		guy->x += dx;
		guy->y += dy;
	}
}

void bzzd_stagger_random(struct bzzd_guy *guy)
{
	int dirs[4][2] = {
		{-1,  0},
		{ 0, -1},
		{ 0,  1},
		{ 1,  0},
	};

	int *dir = dirs[xor128_next_index(guy->park->rng, 4)];
	int dx = dir[0];
	int dy = dir[1];

	bzzd_stagger_by(guy, dx, dy);
}

/* Okay, so here's the weighted walk algorith. Very sloppy, but I foresee it
 * changing a lot so I'm not too worried. The algorithm is hard coded and
 * not optimized... for now.
 *
 * The two cases you have in a four directional weighted walk is where the
 * walker is ON an axis that the target is on, and when the walker is not...
 * Each case is handled differently.
 *
 * === On a same axis ===
 *
 * With this weighted walk I desire a spread of 25% for each direction when
 * weight = 50%. That's right in the middle. In the same axis case there is only
 * one direction that heads directly to the target, so in order to give it the
 * proper weight, it's chance of occuring is weight / 2, so at weight = 50%, the
 * exact chance to go directly towards the target is 25%. What do we do with
 * the remainding percent and the other three directions? Well the remaining
 * weight is divided evenly, so each of the three directions gets a 3rd of it
 * or a (6th of the total since the direct direction took half already). They
 * also get the left over anti-weight (1 - weight), which is divided evenly so
 * they get a 3rd of it.
 *
 * (X is target, @ is location, W is weight):
 *                  (W / 6 + (1 - W) / 3)
 *                          |
 *                          |
 * (W / 6 + (1 - W) / 3) ---@--- X (W / 2)
 *                          |
 *                          |
 *                  (W / 6 + (1 - W) / 3)
 *
 * For example, when W = 0.75:
 *         ~20.83%
 *            |
 *            |
 * ~20.83% ---@--- X 37.5%
 *            |
 *            |
 *         ~20.83%
 *
 * When W = 1.0:
 *         ~16.66%
 *            |
 *            |
 * ~16.66% ---@--- X 50%
 *            |
 *            |
 *         ~16.66%
 *
 * === Not on a same axis ===
 *
 * The same axis is easier, the formula is:
 *
 * (X is target, @ is location, W is weight):
 *             (W / 2)
 *                |
 *                |  X
 * (1 - W) / 2 ---@--- (W / 2)
 *                |
 *                |
 *           (1 - W) / 2
 *
 * For example, when W = 0.75:
 *        37.5%
 *          |
 *          |  X
 * 12.5% ---@--- 37.5%
 *          |
 *          |
 *        12.5%
 */

void bzzd_stagger_to_target(struct bzzd_guy *guy, double weight)
{
	int dx = guy->target_x - guy->x;
	int dy = guy->target_y - guy->y;
	double r;
	int *dpz, *dpnz;

	if (dx >  1) dx = 1;
	if (dx < -1) dx = -1;
	if (dy >  1) dy = 1;
	if (dy < -1) dy = -1;

	r = xor128_next_unit(guy->park->rng);

	if (dx == 0 && dy == 0) {
		/* On target, do nothing. */
	} else if (dx == 0 || dy == 0) {
		if (dx == 0) {
			dpz = &dx;
			dpnz = &dy;
		} else {
			dpz = &dy;
			dpnz = &dx;
		}

		if (r >= (weight * 0.5)) {
			r -= weight * 0.5;
			if (r < weight * (1.0 / 6) + (1 - weight) * (1.0 / 3)) {
				*dpz = -1;
				*dpnz = 0;
			} else if (r < weight * (2.0 / 6) + (1 - weight) * (2.0 / 3)) {
				*dpz = 1;
				*dpnz = 0;
			} else {
				*dpz = 0;
				*dpnz *= -1;
			}
		}
	} else {
		if (r < weight * 0.5) {
			dy = 0;
		} else if (r < weight) {
			dx = 0;
		} else if (r < weight + (1 - weight) * 0.5) {
			dx *= -1;
			dy = 0;
		} else {
			dx = 0;
			dy *= -1;
		}
	}

	bzzd_stagger_by(guy, dx, dy);
}

struct line_path_struct {
	int dx;
	int sx;
	int dy;
	int sy;
	int err;
	int e2;
};

static int line_path(struct bzzd_guy *guy)
{
	struct line_path_struct *data = (void *)guy->path_data;

	if (guy->x == guy->target_x && guy->y == guy->target_y)
		return 0;

	data->e2 = data->err;

	if (data->e2 > -data->dx) {
		data->err -= data->dy;
		guy->x += data->sx;
	}
	if (data->e2 < data->dy) {
		data->err += data->dx;
		guy->y += data->sy;
	}

	return 1;
}

void bzzd_line_path_to_target(struct bzzd_guy *guy)
{
	struct line_path_struct *data = (void *)guy->path_data;

	data->dx = abs(guy->target_x - guy->x);
	data->sx = guy->x < guy->target_x ? 1 : -1;

	data->dy = abs(guy->target_y - guy->y);
	data->sy = guy->y < guy->target_y ? 1 : -1;

	data->err = (data->dx > data->dy ? data->dx : -data->dy) / 2;

	guy->pathing_function = line_path;
}

enum {NONE, HORIZONTAL, VERTICAL};

struct tunnel_path_struct {
	int first;
	int second;
};

static int tunnel_path(struct bzzd_guy *guy)
{
	struct tunnel_path_struct *data = (void *)guy->path_data;

	if (!data->first)
		return 0;

	if (data->first == HORIZONTAL) {
		guy->x += (guy->target_x - guy->x) < 0 ? -1 : 1;

		if (guy->x == guy->target_x) {
			data->first = data->second;
			data->second = NONE;
		}
	} else if (data->first == VERTICAL) {
		guy->y += (guy->target_y - guy->y) < 0 ? -1 : 1;

		if (guy->y == (guy->target_y - guy->y)) {
			data->first = data->second;
			data->second = NONE;
		}
	}

	return 1;
}

void bzzd_tunnel_path_to_target(struct bzzd_guy *guy)
{
	struct tunnel_path_struct *data = (void *)guy->path_data;

	int dx = guy->target_x - guy->x;
	int dy = guy->target_y - guy->y;

	if (dx != 0 && dy != 0) {
		if (xor128_next_unit(guy->park->rng) < 0.5) {
			data->first = HORIZONTAL;
			data->second = VERTICAL;
		} else {
			data->first = VERTICAL;
			data->second = HORIZONTAL;
		}
	} else if (dx != 0) {
		data->first = HORIZONTAL;
		data->second = NONE;
	} else if (dy != 0) {
		data->first = VERTICAL;
		data->second = NONE;
	}

	guy->pathing_function = tunnel_path;
}

int bzzd_walk_path(struct bzzd_guy *guy)
{
	if (guy->pathing_function) {
		if (!guy->pathing_function(guy)) {
			bzzd_cancel_path(guy);
		}

		return 1;
	} else {
		return 0;
	}
}

void bzzd_cancel_path(struct bzzd_guy *guy)
{
	guy->pathing_function = NULL;
}

/*

*/

int bzzd_is_on_marked(struct bzzd_guy *guy)
{
	return bzzd_is_marked_spot(guy->park, guy->x, guy->y);
}

int bzzd_is_on_fresh(struct bzzd_guy *guy)
{
	return bzzd_is_fresh_spot(guy->park, guy->x, guy->y);
}

int bzzd_is_on_target(struct bzzd_guy *guy)
{
	return guy->x == guy->target_x && guy->y == guy->target_y;
}

int bzzd_is_on_fixed(struct bzzd_guy *guy, int x, int y)
{
	return guy->x == x && guy->y == y;
}

int bzzd_is_on_fixed_x(struct bzzd_guy *guy, int x)
{
	return guy->x == x;
}

int bzzd_is_on_fixed_y(struct bzzd_guy *guy, int y)
{
	return guy->y == y;
}

