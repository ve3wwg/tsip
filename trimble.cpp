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
cmdcb(Packet& pkt,char cmd) {

	fprintf(stderr,"\nCMD = '%c'\n",cmd);
	fflush(stderr);

	switch ( cmd ) {
	case 'c' :
		printf("CLEAR : 10 1E 4B 10 03 ;\n");
		pkt.putb(0x10);
		pkt.putb(0x1E);
		pkt.putb(0x4B);
		pkt.putb(0x10);
		pkt.putb(0x03);
		break;
	case 't' :
		printf("TIME : 10 21 10 03 ; EXP(41)\n");
		pkt.putb(0x10);
		pkt.putb(0x21);
		pkt.putb(0x10);
		pkt.putb(0x03);
		break;
	case 'f' :
		printf("FRMW : 10 1F 10 03 ; EXP(45)\n");
		pkt.putb(0x10);
		pkt.putb(0x1F);
		pkt.putb(0x10);
		pkt.putb(0x03);
		break;
	case 'F' :
		printf("FACTORY : 10 8E 45 03 10 03 ; EXP(?)\n");
		pkt.putb(0x10);
		pkt.putb(0x8E);
		pkt.putb(0x45);
		pkt.putb(0x03);
		pkt.putb(0x10);
		pkt.putb(0x03);
		break;
	case 'r' :
		printf("SRST : 10 25 10 03 ; EXP(41,45,46,4B..)\n");
		pkt.putb(0x10);
		pkt.putb(0x25);
		pkt.putb(0x10);
		pkt.putb(0x03);
		break;
	case 'h' :
		printf("HLTH : 10 26 10 03 ; EXP(46/4B)\n");
		pkt.putb(0x10);
		pkt.putb(0x26);
		pkt.putb(0x10);
		pkt.putb(0x03);
		break;
	case 'x' :
	case 'q' :
		tcsetattr(0,TCSANOW,&sv_tios);
		exit(0);
		break;
	}
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
	uint8_t id;

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
		pkt.get(&packet,&pktlen,ended);
		if ( pktlen <= 0 ) {
			puts("<EOF>");
			break;
		}
		idset.insert(packet[0]);
		dump(packet,pktlen,ended);

		rxpkt.load(packet,pktlen);
		rxpkt.get(id);
		switch ( id ) {
		case 0x40 :
			{
				s_R40 r;
				if ( !rxpkt.get(r) ) {
					puts(" ERR");
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
					puts(" ERR");
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
					puts(" ERR");
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
					puts(" ERR");
				} else	{
				}
			}
			break;
		case 0x4B :
			{
				s_R4B r;
				if ( !rxpkt.get(r) ) {
					puts(" ERR");
				} else	{
					printf("  machine_id = %02X\n",r.machine_id);
				}
			}
			break;
		case 0x5B :
			{
				s_R5B r;
				if ( !rxpkt.get(r) ) {
					puts(" ERR");
				} else	{
				}
			}
			break;
		case 0x6D :
			{
				s_R6D r;
				if ( !rxpkt.get(r) ) {
					puts(" ERR");
				} else	{
				}
			}
			break;
		case 0x82 :
			{
				s_R82 r;
				if ( !rxpkt.get(r) ) {
					puts(" ERR");
				} else	{
					printf("  mode      = %d\n",r.mode);
				}
			}
			break;
		default :
			puts(" ???");
		}
	}

	//////////////////////////////////////////////////////////////
	// Display all IDs encountered
	//////////////////////////////////////////////////////////////

	puts("\nIDs Encountered:");

	for ( auto it=idset.begin(); it != idset.end(); ++it ) {
		uint8_t id = *it;
		printf("ID %02X\n",id);
	}

	return 0;
}

// End trimble.cpp

