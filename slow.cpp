#include "include/hash_table_slow.hpp"
#include <cstdio>

const int MAX_LINE = 100;

struct double_word
{
    const char* primary_word;
    const char* translated_word;
};

size_t get_file_size(FILE* file)
{
    size_t size = 0;
    fseek(file, 0, SEEK_END);
    size = ftell(file);
    fseek(file, 0, SEEK_SET);

    return size;
}

size_t get_eol_count(const char* buffer, size_t file_size)
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

double_word* parser(char* buffer, size_t eol_count, size_t file_size)
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

bool main_test(HashTable *hash_table, double_word *translates, size_t eol_count)
{
    for (size_t i = 0; i < eol_count; i++)
    {    
        if (*(get(hash_table, translates[i].primary_word)) != translates[i].translated_word)
        {
            printf("PRIMARY:%s\nSHOULD:%s\nGIVE:%s\n", translates[i].primary_word, translates[i].translated_word, *(get(hash_table, translates[i].primary_word)));
            return false;
        }
    }
    return true;
}

bool translator_handler(HashTable *hash_table)
{
    char input[MAX_LINE + 1] = {0};

    fgets(input, MAX_LINE, stdin);
    *strchr(input, '\n') = '\0';

    if (!strcmp(input, "EXIT")) return false;

    const char** result = get(hash_table, input);

    if (result == NULL)
        printf("NULL ptr return\n");
    else
        printf("%s\n", *result);

    return true;
}

void get_collisions(HashTable *hash_table)
{
    FILE *file = fopen("collisions.txt", "wb");
    for (int i = 0; i < hash_table->capacity; i++)
        fprintf(file, "%i: %i\n", i, hash_table->buckets[i].get_size());
    fclose(file);
}

void speed_test(HashTable *hash_table, double_word *translates, size_t words_count)
{
    for (int j = 0; j < 1000; j++)
    for (size_t i = 0; i < words_count; i++)
            get(hash_table, translates[i].primary_word);
}

int main()
{
    FILE* file = fopen("src/new_dict.dic", "rb");
    size_t file_size = get_file_size(file);

    char *buffer = (char *)calloc(file_size + 1, sizeof(char));
    fread(buffer, sizeof(char), file_size, file);
    fclose(file);
    
    size_t eol_count = get_eol_count(buffer, file_size);
    double_word *translates = parser(buffer, eol_count, file_size);

    HashTable hash_table;
    construct(&hash_table, 100000);


    for (int i = 0; i < eol_count; i++)
        put(&hash_table, translates[i].primary_word, translates[i].translated_word);

    speed_test(&hash_table, translates, eol_count);

    free(translates);
    free(buffer);
    destruct(&hash_table);
}
