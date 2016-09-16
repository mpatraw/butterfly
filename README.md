![butterfly](https://github.com/mpatraw/butterfly/raw/master/doc/butterfly.png "butterfly")

# Butterfly

**Butterfly** is designed to be a simple and general implementation of the [Drunkard's Walk](https://en.wikipedia.org/wiki/Random_walk) algorithm for generating 2D maps.

# Building

**Butterfly** uses [tup] for it's build system and running `tup upd` should give you everything you need and build all of the examples.

# Algorithm

## Steps

**Butterfly** breaks down the Drunkard's Walk algorithm into several steps. Each step can be controlled to produce varying dungeons.

The steps are:

* **Morph** (Optional) - This is where the butterfly starts it's "walk." For example, a butterfly might choose only safe (walkable) spaces in which to start.
* **Goal** (Optional) - This just sets an optional goal location for the butterfly to aim towards. Other steps can use this information.
* **Flutter** (Optional) - This is the "walk" algorithm. It can be a weighted walk or predefined path like a straight line. Only *one* flutter instinct per instinct list.
* **Look** (Optional) - This is the way the butterfly "digs" out the dungeon. You can choose different shapes and patterns for the butterfly to dig out.
* **Die** (Required) - This is how the butterfly stops it's digging. For example, the butterfly might have a 1 in 20 chance to die each time it looks (digs).

Actions for each of these steps is called an "instinct," and may require arguments. The first thing you do is build a simple array of instincts and pass that to the library. When a butterfly spawns, it runs through this logic to execute each instinct.

* Pick random morph instinct.
* Pick random goal instinct.
* Repeat
 * Check if any of the death instincts are true and break if so.
 * Use flutter instinct (you can only have one for each butterfly).
 * Pick random look instinct.

## Safe & Dangerous (Walkable & Obstructed)

In **Butterfly**, and any map generation algorithm, there has to be a way to determine if a tile is walkable or not. This is so we can guarantee every part of the map is connected to each other. **Butterfly** does this by just assuming any number 1 or over is walkable, and any number 0 or below is not. Obviously this doesn't work if you assume each tile is an array index, but you can always solve that by adding and subtracting by the index when your walkable tiles start. Another option is to have a translation step.

For the examples provided, you will see this enum:

```
enum {
	WALL,
	FLOOR
};
```

This works because `WALL` is 0, so **Butterfly** treats this as unwalkable, and `FLOOR` is 1, which is walkable. Think positivity is walkibility!

## Ensuring Everything Connects

Some map generators have a second step where they remove any orphaned areas that's smaller than the largest. **Butterfly** does not provide any of that post-processing functionality. Instead, you have make sure the logic of your instincts (instructions) are sound.

 My recommended method is to always start or end on a safe spot.

# The API!

The API is aimed to be super simple, in fact, currently it's only 3 functions. One for cleanup.

```c
int bf_spawn(struct bf_farm *farm, struct bf_instinct *instincts, size_t count, struct bf_config *config);
void bf_commit(struct bf_farm *farm);
void bf_cleanup(struct bf_farm *farm);
```

# Simple Example (With Comments)

This example builds a cave. You can see the full source here: [ex1.c](https://github.com/mpatraw/butterfly/blob/master/examples/ex1.c) and play with the values.

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
	{.event = BF_MORPH, .action = BF_MORPH_AT_RANDOM_SPOT},
	{.event = BF_LOOK, .action = BF_LOOK_1_AREA, .args = {FLOOR}},
	{.event = BF_DIE, .action = BF_DIE_AFTER_N, {1}}
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
	{.event = BF_MORPH, .action = BF_MORPH_AT_RANDOM_SPOT},
	{.event = BF_GOAL, .action = BF_GOAL_RANDOM_SAFE_SPOT},
	{.event = BF_FLUTTER, .action = BF_FLUTTER_RANDOMLY_TO_GOAL, {60}},
	{.event = BF_LOOK, .action = BF_LOOK_PLUS_AREA, .args = {FLOOR}},
	{.event = BF_DIE, .action = BF_DIE_AT_SAFE_SPOT},
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
