/*
 * This file is part of the openfat project.
 *
 * Copyright (C) 2011  Department of Physics, University of Otago
 * Written by Gareth McMullin <gareth@blacksphere.co.nz>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

/* This program is an unfinished diagnostic tool for investigating the
 * sector level contents of a FAT filesystem.  It was written to aid
 * debugging during the development of OpenFAT.  It is not expected to
 * be useful to the user, but is included for completeness, and may be
 * of use to future developers.  It currently only works with FAT32 
 * filesystems with 2 fats.
 */

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>

#include <assert.h>

#include <alloca.h>

#include <sys/stat.h>

#include <gtk/gtk.h>

#include "openfat.h"
#include "openfat/unixlike.h"

#include "../src/bpb.h"
#include "../src/fat_core.h"

/* Prototypes for blockdev_file.c functions */
extern struct block_device * 
block_device_file_new(const char *filename, const char *mode);
extern void block_device_file_destroy(struct block_device *bldev);

void dump_bootsector(struct fat_vol_handle *vol)
{
	uint8_t bootsector[512];
	struct bpb_common *bpb = (void*)bootsector;
	struct bpb_fat32 *bpb32 = (void*)bootsector;

	block_read_sectors(vol->dev, 0, 1, bootsector);

	fprintf(stdout, "boot_jmp:\t\t %02X %02X %02X\n", 
			bpb->boot_jmp[0], bpb->boot_jmp[1], bpb->boot_jmp[2]);
	fprintf(stdout, "oem_name:\t\t "); 
	for(int i = 0; i < 8; i++)
		fprintf(stdout, "%c", bpb->oem_name[i]);
	fprintf(stdout, "\nbytes_per_sector:\t %d\n", 
			bpb->bytes_per_sector);
	fprintf(stdout, "sectors_per_cluster:\t %d\n", 
			bpb->sectors_per_cluster);
	fprintf(stdout, "reserved_sector_count:\t %d\n", 
			bpb->reserved_sector_count);
	fprintf(stdout, "num_fats:\t\t %d\n", 
			bpb->num_fats);
	fprintf(stdout, "root_entry_count:\t %d\n", 
			bpb->root_entry_count);
	fprintf(stdout, "total_sectors_16:\t %d\n", 
			bpb->total_sectors_16);
	fprintf(stdout, "media:\t\t\t 0x%02X\n", 
			bpb->media);
	fprintf(stdout, "fat_size_16:\t\t %d\n", 
			bpb->fat_size_16);
	fprintf(stdout, "sectors_per_track:\t %d\n", 
			bpb->sectors_per_track);
	fprintf(stdout, "num_heads:\t\t %d\n", 
			bpb->num_heads);
	fprintf(stdout, "hidden_sectors:\t\t %d\n", 
			bpb->hidden_sectors);
	fprintf(stdout, "total_sectors_32:\t %d\n", 
			bpb->total_sectors_32);
	switch(vol->type) {
	case FAT_TYPE_FAT32:
		fprintf(stdout, "fat_size_32:\t\t %d\n", 
				bpb32->fat_size_32);
		fprintf(stdout, "ext_flags:\t\t 0x%02X\n", 
				bpb32->ext_flags);
		fprintf(stdout, "fs_version:\t\t %d\n", 
				bpb32->fs_version);
		fprintf(stdout, "root_cluster:\t\t %d\n", 
				bpb32->root_cluster);
		fprintf(stdout, "fs_info:\t\t %d\n", 
				bpb32->fs_info);
		fprintf(stdout, "bk_boot_sec:\t\t 0x%X\n", 
				bpb32->bk_boot_sec);
		fprintf(stdout, "reserved:\t\t "); 
		for(int i = 0; i < 12; i++)
			fprintf(stdout, "%02X ", bpb32->Reserved[i]);
		fprintf(stdout, "\ndrive_num:\t\t %d\n", 
				bpb32->drive_num);
		fprintf(stdout, "boot_sig:\t\t %d\n", 
				bpb32->boot_sig);
		fprintf(stdout, "volume_id:\t\t 0x%X\n", 
				bpb32->volume_id);
		fprintf(stdout, "volume_label:\t\t "); 
		for(int i = 0; i < 11; i++)
			fprintf(stdout, "%c", bpb32->volume_label[i]);
		fprintf(stdout, "\nfs_type:\t\t "); 
		for(int i = 0; i < 8; i++)
			fprintf(stdout, "%c", bpb32->fs_type[i]);
		fprintf(stdout, "\n\n"); 
		break;
		
	}
	fprintf(stdout, "first_data_sector:\t %d\n", 
			vol->first_data_sector);
	fprintf(stdout, "cluster_count:\t\t %d\n", 
			vol->cluster_count);
	fprintf(stdout, "fat_size:\t\t %d\n", 
			vol->fat_size);
	fprintf(stdout, "root_cluster:\t\t %d\n", 
			vol->fat32.root_cluster);
}

