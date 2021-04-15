/*
 *  Copyright (C) 2021 CS416 Rutgers CS
 *	Tiny File System
 *	File:	tfs.c
 *
 */

#define FUSE_USE_VERSION 26

#include <fuse.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <errno.h>
#include <sys/time.h>
#include <libgen.h>
#include <limits.h>

#include "block.h"
#include "tfs.h"

char diskfile_path[PATH_MAX];

// Declare your in-memory data structures here
struct superblock sblock;
int totalblocks = DISK_SIZE/BLOCK_SIZE;
int num_inode_blocks = ((MAX_INUM*sizeof(struct inode))+BLOCK_SIZE-1)/BLOCK_SIZE;
int num_data_blocks = (MAX_DNUM+BLOCK_SIZE-1)/BLOCK_SIZE;
int num_inodebmap_blocks = (((MAX_INUM*sizeof(unsigned char))/8)+BLOCK_SIZE-1)/BLOCK_SIZE;
int num_dblockbmap_blocks = (((MAX_DNUM*sizeof(unsigned char))/8)+BLOCK_SIZE-1)/BLOCK_SIZE;
bitmap_t inodebmap;
bitmap_t dblockbmap;

/* 
 * Get available inode number from bitmap
 */
int get_avail_ino() {

	// Step 1: Read inode bitmap from disk
	for(int i = sblock.i_bitmap_blk; i < (sblock.i_bitmap_blk+num_inodebmap_blocks); i++){
		bio_read(i, (inodebmap+((i-sblock.i_bitmap_blk)*BLOCK_SIZE)));
	}

	// Step 2: Traverse inode bitmap to find an available slot
	int index = 0;
	while(get_bitmap(inodebmap, index) == 1 && index < MAX_INUM) index++;
	if(index == MAX_INUM) return -1; //nothing found
	
	// Step 3: Update inode bitmap and write to disk 
	set_bitmap(inodebmap, index);
	return index;
}

/* 
 * Get available data block number from bitmap
 */
int get_avail_blkno() {

	// Step 1: Read data block bitmap from disk
	for(int i = sblock.d_bitmap_blk; i < (sblock.d_bitmap_blk+num_dblockbmap_blocks); i++){
		bio_read(i, (dblockbmap+((i-sblock.d_bitmap_blk)*BLOCK_SIZE)));
	}
	
	// Step 2: Traverse data block bitmap to find an available slot
	int index = 0;
	while(get_bitmap(dblockbmap, index) == 1 && index < MAX_DNUM) index++;
	if(index == MAX_DNUM) return -1; //nothing found

	// Step 3: Update data block bitmap and write to disk 
	set_bitmap(dblockbmap, index);
	return index;
}

/* 
 * inode operations
 */
int readi(uint16_t ino, struct inode *inode) {

  // Step 1: Get the inode's on-disk block number
  int block_no = sblock.i_start_blk;

  //block_no will be right after this calculation
  int inodes_per_block = MAX_INUM/num_inode_blocks;
  int i = ino;
  while((i/inodes_per_block) > 0){
	  i-=inodes_per_block;
	  block_no++;
  }
  //block_no will be accurate now

  // Step 2: Get offset of the inode in the inode on-disk block
  int offset = i;
  printf("This is the block_no: %d\nThis is the offset in that block: %d\n", block_no, offset);
  // Step 3: Read the block from disk and then copy into inode structure
  struct inode* buf = (struct inode *)malloc(BLOCK_SIZE);
  bio_read(block_no, (void*)buf);
  inode = &buf[offset];
  free(buf);
  //IF INODE DOES NOT EXIST IT WILL CAUSE SEGFAULT
  printf("This is ino: %d\n", inode->ino);
  return 0;
}

int writei(uint16_t ino, struct inode *inode) {

	// Step 1: Get the block number where this inode resides on disk
	int block_no = sblock.i_start_blk;

	//block_no will be right after this calculation
	int inodes_per_block = MAX_INUM/num_inode_blocks;
	int i = ino;
	while((i/inodes_per_block) > 0){
		i-=inodes_per_block;
		block_no++;
	}
	//block_no will be accurate now
	struct inode* buf = (struct inode *)malloc(BLOCK_SIZE);
	bio_read(block_no, (void*)buf);
	// Step 2: Get the offset in the block where this inode resides on disk
	int offset = i;
	printf("This is the block_no: %d\nThis is the offset in that block: %d\n", block_no, offset);
	buf[offset] = *inode;
	printf("This is ino: %d\n", buf[offset].ino);
	// Step 3: Write inode to disk 
	bio_write(block_no, (void*)buf);
	free(buf);
	return 0;
}


