#include "helper.h"

size_t readFromImage(VolInfo *volInfo, void *dest, size_t numBytes)
{
    return read(volInfo->imageForReading, dest, numBytes);
}

/**
    readSeekSet()
    Seek set for reading from the iso
**/
off_t readSeekSet(VolInfo *volInfo, off_t offset, int origin)
{
    return lseek(volInfo->imageForReading, offset, origin);
}

/**
    readSeekTell()
    Seek tell for reading from the iso
**/
off_t readSeekTell(VolInfo *volInfo)
{
    return lseek(volInfo->imageForReading, 0, SEEK_CUR);
}

int read721(VolInfo *volInfo, unsigned short *value)
{
    int rc;
    unsigned char array[2];

    rc = readFromImage(volInfo, array, 2);
    if (rc != 2)
        return rc;

    *value = array[1];
    *value <<= 8;
    *value |= array[0];

    return rc;
}

int read731(VolInfo *volInfo, unsigned *value)
{
    int rc;
    unsigned char array[4];

    rc = readFromImage(volInfo, array, 4);
    if (rc != 4)
        return rc;

    readFromCharArray(array, value);

    return rc;
}

int read733(VolInfo *volInfo, unsigned *value)
{
    int rc;
    unsigned char both[8];

    rc = readFromImage(volInfo, both, 8);
    if (rc != 8)
        return rc;

    readFromCharArray(both, value);

    return rc;
}

void readFromCharArray(unsigned char *array, unsigned *value)
{
    *value = array[3];
    *value <<= 8;
    *value |= array[2];
    *value <<= 8;
    *value |= array[1];
    *value <<= 8;
    *value |= array[0];
}

//To skip past "." and ".." entries
int skipDirectory(VolInfo *volInfo)
{
    unsigned char len;
    int rc;
    rc = readFromImage(volInfo, &len,1);
    if (rc <= 0)
        return 0;
    readSeekSet(volInfo, len - 1, SEEK_CUR);
    return len;
}

int isDirOrFile(VolInfo *volInfo)
{
    unsigned char file_flags;
    off_t original_pos;
    int rc;

    original_pos = readSeekTell(volInfo);

    readSeekSet(volInfo, 25, SEEK_CUR);
    rc = readFromImage(volInfo, &file_flags,1);
    if (rc != 1)
    {
        printf("error reading file flags!\n");
        return 0;
    }
    readSeekSet(volInfo, original_pos, SEEK_SET);
    if ((file_flags & 2) == 2)
        return 2;
    else
        return 1;
}
int isNextRecordInSector(VolInfo *volInfo)
{
    off_t origPos;
    char testByte;
    int rc;
    
    origPos = readSeekTell(volInfo);
    
    rc = readFromImage(volInfo, &testByte, 1);
    if(rc != 1)
        return false;
    
    readSeekSet(volInfo, origPos, SEEK_SET);
    
    return (testByte == 0) ? false : true;
}