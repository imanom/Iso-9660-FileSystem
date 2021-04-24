#include <fcntl.h>
#include "helper.h"
#include "isoSpec.h"

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
}
