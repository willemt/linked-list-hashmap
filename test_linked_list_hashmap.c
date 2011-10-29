#include <stdbool.h>
#include <assert.h>
#include <setjmp.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "CuTest.h"

#include "linked_list_hashmap.h"

static unsigned int __uint_hash(
    const void *e1
)
{
    const long i1 = (unsigned long) e1;

    assert(i1 >= 0);
    return i1;
}

static int __uint_compare(
    const void *e1,
    const void *e2
)
{
    const long i1 = (unsigned long) e1, i2 = (unsigned long) e2;

//      return !(*i1 == *i2); 
    return i1 - i2;
}

void TestHashmaplinked_New(
    CuTest * tc
)
{
    hashmap_t *hm;

    hm = hashmap_new(__uint_hash, __uint_compare);

    CuAssertTrue(tc, 0 == hashmap_count(hm));
}

void TestHashmaplinked_Put(
    CuTest * tc
)
{
    hashmap_t *hm;

    hm = hashmap_new(__uint_hash, __uint_compare);
    hashmap_put(hm, (void *) 50, (void *) 92);

    CuAssertTrue(tc, 1 == hashmap_count(hm));
}

void TestHashmaplinked_Get(
    CuTest * tc
)
{
    hashmap_t *hm;

    unsigned long val;

    hm = hashmap_new(__uint_hash, __uint_compare);

    hashmap_put(hm, (void *) 50, (void *) 92);

    val = (unsigned long) hashmap_get(hm, (void *) 50);

    CuAssertTrue(tc, 0 != val);
    CuAssertTrue(tc, val == 92);
}

void TestHashmaplinked_DoublePut(
    CuTest * tc
)
{
    hashmap_t *hm;

    unsigned long val;

    hm = hashmap_new(__uint_hash, __uint_compare);
    hashmap_put(hm, (void *) 50, (void *) 92);
    hashmap_put(hm, (void *) 50, (void *) 23);
    val = (unsigned long) hashmap_get(hm, (void *) 50);
    CuAssertTrue(tc, val == 23);
}

void TestHashmaplinked_Get2(
    CuTest * tc
)
{
    hashmap_t *hm;

    unsigned long val;

    hm = hashmap_new(__uint_hash, __uint_compare);
    hashmap_put(hm, (void *) 50, (void *) 92);
    hashmap_put(hm, (void *) 49, (void *) 91);
    hashmap_put(hm, (void *) 48, (void *) 90);
    hashmap_put(hm, (void *) 47, (void *) 89);
    hashmap_put(hm, (void *) 46, (void *) 88);
    hashmap_put(hm, (void *) 45, (void *) 87);
    val = (unsigned long) hashmap_get(hm, (void *) 48);
    CuAssertTrue(tc, val == 90);
}

void TestHashmaplinked_Remove(
    CuTest * tc
)
{
    hashmap_t *hm;

    unsigned long val;

    hm = hashmap_new(__uint_hash, __uint_compare);

    hashmap_put(hm, (void *) 50, (void *) 92);

    val = (unsigned long) hashmap_remove(hm, (void *) 50);

    CuAssertTrue(tc, 0 != val);
    CuAssertTrue(tc, val == 92);
    CuAssertTrue(tc, 0 == hashmap_count(hm));
}

void TestHashmaplinked_Iterate(
    CuTest * tc
)
{
    hashmap_t *hm;

    hashmap_t *hm2;

    hashmap_iterator_t iter;

    void *key;

    unsigned long val;

    hm = hashmap_new(__uint_hash, __uint_compare);
    hm2 = hashmap_new(__uint_hash, __uint_compare);

    hashmap_put(hm, (void *) 50, (void *) 92);
    hashmap_put(hm, (void *) 49, (void *) 91);
    hashmap_put(hm, (void *) 48, (void *) 90);
    hashmap_put(hm, (void *) 47, (void *) 89);
    hashmap_put(hm, (void *) 46, (void *) 88);
    hashmap_put(hm, (void *) 45, (void *) 87);

    hashmap_put(hm2, (void *) 50, (void *) 92);
    hashmap_put(hm2, (void *) 49, (void *) 91);
    hashmap_put(hm2, (void *) 48, (void *) 90);
    hashmap_put(hm2, (void *) 47, (void *) 89);
    hashmap_put(hm2, (void *) 46, (void *) 88);
    hashmap_put(hm2, (void *) 45, (void *) 87);

    hashmap_iterator(hm, &iter);

    /*  remove every key we iterate on */
    while ((key = hashmap_iterator_next(hm, &iter)))
    {
        CuAssertTrue(tc, NULL != hashmap_remove(hm2, key));
    }

    /*  check if the hashmap is empty */
    CuAssertTrue(tc, 0 == hashmap_count(hm2));
}
