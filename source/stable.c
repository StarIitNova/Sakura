#include "stable.h"

#include <stdlib.h>

struct SakuraTTable *sakuraX_initializeTTable() {
    struct SakuraTTable *table = malloc(sizeof(struct SakuraTTable));
    if (table == NULL) {
        printf("Error: failed to allocate memory for table\n");
        exit(1);
    }

    table->hashPart = calloc(8, sizeof(struct TTableHashEntry));
    if (table->hashPart == NULL) {
        printf("Error: failed to allocate memory for table\n");
        free(table);
        exit(1);
    }

    table->capacity = 8;
    table->size = 0;

    return table;
}

void sakuraX_resizeTTable(struct SakuraTTable *table, size_t newCapacity) {
    struct TTableHashEntry **newHashPart = calloc(newCapacity, sizeof(struct TTableHashEntry));
    if (newHashPart == NULL) {
        printf("Error: failed to allocate memory for table\n");
        exit(1);
    }

    for (size_t i = 0; i < table->capacity; i++) {
        struct TTableHashEntry *entry = table->hashPart[i];
        if (entry != NULL) {
            struct TTableHashEntry *next = entry->next;

            unsigned int newIdx = sakuraX_hashTValue(&entry->key, newCapacity);
            entry->next = newHashPart[newIdx];
            newHashPart[newIdx] = entry;

            entry = next;
        }
    }

    free(table->hashPart);
    table->hashPart = newHashPart;
    table->capacity = newCapacity;
}

void sakuraX_setTTable(struct SakuraTTable *table, const TValue *key, const TValue *value) {
    size_t hashIdx = sakuraX_hashTValue(key, table->capacity);

    struct TTableHashEntry *entry = table->hashPart[hashIdx];
    while (entry != NULL) {
        if (sakuraX_compareTValues(&entry->key, key)) {
            entry->value = *value;
            return;
        }

        entry = entry->next;
    }

    struct TTableHashEntry *newEntry = malloc(sizeof(struct TTableHashEntry));
    if (newEntry == NULL) {
        printf("Error: failed to allocate memory for table\n");
        exit(1);
    }

    newEntry->key = *key;
    newEntry->value = *value;
    newEntry->next = table->hashPart[hashIdx];
    table->hashPart[hashIdx] = newEntry;
    table->size++;

    if ((double)table->size / table->capacity > 0.75) {
        size_t newCapacity = table->capacity * 2;
        sakuraX_resizeTTable(table, newCapacity);
    }
}

TValue sakuraX_getTTable(struct SakuraTTable *table, const TValue *key) {
    size_t hashIdx = sakuraX_hashTValue(key, table->capacity);

    struct TTableHashEntry *entry = table->hashPart[hashIdx];
    while (entry != NULL) {
        if (sakuraX_compareTValues(&entry->key, key))
            return entry->value;

        entry = entry->next;
    }

    TValue defaultValue = {SAKURA_TNIL, {.nil = 1}};
    return defaultValue;
}

void sakuraX_freeTTable(struct SakuraTTable *table) {
    for (size_t i = 0; i < table->capacity; ++i) {
        struct TTableHashEntry *entry = table->hashPart[i];
        while (entry != NULL) {
            struct TTableHashEntry *next = entry->next;
            free(entry);
            entry = next;
        }
    }

    free(table->hashPart);
    free(table);
}