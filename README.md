# ISO 9660 File System

Explores ISO 9660 file systems and loads the kernel file into memory using low-level Block I/O operations which reads 2KB raw blocks.

## C 
This folder includes an implementation in C to traverse through an ISO file (ISO 9660 File Systems) and prints out relevant information such as file names and sizes. Can be extended to print other information.

### How to run?
- Path to the iso file has to be provided in `main.c` and `isoMethods.c` in the function `check_kernel_file()`.
- Navigate to base folder and run `make`.

## UEFI
This folder includes an implementation to perform higher-level operations (reading a file) using the low-level block I/O operations in UEFI. Reads the ISO file, and loads the kernel into memory.

### Tools:
- build-essential, clang, lld, mkisofs, virtualbox. (install with `sudo apt install`)
- Clone this repository - https://github.com/rusnikola/fwimage in the same folder where `Code` and `Include` folders are present.
- VirtualBox

### How to run?
- Navigate to `Code` folder and run make.sh which creates `boot.iso`.
- Load the `boot.iso` into a virtual machine in VirtualBox with UEFI enabled.


## References
- https://wiki.osdev.org/ISO_9660
- https://wiki.osdev.org/Reading_sectors_under_UEFI
- https://uefi.org/sites/default/files/resources/UEFI%20Spec%202.8B%20May%202020.pdf