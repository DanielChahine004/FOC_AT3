#ifndef ENCRYPTION_H
#define ENCRYPTION_H

// Include necessary standard library headers
#include <stdio.h>

// Function declarations
int encrypt_file_in_database(const char* filename, const char* key);
int decrypt_file_from_database(const char* filename, const char* key);

#endif // ENCRYPTION_H