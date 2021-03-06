![butterfly](https://github.com/mpatraw/butterfly/raw/master/doc/butterfly.png "butterfly")

# Butterfly

**Butterfly** is designed to be a simple and general implementation of the [Drunkard's Walk](https://en.wikipedia.org/wiki/Random_walk) algorithm for generating 2D maps. The name was chosen because butterflies move in random patterns while still moving towards a goal location.

Here is a screenshot of a simple dungeon with caves. The full source code (using libtcod) is here: [ss.c](https://github.com/mpatraw/butterfly/blob/master/examples/tcod/ss.c), and the distilled version (just dungeon generation code) is here: [ss.py](https://github.com/mpatraw/butterfly/blob/master/examples/python/ss.py).

![screenshot](https://github.com/mpatraw/butterfly/raw/master/doc/ss.png "screenshot")

# Building

**Butterfly** uses [tup](http://gittup.org/tup/) for its build system and running `tup upd` should give you everything you need, including the examples.

By default the TCOD and BearLibTerminal examples will not build. If you have either of these on your system you can change the `tup.config` file to have `CONFIG_BUILD_TCOD_EXAMPLES` or `CONFIG_BUILD_BEARLIBTERMINAL_EXAMPLES` set to `TRUE`. Running `tup upd` will then build those examples.

# Algorithm

## Steps

**Butterfly** breaks down the Drunkard's Walk algorithm into several steps. Each step can be controlled to produce varying dungeons.

The steps are:

* **Morph** (Optional) - This is where the butterfly starts its "walk." For example, a butterfly might choose only safe (walkable) spaces in which to start. When not provided, the default is the last death location of the butterfly or 0,0.
* **Goal** (Optional) - This just sets an optional goal location for the butterfly to aim towards. Other steps can use this information. When not provided instincts that use the goal will not work.
* **Flutter** (Optional) - This is the "walk" algorithm. It can be a weighted walk or predefined path like a straight line. When not provided, the butterfly doesn't move.
* **Look** (Optional) - This is the way the butterfly "digs" out the dungeon. You can choose different shapes and patterns for the butterfly to dig out. When not provided, the butterfly doesn't look anywhere.
* **Die** (Required) - This is how the butterfly stops its digging. For example, the butterfly might have a 1 in 20 chance to die each time it looks (digs).

Controlling these steps is done with an array of "instincts," which you pass to a function. The ordering of the array doesn't matter, and you can have multiple entries for each step. In some cases all of them will be used (Die), others, each time you call the function it will pick a random entry for each step.

When a butterfly spawns, it runs through this logic to execute each instinct:

* Pick random morph instinct
* Pick random goal instinct
* Pick random flutter instinct
* Repeat
 * Check if any of the death instincts are true and break if so
 * Use the picked flutter instinct
 * Pick random look instinct

## Safe & Dangerous (Walkable & Obstructed)

In **Butterfly**, and any map generation algorithm, there has to be a way to determine if a tile is walkable or not. This is so we can guarantee every part of the map is connected to each other. **Butterfly** does this by just assuming any number 1 or over is walkable, and any number 0 or below is not by default. This can be changed with the `last_dangerous` member of the farm struct.

For the examples provided, you will see this enum:

```
enum {
	WALL,
	FLOOR
};
```

This works because `WALL` is 0, so **Butterfly** treats this as unwalkable, and `FLOOR` is 1, which is walkable. Think positivity is walkibility!

## Ensuring Everything Connects

Some map generators have a second step where they remove any orphaned areas that are smaller than the largest. **Butterfly** does not provide any of that post-processing functionality. Instead, you have make sure the logic of your instincts (instructions) are sound.

 My recommendation is to always start or end on a safe spot.

# The API!

The API is aimed to be super simple, in fact, currently it's only 5 functions. One for cleanup.

```c
/* Spawns a butterfly with a list of instincts. The changes won't be seen
 * on the spots or by other butterflies until bf_commit is called.
 *
 * If an error is returned, nothing will change after commit; the butterfly
 * resets completely.
 *
 * Returns 0 on success or another value on error.
 */
int bf_spawn(struct bf_farm *farm, struct bf_instinct *instincts, size_t count, struct bf_config *config);
/* Commit any changes the butterfly (or butterflies have done). This is so
 * new butterflies can see the new safe/dangerous spots.
 */
void bf_commit(struct bf_farm *farm);
/* Uses the internal RNG. Return a number [0,1)
 */
double bf_random(struct bf_farm *farm);
/* Queries for information like safe percentage.
 */
void bf_query(struct bf_farm *farm, int query, int *out);
/* If you spawn any butterflies, this should be called to clean up any
 * memory. You can still spawn butterflies after this, but then you'd have
 * to clean up again. This can be useful for clearing and regenerating a
 * map.
 */
void bf_cleanup(struct bf_farm *farm);
```

# Simple Example (With Comments)

This example builds a cave. You can see the full source here: [ex1.c](https://github.com/mpatraw/butterfly/blob/master/examples/plain/ex1.c). Clone the repository and play with the values.

```c
/* This can be created any way you'd like. This is an 80x24 map filled
 * with walls (0).
 */
int spots[24][80] = {{WALL}};
/* The only three members that are required when using Butterfly. */
struct bf_farm farm = {
	.spots = (void *)spots,
	.width = 80,
	.height = 24
};
/* This is to "seed" the map with a safe spot to travel. This ensures
 * everything is connected if we always connect to a safe spot to
 * flutter.
 *
 * Building the instincts (instructions).
 * - First is the morph instinct. Here we start anywhere on the map.
 * - Look is how we dig. Here it says at each step dig a 1 FLOOR tile.
 * - And finally how to die, which is right after fluttering and
 *   looking just once.
 */
struct bf_instinct carve_start[] = {
	{.action = BF_MORPH_AT_RANDOM_SPOT},
	{.action = BF_LOOK_1_AREA, .args = {FLOOR}},
	{.action = BF_DIE_AFTER_N, {1}}
};
/* The actual carving instincts. We build by always finding our way
 * back to a safe spot.
 *
 * Building the instincts (instructions).
 * - First is the morph instinct which is to start anywhere on the map.
 * - Next is the goal, which is any safe spot on the map that was
 *   commited. ENSURES CONNECTIONS.
 * - Flutter randomly with a 60% weighted walk to the goal.
 * - Look in a plus symbol. I find plus symbols dig really nice caves.
 * - And finally how to die, by arriving at any safe spot.
 */
struct bf_instinct carve[] = {
	{.action = BF_MORPH_AT_RANDOM_SPOT},
	{.action = BF_GOAL_RANDOM_SAFE_SPOT},
	{.action = BF_FLUTTER_RANDOMLY_TO_GOAL, {60}},
	{.action = BF_LOOK_PLUS_AREA, .args = {FLOOR}},
	{.action = BF_DIE_AT_SAFE_SPOT},
};
/* Run the instincts to produce our first safe area so the other butterflies
 * can find it! This only needs to be run once.
 */
BF_SPAWN_ARR(&farm, carve_start, NULL);
/* Commit these changes. */
bf_commit(&farm);

/* Now carve out the rest. Commiting after each loop so the new spawns
 * can find the new safe spots.
 */
for (int i = 0; i < 20; ++i) {
	BF_SPAWN_ARR(&farm, carve, NULL);
	bf_commit(&farm);
}
```

Which will generate something like:

```
################################################################################
################################################################################
################################################################################
################################################################################
################################################################################
################################################################################
################################################################################
################################################################################
################################################################################
############################################.###################################
############################################.###################################
###########################################.####################################
###########################################.####################################
##########################################.#####################################
##########################################.#####################################
##########################################.#####################################
#########################################.######################################
#########################################.######################################
########################################.#######################################
########################################.#######################################
################################################################################
################################################################################
################################################################################
################################################################################
```

Just kidding. Here is what it can actually produce.

```
##.....#############################.......####################.################
#......#########.###################.......###################...###############
#.....#########...##################........##...#######.#..##....##############
#......#######....###################.......#.....#####............#############
##.#....##.........###################..#.........#####...........##############
####...............#######################..........##...........###############
####..............#######################.#..................#....##############
#####..............#####################.................#.###....##############
######.......#......###########.#######............................#############
######.......##.....###..#####...#####.........#....................############
#######.....###.............##....###.......####.....................###########
################..................###.........##....................############
################..................###..........#......#..........######...######
#################.................####.......................#..######.....#####
#################.....#...........#####.##...........#.......#...#.#........####
################.....##...........########...........##......................###
#################...####....#.......#####.............##......................##
##################.######....##.........#.............###...............#......#
#########################....####.................#..#######.......##..##.......
#########################...#######..........################......######.......
########################....#######.........###################...########.###.#
##################.#####...########...#....#####################################
##############..#....##.....########.###########################################
#############...............####################################################
```

# An Even Simpler Example (In Python)

Okay, so the logic isn't as simple, but it's Python so...

```py
import butterfly as bf

WALL = 0
FLOOR = 1

room = bf.Butterfly(*[
    [bf.MORPH_AT_RANDOM_SPOT],
    [bf.LOOK_RECT_AREA, [FLOOR, 3, 3]],
    [bf.DIE_AFTER_N, [1]]
], cancel_on_looking_at_safe_neighbor_8=True,
   cancel_on_looking_outside_farm=True)

tunnel = bf.Butterfly(*[
    [bf.MORPH_AT_LAST_DEATH_SPOT],
    [bf.GOAL_RANDOM_SAFE_SPOT],
    [bf.FLUTTER_TUNNEL],
    [bf.LOOK_1_AREA, [FLOOR]],
    [bf.DIE_AT_SAFE_SPOT]
])

f = bf.Farm(80, 24)
f.spawn(bf.random_nxm(FLOOR, 3, 3), commit=True)
f.spawn([room, tunnel], 50, commit=True)
```

This can produce something like:

```
############......................................##############################
############...........##########################.###########...........########
############...........##########################.###########...........########
############...........##########################.###########...........########
#############.##############################...........######...........########
#############.##############################...........######...........########
#############.##############################...........###########.#############
#######.......################...........###...........###########.#############
#######.#####.################...........###.......................#############
#######.#####............................#########################.#############
#######.#################.####...........#########################.#############
#######.#################.####...........#########################.#############
#######.#################.########################################.#############
#######.#################.##################################.............#######
#######.#################.##################################.............#######
#######.###########.............############################.............#######
#######.###########.............############################.............#######
#.............#####.............########.............#######.............#######
#.............#####.............########.............#######.............#######
#.............#####.............########.............#######.............#######
#.............#####.............########...........................#############
#.............#####.............########.............###########################
#.............##########################.............###########################
#.............##########################.............###########################
```
