#include <cstdio>

#define SLOW
#include "include/hash_table.hpp"
#undef SLOW

const int MAX_LINE = 100;

struct double_word
{
    const char* primary_word;
    const char* translated_word;
};

size_t GetFileSize(FILE* file)
{
    size_t size = 0;
    fseek(file, 0, SEEK_END);
    size = ftell(file);
    fseek(file, 0, SEEK_SET);

    return size;
}

size_t GetEolCount(const char* buffer, size_t file_size)
{
    size_t eol_count = 0;

    const char* ptr = buffer;

    while(ptr - buffer < file_size)
    {
        if (*ptr == '\n') eol_count++;
        ptr++;
    }

    return eol_count;
}

double_word* Parser(char* buffer, size_t eol_count, size_t file_size)
{
    double_word* translates = (double_word *)calloc(eol_count, sizeof(double_word));

    char* ptr = buffer;

    int beg_cnt = 1;
    int end_cnt = 0;

    translates[0].primary_word = buffer;

    while(ptr - buffer < file_size)
    {
        if (*ptr == '=')
        {
            *ptr = '\0';
            translates[beg_cnt - 1].translated_word = ptr + 1;
        }
        if (*ptr == '\n' && beg_cnt < eol_count){
            *ptr = '\0';
            translates[beg_cnt++].primary_word = ptr + 1;
        }
        ptr++;
    }

    return translates;
}

bool MainTest(HashTable *hash_table, double_word *translates, size_t eol_count)
{
    for (size_t i = 0; i < eol_count; i++)
    {    
        if (*(HashTable_get(hash_table, translates[i].primary_word)) != translates[i].translated_word)
        {
            printf("PRIMARY:%s\nSHOULD:%s\nGIVE:%s\n", translates[i].primary_word, translates[i].translated_word,
                                                       *(HashTable_get(hash_table, translates[i].primary_word)));
            return false;
        }
    }
    return true;
}

bool DictionaryHandler(HashTable *hash_table)
{
    char input[MAX_LINE + 1] = {0};

    fgets(input, MAX_LINE, stdin);
    *strchr(input, '\n') = '\0';

    if (!strcmp(input, "EXIT")) return false;

    const char** result = HashTable_get(hash_table, input);

    if (result == NULL)
        printf("NULL ptr return\n");
    else
        printf("%s\n", *result);

    return true;
}

void PrintCollisions(HashTable *hash_table)
{
    FILE *file = fopen("collisions.txt", "wb");
    for (int i = 0; i < hash_table->capacity; i++)
        fprintf(file, "%i: %i\n", i, hash_table->buckets[i].get_size());
    fclose(file);
}

void SpeedTest(HashTable *hash_table, double_word *translates, size_t words_count)
{
    for (int j = 0; j < 1000; j++)
    for (size_t i = 0; i < words_count; i++)
            HashTable_get(hash_table, translates[i].primary_word);
}

int main()
{
    FILE* file = fopen("src/new_dict.dic", "rb");
    size_t file_size = GetFileSize(file);

    char *buffer = (char *)calloc(file_size + 1, sizeof(char));
    fread(buffer, sizeof(char), file_size, file);
    fclose(file);
    
    size_t eol_count = GetEolCount(buffer, file_size);
    double_word *translates = Parser(buffer, eol_count, file_size);

    HashTable hash_table;
    HashTable_construct(&hash_table, 100000);

    for (int i = 0; i < eol_count; i++)
        HashTable_put(&hash_table, translates[i].primary_word, translates[i].translated_word);

    SpeedTest(&hash_table, translates, eol_count);

    free(translates);
    free(buffer);
    HashTable_destruct(&hash_table);
}
