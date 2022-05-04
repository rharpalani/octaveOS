#pragma once
#include "types.h"
#include "rtc.h"
// #include "system_execute.h"

#define FILENAME_LEN 32
#define RESERVED_BYTES_DENTRY 24
#define RESERVED_BYTES_BB 52
#define NUM_DIRENTRIES 63
#define INODE_DATABLOCK 1023
#define FILE_NOT_OPEN 0
#define FILE_OPEN 1
#define FILE_NAME_SIZE 32
#define DENTRY_SIZE 40
#define DATA_BLOCK_SIZE 1024
#define NUM_DATABLOCKS 4
#define FILE_ARRAY_SIZE 8
#define STDIN_IDX 0
#define STDOUT_IDX 1
#define FILE_START_IDX 2

//define struct for dentry_t
typedef struct dentry_t { 
    int8_t filename[FILENAME_LEN];
    int32_t filetype;
    int32_t inode_num;
    int8_t reserved[RESERVED_BYTES_DENTRY];
} dentry_t;

//define struct for boot_block
typedef struct boot_block_t { 
    int32_t dir_count;
    int32_t inode_count;
    int32_t data_count;
    int8_t reserved[RESERVED_BYTES_BB];
    dentry_t direntries[NUM_DIRENTRIES];
}  boot_block_t;

//define struct for inode_t
typedef struct inode_t {
    int32_t length;
    int32_t data_block_num[INODE_DATABLOCK];
} inode_t;



// typedef struct jump_table_t {
//     int32_t open;
//     int32_t close;
//     int32_t read; 
//     int32_t write;
// } jump_table_t; 


typedef struct jump_table_t {
    int32_t (*open)(const uint8_t* filename);
    int32_t (*close)(int32_t fd);
    int32_t (*read)(int32_t fd, void* buf, int32_t nbytes);
    int32_t (*write)(int32_t fd, const void* buf, int32_t nbytes);
} jump_table_t; 


typedef struct fd_array_t { 
    jump_table_t* file_op_table_ptr;
    int32_t inode;
    int32_t fpos;
    int32_t flags;
} fd_array_t;


extern boot_block_t* boot_block_location;

//Routines provided by file system module
int32_t f_init(uint32_t boot_block_ptr_);
int32_t read_dentry_by_name (const uint8_t* fname, dentry_t* dentry);
int32_t read_dentry_by_index (uint32_t index, dentry_t* dentry);
int32_t read_data (uint32_t inode, uint32_t offset, uint8_t* buf, uint32_t length);

//File IO functions
int32_t fopen(const uint8_t* filename);
int32_t fclose(int32_t fd);
int32_t fread(int32_t fd, void* buf, int32_t nbytes);
int32_t fwrite(int32_t fd, const void* buf, int32_t nbytes);

//Driver IO functions
int32_t dopen(const uint8_t* filename);
int32_t dclose(int32_t fd);
int32_t dread(int32_t fd, void* buf, int32_t nbytes);
int32_t dwrite(int32_t fd, const void* buf, int32_t nbytes);

//Helper functions
dentry_t* get_dentry_address(uint32_t index);
void copy_dentry(dentry_t* to, dentry_t* from);
uint8_t* get_data_address(uint32_t dblock_num);


