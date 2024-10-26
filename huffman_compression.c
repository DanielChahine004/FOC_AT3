#include "huffman_compression.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define MAX_SYMBOLS 256
#define DEBUG 0

typedef unsigned char byte;

typedef struct HuffmanNode {
    int symbol;
    int frequency;
    struct HuffmanNode *left;
    struct HuffmanNode *right;
    struct HuffmanNode *parent;
    int is_found;
} HuffmanNode_t;


typedef struct FileData {
    char *data;        // Pointer to store all the information in the file
    unsigned char *header; // Pointer to store the header of the file
    unsigned int fileSize; // Size of the file
    unsigned int original_fileSize; // Size of the original file
    HuffmanNode_t* huffman_tree; // Pointer to store the huffman tree
} FileData_t;


/* Function Prototypes */
FileData_t* readBMPFile(const char* inputPath);
int* getFrequencyTable(const FileData_t* fileData);
void sortHuffmanTree(HuffmanNode_t* huffman_tree[]);
HuffmanNode_t* createHuffmanTree(const int* freq_table);
char** generateHuffmanCodes(HuffmanNode_t* head);
byte char_array_to_byte(const char bit_array[9]);
void write_compressed_file(FileData_t* fileData, HuffmanNode_t* huffman_tree, char** huffman_codes, const char* outputPath);
FileData_t* read_compressed_file(const char* inputPath);
void decompress_file(FileData_t* compressed_fileData, const char* outputPath);
char* int_to_binary(unsigned int number);
void serialize_huffman_tree(HuffmanNode_t* node, FILE* file);
void free_huffman_tree(HuffmanNode_t* node);
char* create_full_path(const char* directory, const char* filename);
int compress_image_to_database(const char* image_name);
int decompress_file_to_decompressed(const char* image_name);


char* create_full_path(const char* directory, const char* filename) {
    char* full_path = malloc(strlen(directory) + strlen(filename) + 1);
    if (!full_path) {
        printf("Memory allocation failed\n");
        return NULL;
    }
    strcpy(full_path, directory);
    strcat(full_path, filename);
    return full_path;
}


int compress_image_to_database(const char* image_name) {

    char base_name[241];
    char output_name[256];
    
    // Copy with size limit to ensure space for suffix
    strncpy(base_name, image_name, sizeof(base_name) - 1);
    base_name[sizeof(base_name) - 1] = '\0';  // Ensure null termination
    
    // Remove .bmp extension
    size_t len = strlen(base_name);
    if (len >= 4) {
        base_name[len - 4] = '\0';
    }
    
    snprintf(output_name, sizeof(output_name), "%s_compressed.bmp", base_name);
    
    char* inputPath = create_full_path(IMAGE_DIRECTORY, image_name);
    char* outputPath = create_full_path(CLIENT_DATABASE, output_name);

    if (!inputPath || !outputPath) {
        free(inputPath);
        free(outputPath);
        return 1;
    }

    if (DEBUG) {
        printf("Compressing %s to %s\n", inputPath, outputPath);
    }

    FileData_t* original_fileData = readBMPFile(inputPath);
    int* freq_table = getFrequencyTable(original_fileData);
    HuffmanNode_t* huffman_head = createHuffmanTree(freq_table);
    char** huffman_codes = generateHuffmanCodes(huffman_head);

    write_compressed_file(original_fileData, huffman_head, huffman_codes, outputPath);

    // Clean up
    free(inputPath);
    free(outputPath);
    // Add any necessary cleanup for other allocated resources
    return 0;
}


