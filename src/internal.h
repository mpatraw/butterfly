#ifndef BZZD_INTERNAL_H
#define BZZD_INTERNAL_H

#include <stdbool.h>
#include <limits.h>

#include "butterfly.h"

/*
 * very simple way to get size specific types, should work on most platforms.
 */
#if UCHAR_MAX == 0xff
typedef unsigned char u8;
#else
#error "system does not support 8-bit integers"
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

/*
 * random
 */

struct random_state {
	/* xor128, fast speed, low memory, period = 2^128 - 1. */
	u32 q[4];
};

void random_seed(struct random_state *x, u32 seed);
u32 random_next(struct random_state *x);
/* [0.0, 1.0) */
double random_next_unit(struct random_state *x);
/* [0, sz) */
u32 random_next_index(struct random_state *x, u32 sz);
/* [min, max] */
u32 random_next_range(struct random_state *x, u32 min, u32 max);

/*
 * pointset
 */

struct point {
	int x;
	int y;
};

struct point make_point(int x, int y);

struct pointset {
	struct point *arr;
	int length;

	int *map;
	int width, height;
};

int ps_init(struct pointset *ps, int width, int height);
void ps_uninit(struct pointset *ps);
int ps_has(struct pointset *ps, struct point p);
void ps_add(struct pointset *ps, struct point p);
void ps_rem(struct pointset *ps, struct point p);
struct point ps_rnd(struct pointset *ps, struct random_state *x);
void ps_clr(struct pointset *ps);

/*
 * butterfly
 */

#define IN_BOUNDS(f, x, y) \
	((x) >= 0 && (y) >= 0 && x < (f)->width && y < (f)->height)
#define SPOT_AT(arr, w, x, y) ((arr)[(y) * (w) + (x)])
#define IS_SAFE(f, t) ((t) > (f)->last_dangerous)
#define IS_SAFE_AT(f, x, y) IS_SAFE(f, SPOT_AT((f)->spots, (f)->width, x, y))

struct butterfly {
	struct bf_config *config;
	void *path_data;
	int *new_spots;
	int x;
	int y;
	int goal_x;
	int goal_y;
	int death_timer;
	int last_morph_x;
	int last_morph_y;
	int last_death_x;
	int last_death_y;
	struct bf_instinct *flutter;
};

void morph(
	struct butterfly *bf,
	struct bf_farm *farm,
	struct bf_instinct *instinct);
void goal(
	struct butterfly *bf,
	struct bf_farm *farm,
	struct bf_instinct *instinct);
void look(
	struct butterfly *bf,
	struct bf_farm *farm,
	struct bf_instinct *instinct);
void flutter(
	struct butterfly *bf,
	struct bf_farm *farm,
	struct bf_instinct *instinct);
bool die(
	struct butterfly *bf,
	struct bf_farm *farm,
	struct bf_instinct *instinct);

#endif
