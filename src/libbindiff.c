#include "libbindiff.h"

void concatHex(unsigned char source, char * destination) {
        char tempStr[4] = "";
        snprintf(tempStr, sizeof(tempStr), "%02x ", source);
        strcat(destination, tempStr);
}

// Function to recursively get all file paths
void get_all_file_paths(const char * base_path, char ** * file_paths, int * count, int * capacity) {

        DIR * dir = opendir(base_path);
        struct dirent * entry; // Structure to store directory entry information
        char path[1024]; // Buffer to construct full paths

        // Check if directory was opened successfully
        if (dir == NULL) {
                perror("opendir");
                return;
        }

        while ((entry = readdir(dir)) != NULL) {
                // Skip the current (.) and parent (..) directory entries to avoid infinite loops
                if (strcmp(entry -> d_name, ".") == 0 || strcmp(entry -> d_name, "..") == 0) {
                        continue;
                }

                // Construct the full path by combining base path with entry name
                snprintf(path, sizeof(path), "%s/%s", base_path, entry -> d_name);

                if (entry -> d_type == DT_DIR) {
                        // Recursively call this function for subdirectories
                        get_all_file_paths(path, file_paths, count, capacity);
                } else {
                        // check if we need to expand our storage array
                        if ( * count >= * capacity) {
                                // Calculate new capacity: double current size or initialize to 16 if empty
                                int new_capacity = ( * capacity == 0) ? 16 : * capacity * 2;

                                // Reallocate memory for the file paths array
                                char ** new_array = (char ** ) realloc( * file_paths, new_capacity * sizeof(char * ));

                                // Check if reallocation was successful
                                if (new_array == NULL) {
                                        perror("realloc");
                                        closedir(dir);
                                        return;
                                }

                                // Update the array pointer and capacity
                                * file_paths = new_array;
                                * capacity = new_capacity;
                        }

                        // Add the file path to our array
                        ( * file_paths)[ * count] = strdup(path);

                        // Check if string duplication was successful
                        if (( * file_paths)[ * count] == NULL) {
                                perror("strdup");
                                closedir(dir);
                                return;
                        }

                        ( * count) ++;
                }
        }

        // Close the directory handle when done
        closedir(dir);
}

int compare_files(char * fp1, char * fp2, int padding) {
        FILE * file1, * file2;
        size_t bytes_read1, bytes_read2;
        struct stat sb;
        int result = 0; // 0 = identical, 1 = different
        int intracheck = 0;
        int file_size = 0;
        int state = 0;
        char diffblock1[100] = "", diffblock2[100] = "";
        unsigned char * data_block1, * data_block2;
        lstat(fp1, & sb);
        if (S_ISLNK(sb.st_mode)) return -1;
        file1 = fopen(fp1, "rb");
        file2 = fopen(fp2, "rb");

        fseek(file1, 0, SEEK_END);
        file_size = ftell(file1);
        rewind(file1);
        data_block1 = (unsigned char * ) malloc(file_size * sizeof(char));
        data_block2 = (unsigned char * ) malloc(file_size * sizeof(char));
        if (file1 == NULL || file2 == NULL) {
                printf("Error opening files: %s\n", fp1);
                if (file1) fclose(file1);
                if (file2) fclose(file2);
                return -1; // Error code
        }

        do {
                bytes_read1 = fread(data_block1, 1, file_size, file1);
                bytes_read2 = fread(data_block2, 1, file_size, file2);
                intracheck = 0;
                // Check if read lengths differ or content differs
                if ((bytes_read1 > 0 && memcmp(data_block1, data_block2, bytes_read1) != 0)) {
                        printf("Comparing Files:\n%s\n%s\n", fp1, fp2);
                        while (intracheck < bytes_read1) {
                                while (data_block1[intracheck] != data_block2[intracheck]) {
                                        if (state == 0) {
                                                for (int i = padding; i > 0; i--) {
                                                        if (intracheck - i >= 0) {
                                                                concatHex(data_block1[intracheck - i], diffblock1);
                                                                concatHex(data_block2[intracheck - i], diffblock2);
                                                        }
                                                }
                                                state = 1;
                                        }
                                        concatHex(data_block1[intracheck], diffblock1);
                                        concatHex(data_block2[intracheck], diffblock2);
                                        state = 1;
                                        intracheck++;
                                }
                                if (state == 1) {
                                        for (int i = 0; i < padding; i++) {
                                                if (intracheck + i <= file_size - 1) {
                                                        concatHex(data_block1[intracheck + i], diffblock1);
                                                        concatHex(data_block2[intracheck + i], diffblock2);
                                                }
                                        }
                                        printf("\n%07d\tBlock 1: %s\n%07d\tBlock 2: %s\n", intracheck, diffblock1, intracheck, diffblock2);
                                        memset(diffblock1, 0, sizeof(diffblock1));
                                        memset(diffblock2, 0, sizeof(diffblock2));
                                        state = 0;
                                }
                                intracheck++;
                        }
                        printf("\n");
                        result = 1;
                        break;
                }

        } while (bytes_read1 > 0 && bytes_read2 > 0);

        fclose(file1);
        fclose(file2);
        free(data_block1);
        free(data_block2);
        data_block1 = data_block2 = NULL;
        return result;
}
