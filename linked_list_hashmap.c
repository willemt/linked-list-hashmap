/*
 
Copyright (c) 2011, Willem-Hendrik Thiart
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:
    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.
    * The names of its contributors may not be used to endorse or promote
      products derived from this software without specific prior written
      permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL WILLEM-HENDRIK THIART BE LIABLE FOR ANY
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*/

#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <string.h>
#include <assert.h>

#include "linked_list_hashmap.h"

/* when we call for more capacity */
#define SPACERATIO 0.1
#define INITIAL_CAPACITY 11

typedef struct hash_node_s hash_node_t;

struct hash_node_s
{
    hash_entry_t ety;
    hash_node_t *next;
};

static void __ensurecapacity(
    hashmap_t * hmap
);

static hash_node_t *__allocnodes(
    hashmap_t * hmap,
    int count
);

inline static void __freenodes(
    hashmap_t * hmap,
    int count,
    hash_node_t * nodes
)
{
    free(nodes);
}

/*----------------------------------------------------------------------------*/

hashmap_t *hashmap_new(
    func_longhash_f hash,
    func_longcmp_f cmp
)
{
    hashmap_t *hmap;

    hmap = calloc(1, sizeof(hashmap_t));
    hmap->arraySize = INITIAL_CAPACITY;
    hmap->array = __allocnodes(hmap, hmap->arraySize);
    hmap->hash = hash;
    hmap->compare = cmp;
    return hmap;
}

/*----------------------------------------------------------------------------*/

/**
 * @return number of items within hashmap */
int hashmap_count(
    hashmap_t * hmap
)
{
    return hmap->count;
}

/**
 * free all the nodes in a chain, recursively. */
static void __node_empty(
    hashmap_t * hmap,
    hash_node_t * node
)
{
    if (NULL == node)
    {
        return;
    }
    else
    {
        __node_empty(hmap, node->next);
        free(node);
        hmap->count--;
    }
}

/**
 * Empty this hashmap. */
void hashmap_clear(
    hashmap_t * hmap
)
{
    int ii;

    for (ii = 0; ii < hmap->arraySize; ii++)
    {
        hash_node_t *node;

        node = &((hash_node_t *) hmap->array)[ii];

        if (NULL == node->ety.key)
            continue;

        /* normal actions will overwrite the value */
        node->ety.key = NULL;

        /* empty and free this chain */
        __node_empty(hmap, node->next);
        node->next = NULL;

        hmap->count--;
        assert(0 <= hmap->count);
    }

    assert(0 == hashmap_count(hmap));
}

/**
 * Free all the memory related to this hashmap. */
void hashmap_free(
    hashmap_t * hmap
)
{
    assert(hmap);
    hashmap_clear(hmap);
}

/**
 * Free all the memory related to this hashmap.
 * This includes the actual hmap itself. */
void hashmap_freeall(
    hashmap_t * hmap
)
{
    assert(hmap);
    hashmap_free(hmap);
    free(hmap);
}

inline static unsigned int __doProbe(
    hashmap_t * hmap,
    const void *key
)
{
    return hmap->hash(key) % hmap->arraySize;
}

/**
 * Get this key's value. */
void *hashmap_get(
    hashmap_t * hmap,
    const void *key
)
{
    unsigned int probe;

    hash_node_t *node;

    const void *key2;

    if (0 == hashmap_count(hmap))
        return NULL;

    probe = __doProbe(hmap, key);
    node = &((hash_node_t *) hmap->array)[probe];
    key2 = node->ety.key;

    if (NULL == key2)
    {
        return NULL;    /* this one wasn't assigned */
    }
    else
    {
        while (node)
        {
            key2 = node->ety.key;
            if (0 == hmap->compare(key, key2))
            {
                assert(node->ety.val);
                return (void *) node->ety.val;
            }
            node = node->next;
        }
    }

    return NULL;
}

/**
 * Is this key inside this map? */
int hashmap_contains_key(
    hashmap_t * hmap,
    const void *key
)
{
    return (NULL != hashmap_get(hmap, key));
}

/**
 * Remove the value refrenced by this key from the hashmap. */
void hashmap_remove_entry(
    hashmap_t * hmap,
    hash_entry_t * entry,
    const void *key
)
{
    unsigned int probe;

    hash_node_t *node;

    void *key2;

    assert(key);

    /* it's nice if we have guaranteed success */

    probe = __doProbe(hmap, key);
    node = &((hash_node_t *) hmap->array)[probe];
    key2 = node->ety.key;

    assert(0 <= hmap->count);

    if (!key2)
    {

    }
    else if (0 == hmap->compare(key, key2))
    {
        memcpy(entry, &node->ety, sizeof(hash_entry_t));

        /* if we forget about collisions we will suffer */
        /* work linked list */
        if (node->next)
        {
            hash_node_t *tmp = node->next;

            memcpy(&node->ety, &tmp->ety, sizeof(hash_entry_t));
            node->next = tmp->next;
            __freenodes(hmap, 1, tmp);
        }
        else
        {
            /* I'm implying that pointing towards NULL is a bottleneck */
            node->ety.key = NULL;
            node->ety.val = NULL;
        }
        hmap->count--;
        return;
    }
    else
    {
        hash_node_t *node_parent;

        node_parent = node;
        node = node->next;

        /* check chain */
        while (node)
        {
            assert(node->ety.key);
            key2 = node->ety.key;

            if (0 == hmap->compare(key, key2))
            {
                memcpy(entry, &node->ety, sizeof(hash_entry_t));
                /* do a node splice */
                node_parent->next = node->next;
                free(node);
                hmap->count--;
                return;
            }

            node_parent = node;
            node = node->next;
        }
    }

    /* only gets here if it doesn't exist */
    entry->key = NULL;
    entry->val = NULL;
}

