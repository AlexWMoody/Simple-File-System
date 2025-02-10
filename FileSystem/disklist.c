#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <string.h>

typedef struct {
    uint8_t fs_id;
    uint16_t block_size;
    uint64_t block_count;
    uint64_t fat_starts;
    uint64_t fat_blocks;
    uint64_t root_dir_starts;
    uint64_t root_dir_blocks;
} SuperBlock;

typedef struct {
    uint64_t free_blocks;
    uint64_t reserved_blocks;
    uint64_t allocated_blocks;
} FatInfo;

struct __attribute__((__packed__)) dir_entry_timedate_t {
uint16_t year;
uint8_t month;
uint8_t day;
uint8_t hour;
uint8_t minute;
uint8_t second;
}dir_entry_timedate_t;

struct __attribute__((__packed__)) dir_entry_t {
    uint64_t status; 
    uint64_t starting_block;//
    uint64_t block_count;//
    uint64_t size;//
    struct dir_entry_timedate_t create_time;
    struct dir_entry_timedate_t modify_time;
    uint64_t filename[31];
    uint64_t unused[6];
}dir_entry_t;

typedef struct {
    char name[30];
    uint8_t type;
    uint64_t size;
    char creation_time[20];
} FileInfo;

void print_superfat(SuperBlock* super_block, FatInfo* fat_info){
            // Display Super Block information
        printf("Super block information\n");
        printf("Block size: %u\n", super_block->block_size);
        printf("Block count: %lu\n", super_block->block_count);
        printf("FAT starts: %lu\n", super_block->fat_starts);
        printf("FAT blocks: %lu\n", super_block->fat_blocks);
        printf("Root directory starts: %lu\n", super_block->root_dir_starts);
        printf("Root directory blocks: %lu\n\n", super_block->root_dir_blocks);

        // Display FAT information
        printf("FAT information\n");
        printf("Free blocks: %lu\n", fat_info->free_blocks);
        printf("Reserved blocks: %lu\n", fat_info->reserved_blocks);
        printf("Allocated blocks: %lu\n", fat_info->allocated_blocks);
}

void read_super_block(FILE* file, SuperBlock* super_block) {
    fseek(file, 0, SEEK_SET);

    uint64_t *ptr = (uint64_t *)malloc(sizeof(uint64_t));
    uint8_t *ptr8 = (uint8_t *)malloc(sizeof(uint8_t));
    uint16_t *ptr16 = (uint16_t *)malloc(sizeof(uint16_t));
    int offset = 8;
    fread(ptr8, offset, 1, file);
    super_block->fs_id = htonl(*ptr8);

    offset = 2;
    fread(ptr16, offset, 1, file);
    super_block->block_size = ntohs(*ptr16);

    offset = 4;
    fread(ptr, offset, 1, file);
    super_block->block_count = htonl(*ptr);

    fread(ptr, offset, 1, file);
    super_block->fat_starts = htonl(*ptr);

    fread(ptr, offset, 1, file);
    super_block->fat_blocks = htonl(*ptr);

    fread(ptr, offset, 1, file);
    super_block->root_dir_starts = htonl(*ptr);

    fread(ptr, offset, 1, file);
    super_block->root_dir_blocks = htonl(*ptr);

    free(ptr);
    free(ptr8);
    free(ptr16);
}

void read_fat_info(FILE* file, SuperBlock* super_block, FatInfo* fat_info) {
    fseek(file, super_block->fat_starts * super_block->block_size, SEEK_SET);

    uint64_t *ptr = (uint64_t *)malloc(sizeof(uint64_t));
    int offset = 4;
    fat_info->reserved_blocks = 0;
    fat_info->free_blocks = 0;
    fat_info->allocated_blocks = 0;

    for(int i = 0; i < super_block->block_count; i++){
        fread(ptr, offset, 1, file);
        
        if(htonl(*ptr) == 0x00000000){
            fat_info->free_blocks++;
        }
        else if(htonl(*ptr) == 0x00000001){
            fat_info->reserved_blocks++;
        }
        else{
        //else if(htonl(*ptr) >= 0x00000002 && htonl(*ptr) <= 0xFFFFFF00){
            fat_info->allocated_blocks++;
        }
    }
    free(ptr);
}

char hexToAscii(uint64_t hexValue) {
    // Convert hex to decimal
    int decimal = 0;
    int base = 1;

    while (hexValue > 0) {
        int remainder = hexValue % 16;
        decimal += remainder * base;
        hexValue /= 16;
        base *= 16;
    }

    // Convert decimal to ASCII
    return (char)decimal;
}

