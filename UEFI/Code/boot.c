/*
 * boot.c - a boot loader template (Assignment 1, ECE 6504)
 * Copyright 2021 Ruslan Nikolaev <rnikola@vt.edu>
 */

#include <Uefi.h>
#include <Protocol/LoadedImage.h>
#include <Protocol/GraphicsOutput.h>
#include <Protocol/BlockIo.h>
#include <isoMethods.h>

/* Use GUID names 'gEfi...' that are already declared in Protocol headers. */
EFI_GUID gEfiLoadedImageProtocolGuid = EFI_LOADED_IMAGE_PROTOCOL_GUID;
EFI_GUID gEfiGraphicsOutputProtocolGuid = EFI_GRAPHICS_OUTPUT_PROTOCOL_GUID;
EFI_GUID gEfiBlockIoProtocolGuid = EFI_BLOCK_IO_PROTOCOL_GUID;

/* Keep these variables global. */
static EFI_HANDLE ImageHandle;
static EFI_SYSTEM_TABLE *SystemTable;
static EFI_BOOT_SERVICES *BootServices;

static VOID *AllocatePages(UINTN pages, EFI_MEMORY_TYPE type)
{
	EFI_PHYSICAL_ADDRESS ptr = 0;

	EFI_STATUS ret = BootServices->AllocatePages(AllocateAnyPages, type, pages, &ptr);
	if (EFI_ERROR(ret))
		return NULL;

	return (VOID *)ptr;
}

static UINT32 *SetGraphicsMode(UINT32 width, UINT32 height)
{
	EFI_GRAPHICS_OUTPUT_PROTOCOL *graphics;
	EFI_STATUS efi_status;
	UINT32 mode;

	efi_status = BootServices->LocateProtocol(&gEfiGraphicsOutputProtocolGuid,
											  NULL, (VOID **)&graphics);
	if (EFI_ERROR(efi_status))
	{
		SystemTable->ConOut->OutputString(SystemTable->ConOut,
										  L"Cannot get the GOP handle!\r\n");
		BootServices->Stall(5 * 1000000);
		return NULL;
	}

	if (!graphics->Mode || graphics->Mode->MaxMode == 0)
	{
		SystemTable->ConOut->OutputString(SystemTable->ConOut,
										  L"Incorrect GOP mode information!\r\n");
		BootServices->Stall(5 * 1000000);
		return NULL;
	}

	for (mode = 0; mode < graphics->Mode->MaxMode; mode++)
	{
		EFI_GRAPHICS_OUTPUT_MODE_INFORMATION *info;
		UINTN size;

		// TODO: Locate BlueGreenRedReserved, aka BGRA (8-bit per color)
		// Resolution width x height (800x600 in our code)

		// Activate (set) this graphics mode
		efi_status = graphics->QueryMode(graphics, mode, &size, &info);

		if (EFI_ERROR(efi_status))
		{
			SystemTable->ConOut->OutputString(SystemTable->ConOut,
											  L"ERROR: Bad response from QueryMode\r\n");
			BootServices->Stall(5 * 1000000);
			continue;
		}

		switch (info->PixelFormat)
		{

		case PixelBlueGreenRedReserved8BitPerColor:
			if (info->HorizontalResolution == width && info->VerticalResolution == height)
			{
				graphics->SetMode(graphics, mode);
			}
			else
			{
				continue;
			}

			break;
		default:
			continue;
		}

		// Return the frame buffer base address
		return (UINT32 *)graphics->Mode->FrameBufferBase;
	}
	return NULL;
}

static VOID setExitBootServices()
{
	EFI_STATUS efi_status;
	UINTN mapSize = 0, mapKey, descriptorSize;
	EFI_MEMORY_DESCRIPTOR *memoryMap = NULL;
	UINT32 descriptorVersion;

	//Memory map

	do
	{
		efi_status = BootServices->GetMemoryMap(&mapSize, memoryMap, &mapKey, &descriptorSize, &descriptorVersion);
		if (efi_status == EFI_BUFFER_TOO_SMALL)
		{
			UINTN pages = EFI_SIZE_TO_PAGES(mapSize + 1);
			memoryMap = AllocatePages(pages, EfiLoaderData);
			efi_status = BootServices->GetMemoryMap(&mapSize, memoryMap, &mapKey, &descriptorSize, &descriptorVersion);
		}

	} while (efi_status != EFI_SUCCESS);

	//Exit boot services

	while (1)
	{
		efi_status = BootServices->ExitBootServices(ImageHandle, mapKey);
		if (efi_status == EFI_SUCCESS)
		{
			break;
		}
		if (efi_status != EFI_INVALID_PARAMETER)
		{
			SystemTable->ConOut->OutputString(SystemTable->ConOut,
											  L"Exit Boot Services error!\r\n");
			BootServices->Stall(5 * 1000000);
			break;
		}
	}
}