int decompress_file_to_decompressed(const char* image_name) {

    char base_name[239];
    char output_name[256];
    
    // Copy with size limit to ensure space for suffix
    strncpy(base_name, image_name, sizeof(base_name) - 1);
    base_name[sizeof(base_name) - 1] = '\0';  // Ensure null termination
    
    // Remove .bmp extension
    size_t len = strlen(base_name);
    if (len >= 25) {
        base_name[len - 25] = '\0';
    }
    
    snprintf(output_name, sizeof(output_name), "%s_decompressed.bmp", base_name);

    char* inputPath = create_full_path(COMPRESSED_AND_DECRYPTED_DIRECTORY, image_name);
    char* outputPath = create_full_path(DECOMPRESSED_DIRECTORY, output_name);

    if (!inputPath || !outputPath) {
        free(inputPath);
        free(outputPath);
        return 1;
    }

    if (DEBUG) {
        printf("Decompressing %s to %s\n", inputPath, outputPath);
    }

    FileData_t* compressed_fileData = read_compressed_file(inputPath);
    decompress_file(compressed_fileData, outputPath);

    // Clean up
    free(inputPath);
    free(outputPath);
    // Add any necessary cleanup for compressed_fileData
    return 0;
}


FileData_t* readBMPFile(const char* inputPath) {
    FileData_t *fileData = (FileData_t*)malloc(sizeof(FileData_t));
    if (fileData == NULL) {
        printf("Memory allocation failed\n");
        return NULL;
    }
    
    FILE *file = fopen(inputPath, "rb");
    if (file == NULL) {
        printf("Error opening file\n");
        free(fileData);
        return NULL;
    }

    // Get file size
    fseek(file, 0, SEEK_END);
    fileData->fileSize = ftell(file);
    fseek(file, 0, SEEK_SET);

    // Allocate memory for file data
    fileData->data = (char*)malloc(fileData->fileSize);
    if (fileData->data == NULL) {
        printf("Memory allocation failed for file data\n");
        fclose(file);
        free(fileData);
        return NULL;
    }

    // Read all the characters in the file's binaries and store them in fileData->data
    size_t bytesRead = fread(fileData->data, 1, fileData->fileSize, file);
    if (bytesRead != fileData->fileSize) {
        printf("Error reading file\n");
        fclose(file);
        free(fileData->data);
        free(fileData);
        return NULL;
    }

    // Allocate memory for header (assuming 54 bytes for standard BMP header)
    fileData->header = (unsigned char*)malloc(54);
    if (fileData->header == NULL) {
        printf("Memory allocation failed for header\n");
        fclose(file);
        free(fileData->data);
        free(fileData);
        return NULL;
    }

    // Copy the first 54 bytes to the header
    memcpy(fileData->header, fileData->data, 54);

    fclose(file);
    return fileData;
}


int* getFrequencyTable(const FileData_t* fileData) {
    /* Create frequency table */
    int* freq_table = (int*)malloc(MAX_SYMBOLS * sizeof(int));

    /* Zero the frequency table */
    for (int i = 0; i < MAX_SYMBOLS; i++) {
        freq_table[i] = 0;
    }

    /* Populate the frequency table */
    for (unsigned int i = 54; i < fileData->fileSize; i++) { // Start after the header
        freq_table[(unsigned char)fileData->data[i]]++;
    }

    return freq_table;
}


void sortHuffmanTree(HuffmanNode_t* huffman_tree[]) {
    int i, j;
    for (i = 0; i < MAX_SYMBOLS; i++) {
        for (j = i + 1; j < MAX_SYMBOLS; j++) {
            if (huffman_tree[i]->frequency < huffman_tree[j]->frequency) {
                HuffmanNode_t* temp = huffman_tree[i];
                huffman_tree[i] = huffman_tree[j];
                huffman_tree[j] = temp;
            }
        }
    }
}


