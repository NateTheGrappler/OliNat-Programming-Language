//
// Created by natang on 6/4/26.
//
#include "Hashmap.h"
#include "object.h"

#define TABLE_MAX_LOAD 0.75

void initMap(Hashmap* hashmap)
{
    hashmap->capacity =0;
    hashmap->count    =0;
    hashmap->buckets  = NULL;
}
void freeMap(Hashmap* hashmap)
{
    FREE_ARRAY(Bucket, hashmap->buckets, hashmap->capacity);
}


//helper functions
static Bucket* findBucket(Bucket* buckets, int capacity, ObjString* key)
{
    //modulo over the index
    uint32_t index = key->hash % capacity;

    Bucket* tombStone = NULL;
    for (;;)
    {
        Bucket* bucket = &buckets[index];

        if (bucket->key == NULL)
        {
            //return the empty bucket if youve found it, checking for tombstones
            if (IS_EMPTY(bucket->value))
            {
                return tombStone != NULL ? tombStone : bucket;
            }
            if (tombStone == NULL) tombStone = bucket;
        }
        else if (bucket->key == key)
        {
            //found the key
            return bucket;
        }

        //loop around
        index = (index + 1) % capacity;
    }
}
static void adjustCapacity(Hashmap* hashmap, int capacity)
{
    Bucket* buckets = ALLOCATE(Bucket, capacity);


    //init all buckets as empty
    for (int i = 0; i < capacity; i++)
    {
        buckets[i].key   = NULL;
        buckets[i].value = CREATE_EMPTY_VAL();
    }

    //copy all of the filled buckets from old hashmap allocation
    hashmap->count = 0;
    for (int i = 0; i < hashmap->capacity; i++)
    {
        Bucket* bucket = &hashmap->buckets[i];
        if (bucket->key == NULL) continue; //no empty buckets

        //get the new location and rewrite the data to it
        Bucket* destination = findBucket(buckets, capacity, bucket->key);
        destination->key   = bucket->key;
        destination->value = bucket->value;
        hashmap->count++;
    }

    //free the hold hashmap and update the new one
    FREE_ARRAY(Bucket, hashmap->buckets, hashmap->capacity);
    hashmap->buckets = buckets;
    hashmap->capacity = capacity;
}

//-------------------Hashmap functionality functions----------------//
bool MapSet(Hashmap* hashmap, ObjString* key, Value value)
{
    if (hashmap->count + 1 > hashmap->capacity * TABLE_MAX_LOAD)
    {
        int capacity = GROW_CAPACITY(hashmap->capacity);
        adjustCapacity(hashmap, capacity);
    }

    Bucket* bucket = findBucket(hashmap->buckets, hashmap->capacity, key);

    //update inner metadata for hashmap
    bool isNewKey = bucket->key == NULL;
    if (isNewKey && IS_EMPTY(bucket->value)) hashmap->count++;

    //write the actual data
    bucket->key = key;
    bucket->value = value;
    return isNewKey;


}
bool MapGet(Hashmap* hashmap, ObjString* key, Value* value)
{
    if (hashmap->count == 0) return false;

    Bucket* bucket = findBucket(hashmap->buckets, hashmap->capacity, key);
    if (bucket->key == NULL) return false;

    *value = bucket->value;
    return true;
}
bool MapDelete(Hashmap* hashmap, ObjString* key)
{
    if (hashmap->count == 0) return false;

    Bucket* bucket = findBucket(hashmap->buckets, hashmap->capacity, key);
    if (bucket->key == NULL) return false;

    //set it as a tombstone
    bucket->key = NULL;
    bucket->value = CREATE_BOOL_VAL(true);
    return true;
}


//Interning strings
ObjString* hashmapFindString(Hashmap* hashmap, const char* chars, int length, uint32_t hash)
{
    if (hashmap->count == 0) return NULL;

    uint32_t index = hash % hashmap->capacity;

    for (;;)
    {
        Bucket* bucket = &hashmap->buckets[index];

        if (bucket->key == NULL)
        {
            //if empty then return null
            if (IS_EMPTY(bucket->value)) return NULL;
        }
        else if (bucket->key->length == length && bucket->key->hash == hash && memcmp(bucket->key->chars, chars, length) == 0)
        {
            //found the string so just return stringObj pointer
            return bucket->key;
        }

        index = (index+1) % hashmap->capacity; //loop
    }
}
