This is pilights, a Tcl interface to do lighting stuff on raspberry pi.

Version 1.0

This package is a freely available open source package under the "Berkeley"
license, same as Tcl.  You can do virtually anything you like with it, such as 
modifying it, redistributing it, and selling it either in whole or in part.  
See the file "license.terms" for complete information.

Pilights was written by Karl Lehenbauer.

### Building pilights

Pilights requires tclspi, a Tcl interface to the Serial Peripheral Interface Bus on the Raspberry Pi from https://github.com/lehenbauer/tclspi

It also requires tcl.gd, a Tcl interface to the GD graphics drawing library from FlightAware at https://github.com/flightaware/tcl.gd

Follow README.Debian 

```sh
autoconf
./configure --with-tcl=/usr/lib/tcl8.6
make
sudo make install
```

Edit /etc/modprobe.d/raspi-blacklist.conf and comment out the blacklist line for spi-bcm2708.

Add "dtparam=spi=on" to your /boot/config.txt and reboot.

### Using pilights

package require pilights

### Pilights Quick Ref

Please see the pilights manpage in the doc directory.

### Examples

A number of simple examples of the use of tcl.gd can be found in the playpen
directory.

### Bugs

Yes.

### Contents

Makefile.in	Makefile template.  The configure script uses this file to
		produce the final Makefile.

README		This file

aclocal.m4	Generated file.  Do not edit.  Autoconf uses this as input
		when generating the final configure script.  See "tcl.m4"
		below.

configure	Generated file.  Do not edit.  This must be regenerated
		anytime configure.in or tclconfig/tcl.m4 changes.

configure.in	Configure script template.  Autoconf uses this file as input
		to produce the final configure script.

generic/pilights.c	Interface between Tcl and SPI and GD
generic/pilights.h	include file
generic/tclpilights.c	Init routines.


tclconfig/	This directory contains various template files that build
		the configure script.  They should not need modification.

	install-sh	Program used for copying binaries and script files
			to their install locations.

	tcl.m4		Collection of Tcl autoconf macros.  Included by
			aclocal.m4 to define SC_* macros.

### Unix build

Building under most UNIX systems is easy, just run the configure script
and then run make. 

	$ cd pilights
	$ ./configure
	$ make
	$ make install

### Windows build

Nah.

