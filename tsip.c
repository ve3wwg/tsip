/* TSIP protocol
 * Warren W. Gay VE3WWG
 * Fri Aug  2 22:34:18 2013
 */
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <math.h>

#include "tsip.h"

/*
 * Initialize s_inpkt for packet reception :
 *
 * 	inctl		Initialized input control block
 * 	buf		Specifies where the input buffer is
 *	bufsize		Specifies the maximum buffer size (bytes)
 */
void
inp_init(s_inpkt *inctl,uint8_t *buf,uint16_t bufsiz) {
	inctl->buf 	= buf;
	inctl->bufsiz	= bufsiz;
	inctl->length	= 0;
	inctl->checksum[0] = inctl->checksum[1] = inctl->checksum[2] = 0;
	inctl->state	= Start;
	inctl->cksumok	= 0;
	inctl->offset	= 0;
}

/*
 * Enter a received byte into the input packet buffer.
 *
 *	inctl		Input control block
 *	byte		Byte to add to the input packet
 *
 * RETURNS:
 *	0		More data is required
 *	< 0		An error has occurred
 *	> 0		The length of the captured packet
 *			in user specified buffer.
 */
short
inp_addb(s_inpkt *inctl,uchar byte) {

	switch ( inctl->state ) {
	case Start :
		if ( byte == TSIP_DLE ) {
			inctl->state = WaitID;
		}
		break;
	case WaitID :
		switch ( byte ) {
		case TSIP_DLE :
			break;
		case TSIP_ETX :
			inctl->state = Start;
			break;
		default :
			inctl->buf[0] = byte;
			inctl->length = 0;
			inctl->state = WaitData;
			inctl->checksum[0] = inctl->checksum[1] = 0;
			inctl->cksumok = 0;
			inctl->offset = 0;

			if ( inctl->length >= inctl->bufsiz ) {
				inctl->state = Start;
				return TSIPER_TOOLONG;	// Notify caller about buffer length
			}
			inctl->checksum[2] = byte;
			inctl->buf[inctl->length++] = byte;
		}
		break;
	case WaitData :
		switch ( byte ) {
		case TSIP_DLE :
			inctl->state = EscData;
			break;
		default :
			if ( inctl->length >= inctl->bufsiz ) {
				inctl->state = Start;
				return TSIPER_TOOLONG;	// Notify caller about buffer length
			}
			inctl->checksum[0] = inctl->checksum[1];
			inctl->checksum[1] = inctl->checksum[2];
			inctl->checksum[2] += (int)byte;
			inctl->buf[inctl->length++] = byte;
		}
		break;
	case EscData :
		switch ( byte ) {
		case TSIP_DLE :
			if ( inctl->length >= inctl->bufsiz ) {
				inctl->state = Start;
				return TSIPER_TOOLONG;
			}
			inctl->checksum[0] = inctl->checksum[1];
			inctl->checksum[1] = inctl->checksum[2];
			inctl->checksum[2] += (int)byte;
			inctl->buf[inctl->length++] = byte;
			inctl->state = WaitData;
			break;
		case TSIP_ETX :
			inctl->state = EndPacket;
			break;
		default :
			inctl->state = Start;		// Protocol error (noise?)
		}		
		break;
	case EndPacket :
		break;		// Should not get here, since this is a temp state
	}

	/*
	 * Process the checksum for the packet :
	 */
	if ( inctl->state == EndPacket ) {
		if ( inctl->length > 2 ) {
			inctl->cksumok = ( ((ushort)inctl->checksum[0] & 0xFF) == inctl->buf[inctl->length-1] )
				&&       ( ((ushort)inctl->checksum[0] >> 8)   == inctl->buf[inctl->length-2] );
		}
		return (short) inctl->length;		// Return packet length
	}
	return 0;					// Need more input
}

/*
 * Check the received checksum against the packet contents.
 * User calls this routine to see if checksum passed.
 * Note that not all packets have checksums.
 *
 *	inctl		Input control block
 *
 * RETURNS:
 *	!= 0 (true)	When the computed checksum matched
 *	0		Computed checksum does not match packet checksum
 */
int16_t
inp_checksum(s_inpkt *inctl) {
	return inctl->cksumok ? 1 : 0;
}

/*
 * Get n bytes from the received packet :
 *
 *	inctl		Input control block
 *	buf		Buffer where to return count bytes
 *	count		Number of bytes to fetch
 *
 * RETURNS:
 *	0		Successful
 *	TSIP_SUPPLY	Arg count asked for more than there were
 */
int16_t
inp_getb(s_inpkt *inctl,uchar *buf,ushort count) {
	
	for ( ; count > 0; --count ) {
		if ( inctl->offset >= inctl->length )
			return TSIPER_SUPPLY;
		*buf++ = inctl->buf[inctl->offset++];
	}
	return 0;
}