/* 
 * directory operations
 */
int dir_find(uint16_t ino, const char *fname, size_t name_len, struct dirent *dirent) {

  // Step 1: Call readi() to get the inode using ino (inode number of current directory)

  // Step 2: Get data block of current directory from inode

  // Step 3: Read directory's data block and check each directory entry.
  //If the name matches, then copy directory entry to dirent structure

	return 0;
}

int dir_add(struct inode dir_inode, uint16_t f_ino, const char *fname, size_t name_len) {

	// Step 1: Read dir_inode's data block and check each directory entry of dir_inode
	
	// Step 2: Check if fname (directory name) is already used in other entries

	// Step 3: Add directory entry in dir_inode's data block and write to disk

	// Allocate a new data block for this directory if it does not exist

	// Update directory inode

	// Write directory entry

	return 0;
}

int dir_remove(struct inode dir_inode, const char *fname, size_t name_len) {

	// Step 1: Read dir_inode's data block and checks each directory entry of dir_inode
	
	// Step 2: Check if fname exist

	// Step 3: If exist, then remove it from dir_inode's data block and write to disk

	return 0;
}

/* 
 * namei operation
 */
int get_node_by_path(const char *path, uint16_t ino, struct inode *inode) {
	
	// Step 1: Resolve the path name, walk through path, and finally, find its inode.
	// Note: You could either implement it in a iterative way or recursive way

	return 0;
}

/* 
 * Make file system
 */
int tfs_mkfs() {
	/*
	ORDER OF STORAGE FOR FILE SYSTEM:
	Superblock is first thing in file system
	Then comes inode bitmap and data block bitbmap
	Then comes inode region
	Then comes data block region
	(Found on page 4 of Chapter 41 in textbook)
	*/
	printf("Disk size: %d\nBlock size: %d\nTotal blocks: %d\n", DISK_SIZE, BLOCK_SIZE, totalblocks);
	printf("Max inum: %d\nNum blocks for inode: %d\nNum blocks for inodebmap: %d\n", MAX_INUM, num_inode_blocks, num_inodebmap_blocks);
	printf("Max dblock: %d\nNum blocks for data block: %d\nNum blocks for dblockbmap: %d\n",MAX_DNUM, num_data_blocks, num_dblockbmap_blocks);
	// Call dev_init() to initialize (Create) Diskfile
	dev_init(diskfile_path);

	//write superblock information 
	//sblock is a globally declared superblock, structure for a superblock is in tfs.h
	sblock.magic_num = MAGIC_NUM; //Dont know what this does
	sblock.max_inum = MAX_INUM; //Maximum number of inodes
	sblock.max_dnum = MAX_DNUM; //Maximum number of datablocks
	sblock.i_bitmap_blk = 1; //Start block of inode bitmap -- One block after superblock which will always take up 1 block
	sblock.d_bitmap_blk = sblock.i_bitmap_blk + num_inodebmap_blocks; //Start block of datablock bitmap
	sblock.i_start_blk = sblock.d_bitmap_blk + num_dblockbmap_blocks; //Start block of inodes
	sblock.d_start_blk = sblock.i_start_blk + num_inode_blocks; //Start block of datablockprintf("Super block information read: \n");
	printf("Writing superblock to block 0\n");
	printf("This is block 0 writing to DISK from: %p\n", &sblock);
	bio_write(0, &sblock);

	// initialize inode bitmap
	inodebmap = malloc(sizeof(unsigned char)*(MAX_INUM/8));
	for(int i = 0; i < MAX_INUM; i++){
		unset_bitmap(inodebmap, i);
	}
	// initialize data block bitmap
	dblockbmap = malloc(sizeof(unsigned char) * (MAX_DNUM/8));
	for(int i = 0; i < MAX_DNUM; i++){
		unset_bitmap(dblockbmap, i);
	}
	// update bitmap information for root directory
	set_bitmap(dblockbmap, 0);
	// update inode for root directory
	set_bitmap(inodebmap, 0);
	//write inodebmap to disk
	printf("Writing inode bitmap to blocks %d - %d\n", sblock.i_bitmap_blk, sblock.i_bitmap_blk+num_inodebmap_blocks-1);
	for(int i = sblock.i_bitmap_blk; i < sblock.i_bitmap_blk+num_inodebmap_blocks; i++){
		printf("This is block %d writing to DISK from: %p\n", i-sblock.i_bitmap_blk, (inodebmap+(i*BLOCK_SIZE)));
		bio_write(i, (inodebmap+((i-sblock.i_bitmap_blk)*BLOCK_SIZE)));
	}
	//write dblockbmap to disk
	printf("Writing dblock bitmap to blocks %d - %d\n", sblock.d_bitmap_blk,sblock.d_bitmap_blk+num_dblockbmap_blocks-1);
	for(int i = sblock.d_bitmap_blk; i < sblock.d_bitmap_blk+num_dblockbmap_blocks; i++){
		printf("This is block %d writing to DISK from: %p\n", i-sblock.d_bitmap_blk, (dblockbmap+(i*BLOCK_SIZE)));
		bio_write(i, (dblockbmap+((i-sblock.d_bitmap_blk)*BLOCK_SIZE)));
	}
	return 0;
}


