/* tsip.h : TSIP Protocol Header
 * Warren W. Gay VE3WWG
 * Fri Aug  2 22:06:25 2013
 */
#ifndef TSIP_H
#define TSIP_H

#include <stdint.h>

typedef unsigned char uchar;
typedef unsigned short ushort;

#define TSIP_DLE	((uchar)0x10)
#define TSIP_ETX	((uchar)0x03)

#define TSIPER_TOOLONG	(-1)		// Buffer too small for this message
#define TSIPER_SUPPLY	(-2)		// Insufficient supply for count

typedef enum {
	Start,		// DLE -> WaitID
	WaitID,		// Byte (none DLE/ETX) -> WaitData, ETX/DLE -> Start
	WaitData,	// XX -> WaitData, DLE -> EscData
	EscData,	// DLE -> WaitData, ETX -> EndPacket, else -> Start
	EndPacket	// Check checksum (if bad -> Start)
} InState;

typedef struct {
	uint8_t		*buf;		// receiving buffer
	uint16_t	bufsiz;		// max length of receiving buffer
	uint16_t	length;		// current buffer length
	int16_t		checksum[3];	// computed checksums
	InState		state;		// current receive state
	int16_t		cksumok;	// True if checksum ok
	uint16_t	offset;		// Extraction offset
} s_inpkt;

void    inp_init(s_inpkt *inctl,uint8_t *buf,uint16_t bufsiz);
int16_t inp_addb(s_inpkt *inctl,uint8_t byte);

int16_t	inp_getb(s_inpkt *inctl,uchar *buf,ushort count);
int16_t inp_geti16(s_inpkt *inctl,int16_t *int16p);
int16_t	inp_geti32(s_inpkt *inctl,int32_t *int32p);
int16_t	inp_geti64(s_inpkt *inctl,int64_t *int64p);
int16_t	inp_getf32(s_inpkt *inctl,float *floatp);
int16_t	inp_getf64(s_inpkt *inctl,double *f64p);

int16_t inp_checksum(s_inpkt *inctl);


typedef struct {
	uint8_t		*buf;		// Encoding buffer size
	uint16_t	bufsiz;		// Buffer size in bytes
	uint16_t	length;		// Current encoded length
	int16_t		checksum;	// Current computed checksum
} s_outpkt;

void	 out_init(s_outpkt *outctl,uint8_t *buf,uint16_t bufsiz);

uint16_t out_putb(s_outpkt *outctl,const uint8_t *buf,uint16_t count);
uint16_t out_puti16(s_outpkt *outctl,int16_t int16);
uint16_t out_puti32(s_outpkt *outctl,int32_t int32);
uint16_t out_puti64(s_outpkt *outctl,int64_t int64);
uint16_t out_putf32(s_outpkt *outctl,float f32);
uint16_t out_putf64(s_outpkt *outctl,double f64);

uint16_t out_checksum(s_outpkt *outctl);
uint16_t out_close(s_outpkt *outctl);

uint16_t out_length(s_outpkt *outctl);

