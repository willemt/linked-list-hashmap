typedef unsigned int (
    *func_inthash_f
)   (
    const void *
);

typedef int (
    *func_intcmp_f
)   (
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
    func_inthash_f hash;
    func_intcmp_f compare;
} hashmap_t;

typedef struct
{
    int cur;
    void *cur_linked;
} hashmap_iterator_t;

hashmap_t *hashmap_new(
    func_inthash_f hash,
    func_intcmp_f cmp
);

int hashmap_count(
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
