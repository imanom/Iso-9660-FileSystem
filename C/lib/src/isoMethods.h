#include "helper.h"


int open_image_file(VolInfo *volInfo, const char *filename);
int read_volume(VolInfo *volInfo);
int init_volume(VolInfo *volInfo);
int read_directory_tree(VolInfo *volInfo, int filename_type);
int read_directory(VolInfo *volInfo, Dir *dir, int filename_type);
int read_dir_contents(VolInfo *volInfo, Dir *dir, unsigned size, int filenameType);
int read_file(VolInfo *volInfo, File *file, int filename_type);
void check_kernel_file(unsigned int size, off_t loc_extent);