GtkWidget *setup_fat_view(struct fat_vol_handle *vol)
{
	GtkWidget *sw, *tv;
	GtkListStore *model;
	GtkTreeIter iter;
	GtkTreeViewColumn *col;
	GtkCellRenderer *cell;

	uint32_t sector;
	uint32_t cluster = 0;
	uint8_t *fat[vol->num_fats];
	int i;

	sw = gtk_scrolled_window_new(NULL, NULL);
	model = gtk_list_store_new(3, 
			G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING);
	tv = gtk_tree_view_new_with_model(GTK_TREE_MODEL(model));
	gtk_tree_view_set_headers_visible(GTK_TREE_VIEW(tv), FALSE);
	gtk_widget_modify_font(tv, 
			pango_font_description_from_string("Monospace 8"));
	gtk_container_add(GTK_CONTAINER(sw), tv);

	cell = GTK_CELL_RENDERER(gtk_cell_renderer_text_new()); 
	col = gtk_tree_view_column_new_with_attributes(NULL, cell,  
			"text", 0, NULL); 
	gtk_tree_view_append_column(GTK_TREE_VIEW(tv), col); 

	for(i = 0; i < vol->num_fats; i++) {
		fat[i] = alloca(vol->bytes_per_sector);

		cell = GTK_CELL_RENDERER(gtk_cell_renderer_text_new()); 
		col = gtk_tree_view_column_new_with_attributes(NULL, cell,  
				"text", i + 1, NULL); 
		gtk_tree_view_append_column(GTK_TREE_VIEW(tv), col); 
	}

	for(sector = 0; sector < vol->fat_size; sector++) {
		for(i = 0; i < vol->num_fats; i++) {
			block_read_sectors(vol->dev, 
				vol->reserved_sector_count + 
				(vol->fat_size * i) + 
				sector, 1, fat[i]);
		}
		for(int j = 0; j < vol->bytes_per_sector; j += 4) {
			char str1[20], str2[20], str3[20];
			gtk_list_store_append(model, &iter);
			sprintf(str1, "0x%08X", cluster++);
			sprintf(str2, "0x%08X",__get_le32((void*)(fat[0] + j)));
			sprintf(str3, "0x%08X",__get_le32((void*)(fat[1] + j)));
			gtk_list_store_set(model, &iter, 
					0, str1, 1, str2, 2, str3, -1);
		}
	}

	return sw;
}


static void gtk_text_view_puts(GtkTextView *tv, const char *buf)
{
	GtkTextBuffer *tb = gtk_text_view_get_buffer(tv);
	GtkTextIter iter;

	gtk_text_buffer_get_end_iter(tb, &iter);
	gtk_text_buffer_insert(tb, &iter, buf, -1);
}

void cluster_changed_cb(GtkComboBox *clust, GtkComboBox *sector)
{
	struct fat_vol_handle *vol = g_object_get_data(G_OBJECT(clust), "vol");
	uint32_t cluster = strtol(gtk_combo_box_get_active_text(clust), 0, 0);
	char sect[20];
	int i;
	puts(sect);
	for(i = 0; i < vol->sectors_per_cluster; i++)
		gtk_combo_box_remove_text(sector, 0);
	for(i = 0; i < vol->sectors_per_cluster; i++) {
		sprintf(sect, "%d", fat_first_sector_of_cluster(vol, cluster)+i);
		gtk_combo_box_append_text(sector, sect);
	}
	gtk_combo_box_set_active(sector, 0);
}

