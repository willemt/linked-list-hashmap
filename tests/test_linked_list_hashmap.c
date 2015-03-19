#include <stdbool.h>
#include <assert.h>
#include <setjmp.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "CuTest.h"

#include "linked_list_hashmap.h"

static unsigned long __uint_hash(
    const void *e1
    )
{
    const long i1 = (unsigned long)e1;

    assert(i1 >= 0);
    return i1;
}

static long __uint_compare(
    const void *e1,
    const void *e2
    )
{
    const long i1 = (unsigned long)e1, i2 = (unsigned long)e2;

//      return !(*i1 == *i2);
    return i1 - i2;
}

void TestHashmaplinked_New(
    CuTest * tc
    )
{
    hashmap_t *hm;

    hm = hashmap_new(__uint_hash, __uint_compare, 11);

    CuAssertTrue(tc, 0 == hashmap_count(hm));
    CuAssertTrue(tc, 11 == hashmap_size(hm));

    hashmap_freeall(hm);
}

void TestHashmaplinked_Put(
    CuTest * tc
    )
{
    hashmap_t *hm;

    hm = hashmap_new(__uint_hash, __uint_compare, 11);
    hashmap_put(hm, (void*)50, (void*)92);

    CuAssertTrue(tc, 1 == hashmap_count(hm));

    hashmap_freeall(hm);
}

void TestHashmaplinked_PutEnsuresCapacity(
    CuTest * tc
    )
{
    hashmap_t *hm;

    hm = hashmap_new(__uint_hash, __uint_compare, 1);
    hashmap_put(hm, (void*)50, (void*)92);
    hashmap_put(hm, (void*)51, (void*)92);

    CuAssertTrue(tc, 2 == hashmap_count(hm));
    CuAssertTrue(tc, 2 == hashmap_size(hm));

    hashmap_freeall(hm);
}

void TestHashmaplinked_PutHandlesCollision(
    CuTest * tc
    )
{
    hashmap_t *hm;
    unsigned long val;

    hm = hashmap_new(__uint_hash, __uint_compare, 4);
    hashmap_put(hm, (void*)1, (void*)92);
    hashmap_put(hm, (void*)5, (void*)93);

    CuAssertTrue(tc, 2 == hashmap_count(hm));

    val = (unsigned long)hashmap_get(hm, (void*)5);
    CuAssertTrue(tc, 0 != val);
    CuAssertTrue(tc, val == 93);

    val = (unsigned long)hashmap_get(hm, (void*)1);
    CuAssertTrue(tc, 0 != val);
    CuAssertTrue(tc, val == 92);

    hashmap_freeall(hm);
}

void TestHashmaplinked_GetHandlesCollisionByTraversingChain(
    CuTest * tc
    )
{
    hashmap_t *hm;
    unsigned long val;

    hm = hashmap_new(__uint_hash, __uint_compare, 4);
    hashmap_put(hm, (void*)1, (void*)92);
    hashmap_put(hm, (void*)5, (void*)93);

    CuAssertTrue(tc, 2 == hashmap_count(hm));

    val = (unsigned long)hashmap_get(hm, (void*)5);
    CuAssertTrue(tc, 0 != val);
    CuAssertTrue(tc, val == 93);

    hashmap_freeall(hm);
}

void TestHashmaplinked_RemoveReturnsNullIfMissingAndTraversesChain(
    CuTest * tc
    )
{
    hashmap_t *hm;
    unsigned long val;

    hm = hashmap_new(__uint_hash, __uint_compare, 4);
    hashmap_put(hm, (void*)1, (void*)92);

    val = (unsigned long)hashmap_remove(hm, (void*)5);
    CuAssertTrue(tc, 0 == val);

    val = (unsigned long)hashmap_remove(hm, (void*)1);
    CuAssertTrue(tc, 0 != val);
    CuAssertTrue(tc, val == 92);

    hashmap_freeall(hm);
}

