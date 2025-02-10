
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

int main(int argc, char* argv[]) {
    if (argc != 2) {
        printf("Usage: %s <file_system_image>\n", argv[0]);
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

    fclose(file);

    // Display Super Block information
    printf("Super block information\n");
    printf("Block size: %u\n", super_block.block_size);
    printf("Block count: %lu\n", super_block.block_count);
    printf("FAT starts: %lu\n", super_block.fat_starts);
    printf("FAT blocks: %lu\n", super_block.fat_blocks);
    printf("Root directory starts: %lu\n", super_block.root_dir_starts);
    printf("Root directory blocks: %lu\n\n", super_block.root_dir_blocks);

    // Display FAT information
    printf("FAT information\n");
    printf("Free blocks: %lu\n", fat_info.free_blocks);
    printf("Reserved blocks: %lu\n", fat_info.reserved_blocks);
    printf("Allocated blocks: %lu\n", fat_info.allocated_blocks);

    return 0;
}
