// vim: set colorcolumn=85
// vim: fdm=marker

// {{{ include
#include "body.h"
#include "box2d/id.h"
#include "box2d/types.h"
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
// }}}

static bool verbose = false;
extern bool b2_parallel;

static MunitResult test_shapes_density_and_friction(
    const MunitParameter params[], void* data
) {
    b2_parallel = false;
    b2WorldDef world_def =  b2DefaultWorldDef();
    world_def.gravity = b2Vec2_zero;

    b2WorldId world = b2CreateWorld(&world_def);
    int relax_iterations = 6;
    int vel_iteratioins = 2;

    for (int i = 0; i < 500; i++) {
        b2Polygon poly = b2MakeSquare(50.);
        float rnd = rand() / (double)RAND_MAX;
        if (verbose)
            trace("test_shapes_density_and_friction: rnd %f\n", rnd);
        float w = 2. * (M_PI - rnd * M_PI * 2.);
        const int32_t abs_linear_vel = 100;
        b2Vec2 linear_vel = {
            abs_linear_vel - rand() % (2 * abs_linear_vel),
            abs_linear_vel - rand() % (2 * abs_linear_vel),
        };

        int width = 1920, height = 1080;
        b2Vec2 pos = {
            .x = rand() % width,
            .y = rand() % height,
        };

        if (verbose)
            printf(
                "test_shapes_density_and_friction: "
                "{ pos = %s, poly = %s, v = %s, w = %f, }\n",
                b2Vec2_to_str(pos),
                b2Polygon_to_str(&poly),
                b2Vec2_to_str(linear_vel),
                w
            );

        b2BodyDef body_def = b2DefaultBodyDef();
        //body_def.isEnabled = true;
        body_def.position = pos;
        //body_def.isAwake = true;

        const bool use_static = false;
        if (use_static)
            body_def.type = b2_staticBody;
        else
            body_def.type = b2_dynamicBody;

        b2BodyId body = b2World_CreateBody(world, &body_def);
        b2ShapeDef shape_def = b2DefaultShapeDef();
        shape_def.density = 1.0;
        shape_def.friction = 0.5;
        b2Body_CreatePolygon(body, &shape_def, &poly);

        //b2Body_SetLinearVelocity(body, linear_vel);
        //b2Body_SetAngularVelocity(body, w);

        //b2Body_Enable(body);
    }

    for (int i = 0; i < 100; i++) {
        b2World_Step(world, 1. / 1000., vel_iteratioins, relax_iterations);
    }

    b2DestroyWorld(world);

    return MUNIT_OK;
}

static MunitResult test_b2Statistics_to_str(
    const MunitParameter params[], void* data
) {
    b2WorldDef wd = b2DefaultWorldDef();
    b2WorldId world = b2CreateWorld(&wd);
    char **lines = b2Statistics_to_str(world, false);
    munit_assert_ptr_not_null(lines);

    FILE *out;
    if (verbose)
        out = stdout;
    else
        out = fopen("/dev/null", "w");

    while (*lines) {
        fprintf(out, "%s\n", *lines);
        lines++;
    }

    lines = b2Statistics_to_str(world, true);
    char *line = calloc(2048, sizeof(char));
    if (!line)
        return MUNIT_ERROR;

    // Проверка за выгрузку данных в Луа таблицу
    strcat(line, "return ");
    while (*lines) {
        strcat(line, *lines);
        lines++;
    }
    if (verbose)
        printf("test_b2Statistics_to_str: line '%s'\n", line);
    lua_State *l = luaL_newstate();
    assert(l);
    luaL_openlibs(l);
    if (luaL_loadstring(l, line) != LUA_OK) {
        printf("test_b2Statistics_to_str: %s\n", lua_tostring(l, -1));
        free(line);
        lua_close(l);
        return MUNIT_FAIL;
    }
    if (verbose)
        printf("test_b2Statistics_to_str: [%s]", stack_dump(l));
    munit_assert(lua_gettop(l) == 1);
    lua_call(l, 0, 0);
    munit_assert(lua_gettop(l) == 0);
    if (verbose)
        printf("test_b2Statistics_to_str: [%s]", stack_dump(l));

    lua_close(l);
    free(line);

    fclose(out);
    b2DestroyWorld(world);

    return MUNIT_OK;
}

static MunitResult test_b2WorldDef_to_str(
    const MunitParameter params[], void* data
) {
    b2WorldDef wdef = b2DefaultWorldDef();
    char **lines = b2WorldDef_to_str(wdef, false);

    FILE *out;
    if (verbose)
        out = stdout;
    else
        out = fopen("/dev/null", "w");

    while (*lines) {
        fprintf(out, "%s\n", *lines);
        lines++;
    }

    if (!verbose)
        fclose(out);

    lines = b2WorldDef_to_str(wdef, true);
    char *line = calloc(2048, sizeof(char));
    if (!line)
        return MUNIT_ERROR;

    // Проверка за выгрузку данных в Луа таблицу
    strcat(line, "return ");
    while (*lines) {
        strcat(line, *lines);
        lines++;
    }
    if (verbose)
        printf("test_b2WorldDef_to_str: line '%s'\n", line);
    lua_State *l = luaL_newstate();
    assert(l);
    luaL_openlibs(l);
    if (luaL_loadstring(l, line) != LUA_OK) {
        printf("test_b2WorldDef_to_str: %s\n", lua_tostring(l, -1));
        free(line);
        lua_close(l);
        return MUNIT_FAIL;
    }
    if (verbose)
        printf("test_b2WorldDef_to_str: [%s]", stack_dump(l));
    munit_assert(lua_gettop(l) == 1);
    lua_call(l, 0, 0);
    munit_assert(lua_gettop(l) == 0);
    if (verbose)
        printf("test_b2WorldDef_to_str: [%s]", stack_dump(l));

    lua_close(l);
    free(line);

    return MUNIT_OK;
}

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
        .name =  (char*) "/test_shapes_density_and_friction",
        .test = test_shapes_density_and_friction,
        .setup = NULL,
        .tear_down = NULL,
        .options = MUNIT_TEST_OPTION_NONE,
        .parameters = NULL,
    },
    {
        .name =  (char*) "/test_b2Vec2_tostr_alloc",
        .test = test_b2Vec2_tostr_alloc,
        .setup = NULL,
        .tear_down = NULL,
        .options = MUNIT_TEST_OPTION_NONE,
        .parameters = NULL,
    },
    {
        .name =  (char*) "/test_b2Statistics_to_str",
        .test = test_b2Statistics_to_str,
        .setup = NULL,
        .tear_down = NULL,
        .options = MUNIT_TEST_OPTION_NONE,
        .parameters = NULL,
    },
    {
        .name =  (char*) "/test_b2WorldDef_to_str",
        .test = test_b2WorldDef_to_str,
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
