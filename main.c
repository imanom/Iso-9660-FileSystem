#include "./lib/src/isoMethods.h"

void test(const char *fileName);

int main()
{
    const char *iso_filename = "./boot.iso";
    test(iso_filename);
    return 1;
}

void test(const char *fileName)
{
    int rc;
    VolInfo volInfo;
    rc = init_volume(&volInfo);
    if (rc <= 0)
        printf("error initializing volume\n");
    rc = open_image_file(&volInfo, fileName);
    if (rc <= 0)
        printf("error opening iso file\n");
    rc = read_volume(&volInfo);
    if (rc <= 0)
        printf("error reading iso\n");
}