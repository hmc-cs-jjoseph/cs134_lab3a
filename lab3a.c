/* \author Jesse Joseph
 * \ID 040161840
 * \email jjoseph@hmc.edu
 */

#define _XOPEN_SOURCE 500
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <string.h>

typedef union __read_data_t {
	int16_t int16;
	int32_t int32;
	int64_t int64;
	char bytes[4096];
} read_data_t;

void check_rc(int expected, int actual, char *msg);

void get_val(int fd, read_data_t *buff, int size, int offset, char *msg);

int count_bits(char* bitmap, int bitmap_size_bytes);

int main(int argc, char **argv) {
	if(argc < 2) {
		fprintf(stderr, "usage:\n./lab3a <filename>\n");
		exit(1);
	}
	char *filename = argv[1];
	int fd = open(filename, O_RDONLY);
	if(fd < 0) {
		perror("Couldn't open input filesystem image");
		exit(1);
	}
	read_data_t data;
	memset(&data.bytes, 0, 1024);


	/* SUPERBLOCK */
	int superblock_base = 1024;
	int offset = 0;
	fprintf(stdout, "SUPERBLOCK,");
	/* block count */
	offset = 4;
	get_val(fd, &data, 4, superblock_base + offset, "Failed to read block count data\n");
	int block_count = data.int32;
	fprintf(stdout, "%d,", block_count);
	/* inode count */
	offset = 0;
	get_val(fd, &data, 4,  superblock_base + offset, "Failed to read inode count data\n");
	int inode_count = data.int32;
	fprintf(stdout, "%d,", inode_count);
	/* block size */
	offset = 24;
	get_val(fd, &data, 4, superblock_base + offset, "Failed to read block size data\n");
	int block_size = 1024 << data.int32;
	fprintf(stdout, "%d,", block_size);
	/* inode size */
	offset = 88;
	get_val(fd, &data, 2, superblock_base + offset, "Failed to read inode size data\n");
	int inode_size = data.int16;
	fprintf(stdout, "%d,", inode_size);
	/* blocks per group */
	offset = 32;
	get_val(fd, &data, 4,  superblock_base + offset, "Failed to read blocks per group data\n");
	int blocks_per_group = data.int32;
	fprintf(stdout, "%d,", blocks_per_group);
	/* inodes per group */
	offset = 40;
	get_val(fd, &data, 4,  superblock_base + offset, "Failed to read inodes per group data\n");
	int inodes_per_group = data.int32;
	fprintf(stdout, "%d,", inodes_per_group);
	/* first free inode */
	offset = 84;
	get_val(fd, &data, 4, superblock_base + offset, "Failed to read first free inode data\n");
	int first_free_inode = data.int32;
	fprintf(stdout, "%d\n", first_free_inode);

	/* GROUP SUMMARY */
	int group_descriptor_table_base = superblock_base + block_size;
	int group_descriptor_base;
	int group_count = block_count/blocks_per_group;
	if(block_count % blocks_per_group != 0) {
		group_count = group_count + 1;
	}
	int group_descriptor_size = 32;
	for(int i = 0; i < group_count; ++i) {
		fprintf(stdout, "GROUP,");
		group_descriptor_base = group_descriptor_table_base + i*group_descriptor_size;
		/* group number */
		fprintf(stdout, "%d,", i);
		/* total blocks in this group */
		offset = 0;
		get_val(fd, &data, 4, group_descriptor_base + offset, "Failed to read block ID of group block bitmap\n");
		int block_bitmap_bid = data.int32;
		int block_bitmap_bytes_size = blocks_per_group/8;
		get_val(fd, &data, block_bitmap_bytes_size, block_bitmap_bid*block_size, "Failed to read block bitmap\n");
		int used_blocks = count_bits(&data.bytes, block_bitmap_bytes_size);
		fprintf(stdout, "\nblock_bitmap_bid = %d\n", block_bitmap_bid);
		fprintf(stdout, "%d,", blocks_per_group);
		/* total inodes in this group */
		fprintf(stdout, "%d,", inodes_per_group);
		
		fprintf(stdout, "\n");
	}


	if(close(fd) < 0) {
		perror("Couldn't close filesystem image file");
		exit(1);
	}
	return 0;
}


void check_rc(int expected, int actual, char *msg) {
	if(actual < expected) {
		fprintf(stderr, "%s", msg);
		exit(1);
	}
}

void get_val(int fd, read_data_t *buff, int size, int offset, char *msg) {
	int nbytes = pread(fd, buff->bytes, size, offset);
	check_rc(size, nbytes, msg);
}


int count_bits(char* bitmap, int bitmap_size_bytes) {
	int counter = 0;
	for(int i = 0; i < bitmap_size_bytes; ++i) {
		char bitmap_byte = bitmap[i];
		while(bitmap_byte) {
			counter += bitmap_byte % 2;
			bitmap_byte = bitmap_byte >> 1;
		}
	}
	return counter;
}