/**
 * Remove this key and value from the map.
 * @return value of key, or NULL on failure */
void *hashmap_remove(
    hashmap_t * hmap,
    const void *key
)
{
    hash_entry_t entry;

    hashmap_remove_entry(hmap, &entry, key);
    return (void *) entry.val;
}

inline static void __nodeassign(
    hashmap_t * hmap,
    hash_node_t * node,
    void *key,
    void *val
)
{
    hmap->count++;
    assert(hmap->count < 32768);
    node->ety.key = key;
    node->ety.val = val;
}

/**
 * Associate key with val.
 * Does not insert key if an equal key exists.
 * @return previous associated val, otherwise null */
void *hashmap_put(
    hashmap_t * hmap,
    void *key,
    void *val
)
{
    hash_node_t *node;

    void *key2;

    assert(key && val);

    __ensurecapacity(hmap);

    node = &((hash_node_t *) hmap->array)[__doProbe(hmap, key)];

    assert(node);
    key2 = node->ety.key;

    /* this one wasn't assigned */
    if (NULL == key2)
    {
        __nodeassign(hmap, node, key, val);
    }
    else
    {
        /* check the linked list */
        do
        {
            /* replacing val */
            if (0 == hmap->compare(key, key2))
            {
                void *val_prev = node->ety.val;

                node->ety.val = val;
                return val_prev;
            }
            key2 = node->ety.key;
        }
        while (node->next && (node = node->next));
        node->next = __allocnodes(hmap, 1);
        __nodeassign(hmap, node->next, key, val);
    }

    return NULL;
}

/**
 * Put this key/value entry into the hashmap */
void hashmap_put_entry(
    hashmap_t * hmap,
    hash_entry_t * entry
)
{
    hashmap_put(hmap, entry->key, entry->val);
}

// FIXME: make a chain node reservoir
/**
 * Allocate memory for nodes. used for chained nodes. */
static hash_node_t *__allocnodes(
    hashmap_t * hmap,
    int count
)
{
    return calloc(count, sizeof(hash_node_t));
}

static void __ensurecapacity(
    hashmap_t * hmap
)
{
    hash_node_t *array_old;

    int ii, asize_old;

    if ((float) hmap->count / hmap->arraySize < SPACERATIO)
        return;

    /*  stored old array */
    array_old = hmap->array;
    asize_old = hmap->arraySize;

    /*  double array capacity */
    hmap->arraySize *= 2;
    hmap->array = __allocnodes(hmap, hmap->arraySize);
    hmap->count = 0;

    for (ii = 0; ii < asize_old; ii++)
    {
        hash_node_t *node = &((hash_node_t *) array_old)[ii];

        /*  if key is null */
        if (NULL == node->ety.key)
            continue;

        hashmap_put(hmap, node->ety.key, node->ety.val);

        /* re-add chained hash nodes */
        node = node->next;

        while (node)
        {
            hash_node_t *next = node->next;

            hashmap_put(hmap, node->ety.key, node->ety.val);

            assert(NULL != node->ety.key);

            free(node);
            node = next;
        }
    }

    free(array_old);
}

/**
 * Iterate to the next item on a hashmap iterator
 * @return next item from iterator */
void *hashmap_iterator_next(
    hashmap_t * hmap,
    hashmap_iterator_t * iter
)
{
    hash_node_t *node;

    node = iter->cur_linked;

    /* if we have a node ready to look at on the chain.. */
    if (node)
    {
        iter->cur_linked = node->next;
        return node->ety.key;
    }
    /*  otherwise check if we have a node to look at */
    else
    {
        for (; iter->cur < hmap->arraySize; iter->cur++)
        {
            node = &((hash_node_t *) hmap->array)[iter->cur];

            if (node->ety.val)
                break;
        }

        /*  exit if we are at the end */
        if (hmap->arraySize == iter->cur)
        {
            return NULL;
        }

        node = &((hash_node_t *) hmap->array)[iter->cur];

        if (node->next)
        {
            iter->cur_linked = node->next;
        }

        iter->cur += 1;
        return node->ety.key;
    }
}

/**
 * initialise a new hashmap iterator over this hashmap */
void hashmap_iterator(
    hashmap_t * hmap,
    hashmap_iterator_t * iter
)
{
    iter->cur = 0;
    iter->cur_linked = NULL;
}

/*--------------------------------------------------------------79-characters-*/
