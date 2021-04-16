#include <cstdio>
#include <cstdlib>
#include <cstring>

size_t get_file_size(FILE* file)
{
    size_t size = 0;
    fseek(file, 0, SEEK_END);
    size = ftell(file);
    fseek(file, 0, SEEK_SET);

    return size;
}

size_t get_eol_count(const char* buffer)
{
    size_t eol_count = 0;

    while(*buffer != '\0')
    {
        if (*buffer == '\n') eol_count++;
        buffer++;
    }

    return eol_count;
}

int main()
{
    FILE* file = fopen("src/dict.dic", "rb");
    size_t file_size = get_file_size(file);

    char *buffer = (char *)calloc(file_size + 1, sizeof(char));
    fread(buffer, sizeof(char), file_size, file);
    fclose(file);

    char* new_buffer = (char *)calloc(file_size * 8 + 1, sizeof(char));
    size_t eol_count = get_eol_count(buffer);
    
    char* buf_ptr = buffer;
    char* new_buf_ptr = new_buffer;

    char* eq = strchr(buf_ptr, '=');
    file = fopen("src/new_dict.dic", "wb");

    for (int i = 0; i < eol_count; i++)
    {
        memcpy(new_buf_ptr, buf_ptr, eq - buf_ptr);

        new_buf_ptr += eq - buf_ptr;

        for (int j = 0; j < (8 - (eq - buf_ptr) % 8) % 8; j++)
            *(new_buf_ptr++) = 0;
        
        *new_buf_ptr++ = '=';
        buf_ptr = eq + 1;

        eq = strchr(buf_ptr, '\n');
        memcpy(new_buf_ptr, buf_ptr, eq - buf_ptr);
        new_buf_ptr += eq - buf_ptr;
        
        for (int j = 0; j < (8 - (eq - buf_ptr) % 8) % 8; j++)
            *(new_buf_ptr++) = 0;
        
        *new_buf_ptr++ = '\n';
        buf_ptr = eq + 1;

        eq = strchr(buf_ptr, '=');
    }

    printf("%i\n", new_buf_ptr - new_buffer);

    fwrite(new_buffer, sizeof(char), new_buf_ptr - new_buffer, file);
    
    fclose(file);
    free(new_buffer);
    free(buffer);
}
