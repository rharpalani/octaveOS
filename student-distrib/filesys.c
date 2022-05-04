#include "filesys.h"
#include "lib.h"
#include "system_execute.h"
#include "rtc.h"

dentry_t directory_entry;
boot_block_t* boot_block_location;
// boot_block* boot_block_location;

//set up jump tables
// jump_table_t rtc_jump_s = {.open = (void*)rtc_open, .close = (void*)rtc_close, .read = (void*)rtc_read, .write = (void*)rtc_write};
jump_table_t dir_jump_s = {.open = (void*)dopen, .close = (void*)dclose, .read = (void*)dread, .write = (void*)dwrite};
jump_table_t file_jump_s = {.open = (void*)fopen, .close = (void*)fclose, .read = (void*)fread, .write = (void*)fwrite};


// TODO create global vars for the jump tables, using jump_table_t struct
// - one for each of the three file types

/* f_init()
*  DESCRIPTION: Initialize structs for file system
*  INPUTS: boot_block_ptr_ - starting address of file system
*  OUTPUTS: none
*  RETURN VALUE: -1 if invalid, 0 on success
*/
int32_t f_init(uint32_t boot_block_ptr_){
    // if NULL
    if (!boot_block_ptr_){
        return -1; 
    }

    boot_block_location = (boot_block_t*)boot_block_ptr_;
    // file_open = FILE_NOT_OPEN;

    return 0;
}


/* read_dentry_by_name()
*  DESCRIPTION: Searches for file in file directory by name
*  INPUTS: fname - name of file to be found
           dentry - struct to copy information to
*  OUTPUTS: none
*  RETURN VALUE: -1 if file not in directory, 0 if file found in directory
*/
int32_t read_dentry_by_name (const uint8_t* fname, dentry_t* dentry){
    int i;

    // loops through directories in directory to find fname
    for (i = 0; i < boot_block_location->dir_count; ++i){
        if (!strncmp((int8_t*)fname,get_dentry_address(i)->filename, FILE_NAME_SIZE)){
            // if the fname is the same
            copy_dentry(dentry, get_dentry_address(i));
            return 0;   //fname found
        }
    }

    return -1; // fname not found

}

/* read_dentry_by_index()
*  DESCRIPTION: Copies file information of file given by index into dentry struct
*  INPUTS: index - index of file in directory
           dentry - struct to copy information to
*  OUTPUTS: none
*  RETURN VALUE: -1 if index isn't in range, 0 on success
*/
int32_t read_dentry_by_index (uint32_t index, dentry_t* dentry){

    // first check if the index is in range
    if (index >= boot_block_location->dir_count){
        return -1;
    }

    // copy file information into dentry
    copy_dentry(dentry, get_dentry_address(index));

    return 0;   //file copied
}

/* read_data()
*  DESCRIPTION: Reads up to "length" bytes starting from "offset" in file with 
                inode number "inode"
*  INPUTS: inode - inode of file to be read
           offset - starting position in file
           buf - buffer to copy data into
           length - number of bytes to read
*  OUTPUTS: none
*  RETURN VALUE: Number of bytes read and placed into buffer
*/
int32_t read_data (uint32_t inode, uint32_t offset, uint8_t* buf, uint32_t length){
    
    uint32_t d_block_ind; // the current index into the data_block_num for this inode
    int32_t read; // num of bytes that have been read 
    uint32_t left_to_read; // num of bytes left to be read
    inode_t* t_inode; //inode of file

    // make sure this inode is in range
    if (inode >= boot_block_location->inode_count) {
        return -1;
    }

    // pull up this inode 
    t_inode = (inode_t*) (boot_block_location + inode + 1); //add 1 because i_node is zero indexed

    // check offset in range
    if (offset >= t_inode->length){
        // printf("Offset out of range from beginning\n");
        return 0;
    }

    left_to_read = (length < t_inode->length) ? length : t_inode->length;
    read = 0; 

    // calculate which data block offset starts from
    d_block_ind = offset / (DATA_BLOCK_SIZE * NUM_DATABLOCKS); 

    while (left_to_read){

        // printf("In while loop in read_data\n");

        // check if our dblock_num is allowed 
        if ( t_inode->data_block_num[d_block_ind] >= boot_block_location->data_count) {
            return -1;
        }
        
        // get address to appropriate data block
        uint8_t* dblock = get_data_address(t_inode->data_block_num[d_block_ind]);

        // if multiple data blocks left to be read
        if (left_to_read >= DATA_BLOCK_SIZE * NUM_DATABLOCKS) {
            memcpy(buf, dblock+offset, DATA_BLOCK_SIZE * NUM_DATABLOCKS - offset);
            d_block_ind++;
            left_to_read -= NUM_DATABLOCKS * DATA_BLOCK_SIZE - offset;
            read += NUM_DATABLOCKS * DATA_BLOCK_SIZE - offset;
            buf += NUM_DATABLOCKS * DATA_BLOCK_SIZE - offset; 
            offset = 0; // offset should be 0 for every block every first
        }
        
        // on the last data block for this read
        else { 
            // printf("On last block for this read\n");
            // memcpy(buf, dblock+offset, left_to_read - offset + 1);
            // read += left_to_read - offset + 1;
            memcpy(buf, dblock+offset, left_to_read);
            read += left_to_read;
            left_to_read = 0; // we don't want to read anymore
        }
    }

    return read;    // return num of bytes read

}