void read_directory(FILE *file, struct dir_entry_t *directory, SuperBlock* super_block) {
    
    //printf("size: %u\n", htonl(directory->size));
    int start_offset = directory->starting_block * super_block->block_size;
    fseek(file, start_offset, SEEK_SET);

    uint64_t *ptr64 = (uint64_t *)malloc(sizeof(uint64_t));

    long int offset = 1;
    int tmp = super_block->block_size/64;
    int count = (directory->block_count)*tmp;
    //printf("count in nr: %d\n", count);
    //printf("bc in nr: %lu\n", directory->block_count);

    for(int i = 0; i < count; i++){
        //printf("here\n");
        *ptr64 = 0;

        offset = 1;
        fread(ptr64, offset, 1, file);
        directory->status = *ptr64;
        
        offset = 4;
        fread(ptr64, offset, 1, file);
        directory->starting_block = *ptr64;

        fread(ptr64, offset, 1, file);
        directory->block_count = *ptr64;

        fread(ptr64, offset, 1, file);
        directory->size = *ptr64;

        offset = 2;
        fread(ptr64, offset, 1, file);
        directory->create_time.year = ntohs(*ptr64);
        offset = 1;
        fread(ptr64, offset, 1, file);
        directory->create_time.month = (*ptr64);
        fread(ptr64, offset, 1, file);
        directory->create_time.day = (*ptr64);
        fread(ptr64, offset, 1, file);
        directory->create_time.hour = (*ptr64);
        fread(ptr64, offset, 1, file);
        directory->create_time.minute = (*ptr64);
        fread(ptr64, offset, 1, file);
        directory->create_time.second = (*ptr64);

        offset = 2;
        fread(ptr64, offset, 1, file);
        directory->modify_time.year = (*ptr64);
        offset = 1;
        fread(ptr64, offset, 1, file);
        directory->modify_time.month = (*ptr64);
        fread(ptr64, offset, 1, file);
        directory->modify_time.day = (*ptr64);
        fread(ptr64, offset, 1, file);
        directory->modify_time.hour = (*ptr64);
        fread(ptr64, offset, 1, file);
        directory->modify_time.minute = (*ptr64);
        fread(ptr64, offset, 1, file);
        directory->modify_time.second = (*ptr64);

        offset=1;
        for(int i = 0; i < 31; i++){
            fread(ptr64, offset, 1, file);
            directory->filename[i] = (*ptr64);
        }

        for(int i = 0; i < 6; i++){
            fread(ptr64, offset, 1, file);
            directory->unused[i] = (*ptr64);
        }

        char* str = (char *)malloc(sizeof(char *));
        str[0] = '\0';
        char buffer[1];
        for(int i = 0; i < 31; i++){
            buffer[0] = hexToAscii(directory->filename[i]);
            strcat(str, buffer);
        }

        if (directory->status == 3) {
                printf("F %10u ", htonl(directory->size));
                printf("%30s", str);
                printf(" %d/%02d/%02d %02d:%02d:%02d\n", directory->create_time.year, directory->create_time.month, directory->create_time.day, directory->create_time.hour, directory->create_time.minute, directory->create_time.second);
        }  
        if (directory->status == 5) {
                printf("D %10u ", htonl(directory->size));
                printf("%30s", str);
                printf(" %d/%02d/%02d %02d:%02d:%02d\n", directory->create_time.year, directory->create_time.month, directory->create_time.day, directory->create_time.hour, directory->create_time.minute, directory->create_time.second);
        } 

    }

}

