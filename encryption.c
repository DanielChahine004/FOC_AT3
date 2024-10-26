#include "huffman_compression.h"
#include "encryption.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define DEBUG 0
#define MAX_KEY_LENGTH 256

int encrypt_file_in_database(const char* filename, const char* key) {
    size_t key_len = strlen(key);
    if (key_len == 0 || key_len > MAX_KEY_LENGTH) {
        printf("Invalid key length. Must be between 1 and %d bytes.\n", MAX_KEY_LENGTH);
        return 1;
    }

    // Create input path
    char* inputPath = create_full_path(CLIENT_DATABASE, filename);
    if (!inputPath) {
        printf("Failed to create input path\n");
        return 1;
    }

    // Create output filename (add _encrypted suffix before extension)
    char base_name[242];  // 256 - 14 (length of "_encrypted.bmp")
    char output_name[256];
    
    // Safe copy with limited size
    strncpy(base_name, filename, sizeof(base_name) - 1);
    base_name[sizeof(base_name) - 1] = '\0';
    
    size_t len = strlen(base_name);
    if (len >= 4) {
        base_name[len - 4] = '\0';  // Remove .bmp
    }
    
    // Now we're guaranteed to have enough space
    snprintf(output_name, sizeof(output_name), "%s_encrypted.bmp", base_name);

    // copying output path
    char* outputPath = create_full_path(COMPRESSED_AND_ENCRYPTED_DIRECTORY, output_name);
    if (!outputPath) {
        printf("Failed to create output path\n");
        free(inputPath);
        return 1;
    }

    // output path
    char final_path[512];
    strncpy(final_path, outputPath, sizeof(final_path) - 1);
    final_path[sizeof(final_path) - 1] = '\0';

    FILE* inFile = fopen(inputPath, "rb");
    FILE* outFile = fopen(outputPath, "wb");
    
    if (!inFile || !outFile) {
        printf("Failed to open files\n");
        if (inFile){
            fclose(inFile);
        }
        if (outFile){
            fclose(outFile);
        }
        free(inputPath);
        free(outputPath);
        return 1;
    }

    // buffer for batch processing
    unsigned char buffer[4096];
    unsigned char encrypted_buffer[4096];
    size_t bytes_read;
    size_t key_pos = 0;
    
    while ((bytes_read = fread(buffer, 1, sizeof(buffer), inFile)) > 0) {
        for (size_t i = 0; i < bytes_read; i++) {
            encrypted_buffer[i] = buffer[i] ^ key[key_pos];
            key_pos = (key_pos + 1) % key_len;
            
            if (DEBUG) {
                printf("Original byte: ");
                for (int j = 7; j >= 0; j--) {
                    printf("%d", (buffer[i] >> j) & 1);
                }
                printf(" XOR Key: ");
                for (int j = 7; j >= 0; j--) {
                    printf("%d", (key[key_pos] >> j) & 1);
                }
                printf(" = Encrypted: ");
                for (int j = 7; j >= 0; j--) {
                    printf("%d", (encrypted_buffer[i] >> j) & 1);
                }
                printf("\n");
            }
        }
        
        // Write encrypted buffer to output file
        if (fwrite(encrypted_buffer, 1, bytes_read, outFile) != bytes_read) {
            printf("Failed to write to output file\n");
            fclose(inFile);
            fclose(outFile);
            free(inputPath);
            free(outputPath);
            return 1;
        }
    }

    // Cleanup
    fclose(inFile);
    fclose(outFile);
    free(inputPath);
    free(outputPath);
    
    if (DEBUG) {
        printf("File encrypted successfully: %s\n", final_path);
    }
    return 0;
}

int decrypt_file_from_database(const char* filename, const char* key) {
    size_t key_len = strlen(key);
    if (key_len == 0 || key_len > MAX_KEY_LENGTH) {
        printf("Invalid key length. Must be between 1 and %d bytes.\n", MAX_KEY_LENGTH);
        return 1;
    }

    // Create input path
    char* inputPath = create_full_path(COMPRESSED_AND_ENCRYPTED_DIRECTORY, filename);
    if (!inputPath) {
        printf("Failed to create input path\n");
        return 1;
    }

    // Create output filename (remove _encrypted and add _decrypted)
    char base_name[242];
    char output_name[256];
    
    // Safe copy with limited size
    strncpy(base_name, filename, sizeof(base_name) - 1);
    base_name[sizeof(base_name) - 1] = '\0';
    
    // Remove _encrypted.bmp
    size_t len = strlen(base_name);
    if (len >= 14) {  // length of "_encrypted.bmp"
        base_name[len - 14] = '\0';
    }
    
    // Add _decrypted.bmp
    snprintf(output_name, sizeof(output_name), "%s_decrypted.bmp", base_name);

    // Create output path
    char* outputPath = create_full_path(COMPRESSED_AND_DECRYPTED_DIRECTORY, output_name);
    if (!outputPath) {
        printf("Failed to create output path\n");
        free(inputPath);
        return 1;
    }

    // Store output path for final message
    char final_path[512];
    strncpy(final_path, outputPath, sizeof(final_path) - 1);
    final_path[sizeof(final_path) - 1] = '\0';

    FILE* inFile = fopen(inputPath, "rb");
    FILE* outFile = fopen(outputPath, "wb");
    
    if (!inFile || !outFile) {
        printf("Failed to open files\n");
        if (inFile) fclose(inFile);
        if (outFile) fclose(outFile);
        free(inputPath);
        free(outputPath);
        return 1;
    }

    // buffer for batch processing
    unsigned char buffer[4096];
    unsigned char decrypted_buffer[4096];
    size_t bytes_read;
    size_t key_pos = 0;
    
    while ((bytes_read = fread(buffer, 1, sizeof(buffer), inFile)) > 0) {
        for (size_t i = 0; i < bytes_read; i++) {
            decrypted_buffer[i] = buffer[i] ^ key[key_pos];
            key_pos = (key_pos + 1) % key_len;
            
            if (DEBUG) {
                printf("Encrypted byte: ");
                for (int j = 7; j >= 0; j--) {
                    printf("%d", (buffer[i] >> j) & 1);
                }
                printf(" XOR Key: ");
                for (int j = 7; j >= 0; j--) {
                    printf("%d", (key[key_pos] >> j) & 1);
                }
                printf(" = Decrypted: ");
                for (int j = 7; j >= 0; j--) {
                    printf("%d", (decrypted_buffer[i] >> j) & 1);
                }
                printf("\n");
            }
        }
        
        // Write decrypted buffer to output file
        if (fwrite(decrypted_buffer, 1, bytes_read, outFile) != bytes_read) {
            printf("Failed to write to output file\n");
            fclose(inFile);
            fclose(outFile);
            free(inputPath);
            free(outputPath);
            return 1;
        }
    }

    // Cleanup
    fclose(inFile);
    fclose(outFile);
    free(inputPath);
    free(outputPath);
    
    if (DEBUG) {
        printf("File decrypted successfully: %s\n", final_path);
    }
    return 0;
}