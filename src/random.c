
#include <math.h>
#include <stdlib.h>

#include "internal.h"

void random_seed(struct random_state *x, u32 seed)
{
	int i;

	srand(seed);
	/* magic number 4, see struct declaration */
	for (i = 0; i < 4; ++i) {
		x->q[i] = rand();
	}
}

u32 random_next(struct random_state *x)
{
	u32 t;
	t = (x->q[0] ^ (x->q[0] << 11));
	x->q[0] = x->q[1];
	x->q[1] = x->q[2];
	x->q[2] = x->q[3];
	return x->q[3] = x->q[3] ^ (x->q[3] >> 19) ^ (t ^ (t >> 8));
}

double random_next_unit(struct random_state *x)
{
	return random_next(x) * 2.3283064365386963e-10;
}

u32 random_next_index(struct random_state *x, u32 sz)
{
	return floor(random_next_unit(x) * sz);
}

u32 random_next_range(struct random_state *x, u32 min, u32 max)
{
	return random_next_index(x, max - min + 1) + min;
}
