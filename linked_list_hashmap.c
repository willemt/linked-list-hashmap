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
#define SPACERATIO 0.9

typedef struct hash_node_s hash_node_t;

struct hash_node_s
{
    hash_entry_t ety;
    hash_node_t *next;
};

static void __ensurecapacity(
    hashmap_t * hmap
);

/**
 * Allocate memory for nodes. Used for chained nodes. */
static hash_node_t *__allocnodes(
    unsigned int count
)
{
    // FIXME: make a chain node reservoir
    return calloc(count, sizeof(hash_node_t));
}

/*----------------------------------------------------------------------------*/

hashmap_t *hashmap_new(
    func_longhash_f hash,
    func_longcmp_f cmp,
    unsigned int initial_capacity
)
{
    hashmap_t *hmap;

    hmap = calloc(1, sizeof(hashmap_t));
    hmap->arraySize = initial_capacity;
    hmap->array = __allocnodes(hmap->arraySize);
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
 * @return size of the array used within hashmap */
int hashmap_size(
    hashmap_t * hmap
)
{
    return hmap->arraySize;
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
 * Get this key's value.
 * @return key's item, otherwise NULL */
void *hashmap_get(
    hashmap_t * hmap,
    const void *key
)
{
    unsigned int probe;

    hash_node_t *node;

    if (0 == hashmap_count(hmap))
        return NULL;

    probe = __doProbe(hmap, key);
    node = &((hash_node_t *) hmap->array)[probe];
    
    if (NULL == node->ety.key)
    {
        return NULL;    /* this one wasn't assigned */
    }
    else
    {
        /* iterate down the node's linked list chain */
        while (node)
        {
            if (0 == hmap->compare(key, node->ety.key))
            {
                return (void *) node->ety.val;
            }
            node = node->next;
        }
    }

    return NULL;
}

/**
 * Is this key inside this map?
 * @return 1 if key is in hashmap, otherwise 0 */
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
    hash_node_t *node, *node_parent;

    node = &((hash_node_t *) hmap->array)[__doProbe(hmap, key)];

    if (!node->ety.key)
        goto notfound;

    node_parent = NULL;

    do {
        if (0 != hmap->compare(key, node->ety.key))
        {
            /* does not match, lets traverse the chain.. */
            node_parent = node;
            node = node->next;
            continue;
        }

        memcpy(entry, &node->ety, sizeof(hash_entry_t));

        /* I am a root node on the array */
        if (!node_parent)
        {
            /* I have a node on my chain. This node will replace me */
            if (node->next)
            {
                hash_node_t *tmp;
                
                tmp = node->next;
                memcpy(&node->ety, &tmp->ety, sizeof(hash_entry_t));
                /* Replace me with my next on chain */
                node->next = tmp->next;
                free(tmp);
            }
            else
            {
                /* un-assign */
                node->ety.key = NULL;
            }
        }
        else
        {
            /* Replace me with my next on chain */
            node_parent->next = node->next;
            free(node);
        }

        hmap->count--;
        return;

    } while (node);
   
notfound:
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
    void *val_new
)
{
    hash_node_t *node;

    assert(key);
    assert(val_new);

    __ensurecapacity(hmap);

    node = &((hash_node_t *) hmap->array)[__doProbe(hmap, key)];

    assert(node);

    /* this one wasn't assigned */
    if (NULL == node->ety.key)
    {
        __nodeassign(hmap, node, key, val_new);
    }
    else
    {
        /* check the linked list */
        do
        {
            /* if same key, then we are just replacing val */
            if (0 == hmap->compare(key, node->ety.key))
            {
                void *val_prev;
                
                val_prev = node->ety.val;
                node->ety.val = val_new;
                return val_prev;
            }
        }
        while (node->next && (node = node->next));

        node->next = __allocnodes(1);
        __nodeassign(hmap, node->next, key, val_new);
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

/**
 * Increase hashmap capacity.
 * @param factor : increase by this factor */
void hashmap_increase_capacity(
    hashmap_t * hmap,
    unsigned int factor)
{
    hash_node_t *array_old;

    int ii, asize_old;

    /*  stored old array */
    array_old = hmap->array;
    asize_old = hmap->arraySize;

    /*  double array capacity */
    hmap->arraySize *= factor;
    hmap->array = __allocnodes(hmap->arraySize);
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

static void __ensurecapacity(
    hashmap_t * hmap
)
{
    if ((float) hmap->count / hmap->arraySize < SPACERATIO)
    {
        return;
    }
    else
    {
        hashmap_increase_capacity(hmap,2);
    }
}

int hashmap_iterator_has_next(
    hashmap_t * hmap,
    hashmap_iterator_t * iter
)
{
    if (NULL == iter->cur_linked)
    {
        for (; iter->cur < hmap->arraySize; iter->cur++)
        {
            hash_node_t *node;

            node = &((hash_node_t *) hmap->array)[iter->cur];
            if (node->ety.val)
                break;
        }

        if (iter->cur == hmap->arraySize)
        {
            return 0;
        }
    }

    return 1;
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
    hashmap_t * hmap __attribute__((__unused__)),
    hashmap_iterator_t * iter
)
{
    iter->cur = 0;
    iter->cur_linked = NULL;
}

/*--------------------------------------------------------------79-characters-*/
