#include <fcntl.h>
#include "isoMethods.h"

/**
 
Open ISO Image

**/
int open_image_file(VolInfo *volInfo, const char *filename)
{
    size_t len;
    volInfo->imageForReading = open(filename, O_RDONLY, 0);
    if (volInfo->imageForReading == -1)
        return 0;
    return 1;
}

/**

Read volume descriptors - mainly the primary vol descriptor
and the boot record

**/
int read_volume(VolInfo *volInfo)
{
    int rc;
    unsigned char vol_descriptor_type;
    char time[17];
    off_t loc_next_descriptor;
    unsigned boot_catalog_loc;
    char elTorito[24];
    unsigned char boot_media;
    unsigned short boot_record_length;
    unsigned boot_record_sector_num;

    volInfo->filenameTypes = FNTYPE_9660;

    //skip the first 16 blocks of 2048 Bytes (system area)
    readSeekSet(volInfo, NLS_SYSTEM_AREA * NBYTES_LOGICAL_BLOCK, SEEK_SET); 

    rc = read711(volInfo, &vol_descriptor_type);    //read the first byte of PVD(type code)
    if(rc!=1)
    {
        printf("error while reading PVD\n");
        return 0;
    }
    if(vol_descriptor_type!= VDTYPE_PRIMARY)    //type code always 1 for a PVD
    {
        printf("First descriptor is not primary.\n");
        return 0;
    }

    //go to offset 40 of PVD - volume identifier
    readSeekSet(volInfo, 39, SEEK_CUR);
    
    //read 32 bytes containing the vol. identifier into the volID field
    rc = readFromImage(volInfo, volInfo->volId, 32);
    if(rc!=32)
    {
        printf("Error reading volume ID.\n");
        return 0;
    }
    volInfo->volId[32] = '\0';
    printf("volume id is: %s\n",volInfo->volId);
    return 1;


}

int init_volume(VolInfo *volInfo)
{
    memset(volInfo, 0, sizeof(VolInfo));
    volInfo->dirTree.root.posixFileMode = 040755;
    return 1;
}
