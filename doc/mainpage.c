/*! \mainpage OpenFAT Documentation

\section intro_sec Introduction

OpenFAT is a FAT filesystem for embedded processors, created by the 
Electronics Research Group at the University of Otago, and supported by a 
grant from the New Zealand Ministry of Science and Innovation. 

\section req_sec Requirements

OpenFAT is written entirely in C using C99 and GNU extentions.  It is 
intended for use in microcontrollers, but is written to be portable and
an example using a FAT image in a POSIX file is included with the source.
It should build with GCC for any target architecture.

We are currently using OpenFAT with the STM32 family of ARM Cortex-M3 
microcontrollers from ST Microelectronics, using 
<a href="https://github.com/esden/summon-arm-toolchain">summon-arm-toolchain</a> and 
<a href="http://sourceforge.net/apps/mediawiki/libopenstm32/">libopenstm32</a>.

\section install_sec Installation
 
If building for a Unix host:
\verbatim
make
make install
\endverbatim

If building for STM32 with summon-arm-toolchain:
\verbatim
make HOST=stm32
make HOST=stm32 DESTDIR=~/sat install
\endverbatim

\section usage_sec Using the OpenFAT library.
The public interface is defined in openfat.h.  A concrete implementation of
the abstract ::block_device must be provided by the application.

\code
#include <openfat.h>

int main(void)
{
	struct block_device *bldev;
	FatVol vol;
	FatFile file;
	char buffer[128];

	bldev = block_device_new(); // Must be provided by the application.
	fat_vol_init(bldev, &vol);

	fat_open(&vol, "file.dat", O_RDONLY, &file);
	fat_read(&file, buffer, sizeof(buffer));

	// Do something with buffer...

	return 0;
}
\endcode

Only low-level access functions fat_read() and fat_write() are provided by
OpenFAT.  If higher level functions such as fprintf() or fgets() are needed by
the application, <a href="http://sourceware.org/newlib/">Newlib</a> provides 
an stdio implementation that may easily be wrapped around the OpenFAT low-level 
functions.

\section link_sec Linking to the OpenFAT library.
The library will be installed as libopenfat.a, so pass the option '-lopenfat' 
to GCC at the linker stage.
\verbatim
gcc fat_example.c -o fat_example -lopenfat
\endverbatim

 */

