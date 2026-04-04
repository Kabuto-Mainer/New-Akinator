#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <ctype.h>
#include <sys/stat.h>
#include <unistd.h>

#include "GK_AllFunc.h"

// ------------------------------------------------------------------
int gk_get_file_size(const char *name) {
    assert(name);

    struct stat file_stat = {};
    if (stat(name, &file_stat) == -1)   ExitF("Bad Stat", 0);

    return (int) file_stat.st_size;
}

// ------------------------------------------------------------------
char *gk_create_file_buffer(const char *name, int size) {
    assert(name);
    assert(size >= 0);

    FILE *stream = fopen(name, "rb");
    if (stream == NULL) ExitF("NULL File", NULL);

    char *buffer = (char *)calloc((size_t)size + 1, sizeof(char));
    if (buffer == NULL) {
        fclose(stream);
        ExitF("NULL Calloc", NULL);
    }

    fread(buffer, sizeof(char), (size_t)size, stream);
    fclose(stream);

    return buffer;
}
