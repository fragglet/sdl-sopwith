//
// Unit tests for collision system using the Check framework
//

#include <check.h>
#include <stdlib.h>
#include <string.h>

/* config.h must come first - std.h includes it */
#include "../config.h"

/* Include std.h first - it provides bool definition */
#include "../src/std.h"

#include "../src/sw.h"
#include "../src/swcollsn.h"
#include "../src/swsymbol.h"

/* Stubs for global variables referenced by swcollsn.c */
playmode_t playmode = PLAYMODE_SINGLE;
int ground[640];  /* Simulated ground array */
OBJECTS *planes[MAX_PLYR] = {NULL};
OBJECTS consoleplayer_obj;
OBJECTS *consoleplayer = &consoleplayer_obj;
int numtarg[NUM_FACTIONS] = {0};
GAMES currgame_obj;
GAMES *currgame = &currgame_obj;
int gamenum = 0;

/* Stub functions from other modules */
void stopsound(OBJECTS *ob) { (void)ob; }
void initexpl(OBJECTS *ob, int flag) { (void)ob; (void)flag; }
void endgame(faction_t winner) { (void)winner; }
void crashpln(OBJECTS *ob) { (void)ob; }
void hitpln(OBJECTS *ob) { (void)ob; }
bool moveplyr(OBJECTS *ob) { (void)ob; return false; }
void swwindshot(void) {}
void swsplatox(void) {}
void swsplattarget(void) {}
void swsplatbird(void) {}

/* Additional stubs for swcollsn.c */
bool conf_wounded = false;
int countmove = 0;
OBJECTS topobj = {0}, botobj = {0};
bool PlaneIsWounded(obstate_t state) { return state == WOUNDED || state == WOUNDSTALL; }
bool PlaneIsFlying(obstate_t state) { return state == FLYING || state == WOUNDED; }
void initbird(OBJECTS *ob, int i) { (void)ob; (void)i; }

/* Helper to create a simple 4x4 sprite with a single pixel set */
static sopsym_t *make_test_symbol(int x, int y)
{
    sopsym_t *sym = malloc(sizeof(sopsym_t));
    ck_assert_ptr_ne(sym, NULL);
    sym->w = 4;
    sym->h = 4;
    sym->data = calloc(16, sizeof(uint8_t));
    ck_assert_ptr_ne(sym->data, NULL);

    /* Set only the pixel at (x, y) within the 4x4 grid */
    if (x >= 0 && x < 4 && y >= 0 && y < 4) {
        sym->data[y * 4 + x] = 1;
    }
    return sym;
}

/* Create a sprite that fills all pixels */
static sopsym_t *make_full_symbol(void)
{
    sopsym_t *sym = malloc(sizeof(sopsym_t));
    ck_assert_ptr_ne(sym, NULL);
    sym->w = 4;
    sym->h = 4;
    sym->data = calloc(16, sizeof(uint8_t));
    ck_assert_ptr_ne(sym->data, NULL);
    memset(sym->data, 1, 16);  /* All pixels filled */
    return sym;
}

static void free_symbol(sopsym_t *sym)
{
    free(sym->data);
    free(sym);
}

static OBJECTS *make_test_object(int x, int y, int type, obstate_t state)
{
    OBJECTS *ob = calloc(1, sizeof(OBJECTS));
    ck_assert_ptr_ne(ob, NULL);
    ob->ob_x = x;
    ob->ob_y = y;
    ob->ob_type = type;
    ob->ob_state = state;
    ob->ob_symbol = make_test_symbol(1, 1);  /* Simple 4x4 sprite with pixel at (1,1) */
    ob->ob_onmap = true;
    return ob;
}

static void free_object(OBJECTS *ob)
{
    if (ob && ob->ob_symbol) {
        free_symbol(ob->ob_symbol);
    }
    free(ob);
}

/* Test that non-overlapping objects return false */
START_TEST(test_collision_no_overlap)
{
    OBJECTS *ob1 = make_test_object(0, 10, PLANE, FLYING);
    OBJECTS *ob2 = make_test_object(100, 10, PLANE, FLYING);

    ck_assert(!CollisionTest(ob1, ob2));

    free_object(ob1);
    free_object(ob2);
}
END_TEST

/* Test that overlapping objects return true */
START_TEST(test_collision_overlap)
{
    /* Two objects at same position should collide */
    OBJECTS *ob1 = make_test_object(10, 10, PLANE, FLYING);
    OBJECTS *ob2 = make_test_object(10, 10, PLANE, FLYING);

    ck_assert(CollisionTest(ob1, ob2));

    free_object(ob1);
    free_object(ob2);
}
END_TEST

/* Test finished objects don't collide */
START_TEST(test_collision_finished_objects)
{
    OBJECTS *ob1 = make_test_object(10, 10, PLANE, FINISHED);
    OBJECTS *ob2 = make_test_object(10, 10, PLANE, FLYING);

    ck_assert(!CollisionTest(ob1, ob2));
    ck_assert(!CollisionTest(ob2, ob1));

    free_object(ob1);
    free_object(ob2);
}
END_TEST

/* Test that partial overlap works correctly */
START_TEST(test_collision_partial_overlap)
{
    /* Two fully-filled sprites that partially overlap should collide */
    OBJECTS *ob1;
    OBJECTS *ob2;

    ob1 = make_test_object(0, 10, PLANE, FLYING);
    free_symbol(ob1->ob_symbol);
    ob1->ob_symbol = make_full_symbol();

    ob2 = make_test_object(2, 10, PLANE, FLYING);
    free_symbol(ob2->ob_symbol);
    ob2->ob_symbol = make_full_symbol();

    ck_assert(CollisionTest(ob1, ob2));

    free_object(ob1);
    free_object(ob2);
}
END_TEST

Suite *swcollsn_suite(void)
{
    Suite *s;
    TCase *tc_core;

    s = suite_create("swcollsn");
    tc_core = tcase_create("Core");

    tcase_add_test(tc_core, test_collision_no_overlap);
    tcase_add_test(tc_core, test_collision_overlap);
    tcase_add_test(tc_core, test_collision_finished_objects);
    tcase_add_test(tc_core, test_collision_partial_overlap);

    suite_add_tcase(s, tc_core);

    return s;
}

int main(void)
{
    int number_failed;
    SRunner *sr;

    sr = srunner_create(swcollsn_suite());
    srunner_run_all(sr, CK_NORMAL);
    number_failed = srunner_ntests_failed(sr);
    srunner_free(sr);
    return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}