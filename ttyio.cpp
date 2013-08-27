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

void
Packet::open(const char *dev,int maxbuflen,int fd) {
	struct termios tios;
	int rc;

	if ( !dev )
		dev = "/dev/cu.usbserial-A100MX3L";

	if ( maxbuflen <= 0 )
		maxbuflen = 1024;

	buf = new uint8_t[maxbuflen+1];

	maxlen = maxbuflen;
	buflen = 0;
	state = pkt_idle;
	ungot = false;
	unbyte = 0;
	tty_fd = -1;

	if ( fd < 0 ) {
		device = strdup(dev);
		tty_fd = ::open(device,O_RDWR);
		assert(tty_fd >= 0);
	} else	{
		device = 0;
		tty_fd = fd;
	}

	if ( isatty(tty_fd) ) {
		rc = tcgetattr(tty_fd,&tios);
		assert(!rc);
		cfsetspeed(&tios,9600);
		cfmakeraw(&tios);

		tios.c_cflag |= PARODD | PARENB;
		tios.c_cflag &= ~CSIZE;
		tios.c_cflag |= CS8;
		rc = tcsetattr(tty_fd,TCSANOW,&tios);
		assert(!rc);
	}
}

Packet::Packet() {
	device = 0;
	tty_fd = -1;
	buf = 0;
	buflen = 0;
	maxlen = 0;
	state = pkt_idle;
	ungot = false;
	unbyte = 0x00;
}

Packet::~Packet() {
	close(tty_fd);
	tty_fd = -1;
	if ( buf )
		delete buf;
	buf = 0;
}

//////////////////////////////////////////////////////////////////////
// Read a byte from fd
//////////////////////////////////////////////////////////////////////

int
Packet::getb(int fd) {
	uint8_t byte;
	int rc;

	do	{
		rc = read(fd,&byte,1);
		if ( !rc )
			return -1;		// EOF
	} while ( rc < 0 && errno == EINTR );
	assert(rc==1);
	return (int)byte;
}

//////////////////////////////////////////////////////////////////////
// Get a byte from serial port or stdin
// 
// RETURNS:
// 	byte_serial	- Serial byte returned
// 	byte_stdin	- Stdin byte returned
//	byte_timeout	- No byte (timed out)
//	byte_eof	- EOF when not a tty and at EOF
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

	if ( fds[0].fd == fds[1].fd )
		n = 1;		// stdin is packet data in (not a tty)

	do	{
		rc = poll(fds,n,ms);
	} while ( rc < 0 && errno == EINTR );

	for ( x=0; x<n; ++x ) {
		if ( fds[x].revents & POLLIN ) {
			int b = getb(fds[x].fd);
			if ( b == -1 )
				return byte_eof;
			byte = (uint8_t) b & 0xFF;
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
// Put n bytes
//////////////////////////////////////////////////////////////////////

void
Packet::put(uint8_t *buf,uint16_t len) {

	for ( ; len > 0; --len )
		putb(*buf++);
}

//////////////////////////////////////////////////////////////////////
// Put a byte into the receiving buffer
//////////////////////////////////////////////////////////////////////

void
Packet::putbuf(uint8_t byte) {

	assert(buflen > 0 || byte != 0x10);

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
Packet::get(uint8_t **packet,int *length,bool& ended) {
	uint8_t byte;
	e_gstate e;

	buflen = 0;
	ended = false;

	//////////////////////////////////////////////////////////////
	// First wait for arrival of 0x10 (DLE)
	//////////////////////////////////////////////////////////////

	do	{
		e = getb(byte,250);

		switch ( e ) {
		case byte_eof :
			*length = 0;
			ended = false;
			return;
		case byte_serial :
			switch ( state ) {
			case pkt_idle :
				if ( byte == 0x10 ) {
					state = pkt_data;
					buflen = 0;
				}
				break;
			case pkt_data :
				if ( byte == 0x10 ) {
					if ( buflen > 0 )
						state = pkt_escape;
				} else	{
					putbuf(byte);
				}
				break;	
			case pkt_escape :
				if ( byte == 0x10 ) {
					putbuf(byte);
					state = pkt_data;
				} else if ( byte == 0x03 ) {
					state = pkt_end;
					ended = true;
				} else	{
					unget(byte);
					state = pkt_idle;
				}
				break;
			case pkt_end :
				assert(0);
			}
			break;
		case byte_stdin :
			if ( callback )
				callback(*this,(char)byte);
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
	state = pkt_idle;
}	

// End ttyio.cpp

