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
#include <time.h>
#include <inttypes.h>

typedef union __read_data_t {
	uint8_t int8;
	uint16_t int16;
	uint32_t int32;
	char bytes[4096];
} read_data_t;

void check_rc(int expected, int actual, char *msg);

void get_val(int fd, read_data_t *buff, int size, int offset, char *msg);

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
	unsigned int block_count = data.int32;
	fprintf(stdout, "%d,", block_count);
	/* inode count */
	offset = 0;
	get_val(fd, &data, 4,  superblock_base + offset, "Failed to read inode count data\n");
	unsigned int inode_count = data.int32;
	fprintf(stdout, "%d,", inode_count);
	/* block size */
	offset = 24;
	get_val(fd, &data, 4, superblock_base + offset, "Failed to read block size data\n");
	unsigned int block_size = 1024 << data.int32;
	fprintf(stdout, "%d,", block_size);
	/* inode size */
	offset = 88;
	get_val(fd, &data, 2, superblock_base + offset, "Failed to read inode size data\n");
	unsigned int inode_size = data.int16;
	fprintf(stdout, "%d,", inode_size);
	/* blocks per group */
	offset = 32;
	get_val(fd, &data, 4,  superblock_base + offset, "Failed to read blocks per group data\n");
	unsigned int blocks_per_group = data.int32;
	fprintf(stdout, "%d,", blocks_per_group);
	/* inodes per group */
	offset = 40;
	get_val(fd, &data, 4,  superblock_base + offset, "Failed to read inodes per group data\n");
	unsigned int inodes_per_group = data.int32;
	fprintf(stdout, "%d,", inodes_per_group);
	/* first free inode */
	offset = 84;
	get_val(fd, &data, 4, superblock_base + offset, "Failed to read first free inode data\n");
	unsigned int first_free_inode = data.int32;
	fprintf(stdout, "%d\n", first_free_inode);

	/* GROUP SUMMARY */
	unsigned int group_descriptor_table_base = superblock_base + block_size;
	unsigned int group_descriptor_base;
	unsigned int group_count = block_count/blocks_per_group;
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
		unsigned int blocks_in_this_group;
		if(i == group_count - 1) {
			blocks_in_this_group = block_count % blocks_per_group;
		} else {
			blocks_in_this_group = blocks_per_group;
		}
		fprintf(stdout, "%d,", blocks_in_this_group);

		/* total inodes in this group */
		fprintf(stdout,"%d,", inodes_per_group);

		/* free blocks in this group */
		offset = 12;
		get_val(fd, &data, 2, group_descriptor_base + offset, "Failed to read number of free blocks in group\n");
		unsigned int free_blocks_count = data.int16;
		fprintf(stdout, "%d,", free_blocks_count);

		/* free inodes in this group */
		offset = 14;
		get_val(fd, &data, 2, group_descriptor_base + offset, "Failed to read number of free inodes in group\n");
		unsigned int free_inodes_count = data.int16;
		fprintf(stdout, "%d,", free_inodes_count);
		
		/* block id of free block bitmap */
		offset = 0;
		get_val(fd, &data, 4, group_descriptor_base + offset, "Failed to read block ID of group block bitmap\n");
		unsigned int block_bitmap_bid = data.int32;
		fprintf(stdout, "%d,", block_bitmap_bid);

		/* block id of free inode bitmap */
		offset = 4;
		get_val(fd, &data, 4, group_descriptor_base + offset, "Failed to read block ID of group inode bitmap\n");
		unsigned int inode_bitmap_bid = data.int32;
		fprintf(stdout, "%d,", inode_bitmap_bid);

		/* block id of inode table */
		offset = 8;
		get_val(fd, &data, 4, group_descriptor_base + offset, "Failed to read block ID of group inode table\n");
		unsigned int inode_table_bid = data.int32;
		fprintf(stdout, "%d\n", inode_table_bid);
		

		/* FREE BLOCK ENTRIES */
		unsigned int block_bitmap_bytes_size = blocks_in_this_group/8 + (blocks_in_this_group % 8 != 0);
		get_val(fd, &data, block_bitmap_bytes_size, block_bitmap_bid*block_size, "Failed to read block bitmap\n");
		int counter = 0;
		for(int i = 0; i < block_bitmap_bytes_size; ++i) {
			uint8_t bitmap_byte = data.bytes[i];
			for(int j = 0; j < 8; ++j) {
				counter ++;
				if(bitmap_byte % 2 == 0) {
					fprintf(stdout, "BFREE,%d\n", counter);
				}
				bitmap_byte /= 2;
			}
		}

		/* FREE INODE ENTRIES */
		unsigned int inode_bitmap_bytes_size = inodes_per_group/8 + (inodes_per_group % 8 != 0);
		get_val(fd, &data, inode_bitmap_bytes_size, inode_bitmap_bid*block_size, "Failed to read block bitmap\n");
		counter = 0;
		for(int i = 0; i < inode_bitmap_bytes_size; ++i) {
			uint8_t bitmap_byte = data.bytes[i];
			for(int j = 0; j < 8; ++j) {
				counter ++;
				if(bitmap_byte % 2 == 0) {
					fprintf(stdout, "IFREE,%d\n", counter);
				}
				bitmap_byte /= 2;
			}
		}

		/* INODE SUMMARY */
		for(int i = 0; i < inodes_per_group; ++ i) {
			unsigned int inode_base = inode_table_bid*block_size + i*inode_size;

			/* mode and link count */
			offset = 0;
			get_val(fd, &data, 2, inode_base + offset, "Failed to read inode mode\n");
			unsigned int inode_mode = data.int16;
			offset = 26;
			get_val(fd, &data, 2, inode_base + offset, "Failed to read inode link count\n");
			unsigned int link_count = data.int16;

			if(link_count && inode_mode) {
				fprintf(stdout, "INODE,");

				/* inode number */
				fprintf(stdout, "%d,", i+1);

				/* file type */
				char file_type;
				int file_format_bitmask = 0xF000;
				int mode_type = inode_mode & file_format_bitmask;
				if(mode_type == 0x8000) {
					file_type = 'f';
				} else if(mode_type == 0x4000) {
					file_type = 'd';
				} else if(mode_type == 0xA000) {
					file_type = 's';
				} else {
					file_type = '?';
				}
				fprintf(stdout, "%c,", file_type);

				/* mode */
				fprintf(stdout, "%o,", inode_mode & 0xFFF);

				/* owner */
				offset = 2;
				get_val(fd, &data, 2, inode_base + offset, "Failed to read inode owner\n");
				unsigned int owner = data.int16;
				fprintf(stdout, "%d,", owner);

				/* group */
				offset = 24;
				get_val(fd, &data, 2, inode_base + offset, "Failed to read inode owner\n");
				unsigned int gid = data.int16;
				fprintf(stdout, "%d,", gid);

				/* link count */
				fprintf(stdout, "%d,", link_count);

				/* creation time */
				offset = 12;
				get_val(fd, &data, 4, inode_base + offset, "Failed to read inode owner\n");
				time_t creation_secs_since_epoch = data.int32;
				struct tm *creation_time = gmtime(&creation_secs_since_epoch);
				char timebuff[19];
				strftime(timebuff, 19, "%D %T,", creation_time);
				fprintf(stdout, "%s", timebuff);

				/* modification time */
				offset = 16;
				get_val(fd, &data, 4, inode_base + offset, "Failed to read inode owner\n");
				time_t mod_secs_since_epoch = data.int32;
				struct tm *mod_time = gmtime(&mod_secs_since_epoch);
				strftime(timebuff, 19, "%D %T,", mod_time);
				fprintf(stdout, "%s", timebuff);

				/* access time */
				offset = 8;
				get_val(fd, &data, 4, inode_base + offset, "Failed to read inode owner\n");
				time_t access_secs_since_epoch = data.int32;
				struct tm *access_time = gmtime(&access_secs_since_epoch);
				strftime(timebuff, 19, "%D %T,", access_time);
				fprintf(stdout, "%s", timebuff);

				/* size */
				offset = 4;
				get_val(fd, &data, 4, inode_base + offset, "Failed to read inode owner\n");
				int file_size = data.int32;
				fprintf(stdout, "%d,", file_size);

				/* number of blocks */
				offset = 28;
				get_val(fd, &data, 4, inode_base + offset, "Failed to read inode owner\n");
				unsigned int number_of_blocks = data.int32;
				fprintf(stdout, "%d,", number_of_blocks);

				/* block addresses */
				unsigned int block_addresses[15];
				int j;
				for(j = 0; j < 15; ++j) {
					offset = 40 + j*4;
					get_val(fd, &data, 4, inode_base + offset, "Failed to read inode block address\n");
					block_addresses[j] = data.int32;
					if(block_addresses[j] == 0) {
						break;
					}
					fprintf(stdout, "%d", block_addresses[j]);
					if(j != 14) {
						fprintf(stdout, ",");
					} else {
						fprintf(stdout, "\n");
					}
				}
				/* according to the ext2 docs, after the first 0 is encountered,
				 * all the following block addresses are invalid
				 */
				for(; j < 15; ++j) {
					if(j != 14) {
						fprintf(stdout, "0,");
					} else {
						fprintf(stdout, "0\n");
					}
				}
				
				/* DIRECTORY ENTRIES */
				if(file_type == 'd') {
					unsigned int block_address_base;
					unsigned int dirent_base;
					unsigned int inode_number;
					unsigned int rec_length;
					unsigned int name_len;
					for(int j = 0; j < 12; ++j) {
						block_address_base = block_addresses[j]*block_size;
						dirent_base = 0;
						offset = 0;
						get_val(fd, &data, 4, block_address_base + dirent_base + offset, "Failed to read dirent inode number\n");
						inode_number = data.int32;
						/* record length */
						offset = 4;
						get_val(fd, &data, 2, block_address_base + dirent_base + offset, "Failed to read dirent record length\n");
						rec_length = data.int16;
						while((dirent_base < block_size) && (rec_length != 0) && (inode_number != 0)) {
							offset = 0;
							get_val(fd, &data, 4, block_address_base + dirent_base + offset, "Failed to read dirent inode number\n");
							inode_number = data.int32;

							/* record length */
							offset = 4;
							get_val(fd, &data, 2, block_address_base + dirent_base + offset, "Failed to read dirent record length\n");
							rec_length = data.int16;

							/* name length */
							offset = 6;
							get_val(fd, &data, 1, block_address_base + dirent_base + offset, "Failed to read dirent name length\n");
							name_len = data.int8;

							if(inode_number != 0) {
								fprintf(stdout, "DIRENT,");
								
								/* parent inode number */
								fprintf(stdout, "%d,", i+1);

								/* byte offset of this entry */
								fprintf(stdout, "%d,", dirent_base);

								/* inode number of referenced file */
								fprintf(stdout, "%d,", inode_number);
								
								/* record length */
								fprintf(stdout, "%d,", rec_length);

								/* name length */
								fprintf(stdout, "%d,", name_len);

								/* name */
								memset(&data.bytes, 0, 1024);
								offset = 8;
								get_val(fd, &data, name_len, block_address_base + dirent_base + offset, "Failed to read dirent file name\n");
								fprintf(stdout, "'%s'\n", data.bytes);
							}
							dirent_base += rec_length;
						}
					}
				}
				/* INDIRECT REFERENCES */
				if(file_type == 'f' || file_type == 'd') {
					for(int k = 0; k < 3; ++k){
						/* singly indirect */
						unsigned int indirect_block_address = block_addresses[12+k];
						if(indirect_block_address != 0) {
							unsigned int indirect_block_base = indirect_block_address*block_size;
							for(int m = 0; m < block_size/4; ++m) {
								unsigned int offset = m*4;
								get_val(fd, &data, 4, indirect_block_base + offset, "Failed to read indirect block pointer\n");
								unsigned int block_pointer = data.int32;
								if(block_pointer != 0) {
									fprintf(stdout, "INDIRECT,");

									/* parent inode number */
									fprintf(stdout, "%d,", i+1);

									/* indirection level */
									fprintf(stdout, "%d,", 1);

									/* file offset */
									fprintf(stdout, "%d,", 12+m);

									/* block number of the indirect block being scanned */
									fprintf(stdout, "%d,", indirect_block_address);

									/* block number of the referenced block */
									fprintf(stdout, "%d\n", block_pointer);
								}
							}
						}
					}
				}
			}
		}
	}


	if(close(fd) < 0) {
		perror("Couldn't close filesystem image file");
		exit(1);
	}
	return 0;
}

void check_rc(int expected, int actual, char *msg) {
	if(actual < expected) {
		perror(msg);
		exit(1);
	}
}

void get_val(int fd, read_data_t *buff, int size, int offset, char *msg) {
	memset(buff->bytes, 0, size);
	int nbytes = pread(fd, buff->bytes, size, offset);
	check_rc(size, nbytes, msg);
}

