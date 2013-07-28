//-----------------------------------------------------------------------------
//
//	PICSTART communications snooper
//
//-----------------------------------------------------------------------------
//
//	Copyright (C) 2002 Cosmodog, Ltd.
//	Copyright (C) 2005 Jeffery L. Post
//
//	This program is free software; you can redistribute it and/or
//	modify it under the terms of the GNU General Public License
//	as published by the Free Software Foundation; either version 2
//	of the License, or (at your option) any later version.
//
//	This program is distributed in the hope that it will be useful,
//	but WITHOUT ANY WARRANTY; without even the implied warranty of
//	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//	GNU General Public License for more details.
//
//	You should have received a copy of the GNU General Public License
//	along with this program; if not, write to the Free Software
//	Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//
//-----------------------------------------------------------------------------
//
//  Please see README.TXT for information on using this program
//
//-----------------------------------------------------------------------------
// thanks to Todd Squires for the orignal terminal interface
//-----------------------------------------------------------------------------
//
// Modified to add extra PIC_DEFINITION data for 18xxx support - JLP
// Additional definitions - JLP
// Ported to DOS - JLP
//

static const char * const version = "0.2";

#include <stdio.h>
#include <string.h>
#include <signal.h>
#include	<conio.h>
#include	<dos.h>

#include "serial.h"

#define	DEF_SIZE		44
#define	DEFX_SIZE	32

extern int	errno;
static bool	done;

int	kbscan(void);

//-----------------------------------------------------------------------------
// dump the data from the port in hex

void Dump(int theDevice)
{
	unsigned char	theBuffer[10];
	int	numRead, numShown, kchr;
	
	numShown = 0;

	while (!done)
	{
		numRead = read(theDevice, theBuffer, 1);		// try to read a character

		if (numRead >= 1)
		{
			fprintf(stdout, "%02x ", theBuffer[0]);
			fflush(stdout);					// make sure it dumps it out right away
			numShown++;

			if (numShown == 16)
			{
				numShown = 0;
				fputc('\n', stdout);
			}
		}

		if (numRead < 0)
			fprintf(stderr, "error %d, %s\n", errno, strerror(errno));

		kchr = kbscan();

		if (kchr == 0x03)
			done = true;
	}

	fputc('\n', stdout);
}

//-----------------------------------------------------------------------------
// capture profiles, display them in a usable format

