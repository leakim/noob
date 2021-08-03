#include "doctest.h"

#include <unistd.h>
#include <iostream>
#include <cstdint>

#define TYPE_A    (0x10 << 24)
#define TYPE_B    (0x20 << 24)
#define TYPE_MASK (0xff << 24)

struct sample {
    uint32_t key;
    uint32_t value;
};

struct key_iterator {
    int count;
};

void
key_worker(void *ctx, uint32_t key, uint32_t value)
{
    struct key_iterator *p = (struct key_iterator*)ctx;
    p->count++;
    // do something with the key and value pair
}

typedef void (*work_cb_fp)(void *ctx, uint32_t key, uint32_t value);

void
iterate_keys(void* ctx, struct sample *data, int count, work_cb_fp fp)
{
    for (int i=0; i < count; i++)
    {
        uint32_t x = data->key;
        uint32_t type = x & TYPE_MASK;
        if (type == TYPE_A)
            fp(ctx, x, data->value);
        if (type == TYPE_B) {
            for(; x != 0 ;)
            {
                int pos = __builtin_ctz(x);
                fp(ctx, TYPE_B | pos, data->value);
                x = x & ~(1 << pos);
                //std::cout << pos << std::endl;
            }
        }
        data++;
    }
}

SCENARIO("interate over sample data") {
    GIVEN("sampes") {

        struct sample data[5] = {
            { TYPE_A | 0x1, 0},
            { TYPE_A | 0x2, 1},
            { TYPE_A | 1 | (1 << 4), 2},
            { TYPE_A | 0x1000, 3},
            { TYPE_B | 1 | (1 << 4), 4}  // bit 0 and bit 4
        };

        struct key_iterator ctx = {0};

        WHEN("next") {
            iterate_keys(&ctx, data, 5, key_worker);

            THEN("done") {
                CHECK(ctx.count == 6);
            }
        }
    }
}
