//
// Unit tests for swconf.c using the Check framework
//

#include <check.h>
#include <stdlib.h>
#include <string.h>

/* Include std.h first - it provides bool definition that swconf.h needs */
#include "../src/std.h"

/* Include the source we're testing - but swconf.c has heavy dependencies.
 * Instead, we test the conf_option struct and header interface.
 * For full integration tests, we'd need to link with the actual objects.
 */

/* Test stub: minimal implementation of the conf_option array access
 * We test the header interface and conf_option_by_name() function
 */

/* Include the actual header being tested.
 * Note: swconf.h uses bool but doesn't include std.h itself,
 * so we include std.h above first. */
#include "../src/swconf.h"

/* Stubs for variables referenced by swconf.c (defined elsewhere in the project) */
bool conf_missiles = false;
bool conf_solidground = false;
bool conf_hudsplats = false;
bool conf_wounded = false;
bool conf_animals = false;
bool conf_harrykeys = false;
bool conf_big_explosions = false;
bool conf_medals = false;
bool vid_fullscreen = false;
bool snd_tinnyfilter = false;
int conf_video_palette = 0;

/* Key binding stubs - must match the definitions in the project */
#define KEY_ACCEL 0
#define KEY_DECEL 1
#define KEY_PULLUP 2
#define KEY_PULLDOWN 3
#define KEY_FLIP 4
#define KEY_FIRE 5
#define KEY_BOMB 6
#define KEY_HOME 7
#define KEY_MISSILE 8
#define KEY_STARBURST 9
int keybindings[10] = {0};
int controller_bindings[10] = {0};

/* Menu stubs from swmenu.h */
enum menu_action {
    MenuRun,
    MenuClose,
    MenuSubMenu,
};

struct menuitem {
    int key;
    const char *label;
    enum menu_action (*callback)(struct menuitem *);
    const char *data;
};

struct menu {
    void (*draw_bg)(void *);
    const char *title;
    struct menuitem *items;
};

#define CONFIG_OPTION(label, config_name) {'1', label, ToggleConfigOption, config_name}

enum menu_action ToggleConfigOption(struct menuitem *item) { return MenuClose; }
enum menu_action SubMenu(struct menuitem *item) { return MenuClose; }
void FullscreenBackground(void *title) {}

/* Video stubs */
const char *Vid_GetPrefPath(void) { return "./"; }
int Vid_GetNumVideoPalettes(void) { return 1; }

/* Memory stubs */
void *checked_realloc(void *p, size_t len) { (void)p; return malloc(len); }
void *checked_calloc(size_t n, size_t s) { return calloc(n, s); }
char *checked_strdup(const char *s) { return strdup(s); }

START_TEST(test_conf_option_by_name_valid)
{
    const struct conf_option *opt;

    opt = ConfOptionByName("conf_missiles");
    ck_assert_ptr_ne(opt, NULL);
    ck_assert_int_eq(opt->type, CONF_BOOL);

    opt = ConfOptionByName("conf_solidground");
    ck_assert_ptr_ne(opt, NULL);
    ck_assert_int_eq(opt->type, CONF_BOOL);

    opt = ConfOptionByName("vid_fullscreen");
    ck_assert_ptr_ne(opt, NULL);
    ck_assert_int_eq(opt->type, CONF_BOOL);

    opt = ConfOptionByName("conf_video_palette");
    ck_assert_ptr_ne(opt, NULL);
    ck_assert_int_eq(opt->type, CONF_INT);

    opt = ConfOptionByName("key_accelerate");
    ck_assert_ptr_ne(opt, NULL);
    ck_assert_int_eq(opt->type, CONF_KEY);
}
END_TEST

START_TEST(test_conf_option_by_name_invalid)
{
    const struct conf_option *opt;

    opt = ConfOptionByName("nonexistent_option");
    ck_assert_ptr_eq(opt, NULL);

    /* Note: ConfOptionByName uses strcasecmp, so it's case-insensitive */
    /* Use a clearly non-existent option */
    opt = ConfOptionByName("invalid_option");
    ck_assert_ptr_eq(opt, NULL);
}
END_TEST

START_TEST(test_conf_option_name_lengths)
{
    const struct conf_option *opt;
    size_t namelen;

    /* Test that name is properly null-terminated by checking lengths */
    opt = ConfOptionByName("conf_missiles");
    ck_assert_ptr_ne(opt, NULL);
    namelen = strlen(opt->name);
    ck_assert_int_eq(namelen, 13);  /* "conf_missiles" */

    opt = ConfOptionByName("controller_accelerate");
    ck_assert_ptr_ne(opt, NULL);
    namelen = strlen(opt->name);
    ck_assert_int_eq(namelen, 21);  /* "controller_accelerate" */
}
END_TEST

Suite *swconf_suite(void)
{
    Suite *s;
    TCase *tc_core;

    s = suite_create("swconf");
    tc_core = tcase_create("Core");

    tcase_add_test(tc_core, test_conf_option_by_name_valid);
    tcase_add_test(tc_core, test_conf_option_by_name_invalid);
    tcase_add_test(tc_core, test_conf_option_name_lengths);

    suite_add_tcase(s, tc_core);

    return s;
}

int main(void)
{
    int number_failed;
    SRunner *sr;

    sr = srunner_create(swconf_suite());
    srunner_run_all(sr, CK_NORMAL);
    number_failed = srunner_ntests_failed(sr);
    srunner_free(sr);
    return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