// FILE FUNCTIONS


/* fopen()
*  DESCRIPTION: Opens file given a filename
*  INPUTS: filename - tells which file to open
            current_pcb - current process block being run
*  OUTPUTS: none
*  RETURN VALUE: -1 if failed, 0 on success
*/
int32_t fopen(const uint8_t* filename){

    int i;
    int fd = 0; // marks which pos in file array open, 0 is meaningless bc
        // it should already be filled by stdin
    dentry_t f_directory_entry;

    pcb_t* pcb = pid_to_pcb(curr_pid);

    if (!pcb | !filename){
        return -1; 
    }

    for (i = 2; i < FILE_ARRAY_SIZE; ++i){
        // find the first open entry in the array
        if (pcb->file_array[i].flags != FILE_OPEN){
            fd = i;
            break;
        }
    }
    if (fd > 0){
        if (read_dentry_by_name(filename,&f_directory_entry) == -1){
            printf("did not read dentry correctly\n");
            printf("%s\n",filename);
            return -1;
        }
        // now our temp directory entry has the value we need
        pcb->file_array[fd].inode = f_directory_entry.inode_num;
        pcb->file_array[fd].fpos = 0;
        pcb->file_array[fd].flags = FILE_OPEN; 
        pcb->file_array[fd].file_op_table_ptr = &file_jump_s;
        // printf("fopen fd is %d\n",fd);
        return fd;
    }
    // file_open = FILE_OPEN; 
    // printf("Read dentry correctly in fopen\n");
    return -1; 
}

/* fread()
*  DESCRIPTION: Copies file data
*  INPUTS: fd - file descriptor to see which files are open
           buf - buffer to copy information to
           nbytes - number of bytes to be copied
           current_pcb - current process block being run
*  OUTPUTS: none
*  RETURN VALUE: -1 if invalid, number if bytes read if success
*/
int32_t fread(int32_t fd, void* buf, int32_t nbytes) {
    // checks if buffer is null
    // printf("Entering fread\n");
    int retval;
    
    pcb_t* pcb = pid_to_pcb(curr_pid);

    if (!buf | !pcb) { // null checking
        // printf("Buf in fread is null");
        return -1;
    }

    // return number of bytes read by calling read_data
    retval = read_data(pcb->file_array[fd].inode, pcb->file_array[fd].fpos, (uint8_t*)buf, nbytes);
    if (retval>=0){
        pcb->file_array[fd].fpos += retval;
    }
    return retval;
}

/* fclose()
*  DESCRIPTION: Closes an open file
*  INPUTS: fd - file descriptor to see which files are open
            current_pcb - current process block being run
*  OUTPUTS: none
*  RETURN VALUE: 0 on success
*/
int32_t fclose(int32_t fd) {
    
    pcb_t* pcb = pid_to_pcb(curr_pid);

    if (!pcb){
        printf("Passed in null ptr to fclose");
        return -1;
    }
    if ((fd >= FILE_ARRAY_SIZE) | (fd<2)) {
        printf("Passed in invalid fd to fclose");
        return -1;
    }


    pcb->file_array[fd].flags = FILE_NOT_OPEN;

    return 0;
}

/* fwrite()
*  DESCRIPTION: Write information to file
*  INPUTS: none
*  OUTPUTS: none
*  RETURN VALUE: -1 always
*/
int32_t fwrite(int32_t fd, const void* buf, int32_t nbytes) {
    return -1;
}


// // DIRECTORY FUNCTIONS


/* dclose()
*  DESCRIPTION: Close the directory
*  INPUTS: fd - file descriptor to see which files are open
            current_pcb - current process block being run
*  OUTPUTS: none
*  RETURN VALUE: 0 on success
*/
int32_t dclose(int32_t fd){
    // file_open = FILE_NOT_OPEN;
    pcb_t* pcb = pid_to_pcb(curr_pid); 
        
    if (!pcb){
        printf("Passed in null ptr to dclose");
        return -1;
    }
    if ((fd >= FILE_ARRAY_SIZE) | (fd<2)) {
        printf("Passed in invalid fd to dclose");
        return -1;
    }


    pcb->file_array[fd].flags = FILE_NOT_OPEN;
    // file_open = FILE_NOT_OPEN;
    return 0;
}