void CaptureProfiles(int theDevice)
{
	const unsigned char	preamble[] = {0x88,0x8d,0x80,0x81};
	unsigned char			theBuffer[256];
	int			numRead, idx, kchr;
		
	while (!done)
	{
		// first try to find the preamble (initial communications from MPLAB to the PICSTART)
		for (idx=0; idx < sizeof(preamble) && !done; idx++)
		{
			do
			{
				numRead = read(theDevice, theBuffer, 1);			// read one character
			}while (numRead == 0 && !done);
			
			if (numRead == 1 && *theBuffer != preamble[idx])
				idx = 0;										// mismatch, start over

			if (numRead < 0)
				fprintf(stderr, "error %d, %s\n", errno, strerror(errno));

			kchr = kbscan();

			if (kchr == 0x03)
				done = true;
		}

		idx = 0;

		while (idx < DEF_SIZE + DEFX_SIZE + 3 && !done)
		{
			// the extra three bytes are the two checksums and the command in the middle (before the second block)
			numRead = read(theDevice, &theBuffer[idx], DEF_SIZE + DEFX_SIZE + 3 - idx);
			// try to read a total of DEF_SIZE + DEFX_SIZE + 3 bytes into the buffer

			if (numRead > 0)
				idx += numRead;

			if (numRead < 0)
				fprintf(stderr, "error %d, %s\n", errno, strerror(errno));
		}
		if (!done)
		{
			fprintf(stdout, "//-----------------------------------------------------------\n");
			fprintf(stdout, "const static unsigned char def_PICxxxxx[] =\n");
			fprintf(stdout, "{\n");
			fprintf(stdout, "\t0x%02x, 0x%02x,\t\t// size of program space\n", theBuffer[0], theBuffer[1]);
			fprintf(stdout, "\t0x%02x, 0x%02x,\t\t// width of address word\n", theBuffer[2], theBuffer[3]);
			fprintf(stdout, "\t0x%02x, 0x%02x,\t\t// width of data word\n", theBuffer[4], theBuffer[5]);
			fprintf(stdout, "\t0x%02x, 0x%02x,\t\t// width of ID\n", theBuffer[6], theBuffer[7]);
			fprintf(stdout, "\t0x%02x, 0x%02x,\t\t// ID mask\n", theBuffer[8], theBuffer[9]);
			fprintf(stdout, "\t0x%02x, 0x%02x,\t\t// width of configuration word\n", theBuffer[10], theBuffer[11]);
			fprintf(stdout, "\t0x%02x, 0x%02x,\t\t// configuration word mask\n", theBuffer[12], theBuffer[13]);
			fprintf(stdout, "\t0x%02x, 0x%02x,\t\t// EEPROM data width\n", theBuffer[14], theBuffer[15]);
			fprintf(stdout, "\t0x%02x, 0x%02x,\t\t// EEPROM data mask\n", theBuffer[16], theBuffer[17]);
			fprintf(stdout, "\t0x%02x, 0x%02x,\t\t// Calibration width\n", theBuffer[18], theBuffer[19]);
			fprintf(stdout, "\t0x%02x, 0x%02x,\t\t// Calibration mask\n", theBuffer[20], theBuffer[21]);
			fprintf(stdout, "\t0x%02x, 0x%02x,\t\t// ??\n", theBuffer[22], theBuffer[23]);
			fprintf(stdout, "\t0x%02x, 0x%02x,\t\t// ??\n", theBuffer[24], theBuffer[25]);
			fprintf(stdout, "\t0x%02x, 0x%02x,\t\t// address of ID locations\n", theBuffer[26], theBuffer[27]);
			fprintf(stdout, "\t0x%02x,\t\t\t// size of ID locations\n", theBuffer[28]);
			fprintf(stdout, "\t0x%02x, 0x%02x,\t\t// address of configuration bits\n", theBuffer[29], theBuffer[30]);
			fprintf(stdout, "\t0x%02x,\t\t\t// size of configuration register\n", theBuffer[31]);
			fprintf(stdout, "\t0x%02x, 0x%02x,\t\t// address of data space\n", theBuffer[32], theBuffer[33]);
			fprintf(stdout, "\t0x%02x, 0x%02x,\t\t// size of data space\n", theBuffer[34], theBuffer[35]);
			fprintf(stdout, "\t0x%02x, 0x%02x,\t\t// address of internal clock calibration value\n", theBuffer[36], theBuffer[37]);
			fprintf(stdout, "\t0x%02x, 0x%02x,\t\t// size of clock calibration space\n", theBuffer[38], theBuffer[39]);
			fprintf(stdout, "\t0x%02x,\t\t\t// additional programming pulses for C devices\n", theBuffer[40]);
			fprintf(stdout, "\t0x%02x,\t\t\t// main programming pulses for C devices\n", theBuffer[41]);
			fprintf(stdout, "\t0x%02x, 0x%02x,\t\t//  ZIF configuration ??\n", theBuffer[42], theBuffer[43]);
			fprintf(stdout, "};\n\n");

			fprintf(stdout, "const static unsigned char defx_PICxxxxx[] =\n");
			fprintf(stdout, "{\n");
			fprintf(stdout, "\t0x%02x, 0x%02x, 0x%02x, 0x%02x,\n", theBuffer[46], theBuffer[47], theBuffer[48], theBuffer[49]);
			fprintf(stdout, "\t0x%02x, 0x%02x, 0x%02x, 0x%02x,\n", theBuffer[50], theBuffer[51], theBuffer[52], theBuffer[53]);
			fprintf(stdout, "\t0x%02x, 0x%02x, 0x%02x, 0x%02x,\n", theBuffer[54], theBuffer[55], theBuffer[56], theBuffer[57]);
			fprintf(stdout, "\t0x%02x, 0x%02x, 0x%02x, 0x%02x,\n", theBuffer[58], theBuffer[59], theBuffer[60], theBuffer[61]);
			fprintf(stdout, "\t0x%02x, 0x%02x, 0x%02x, 0x%02x,\n", theBuffer[62], theBuffer[63], theBuffer[64], theBuffer[65]);
			fprintf(stdout, "\t0x%02x, 0x%02x, 0x%02x, 0x%02x,\n", theBuffer[66], theBuffer[67], theBuffer[68], theBuffer[69]);
			fprintf(stdout, "\t0x%02x, 0x%02x, 0x%02x, 0x%02x,\n", theBuffer[70], theBuffer[71], theBuffer[72], theBuffer[73]);
			fprintf(stdout, "\t0x%02x, 0x%02x, 0x%02x, 0x%02x,\n", theBuffer[74], theBuffer[75], theBuffer[76], theBuffer[77]);
			fprintf(stdout, "};\n\n");
			
			fprintf(stdout, "const static PIC_DEFINITION PICxxxxx =\n");
			fprintf(stdout, "{\n");
			fprintf(stdout, "\t\"xxxxx\",\t\t// device name\n");
			fprintf(stdout, "\tdef_PICxxxxx,\t\t// definition\n");
			fprintf(stdout, "\tdefx_PICxxxxx,\t\t// extended definition\n");
			fprintf(stdout, "\t0,\t\t\t// config word: code protect bit mask\n");
			fprintf(stdout, "\t0,\t\t\t// config word: watchdog bit mask\n");

			fprintf(stdout, "\n\t0,\t\t\t// Word alignment for writing to this device\n");
			fprintf(stdout, "\t0,\t\t\t// Configuration memory start address\n");
			fprintf(stdout, "\t0, 0,\t\t\t// ID Locations addr\n");
			fprintf(stdout, "\t0,\t\t\t// Data eeprom address\n");

			fprintf(stdout, "\t0,\t\t\t// number of words in cfg bits with factory set bits\n");
			fprintf(stdout, "\t{0, 0, 0, 0, 0, 0, 0, 0},\t// fixed bits mask\n");
					// we assume that all programmers support the new chip (fat chance)
			fprintf(stdout, "\t(P_PICSTART | P_WARP13 | P_JUPIC),\t// bit map of supporting programmers\n");

			fprintf(stdout, "};\n\n\n");
			fflush(stdout);					// make sure it dumps it out right away

			done = true;
		}
	}
}

