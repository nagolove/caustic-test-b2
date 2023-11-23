// vim: set colorcolumn=85
// vim: fdm=marker

#include "koh.h"
#include "koh_b2.h"
#include "koh_common.h"
#include "koh_metaloader.h"
#include "koh_rand.h"
#include "koh_set.h"
#include "lauxlib.h"
#include "lua.h"
#include "lualib.h"
#include "munit.h"
#include "raylib.h"
#include <assert.h>
#include "box2d/box2d.h"
#include <math.h>
#include <memory.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "lua.h"
#include "koh_lua_tools.h"

static bool verbose = false;

static MunitResult test_b2Vec2_tostr_alloc(
    const MunitParameter params[], void* data
) {
    b2Vec2 verts[] = {
        {1., 1.},
        {-1., -1.},
        {0., 0.},
        {34., 0.3434},
    };
    int num = sizeof(verts) / sizeof(verts[0]);
    char *str = b2Vec2_tostr_alloc(verts, num);
    munit_assert_not_null(str);
    if (str) {
        if (verbose)
            printf("str %s\n", str);
        
        lua_State *l = luaL_newstate();
        assert(l);
        luaL_openlibs(l);

        const char *code = 
            "return function(s)\n"
            "local ok, _ = string.match(s, \n"
            "'{.*1%.0.*1%.0.*}.*,.*' .. \n"
            "'{.*%-.*1%.0.*%-.*1%.0.*}.*,.*' .. \n"
            "'.*{.*0%.0.*}.*,.*' .. \n"
            "'.*{.*34%.0.*0%.3434.*}.*,.*'\n"
            ")\n"
            "return ok\n"
            "end\n";
            //"return CHECK\n";

        if (verbose)
            printf("[%s]\n", stack_dump(l));
        int res = luaL_loadstring(l, code);
        if (res != LUA_OK) {
            printf("%s", lua_tostring(l, -1));
            exit(EXIT_FAILURE);
        }
        lua_call(l, 0, LUA_MULTRET);
        if (verbose)
            printf("[%s]\n", stack_dump(l));

        lua_pushstring(l, str);

        if (verbose)
            printf("[%s]\n", stack_dump(l));
        lua_call(l, 1, 1);
        munit_assert(lua_toboolean(l, -1) == true);
        if (verbose)
            printf("[%s]\n", stack_dump(l));
        lua_close(l);

        free(str);
    }
    return MUNIT_OK;
}

static MunitTest t_suite_common[] = {
    {
        .name =  (char*) "/test_b2Vec2_tostr_alloc",
        .test = test_b2Vec2_tostr_alloc,
        .setup = NULL,
        .tear_down = NULL,
        .options = MUNIT_TEST_OPTION_NONE,
        .parameters = NULL,
    },
    {
        .name =  NULL,
        .test = NULL,
        .setup = NULL,
        .tear_down = NULL,
        .options = MUNIT_TEST_OPTION_NONE,
        .parameters = NULL,
    },
};


static const MunitSuite suite_root = {
    .prefix = (char*) "b2",
    .tests =  t_suite_common,
    /*.suites = suite_nested,*/
    .suites = NULL,
    .iterations = 1,
    .options = MUNIT_SUITE_OPTION_NONE,
};

int main(int argc, char **argv) {
    return munit_suite_main(&suite_root, (void*) "µnit", argc, argv);
}
