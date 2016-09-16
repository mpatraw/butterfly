![butterfly](https://github.com/mpatraw/butterfly/raw/master/doc/butterfly.png "butterfly")

# Butterfly

Butterfly is designed to be a simple general implementation of the Drunkard's Walk algorithm for generation 2D maps.

# Algorithm

Butterfly breaks down the Drunkard's Walk algorithm into several steps. Each step can be controlled to produce varying dungeons.

The steps are:

* Morph (Optional) - This is where the butterfly starts it's "walk."
* Goal (Optional) - This just sets an optional goal location for the butterfly to aim towards.
* Flutter (Optional) - This is the "walk" algorithm. It can be a weighted walk or predefined path like a straight line.
* Look (Optional) - This is the way the butterfly "digs" out the dungeon.
* Die (Required) - This is how the butterfly stops it's digging.

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
/* This can be created any way you'd like. This is an 80x24 map filled with
 * walls (0).
 */
int spots[24][80] = {{WALL}};
/* The only three members that are required when using Butterfly. */
struct bf_farm farm = {
	.spots = (void *)spots,
	.width = 80,
	.height = 24
};
/* Building the instincts (instructions).
 * - First is the morph instinct which is to start anywhere on the
 *   map.
 * - Next is the goal, which is another random spot.
 * - Flutter is how it moves somewhere, in this case, just a line.
 * - Look is how we dig, this says just at each step dig a 1 FLOOR
 *   tile.
 * - And finally how to die, buy arriving at our goal (flutter
 *   guarantees this).
 */
struct bf_instinct carve[] = {
	{.event = BF_MORPH, .action = BF_MORPH_AT_RANDOM_SPOT},
	{.event = BF_GOAL, .action = BF_GOAL_RANDOM_SPOT},
	{.event = BF_FLUTTER, .action = BF_FLUTTER_LINE_TO_GOAL},
	{.event = BF_LOOK, .action = BF_LOOK_1_AREA, .args = {FLOOR}},
	{.event = BF_DIE, .action = BF_DIE_AT_GOAL},
};
/* Run the instincts on our map. */
BF_SPAWN_ARR(&farm, carve, NULL);
/* Commit these changes. */
bf_commit(&farm);
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
