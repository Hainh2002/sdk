//
// Created by vnbk on 12/04/2023.
//
#include <sm_uuid.h>
#include <stdlib.h>

/**
 *
 * @param str
 * @param size
 * @return
 */
char *sm_rand_string(char *str, int size)
{
    const char charset[] = "abcdefghijklmnopqrstuvwxyz123456789";
    if (size) {
        --size;
        for (size_t n = 0; n < size; n++) {
            int key = rand() % ((int)(sizeof(charset) - 1));
            str[n] = charset[key];
        }
        str[size] = '\0';
    }
    return str;
}

static char g_uuid[37] = {0,};

const char* sm_get_uuid(){
    char v[] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'};
    //3fb17ebc-bc38-4939-bc8b-74f2443281d4
    //8 dash 4 dash 4 dash 4 dash 12
    //gen random for all spaces because lazy
    for(int i = 0; i < 36; ++i) {
        g_uuid[i] = v[rand()%16];
    }

    //put dashes in place
    g_uuid[8] = '-';
    g_uuid[13] = '-';
    g_uuid[18] = '-';
    g_uuid[23] = '-';

    //needs end byte
    g_uuid[36] = '\0';

    return g_uuid;
}
