#ifndef HUFFMAN_COMPRESSION_H
#define HUFFMAN_COMPRESSION_H

// You might want to include these if they're needed by users of this header
#include <stdio.h>

// If you want these to be accessible to users of the header
#define IMAGE_DIRECTORY "D:\\Programming\\C\\FOC_AT3\\Captures\\"
#define CLIENT_DATABASE "D:\\Programming\\C\\FOC_AT3\\ClientDataBase\\"
#define DECOMPRESSED_DIRECTORY "D:\\Programming\\C\\FOC_AT3\\Decompressed\\"

// Function to compress an image and save it to the database
// Returns 0 on success, non-zero on failure
int compress_image_to_database(const char* image_name);

// Function to decompress a file from the database and save it to the decompressed directory
// Returns 0 on success, non-zero on failure
int decompress_file_to_decompressed(const char* image_name);

#endif // HUFFMAN_COMPRESSION_H