void TestHashmaplinked_RemoveHandlesCollision(
    CuTest * tc
    )
{
    hashmap_t *hm;
    unsigned long val;

    hm = hashmap_new(__uint_hash, __uint_compare, 4);
    hashmap_put(hm, (void*)1, (void*)92);
    hashmap_put(hm, (void*)5, (void*)93);
    hashmap_put(hm, (void*)9, (void*)94);

    val = (unsigned long)hashmap_remove(hm, (void*)5);
    CuAssertTrue(tc, 0 != val);
    CuAssertTrue(tc, val == 93);
    CuAssertTrue(tc, 2 == hashmap_count(hm));

    val = (unsigned long)hashmap_remove(hm, (void*)1);
    CuAssertTrue(tc, 0 != val);
    CuAssertTrue(tc, val == 92);
    CuAssertTrue(tc, 1 == hashmap_count(hm));

    hashmap_freeall(hm);
}

void TestHashmaplinked_PutEntry(
    CuTest * tc
    )
{
    hashmap_t *hm;
    hashmap_entry_t entry;

    hm = hashmap_new(__uint_hash, __uint_compare, 11);
    entry.key = (void*)50;
    entry.val = (void*)92;
    hashmap_put_entry(hm, &entry);
    CuAssertTrue(tc, 1 == hashmap_count(hm));

    hashmap_freeall(hm);
}

void TestHashmaplinked_Get(
    CuTest * tc
    )
{
    hashmap_t *hm;
    unsigned long val;

    hm = hashmap_new(__uint_hash, __uint_compare, 11);

    hashmap_put(hm, (void*)50, (void*)92);

    val = (unsigned long)hashmap_get(hm, (void*)50);

    CuAssertTrue(tc, 0 != val);
    CuAssertTrue(tc, val == 92);

    hashmap_freeall(hm);
}

void TestHashmaplinked_ContainsKey(
    CuTest * tc
    )
{
    hashmap_t *hm;

    hm = hashmap_new(__uint_hash, __uint_compare, 11);

    hashmap_put(hm, (void*)50, (void*)92);

    CuAssertTrue(tc, 1 == hashmap_contains_key(hm, (void*)50));

    hashmap_freeall(hm);
}


void TestHashmaplinked_DoublePut(
    CuTest * tc
    )
{
    hashmap_t *hm;
    unsigned long val;

    hm = hashmap_new(__uint_hash, __uint_compare, 11);
    hashmap_put(hm, (void*)50, (void*)92);
    hashmap_put(hm, (void*)50, (void*)23);
    val = (unsigned long)hashmap_get(hm, (void*)50);
    CuAssertTrue(tc, val == 23);
    CuAssertTrue(tc, 1 == hashmap_count(hm));

    hashmap_freeall(hm);
}

void TestHashmaplinked_Get2(
    CuTest * tc
    )
{
    hashmap_t *hm;

    unsigned long val;

    hm = hashmap_new(__uint_hash, __uint_compare, 11);
    hashmap_put(hm, (void*)50, (void*)92);
    hashmap_put(hm, (void*)49, (void*)91);
    hashmap_put(hm, (void*)48, (void*)90);
    hashmap_put(hm, (void*)47, (void*)89);
    hashmap_put(hm, (void*)46, (void*)88);
    hashmap_put(hm, (void*)45, (void*)87);
    val = (unsigned long)hashmap_get(hm, (void*)48);
    CuAssertTrue(tc, val == 90);

    hashmap_freeall(hm);
}

void TestHashmaplinked_IncreaseCapacityDoesNotBreakHashmap(
    CuTest * tc
    )
{
    hashmap_t *hm;
    unsigned long val;

    hm = hashmap_new(__uint_hash, __uint_compare, 4);
    hashmap_put(hm, (void*)1, (void*)90);
    hashmap_put(hm, (void*)5, (void*)91);
    hashmap_put(hm, (void*)2, (void*)92);
    hashmap_increase_capacity(hm, 2);
    CuAssertTrue(tc, 3 == hashmap_count(hm));
    val = (unsigned long)hashmap_get(hm, (void*)1);
    CuAssertTrue(tc, val == 90);
    val = (unsigned long)hashmap_get(hm, (void*)5);
    CuAssertTrue(tc, val == 91);
    val = (unsigned long)hashmap_get(hm, (void*)2);
    CuAssertTrue(tc, val == 92);

    hashmap_freeall(hm);
}