/* 
 * FUSE file operations
 */
static void *tfs_init(struct fuse_conn_info *conn) {
	// Step 1a: If disk file is not found, call mkfs
	if(dev_open(diskfile_path) == -1) {
		printf("Calling tfs_mkfs() from tfs_init()\n");
		tfs_mkfs();
	}
	// Step 1b: If disk file is found, just initialize in-memory data structures
	// and read superblock from disk
	else{
		//get space for bitmaps in local storage
		printf("Making space for inode bitmap...\n");
		inodebmap = malloc(sizeof(unsigned char)*(MAX_INUM/8));
		printf("Making space for datablock bitmap...\n");
		dblockbmap = malloc(sizeof(unsigned char)*(MAX_DNUM/8));
		//read superblock from disk
		printf("Reading superblock from disk...\n");
		bio_read(0, &sblock);
	}
	
	
	return NULL;
}

static void tfs_destroy(void *userdata) {

	// Step 1: De-allocate in-memory data structures
	printf("Freeing inode bitmap...\n");
	free(inodebmap);
	printf("Freeing data block bitmap...\n");
	free(dblockbmap);
	// Step 2: Close diskfile
	printf("Closing %s...\n", diskfile_path);
	dev_close();

}

static int tfs_getattr(const char *path, struct stat *stbuf) {

	// Step 1: call get_node_by_path() to get inode from path

	// Step 2: fill attribute of file into stbuf from inode

		stbuf->st_mode   = S_IFDIR | 0755;
		stbuf->st_nlink  = 2;
		time(&stbuf->st_mtime);

	return 0;
}

static int tfs_opendir(const char *path, struct fuse_file_info *fi) {

	// Step 1: Call get_node_by_path() to get inode from path

	// Step 2: If not find, return -1

    return 0;
}

static int tfs_readdir(const char *path, void *buffer, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi) {

	// Step 1: Call get_node_by_path() to get inode from path

	// Step 2: Read directory entries from its data blocks, and copy them to filler

	return 0;
}


static int tfs_mkdir(const char *path, mode_t mode) {

	// Step 1: Use dirname() and basename() to separate parent directory path and target directory name

	// Step 2: Call get_node_by_path() to get inode of parent directory

	// Step 3: Call get_avail_ino() to get an available inode number

	// Step 4: Call dir_add() to add directory entry of target directory to parent directory

	// Step 5: Update inode for target directory

	// Step 6: Call writei() to write inode to disk
	

	return 0;
}

static int tfs_rmdir(const char *path) {

	// Step 1: Use dirname() and basename() to separate parent directory path and target directory name

	// Step 2: Call get_node_by_path() to get inode of target directory

	// Step 3: Clear data block bitmap of target directory

	// Step 4: Clear inode bitmap and its data block

	// Step 5: Call get_node_by_path() to get inode of parent directory

	// Step 6: Call dir_remove() to remove directory entry of target directory in its parent directory

	return 0;
}