/*
 * Get a 16-bit signed integer from the received packet :
 *
 *	inctl		Input control block
 *	int16p		Pointer to 16-bit integer
 *	
 *
 * RETURNS:
 *	0		Successful
 *	TSIP_SUPPLY	Arg count asked for more than there were
 */
int16_t
inp_geti16(s_inpkt *inctl,int16_t *int16p) {
	uchar bytes[2];
	int16_t rc;
	
	rc = inp_getb(inctl,bytes,sizeof bytes);
	if ( rc )
		return rc;
	*int16p = ((short)bytes[0] << 8) | (short)bytes[1];
	return 0;
}

/*
 * Get a 32-bit signed integer from the received packet :
 *
 *	inctl		Input control block
 *	int32p		Pointer to 32-bit integer
 *	
 *
 * RETURNS:
 *	0		Successful
 *	TSIP_SUPPLY	Arg count asked for more than there were
 */
int16_t
inp_geti32(s_inpkt *inctl,int32_t *int32p) {
	uchar bytes[4];
	short rc;
	
	rc = inp_getb(inctl,bytes,4);
	if ( rc )
		return rc;
	*int32p	= ((int)bytes[0] << 24)
		| ((int)bytes[1] << 16)
		| ((int)bytes[2] <<  8)
		| ((int)bytes[3]);
	return 0;
}

/*
 * Get a 64-bit signed integer from the received packet :
 *
 *	inctl		Input control block
 *	int64p		Pointer to 64-bit integer
 *	
 *
 * RETURNS:
 *	0		Successful
 *	TSIP_SUPPLY	Arg count asked for more than there were
 */
int16_t
inp_geti64(s_inpkt *inctl,int64_t *int64p) {
	int32_t i32;
	short rc;
	
	rc = inp_geti32(inctl,&i32);
	if ( rc )
		return rc;

	*int64p = (int64_t)i32 << 32;

	rc = inp_geti32(inctl,&i32);
	if ( rc )
		return rc;

	*int64p |= (int64_t)i32;
	return 0;
}

/*
 * Get a float from the received packet :
 *
 *	inctl		Input control block
 *	floatp		Pointer to a float
 *
 * RETURNS:
 *	0		Successful
 *	TSIP_SUPPLY	Arg count asked for more than there were
 */
int16_t
inp_getf32(s_inpkt *inctl,float *floatp) {
	union	{
		int32_t	i32;
		float	f32;
	} u;
	short rc;
	
	rc = inp_geti32(inctl,&u.i32);
	if ( rc )
		return rc;
	*floatp = u.f32;
	return 0;
}

/*
 * Get a double from the received packet :
 *
 *	inctl		Input control block
 *	doublep		Pointer to a double
 *
 * RETURNS:
 *	0		Successful
 *	TSIP_SUPPLY	Arg count asked for more than there were
 */
int16_t
inp_getf64(s_inpkt *inctl,double *f64p) {
	union	{
		int64_t	i64;
		double	f64;
	} u;
        short rc;
        
	rc = inp_geti64(inctl,&u.i64);
        if ( rc )
                return rc;
	*f64p = u.f64;
        return 0;
}

/*
 * Initialize s_outpkt for packet formatting (for output)  :
 *
 * 	outctl		Initialized output control block
 * 	buf		Specifies where the output buffer is
 *	bufsize		Specifies the maximum buffer size (bytes)
 */
void
out_init(s_outpkt *outctl,uint8_t *buf,uint16_t bufsiz) {
	outctl->buf = buf;
	outctl->bufsiz = bufsiz;
	outctl->length = 0;
	outctl->checksum = 0;
}

/*
 * Put count data bytes into the output packet:
 *
 *	outctl		Output control block
 *	buf		Data pointer
 *	count		# of bytes to put into the output buffer
 *
 * RETURNS:
 *	0		Success
 *	TSIPER_TOOLONG	Exceeded max size of output buffer
 */
uint16_t
out_putb(s_outpkt *outctl,const uint8_t *buf,uint16_t count) {
	uint8_t byte;

	if ( !outctl->length ) {
		if ( outctl->length >= outctl->bufsiz )
			return TSIPER_TOOLONG;
		outctl->buf[outctl->length++] = TSIP_DLE;		
	}

	for ( ; count > 0; --count ) {
		if ( outctl->length >= outctl->bufsiz )
			return TSIPER_TOOLONG;
		byte = *buf++;
		if ( byte == TSIP_DLE ) {
			if ( outctl->length >= outctl->bufsiz )
				return TSIPER_TOOLONG;
			outctl->buf[outctl->length++] = TSIP_DLE;
		}
		outctl->buf[outctl->length++] = byte;
		outctl->checksum += (int16_t) byte;
	}
	return 0;
}

