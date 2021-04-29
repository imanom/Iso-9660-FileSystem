#include "isoMethods.h"
#include <unistd.h>
#include <fcntl.h>
/**
 
Open ISO Image

**/
int open_image_file(VolInfo *volInfo, const char *filename)
{
    volInfo->imageForReading = open(filename, O_RDONLY, 0);
    if (volInfo->imageForReading == -1)
        return 0;
    return 1;
}

int init_volume(VolInfo *volInfo)
{
    memset(volInfo, 0, sizeof(VolInfo));
    return 1;
}

/**

Read primary volume descriptor

**/
int read_volume(VolInfo *volInfo)
{
    int rc;
    unsigned char vol_descriptor_type;
    char time[17];

    volInfo->filenameTypes = FNTYPE_9660;

    //skip the first 16 blocks of 2048 Bytes (system area)
    readSeekSet(volInfo, NLS_SYSTEM_AREA * NBYTES_LOGICAL_BLOCK, SEEK_SET);

    rc = readFromImage(volInfo, &vol_descriptor_type, 1); //read the first byte of PVD(type code)
    if (rc != 1)
    {
        printf("error while reading PVD\n");
        return 0;
    }
    if (vol_descriptor_type != VDTYPE_PRIMARY) //type code always 1 for a PVD
    {
        printf("First descriptor is not primary.\n");
        return 0;
    }

    //go to offset 40 of PVD - volume identifier
    readSeekSet(volInfo, 39, SEEK_CUR);

    //read 32 bytes containing the vol. identifier into the volID field
    rc = readFromImage(volInfo, volInfo->volId, 32);
    if (rc != 32)
    {
        printf("Error reading volume ID.\n");
        return 0;
    }
    volInfo->volId[32] = '\0';
    printf("volume id is: %s\n", volInfo->volId);

    //go to offset 156 (40+32+84) of PVD - directory entry for root dir
    readSeekSet(volInfo, 84, SEEK_CUR);
    //store this offset value
    volInfo->pRootDrOffset = readSeekTell(volInfo);

    return 1;
}

int read_directory_tree(VolInfo *volInfo, int filename_type)
{
    if (filename_type == FNTYPE_9660)
        readSeekSet(volInfo, volInfo->pRootDrOffset, SEEK_SET);

    return read_directory(volInfo, &(volInfo->dirTree), filename_type);
}

int read_directory(VolInfo *volInfo, Dir *dir, int filename_type)
{
    int rc;
    unsigned char record_length, len_file_id;
    unsigned loc_extent, len_extent;
    off_t original_pos;
    int lenSU;

    dir->child = NULL;

    //read length of dir record
    rc = readFromImage(volInfo, &record_length, 1);
    if (rc != 1)
    {
        printf("error reading length of dir record\n");
        return 0;
    }

    //skip extended attribute record length
    readSeekSet(volInfo, 1, SEEK_CUR);

    rc = readBothEndian(volInfo, &loc_extent);
    if (rc != 8)
    {
        printf("error reading location of extent\n");
        return 0;
    }

    rc = readBothEndian(volInfo, &len_extent);
    if (rc != 8)
    {
        printf("error reading length of extent\n");
        return 0;
    }
    printf("len of record:%ld\n", record_length);
    printf("loc:%ld\n", loc_extent);
    printf("len:%ld\n", len_extent);
    readSeekSet(volInfo, 14, SEEK_CUR);

    rc = readFromImage(volInfo, &len_file_id, 1);
    if (rc != 1)
    {
        printf("error reading length of file id\n");
        return 0;
    }
    lenSU = record_length - 33 - len_file_id;
    if (len_file_id % 2 == 0)
        lenSU -= 1;

    //read dir name
    if (volInfo->rootRead)
    {
        off_t pos = readSeekTell(volInfo);
        rc = readFromImage(volInfo, ((FileBase *)(dir))->name, len_file_id);
        if (rc != len_file_id)
        {
            printf("error reading 1\n");
            return 0;
        }
        ((FileBase *)(dir))->name[len_file_id] = '\0';
        printf("len of file id: %d\n", len_file_id);
        printf("dir name: %s\n\n", ((FileBase *)(dir))->name);
        if (len_file_id % 2 == 0)
            readSeekSet(volInfo, 1, SEEK_CUR);
    }

    readSeekSet(volInfo, lenSU, SEEK_CUR);
    original_pos = readSeekTell(volInfo);
    readSeekSet(volInfo, loc_extent * NBYTES_LOGICAL_BLOCK, SEEK_SET);
    volInfo->rootRead = true;

    rc = read_dir_contents(volInfo, dir, len_extent, filename_type);
    if (rc < 0)
    {
        printf("error reading dir content\n");
        return 0;
    }

    readSeekSet(volInfo, original_pos, SEEK_SET);
    return record_length;
}

