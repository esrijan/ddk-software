ddk-software
============

Device Development Kit (DDK)'s software explorations

This project is released under the Juice Sharing License (See LICENSE file
for details). Many parts of this project have been derived from other open
source projects. Here's the list of content, sourced from their corresponding
open source projects, and then modified / ported for the DDK:
+ BootloadHID - http://www.obdev.at/products/vusb/bootloadhid.html
+ LDDKFirmware/Code - http://www.obdev.at/products/vusb/
+ LDDKFirmware/TestUtils/USBDeviceTest - http://www.obdev.at/
+ LDDKFirmware/TestUtils/USBLEDTest - http://www.obdev.at/

Check the above individually for the licenses, they are bound to.

This directory contains various sub-projects, each an exploration on its
own, along with the following in capital letters:

+ README.md - this file
+ LICENSE - project's license file

+ rules.mk - master Makefile for building various DDK related stuff
+ BootloadHID - boot loader for DDK (typically already flashed on a DDK)
+ Examples - various "code examples" aka "libraries + demos" for DDK hacks
+ LDDKFirmware - Firmware for DDK to become a USB device for various Linux
	device driver experiments