static VOID *handleBlockIO()
{
	EFI_BLOCK_IO *bio;
	EFI_HANDLE *handles = NULL;
	UINTN handleSize = 0;
	EFI_STATUS efi_status;
	UINTN num = 1024;
	VolInfo vol;

	VOID *buffer = NULL;
	int flag;
	unsigned char *array_loc, *array_len;
	unsigned loc = 0, len = 0;

	do
	{
		handles = AllocatePages(num, EfiBootServicesData);
		handleSize = num * sizeof(EFI_HANDLE);
		efi_status = BootServices->LocateHandle(ByProtocol, &gEfiBlockIoProtocolGuid, NULL, &handleSize, handles);
		num += 1024;
	} while (efi_status == EFI_BUFFER_TOO_SMALL || handleSize == 0);

	for (UINTN i = 0; i < handleSize; i++)
	{
		efi_status = BootServices->HandleProtocol(handles[i], &gEfiBlockIoProtocolGuid, (VOID **)&bio);
		if (EFI_ERROR(efi_status))
			continue;
		if (bio && bio->Media && !bio->Media->LogicalPartition && bio->Media->MediaPresent)
		{
			UINTN pages = EFI_SIZE_TO_PAGES(bio->Media->BlockSize);
			buffer = AllocatePages(pages, EfiBootServicesData);

			//Skip the system area (first 16 blocks) and locate the PVD
			efi_status = bio->ReadBlocks(bio, bio->Media->MediaId, (EFI_LBA)NLS_SYSTEM_AREA, bio->Media->BlockSize, buffer);
			if (!EFI_ERROR(efi_status))
			{
				vol.vol_descriptor_type = ((char *)(buffer))[0]; //read 1st byte
				if (vol.vol_descriptor_type == VDTYPE_PRIMARY)	 //check if we are in PVD
				{
					vol.pRootDrOffset = buffer + root_dir_offset; //locate root directory in PVDs
					array_loc = vol.pRootDrOffset + loc_offset;	  //read LBA address of first directory
					array_len = vol.pRootDrOffset + len_offset;	  //read size of extent of first directory

					convertBothEndian(array_loc, &loc);
					convertBothEndian(array_len, &len);
				}
				else
					break;
			}

			//Now that we have the root directory located, iterate through the directories till you reach the first file
			do
			{

				pages = EFI_SIZE_TO_PAGES(len);
				buffer = AllocatePages(pages, EfiBootServicesData);

				efi_status = bio->ReadBlocks(bio, bio->Media->MediaId, (EFI_LBA)(loc), len, buffer);
				if (!EFI_ERROR(efi_status))
				{
					vol.Dir = buffer + skip_dirs;
					array_loc = vol.Dir + loc_offset; //LBA address of child Dir/File record
					array_len = vol.Dir + len_offset; //size of extent of child Dir/File record

					convertBothEndian(array_loc, &loc);
					convertBothEndian(array_len, &len);

					vol.recordLength = ((char *)vol.Dir)[0]; //length of current Dir/File record
				}
				flag = isDirOrFile((char *)(vol.Dir + flag_offset));

			} while (flag == 2); //iterate through the directories (2=Dir, 1=File)

			//Found the first file!
			if (flag == 1)
			{
				//recordLength stores the size of the first file ie. "BOOTX64.EFI;1" (as seen from the results in C implementation)
				//Add the record size of this file, and the next sector should contain the kernel file.
				vol.kernel = vol.Dir + vol.recordLength;

				array_loc = (vol.kernel + loc_offset); //Kernel's LBA address
				array_len = (vol.kernel + len_offset); //Kernel size

				convertBothEndian(array_loc, &loc);
				convertBothEndian(array_len, &len);

				//since buffer size should be a multiple of device size (ie. 2048 bytes) 
				//Checked the kernel file size from C implementation; approx 7616 bytes
				len = 2048 * 4; 

				pages = EFI_SIZE_TO_PAGES(len);
				buffer = AllocatePages(pages, EfiLoaderCode);

				efi_status = bio->ReadBlocks(bio, bio->Media->MediaId, (EFI_LBA)(loc), len, buffer);
				if (!EFI_ERROR(efi_status))
				{
					SystemTable->ConOut->OutputString(SystemTable->ConOut,
													  L"Success! \r\n");
					BootServices->Stall(1 * 1000000);
				}
				else
				{
					SystemTable->ConOut->OutputString(SystemTable->ConOut,
													  L"Error! \r\n");
					BootServices->Stall(5 * 1000000);
				}
			}

			break;
		}
	}

	return buffer;
}

/* Use System V ABI rather than EFI/Microsoft ABI. */
typedef void (*kernel_entry_t)(void *, unsigned int *) __attribute__((sysv_abi));

EFI_STATUS EFIAPI
efi_main(EFI_HANDLE imageHandle, EFI_SYSTEM_TABLE *systemTable)
{

	UINT32 *fb;

	ImageHandle = imageHandle;
	SystemTable = systemTable;
	BootServices = systemTable->BootServices;

	//Allocate pages 2054 + 1 page (for kernel stack)
	VOID *kernel_addr = AllocatePages(2055, EfiLoaderData);
	kernel_addr += 0x1000;

	//Get the frame buffer base address
	fb = SetGraphicsMode(800, 600);

	//load kernel using block i/o
	VOID *kernel_buffer = handleBlockIO();

	//Memory Map and ExitBootServices()
	setExitBootServices();

	// kernel's _start() is at base #0 (pure binary format)
	// cast the function pointer appropriately and call the function
	kernel_entry_t func = (kernel_entry_t)kernel_buffer;
	func(kernel_addr, fb);

	return EFI_SUCCESS;
}