static int tfs_releasedir(const char *path, struct fuse_file_info *fi) {
	// For this project, you don't need to fill this function
	// But DO NOT DELETE IT!
    return 0;
}

static int tfs_create(const char *path, mode_t mode, struct fuse_file_info *fi) {

	// Step 1: Use dirname() and basename() to separate parent directory path and target file name

	// Step 2: Call get_node_by_path() to get inode of parent directory

	// Step 3: Call get_avail_ino() to get an available inode number

	// Step 4: Call dir_add() to add directory entry of target file to parent directory

	// Step 5: Update inode for target file

	// Step 6: Call writei() to write inode to disk

	return 0;
}

static int tfs_open(const char *path, struct fuse_file_info *fi) {

	// Step 1: Call get_node_by_path() to get inode from path

	// Step 2: If not find, return -1

	return 0;
}

static int tfs_read(const char *path, char *buffer, size_t size, off_t offset, struct fuse_file_info *fi) {

	// Step 1: You could call get_node_by_path() to get inode from path

	// Step 2: Based on size and offset, read its data blocks from disk

	// Step 3: copy the correct amount of data from offset to buffer

	// Note: this function should return the amount of bytes you copied to buffer
	return 0;
}

static int tfs_write(const char *path, const char *buffer, size_t size, off_t offset, struct fuse_file_info *fi) {
	// Step 1: You could call get_node_by_path() to get inode from path

	// Step 2: Based on size and offset, read its data blocks from disk

	// Step 3: Write the correct amount of data from offset to disk

	// Step 4: Update the inode info and write it to disk

	// Note: this function should return the amount of bytes you write to disk
	return size;
}

static int tfs_unlink(const char *path) {

	// Step 1: Use dirname() and basename() to separate parent directory path and target file name

	// Step 2: Call get_node_by_path() to get inode of target file

	// Step 3: Clear data block bitmap of target file

	// Step 4: Clear inode bitmap and its data block

	// Step 5: Call get_node_by_path() to get inode of parent directory

	// Step 6: Call dir_remove() to remove directory entry of target file in its parent directory

	return 0;
}

static int tfs_truncate(const char *path, off_t size) {
	// For this project, you don't need to fill this function
	// But DO NOT DELETE IT!
    return 0;
}

static int tfs_release(const char *path, struct fuse_file_info *fi) {
	// For this project, you don't need to fill this function
	// But DO NOT DELETE IT!
	return 0;
}

static int tfs_flush(const char * path, struct fuse_file_info * fi) {
	// For this project, you don't need to fill this function
	// But DO NOT DELETE IT!
    return 0;
}

static int tfs_utimens(const char *path, const struct timespec tv[2]) {
	// For this project, you don't need to fill this function
	// But DO NOT DELETE IT!
    return 0;
}


static struct fuse_operations tfs_ope = {
	.init		= tfs_init,
	.destroy	= tfs_destroy,

	.getattr	= tfs_getattr,
	.readdir	= tfs_readdir,
	.opendir	= tfs_opendir,
	.releasedir	= tfs_releasedir,
	.mkdir		= tfs_mkdir,
	.rmdir		= tfs_rmdir,

	.create		= tfs_create,
	.open		= tfs_open,
	.read 		= tfs_read,
	.write		= tfs_write,
	.unlink		= tfs_unlink,

	.truncate   = tfs_truncate,
	.flush      = tfs_flush,
	.utimens    = tfs_utimens,
	.release	= tfs_release
};


int main(int argc, char *argv[]) {
	int fuse_stat;

	getcwd(diskfile_path, PATH_MAX);
	strcat(diskfile_path, "/DISKFILE");

	fuse_stat = fuse_main(argc, argv, &tfs_ope, NULL);
	struct inode test;
	test.ino = 15;
	struct inode test2;
	tfs_init(NULL);
	get_avail_blkno();
	get_avail_ino();
	writei(15, &test);
	readi(15, &test2);
	tfs_destroy(NULL);
	return fuse_stat;
}

