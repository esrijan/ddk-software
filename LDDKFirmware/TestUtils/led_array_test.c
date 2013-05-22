/*
 * Copyright (C) eSrijan Innovations Private Limited
 * 
 * Author: Anil Kumar Pugalia <anil_pugalia@eSrijan.com>
 *
 * Licensed under: JSL (See LICENSE file for details)
 *
 * LED Array Test Program
 */
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <termios.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <sys/ioctl.h>
#include <signal.h>

static struct termios org_options;
static int flags;

void set_serial_options(int fd)
{
	struct termios options;

	fcntl(fd, F_SETFL, 0);
	tcgetattr(fd, &options);
	org_options = options;

	//cfmakeraw(&options);
	/* Input baud rate is set */
	cfsetispeed(&options, B9600);
	/* Output baud rate is set */
	cfsetospeed(&options, B9600);
	/* Reciever is enabled, so character can be recieved */
	options.c_cflag |= (CLOCAL | CREAD);
	options.c_cflag &= ~PARENB; // No parity
	/*
	 * Parity:
	 * options.c_cflag |= PARENB;
	 * 	Odd: options.c_cflag |= PARODD;
	 * 	Even: options.c_cflag &= ~PARODD;
	 */
	options.c_cflag &= ~CSTOPB; // 1 stop bit
	// options.c_cflag |= CSTOPB; // 2 stop bits
	options.c_cflag &= ~CSIZE;
	options.c_cflag |= CS8; // Can be from 5 to 8
	// No flow control
	options.c_cflag &= ~CRTSCTS;
	options.c_iflag &= ~(IXON | IXOFF);
	// options.c_cflag |= CRTSCTS; // Hardware flow control
	// options.c_iflag |= (IXON | IXOFF); // Software flow control
	options.c_lflag &= ~(ICANON | ECHO);
	options.c_oflag &= ~OPOST;
	/* Applying the new attributes */
	tcsetattr(fd, TCSANOW, &options);
}

void reset_serial_options(int fd)
{
	fcntl(fd, F_SETFL, 0);

	/* Applying the original attributes */
	tcsetattr(fd, TCSANOW, &org_options);
}

int data(int fd, int val)
{
	if (ioctl(fd, (val % 2) ? TIOCSBRK : TIOCCBRK, 0) == -1)
	{
		return 0;
	}
	usleep(10);
	return 1;
}

int clock(int fd)
{
	flags |= TIOCM_DTR;
	if (ioctl(fd, TIOCMSET, &flags) == -1)
	{
		return 0;
	}
	usleep(10000);
	flags &= ~TIOCM_DTR;
	if (ioctl(fd, TIOCMSET, &flags) == -1)
	{
		return 0;
	}
	usleep(10000);
	return 1;
}

int main(int argc, char **argv)
{
	int fd;
	int i;

	if (argc < 2)
	{
		fprintf(stderr, "Usage: %s <device>\n", argv[0]);
		return 1;
	}

	if ((fd = open(argv[1], O_RDWR | O_NDELAY)) < 0)
	{
		fprintf(stderr, "Error opening %s: %s\n", argv[1], strerror(errno));
		return 2;
	}

	set_serial_options(fd);
	if (ioctl(fd, TIOCMGET, &flags) == -1)
	{
		fprintf(stderr, "ioctl Error on %s: %s\n", argv[1], strerror(errno));
	}

	/* All on */
	data(fd, 1);
	for (i = 0; i < 8; i++) clock(fd);
	sleep(1);
	/* All off */
	data(fd, 0);
	for (i = 0; i < 8; i++) clock(fd);
	sleep(1);
	/* Control lighting */
	for (i = 0; i < 8; i++)
	{
		data(fd, i % 2);
		clock(fd);
	}
	sleep(1);
	for (i = 0; i < 2; i++)
	{
		data(fd, i % 2);
		clock(fd);
		sleep(1);
	}
	sleep(1);

	/* Switch off before quitting */
	data(fd, 0);
	for (i = 0; i < 8; i++) clock(fd);

	reset_serial_options(fd);
	
	close(fd);
}
