#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include <assert.h>

#include "fat.h"
#include "direntry.h"


/* Prototypes for blockdev_file.c functions */
extern struct block_device * 
block_device_file_new(const char *filename, const char *mode);
extern void block_device_file_destroy(struct block_device *bldev);

void print_tree(struct fat_file_handle *dir, int nest)
{
	struct dirent ent;

	while(fat_dir_read(dir, &ent)) {
		for(int i = 0; i < nest; i++) printf("\t");
		printf("%02x - %s\n", ent.fatent.attr, ent.d_name);
		
		if((ent.fatent.attr == FAT_ATTR_DIRECTORY) && (ent.d_name[0] != '.')) {
			struct fat_file_handle subdir;
			fat_file_init(dir->fat, &ent.fatent, &subdir);
			print_tree(&subdir, nest + 1);
		}
	}

}

int main(int argc, char *argv[])
{
	struct block_device *bldev;
	struct fat_vol_handle fat;
	struct fat_file_handle root;

	bldev = block_device_file_new(argc > 1 ? argv[1] : "fat32.img", "r");
	assert(bldev != NULL);

	fat_vol_init(bldev, &fat);
	fat_file_init(&fat, NULL, &root);

	printf("Fat type is FAT%d\n", fat.type);

	print_tree(&root, 0);

	block_device_file_destroy(bldev);
}

