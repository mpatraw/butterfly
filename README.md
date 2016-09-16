![butterfly](https://github.com/mpatraw/butterfly/raw/master/doc/butterfly.png "butterfly")

# Butterfly

Butterfly is designed to be a simple general implementation of the Drunkard's Walk algorithm for generation 2D maps.

# Algorithm

Butterfly breaks down the Drunkard's Walk algorithm into several steps. Each step can be controlled to produce varying dungeons.

The steps are:

* Morph (Optional) - This is where the butterfly starts it's "walk." For example, a butterfly might choose only safe (walkable) spaces in which to start.
* Goal (Optional) - This just sets an optional goal location for the butterfly to aim towards. Other steps can use this information.
* Flutter (Optional) - This is the "walk" algorithm. It can be a weighted walk or predefined path like a straight line.
* Look (Optional) - This is the way the butterfly "digs" out the dungeon. You can choose different shapes and patterns for the butterfly to dig out.
* Die (Required) - This is how the butterfly stops it's digging. For example, the butterfly might have a 1 in 20 chance to die each time it looks (digs).

Writing actions for each of these steps is called an "instinct," and may require arguments. When a butterfly spawns, it runs through this logic:

* Pick random morph instinct.
* Pick random goal instinct.
* Repeat
 * Check if any of the death instincts and break repeating if true.
 * Use flutter instinct (you can only have one for each butterfly).
 * Pick random look instinct.

# The API!

The API is aimed to be super simple, in fact, currently it's only 3 functions. One for cleanup.

```c
int bf_spawn(struct bf_farm *farm, struct bf_instinct *instincts, size_t count, struct bf_config *config);
void bf_commit(struct bf_farm *farm);
void bf_cleanup(struct bf_farm *farm);
```

# Simple Example (With Comments)

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
 * - First is the morph instinct which is to start anywhere on the
 *   map.
 * - Look is how we dig, this says just at each step dig a 1 FLOOR
 *   tile.
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
 * - First is the morph instinct which is to start anywhere on the
 *   map.
 * - Next is the goal, which is any safe spot on the map that was
 *   commited.
 * - Flutter randomly with a 60% weighted walk to the goal.
 * - Look in a plus symbol. I find this digs really nice caves.
 * - And finally how to die, by arriving at any safe spot.
 */
struct bf_instinct carve[] = {
	{.event = BF_MORPH, .action = BF_MORPH_AT_RANDOM_SPOT},
	{.event = BF_GOAL, .action = BF_GOAL_RANDOM_SAFE_SPOT},
	{.event = BF_FLUTTER, .action = BF_FLUTTER_RANDOMLY_TO_GOAL, {60}},
	{.event = BF_LOOK, .action = BF_LOOK_PLUS_AREA, .args = {FLOOR}},
	{.event = BF_DIE, .action = BF_DIE_AT_SAFE_SPOT},
};
/* Run the instincts on our map. */
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

Just kidding.

```
############################################.......######.........##############
######################....###############..........######..........#############
#####################.......############..........#...........##...#############
#####################........#########........................###.##############
####################........#########.....................#......############.##
####################........#########...................###.......######.##....#
#####################.#......#######.............#......####.....######........#
####....##############........#######...........###.....######..#######........#
###.......#...######.#.........#....#.........#####......#############..........
####...........####..........................######.........#########......##...
#####............#.........#.................######..........########.......#...
#######...........#..............#............#####..........########...........
########.........................##......##...######........######..............
#########.##.............##.......#......###.#######........#####...........##..
#########...............####.............############......###..#...........###.
########...............#####............#######....##......##.......###........#
########...........##########............#####............###.......##..........
#########.#######.##############........######............####.....###..........
##################################..#########............#...#########..........
############################################....................####...........#
###########################################......................##.......#....#
############################################..##.................##.............
################################################.......#.......................#
################################################......##....................#.##
```
