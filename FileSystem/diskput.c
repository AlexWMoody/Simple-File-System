#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>

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
        //printf("Super block information\n");
        //printf("Block size: %u\n", super_block->block_size);
        //printf("Block count: %lu\n", super_block->block_count);
        //printf("FAT starts: %lu\n", super_block->fat_starts);
        //printf("FAT blocks: %lu\n", super_block->fat_blocks);
        //printf("Root directory starts: %lu\n", super_block->root_dir_starts);
        //printf("Root directory blocks: %lu\n\n", super_block->root_dir_blocks);

        // Display FAT information
        //printf("FAT information\n");
        //printf("Free blocks: %lu\n", fat_info->free_blocks);
        //printf("Reserved blocks: %lu\n", fat_info->reserved_blocks);
        //printf("Allocated blocks: %lu\n", fat_info->allocated_blocks);
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

void read_directory(FILE *file, struct dir_entry_t *directory, SuperBlock* super_block, char *filename, char **tokens, int size) {
    
    ////printf("size: %u\n", htonl(directory->size));
    int start_offset = directory->starting_block * super_block->block_size;
    fseek(file, start_offset, SEEK_SET);

    uint64_t *ptr64 = (uint64_t *)malloc(sizeof(uint64_t));

    long int offset = 1;
    int tmp = super_block->block_size/64;
    int count = (directory->block_count)*tmp;
    ////printf("count in nr: %d\n", count);
    ////printf("bc in nr: %lu\n", directory->block_count);

    for(int i = 0; i < count; i++){
        ////printf("here\n");
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

        /*if (directory->status == 3) {
                //printf("F %10u ", htonl(directory->size));
                //printf("%30s", str);
                //printf(" %d/%02d/%02d %02d:%02d:%02d\n", directory->create_time.year, directory->create_time.month, directory->create_time.day, directory->create_time.hour, directory->create_time.minute, directory->create_time.second);
        }  
        if (directory->status == 5) {
                //printf("D %10u ", htonl(directory->size));
                //printf("%30s", str);
                //printf(" %d/%02d/%02d %02d:%02d:%02d\n", directory->create_time.year, directory->create_time.month, directory->create_time.day, directory->create_time.hour, directory->create_time.minute, directory->create_time.second);
        } */
        int sz = size - 1;
        ////printf("size-1: %d\n", sz);

        if(strcasecmp(str, tokens[size-1]) == 0){//don't wnat this?just wanna be in directory

            //copy file to linux current directory
            ////printf("success\n");

            //int outFile = open(filename, O_WRONLY | O_CREAT | O_TRUNC, 0666);
            FILE* input_file = fopen(filename, "wb");
            if (!input_file) {
                perror("Error creating output file");
                fclose(file);
                exit(EXIT_FAILURE);
            }

            directory->starting_block = htonl(directory->starting_block);
            directory->block_count = htonl(directory->block_count);

            fseek(file, directory->starting_block * super_block->block_size, SEEK_SET);

            char *buffer = malloc(super_block->block_size);
            if(!buffer){
                perror("Mem allocation failed");
                fclose(input_file);
                fclose(file);
                exit(1);
            }
            ////printf("Starting block: %lu, Block count: %lu\n", directory->starting_block, directory->block_count);
            for(unsigned int i = 0; i < directory->block_count; i++){
                ////printf("copying block %u\n", i);
                fread(buffer, super_block->block_size, 1, file);
                fwrite(buffer, super_block->block_size, 1, input_file);
                ////printf("%s\n", *buffer);
            }
            
            fclose(input_file);
            return;
        }

    }
    printf("File not found.\n");

}

