#include "isoSpec.h"

#define size_t unsigned long int

size_t readFromImage(VolInfo *volInfo, void *dest, size_t numBytes);
off_t readSeekSet(VolInfo *volInfo, off_t offset, int origin);
off_t readSeekTell(VolInfo *volInfo);

int readBothEndian(VolInfo *volInfo, unsigned *value);
void readFromCharArray(unsigned char *array, unsigned *value);

int skipDirectory(VolInfo* volInfo);
int isDirOrFile(VolInfo *volInfo);
int isNextRecordInSector(VolInfo *volInfo);