void sector_changed_cb(GtkComboBox *entry, GtkTextView *tv)
{
	struct fat_vol_handle *vol = g_object_get_data(G_OBJECT(entry), "vol");
	uint32_t sector;
	char linebuf[80];
	uint8_t *buf = alloca(vol->bytes_per_sector);
	int i, j, k;

	sector =  strtol(gtk_combo_box_get_active_text(entry), 0, 0);
	block_read_sectors(vol->dev, sector, 1, buf);

	gtk_text_buffer_set_text(gtk_text_view_get_buffer(tv), "", 0);
	memset(linebuf, ' ', 79);
	linebuf[79] = 0;
	linebuf[78] = '\n';
	for(i = 0; i < vol->bytes_per_sector;) {
		k = 0;
		for(j = 0; j < 16; j++, i++) {
			k += sprintf(linebuf+k, "%02X ", buf[i]);
			linebuf[k] = ' ';
			if((j + 1) % 4 == 0) k++;
			if(j == 7) k++;
			linebuf[54+j] = ((buf[i]<32)||(buf[i]>126))?'.':buf[i];
		}
		gtk_text_view_puts(tv, linebuf);
	}
}

GtkWidget *setup_sector_view(struct fat_vol_handle *vol)
{
	GtkWidget *vbox, *hbox, *clust, *entry, *sw, *tv;

	vbox = gtk_vbox_new(FALSE, FALSE);

	hbox = gtk_hbox_new(FALSE, FALSE);
	gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 0);

	clust = gtk_combo_box_entry_new_text();
	g_object_set_data(G_OBJECT(clust), "vol", vol);
	gtk_box_pack_start(GTK_BOX(hbox), clust, FALSE, FALSE, 0);

	entry = gtk_combo_box_entry_new_text();
	g_object_set_data(G_OBJECT(entry), "vol", vol);
	gtk_box_pack_start(GTK_BOX(hbox), entry, FALSE, FALSE, 0);

	sw = gtk_scrolled_window_new(NULL, NULL);
	gtk_box_pack_start(GTK_BOX(vbox), sw, TRUE, TRUE, 0);

	tv = gtk_text_view_new();
	gtk_text_view_set_editable(GTK_TEXT_VIEW(tv), FALSE);
	gtk_widget_modify_font(tv, 
			pango_font_description_from_string("Monospace 8"));
	gtk_container_add(GTK_CONTAINER(sw), tv);
	
	g_signal_connect(clust, "changed", G_CALLBACK(cluster_changed_cb), entry);
	g_signal_connect(entry, "changed", G_CALLBACK(sector_changed_cb), tv);

	return vbox;
}

void setup_ui(struct fat_vol_handle *vol)
{
	GtkWidget *window, *paned;

	window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_default_size(GTK_WINDOW(window), 700, 500);
	gtk_window_set_title(GTK_WINDOW(window), "Sector Inspector");
	g_signal_connect(window, "destroy", gtk_main_quit, NULL);

	paned = gtk_hpaned_new();
	gtk_paned_pack1(GTK_PANED(paned), setup_fat_view(vol), TRUE, TRUE);
	gtk_paned_pack2(GTK_PANED(paned), setup_sector_view(vol), TRUE, TRUE);
	gtk_container_add(GTK_CONTAINER(window), paned);

	gtk_widget_show_all(window);
}


int main(int argc, char *argv[])
{
	struct block_device *bldev;
	struct fat_vol_handle *vol;
	struct fat_file_handle *root, *file;
	char *rootpath = argc > 2 ? argv[2] : "/";

	bldev = block_device_file_new(argc > 1 ? argv[1] : "fat32.img", "r+");
	assert(bldev != NULL);

	vol = ufat_mount(bldev);
	fprintf(stderr, "Fat type is FAT%d\n", vol->type);

	dump_bootsector(vol);

	if((root = ufat_open(vol, rootpath, 0)) == NULL) {
		fprintf(stderr, "Failed to open file: %s\n", rootpath);
		return -1;
	}

	gtk_init(NULL, NULL);
	setup_ui(vol);
	gtk_main();

	ufat_close(root);
	ufat_umount(vol);

	block_device_file_destroy(bldev);
}