//-----------------------------------------------------------------------------
// Once the device is opened and locked, this sets
// up the port, and makes sure the handshake looks good.

static bool InitDevice(int theDevice, unsigned int baudRate, unsigned char dataBits, unsigned char parity)
{
	if (ConfigureDevice(theDevice, baudRate, dataBits, parity))	// set up the device
		return true;
	else
		fprintf(stderr, "Could not configure device parameters\n");

	return(false);
}

//-----------------------------------------------------------------------------
// tell the user how to use this program

void Usage(void)
{
	fprintf(stderr, "Usage: picsnoop ttyname [flag]\n");
	fprintf(stderr, "  if no flag then data will be formatted for use in picdev.c\n");
	fprintf(stderr, "  if flag then data from port will be dumped in hex\n\n");
}

//-----------------------------------------------------------------------------
// open the device, and begin terminal operations on it

int main(int argc,char *argv[])
{
	int				theDevice;
	unsigned int	baudRate;
	unsigned char	dataBits, parity;

	done = false;

	fprintf(stderr, "\npicsnoop version 0.1 (c) 2002 Cosmodog, Ltd.\n");
	fprintf(stderr, "picsnoop version %s for DOS (c) 2005 Jeff Post.\n\n", version);
	
	if (argc >= 2)
	{
		if (OpenDevice(argv[1], &theDevice))				// open the device
		{
			baudRate = 19200;
			dataBits = 8;
			parity = 0;

			if (InitDevice(theDevice, baudRate, dataBits, parity))
			{
				fprintf(stderr, "%d,%d,%c\n", baudRate, dataBits, (parity == 0) ? 'N' : (parity == 1) ? 'O' : 'E');
				fprintf(stderr, "Type <ctrl-C> to exit\n");

				if (argc==2)
					CaptureProfiles(theDevice);
				else
					Dump(theDevice);

				fprintf(stderr, "exiting...\n");
			}
			else
				fprintf(stderr, "Failed to set up the serial port\n");

			CloseDevice(theDevice);
		}
		else
			fprintf(stderr, "Failed to open device '%s'\n", argv[1]);
	}
	else
		Usage();

	return(0);
}

int kbscan(void)
{
	int i;

	i = 0;

	if (peek(0, KBHEAD) != peek(0, KBTAIL))
	{
		i = getch();

		if (!i)
			i = getch() | 0x100;
	}

	return(i);
}

// end

