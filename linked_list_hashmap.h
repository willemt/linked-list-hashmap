typedef unsigned long (
    *func_longhash_f
)    (
    const void *
);

typedef long (
    *func_longcmp_f
)    (
    const void *,
    const void *
);

typedef struct
{
    void *key;                  // the refrence 
    void *val;                  // the data
} hash_entry_t;

typedef struct
{
    int count;
    int arraySize;
    void *array;
    func_longhash_f hash;
    func_longcmp_f compare;
} hashmap_t;

typedef struct
{
    int cur;
    void *cur_linked;
} hashmap_iterator_t;

hashmap_t *hashmap_new(
    func_longhash_f hash,
    func_longcmp_f cmp,
    unsigned int initial_capacity
);

int hashmap_count(
    hashmap_t * hmap
);

int hashmap_size(
    hashmap_t * hmap
);

void hashmap_clear(
    hashmap_t * hmap
);

void hashmap_free(
    hashmap_t * hmap
);

void hashmap_freeall(
    hashmap_t * hmap
);

void *hashmap_get(
    hashmap_t * hmap,
    const void *key
);

int hashmap_contains_key(
    hashmap_t * hmap,
    const void *key
);

void hashmap_remove_entry(
    hashmap_t * hmap,
    hash_entry_t * entry,
    const void *key
);

void *hashmap_remove(
    hashmap_t * hmap,
    const void *key
);

void *hashmap_put(
    hashmap_t * hmap,
    void *key,
    void *val
);

void hashmap_put_entry(
    hashmap_t * hmap,
    hash_entry_t * entry
);

void *hashmap_iterator_next(
    hashmap_t * hmap,
    hashmap_iterator_t * iter
);

void hashmap_iterator(
    hashmap_t * hmap,
    hashmap_iterator_t * iter
);

void hashmap_increase_capacity(
    hashmap_t * hmap,
    unsigned int factor);
