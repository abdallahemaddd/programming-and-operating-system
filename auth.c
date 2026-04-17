#include <string.h>
#include "auth.h"

User users[] = {
    {"ali", "123", 1},
    {"sara", "456", 2},
    {"admin", "789", 3}
};

int authenticate_user(const char *username, const char *password, int *level) {
    int total = sizeof(users) / sizeof(users[0]);

    for(int i = 0; i < total; i++) {
        if(strcmp(users[i].username, username) == 0 &&
           strcmp(users[i].password, password) == 0) {
            *level = users[i].level;
            return 1;
        }
    }
    return 0;
}