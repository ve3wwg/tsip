//////////////////////////////////////////////////////////////////////
// trimble.cpp -- Main Program
// Date: Mon Aug 19 20:52:44 2013
///////////////////////////////////////////////////////////////////////

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <termios.h>
#include <assert.h>

#include "ttyio.hpp"
#include "tsip.hpp"

#include <unordered_set>

struct termios tios, sv_tios;
bool quit = false;

static void
cdump(uint8_t *packet,int plen) {
	int x;
	
	printf("CMD[%02d] : ",plen);

	for ( x=0; x<plen; ++x )
		printf("%02X ",packet[x]);

	fputs(";\n",stdout);
	fflush(stdout);
}

static void
cmdcb(Packet& pkt,char cmd) {
	TxPacket tx;
	uint8_t buf[512];

	tx.open(buf,sizeof buf);

	fprintf(stderr,"\nCMD = '%c'\n",cmd);
	fflush(stderr);

	switch ( cmd ) {
	case '?' :
		printf(	"v - Software Version\n"
			"V - Hardware Version\n"
			"p - GPS Position Fix\n"
			"m - GPS System Message Request\n"
			"L - Signals Levels Request (27)\n"
			"c - Clear\n"
			"t - Time request\n"
			"f - Firmware\n"
			"F - Factory Reset Request\n"
			"k - Cold Reset\n"
			"r - Software Reset\n"
			"h - Health Request\n"
			"A - Almanac Request (20)\n"
			"x/q Quit\n");
		break;
	case 'A' :
		printf("20 - Almanac Request (A)\n");
		tx.C20();
		pkt.put(buf,tx.size());
		cdump(buf,tx.size());
		break;
	case 't' :
		printf("21 - Time Request\n");
		tx.C21();
		pkt.put(buf,tx.size());
		cdump(buf,tx.size());
		break;
	case 'v' :
		printf("1C01 - Software Version\n");
		tx.C1C01();
		pkt.put(buf,tx.size());
		cdump(buf,tx.size());
		break;
	case 'V' :
		printf("1C03 - Hardware version\n");
		tx.C1C03();
		pkt.put(buf,tx.size());
		cdump(buf,tx.size());
		break;
	case 'p' :
		printf("24 - GPS Receiver Position Fix Mode Request\n");
		tx.C24();
		pkt.put(buf,tx.size());
		cdump(buf,tx.size());
		break;
	case 'r' :
		printf("25 - Soft Reset/Self Test (r)\n");
		tx.C25();
		pkt.put(buf,tx.size());
		cdump(buf,tx.size());
		break;
	case 'm' :
		printf("28 - GPS System Message Request (m)\n");
		tx.C28();
		pkt.put(buf,tx.size());
		cdump(buf,tx.size());
		break;
	case 'h' :
		printf("26 - Health Request\n");
		tx.C26();
		pkt.put(buf,tx.size());
		cdump(buf,tx.size());
		break;
	case 'k' :
		printf("1E 'K' - Cold Reset (K)\n");
		tx.C1E('K');
		pkt.put(buf,tx.size());
		cdump(buf,tx.size());
		break;
	case 'F' :
		printf("1E 'R' - Factory Reset (F)\n");
		tx.C1E('R');
		pkt.put(buf,tx.size());
		cdump(buf,tx.size());
		break;
	case 'L' :
		printf("27 - Signal Levels Request (L)\n");
		tx.C27();
		pkt.put(buf,tx.size());
		cdump(buf,tx.size());
		break;
	case 'f' :
		printf("1F - Software Versions Request(f)\n");
		tx.C1F();
		pkt.put(buf,tx.size());
		cdump(buf,tx.size());
		break;
	case 'x' :
	case 'q' :
		tcsetattr(0,TCSANOW,&sv_tios);
		exit(0);
		break;
	}

	fflush(stdout);
	fflush(stderr);
}

static void
dump(uint8_t *packet,int plen,bool ended) {
	int x;
	
	printf("PKT[%02d] : ",plen);

	for ( x=0; x<plen; ++x )
		printf("%02X ",packet[x]);
	if ( ended )
		fputs(";\n",stdout);
	else	fputs("-\n",stdout);
	fflush(stdout);
}

