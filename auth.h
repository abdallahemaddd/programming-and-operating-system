#ifndef AUTH_H
#define AUTH_H

typedef struct {
    char username[50];
    char password[50];
    int level;
} User;

int authenticate_user(const char *username, const char *password, int *level);

#endif