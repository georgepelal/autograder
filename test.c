//This is an example program generated from chatgpt for testing the autograder

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_WORD_LEN 100
#define HASH_SIZE 1009  // Prime number for hash table size

typedef struct WordEntry {
    char *word;
    int count;
    struct WordEntry *next;
} WordEntry;

WordEntry *hash_table[HASH_SIZE] = { NULL };

unsigned int hash(const char *str) {
    unsigned int h = 0;
    while (*str) {
        h = (h * 31) + tolower(*str++);
    }
    return h % HASH_SIZE;
}

void insert_word(const char *word) {
    unsigned int h = hash(word);
    WordEntry *entry = hash_table[h];

    while (entry) {
        if (strcasecmp(entry->word, word) == 0) {
            entry->count++;
            return;
        }
        entry = entry->next;
    }

    entry = malloc(sizeof(WordEntry));
    entry->word = strdup(word);
    entry->count = 1;
    entry->next = hash_table[h];
    hash_table[h] = entry;
}

void free_table() {
    for (int i = 0; i < HASH_SIZE; ++i) {
        WordEntry *entry = hash_table[i];
        while (entry) {
            WordEntry *tmp = entry;
            entry = entry->next;
            free(tmp->word);
            free(tmp);
        }
    }
}

int is_word_char(char c) {
    return isalpha(c);
}

typedef struct {
    char *word;
    int count;
} WordCount;

int compare_wordcount(const void *a, const void *b) {
    WordCount *wa = (WordCount *)a;
    WordCount *wb = (WordCount *)b;
    if (wa->count != wb->count)
        return wb->count - wa->count;  // Descending frequency
    return strcasecmp(wa->word, wb->word);  // Alphabetical
}

void collect_entries(WordCount *array, int *size) {
    for (int i = 0; i < HASH_SIZE; ++i) {
        WordEntry *entry = hash_table[i];
        while (entry) {
            array[*size].word = entry->word;
            array[*size].count = entry->count;
            (*size)++;
            entry = entry->next;
        }
    }
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <input_file> <top_N>\n", argv[0]);
        return 1;
    }

    FILE *file = fopen(argv[1], "r");
    if (!file) {
        perror("Failed to open input file");
        return 1;
    }

    int top_n = atoi(argv[2]);
    char c, buffer[MAX_WORD_LEN];
    int idx = 0;

    while ((c = fgetc(file)) != EOF) {
        if (is_word_char(c)) {
            if (idx < MAX_WORD_LEN - 1)
                buffer[idx++] = tolower(c);
        } else if (idx > 0) {
            buffer[idx] = '\0';
            insert_word(buffer);
            idx = 0;
        }
    }
    if (idx > 0) {
        buffer[idx] = '\0';
        insert_word(buffer);
    }

    fclose(file);

    // Collect and sort results
    WordCount entries[HASH_SIZE * 2];
    int count = 0;
    collect_entries(entries, &count);
    qsort(entries, count, sizeof(WordCount), compare_wordcount);

    if (top_n > count) top_n = count;
    for (int i = 0; i < top_n; ++i) {
        printf("%s %d\n", entries[i].word, entries[i].count);
    }
    free_table();
    return 0;
}