int read_dir_contents(VolInfo *volInfo, Dir *dir, unsigned size, int filenameType)
{
    int rc;
    unsigned bytes_read = 0, child_bytes_read = 0;
    FileBase **nextChild;

    //skip self and parent (. and ..)
    rc = skipDirectory(volInfo);
    if (rc <= 0)
    {
        printf("error skipping directory\n");
        return 0;
    }
    bytes_read += rc;
    rc = skipDirectory(volInfo);
    if (rc <= 0)
    {
        printf("error skipping directory\n");
        return 0;
    }
    bytes_read += rc;

    nextChild = &(dir->child);
    child_bytes_read = 0;
    while (child_bytes_read + bytes_read < size)
    {
        if (isNextRecordInSector(volInfo))
        {
            int record_len;
            rc = isDirOrFile(volInfo);
            if (rc == 2) //is a directory
            {
                *nextChild = malloc(sizeof(Dir));
                if (*nextChild == NULL)
                {
                    printf("out of memory!\n");
                    return 0;
                }
                memset(*nextChild, 0, sizeof(Dir));
                record_len = read_directory(volInfo, (Dir *)nextChild, filenameType);
                if (record_len < 0)
                    return record_len;
            }
            else //is a file
            {
                *nextChild = malloc(sizeof(File));
                if (*nextChild == NULL)
                {
                    printf("out of memory!\n");
                    return 0;
                }
                memset(*nextChild, 0, sizeof(File));
                record_len = read_file(volInfo, (File *)nextChild, filenameType);
                if (record_len < 0)
                    return record_len;
            }
            child_bytes_read += record_len;
            *nextChild = NULL;
        }
        else
        {
            char testByte;
            off_t original_pos;
            do
            {
                original_pos = readSeekTell(volInfo);
                rc = readFromImage(volInfo, &testByte, 1);
                if (rc != 1)
                {
                    printf("error reading test byte\n");
                    return 0;
                }
                if (testByte != 0)
                {
                    readSeekSet(volInfo, original_pos, SEEK_SET);
                    break;
                }
                child_bytes_read += 1;
            } while (child_bytes_read + bytes_read < size);
        }
    }

    return bytes_read;
}

int read_file(VolInfo *volInfo, File *file, int filename_type)
{
    int rc;
    unsigned char record_length;
    unsigned loc_extent, len_extent;
    unsigned char len_file_id;
    int lenSU;

    file->pathAndName = NULL;

    rc = readFromImage(volInfo, &record_length, 1);
    if (rc != 1)
    {
        printf("error reading record length of file\n");
        return 0;
    }
    readSeekSet(volInfo, 1, SEEK_CUR);

    rc = readBothEndian(volInfo, &loc_extent);
    if (rc != 8)
    {
        printf("error reading loc extent of file\n");
        return 0;
    }
    rc = readBothEndian(volInfo, &len_extent);
    if (rc != 8)
    {
        printf("error reading len extent of file\n");
        return 0;
    }

    readSeekSet(volInfo, 14, SEEK_CUR);
    rc = readFromImage(volInfo, &len_file_id, 1);
    if (rc != 1)
    {
        printf("error getting length of file id\n");
        return 0;
    }

    lenSU = record_length - 33 - len_file_id;
    if (len_file_id % 2 == 0)
        lenSU -= 1;

    //now read file name
    char name[len_file_id];
    rc = readFromImage(volInfo, name, len_file_id);
    if (rc != len_file_id)
    {
        printf("error getting file name\n");
        return 0;
    }
    name[len_file_id] = '\0';
    printf("file name:%s\n", name);
    printf("record length: %d\n", record_length);
    printf("file loc: %d\n", loc_extent);
    printf("file len: %d\n\n", len_extent);

    strcpy(((FileBase *)(file))->name, name);
    ((FileBase *)(file))->name[NCHARS_FILE_ID_MAX_STORE - 1] = '\0';

    readSeekSet(volInfo, 0, SEEK_CUR);

    file->size = len_extent;
    file->position = ((off_t)loc_extent) * ((off_t)NBYTES_LOGICAL_BLOCK);

    /**
     * 
     *  Check for kernel file
     * 
     * */
    if (strcmp(name, "KERNEL.;1") == 0)
    {
        check_kernel_file(file->size, loc_extent);
    }
    return record_length;
}

void check_kernel_file(unsigned int size, off_t loc_extent)
{
    printf("\nsize of kernel file: %d bytes\n", size);
    ssize_t s = 1;
    char buff[(size) + 1];
    char file_name[] = {"/home/monami/Downloads/ISO9660_project/boot.iso"};
    int fd;
    int nbyte = sizeof(buff);
    fd = open(file_name, O_RDONLY, 0444);
    printf("fd: %d\n", fd);
    s = pread(fd, &buff, size, loc_extent);
    printf("\nReading kernel file from boot.iso\n");
    printf("size of file: %d\n", s);
}