void read_directory_recursive(FILE *file, struct dir_entry_t *directory, char **tokens, int size, SuperBlock* super_block) {
    
    int start_offset = directory->starting_block * super_block->block_size;
    fseek(file, start_offset, SEEK_SET);

    uint64_t *ptr64 = (uint64_t *)malloc(sizeof(uint64_t));

    long int offset = 1;
    //int count = directory->block_count;
    int tmp = super_block->block_size/64;
    int count = (directory->block_count)*tmp;
    //printf("count: %d\n", count);
    int blocks = 0;

    for(int i = 0; i < count; i++){
        *ptr64 = 0;

        offset = 1;
        fread(ptr64, offset, 1, file);
        directory->status = *ptr64;
        
        offset = 4;
        fread(ptr64, offset, 1, file);
        directory->starting_block = *ptr64;

        fread(ptr64, offset, 1, file);
        directory->block_count = *ptr64;

        fread(ptr64, offset, 1, file);
        directory->size = *ptr64;

        offset = 2;
        fread(ptr64, offset, 1, file);
        directory->create_time.year = ntohs(*ptr64);
        offset = 1;
        fread(ptr64, offset, 1, file);
        directory->create_time.month = (*ptr64);
        fread(ptr64, offset, 1, file);
        directory->create_time.day = (*ptr64);
        fread(ptr64, offset, 1, file);
        directory->create_time.hour = (*ptr64);
        fread(ptr64, offset, 1, file);
        directory->create_time.minute = (*ptr64);
        fread(ptr64, offset, 1, file);
        directory->create_time.second = (*ptr64);

        offset = 2;
        fread(ptr64, offset, 1, file);
        directory->modify_time.year = (*ptr64);
        offset = 1;
        fread(ptr64, offset, 1, file);
        directory->modify_time.month = (*ptr64);
        fread(ptr64, offset, 1, file);
        directory->modify_time.day = (*ptr64);
        fread(ptr64, offset, 1, file);
        directory->modify_time.hour = (*ptr64);
        fread(ptr64, offset, 1, file);
        directory->modify_time.minute = (*ptr64);
        fread(ptr64, offset, 1, file);
        directory->modify_time.second = (*ptr64);

        offset=1;
        for(int i = 0; i < 31; i++){
            fread(ptr64, offset, 1, file);
            directory->filename[i] = (*ptr64);
        }

        for(int i = 0; i < 6; i++){
            fread(ptr64, offset, 1, file);
            directory->unused[i] = (*ptr64);
        }

        char* str = (char *)malloc(sizeof(char *));
        str[0] = '\0';
        char buffer[1];
        for(int i = 0; i < 31; i++){
            buffer[0] = hexToAscii(directory->filename[i]);
            strcat(str, buffer);
        }
        if (directory->status == 5) {
            for(int i = 0; i < size; i++){
                if(strcmp(str, tokens[i]) == 0){
                    if(i == size - 1){
                        directory->starting_block = htonl(directory->starting_block);
                        directory->block_count = htonl(directory->block_count);
                        read_directory(file, directory, super_block);
                        return;
                    }else{
                        directory->starting_block = htonl(directory->starting_block);
                        directory->block_count = htonl(directory->block_count);
                        read_directory_recursive(file, directory, tokens, size, super_block);
                        return;
                    }
                }
            }
        } 
        blocks++;
    }

}

int main(int argc, char* argv[]) {
    if (argc != 3 && argc != 2) {
        printf("Usage: %s <file_system_image> <directory_path>\n", argv[0]);
        return 1;
    }

    FILE* file = fopen(argv[1], "rb");
    if (!file) {
        perror("Error opening file");
        return 1;
    }

    SuperBlock super_block;
    read_super_block(file, &super_block);

    FatInfo fat_info;
    read_fat_info(file, &super_block, &fat_info);

    uint64_t dir_start = super_block.root_dir_starts;

    struct dir_entry_t directory;

    if(argc == 3){

        directory.starting_block = super_block.root_dir_starts;
        directory.size = super_block.block_size;
        directory.block_count = super_block.root_dir_blocks;

        char* directory_path = argv[2];
        char* directory_path1;
        strcpy(directory_path1, directory_path);

        if(strlen(directory_path) == 1){
            //printf("do i enter here?\n");
            read_directory(file, &directory, &super_block);
            fclose(file);
            return 0;
        }

        //printf("dir path %s\n", directory_path);
        //printf("dir path1 %s\n", directory_path1);

        char *token = strtok(directory_path , "/");
        int count = 0;
        while(token != NULL){
            //printf("loop 1: %s\n", token);
            count++;
            token = strtok(NULL, "/");
        }

        char *tokens[count];
        char *token1 = strtok(directory_path1 , "/");
        int i = 0;
        while(token1 != NULL){
            //printf("loop 2: %s\n", token1);
            tokens[i] = token1;
            //printf("Array: %s\n", tokens[i]);
            i++;
            token1 = strtok(NULL, "/");
        }

        //print_superfat(&super_block, &fat_info);

        read_directory_recursive(file, &directory, tokens, count, &super_block);
    }
    else if(argc == 2){

        directory.starting_block = super_block.root_dir_starts;
        directory.size = super_block.block_size;
        directory.block_count = super_block.root_dir_blocks;

        //print_superfat(&super_block, &fat_info);

        read_directory(file, &directory, &super_block);
    }

    fclose(file);

    return 0;
}