void read_directory_recursive(FILE *file, struct dir_entry_t *directory, char **tokens, int size, SuperBlock* super_block, char *filename, FILE *input_file, FatInfo* fat_info) {
    
    int start_offset = directory->starting_block * super_block->block_size;
    fseek(file, start_offset, SEEK_SET);

    uint64_t *ptr64 = (uint64_t *)malloc(sizeof(uint64_t));

    long int offset = 1;
    //int count = directory->block_count;
    int tmp = super_block->block_size/64;
    int count = (directory->block_count)*tmp;
    ////printf("count: %d\n", count);
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
        ////printf("str: %s\n", str);

        if (directory->status == 5 || size == 1) {
            ////printf("suize: %d\n", size);
            for(int i = 0; i < size; i++){
                ////printf("i: %d, %s, %s\n",i,  str, tokens[i]);
                if(strcmp(str, tokens[i]) == 0 || size == 1){
                    if(size > 1){
                    if(i == size - 2){
                        ////printf("here?\n");
                        directory->starting_block = htonl(directory->starting_block);
                        directory->block_count = htonl(directory->block_count);
                    //    read_directory(file, directory, super_block, filename, tokens, size); //don't wanna do this? have directory, so good.

                        //check for enough space on disk?
                        struct stat sb;
                        stat(filename, &sb);
                        uint64_t sb_size = sb.st_size;
                        ////printf("sb size: %lu\n", sb_size);
                        if(sb_size > fat_info->free_blocks*super_block->block_size){
                            fclose(input_file);
                            return;
                        }
                        
                        //call new function for this:
                            //create new entry
                            struct dir_entry_t new_entry;
                            new_entry.size = sb_size;
                            new_entry.block_count = new_entry.size/super_block->block_size;
                            //printf("blk cnt: %lu\n", new_entry.block_count);
                            new_entry.status = 3;
                            char* stri = tokens[size-1];
                            for(int i = 0; i < 31; i++){
                                new_entry.filename[i] = stri[i];
                                ////printf("ne filename: %lu, filename: %c\n", new_entry.filename[i], stri[i]);
                                ////printf("%c\n", hexToAscii(new_entry.filename[i]));
                            }
                            for(int i = 0; i < 6; i++){
                                new_entry.unused[i] = 0xFF;
                            }

                            time_t t;
                            //struct tm *tm_info;
                            //time(&t);
                            //tm_info = localtime(&t); 

                            struct tm *tm_info = localtime(&sb.st_ctime);

                            new_entry.create_time.year = tm_info->tm_year+1900;
                            new_entry.create_time.month = tm_info->tm_mon+1;
                            new_entry.create_time.day = tm_info->tm_mday;
                            new_entry.create_time.hour = tm_info->tm_hour;
                            new_entry.create_time.minute = tm_info->tm_min;
                            new_entry.create_time.second = tm_info->tm_sec;

                           // //printf(" %d/%02d/%02d %02d:%02d:%02d\n", new_entry.create_time.year, new_entry.create_time.month, new_entry.create_time.day, new_entry.create_time.hour, new_entry.create_time.minute, new_entry.create_time.second);

                            ////printf("%d/%02d/%02d %02d:%02d:%02d\n", tm_info->tm_year + 1900,tm_info->tm_mon + 1,tm_info->tm_mday,tm_info->tm_hour,tm_info->tm_min, tm_info->tm_sec); 

                            //add filename and times

                            // go through fat to find 0000 0001 unused blocks to match size of input file 
                            uint64_t available_arr[sb_size/super_block->block_size];
                            uint64_t *fatbuffer = (uint64_t *)malloc(sizeof(uint64_t));
                            uint64_t *fatbuffer2 = (uint64_t *)malloc(sizeof(uint64_t));
                            fseek(file, super_block->fat_starts*super_block->block_size, SEEK_SET);

                            uint64_t entry_num = 1;
                            for(int i = 0; i < new_entry.block_count; i++){
                                while(htonl(*fatbuffer) != 0x00000000){
                                    fread(fatbuffer, 4, 1, file);
                                    entry_num++;
                                }
                                available_arr[i] = entry_num-1;
                                if(i == 0){
                                    new_entry.starting_block = available_arr[i];
                                }
                                //printf("array value %d: %lu\n", i, available_arr[i]);
                                *fatbuffer = 2;
                            }

                            /*int i = 0;
                            int j = 0;
                            int n = 0;
                            int last_open = 0;
                            while(n < (super_block->fat_blocks*super_block->block_size)/4){//starting block
                                fread(fatbuffer, 4, 1, file);
                                if(htonl(*fatbuffer) == 0x00000001){
                                    last_open = n;
                                    new_entry.starting_block = n;
                                    //printf("starting block: %lu\n", new_entry.starting_block);
                                    break;
                                }
                            }
                            i = n;
                            while(i < (super_block->fat_blocks*super_block->block_size)/4){
                                fread(fatbuffer, 4, 1, file);
                                if(htonl(*fatbuffer) == 0x00000001){
                                    //last_open = i; //entry i
                                    //j=0;
                                    fseek(file, (super_block->fat_starts*super_block->block_size)+(last_open*4), SEEK_SET);
                                    //fwrite( i, 4, 1, file);
                                    //printf("Would write: %d, to entry: %lu\n", i, (super_block->fat_starts*super_block->block_size)+(last_open*4));
                                    last_open = i;
                                    fseek(file, (super_block->fat_starts*super_block->block_size)+(last_open*4), SEEK_SET);

                                    /*while(j < (super_block->fat_blocks*super_block->block_size)-i){
                                        fread(fatbuffer2, 4, 1, file);
                                        if(*fatbuffer2 == 0x00000001){ //want entry i to point here


                                        }
                                    }
                                }
                                i++;

                                
                            }*/


                            start_offset = directory->starting_block * super_block->block_size;
                            fseek(file, start_offset, SEEK_SET);
                            for(int i = 0; i < directory->block_count*8; i++){
                                *ptr64 = 0;
                                offset = 1;
                                
                                fread(ptr64, offset, 1, file);
                                //printf("reading: %lu\n", *ptr64);
                                //directory->status = *ptr64;
                                if(*ptr64 == 0){
                                    break;
                                }
                                start_offset = start_offset + 64;
                                fseek(file, start_offset, SEEK_SET);
                            }
                            fseek(file, start_offset, SEEK_SET);
                            //printf("seeked to: %d\n", start_offset-1);

                            *ptr64 = new_entry.status;
                            //printf("ptr64: %lu\n", *ptr64);
                            fwrite(ptr64, 1, 1, file);

                            *ptr64 = new_entry.starting_block;
                            *ptr64 = htonl(*ptr64);
                            fwrite(ptr64, 4, 1, file);

                            *ptr64 = new_entry.block_count;
                            *ptr64 = htonl(*ptr64);
                            fwrite(ptr64, 4, 1, file);

                           // *ptr64 = 0;
                           // fwrite(ptr64, 1,1,file);
                            *ptr64 = new_entry.size;
                            *ptr64 = htonl(*ptr64);
                            fwrite(ptr64, 4, 1, file);

                            *ptr64 = new_entry.create_time.year;
                            *ptr64 = ntohs(*ptr64);
                            ////printf("year: %lu\n", *ptr64);

                            /*uint64_t temper = *ptr64;
                            //printf("temper: %lu\n", temper);
                            uint64_t reverse = 0;
                            int remainder = 0;

                            while (temper != 0) {
                                remainder = temper % 10;
                                reverse = reverse * 10 + remainder;
                                temper /= 10;
                                //printf("temper: %lu\n", temper);
                            }
                            //printf("rev: %lu\n", reverse);
                            *ptr64 = reverse;*/

                            fwrite(ptr64, 2, 1, file);

                            *ptr64 = new_entry.create_time.month;
                            fwrite(ptr64, 1, 1, file);


                            *ptr64 = new_entry.create_time.day;
                            fwrite(ptr64, 1, 1, file);

                            *ptr64 = new_entry.create_time.hour;
                            fwrite(ptr64, 1, 1, file);

                            *ptr64 = new_entry.create_time.minute;
                            fwrite(ptr64, 1, 1, file);

                            *ptr64 = new_entry.create_time.second;
                            fwrite(ptr64, 1, 1, file);

                            *ptr64 = new_entry.modify_time.year;
                            fwrite(ptr64, 2, 1, file);

                            *ptr64 = new_entry.modify_time.month;
                            fwrite(ptr64, 1, 1, file);

                            *ptr64 = new_entry.modify_time.day;
                            fwrite(ptr64, 1, 1, file);

                            *ptr64 = new_entry.modify_time.hour;
                            fwrite(ptr64, 1, 1, file);

                            *ptr64 = new_entry.modify_time.minute;
                            fwrite(ptr64, 1, 1, file);

                            *ptr64 = new_entry.modify_time.second;
                            fwrite(ptr64, 1, 1, file);
                            
                            int p = 0;
                            for(int i = 0; i < strlen(stri); i++){
                                //if(new_entry.filename[i] && new_entry.filename[i] != '\0'){
                                    *ptr64 = new_entry.filename[i];
                                    fwrite(ptr64, 1, 1, file);
                                    p++;
                                //}else{
                                //}
                            }
                            fseek(file, start_offset+27+p, SEEK_SET);

                            for(int i = 0; i < 6; i++){
                                *ptr64 = new_entry.unused[i];
                                fwrite(ptr64, 1, 1, file);
                            }
                            


                            //use loop to copy blcoks into correct block places in img file and update fat table accordingly
                            int seek = new_entry.starting_block*4;
                            //printf("Starting block: %lu\n", new_entry.starting_block);
                            fseek(file, super_block->fat_starts*super_block->block_size + seek, SEEK_SET);
                            //printf("Starting byte in fat: %lu\n", super_block->fat_starts*super_block->block_size + seek);
                            int last = 0;

                            uint64_t *buffer_arr = (uint64_t *)malloc(sizeof(uint64_t));

                            for(int i = 0; i < new_entry.block_count-1; i++){
                                fseek(file, super_block->fat_starts*super_block->block_size + seek-1, SEEK_SET);
                                *buffer_arr = available_arr[i+1];
                                fwrite(buffer_arr, 4, 1, file);
                                //printf("Would write: %ld ,to entry %lu, byte value: %lu\n", *buffer_arr, available_arr[i], super_block->fat_starts*super_block->block_size + seek);
                                seek = available_arr[i+1]*4;
                                last++;
                            }
                            fseek(file, super_block->fat_starts*super_block->block_size+seek-4, SEEK_SET);
                            *buffer_arr = 0xffffffff;
                            fwrite(buffer_arr, 4, 1, file);
                            //printf("Would write: %ld ,to entry %lu, byte value: %lu\n", *buffer_arr, available_arr[last], super_block->fat_starts*super_block->block_size + seek);

                            //seek = new_entry.starting_block*4;
                            //fseek(file, super_block->fat_starts*super_block->block_size + seek, SEEK_SET);
                            char *buffer = malloc(super_block->block_size);
                            fseek(input_file, 0, SEEK_SET);
                            if(!buffer){
                                perror("Mem allocation failed");
                                fclose(input_file);
                                fclose(file);
                                exit(1);
                            }
                            for(int i = 0; i < new_entry.block_count; i++){
                                //fseek(file, available_arr[i]*super_block->block_size, SEEK_SET);
                                ////printf("copying block %u into block: %lu\n", i, available_arr[i]);
                                //fseek(input_file, , SEEK_SET);
                                fread(buffer, super_block->block_size, 1, input_file);
                                ////printf("buffer:%s\n", buffer);
                                fseek(file, available_arr[i]*super_block->block_size, SEEK_SET);
                                //printf("seeked to: %lu\n", available_arr[i]*super_block->block_size);
                                fwrite(buffer, super_block->block_size, 1, file);
                                ////printf("%s\n", buffer);
                                ////printf("would write: \n%s\nto block %lu\n", buffer, available_arr[i]);

                            }
                            //printf("FAT COUNT  %lu\n", fat_info->allocated_blocks);
                            fat_info->allocated_blocks = fat_info->allocated_blocks+new_entry.block_count;
                            //printf("BLOCK COUNT ADDED %lu\n", new_entry.block_count);
                            //printf("FAT COUNT  %lu\n", fat_info->allocated_blocks);
                            fat_info->free_blocks = fat_info->free_blocks-new_entry.block_count;

                            print_superfat(super_block, fat_info);
                        

                        return;
                    }else{
                        directory->starting_block = htonl(directory->starting_block);
                        directory->block_count = htonl(directory->block_count);
                        read_directory_recursive(file, directory, tokens, size, super_block, filename, input_file, fat_info);
                        return;
                    }

                    }else{
                        if(i == size - 1){
                        ////printf("here?\n");
                        directory->starting_block = htonl(directory->starting_block);
                        directory->block_count = htonl(directory->block_count);

                        //check for enough space on disk?
                        struct stat sb;
                        stat(filename, &sb);
                        uint64_t sb_size = sb.st_size;
                        ////printf("sb size: %lu\n", sb_size);
                        if(sb_size > fat_info->free_blocks*super_block->block_size){
                            fclose(input_file);
                            return;
                        }
                        

                        //call new function for this:
                            //create new entry
                            struct dir_entry_t new_entry;
                            new_entry.size = sb_size;
                            new_entry.block_count = new_entry.size/super_block->block_size;
                            //printf("blk cnt: %lu\n", new_entry.block_count);
                            new_entry.status = 3;
                            //add filename and times
                            char* stri = tokens[size-1];
                            for(int i = 0; i < 31; i++){
                                new_entry.filename[i] = stri[i];
                                ////printf("ne filename: %lu, filename: %c\n", new_entry.filename[i], stri[i]);
                                ////printf("%c\n", hexToAscii(new_entry.filename[i]));
                            }

                            /*har *stri = tokens[size-1];
                            char fn[31];
                            strcpy(fn, stri);
                            //printf("filesname: %hhn\n", fn);*/

                            for(int i = 0; i < 6; i++){
                                new_entry.unused[i] = 0xFF;
                            }

                            // Get current time
                            time_t t;
                            //struct tm *tm_info;
                            //time(&t);
                            //tm_info = localtime(&t); 

                            struct tm *tm_info = localtime(&sb.st_ctime);

                            new_entry.create_time.year = tm_info->tm_year+1900;
                            new_entry.create_time.month = tm_info->tm_mon+1;
                            new_entry.create_time.day = tm_info->tm_mday;
                            new_entry.create_time.hour = tm_info->tm_hour;
                            new_entry.create_time.minute = tm_info->tm_min;
                            new_entry.create_time.second = tm_info->tm_sec;

                            ////printf("TIME: %ld\n", sb.st_ctime);
                            ////printf("Last status change:       %s", ctime(&sb.st_ctime));

                            ////printf(" %d/%02d/%02d %02d:%02d:%02d\n", new_entry.create_time.year, new_entry.create_time.month, new_entry.create_time.day, new_entry.create_time.hour, new_entry.create_time.minute, new_entry.create_time.second);

                           // //printf("%d/%02d/%02d %02d:%02d:%02d\n", tm_info->tm_year + 1900,tm_info->tm_mon + 1,tm_info->tm_mday,tm_info->tm_hour,tm_info->tm_min, tm_info->tm_sec); 

                            

                            // go through fat to find 0000 0001 unused blocks to match size of input file 
                            uint64_t available_arr[sb_size/super_block->block_size];
                            uint64_t *fatbuffer = (uint64_t *)malloc(sizeof(uint64_t));
                            uint64_t *fatbuffer2 = (uint64_t *)malloc(sizeof(uint64_t));
                            fseek(file, super_block->fat_starts*super_block->block_size, SEEK_SET);
                            ////printf("%lu\n", super_block->fat_starts*super_block->block_size);

                            uint64_t entry_num = 1;
                            for(int i = 0; i < new_entry.block_count; i++){
                                while(htonl(*fatbuffer) != 0x00000000){
                                    fread(fatbuffer, 4, 1, file);
                                    entry_num++;
                                }
                                available_arr[i] = entry_num-1;
                                if(i == 0){
                                    new_entry.starting_block = available_arr[i];
                                }
                                //printf("array value %d: %lu\n", i, available_arr[i]);
                                *fatbuffer = 2;
                            }

                            //add entry to root directory
                            start_offset = super_block->root_dir_starts * super_block->block_size;
                            fseek(file, start_offset, SEEK_SET);
                            for(int i = 0; i < super_block->root_dir_blocks*8; i++){
                                *ptr64 = 0;
                                offset = 1;
                                
                                fread(ptr64, offset, 1, file);
                                //printf("reading: %lu\n", *ptr64);
                                //directory->status = *ptr64;
                                if(*ptr64 == 0){
                                    break;
                                }
                                start_offset = start_offset + 64;
                                fseek(file, start_offset, SEEK_SET);
                            }
                            fseek(file, start_offset, SEEK_SET);
                            //printf("seeked to: %d\n", start_offset-1);

                            *ptr64 = new_entry.status;
                            //printf("ptr64: %lu\n", *ptr64);
                            fwrite(ptr64, 1, 1, file);

                            *ptr64 = new_entry.starting_block;
                            *ptr64 = htonl(*ptr64);
                            fwrite(ptr64, 4, 1, file);

                            *ptr64 = new_entry.block_count;
                            *ptr64 = htonl(*ptr64);
                            fwrite(ptr64, 4, 1, file);

                           // *ptr64 = 0;
                           // fwrite(ptr64, 1,1,file);
                            *ptr64 = new_entry.size;
                            *ptr64 = htonl(*ptr64);
                            fwrite(ptr64, 4, 1, file);

                            *ptr64 = new_entry.create_time.year;
                            *ptr64 = ntohs(*ptr64);
                            ////printf("year: %lu\n", *ptr64);

                            /*uint64_t temper = *ptr64;
                            //printf("temper: %lu\n", temper);
                            uint64_t reverse = 0;
                            int remainder = 0;

                            while (temper != 0) {
                                remainder = temper % 10;
                                reverse = reverse * 10 + remainder;
                                temper /= 10;
                                //printf("temper: %lu\n", temper);
                            }
                            //printf("rev: %lu\n", reverse);
                            *ptr64 = reverse;*/

                            fwrite(ptr64, 2, 1, file);

                            *ptr64 = new_entry.create_time.month;
                            fwrite(ptr64, 1, 1, file);


                            *ptr64 = new_entry.create_time.day;
                            fwrite(ptr64, 1, 1, file);

                            *ptr64 = new_entry.create_time.hour;
                            fwrite(ptr64, 1, 1, file);

                            *ptr64 = new_entry.create_time.minute;
                            fwrite(ptr64, 1, 1, file);

                            *ptr64 = new_entry.create_time.second;
                            fwrite(ptr64, 1, 1, file);

                            *ptr64 = new_entry.modify_time.year;
                            fwrite(ptr64, 2, 1, file);

                            *ptr64 = new_entry.modify_time.month;
                            fwrite(ptr64, 1, 1, file);

                            *ptr64 = new_entry.modify_time.day;
                            fwrite(ptr64, 1, 1, file);

                            *ptr64 = new_entry.modify_time.hour;
                            fwrite(ptr64, 1, 1, file);

                            *ptr64 = new_entry.modify_time.minute;
                            fwrite(ptr64, 1, 1, file);

                            *ptr64 = new_entry.modify_time.second;
                            fwrite(ptr64, 1, 1, file);
                            
                            int p = 0;
                            for(int i = 0; i < strlen(stri); i++){
                                //if(new_entry.filename[i] && new_entry.filename[i] != '\0'){
                                    *ptr64 = new_entry.filename[i];
                                    fwrite(ptr64, 1, 1, file);
                                    p++;
                                //}else{
                                //}
                            }
                            fseek(file, start_offset+27+p, SEEK_SET);

                            for(int i = 0; i < 6; i++){
                                *ptr64 = new_entry.unused[i];
                                fwrite(ptr64, 1, 1, file);
                            }
                            

                            /*int i = 0;
                            int j = 0;
                            int n = 0;
                            int last_open = 0;
                            while(n < (super_block->fat_blocks*super_block->block_size)/4){//starting block
                                fread(fatbuffer, 4, 1, file);
                                if(htonl(*fatbuffer) == 0x00000001){
                                    last_open = n;
                                    new_entry.starting_block = n;
                                    //printf("starting block: %lu\n", new_entry.starting_block);
                                    break;
                                }
                            }
                            i = n;
                            while(i < (super_block->fat_blocks*super_block->block_size)/4){
                                fread(fatbuffer, 4, 1, file);
                                if(htonl(*fatbuffer) == 0x00000001){
                                    //last_open = i; //entry i
                                    //j=0;
                                    fseek(file, (super_block->fat_starts*super_block->block_size)+(last_open*4), SEEK_SET);
                                    //fwrite( i, 4, 1, file);
                                    //printf("Would write: %d, to entry: %lu\n", i, (super_block->fat_starts*super_block->block_size)+(last_open*4));
                                    last_open = i;
                                    fseek(file, (super_block->fat_starts*super_block->block_size)+(last_open*4), SEEK_SET);

                                    /*while(j < (super_block->fat_blocks*super_block->block_size)-i){
                                        fread(fatbuffer2, 4, 1, file);
                                        if(*fatbuffer2 == 0x00000001){ //want entry i to point here


                                        }
                                    }
                                }
                                i++;

                                
                            }*/

                            //use loop to copy blcoks into correct block places in img file and update fat table accordingly
                            int seek = new_entry.starting_block*4;
                            //printf("Starting block: %lu\n", new_entry.starting_block);
                            fseek(file, super_block->fat_starts*super_block->block_size + seek, SEEK_SET);
                            //printf("Starting byte in fat: %lu\n", super_block->fat_starts*super_block->block_size + seek);
                            int last = 0;

                            uint64_t *buffer_arr = (uint64_t *)malloc(sizeof(uint64_t));

                            for(int i = 0; i < new_entry.block_count-1; i++){
                                fseek(file, super_block->fat_starts*super_block->block_size + seek-1, SEEK_SET);
                                *buffer_arr = available_arr[i+1];
                                fwrite(buffer_arr, 4, 1, file);
                                //printf("Would write: %ld ,to entry %lu, byte value: %lu\n", *buffer_arr, available_arr[i], super_block->fat_starts*super_block->block_size + seek);
                                seek = available_arr[i+1]*4;
                                last++;
                            }
                            fseek(file, super_block->fat_starts*super_block->block_size+seek-4, SEEK_SET);
                            *buffer_arr = 0xffffffff;
                            fwrite(buffer_arr, 4, 1, file);
                            //printf("Would write: %ld ,to entry %lu, byte value: %lu\n", *buffer_arr, available_arr[last], super_block->fat_starts*super_block->block_size + seek);

                            //seek = new_entry.starting_block*4;
                            //fseek(file, super_block->fat_starts*super_block->block_size + seek, SEEK_SET);
                            char *buffer = malloc(super_block->block_size);
                            fseek(input_file, 0, SEEK_SET);
                            if(!buffer){
                                perror("Mem allocation failed");
                                fclose(input_file);
                                fclose(file);
                                exit(1);
                            }
                            for(int i = 0; i < new_entry.block_count; i++){
                                //fseek(file, available_arr[i]*super_block->block_size, SEEK_SET);
                                ////printf("copying block %u into block: %lu\n", i, available_arr[i]);
                                //fseek(input_file, , SEEK_SET);
                                fread(buffer, super_block->block_size, 1, input_file);
                                ////printf("buffer:%s\n", buffer);
                                fseek(file, available_arr[i]*super_block->block_size, SEEK_SET);
                                //printf("seeked to: %lu\n", available_arr[i]*super_block->block_size);
                                fwrite(buffer, super_block->block_size, 1, file);
                                ////printf("%s\n", buffer);
                                ////printf("would write: \n%s\nto block %lu\n", buffer, available_arr[i]);

                            }
                            fat_info->allocated_blocks = fat_info->allocated_blocks+new_entry.block_count;
                            //printf("BLOCK COUNT ADDED %lu\n", new_entry.block_count);
                            //printf("FAT COUNT  %lu\n", fat_info->allocated_blocks);
                            fat_info->free_blocks = fat_info->free_blocks-new_entry.block_count;

                            print_superfat(super_block, fat_info);
                        

                        return;
                    }else{
                        directory->starting_block = htonl(directory->starting_block);
                        directory->block_count = htonl(directory->block_count);
                        read_directory_recursive(file, directory, tokens, size, super_block, filename, input_file, fat_info);
                        return;
                    }
                    }
                }
            }
        } 
        blocks++;
    }
    printf("File not found.\n");
}