typedef enum {
	None 		= 0x10,
	R13		= 0x13,
	R1A		= 0x1A,
	R1A00		= 0x1A00,
	R3D		= 0x3D,
	R40		= 0x40,
	R41		= 0x41,
	R42		= 0x42,
	R43		= 0x43,
	R44		= 0x44,
	R45		= 0x45,
	R46		= 0x46,
	R47		= 0x47,
	R48		= 0x48,
	R49		= 0x49,
	R4A		= 0x4A,
	R4B		= 0x4B,
	R4C		= 0x4C,
	R4D		= 0x4D,
	R4E		= 0x4E,
	R4F		= 0x4F,
	R53		= 0x53,
	R54		= 0x54,
	R55		= 0x55,
	R56		= 0x56,
	R57		= 0x57,
	R58		= 0x58,
	R59		= 0x59,
	R5A		= 0x5A,
	R5B		= 0x5B,
	R5C		= 0x5C,
	R60		= 0x60,
	R61		= 0x61,
	R6A		= 0x6A,
	R6A00		= 0x6A00,
	R6A01		= 0x6A01,
	R6D		= 0x6D,
	R6E		= 0x6E,
	R6E01		= 0x6E01,
	R70		= 0x70,
	R76		= 0x76,
	R78		= 0x78,
	R7B		= 0x7B,
	R7B00		= 0x7B00,
	R7B04		= 0x7B04,
	R7B05		= 0x7B05,
	R7B06		= 0x7B06,
	R7B80		= 0x7B80,
	R7B84		= 0x7B84,
	R7B85		= 0x7B85,
	R7B86		= 0x7B86,
	R7D		= 0x7D,
	R7D00		= 0x7D00,
	R7D01		= 0x7D01,
	R7D02		= 0x7D02,
	R7D03		= 0x7D03,
	R7D05		= 0x7D05,
	R7D06		= 0x7D06,
	R7D09		= 0x7D09,
	R7D7F		= 0x7D7F,
	R82		= 0x82,
	R83		= 0x83,
	R84		= 0x84,
	R85		= 0x85,
	R87		= 0x87,
	R8700		= 0x8700,
	R8701		= 0x8701,
	R8702		= 0x8702,
	R8703		= 0x8703,
	R8704		= 0x8704,
	R8705		= 0x8705,
	R8706		= 0x8706,
	R8708		= 0x8708,
	R8709		= 0x8709,
	R870A		= 0x870A,
	R877D		= 0x877D,
	R877E		= 0x877E,
	R877F		= 0x877F,
	R88		= 0x88,
	R8800		= 0x8800,
	R8801		= 0x8801,
	R8802		= 0x8802,
	R8803		= 0x8803,
	R8804		= 0x8804,
	R8808		= 0x8808,
	R887F		= 0x887F,
	R8B		= 0x8B,
	R8B00		= 0x8B00,
	R8B01		= 0x8B01,
	R8B02		= 0x8B02,
	R8B03		= 0x8B03,
	R8D		= 0x8D,
	R8D00		= 0x8D00,
	R8D01		= 0x8D01,
	R8D02		= 0x8D02,
	R8D03		= 0x8D03,
	R8D04		= 0x8D04,
	R8F		= 0x8F,
	R8F20		= 0x8F20,
	R8F60		= 0x8F60,
	R8F62		= 0x8F62,
	R8F64		= 0x8F64,
	R8F6B		= 0x8F6B,
	R8F6D		= 0x8F6D,
	R8F6F		= 0x8F6F,
	R8F70		= 0x8F70,
	R8F71		= 0x8F71,
	R8F72		= 0x8F72,
	R8F73		= 0x8F73,
	R8F74		= 0x8F74,
	R8F75		= 0x8F75,
	R8F76		= 0x8F76,
	R8F77		= 0x8F77,
	R8F78		= 0x8F78,
	R8F79		= 0x8F79,
	R8F7A		= 0x8F7A,
	R8F7B		= 0x8F7B,
	R8F7C		= 0x8F7C,
	R8F7D		= 0x8F7D,
	R8F7E		= 0x8F7E,
	R8F7F		= 0x8F7F,
	R8F80		= 0x8F80,
	R8F81		= 0x8F81,
	R8F82		= 0x8F82,
	R8F84		= 0x8F84,
	R8F85		= 0x8F85,
	R8F86		= 0x8F86,
	R8F87		= 0x8F87,
	R8F88		= 0x8F88,
	R8F89		= 0x8F89,
	R8F8A		= 0x8F8A,
	R8F8B		= 0x8F8B,
	R8F8E		= 0x8F8E,
	R8F8F		= 0x8F8F,
	R8F90		= 0x8F90,
	R8F91		= 0x8F91,
	R8F92		= 0x8F92,
	R8F94		= 0x8F94,
	R8F95		= 0x8F95,
	R8F96		= 0x8F96,
	R8F97		= 0x8F97,
	R8F98		= 0x8F98,
	R8F9A		= 0x8F9A,
	RB0		= 0xB0,
	RB080		= 0xB080,
	RB081		= 0xB081,
	RB082		= 0xB082,
	RB0C0		= 0xB0C0,
	RB0C1		= 0xB0C1,
	RB0C2		= 0xB0C2,
	RB0C3		= 0xB0C3,
	RB0C4		= 0xB0C4,
	RBB		= 0xBB,
	RBB00		= 0xBB00,
	RBC		= 0xBC,
} ReportID;


#endif // TSIP_H

/* End tsip.h */
