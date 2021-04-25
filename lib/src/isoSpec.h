#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>


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


/**
* FileBase
* Linked list node.
**/
typedef struct FileBase
{
    char name[NCHARS_FILE_ID_MAX_STORE]; /* '\0' terminated */
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
    off_t pRootDrOffset; /* primary VD (for 9660). secondary VD not needed for iso 9660*/
    int imageForReading;
    bool rootRead;
    long int creationTime;
    Dir dirTree;
    char volId[33];
    char publisher[129];
    char dataPreparer[129];

} VolInfo;
