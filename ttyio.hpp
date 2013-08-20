//////////////////////////////////////////////////////////////////////
// ttyio.hpp -- Serial I/O Module
// Date: Mon Aug 19 20:03:11 2013   (C) datablocks.net
///////////////////////////////////////////////////////////////////////

#ifndef TTYIO_HPP
#define TTYIO_HPP

#include <stdint.h>

#include <string>

typedef void (*cmdcb_t)(char ch);

class Packet {
	std::string	device;
	enum e_state {
		pkt_idle,
		pkt_data,
		pkt_escape,
		pkt_end
	};

	int	tty_fd;		// Open fd
	uint8_t	*buf;		// Packet buffer
	int	buflen;		// Current length of buffer
	int	maxlen;		// Max length of buffer
	e_state	state;		// Current state of packet buffer

	cmdcb_t	callback;	// Callback for stdin data

protected:
	enum e_gstate {
		byte_serial,
		byte_stdin,
		byte_timeout
	};

	bool	ungot;		// True if we "ungot" a byte
	uint8_t	unbyte;		// The "ungot" byte, if any

	uint8_t getb(int fd);			// Read bytte with timeout
	e_gstate getb(uint8_t& byte,int ms);	// Get byte with timeout
	void putb(uint8_t byte);		// Put byte out to serial port
	void unget(uint8_t byte);		// Put back a got byte

	void putbuf(uint8_t byte);		// Put byte into buffer (if room)

public:	Packet(const char *dev,int maxbuflen);
	~Packet();

	inline void registercb(cmdcb_t usrcb) { callback = usrcb; }

	void get(uint8_t **packet,int *length);
};

#endif // TTYIO_HPP

// End ttyio.hpp

