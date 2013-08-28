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
			"l - Last Position and Velocity Request\n"
			"a - Last Raw Measurement Request for sat PRN 0 (3A)\n"
			"E - Satellite Ephemeris Status Request (3B)\n"
			"s - Satellite Tracking status (3C)\n"
			"0 - Configuration 0 : Land (BB00)\n"
			"9 - Set Request Packet Broadcast Mask (8EA5)\n"
			"x/q Quit\n");
		break;
	case '0' :
		printf("BB00 - Configuration 0\n");
		{
			s_RBB00 p;

			p.opdim 	= 0;	// Automatic
			p.dgps_mode 	= 3;	// Auto
			p.dyn_mode 	= 1;	// Land
			p.sol_mode 	= 1;
			p.elev_mask 	= 0.1745;
			p.amu_mask 	= 4.0;
			p.pdop_mask 	= 8;
			p.pdop_switch 	= 6;
			p.dgps_age 	= 30;
			p.foliage_mode 	= 0;
			p.meas_rate 	= 0;
			p.posfx_rate 	= 0;

			tx.CBB00(p);
			pkt.put(buf,tx.size());
			cdump(buf,tx.size());
		}
		break;
	case '9' :
		printf("8EA5 - Configure Packet Broadcast Mask\n");
		{
			s_R8FA5 p;

			p.u.x8F20 	= 1;
			p.u.auto_tsip 	= 1;
			p.u.x8FAB 	= 1;
			p.u.x8FAC 	= 1;
			p.u.x8F0B_sya 	= 0;
			p.u.x8F0B_eva 	= 1;
			p.u.x8F0B_evb 	= 0;
			p.u.x8F0B_syb 	= 0;
			p.u.x8FAD_eva 	= 1;
			p.u.x8FAD_syb 	= 0;
			p.u.x8FAD_evb 	= 0;
			p.mbz = 0;

			tx.C8EA5(p);
			pkt.put(buf,tx.size());
			cdump(buf,tx.size());
		}
		break;
	case 's' :
		printf("3C - Satellite Tracking Status\n");
		tx.C3C();
		pkt.put(buf,tx.size());
		cdump(buf,tx.size());
		break;
	case 'a' :
		printf("3A - Last Raw Measurement Request for sat PRN 0\n");
		tx.C3A();
		pkt.put(buf,tx.size());
		cdump(buf,tx.size());
		break;
	case 'E' :
		printf("3B - Satellite Ephemeris Status Request\n");
		tx.C3B(0);
		pkt.put(buf,tx.size());
		cdump(buf,tx.size());
		break;
	case 'l' :
		printf("37 - Last Position and Velocity Request (l)\n");
		tx.C37();
		pkt.put(buf,tx.size());
		cdump(buf,tx.size());
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
					switch ( r.status ) {
					case DoingPositionFixes	:
						puts("  (Doing position fixes)");
						break;
					case DoNotHaveGPSTimeYet :
						puts("  (Do not have GPS time yet)");
						break;
					case PDOPIsTooHigh :
						puts("  (PDOP is too high)");
						break;
					case NoUsableSatellites	:
						puts("  (No usable satellites)");
						break;
					case Only1UsableSat :
						puts("  (Only 1 usable satellite)");
						break;
					case Only2UsableSats :
						puts("  (Only 2 usable satellites)");
						break;
					case Only3UsableSats :
						puts("  (Only 3 usable satellites)");
						break;
					case ChosenSatIsUnusable :
						puts("  (Chosen satellite is unusable)");
						break;
					default :
						;		
					};
					printf("  error_code = %02X\n",r.u.error_code);
					printf("    bat failed = %d\n",r.u.flags.battery_failed);
					printf("    ant fault  = %d\n",r.u.flags.antenna_fault);
					printf("    exc errors = %d\n",r.u.flags.excessive_errs);
				}
			}
			break;
		case 0x47 :
			{
				s_R47 r;
				if ( !rxpkt.get(r) ) {
					printf(" ERR %d\n",rxpkt.get_offset());
				} else	{
					printf("  count = %u\n",r.count);
					for ( unsigned ux=0; ux<r.count; ++ux ) {
						printf("    %02X level %.lf\n",
							r.sat[ux].prn,
							r.sat[ux].siglevel);
					}
				}
			}
			break;
		case 0x48 :
			{
				s_R48 r;
				if ( !rxpkt.get(r) ) {
					printf(" ERR %d\n",rxpkt.get_offset());
				} else	{
					printf(" message: %-22.22s\n",r.message);
				}
			}
			break;
		case 0x49 :
			{
				s_R49 r;
				if ( !rxpkt.get(r) ) {
					printf(" ERR %d\n",rxpkt.get_offset());
				} else	{
					for ( short x=0; x<32; ++x )
						printf("  %2d : %02X\n",x,r.health[x]);
				}
			}
			break;
		case 0x4A :
			{
				s_R4A r;
				if ( !rxpkt.get(r) ) {
					printf(" ERR %d\n",rxpkt.get_offset());
				} else	{
					printf("  latitude    %f\n",r.latitude);
					printf("  longitude   %f\n",r.longitude);
					printf("  altitude    %f\n",r.altitude);
					printf("  clock bias  %f\n",r.clock_bias);
					if ( !rxpkt.is_double() )
						printf("  time of fix %f\n",r.u.time_of_fix1);
					else	printf("  time of fix %lf\n",r.u.time_of_fix2);
				}
			}
			break;
		case 0x4B :
			{
				s_R4B r;
				if ( !rxpkt.get(r) ) {
					printf(" ERR %d\n",rxpkt.get_offset());
				} else	{
					printf("  machine_id         = %02X\n",r.machine_id);
					printf("  almanac incomplete = %d\n",r.u1.status1.almanac_incomplete);
					printf("  super packets      = %02X\n",r.status2);
				}
			}
			break;
		case 0x4E :
			{
				s_R4E r;
				if ( !rxpkt.get(r) ) {
					printf(" ERR %d\n",rxpkt.get_offset());
				} else	{
					printf("  Response '%c'\n",r.yn);
				}
			}
			break;
		case 0x4F :
			{
				s_R4F r;
				if ( !rxpkt.get(r) ) {
					printf(" ERR %d\n",rxpkt.get_offset());
				} else	{
					printf("  a0          %lf\n",r.a0);
					printf("  a1          %f\n",r.a1);
					printf("  delta t_ls  %d\n",r.delta_t_ls);
					printf("  tot         %f\n",r.tot);
					printf("  wn_t        %d\n",r.wn_t);
					printf("  wn_lsf      %d\n",r.wn_lsf);
					printf("  dn          %d\n",r.dn);
					printf("  delta t_lsf %d\n",r.delta_t_lsf);
				}
			}
			break;
		case 0x54 :
			{
				s_R54 r;
				if ( !rxpkt.get(r) ) {
					printf(" ERR %d\n",rxpkt.get_offset());
				} else	{
					printf("  bias         %f\n",r.bias);
					printf("  bias rate    %f\n",r.bias_rate);
					if ( !rxpkt.is_double() )
						printf("  time of fix  %f\n",r.u.time_of_fix1);
					else	printf("  time of fix  %lf\n",r.u.time_of_fix2);
				}
			}
			break;
		case 0x55 :
			{
				s_R55 r;
				if ( !rxpkt.get(r) ) {
					printf(" ERR %d\n",rxpkt.get_offset());
				} else	{
					printf("  position  %02X\n",r.position);
					printf("  velocity  %02X\n",r.velocity);
					printf("  timing    %02X\n",r.timing);
					printf("  auxiliary %02X\n",r.auxiliary);
				}
			}
			break;
		case 0x56 :
			{
				s_R56 r;
				if ( !rxpkt.get(r) ) {
					printf(" ERR %d\n",rxpkt.get_offset());
				} else	{
					printf("  East velocity   %f\n",r.eastvel);
					printf("  North velocity  %f\n",r.northvel);
					printf("  Up velocity     %f\n",r.upvel);
					printf("  Clock bias rate %f\n",r.clock_bias_rate);
					if ( !rxpkt.is_double() )
						printf("  Time of fix     %f\n",r.u.time_of_fix1);
					else	printf("  Time of fix     %lf\n",r.u.time_of_fix2);
				}
			}
			break;
		case 0x57 :
			{
				s_R57 r;
				if ( !rxpkt.get(r) ) {
					printf(" ERR %d\n",rxpkt.get_offset());
				} else	{
					printf("  info src         %02X (%s)\n",r.info_src,
						!r.info_src ? "none" : "regular fix");
					printf("  tracking mode    %02X ",r.track_mode);
					switch ( r.track_mode ) {
					case 0 :
						puts("auto");
						break;
					case 1 :
						puts("time only 1-SV");
						break;
					case 2 :
						puts("2D clock hold");
						break;
					case 3 :
						puts("2D");
						break;
					case 4 :
						puts("3D");
						break;
					case 5 :
						puts("overdetermined clock");
						break;
					case 6 :
						puts("DGPS reference");
						break;
					default :
						printf("??\n");
					}
					printf("  time of last fix  %f\n",r.fix_time);
					printf("  week of last fix  %d\n",r.fix_week);
				}
			}
			break;
		case 0x5A :
			{
				s_R5A r;
				if ( !rxpkt.get(r) ) {
					printf(" ERR %d\n",rxpkt.get_offset());
				} else	{
					printf("  Satellite PRN  %u\n",r.sv_prn);
					printf("  Sample length  %f ms\n",r.samplength);
					printf("  Signal level   %f AMUs\n",r.siglevel);
					printf("  Code phase     %f\n",r.code_phase);
					printf("  Doppler        %f Hz\n",r.doppler);
					printf("  Time           %lf seconds\n",r.time);
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
		case 0x5F11 :
			{
				s_R5F11 r;
				if ( !rxpkt.get(r) ) {
					printf(" ERR %d\n",rxpkt.get_offset());
				} else	{
					printf("  status        = %02X\n",r.status);
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
		case 0x83 :
			{
				s_R83 r;
				if ( !rxpkt.get(r) ) {
					printf(" ERR %d\n",rxpkt.get_offset());
				} else	{
					printf("  x            %lf\n",r.x);
					printf("  y            %lf\n",r.y);
					printf("  z            %lf\n",r.z);
					printf("  clock bias   %lf\n",r.clock_bias);
					if ( !rxpkt.is_double() )
						printf("  time of fix  %f\n",r.u.time_of_fix1);
					else	printf("  time of fix  %lf\n",r.u.time_of_fix2);
				}
			}
			break;
		case 0x84 :
			{
				s_R84 r;
				if ( !rxpkt.get(r) ) {
					printf(" ERR %d\n",rxpkt.get_offset());
				} else	{
					printf("  latitude     %lf\n",r.latitude);
					printf("  longitude    %lf\n",r.longitude);
					printf("  altitude     %lf\n",r.altitude);
					printf("  clock bias   %lf\n",r.clock_bias);
					if ( !rxpkt.is_double() )
						printf("  time of fix  %f\n",r.u.time_of_fix1);
					else	printf("  time of fix  %lf\n",r.u.time_of_fix2);
				}
			}
			break;
		case 0x8FA5 :
			{
				s_R8FA5 r;
				if ( !rxpkt.get(r) ) {
					printf(" ERR %d\n",rxpkt.get_offset());
				} else	{
					printf("  x8F20     = %d\n",r.u.x8F20);
					printf("  auto_tsip = %d\n",r.u.auto_tsip);
					printf("  x8FAB     = %d\n",r.u.x8FAB);
					printf("  x8FAC     = %d\n",r.u.x8FAC);
					printf("  x8F0B_sya = %d\n",r.u.x8F0B_sya);
					printf("  x8F0B_eva = %d\n",r.u.x8F0B_eva);
					printf("  x8F0B_evb = %d\n",r.u.x8F0B_evb);
					printf("  x8F0B_syb = %d\n",r.u.x8F0B_syb);
					printf("  x8FAD_eva = %d\n",r.u.x8FAD_eva);
					printf("  x8FAD_syb = %d\n",r.u.x8FAD_syb);
					printf("  x8FAD_evb = %d\n",r.u.x8FAD_evb);
				}
			}
			break;			
		case 0xBB00 :
			{
				s_RBB00 r;
				if ( !rxpkt.get(r) ) {
					printf(" ERR %d\n",rxpkt.get_offset());
				} else	{
					printf("  opdim        = %u\n",r.opdim);
					printf("  dgps_mode    = %u\n",r.dgps_mode);
					printf("  dyn_mode     = %u\n",r.dyn_mode);
					printf("  sol_mode     = %u\n",r.sol_mode);
					printf("  elev_mask    = %f\n",r.elev_mask);
					printf("  amu_mask     = %f\n",r.amu_mask);
					printf("  pdop_mask    = %f\n",r.pdop_mask);
					printf("  pdop_switch  = %f\n",r.pdop_switch);
					printf("  dgps_age     = %u\n",r.dgps_age);
					printf("  foliage_mode = %u\n",r.foliage_mode);
					printf("  meas_rate    = %u\n",r.meas_rate);
					printf("  posfx_rate   = %u\n",r.posfx_rate);
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