/*
 * Put int16 value into the output buffer:
 *
 *	outctl		Output control block
 *	i16		16-bit integer value
 *
 * RETURNS:
 *	0		Success
 *	TSIPER_TOOLONG	Exceeded max size of output buffer
 */
uint16_t
out_puti16(s_outpkt *outctl,int16_t int16) {
	uint8_t buf[2];

	buf[0] = ((uint16_t) int16) >> 8;
	buf[1] = ((uint16_t) int16) & 0xFF;
	return out_putb(outctl,buf,sizeof buf);
}

/*
 * Put int32 value into the output buffer:
 *
 *	outctl		Output control block
 *	i32		32-bit integer value
 *
 * RETURNS:
 *	0		Success
 *	TSIPER_TOOLONG	Exceeded max size of output buffer
 */
uint16_t
out_puti32(s_outpkt *outctl,int32_t int32) {
	uint8_t buf[4];

	buf[0] = (((uint32_t) int32) >> 24) & 0xFF;
	buf[1] = (((uint32_t) int32) >> 16) & 0xFF;
	buf[2] = (((uint32_t) int32) >>  8) & 0xFF;
	buf[3] = ((uint32_t) int32) & 0xFF;
	return out_putb(outctl,buf,sizeof buf);
}

/*
 * Put int64 value into the output buffer:
 *
 *	outctl		Output control block
 *	i64		64-bit integer value
 *
 * RETURNS:
 *	0		Success
 *	TSIPER_TOOLONG	Exceeded max size of output buffer
 */
uint16_t
out_puti64(s_outpkt *outctl,int64_t int64) {
	uint8_t buf[8];

	buf[0] = (((uint64_t) int64) >> 56) & 0xFF;
	buf[1] = (((uint64_t) int64) >> 48) & 0xFF;
	buf[2] = (((uint64_t) int64) >> 40) & 0xFF;
	buf[3] = (((uint64_t) int64) >> 32) & 0xFF;
	buf[4] = (((uint64_t) int64) >> 24) & 0xFF;
	buf[5] = (((uint64_t) int64) >> 16) & 0xFF;
	buf[6] = (((uint64_t) int64) >>  8) & 0xFF;
	buf[7] = ((uint64_t) int64) & 0xFF;
	return out_putb(outctl,buf,sizeof buf);
}

/*
 * Put float (f32) value into the output buffer:
 *
 *	outctl		Output control block
 *	f32		float value
 *
 * RETURNS:
 *	0		Success
 *	TSIPER_TOOLONG	Exceeded max size of output buffer
 */
uint16_t
out_putf32(s_outpkt *outctl,float f32) {
	union	{
		int32_t	i32;
		float	f32;
	} u;

	u.f32 = f32;
	return out_puti32(outctl,u.i32);
}

/*
 * Put double (f64) value into the output buffer:
 *
 *	outctl		Output control block
 *	f64		double value
 *
 * RETURNS:
 *	0		Success
 *	TSIPER_TOOLONG	Exceeded max size of output buffer
 */
uint16_t
out_putf64(s_outpkt *outctl,double f64) {
	union	{
		int64_t	i64;
		double	f64;
	} u;
	u.f64 = f64;
	return out_puti64(outctl,u.i64);
}

/*
 * Add the checksum to the end of this current message
 *
 *	outctl		Output control block
 *
 * RETURNS:
 *	0		Success
 *	TSIPER_TOOLONG	Exceeded max size of output buffer
 */
uint16_t
out_checksum(s_outpkt *outctl) {
	int16_t checksum = outctl->checksum;	// Capture checksum
	return out_puti16(outctl,checksum);	// This will modify outctl->checksum
}

/*
 * Close the packet
 *
 *	outctl		Output control block
 *
 * RETURNS:
 *	0		Success
 *	TSIPER_TOOLONG	Exceeded max size of output buffer
 */
uint16_t
out_close(s_outpkt *outctl) {
	if ( outctl->length >= outctl->bufsiz )
		return TSIPER_TOOLONG;
	outctl->buf[outctl->length++] = TSIP_DLE;		

	if ( outctl->length >= outctl->bufsiz )
		return TSIPER_TOOLONG;
	outctl->buf[outctl->length++] = TSIP_ETX;		

	return 0;	/* Success */
}

/*
 * Return the output packet length:
 *
 *	outctl		Output control block
 *
 * RETURNS:
 *	length		In bytes
 */
uint16_t
out_length(s_outpkt *outctl) {
	return outctl->length;
}

/* End tsip.c */
