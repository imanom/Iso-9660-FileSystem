#include <unistd.h>
#include <string.h>
#include <stdio.h>

#define off_t long int
#define ino_t unsigned long int

/* number of logical sectors in system area, which is 16 */
#define NLS_SYSTEM_AREA 16
/* number of bytes in a logical block, which is 2K = 2048) */
#define NBYTES_LOGICAL_BLOCK 2048
/* for el torito boot images */
#define NBYTES_VIRTUAL_SECTOR 512
#define NCHARS_FILE_ID_MAX_STORE 256

/* numbers as recorded on image */
#define VDTYPE_BOOT 0
#define VDTYPE_PRIMARY 1
#define VDTYPE_SUPPLEMENTARY 2
#define VDTYPE_VOLUMEPARTITION 3
#define VDTYPE_TERMINATOR 255

#define FNTYPE_9660 1

/* options for VolInfo.bootMediaType */
#define BOOT_MEDIA_NONE 0
#define BOOT_MEDIA_NO_EMULATION 1
#define BOOT_MEDIA_1_2_FLOPPY 2  /* 1.2 meg diskette */
#define BOOT_MEDIA_1_44_FLOPPY 3 /* 1.44 meg diskette  */
#define BOOT_MEDIA_2_88_FLOPPY 4 /* 2.88 meg diskette  */
#define BOOT_MEDIA_HARD_DISK 5

/**
* FileBase
* Linked list node.
**/
typedef struct FileBase
{
    char original9660name[15];
    char name[NCHARS_FILE_ID_MAX_STORE]; /* '\0' terminated */
    unsigned posixFileMode;              /* file type and permissions */
    struct FileBase *next;

} FileBase;

/**
* File
* Linked list node.
**/
typedef struct File
{
    FileBase base;
    unsigned size;
    off_t position;
    char *pathAndName;
} File;

/**
* Dir
* Linked list node.
* Information about a directory and it's contents. 
**/
typedef struct Dir
{
    FileBase root;
    FileBase *child;

} Dir;

/**
* VolInfo
* Information about a volume (one image).
* Strings are '\0' terminated. 
**/
typedef struct VolInfo
{
    unsigned filenameTypes;
    off_t pRootDrOffset; /* primary (9660) */
    off_t bootRecordSectorNumberOffset;
    int imageForReading;
    const File *bootRecordOnImage; /* pointer to the file in the directory tree */
    long int creationTime;
    Dir dirTree;
    unsigned char bootMediaType;
    unsigned bootRecordSize;     /* in bytes */
    off_t bootRecordOffset;      /* if on image */
    char *bootRecordPathAndName; /* if on filesystem */
    char volId[33];
    char publisher[129];
    char dataPreparer[129];

} VolInfo;
