#pragma once

#define NLS_SYSTEM_AREA 16

//Offsets inside Primary volume descriptor
#define VDTYPE_PRIMARY 1
#define root_dir_offset 156

//Offsets inside directory structure
#define loc_offset 2
#define len_offset 10
#define skip_dirs 68    //To skip '.' and '..' directories
#define flag_offset 25


/**
* VolInfo
* Information about a volume (one image).
* Included only info that is relevant to us
**/
typedef struct VolInfo
{
    int vol_descriptor_type;
    void* pRootDrOffset; // root directory offset in PVD
    void* Dir;          // Directory record (can be Dir or File)
    int recordLength;   //length of record (Dir or File)
    void* kernel;       // Kernel 

} VolInfo;



static inline void convertBothEndian(unsigned char *array, unsigned *value)
{
	*value = array[3];
	*value <<= 8;
	*value |= array[2];
	*value <<= 8;
	*value |= array[1];
	*value <<= 8;
	*value |= array[0];
}

static inline int isDirOrFile(char *volInfo)
{
	unsigned char file_flags = volInfo[0];

	if ((file_flags & 2) == 2)
		return 2;
	else
		return 1;
}