/* dwrite()
*  DESCRIPTION: Write information to directory
*  INPUTS: none
*  OUTPUTS: none
*  RETURN VALUE: -1 always
*/
int32_t dwrite(int32_t fd, const void* buf, int32_t nbytes) {
    return -1;
}

/* dopen()
*  DESCRIPTION: Opens the file directory
*  INPUTS: current_pcb - current process block being run
*  OUTPUTS: none
*  RETURN VALUE: -1 if invalid, 0 on success
*/
int32_t dopen(const uint8_t* filename){

    int i;
    int fd = 0; // marks which pos in file array open, 0 is meaningless bc
        // it should already be filled by stdin
    // dentry_t d_directory_entry;

    pcb_t* current_pcb = pid_to_pcb(curr_pid);

    if (!current_pcb){
        printf("Null pcb");
        return -1; 
    }


    pcb_t* pcb = (pcb_t*)current_pcb;

    for (i = 2; i < FILE_ARRAY_SIZE; ++i){
        // find the first open entry in the array
        if (pcb->file_array[i].flags != FILE_OPEN){
            fd = i;
            break;
        }
    }

    if (fd > 0){
        // if (read_dentry_by_name((uint8_t*)".",&directory_entry) == -1){
        //     printf("did not read dentry correctly\n");
        //     return -1;
        // }
        // now our temp directory entry has the value we need
        pcb->file_array[fd].inode = 0;
        pcb->file_array[fd].fpos = 0;
        pcb->file_array[fd].flags = FILE_OPEN; 
        pcb->file_array[fd].file_op_table_ptr = &dir_jump_s;
        // printf("Leaving dopen with fd %d\n",fd);
        return fd;
    }
    // file_open = FILE_OPEN; 
    // printf("Read dentry correctly in fopen\n");
    // printf("Could not find empty space to dopen, i and fd were %d and %d\n", i, fd);
    return -1; 


    // old version
    // // checks if already open
    // if (file_open){
    //     return -1; 
    // }

    // // checks if file is in directory
    // fpos = 0; 
    // if (read_dentry_by_name((uint8_t*)".",&directory_entry) == -1){
    //     return -1;
    // }

    // // opens directory
    // file_open = FILE_OPEN;
    // return 0; 
}

/* dread()
*  DESCRIPTION: Copies file name
*  INPUTS: fd - file descriptor to see which files are open
           buf - buffer to copy information to
           nbytes - number of bytes to be copied
           current_pcb - current process block being run
*  OUTPUTS: none
*  RETURN VALUE: -1 if invalid, 0 if nothing to print, number of bytes copied otherwise
*/
int32_t dread(int32_t fd, void* buf, int32_t nbytes){
    

    dentry_t temp_dentry; 
    pcb_t* pcb = pid_to_pcb(curr_pid);

    // printf("Entering dread()");
    if (!buf | !pcb) {
        return -1;
    }


    if (pcb->file_array[fd].fpos >= boot_block_location -> dir_count){
        // just return without printing bc we've already printed all files
        return 0;
    }
    
    // print out the fpos filename
    if (read_dentry_by_index(pcb->file_array[fd].fpos, &temp_dentry) == -1) {
        return -1;
    }

    // dentry now has the dentry corresponding to filename for printing
    memcpy(buf,temp_dentry.filename, FILE_NAME_SIZE);
    pcb->file_array[fd].fpos++;

    return FILE_NAME_SIZE;
}


// // HELPER FUNCTIONS


/* get_dentry_address()
*  DESCRIPTION: Returns dentry address in memory given index
*  INPUTS: index - index in file directory
*  OUTPUTS: none
*  RETURN VALUE: address of inputted index
*/
dentry_t* get_dentry_address(uint32_t index){
    return (dentry_t*)(&(boot_block_location->direntries[index]));
}

/* copy_dentry()
*  DESCRIPTION: Copies dentry information from one to another
*  INPUTS: to - dentry to copy information to
           from - dentry to copy information from
*  OUTPUTS: none
*  RETURN VALUE: none
*/
void copy_dentry(dentry_t* to, dentry_t* from) {
    memcpy(to, from, DENTRY_SIZE); // copy over the entire dentry except reserved
}

/* get_data_address()
*  DESCRIPTION: Get address of inputted data
*  INPUTS: dblocknum - number of dentry
*  OUTPUTS: none
*  RETURN VALUE: address of inputted dentry
*/
uint8_t* get_data_address(uint32_t dblock_num) {
    return (uint8_t*)(boot_block_location + boot_block_location->inode_count + 1 + dblock_num);
}
