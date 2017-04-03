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

typedef union __read_data_4_t {
	int32_t num;
	char bytes[4];
} read_data_4_t;

typedef union __read_data_2_t {
	int16_t num;
	char bytes[2];
} read_data_2_t;

void check_rc(int expected, int actual, char *msg);

int32_t get_val_4_byte(int fd, read_data_4_t *buff, int offset, char *msg);

int16_t get_val_2_byte(int fd, read_data_2_t *buff, int offset, char *msg);


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
	read_data_4_t data_4;
	read_data_2_t data_2;
	memset(&data_4.bytes, 0, 4);
	memset(&data_2.bytes, 0, 2);


	/* SUPERBLOCK */
	int superblock_base = 1024;
	int offset = 0;
	fprintf(stdout, "SUPERBLOCK,");
	/* block count */
	offset = 4;
	int block_count = get_val_4_byte(fd, &data_4, superblock_base + offset, "Failed to read block count data\n");
	fprintf(stdout, "%d,", block_count);
	/* inode count */
	offset = 0;
	int inode_count = get_val_4_byte(fd, &data_4, superblock_base + offset, "Failed to read inode count data\n");
	fprintf(stdout, "%d,", inode_count);
	/* block size */
	offset = 24;
	int block_size = 1024 << get_val_4_byte(fd, &data_4, superblock_base + offset, "Failed to read block size data\n");
	fprintf(stdout, "%d,", block_size);
	/* inode size */
	offset = 88;
	int inode_size = get_val_2_byte(fd, &data_2, superblock_base + offset, "Failed to read inode size data\n");
	fprintf(stdout, "%d,", inode_size);
	/* blocks per group */
	offset = 32;
	int blocks_per_group = get_val_4_byte(fd, &data_4, superblock_base + offset, "Failed to read blocks per group data\n");
	fprintf(stdout, "%d,", blocks_per_group);
	/* inodes per group */
	offset = 40;
	int inodes_per_group = get_val_4_byte(fd, &data_4, superblock_base + offset, "Failed to read inodes per group data\n");
	fprintf(stdout, "%d,", inodes_per_group);
	/* first free inode */
	offset = 84;
	int first_free_inode = get_val_4_byte(fd, &data_4, superblock_base + offset, "Failed to read first free inode data\n");
	fprintf(stdout, "%d\n", first_free_inode);

	/* GROUP SUMMARY */
	int group_descriptor_table_base = superblock_base + block_size;
	int group_descriptor_base;
	int group_count = block_count/blocks_per_group;
	fprintf(stdout, "%d\n", group_count);
	int group_descriptor_size = 32;
	for(int i = 0; i < group_count; ++i) {
		fprintf(stdout, "GROUP,");
		group_descriptor_base = group_descriptor_table_base + i*group_descriptor_size;
		/* group number */
		fprintf(stdout, "%d,", i);
		/* total blocks in this group */
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

int32_t get_val_4_byte(int fd, read_data_4_t *buff, int offset, char *msg) {
	int nbytes = pread(fd, buff->bytes, 4, offset);
	check_rc(4, nbytes, msg);
	return buff->num;
}

int16_t get_val_2_byte(int fd, read_data_2_t *buff, int offset, char *msg) {
	int nbytes = pread(fd, buff->bytes, 2, offset);
	check_rc(2, nbytes, msg);
	return buff->num;
}