HuffmanNode_t* createHuffmanTree(const int* freq_table) {

    HuffmanNode_t* huffman_tree[MAX_SYMBOLS];

    /* Populate the huffman tree with leaf nodes */
    int i;
    for (i = 0; i < MAX_SYMBOLS; i++) {
        HuffmanNode_t* node = (HuffmanNode_t*)malloc(sizeof(HuffmanNode_t));
        node->symbol = i;
        node->frequency = freq_table[i];
        node->left = NULL;
        node->right = NULL;
        node->parent = NULL;
        node->is_found = 0;
        huffman_tree[i] = node;
    }  

    /* Sort the huffman tree in descending order */
    sortHuffmanTree(huffman_tree);

    int nodeCount = MAX_SYMBOLS;
    
    /* Create the huffman tree, observing the smallest two nodes of the tree each time 
    the size of the tree reduces by 1 each iteration so this loop shortens its span by 1 each time */
    while (nodeCount > 1) {
        int smallest = nodeCount - 1;
        int secondSmallest = nodeCount - 2;
        
        /* Combine smallest two nodes into a branch, and add it to the tree anywhere above the smallest node*/
        HuffmanNode_t* branch = (HuffmanNode_t*)malloc(sizeof(HuffmanNode_t));

        branch->symbol = -1; /* Branch nodes don't represent a symbol */
        branch->frequency = huffman_tree[smallest]->frequency + huffman_tree[secondSmallest]->frequency;
        branch->left = huffman_tree[smallest];
        branch->right = huffman_tree[secondSmallest];
        branch->parent = NULL;
        branch->is_found = 0;
        branch->right->parent = branch;
        branch->left->parent = branch;
        nodeCount--;
        
        huffman_tree[secondSmallest] = branch;
        
        /* Re-ort the huffman tree */
        sortHuffmanTree(huffman_tree);
    }
    
    // free(freq_table);
    return huffman_tree[0];
}


char** generateHuffmanCodes(HuffmanNode_t* head) {

    char ** huffman_codes = (char**)malloc(MAX_SYMBOLS * sizeof(char*));
    int max_traversal_length = 256;
    char* traversal_path = (char *)malloc(max_traversal_length * sizeof(char));
    
    for (int i = 0; i <= max_traversal_length; i++) {
        traversal_path[i] = '\0';
    }
    // printf("Initial traversal path is: %s\n", traversal_path);

    for (int i = 0; i < MAX_SYMBOLS; i++) {
        huffman_codes[i] = (char*)malloc(max_traversal_length * sizeof(char));
        strcpy(huffman_codes[i], traversal_path);
    }

    HuffmanNode_t* current = head;
    int leafs_found = 0;
    int depth = 0;

    while (leafs_found < MAX_SYMBOLS) {
        if (current->left == NULL && current->right == NULL && current->is_found == 0) {
            // printf("leaf number %d found, go up\n", leafs_found);
            // Leaf node found
            leafs_found++;
            current->is_found = 1;
            strcpy(huffman_codes[current->symbol], traversal_path);
            // printf("Symbol: %d, Code: %s\n", current->symbol, traversal_path);
            
            current = current->parent;
            traversal_path[depth] = '\0';
            depth--;
        } else if (current->left->is_found == 1 && current->right->is_found == 1) {
            // printf("branch, go up\n");
            current->is_found = 1;
            current = current->parent;
            traversal_path[depth] = '\0';
            depth--;
        } else if (current->left != NULL &&current->left->is_found == 0) {
            // printf("go left\n");
            // Go left
            current = current->left;
            traversal_path[depth] = '0';
            depth++;
        } else if (current->right != NULL && current->right->is_found == 0) {
            // printf("go right\n");
            // Go right
            current = current->right;
            traversal_path[depth] = '1';
            depth++;
        } else {
            // Shouldn't reach here
            printf("Error\n");
            break;
        }
    }


    free(traversal_path);

    return huffman_codes;
}


byte char_array_to_byte(const char bit_array[9]) {
    byte result = 0;
    for (int i = 0; i < 8; i++) {  // Still process only 8 bits
        if (bit_array[i] == '1') {
            result = (result << 1) | 1;
        } else {
            // Treat '0', '2', or any other character as '0' bit
            result = result << 1;
        }
    }
    return result;
}


void serialize_huffman_tree(HuffmanNode_t* node, FILE* file) {
    if (node == NULL) {
        return;
    }
    
    // Write a byte to indicate if it's a leaf node (1) or internal node (0)
    unsigned char is_leaf = (node->symbol != -1) ? 1 : 0;
    fwrite(&is_leaf, sizeof(unsigned char), 1, file);
    
    if (is_leaf) {
        // If it's a leaf, write the symbol
        fwrite(&node->symbol, sizeof(int), 1, file);
    } else {
        // Recursively serialize left and right children
        serialize_huffman_tree(node->left, file);
        serialize_huffman_tree(node->right, file);
    }
}