void TestHashmaplinked_Remove(
    CuTest * tc
    )
{
    hashmap_t *hm;
    unsigned long val;

    hm = hashmap_new(__uint_hash, __uint_compare, 11);

    hashmap_put(hm, (void*)50, (void*)92);

    val = (unsigned long)hashmap_remove(hm, (void*)50);

    CuAssertTrue(tc, 0 != val);
    CuAssertTrue(tc, val == 92);
    CuAssertTrue(tc, 0 == hashmap_count(hm));
    hashmap_freeall(hm);
}

void TestHashmaplinked_ClearRemovesAll(
    CuTest * tc
    )
{
    hashmap_t *hm;
    unsigned long val;

    hm = hashmap_new(__uint_hash, __uint_compare, 11);

    hashmap_put(hm, (void*)1, (void*)92);
    hashmap_put(hm, (void*)2, (void*)102);
    hashmap_clear(hm);

    val = (unsigned long)hashmap_get(hm, (void*)1);
    CuAssertTrue(tc, 0 == val);
    CuAssertTrue(tc, 0 == hashmap_count(hm));
    hashmap_freeall(hm);
}

/* The clear function will need to clean the whole chain */
void TestHashmaplinked_ClearHandlesCollision(
    CuTest * tc
    )
{
    hashmap_t *hm;

    hm = hashmap_new(__uint_hash, __uint_compare, 4);

    hashmap_put(hm, (void*)1, (void*)50);
    /* all of the rest cause collisions */
    hashmap_put(hm, (void*)5, (void*)51);
    hashmap_put(hm, (void*)9, (void*)52);
    hashmap_clear(hm);
    CuAssertTrue(tc, 0 == hashmap_count(hm));
    hashmap_freeall(hm);
}

void TestHashmaplinked_DoesNotHaveNextForEmptyIterator(
    CuTest * tc
    )
{
    hashmap_t *hm;
    hashmap_iterator_t iter;

    hm = hashmap_new(__uint_hash, __uint_compare, 11);
    hashmap_iterator(hm, &iter);
    CuAssertTrue(tc, 0 == hashmap_iterator_has_next(hm, &iter));
    hashmap_freeall(hm);
}

void TestHashmaplinked_RemoveItemDoesNotHaveNextForEmptyIterator(
    CuTest * tc
    )
{
    hashmap_t *hm;
    hashmap_iterator_t iter;

    hm = hashmap_new(__uint_hash, __uint_compare, 11);
    hashmap_put(hm, (void*)9, (void*)52);
    hashmap_remove(hm, (void*)9);
    hashmap_iterator(hm, &iter);
    CuAssertTrue(tc, 0 == hashmap_iterator_has_next(hm, &iter));

    hashmap_freeall(hm);
}

