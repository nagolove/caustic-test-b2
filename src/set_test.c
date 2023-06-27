// vim: set colorcolumn=85
// vim: fdm=marker

#include "koh_destral_ecs.h"
#include "koh_set.h"
#include "munit.h"
#include "raylib.h"
#include <assert.h>
#include <math.h>
#include <memory.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct Vectors {
    Vector2 *vecs;
    int     num;
};

static bool iter_set_remove(const void *key, int key_len, void *udata) {
    return false;
}

static bool iter_set_cmp(const void *key, int key_len, void *udata) {
    struct Vectors *lines = udata;

    for (int i = 0; i < lines->num; ++i) {
        if (!memcmp(&lines->vecs[i], key, key_len)) {
            return true;
        }
    }

    //printf("iter_set: key %s not found in each itertor\n", key);
    munit_assert(false);

    return true;
}

static MunitResult test_new_add_exist_free(
    const MunitParameter params[], void* data
) {
    koh_Set *set = set_new();
    munit_assert_ptr_not_null(set);

    Vector2 vecs[] = {
        { 1.,    0. },
        { 12.,   0. },
        { 0.1,   0. },
        { 1.3, -0.1 },
        { 0.,   0.5 },
        { 14,    0. },
    };

    Vector2 other_vecs[] = {
        { -1.,   0. },
        { 12.,  NAN },
        { 0.1,  0.1 },
        { 1.3,  0.1 },
        { 0.,  0.51 },
        { -14,   0. },
    };

    int vecs_num = sizeof(vecs) / sizeof(vecs[0]);
    int other_vecs_num = sizeof(other_vecs) / sizeof(other_vecs[0]);

    for (int i = 0; i< vecs_num; ++i) {
        set_add(set, &vecs[i], sizeof(vecs[0]));
    }

    for (int i = 0; i< vecs_num; ++i) {
        munit_assert(set_exist(set, &vecs[i], sizeof(vecs[0])));
    }

    for (int i = 0; i< other_vecs_num; ++i) {
        munit_assert(!set_exist(set, &other_vecs[i], sizeof(other_vecs[0])));
    }

    struct Vectors vectors_ctx = {
        //.vecs = (Vector2**)vecs, 
        .vecs = vecs, 
        .num = vecs_num,
    };
    // проверка всех ключей
    set_each(set, iter_set_cmp, &vectors_ctx);

    // удаление все ключей
    set_each(set, iter_set_remove, &vectors_ctx);

    Vector2 pi_vec = { M_PI, M_PI };
    set_add(set, &pi_vec, sizeof(Vector2));

    for (int i = 0; i< vecs_num; ++i) {
        munit_assert(!set_exist(set, &vecs[i], sizeof(Vector2)));
    }

    munit_assert(set_exist(set, &pi_vec, sizeof(Vector2)));
    set_clear(set);
    munit_assert(!set_exist(set, &pi_vec, sizeof(Vector2)));

    set_free(set);
    return MUNIT_OK;
}

static MunitTest test_suite_tests[] = {
  {
    (char*) "/new_add_exist_free",
    test_new_add_exist_free,
    NULL,
    NULL,
    MUNIT_TEST_OPTION_NONE,
    NULL
  },
  { NULL, NULL, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL }
};

static const MunitSuite test_suite = {
  (char*) "set", test_suite_tests, NULL, 1, MUNIT_SUITE_OPTION_NONE
};

int main(int argc, char **argv) {
    return munit_suite_main(&test_suite, (void*) "µnit", argc, argv);
}