int
main(int argc,char **argv) {
	Packet pkt;
	RxPacket rxpkt;
	uint8_t *packet = 0;
	int pktlen;
	int rc;
	bool ended;
	std::unordered_set<uint8_t> idset;
	uint16_t id;

	if ( argc < 2 ) {
		pkt.open();
		pkt.registercb(cmdcb);

		rc = tcgetattr(0,&tios);
		sv_tios = tios;
		assert(!rc);
		cfmakeraw(&tios);
		tios.c_oflag |= OPOST | ONLCR;
		rc = tcsetattr(0,TCSANOW,&tios);
		assert(!rc);
	} else	{
		pkt.open(0,1024,0);
	}

	for (;;) {
		fflush(stdout);
		fflush(stderr);
		pkt.get(&packet,&pktlen,ended);
		if ( pktlen <= 0 ) {
			puts("<EOF>");
			break;
		}
		dump(packet,pktlen,ended);

		rxpkt.load(packet,pktlen);
		id = rxpkt.id();

		switch ( id ) {
		case 0x40 :
			{
				s_R40 r;
				if ( !rxpkt.get(r) ) {
					printf(" ERR %d\n",rxpkt.get_offset());
				} else	{
					printf("  satellite = %u\n",r.satellite);
					printf("  t_zc      = %f\n",r.t_zc);
					printf("  week_no   = %d\n",r.week_no);
					printf("  eccentricity = %f\n",r.eccentricity);
					printf("  t_oa      = %f\n",r.t_oa);
					printf("  i_o       = %f\n",r.i_o);
					printf("  omega_dot = %f\n",r.omega_dot);
					printf("  sqrt_a    = %f\n",r.sqrt_a);
					printf("  omega_o   = %f\n",r.omega_o);
					printf("  omega     = %f\n",r.omega);
					printf("  m_o       = %f\n",r.m_o);
				}
			}
			break;
		case 0x41 :
			{
				s_R41 r;
				if ( !rxpkt.get(r) ) {
					printf(" ERR %d\n",rxpkt.get_offset());
				} else	{
					printf("  time      = %f\n",r.time);
					printf("  week      = %d\n",r.week);
					printf("  offset    = %f\n",r.offset);
				}
			}
			break;
		case 0x42 :
			{
				s_R42 r;
				if ( !rxpkt.get(r) ) {
					printf(" ERR %d\n",rxpkt.get_offset());
				} else	{
					printf("  X = %f\n",r.x);
					printf("  Y = %f\n",r.y);
					printf("  Z = %f\n",r.z);
					if ( !rxpkt.is_double() )
						printf("  t = %f GPS secs\n",r.u.time_of_fix1);
					else 	printf("  t = %lf GPS secs\n",r.u.time_of_fix2);
				}
			}
			break;
		case 0x43 :
			{
				s_R43 r;
				if ( !rxpkt.get(r) ) {
					printf("  ERR %u\n",rxpkt.get_offset());
				} else	{
					printf("  x_velocity = %f\n",r.x_velocity);
					printf("  y_velocity = %f\n",r.y_velocity);
				}
			}
			break;
		case 0x46 :
			{
				s_R46 r;
				if ( !rxpkt.get(r) ) {
					printf(" ERR %d\n",rxpkt.get_offset());
				} else	{
					printf("  status = %d\n",int(r.status));
					printf("  error_code = %02X\n",r.u.error_code);
					printf("    bat failed = %d\n",r.u.flags.battery_failed);
					printf("    ant fault  = %d\n",r.u.flags.antenna_fault);
					printf("    exc errors = %d\n",r.u.flags.excessive_errs);
				}
			}
			break;
		case 0x48 :
			{
				s_R48 r;
				if ( !rxpkt.get(r) ) {
					printf(" ERR %d\n",rxpkt.get_offset());
				} else	{
				}
			}
			break;
		case 0x4B :
			{
				s_R4B r;
				if ( !rxpkt.get(r) ) {
					printf(" ERR %d\n",rxpkt.get_offset());
				} else	{
					printf("  machine_id = %02X\n",r.machine_id);
				}
			}
			break;
		case 0x5B :
			{
				s_R5B r;
				if ( !rxpkt.get(r) ) {
					printf(" ERR %d\n",rxpkt.get_offset());
				} else	{
					printf("  sv_prn        = %02X\n",r.sv_prn);
					printf("  coltime       = %f\n",r.coltime);
					printf("  health        = %02X\n",r.health);
					printf("  iode          = %02X\n",r.iode);
					printf("  t_oe          = %f secs\n",r.t_oe);
					printf("  fit_ival_flag = %02X\n",r.fit_ival_flag);
					printf("  ura           = %f m\n",r.ura);
				}
			}
			break;
		case 0x6D :
			{
				s_R6D r;
				if ( !rxpkt.get(r) ) {
					printf(" ERR %d\n",rxpkt.get_offset());
				} else	{
					printf("  fixmod = %02X\n",r.fixmode);
					printf("  PDOP   = %f\n",r.pdop);
					printf("  HDOP   = %f\n",r.hdop);
					printf("  VDOP   = %f\n",r.vdop);
					printf("  TDOP   = %f\n",r.tdop);
					for ( uint8_t ux=0; ux<r.n; ++ux ) {
						printf("  SVPRN[%u] = %02X\n",ux,r.sv_prn[ux]);
					}
				}
			}
			break;
		case 0x82 :
			{
				s_R82 r;
				if ( !rxpkt.get(r) ) {
					printf(" ERR %d\n",rxpkt.get_offset());
				} else	{
					printf("  mode      = %d\n",r.mode);
				}
			}
			break;
		default :
			idset.insert(packet[0]);
			puts(" ???");
		}
	}

	//////////////////////////////////////////////////////////////
	// Display all IDs encountered
	//////////////////////////////////////////////////////////////

	puts("\nUnknown IDs Encountered:");

	for ( auto it=idset.begin(); it != idset.end(); ++it ) {
		uint8_t id = *it;
		printf("ID %02X\n",id);
	}

	return 0;
}

// End trimble.cpp

