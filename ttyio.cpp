//////////////////////////////////////////////////////////////////////
// ttyio.cpp -- Serial I/O Module 
// Date: Mon Aug 19 20:08:26 2013
///////////////////////////////////////////////////////////////////////

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <termios.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <poll.h>
#include <assert.h>

#include "ttyio.hpp"

Packet::Packet(const char *dev,int maxbuflen) {
	struct termios tios;
	int rc;

	if ( !dev )
		dev = "/dev/cu.usbserial-A100MX3L";

	buf = new uint8_t[maxbuflen+1];
	maxlen = maxbuflen;
	buflen = 0;
	state = pkt_idle;
	ungot = false;
	unbyte = 0;
	tty_fd = -1;


	tty_fd = open(device.c_str(),O_RDWR);
	assert(tty_fd >= 0);

	rc = tcgetattr(tty_fd,&tios);
	assert(!rc);
	cfsetspeed(&tios,9600);
	cfmakeraw(&tios);
	rc = tcsetattr(tty_fd,TCSANOW,&tios);
	assert(!rc);
}

Packet::~Packet() {
	close(tty_fd);
	tty_fd = -1;
	delete buf;
}

//////////////////////////////////////////////////////////////////////
// Read a byte from fd
//////////////////////////////////////////////////////////////////////

uint8_t
Packet::getb(int fd) {
	uint8_t byte;
	int rc;

	do	{
		rc = read(tty_fd,&byte,1);
	} while ( rc < 0 && errno == EINTR );
	assert(rc==1);
	return byte;
}

//////////////////////////////////////////////////////////////////////
// Get a byte from serial port or stdin
// 
// RETURNS:
// 	byte_serial	- Serial byte returned
// 	byte_stdin	- Stdin byte returned
//	byte_timeout	- No byte (timed out)
// 
//////////////////////////////////////////////////////////////////////

Packet::e_gstate
Packet::getb(uint8_t& byte,int ms) {

	if ( ungot ) {
		byte = unbyte;
		ungot = false;
		return byte_serial;
	}

	int x, n = 2;
	struct pollfd fds[n];
	int rc;

	fds[0].fd = tty_fd;
	fds[0].events = POLLIN;
	fds[0].revents = 0;

	fds[1].fd = 0;
	fds[1].events = POLLIN;
	fds[1].revents = 0;

	do	{
		rc = poll(fds,n,ms);
	} while ( rc < 0 && errno == EINTR );

	for ( x=0; x<n; ++x ) {
		if ( fds[x].revents & POLLIN ) {
			byte = getb(fds[x].fd);
			if ( !x )
				return byte_serial;
			else	return byte_stdin;
		}
	}

	return byte_timeout;
}

//////////////////////////////////////////////////////////////////////
// Put back a read-ahead byte
//////////////////////////////////////////////////////////////////////

void
Packet::unget(uint8_t byte) {
	assert(!ungot);
	ungot = true;
	unbyte = byte;
}

//////////////////////////////////////////////////////////////////////
// Write one byte out to serial port
//////////////////////////////////////////////////////////////////////

void
Packet::putb(uint8_t byte) {
	int rc;

	do	{
		rc = write(tty_fd,&byte,1);
	} while ( rc < 0 && errno == EINTR );
	assert(rc == 1);
}

//////////////////////////////////////////////////////////////////////
// Put a byte into the receiving buffer
//////////////////////////////////////////////////////////////////////

void
Packet::putbuf(uint8_t byte) {
	if ( buflen >= maxlen ) {	// discard packet -- too long
		state = pkt_idle;
		buflen = 0;
	} else	{
		buf[buflen++] = byte;
	}
}

//////////////////////////////////////////////////////////////////////
// Return a packet
//////////////////////////////////////////////////////////////////////

void
Packet::get(uint8_t **packet,int *length) {
	uint8_t byte;
	e_gstate e;

	buflen = 0;
	state = pkt_idle;

	//////////////////////////////////////////////////////////////
	// First wait for arrival of 0x10 (DLE)
	//////////////////////////////////////////////////////////////

	do	{
		e = getb(byte,250);
		switch ( e ) {
		case byte_serial :
			switch ( state ) {
			case pkt_idle :
				if ( byte == 0x10 )
					state = pkt_data;
				break;
			case pkt_data :
				if ( byte == 0x10 )
					state = pkt_escape;
				else	putb(byte);
				break;	
			case pkt_escape :
				if ( byte == 0x10 ) {
					putb(byte);
				} else if ( byte == 0x03 ) {
					state = pkt_end;
				} else	{
					unget(byte);
					state = pkt_end;
				}
			case pkt_end :
				assert(0);
			}
			break;
		case byte_stdin :
			if ( callback )
				callback((char)byte);
			break;
		case byte_timeout :
			switch ( state ) {
			case pkt_idle :
				break;
			case pkt_data :
			case pkt_escape :
				state = pkt_end;
				break;
			case pkt_end :
				assert(0);
			}
			break;
		}
	} while ( state != pkt_end );

	*packet = buf;
	*length = buflen;
}	

// End ttyio.cpp