void write_compressed_file(FileData_t* fileData, HuffmanNode_t* huffman_tree, char** huffman_codes, const char* outputPath) {
    if (DEBUG) {
        printf("Writing compressed file to %s\n", outputPath);
    }

    FILE* file = fopen(outputPath, "wb");
    if (file == NULL) {
        printf("Error writing to file in write_compressed_file function\n");
        return;
    }

    char buffer_array[9] = "00000000";  // Initialize with '0's, add space for null terminator
    int buffer_index = 0;
    int bytes_written = 0;

    // Write the original file size first (important for decompression)
    fwrite(&fileData->fileSize, sizeof(unsigned int), 1, file);
    bytes_written += sizeof(unsigned int);
    
    if (DEBUG) {
        printf("Original file size written to the header: %d\n", fileData->fileSize);
    }

    // Serialize the Huffman tree
    serialize_huffman_tree(huffman_tree, file);

    // For every byte in the file, including the header
    for (unsigned int i = 0; i < fileData->fileSize; i++) {
        unsigned char current_byte = (unsigned char)fileData->data[i];
        char* binary_str = huffman_codes[current_byte];
        
        // For every bit in the huffman code of the byte
        for (int j = 0; binary_str[j] != '\0'; j++) {
            buffer_array[buffer_index] = binary_str[j];
            buffer_index++;
            
            // If the buffer is full, convert it to a byte and write it to the file
            if (buffer_index == 8) {
                unsigned char b = char_array_to_byte(buffer_array);
                fputc(b, file);
                bytes_written++;
                
                // Reset the buffer
                memset(buffer_array, '0', 8);
                buffer_array[8] = '\0';  // Ensure null termination
                buffer_index = 0;
            }
        }
    }

    // If there are any bits left in the buffer, pad with zeros and write
    if (buffer_index > 0) {
        unsigned char b = char_array_to_byte(buffer_array);
        fputc(b, file);
        bytes_written++;
    }

    if (DEBUG) {
        printf("Bytes written to compressed file: %d\n", bytes_written);
    }

    fclose(file);
}


HuffmanNode_t* deserialize_huffman_tree(FILE* file) {
    unsigned char is_leaf;
    if (fread(&is_leaf, sizeof(unsigned char), 1, file) != 1) {
        return NULL; // End of file or error
    }
    
    HuffmanNode_t* node = (HuffmanNode_t*)malloc(sizeof(HuffmanNode_t));
    if (node == NULL) {
        printf("Memory allocation failed\n");
        return NULL;
    }
    
    if (is_leaf) {
        fread(&node->symbol, sizeof(int), 1, file);
        node->left = node->right = NULL;
    } else {
        node->symbol = -1;
        node->left = deserialize_huffman_tree(file);
        node->right = deserialize_huffman_tree(file);
        if (node->left) node->left->parent = node;
        if (node->right) node->right->parent = node;
    }
    
    node->frequency = 0;
    node->parent = NULL;
    node->is_found = 0;
    
    return node;
}


void free_huffman_tree(HuffmanNode_t* node) {
    if (node == NULL) return;
    free_huffman_tree(node->left);
    free_huffman_tree(node->right);
    free(node);
}