int main(int argc, char* argv[]) {
    if (argc != 3 && argc != 2 && argc != 4) {
        printf("Usage: %s <file_system_image> <img_directory_path>\n", argv[0]);
        return 1;
    }

    FILE* file = fopen(argv[1], "r+");
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
    if(argc == 4){
        directory.starting_block = super_block.root_dir_starts;
        directory.size = super_block.block_size;
        directory.block_count = super_block.root_dir_blocks;

        char* filename = argv[2];
        char* img_directory_path = argv[3];
        char* img_directory_path1;
        strcpy(img_directory_path1, img_directory_path);

        /*if(strlen(img_directory_path) == 1){
            ////printf("do i enter here?\n");
            read_directory(file, &directory, &super_block, filename);
            fclose(file);
            return 0;
        }*/

        ////printf("dir path %s\n", img_directory_path);
        ////printf("dir path1 %s\n", img_directory_path1);

        char *token = strtok(img_directory_path , "/");
        int count = 0;
        while(token != NULL){
            ////printf("loop 1: %s\n", token);
            count++;
            token = strtok(NULL, "/");
        }

        char *tokens[count];
        char *token1 = strtok(img_directory_path1 , "/");
        int i = 0;
        while(token1 != NULL){
            ////printf("loop 2: %s\n", token1);
            tokens[i] = token1;
            ////printf("Array: %s\n", tokens[i]);
            i++;
            token1 = strtok(NULL, "/");
        }
       // //printf("%s %s %s\n", tokens[0], tokens[1], tokens[2]);
        ////printf("count: %d\n", count);

        print_superfat(&super_block, &fat_info);

        FILE* input_file = fopen(filename, "rb");
        if (!input_file) {
            perror("Error opening input file");
            fclose(file);
            return 0;
        }

        /*if(count == 1){ //call function straight?
            read_directory(file, &directory, &super_block, filename, tokens, count);
            fclose(file);
            return 0;
        }*/

        read_directory_recursive(file, &directory, tokens, count, &super_block, filename, input_file, &fat_info);
    }
    else if(argc == 3){
        /*directory.starting_block = super_block.root_dir_starts;
        directory.size = super_block.block_size;
        directory.block_count = super_block.root_dir_blocks;

        char* filename = argv[2];

        char* img_directory_path = argv[2];
        char* img_directory_path1;
        strcpy(img_directory_path1, img_directory_path);
        char *token = strtok(img_directory_path , "/");
        int count = 0;
        while(token != NULL){
            ////printf("loop 1: %s\n", token);
            count++;
            token = strtok(NULL, "/");
        }

        char *tokens[count];
        char *token1 = strtok(img_directory_path1 , "/");
        int i = 0;
        while(token1 != NULL){
            ////printf("loop 2: %s\n", token1);
            tokens[i] = token1;
            ////printf("Array: %s\n", tokens[i]);
            i++;
            token1 = strtok(NULL, "/");
        }
        //printf("%s \n", tokens[0]);
        //printf("count: %d\n", count);

        read_directory(file, &directory, &super_block, filename, tokens, count);*/
        printf("Usage: %s <file_system_image> <file_name> <path>\n", argv[0]);
    }
    else if(argc == 2){
        printf("Usage: %s <file_system_image> <file_name> <path>\n", argv[0]);
    }

    fclose(file);

    return 0;
}
