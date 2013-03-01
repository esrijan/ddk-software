/*
 * Copyright (C) eSrijan Innovations Private Limited
 * 
 * Author: Anil Kumar Pugalia <anil_pugalia@eSrijan.com>
 *
 * Licensed under: JSL (See LICENSE file for details)
 *
 * Host's Serial Port Test Program
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

struct termios org_options;

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

int main(int argc, char **argv)
{
	int fd;
	unsigned char byte;
	int flags, choice, set, val;
	int i;
  unsigned long test1_time, test2_time; // Pugs: Added
  struct timeval tv; // Pugs: Added

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
	while (1)
	{
		printf("0: Exit\n");
		printf("1: Read DCD (Data Carrier Detect)\n");
		printf("2: Read RxD (Receive Data) - NI\n");
		printf("3: Write TxD (Transmit Data)\n");
		printf("4: Write DTR (Data Terminal Ready)\n");
		printf("5: Is just Gnd!!!\n");
		printf("6: Read DSR (Data Set Ready)\n");
		printf("7: Write RTS (Request To Send)\n");
		printf("8: Read CTS (Clear To Send)\n");
		printf("9: Read RI (Ring Indicator)\n");
		printf("10: Off LED Array\n");
		printf("11: On LED Array\n");
		printf("12: 10-second Diwali Light\n");
		printf("Choice: ");
		scanf("%d", &choice);

		if (choice == 0)
		{
			break;
		}

		/* Get the 'BEFORE' line bits */
    gettimeofday (&tv, NULL); // Pugs
    test1_time = (tv.tv_sec * 1000000) + tv.tv_usec; // Pugs
		if (ioctl(fd, TIOCMGET, &flags) == -1)
		{
			fprintf(stderr, "ioctl Error on %s: %s\n", argv[1], strerror(errno));
		}
    gettimeofday (&tv, NULL); // Pugs
    test2_time = (tv.tv_sec * 1000000) + tv.tv_usec; // Pugs
	fprintf(stderr, "T1: %.3fms\n", (test2_time - test1_time) / 1000.0); // Pugs
		fprintf(stderr, "Flags are %x.\n", flags);

		set = 0;
		val = -1;
		switch (choice)
		{
			case 1:
				val = !!(flags & TIOCM_CD);
				break;
			case 2:
				break;
			case 3:
				byte = 0x01;
				//write(fd, &byte, 1);
				if (ioctl(fd, TIOCSBRK, 0) == -1) // Pulling up TxD
				{
					fprintf(stderr, "ioctl Error on %s: %s\n", argv[1], strerror(errno));
				}
				else
				{
					fprintf(stderr, "Wrote 0x%X\n", byte);
				}
				break;
			case 30:
				byte = 0x00;
				//write(fd, &byte, 1);
				if (ioctl(fd, TIOCCBRK, 0) == -1) // Pulling low TxD
				{
					fprintf(stderr, "ioctl Error on %s: %s\n", argv[1], strerror(errno));
				}
				else
				{
					fprintf(stderr, "Wrote 0x%X\n", byte);
				}
				break;
			case 4:
				set = 1;
				flags |= TIOCM_DTR;
				break;
			case 40:
				set = 1;
				flags &= ~TIOCM_DTR;
				break;
			case 5:
				break;
			case 6:
				val = !!(flags & TIOCM_DSR);
				break;
			case 7:
				set = 1;
				flags |= TIOCM_RTS;
				break;
			case 70:
				set = 1;
				flags &= ~TIOCM_RTS;
				break;
			case 8:
				val = !!(flags & TIOCM_CTS);
				break;
			case 9:
				val = !!(flags & TIOCM_RI);
				break;
			case 10:
				if (ioctl(fd, TIOCCBRK, 0) == -1)
				{
					fprintf(stderr, "ioctl Error on %s: %s\n", argv[1], strerror(errno));
					break;
				}
				for (i = 0; i < 8; i++)
				{
					flags |= TIOCM_DTR;
					if (ioctl(fd, TIOCMSET, &flags) == -1)
					{
						break;
					}
					flags &= ~TIOCM_DTR;
					if (ioctl(fd, TIOCMSET, &flags) == -1)
					{
						break;
					}
				}
				if (i < 8)
				{
					fprintf(stderr, "ioctl Error on %s: %s\n", argv[1], strerror(errno));
				}
				break;
			case 11:
				if (ioctl(fd, TIOCSBRK, 0) == -1)
				{
					fprintf(stderr, "ioctl Error on %s: %s\n", argv[1], strerror(errno));
					break;
				}
				for (i = 0; i < 8; i++)
				{
					flags |= TIOCM_DTR;
					if (ioctl(fd, TIOCMSET, &flags) == -1)
					{
						break;
					}
					flags &= ~TIOCM_DTR;
					if (ioctl(fd, TIOCMSET, &flags) == -1)
					{
						break;
					}
				}
				if (i < 8)
				{
					fprintf(stderr, "ioctl Error on %s: %s\n", argv[1], strerror(errno));
				}
				break;
			case 12:
				for (i = 0; i < 8; i++)
				{
					if (ioctl(fd, (i % 2) ? TIOCSBRK : TIOCCBRK, 0) == -1)
					{
						break;
					}
					flags |= TIOCM_DTR;
					if (ioctl(fd, TIOCMSET, &flags) == -1)
					{
						break;
					}
					flags &= ~TIOCM_DTR;
					if (ioctl(fd, TIOCMSET, &flags) == -1)
					{
						break;
					}
				}
				if (i < 8)
				{
					fprintf(stderr, "ioctl Error on %s: %s\n", argv[1], strerror(errno));
					break;
				}
				for (i = 0; i < 10; i++)
				{
					if (ioctl(fd, (i % 2) ? TIOCSBRK : TIOCCBRK, 0) == -1)
					{
						break;
					}
					flags |= TIOCM_DTR;
					if (ioctl(fd, TIOCMSET, &flags) == -1)
					{
						break;
					}
					flags &= ~TIOCM_DTR;
					if (ioctl(fd, TIOCMSET, &flags) == -1)
					{
						break;
					}
					sleep(1);
				}
				if (i < 10)
				{
					fprintf(stderr, "ioctl Error on %s: %s\n", argv[1], strerror(errno));
				}
				break;
			default:
				break;
		}

		if (set)
		{
    gettimeofday (&tv, NULL); // Pugs
    test1_time = (tv.tv_sec * 1000000) + tv.tv_usec; // Pugs
			if (ioctl(fd, TIOCMSET, &flags) == -1)
			{
				fprintf(stderr, "ioctl2 Error on %s: %s\n", argv[1], strerror(errno));
			}
			fprintf(stderr, "Setting %x.\n", flags);
    gettimeofday (&tv, NULL); // Pugs
    test2_time = (tv.tv_sec * 1000000) + tv.tv_usec; // Pugs
	fprintf(stderr, "T2: %.3fms\n", (test2_time - test1_time) / 1000.0); // Pugs

			sleep(1);
		}
		else if (val != -1)
		{
			fprintf(stderr, "Obtained: %d\n", val);
		}

		/* Get the 'AFTER' line bits */
		if (ioctl(fd, TIOCMGET, &flags) == -1)
		{
			fprintf(stderr, "ioctl3 Error on %s: %s\n", argv[1], strerror(errno));
		}
		fprintf(stderr, "Flags are %x.\n", flags);
	}
	reset_serial_options(fd);
	
	close(fd);
}