FileData_t* read_compressed_file(const char* inputPath) {
    FileData_t* compressed_fileData = (FileData_t*)malloc(sizeof(FileData_t));
    if (compressed_fileData == NULL) {
        printf("Memory allocation failed for file data\n");
        return NULL;
    }

    FILE* file = fopen(inputPath, "rb");
    if (file == NULL) {
        printf("Error opening file at read_compressed_file function\n");
        free(compressed_fileData);
        return NULL;
    }

    // Read the original file size
    if (fread(&compressed_fileData->original_fileSize, sizeof(unsigned int), 1, file) != 1) {
        printf("Error reading original file size\n");
        fclose(file);
        free(compressed_fileData);
        return NULL;
    }

    // Deserialize the Huffman tree
    compressed_fileData->huffman_tree = deserialize_huffman_tree(file);
    if (compressed_fileData->huffman_tree == NULL) {
        printf("Error deserializing Huffman tree\n");
        fclose(file);
        free(compressed_fileData);
        return NULL;
    }

    // Get the remaining file size (compressed data)
    long current_position = ftell(file);
    fseek(file, 0, SEEK_END);
    compressed_fileData->fileSize = ftell(file) - current_position;
    fseek(file, current_position, SEEK_SET);

    // Allocate memory for compressed data
    compressed_fileData->data = (char*)malloc(compressed_fileData->fileSize);
    if (compressed_fileData->data == NULL) {
        printf("Memory allocation failed for compressed data\n");
        fclose(file);
        free_huffman_tree(compressed_fileData->huffman_tree);
        free(compressed_fileData);
        return NULL;
    }

    // Read the compressed data
    size_t bytesRead = fread(compressed_fileData->data, 1, compressed_fileData->fileSize, file);
    if (bytesRead != compressed_fileData->fileSize) {
        printf("Error reading compressed data: expected %u bytes, read %zu bytes\n", 
               compressed_fileData->fileSize, bytesRead);
        fclose(file);
        free(compressed_fileData->data);
        free_huffman_tree(compressed_fileData->huffman_tree);
        free(compressed_fileData);
        return NULL;
    }

    fclose(file);
    return compressed_fileData;
}


char* int_to_binary(unsigned int number) {
    // Special case for zero
    if (number == 0) {
        char* zero_str = (char*)malloc(2);
        zero_str[0] = '0';
        zero_str[1] = '\0';
        return zero_str;
    }

    int num_bits = sizeof(number) * 8;  // 8 bits per byte
    char* temp_str = (char*)malloc(num_bits + 1);  // Temporary string for all bits

    if (temp_str == NULL) {
        return NULL;
    }

    temp_str[num_bits] = '\0';  // Null terminator
    int i;

    // Fill the temp string with binary digits
    for (i = num_bits - 1; i >= 0; i--) {
        temp_str[i] = (number & 1) ? '1' : '0';
        number >>= 1;
    }

    // Find the first '1' to eliminate leading zeros
    int first_one_idx = 0;
    while (temp_str[first_one_idx] == '0') {
        first_one_idx++;
    }

    // Allocate memory for the result string with no leading zeros
    char* binary_str = (char*)malloc(num_bits - first_one_idx + 1);  // +1 for null terminator
    if (binary_str == NULL) {
        free(temp_str);  // Clean up temporary string if memory allocation fails
        return NULL;
    }

    // Copy the relevant part of the temp string to the final result
    for (i = 0; temp_str[first_one_idx + i] != '\0'; i++) {
        binary_str[i] = temp_str[first_one_idx + i];
    }
    binary_str[i] = '\0';  // Null terminate the result string

    free(temp_str);  // Clean up the temporary string
    return binary_str;
}


void decompress_file(FileData_t* compressed_fileData, const char* outputPath) {
    FILE* file = fopen(outputPath, "wb");
    if (file == NULL) {
        printf("Error writing to file at decompress_file function\n");
        return;
    }

    if (DEBUG) {
        printf("Original file size according to the header: %u\n", compressed_fileData->original_fileSize);
    }

    HuffmanNode_t* current = compressed_fileData->huffman_tree;
    unsigned int bytes_decoded = 0;

    for (size_t i = 0; i < compressed_fileData->fileSize; i++) {
        unsigned char byte = (unsigned char)compressed_fileData->data[i];
        for (int j = 7; j >= 0; j--) {
            if (byte & (1 << j)) {
                current = current->right;
            } else {
                current = current->left;
            }

            if (current->left == NULL && current->right == NULL) {
                if (bytes_decoded < compressed_fileData->original_fileSize) {
                    fputc(current->symbol, file);
                    bytes_decoded++;
                    current = compressed_fileData->huffman_tree;
                } else {
                    break;
                }
            }
        }
        if (bytes_decoded >= compressed_fileData->original_fileSize) {
            break;
        }
    }
    
    if (DEBUG) {
        printf("Bytes written to decompressed file: %d\n", bytes_decoded);
    }

    fclose(file);
}