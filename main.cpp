#include "include/hash_table.hpp"
#include <cstdio>
#include <SFML/Graphics.hpp>
#include <cassert>

const size_t MAX_LINE = 100;

#ifndef SPEED_TEST_COUNT 
#define SPEED_TEST_COUNT 1000
#endif

struct DoubleWord
{
    const char* primary_word;
    const char* translated_word;
};

//-----------------------------------------------------------------------------

size_t GetFileSize(FILE* file)
{
    size_t size = 0;
    fseek(file, 0, SEEK_END);
    size = ftell(file);
    fseek(file, 0, SEEK_SET);

    return size;
}

//-----------------------------------------------------------------------------

size_t GetEolCount(const char* buffer, size_t buffer_size)
{
    assert(buffer != NULL);

    size_t eol_count = 0;
    const char* ptr  = buffer;

    while((size_t)(ptr - buffer) < buffer_size)
    {
        if (*ptr == '\n') eol_count++;
        ptr++;
    }
    
    return eol_count;
}

//-----------------------------------------------------------------------------

DoubleWord* Parser(char* buffer, size_t eol_count, size_t file_size)
{
    DoubleWord* translates = (DoubleWord *)calloc(eol_count, sizeof(DoubleWord));
    char* ptr = buffer;
    size_t curr_word = 1;

    translates[0].primary_word = buffer;

    while((size_t)(ptr - buffer) < file_size)
    {
        if (*ptr == '=')
        {
            *ptr = '\0';
            translates[curr_word - 1].translated_word = ptr + 1;
        }
        if (*ptr == '\n'){
            *ptr = '\0';
            if (curr_word < eol_count)
                translates[curr_word++].primary_word = ptr + 1;
        }
        ptr++;
    }

    return translates;
}

//-----------------------------------------------------------------------------

bool MainTest(HashTable *hash_table, DoubleWord *translates, size_t eol_count)
{
    for (size_t i = 0; i < eol_count; i++)
    {    
        const char** get_translate = HashTable_get(hash_table, translates[i].primary_word);

        if (get_translate == NULL || *get_translate != translates[i].translated_word)
        {
            printf("TEST HASN'T PASSED\n"
                  "PRIMARY:%s\n"
                  "TRANSLATE:%s\n",
                  translates[i].primary_word, 
                  translates[i].translated_word);
            
            if (get_translate == NULL)
                printf("GIVEN:NULL\n");
            else
                printf("GIVEN:%s\n", *get_translate);

            return false;
        }
    }

    printf("TEST HAS PASSED\n");
    return true;
}

//-----------------------------------------------------------------------------

bool DictionaryHandler(HashTable *hash_table)
{
    char input[MAX_LINE + 1] = {0};
    fgets(input, MAX_LINE, stdin);
    
    char* eol = strchr(input, '\n');
    if (eol) *eol = '\0'; 

    if (!strcmp(input, "EXIT")) return false;

    const char** get_translate = HashTable_get(hash_table, input);

    if (get_translate == NULL) printf("NULL\n");
    else                       printf("%s\n", *get_translate);

    return true;
}

//-----------------------------------------------------------------------------

void PrintCollisions(HashTable *hash_table)
{
    FILE *file = fopen("collisions.txt", "wb");

    for (size_t i = 0; i < hash_table->capacity; i++)
        fprintf(file, "%zu: %zu\n", i, hash_table->buckets[i].get_size());
    
    fclose(file);
}

//-----------------------------------------------------------------------------

int SpeedTest(HashTable *hash_table, DoubleWord *translates, size_t words_count)
{
    const char **get_translate = NULL;

    for (int j = 0; j < SPEED_TEST_COUNT; j++)
    for (size_t i = 0; i < words_count;   i++)
            if (!(get_translate = HashTable_get(hash_table, translates[i].primary_word))) return 1;
    
    return 0;
}

//-----------------------------------------------------------------------------

size_t ReadDataBase(const char* file_name, char** buffer)
{
    assert(file_name != NULL);
    assert(buffer    != NULL);

    FILE* file = fopen(file_name, "rb");
    if (file == NULL) return 0;

    size_t file_size = GetFileSize(file);
    *buffer = (char *)calloc(file_size + 1, sizeof(char));
    fread(*buffer, sizeof(char), file_size, file);
    fclose(file);

    return file_size;
}

//-----------------------------------------------------------------------------

void GetGraph(const char* dictionary_path)
{
    assert(dictionary_path != NULL);

    HashTable hash_table = {};

    FILE* plot_file = fopen("plot.txt", "wb");

    for (float load_factor = 1; load_factor >= 0.2; load_factor -= 0.005)
    {  
        char *buffer = NULL;
        size_t buffer_size = ReadDataBase(dictionary_path, &buffer);
        if (buffer == NULL) 
        {
            printf("Couldn't read database\n");
            return;
        }

        size_t words_count     = GetEolCount(buffer, buffer_size);
        DoubleWord *translates = Parser(buffer, words_count, buffer_size);
        
        HashTable_construct(&hash_table, words_count / load_factor + 1);

        for (size_t i = 0; i < words_count; i++)
            HashTable_put(&hash_table, translates[i].primary_word, translates[i].translated_word);

        clock_t start = clock();
        SpeedTest(&hash_table, translates, words_count);
        clock_t end = clock();
        fprintf(plot_file, "%f %i\n", load_factor, ((1000 * (end - start))) / CLOCKS_PER_SEC);

        HashTable_destruct(&hash_table);
        free(buffer);
        free(translates);
    }
    fclose(plot_file);
}

//-----------------------------------------------------------------------------

int main()
{

#ifdef PLOT
    GetGraph("src/dictionary.dic");

    return 0;
#else

    char *buffer = NULL;
    size_t buffer_size = ReadDataBase("src/dictionary.dic", &buffer);
    if (buffer == NULL)
    {
        printf("Couldn't read database\n");
        return 0;
    }
    
    size_t      words_count = GetEolCount(buffer, buffer_size);
    DoubleWord *translates  = Parser(buffer, words_count, buffer_size);
    
    HashTable hash_table = {};
    HashTable_construct(&hash_table, 100);

    for (size_t i = 0; i < words_count; i++)
        HashTable_put(&hash_table, translates[i].primary_word, translates[i].translated_word);

#ifdef SPEED_TEST
    int pls_dont_optimize = SpeedTest(&hash_table, translates, words_count);
    if (pls_dont_optimize) printf("Get returned NULL in Speed test\n");
#elif  MAIN_TEST
    MainTest(&hash_table, translates, words_count);
#else  
    while (DictionaryHandler(&hash_table)) {}
#endif

    free(translates);
    free(buffer);
    HashTable_destruct(&hash_table);

    return 0;

#endif
}