void TestHashmaplinked_Iterate(
    CuTest * tc
    )
{
    hashmap_t *hm;
    hashmap_t *hm2;
    hashmap_iterator_t iter;
    void *key;

    hm = hashmap_new(__uint_hash, __uint_compare, 11);
    hm2 = hashmap_new(__uint_hash, __uint_compare, 11);

    hashmap_put(hm, (void*)50, (void*)92);
    hashmap_put(hm, (void*)49, (void*)91);
    hashmap_put(hm, (void*)48, (void*)90);
    hashmap_put(hm, (void*)47, (void*)89);
    hashmap_put(hm, (void*)46, (void*)88);
    hashmap_put(hm, (void*)45, (void*)87);

    hashmap_put(hm2, (void*)50, (void*)92);
    hashmap_put(hm2, (void*)49, (void*)91);
    hashmap_put(hm2, (void*)48, (void*)90);
    hashmap_put(hm2, (void*)47, (void*)89);
    hashmap_put(hm2, (void*)46, (void*)88);
    hashmap_put(hm2, (void*)45, (void*)87);

    hashmap_iterator(hm, &iter);

    /*  remove every key we iterate on */
    while ((key = hashmap_iterator_next(hm, &iter)))
        CuAssertTrue(tc, NULL != hashmap_remove(hm2, key));

    /*  check if the hashmap is empty */
    CuAssertTrue(tc, 0 == hashmap_count(hm2));

    hashmap_freeall(hm);
    hashmap_freeall(hm2);
}

void TestHashmaplinked_IterateHandlesCollisions(
    CuTest * tc
    )
{
    hashmap_t *hm, *hm2;

    hashmap_iterator_t iter;

    void *key;

    hm = hashmap_new(__uint_hash, __uint_compare, 4);
    hm2 = hashmap_new(__uint_hash, __uint_compare, 4);

    hashmap_put(hm, (void*)1, (void*)92);
    hashmap_put(hm, (void*)5, (void*)91);
    hashmap_put(hm, (void*)9, (void*)90);

    hashmap_put(hm2, (void*)1, (void*)92);
    hashmap_put(hm2, (void*)5, (void*)91);
    hashmap_put(hm2, (void*)9, (void*)90);

    hashmap_iterator(hm, &iter);

    /*  remove every key we iterate on */
    while ((key = hashmap_iterator_next(hm, &iter)))
        CuAssertTrue(tc, NULL != hashmap_remove(hm2, key));

    /*  check if the hashmap is empty */
    CuAssertTrue(tc, 0 == hashmap_count(hm2));

    hashmap_freeall(hm);
    hashmap_freeall(hm2);
}

void TestHashmaplinked_IterateAndRemoveDoesntBreakIteration(
    CuTest * tc
    )
{
    hashmap_t *hm;
    hashmap_t *hm2;
    hashmap_iterator_t iter;
    void *key;

    hm = hashmap_new(__uint_hash, __uint_compare, 11);
    hm2 = hashmap_new(__uint_hash, __uint_compare, 11);

    hashmap_put(hm, (void*)50, (void*)92);
    hashmap_put(hm, (void*)49, (void*)91);
    hashmap_put(hm, (void*)48, (void*)90);
    hashmap_put(hm, (void*)47, (void*)89);
    hashmap_put(hm, (void*)46, (void*)88);
    hashmap_put(hm, (void*)45, (void*)87);
    /*  the following 3 collide: */
    hashmap_put(hm, (void*)1, (void*)92);
    hashmap_put(hm, (void*)5, (void*)91);
    hashmap_put(hm, (void*)9, (void*)90);

    hashmap_put(hm2, (void*)50, (void*)92);
    hashmap_put(hm2, (void*)49, (void*)91);
    hashmap_put(hm2, (void*)48, (void*)90);
    hashmap_put(hm2, (void*)47, (void*)89);
    hashmap_put(hm2, (void*)46, (void*)88);
    hashmap_put(hm2, (void*)45, (void*)87);
    /*  the following 3 collide: */
    hashmap_put(hm2, (void*)1, (void*)92);
    hashmap_put(hm2, (void*)5, (void*)91);
    hashmap_put(hm2, (void*)9, (void*)90);

    hashmap_iterator(hm, &iter);

    /*  remove every key we iterate on */
    while ((key = hashmap_iterator_next(hm, &iter)))
    {
        CuAssertTrue(tc, NULL != hashmap_remove(hm2, key));
        hashmap_remove(hm, key);
    }

    /*  check if the hashmap is empty */
    CuAssertTrue(tc, 0 == hashmap_count(hm2));

    hashmap_freeall(hm);
    hashmap_freeall(hm2);
}

