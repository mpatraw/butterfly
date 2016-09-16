
#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "internal.h"

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


int ps_init(struct pointset *ps, int width, int height)
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

void ps_uninit(struct pointset *ps)
{
	free(ps->arr);
	free(ps->map);
}

int ps_has(struct pointset *ps, struct point p)
{
	if (p.x < 0 || p.y < 0 || p.x >= ps->width || p.y >= ps->height) {
		return 0;
	}
	if (INDEX2(ps->map, p.x, p.y, ps->width)) {
		return 1;
	}
	return 0;
}

void ps_add(struct pointset *ps, struct point p)
{
	int had = ps_has(ps, p);

	INDEX2(ps->map, p.x, p.y, ps->width) = 1;
	/* we just added 1 to the length. */
	ps->arr[ps->length] = p;

	if (!had) {
		ps->length++;
	}
}

void ps_rem(struct pointset *ps, struct point p)
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

struct point ps_rnd(struct pointset *ps, struct random_state *x)
{
	assert(ps->length && "tried to random 0-length pointset");
	return ps->arr[random_next_index(x, ps->length - 1)];
}

void ps_clr(struct pointset *ps)
{
	ps->length = 0;
	memset(ps->map, 0, sizeof *ps->map * ps->width * ps->height);
}
