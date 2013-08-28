//////////////////////////////////////////////////////////////////////
// tsip.hpp -- TSIP Messages
// Date: Wed Aug 21 19:31:54 2013   (C) datablocks.net
///////////////////////////////////////////////////////////////////////

#ifndef TSIP_HPP
#define TSIP_HPP

#include <stdint.h>

//////////////////////////////////////////////////////////////////////
// Response 40 : Almanac Data for Single Satellite 
//////////////////////////////////////////////////////////////////////

struct s_R40 {
	uint8_t	satellite;	//  SV pseudorandom number (PRN) 1-32 
	float	t_zc;		//  Units are seconds 
	int16_t	week_no;	//  Week number 
	float	eccentricity;	//  Dimensionless 
	float	t_oa;		//  seconds 
	float	i_o;		//  radians 
	float	omega_dot;	//  radians/sec 
	float	sqrt_a;		//  meters ** 0.5 
	float	omega_o;	//  radians 
	float	omega;		//  radians 
	float	m_o;		//  radians 
};
	
//////////////////////////////////////////////////////////////////////
// Response 41 : GPS Time 
//////////////////////////////////////////////////////////////////////

struct s_R41 {
	float	time;		//  GPS time of week (seconds) 
	int16_t	week;		//  GPS week number (weeks) 
	float	offset;		//  UTC/GPS time offset 
};

//////////////////////////////////////////////////////////////////////
// Response 42 : Single Precision Position Fix (XYZ Cartesian ECEF) 
//////////////////////////////////////////////////////////////////////

struct s_R42 {
	float	x;		//  Position along x-axis 
	float	y;		//  Position along y-axis 
	float	z;		//  Position along z-axis 
	union {			//  Time of position solution in GPS seconds 
	  float  time_of_fix1;
	  double time_of_fix2;
	}	u;
};

//////////////////////////////////////////////////////////////////////
// Response 43 : Velocity Fix (XYZ Cartesian ECEF) 
//////////////////////////////////////////////////////////////////////

struct s_R43 {
	float	x_velocity;	//  Velocity of vehicle along x-axis (m/sec) 
	float	y_velocity;	//  Velocity of vehicle along y-axis (m/sec) 
	float	z_velocity;	//  Velocity of vehicle along z-axis (m/sec) 
	float	bias_rate;	//  Bias rate in m/sec 
	union {			//  Time of position fix in GPS seconds 
	  float  time_of_fix1;
	  double time_of_fix2;
	}	u;
};
	
//////////////////////////////////////////////////////////////////////
// Response 45 : Receiver Firmware Information 
//////////////////////////////////////////////////////////////////////

struct s_R45 {
	uint8_t	major;		// Navigation processor..
	uint8_t	minor;
	uint8_t	month;
	uint8_t	day;
	uint8_t	year;		// Year - 1900
	uint8_t	major2;		// Signal processor..
	uint8_t	minor2;
	uint8_t	month2;
	uint8_t	day2;
	uint8_t	year2;		// Year - 1900
};

//////////////////////////////////////////////////////////////////////
// Response 46 : Health of Receiver 
//////////////////////////////////////////////////////////////////////

enum Status46 {
	DoingPositionFixes	= 0x00,
	DoNotHaveGPSTimeYet	= 0x01,
	PDOPIsTooHigh		= 0x03,
	NoUsableSatellites	= 0x08,
	Only1UsableSat		= 0x09,
	Only2UsableSats		= 0x0A,
	Only3UsableSats		= 0x0B,
	ChosenSatIsUnusable	= 0x0C
};

struct s_R46 {
	Status46 	status;		//  Current status of receiver (flags) 
	union	{
		uint8_t	error_code;	//  Error status (flags) 
		struct	{
			uint8_t	battery_failed 	: 1;
			uint8_t	reserved1	: 3;
			uint8_t	antenna_fault	: 1;
			uint8_t	excessive_errs  : 1;
			uint8_t reserved2	: 2;
		} flags;
	} u;
};

//////////////////////////////////////////////////////////////////////
// Response 48 : GPS System Message 
//////////////////////////////////////////////////////////////////////

struct s_R48 {
	uint8_t	message[22];	//  Free form ASCII message 
};
	
//////////////////////////////////////////////////////////////////////
// Response 4B : Machine/Code ID and Additional Status 
//////////////////////////////////////////////////////////////////////

struct s_R4B {
	uint8_t		machine_id;	//  Machine ID for receiver 
	union	{
		uint8_t	raw_status1;
		struct {
			uint8_t	reserved1          : 3;
			uint8_t	almanac_incomplete : 1;	// Stored almanac is not complete
			uint8_t	reserved2          : 4;
		} status1;
	} u1;
	uint8_t		status2;	// != 0 means super packets are supported
};

//////////////////////////////////////////////////////////////////////
// Response 5B : Satellite Ephemeris Status 
//////////////////////////////////////////////////////////////////////

struct s_R5B {
	uint8_t	sv_prn;		//  Pseudorandom number of satellite 
	float	coltime;	//  GPS time when Ephemeris data collected from sat 
	uint8_t	health;		//  6-bit ephemeris health 
	uint8_t	iode;		//  Issue of Data Ephemerus 
	float	t_oe;		//  Seconds 
	uint8_t	fit_ival_flag;
	float	ura;		//  Meters 
};

//////////////////////////////////////////////////////////////////////
// Response 6D : All-In-View Satellite Selection 
//////////////////////////////////////////////////////////////////////

struct s_R6D {
	uint8_t	fixmode;	//  GPS postion fix mode 
	float	pdop;		//  Precision Dilution of Precision 
	float	hdop;		//  Horizontal dilution 
	float	vdop;		//  Vertical dilution 
	float	tdop;		//  Time dilution 
	uint8_t n;		//  # of entries in sv_prn[]
	uint8_t	sv_prn[33];	//  Pseudorandom number (0-32) of first sat in view 
};

//////////////////////////////////////////////////////////////////////
// Response 82 : Differential Position Fix Mode 
//////////////////////////////////////////////////////////////////////

enum Mode82 {
	ManualGPSOn	= 0,
	ManualDGPSOn	= 1,
	AutoGPSOff	= 2,
	AutoDGPSOff	= 3
};

enum RtcmVers {
	Auto		= 0,
	Vers1Only	= 1,
	Vers2OrPRCType9	= 2
};

struct s_R82 {
	Mode82	mode;		//  Manual GPS (Differential Off)
	RtcmVers rtcm_vers;	//  RTCM version used to compute positionsa
	int16_t	refstatn;	//  Reference station ID (-1=accept any) 
};

//////////////////////////////////////////////////////////////////////
// Response 47 : Signal Levels for All 
//////////////////////////////////////////////////////////////////////

struct s_R47 {
	uint8_t	count;		//  Number of satellite records in packet 
	struct	{
		uint8_t	prn;		//  PRN number of first satellite 
		float	siglevel;	//  Signal level of first satellite 
	} sat[12];
};

//////////////////////////////////////////////////////////////////////
// Response 49 : Almanac Health Page 
//////////////////////////////////////////////////////////////////////

struct s_R49 {
	uint8_t health[32];	//  0 == healthy, else flags 
};

//////////////////////////////////////////////////////////////////////
//  Response 4A : Single-Precision LLA Position Fix 
//////////////////////////////////////////////////////////////////////

struct s_R4A {
	float	latitude;	//  Latitude used on position solution computation + for North, - for South 
	float	longitude;	//  Longitude used on the position solution computation + for East, - for West 
	float	altitude;	//  Altitude used in position solution 
	float	clock_bias;	//  Clock bias 
	union {			//  Time of fix (seconds) 
	  float	time_of_fix1;
	  double	time_of_fix2;
	}	u;
};

//////////////////////////////////////////////////////////////////////
// Response 4C : Operating Parameters 
//////////////////////////////////////////////////////////////////////

struct s_R4C {
	uint8_t	dynamics_code;	//  Reports the expected vehicle dynamics and is used to assist solution (default 1=Land) 
	float	elevation_mask;	//  Reports lowest angle at which the receiver can use a satellite (radians) 
	float	signal_level_mask; //  Reports the minimum signal level for fixes (AMUs) 
	float	pdop_mask;	//  Maximum PDOP for calculating position fixes 
	float	podp_switch;	//  Influences 2D or 3D fix depending on PDOP 
};

//////////////////////////////////////////////////////////////////////
// Response 4D : Oscillator Offset 
//////////////////////////////////////////////////////////////////////

struct s_R4D {
	float	offset;	//  GPS oscillator offset in Hz 
};

//////////////////////////////////////////////////////////////////////
// Response 4E : GPS Time Command 
//////////////////////////////////////////////////////////////////////

struct s_R4E {
	uint8_t	yn;	//  Returns 'Y' or 'N' 
};

//////////////////////////////////////////////////////////////////////
// Response 4F : UTC Parameters 
//////////////////////////////////////////////////////////////////////

struct s_R4F {
	double	a0;		//  Refer to ICD-GPS-200 specification 
	float	a1;
	int16_t	delta_t_ls;
	float	tot;
	int16_t	wn_t;
	int16_t	wn_lsf;
	int16_t	dn;
	int16_t	delta_t_lsf;
};

//////////////////////////////////////////////////////////////////////
// Response 54 : One Satellite Bias and Bias Rate 
//////////////////////////////////////////////////////////////////////

struct s_R54 {
	float	bias;		//  One satellite bias (meters) 
	float	bias_rate;	//  Click bias rate (m/sec) 
	union {			//  Time of position fix in GPS seconds 
	  float	time_of_fix1;
	  double	time_of_fix2;
	}	u;
};

//////////////////////////////////////////////////////////////////////
// Response 55 : I/O Options 
//////////////////////////////////////////////////////////////////////

struct s_R55 {
	uint8_t	position;	//  Position flags 
	uint8_t	velocity;	//  Flags 
	uint8_t	timing;		//  Flags 
	uint8_t	auxiliary;	//  Flags 
};

//////////////////////////////////////////////////////////////////////
// Response 56 : Velocity Fix East-North (ENU) 
//////////////////////////////////////////////////////////////////////

struct s_R56 {
	float	eastvel;	//  East velocity + for east, - for west (m/sec) 
	float	northvel;	//  North velocity + for north, - for south (m/sec) 
	float	upvel;		//  Up velocity + for up, - for down 
	float	clock_bias_rate; //  Clock bias rate (m/sec) 
	union {			//  Time when fix computed (seconds) 
	  float	time_of_fix1;
	  double time_of_fix2;
	}	u;
};

//////////////////////////////////////////////////////////////////////
// Response 57 : Last Computed Fix 
//////////////////////////////////////////////////////////////////////

struct s_R57 {
	uint8_t	info_src;	//  Source of info (flags) 
	uint8_t	track_mode;	//  Tracking mode
	float	fix_time;	//  Time of last position fix in GPS seconds 
	int16_t	fix_week;	//  Week of last position fix, in GPS weeks 
};

//////////////////////////////////////////////////////////////////////
// Response 58 : Satellte Sytem Data 
//////////////////////////////////////////////////////////////////////

struct s_R58 {
	enum Type	{
		NotUsed		= 1,
		Almanac		= 2,
		Health		= 3,
		Ionosphere	= 4,
		UTC		= 5,
		Ephemeris	= 6
	};

	uint8_t	operation;	//  Type of satellite operation (flags) 
	Type	datatype;	//  Type of satellite information included (flags) 
	uint8_t	sv_prn;		//  Satellite information in the rpt is for all satellites (0) or specific 
	uint8_t	n;		//  Length of satellite data 
	union	{
		struct s_almanac {
			uint8_t	t_oa_raw;
			uint8_t	sv_health;
			float	e;
			float	t_oa;
			float	i_o;
			float	omegadot;
			float	sqrt_a;
			float	omega_0;
			float	omega;
			float	m_0;
			float	a_f0;
			float	a_f1;
			float	axis;
			float	n;
			float	omega_n;
			float	odot_n;
			float	t_zc;
			int16_t	weeknum;
			int16_t	wn_oa;
		} s2;
		struct s_health {
			uint8_t	weekno;
			uint8_t	sv_health[32];
			uint8_t	t_oa;
			uint8_t	cur_t_oa;
			int16_t	cur_weekno;
		} s3;
		struct s_ionosphere {
			uint8_t	compressed[8];
			float	alpha_0;
			float	alpha_1;
			float	alpha_2;
			float	alpha_3;
			float	beta_0;
			float	beta_1;
			float	beta_2;
			float	beta_3;
		} s4;
		struct s_utc {
			uint8_t	compressed[13];
			double	a_0;
			float	a_1;
			int16_t	delta_t_ls;
			float	t_ot;
			int16_t	wn_t;
			int16_t	wn_lsf;
			int16_t	dn;
			int16_t	delta_t_lsf;
		} s5;
		struct s_ephemeris {
			uint8_t	sv_prn;
			float	t_ephem;
			int16_t	weekno;
			uint8_t	codel2;
			uint8_t	l2pdata;
			uint8_t	svacc_raw;
			uint8_t	sv_health;
			int16_t	iodc;
			float	t_gd;
			float	t_oc;
			float	a_f2;
			float	a_f1;
			float	a_f0;
			float	svacc;
			uint8_t	iode;
			uint8_t	fit_ival;
			float	c_rs;
			float	delta_n;
			double	m_0;
			float	c_uc;
			double	e;
			float	c_us;
			double	sqrt_a;
			float	t_oe;
			float	c_ic;
			double	omega_0;
			float	c_is;
			double 	i_o;
			float	c_rc;
			double	omega;
			float	omegadot;
			float	idot;
			double	axis;
			double	n;
			double	r1me2;
			double	omega_n;
			double	odot_n;
		} s6;
	} u;
};

//////////////////////////////////////////////////////////////////////
// Response 59 : Satellite Attribute Database Status 
//////////////////////////////////////////////////////////////////////

struct s_R59 {
	uint8_t	operation;	//  0x03:1-32 is enable/disable; 0x06:1-32 heed/ignore 
	uint8_t	sv_flags[32];	//  flags 
};

//////////////////////////////////////////////////////////////////////
// Response 5A : Raw Measurement Data 
//////////////////////////////////////////////////////////////////////

struct s_R5A {
	uint8_t	sv_prn;		//  Satellite pseudorandom number (1-32) 
	float	samplength;	//  Time elapsing while a measurement is averaged (msec) 
	float	siglevel;	//  Approx of C/N0, in AMU's. 
	float	code_phase;	//  Average Coarse/Acquisition code delay 
	float	doppler;	//  Apparent carrier frequency offset (Hz) 
	double	time;		//  Center of the sample interval (sconds) 
};

//////////////////////////////////////////////////////////////////////
// Response 5F 11 - EEPROM Segment Status Reports
//////////////////////////////////////////////////////////////////////

struct s_R5F11 {
	int16_t status;
};

//////////////////////////////////////////////////////////////////////
// Response 5C : Satellite Tracking Status 
//////////////////////////////////////////////////////////////////////

struct s_R5C {
	uint8_t	sv_prn;		//  Pseudorandom number of sat. 
	uint8_t	chan_slot;	//  Channel and slot code 
	uint8_t	aquisflag;	//  Aquisition flag 
	uint8_t	ephemflag;	//  Ephemeris flag 
	float	siglevel;	//  Signal level (Same as 0x5A) 
	float	gps_time;	//  GPS Time of last measurement 
	float	elevation;	//  Approximate elevation of sat above horiz (radians) 
	float	azimuth;	//  Approximate azimut from true north (radians) 
	uint8_t	oldmeas;	//  Old measurement flat 
	uint8_t	intmsec;	//  Status of the integer millisecond range to spec. sat. 
	uint8_t	baddata;	//  Bad data flag (or zero) 
	uint8_t	datacol;	//  Data collect flag (0=not collecting) 
};

//////////////////////////////////////////////////////////////////////
// Response 1C81 - Software Version Information
//////////////////////////////////////////////////////////////////////

struct s_R1C81 {
	uint8_t	reserved1;
	uint8_t	major_firm;	// Major number of firmware
	uint8_t	minor_firm;	// Minor firmware no.
	uint8_t	build_no;
	uint8_t	month;
	uint8_t	day;
	int16_t	year;
	uint8_t	length;
	uint8_t	prodname[255];	// Returned product name
};

//////////////////////////////////////////////////////////////////////
// Response 1C83 - Hardware Version Information
//////////////////////////////////////////////////////////////////////

struct s_R1C83 {
	uint32_t serialno;	// Board serial no.
	uint8_t	day;		// Build day
	uint8_t	month;		// Build month
	uint16_t year;		// Build year
	uint8_t	hour;
	uint8_t	hardw_code;	// Hardware code
	uint8_t	length;		// Length of hardw_id returned
	uint8_t	hardw_id[255];	// Hardware ID
};

//////////////////////////////////////////////////////////////////////
// Response 83 : Double Precision XYZ Position Fix and Clock Bias 
//////////////////////////////////////////////////////////////////////

struct s_R83 {
	double	x;		//  X coordinate (m) 
	double	y;		//  Y coordinate (m) 
	double	z;		//  Z coordinate (m) 
	double	clock_bias;	//  Clock bias (m) 
	union {			//  GPS time or UTC (I/O timing option) 
	  float	time_of_fix1;
	  double time_of_fix2;
	}	u;
};

//////////////////////////////////////////////////////////////////////
// Response 84 : Double Precision LLA Position Fix & Clock Bias 
//////////////////////////////////////////////////////////////////////

struct s_R84 {
	double	latitude;	//  Latitude of position (+north) radians 
	double	longitude;	//  Longitude of position (+east) radians 
	double	altitude;	//  Altitude (m) 
	double	clock_bias;	//  (m) 
	union {			//  GPS or UTC (seconds) 
	  float	time_of_fix1;
	  double	time_of_fix2;
	}	u;
};

//////////////////////////////////////////////////////////////////////
// Response 8F 41 : Manufacturing Parameters
//////////////////////////////////////////////////////////////////////

struct s_R8F41 {
	int16_t	serprefix;		// Board serial # prefix
	uint32_t serialno;		// Board serial #
	uint8_t	year;
	uint8_t	month;
	uint8_t	day;
	uint8_t	hour;
	float	oscoffset;		// Oscillator offset
	int16_t	testcode;		// Test code identification number
};

//////////////////////////////////////////////////////////////////////
// Response 8F 42 : Production Parameters
//////////////////////////////////////////////////////////////////////

struct s_R8F42 {
	uint8_t	optsprefix;		// Production options prefix
	uint8_t	pnextension;		// Production number extension
	int16_t	csnpref;		// Case serial number prefix
	uint32_t caseser;		// Case serial number
	uint32_t prodno;		// Production number
	int16_t reserved1;
	int16_t	machid;			// Machine identification number
	int16_t reserved2;
};

//////////////////////////////////////////////////////////////////////
// Response 8F AB : Time Information
//////////////////////////////////////////////////////////////////////

struct s_R8FAB {
	uint64_t	tow;		// Time of week
	uint32_t	weekno;		// Week number
	int32_t		utc_offset;	// Seconds
	union	{
		uint8_t	raw;
		struct	{
			uint8_t	utc_time : 1;	// 1=UTC, else GPS time
			uint8_t	reserved1 : 1;
			uint8_t	time_set : 1;	// 1=set, else not se
			uint8_t	have_utc : 1;	// 0=have UTC info, 1=no UTC info
		} bits;
	} 		timing_flags;
	uint8_t		seconds;
	uint8_t		minutes;
	uint8_t		hours;
	uint8_t		mday;			// Day of month
	uint8_t		month;			// 1-12
	uint32_t	year;			// Four digits of year
};

//////////////////////////////////////////////////////////////////////
// Parse a Received Packet
//////////////////////////////////////////////////////////////////////

class RxPacket {
	uint8_t		*buf;		// receiving buffer
	uint16_t	length;		// current buffer length
	uint16_t	offset;		// Extraction offset
	bool		state_sd;	// Single/Double precision

public:	RxPacket();
	void load(uint8_t *buf,uint16_t buflen);
	inline uint16_t size() { return length; }
	inline uint16_t get_offset() { return offset; }

	uint16_t id();
	bool get(uint8_t& byte);
	uint16_t get(uint8_t *buf,uint16_t count);

	bool get(int16_t& ival);
	bool get(int32_t& ival);
	bool get(int64_t& ival);

	bool get(uint16_t& uval);
	bool get(uint32_t& uval);
	bool get(uint64_t& uval);

	bool get(float& fval);
	bool get(double& fval);

	bool get(s_R1C81& recd);
	bool get(s_R1C83& recd);
	bool get(s_R40& recd);
	bool get(s_R41& recd);
	bool get(s_R42& recd);
	bool get(s_R43& recd);
	bool get(s_R45& recd);
	bool get(s_R46& recd);
	bool get(s_R47& recd);
	bool get(s_R48& recd);
	bool get(s_R49& recd);
	bool get(s_R4A& recd);
	bool get(s_R4B& recd);
	bool get(s_R4C& recd);
	bool get(s_R4D& recd);
	bool get(s_R4E& recd);
	bool get(s_R4F& recd);
	bool get(s_R54& recd);
	bool get(s_R55& recd);
	bool get(s_R56& recd);
	bool get(s_R57& recd);
	bool get(s_R58& recd);
	bool get(s_R59& recd);
	bool get(s_R5A& recd);
	bool get(s_R5B& recd);
	bool get(s_R5C& recd);
	bool get(s_R5F11& recd);
	bool get(s_R6D& recd);
	bool get(s_R82& recd);
	bool get(s_R83& recd);
	bool get(s_R84& recd);
	bool get(s_R8F41& recd);
	bool get(s_R8F42& recd);
	bool get(s_R8FAB& recd);

	inline void set_precision(bool dprecision) { state_sd = dprecision; }
	inline bool is_double() { return state_sd; }
};

//////////////////////////////////////////////////////////////////////
// Encode a Packet
//////////////////////////////////////////////////////////////////////

class TxPacket {
	uint8_t		*buf;
	uint16_t	maxlen;
	uint16_t	buflen;

protected:
	bool put_asis(uint8_t byte);

public:	TxPacket();

	void open(uint8_t *buf,uint16_t maxlen);
	bool put(uint8_t byte);
	bool put(const uint8_t *buf,uint16_t len);
	bool put(int16_t ival);
	bool put(int32_t ival);
	bool put(int64_t ival);
	bool put(float fval);
	bool put(double fval);
	bool command(uint16_t id);
	bool close();

	bool C1C01();			// Software Version Information
	bool C1C03();			// Hardware Version Information
	bool C1E(char rtype='R');	// Factory Reset
	bool C1F();			// Software Versions Request
	bool C20();			// Almanac Request
	bool C21();			// Time Request
	bool C23(float x,float y,float z); // Initial Position (XYZ Cartesian ECEF) Command
	bool C24();			// GPS Receiver Position Fix Mode Request
	bool C25();			// Initiate Soft Reset / Self Test Command
	bool C26();			// Health Request
	bool C27();			// Signal Levels Request
	bool C28();			// GPS System Message Request
	bool C29();			// Almanac Health Page Request
	bool C2A();			// Altitude for 2-D Mode Command
	bool C2A(int16_t alt_meters);
	bool C2A_cancel();
	bool C2B(float latitude,float longitude,float altitude);
	bool C2D();			// Oscillator Offset Request
	bool C2E(float gps_time,int16_t weekno); // GPS Time Command
	bool C2F();			// UTC Parameters Request
	bool C31(float x,float y,float z); // Accurate Initial Position (XYZ Cartesian ECEF) Command
	bool C32(float latitude,float longitude,float altitude); // Accurate Initial Position (Latitude, Longitude, Altitude) Command
	bool C34(uint8_t prn=0);	// Satellite Number For One-Satellite Mode Command

	bool C35(
	  bool pos_xyz_ecef_on,
	  bool lla_on,
	  bool lla_alt,			// 1=MSL geoid else hae (current datum)
	  bool alt_input,		// 1=MSL geoid  else hae (current datum)
	  bool pos_precision,		// 0=single, 1=double
	  bool r8F20_on,		// 1=8F20 enabled
	  bool vel_xyz_ecef_on,
	  bool vel_enu,
	  bool timing_utc,		// 1=UTC else GPS
	  bool timing_comp,		// 1=Next integer sec else ASAP
	  bool timing_fix,		// 1=Only on request else when computed
	  bool timing_sync,		// 1=Synchronized else off
	  bool timing_minproj,		// 1=minimize projection on
	  bool aux_raw,			// 1=raw measurements on
	  bool aux_smoothed,		// 1=Doppler smoothed codephase else raw
	  bool aux_db_hz		// 1=Output db/Hz (0x47) vs AMU (0x5A/5C)
	);

	bool C37();			// Last Position and Velocity Request
	bool C3A(uint8_t prn=0);	// Last Raw Measurement Request for sat prn
	bool C3B(uint8_t prn=0);	// Satellite Ephemeris Status Request
	bool C3C(uint8_t prn=0);	// Satellite Tracking Status Request
	bool C3F11();			// EEPROM Segment Commands

	

	inline uint16_t size() { return buflen; }
};

#if 0
// Response 13 : TSIP Parsing Error 
struct s_R13 {
	uint8_t	packet_id;	//  Packet ID 
	uint8_t	contents[255];	//  Contents of errant packet 
};
	
// Response 1A 00 : RTCM Wrapper / Port A Echo 
struct s_R1A00 {
	uint8_t	data[255];	//  The raw data 
};
	
// Response 3D : Serial Port A Configuration 
struct s_R3D {
	uint8_t	output_baud_rate;	//  Output baud rate 
	uint8_t	input_baud_rate;	//  Input baud rate 
	uint8_t	parity_bits;	//  Parity and # bits 
	uint8_t	stop_flow;	//  Stop bits and hardware flow control 
	uint8_t	out_protocol;	//  Output protocol 
	uint8_t	in_protocol;	//  Input protocol 
};
	
// Response 44 : Non-Overdetermined Satellite Selection 
struct s_R44 {
	uint8_t	flag;	//  Mode 
	uint8_t	satellite1;	//  Satellite number 1 
	uint8_t	satellite2;	//  Satellite number 2 
	uint8_t	satellite3;	//  Satellite number 3 
	uint8_t	satellite4;	//  Satellite number 4 
	float	pdop;	//  Precision dilution of precision 
	float	hdop;	//  Horizontal dilution of precision 
	float	vdop;	//  Vertical dilution of precision 
	float	tdop;	//  Time dilution of precision 
};
	

// Response 5E : Additional Fix Status 
struct s_R5E {
	uint8_t	prevmeas;	//  Measurements used in current fix used in the previous fix/status 
	uint8_t	oldmeas;	//  Number of old measurements in current fix (2-3s) 
};

// Response 60 : Differential GPS Pseudorange Corrections 
struct s_R60 {
	int16_t	zcount;	//  Modified Z-count in units of 0.6 seconds 
	uint8_t	station_health;	//  Health of the reference station 
	uint8_t	verstype;	//  Bits identify RTCM version number and record type 
	uint8_t	scale1;	//  Flags identify RTCM scale factor 
	int16_t	prc1;	//  Pseudorandom correction 
	uint8_t	rrc1;	//  Range rate correction, note signed 
	uint8_t	iode1;	//  The IODE used by the reference station 
	uint8_t	scale2;
	int16_t	prc2;
	uint8_t	rrc2;
	uint8_t	iode2;
	uint8_t	scale3;
	int16_t	prc3;
	uint8_t	rrc3;
	uint8_t	iode3;
	uint8_t	scale4;
	int16_t	prc4;
	uint8_t	rrc4;
	uint8_t	iode4;
	uint8_t	scale5;
	int16_t	prc5;
	uint8_t	rrc5;
	uint8_t	iode5;
	uint8_t	scale6;
	int16_t	prc6;
	uint8_t	rrc6;
	uint8_t	iode6;
	uint8_t	scale7;
	int16_t	prc7;
	uint8_t	rrc7;
	uint8_t	iode7;
	uint8_t	scale8;
	int16_t	prc8;
	uint8_t	rrc8;
	uint8_t	iode8;
	uint8_t	scale9;
	int16_t	prc9;
	uint8_t	rrc9;
	uint8_t	iode9;
	uint8_t	scale10;
	int16_t	prc10;
	uint8_t	rrc10;
	uint8_t	iode10;
	uint8_t	scale11;
	int16_t	prc11;
	uint8_t	rrc11;
	uint8_t	iode11;
	uint8_t	scale12;
	int16_t	prc12;
	uint8_t	rrc12;
	uint8_t	iode12;
};

// Response 61 : Differential GPS Delta Pseudorange Corrections 
struct s_R61 {
	int16_t	zcount;	//  Modified Z count in units of 0.6 seconds 
	uint8_t	vers;	//  RTCM version and number of satellites included in report 
	uint8_t	scale1;	//  Scale/UDRE/SV# of each satellite included 
	int16_t	dprc1;	//  RTCM SC-104 delta pseudorange corrections 
	uint8_t	scale2;
	int16_t	dprc2;
	uint8_t	scale3;
	int16_t	dprc3;
	uint8_t	scale4;
	int16_t	dprc4;
	uint8_t	scale5;
	int16_t	dprc5;
	uint8_t	scale6;
	int16_t	dprc6;
	uint8_t	scale7;
	int16_t	dprc7;
	uint8_t	scale8;
	int16_t	dprc8;
	uint8_t	scale9;
	int16_t	dprc9;
	uint8_t	scale10;
	int16_t	dprc10;
	uint8_t	scale11;
	int16_t	dprc11;
	uint8_t	scale12;
	int16_t	dprc12;
};

// Response 6A 00 : Differential Corrections Used in Fix 
struct s_R6A00 {
	int32_t	fixtimetag;	//  Milliseconds into week of fix 
	uint8_t	sats;	//  Number of satellites 
	uint8_t	satid1;	//  Satellite PRN (1-32) 
	float	rc1;	//  Range correction (meters) 
	float	rrc1;	//  Range rate correction 
	uint8_t	satid2;
	float	rc2;
	float	rrc2;
	uint8_t	satid3;
	float	rc3;
	float	rrc3;
	uint8_t	satid4;
	float	rc4;
	float	rrc4;
	uint8_t	satid5;
	float	rc5;
	float	rrc5;
	uint8_t	satid6;
	float	rc6;
	float	rrc6;
	uint8_t	satid7;
	float	rc7;
	float	rrc7;
	uint8_t	satid8;
	float	rc8;
	float	rrc8;
	uint8_t	satid9;
	float	rc19;
	float	rrc9;
	uint8_t	satid10;
	float	rc110;
	float	rrc10;
	uint8_t	satid11;
	float	rc111;
	float	rrc11;
	uint8_t	satid12;
	float	rc112;
	float	rrc12;
};

// Response 6A 01 : Fix Differential Corrections Output Control 
struct s_R6A01 {
	uint8_t	outen;	//  Output enabled (1) or disabled(0) 
	uint8_t	reserved1;	//  Zero 
	uint8_t	reserved2;	//  Zero 
};

// Response 6E 01 : Synchronized Measurement Parameters 
struct s_R6E01 {
	uint8_t	enable;	//  0=Disabled, 1=filtered, 3=enabled (raw) 
	uint8_t	outival;	//  Output interval in seconds (1-255) 
};

// Response 6F 01 : Synchronized Measurements 
struct s_R6F01 {
	uint8_t	preamble;	//  Begin preamble 
	int16_t	length;	//  No. of bytes: preamble to postamble inclusive 
	double	recvtime;	//  Time of GPS week (msecs) 
	double	clkoffset;	//  Receiver clock offset (msecs) 
	uint8_t	nsats;	//  Number of satellites in packet 
	uint8_t	sv_prn1;	//  Pseudorandom number of satellite (1-32) 
	uint8_t	flagsa1;	//  Flags 1 
	uint8_t	flagsb1;	//  Flags 2 
	uint8_t	elevation_angle1;	//  Satellite elevation angle (degrees) 
	int16_t	azimuth1;	//  Satellite azimuth (degrees) 
	uint8_t	snrx241;	//  Number of AMUs x 4 
	double	pseudorange1;	//  Full L1 C/A pseudorange (meters) 
	double	phase1;	//  L1 band continuous phase 
	float	doppler1;	//  L1 band doppler 
	uint8_t	sv_prn2;
	uint8_t	flagsa2;
	uint8_t	flagsb2;
	uint8_t	elevation_angle2;
	int16_t	azimuth2;
	uint8_t	snrx242;
	double	pseudorange2;
	double	phase2;
	float	doppler2;
	uint8_t	sv_prn3;
	uint8_t	flagsa3;
	uint8_t	flagsb3;
	uint8_t	elevation_angle3;
	int16_t	azimuth3;
	uint8_t	snrx243;
	double	pseudorange3;
	double	phase3;
	float	doppler3;
	uint8_t	sv_prn4;
	uint8_t	flagsa4;
	uint8_t	flagsb4;
	uint8_t	elevation_angle4;
	int16_t	azimuth4;
	uint8_t	snrx244;
	double	pseudorange4;
	double	phase4;
	float	doppler4;
	uint8_t	sv_prn5;
	uint8_t	flagsa5;
	uint8_t	flagsb5;
	uint8_t	elevation_angle5;
	int16_t	azimuth5;
	uint8_t	snrx245;
	double	pseudorange5;
	double	phase5;
	float	doppler5;
	uint8_t	sv_prn6;
	uint8_t	flagsa6;
	uint8_t	flagsb6;
	uint8_t	elevation_angle6;
	int16_t	azimuth6;
	uint8_t	snrx246;
	double	pseudorange6;
	double	phase6;
	float	doppler6;
	uint8_t	sv_prn7;
	uint8_t	flagsa7;
	uint8_t	flagsb7;
	uint8_t	elevation_angle7;
	int16_t	azimuth7;
};

// Response 70 : Position/Velocity Filter Operation 
struct s_R70 {
	uint8_t	dynfilter;	//  Dynamic filter 0=disabled/1=enabled 
	uint8_t	statfilter;	//  Static filter 0=disabled/1=enabled 
	uint8_t	altfilter;	//  Altitude filter 0=disabled/1=enabled 
	uint8_t	reserved;	//  Zero 
};

// Response 76 : Overdetermined Mode 
struct s_R76 {
	uint8_t	fixtype;	//  Overdetermined position fix mode (1=overdetermined/2=weighted) 
};

// Response 78 : Maximum PRC Age 
struct s_R78 {
	int16_t	maxprcage;	//  Max pseudorange correction age (seconds) 
};

// Response 7B 00 : NMEA Output Control 
struct s_R7B00 {
	uint8_t	interval;	//  Time between NMEA messages (pos fix rate when zero) 
	uint8_t	msgmask[4];	//  NMEA bit-mask for outputing messages 
};

// Response 7B 06 : NMEA Message Options and Precision 
struct s_R7B06 {
	uint8_t	message;	//  GGA Options and Precision 
	uint8_t	optflags;	//  GGA Option values 
	uint8_t	precision;	//  GGA precision (0-7 decimals) 
};

// Response 7B 80 : NMEA Interval and Message Mask 
struct s_R7B80 {
	uint8_t	interval;	//  Time between NMEA messages (pos fix rate when zero) 
	uint8_t	msgmask[4];	//  NMEA bit mask for outputing messages 
};

// Response 7B 84 : NMEA Name List/Message Mask 
struct s_R7B84 {
	uint8_t	contents;	//  0x01 Message name list only 
	uint8_t	msglist[253];	//  Comma-delimited NMEA name list 
};

// Response 7B 85 : NMEA Local Time Offsets 
struct s_R7B85 {
	uint8_t	houroffset;	//  GMT hours - local hours 
	uint8_t	minoffset;	//  GMT minuts - local minutes 
};

// Response 7B 86 : NMEA Message Options and Precision 
struct s_R7B86 {
	uint8_t	message;
	uint8_t	optflags;
	uint8_t	precision;	//  0-7 decimals 
};

// Response 7D 00 : Position Fix Rate Configuration 
struct s_R7D00 {
	uint8_t	asap_rate;	//  Position fixes per second (1-10) 
};

// Response 7D 01 : Position Fix Rate Options 
struct s_R7D01 {
	uint8_t	opt1flags;
	uint8_t	opt2flags;
};

// Response 7D 03 : Maximum Measurement Age 
struct s_R7D03 {
	int16_t	max_age;	//  1-5000 ms : Allow meas used in pos to be at most this old 
};

// Response 7D 05 : CTS to Transmit Delay 
struct s_R7D05 {
	int16_t	cts_delay;	//  0-32000 ms 
};

// Response 7D 06 : RTS Trailing Edge Delay 
struct s_R7D06 {
	int16_t	rts_delay;	//  0-32000 ms 
};

// Response 7D 09 : Time-Based Message Interval 
struct s_R7D09 {
	uint8_t	port;
	uint8_t	protocol;	//  0=TSIP/1=NMEA 
	uint8_t	interval;	//  interval code 
	uint8_t	reserved[8];
	int16_t	checksum;
};

// Response 85 : Differential Correction Status 
struct s_R85 {
	uint8_t	sv_prn;	//  Pseudorandom number of satellite (1-32) 
	uint8_t	stats_code;	//  flags 
	uint8_t	stn_health;	//  Health of reference station used by receiver 
	uint8_t	sv_health;
	uint8_t	iode1;	//  Issue of data emphemeris for pseudo random correction 
	uint8_t	iode2;	//  Delta issue of data ephmeris 
	float	zcount;	//  Z count as time of week 
	float	prc;	//  Pseudorange correction PRC 
	float	rrc;	//  Range-rate correction 
	float	deltaprc;	//  Delta pseudorange correction 
};

// Response 87 00 : Reference Station Control 
struct s_R8700 {
	uint8_t	control;	//  flag : ref station on (1), else off (0) 
};

// Response 87 01 : Reference Station Options 
struct s_R8701 {
	uint8_t	options1;
	uint8_t	options2;
};

// Response 87 02 : Reference Station Output Version 
struct s_R8702 {
	uint8_t	version;	//  RTCM version 
};

// Response 87 03 : Reference Station Position 
struct s_R8703 {
	uint8_t	type;	//  XYZ ECEF (FIXME other types.. need union/special handling) 
	double	x;	//  X coordinate (m) 
	double	y;	//  Y coordinate (m) 
	double	z;	//  Z coordinate (m) 
};

// Response 87 04 : Reference Station ID 
struct s_R8704 {
	int16_t	id;	//  Reference Station ID value 
};

// Response 87 05 : RTCM Type 16 Text 
struct s_R8705 {
	uint8_t	length;	//  Number of bytes in type 16 text (up to 90) 
	uint8_t	text[253];
};

// Response 87 06 : RTCM Type Specifiec Output Intervals 
struct s_R8706 {
	uint8_t	type;	//  coded 
	uint8_t	interval;	//  coded 
};

// Response 87 08 : TSIP Notification of Sent Version 2 
struct s_R8708 {
	uint8_t	type;	//  RTCM message type sent 
	int16_t	stationid;	//  RTCM Station ID sent 
	int16_t	zcount;	//  RTCM Z-count sent 
	int16_t	deltatime;	//  GPS time at transmission minux message z-count time 
	uint8_t	sequno;	//  RTCM sequence number sent 
	uint8_t	length;	//  RTCM length sent 
	uint8_t	health;	//  RTCM station health sent 
	uint8_t	svcount;	//  Number of satellites sent in this message: 0==none 
	uint8_t	svprn[233];	//  PRN of satellites sent (1-32) 
};

// Response 87 09 : Average Position - Ref Station Position 
struct s_R8709 {
	uint8_t	type;	//  0x00: XYZ Delta (fixme -- more types) 
	float	x;	//  delta x 
	float	y;	//  delta y 
	float	z;	//  delta z 
};

// Response 87 0A : Time Schedule Message Interval and Offset 
struct s_R870A {
	uint8_t	type;	//  flags 
	int16_t	interval;	//  Reports time in seconds between RTCM message output 
	int16_t	offset;	//  Reports the delay of RTCM message output 
};

// Response 87 7D : Mobile Packet Ignored by Ref Station 
struct s_R877D {
	uint8_t	id;	//  Packet id of the ignored packet 
	uint8_t	length;	//  length of the ignroed packet 
	uint8_t	first;	//  The first byte of the ignored packet or 255 if none 
};

// Response 87 7E : Ref Station Warnings 
struct s_R877E {
	uint8_t	warning;	//  0x00: Port A Output Language is not RTCM or not packat 0x60 or 0x61 
};

// Response 87 7F : Ref Station Option Not Installed 
struct s_R877F {
	uint8_t	none;	//  Fixme (there is no byte 1) 
};

// Response 88 00 : Mobile Differential Parameters 
struct s_R8800 {
	uint8_t	mode;	//  flags 
};

// Response 88 01 : Mobile Differential Options 
struct s_R8801 {
	uint8_t	options1;	//  flags 
	uint8_t	options2;	//  flags 
};

// Response 88 02 : Mobile Differential Input Version 
struct s_R8802 {
	uint8_t	version;	//  flags 
};

// Response 88 03 : Masking Ref Station 
struct s_R8803 {
	uint8_t	type;	//  0x00: XYZ Earth-centered, earth-fixd (ECEF) coordinates (fixme more types) 
	double	x;	//  meters 
	double	y;	//  meters 
	double	z;	//  meters 
};

// Response 88 04 : Input Reference Station ID 
struct s_R8804 {
	int16_t	id;	//  Sets the reference station id: -1 accept any, else accept only this id 
};

// Response 88 05 : Last Received RTCM Type 16 
struct s_R8805 {
	uint8_t	length;	//  No. of chars received (0-90) Fixme: 0xFF == No type 16 received 
	int16_t	id;	//  id of the ref station 
	int32_t	time;	//  Z count of RTCM type 16 record converted to GPS time of week (msecs) 
	uint8_t	text[241];	//  received text, including zero padding, if any 
};

// Response 88 08 : TSIP Notification of Received Version 2 RTCM 
struct s_R8808 {
	uint8_t	type;	//  RTCM Message type received 
	int16_t	stnid;	//  RTCM station id 
	int16_t	zcount;	//  RTCM z count received 
	int16_t	deltatime;	//  GPS time at reception, minus message z count time 
	uint8_t	seqno;	//  RTCM sequence no. received 
	uint8_t	length;	//  length of RTCM message 
	uint8_t	health;	//  RTCM station health 
	uint8_t	svcount;	//  flags 
	uint8_t	svprn[233];	//  Satellite PRNs received (1-32) 
};

// Response 88 7F : Mobile Differential Option Not Installed 
struct s_R887F {
	uint8_t	fixme;	//  fixme -- there is no byte 1 
};

// Response 8B 00 : Position Sigma Information 
struct s_R8B00 {
	uint8_t	enabled;	//  automatic output: 1=enabled, else 0 
	uint8_t	reserved1[4];	//  zero 
	int16_t	checksum;	//  (fixme?) 
};

// Response 8B 01 : Position VCV Parameters 
struct s_R8B01 {
	uint8_t	enabled;	//  automatic output: 1=enabled, 0=disabled 
	uint8_t	reserved1[4];	//  zero 
	int16_t	checksum;	//  fixme? 
};

// Response 8B 02 : Position Signma Information 
struct s_R8B02 {
	int32_t	fixtime;	//  time of position to which the data pertains in msecs since start of GPS week 
	uint8_t	datavalid;	//  flag 1=invalid, else 0 
	float	rms;	//  Position residual root mean square 
	float	sigmaeast;	//  Sigma east (m) 
	float	sigmanorth;	//  Sigma north (m) 
	float	coven;	//  east-north covariance 
	float	sigmaup;	//  Sigma up (m) 
	uint8_t	axesvalid;	//  1=valid, else 0 
	float	semimajor;	//  semi-major axis (m) 
	float	semiminor;	//  semi-minor axis (m) 
	float	orientation;	//  orientation of semi-major axis from true north (radians) 
	float	unitvariance;
	int16_t	epochs;	//  # of epochs for RTK (only) 
	int16_t	dof;	//  degrees of freedom 
	int16_t	flags;	//  reserved (zero) 
	uint8_t	reserved[4];	//  zero 
	int16_t	checksum;	//  (fixme?) 
};

// Response 8B 03 : Position VCV Information 
struct s_R8B03 {
	int32_t	fixtime;	//  Time of position in msec since start of GPS week 
	uint8_t	datavalid;	//  1=valid, else 0 
	float	rms;	//  position residual root mean square 
	float	vcv_xx;	//  VCV matrix element xx 
	float	vcv_xy;
	float	vcv_xz;
	float	vcv_yy;
	float	vcv_yz;
	float	vcv_zz;
	float	unitvariance;
	int16_t	epochs;	//  no. of epochs (for RTK only) 
	int16_t	dof;	//  degrees of freedom 
	int16_t	flags;	//  reserved (zero) 
	uint8_t	reserved[4];
	int16_t	checksum;	//  fixme? 
};

// Response 8D 00 : Average Position Start/Stop Control 
struct s_R8D00 {
	uint8_t	stopcode;	//  flags 
	uint8_t	status;	//  flags 
};

// Response 8D 01 : Average Position Options 
struct s_R8D01 {
	uint8_t	options1;
	uint8_t	options2;
};

// Response 8D 02 : Auto Stop Parameters (Control/Options) 
struct s_R8D02 {
	uint8_t	type;	//  0x00: auto stop control/options (fixme more types) 
	uint8_t	options1;
	uint8_t	options2;
};

// Response 8D 03 : Current Average Position XYZ ECEF 
struct s_R8D03 {
	uint8_t	type;	//  0x00: XYZ ECEF (fixme x,y,z,time may not be incuded) 
	int32_t	number;	//  number of positions in the average 
	int32_t	duration;	//  average duration in seconds 
	double	x;	//  (fixme if number==0, these fields are not included) 
	double	y;
	double	z;
	int32_t	time;	//  avg time in seconds (if MSB=1, process is stopped and lower bits indicate total averaging time) 
};

// Response 8D 04 : Average Position Delta from Last XYZ or ENU 
struct s_R8D04 {
	uint8_t	type;	//  0=XYZ, 1=ENU 
	int32_t	number;	//  no. of positions in the average 
	int32_t	duration;	//  Averaging duration in seconds (seconds) 
	float	xe;	//  X or E (fixme not included if number==0) 
	float	yn;	//  Y or N 
	float	zu;	//  Z or U 
	int32_t	time;	//  seconds, if MSB=1 process stopped and lower bits indicate total averaging time 
};

// Response 8F 20 : Super Packet Output 
struct s_R8F20 {
	uint8_t	key;
	int16_t	evelocity;	//  east in 0.005 m/sec increments 
	int16_t	nvolocity;	//  north velocity in 0.005 m/sec increments 
	int16_t	uvelocity;	//  up velopcity in 0.005 m/sec increments 
	int32_t	weektime;	//  gps time of the week, msecs 
	int32_t	latitude;
	int32_t	longitude;
	int32_t	altitude;
	uint8_t	reserved1[3];
	uint8_t	posfixflags;
	uint8_t	svcount;	//  no. of SVs (1-8) contributing data used to compute position solution 
	uint8_t	utcoffset;	//  no. of leap seconds between utc and GPS time 
	int16_t	week;	//  gps time of position solution, in weeks 
	uint8_t	svprnxiode1a;
	uint8_t	svprnxiode1b;
	uint8_t	svprnxiode2a;
	uint8_t	svprnxiode2b;
	uint8_t	svprnxiode3a;
	uint8_t	svprnxiode3b;
	uint8_t	svprnxiode4a;
	uint8_t	svprnxiode4b;
	uint8_t	svprnxiode5a;
	uint8_t	svprnxiode5b;
	uint8_t	svprnxiode6a;
	uint8_t	svprnxiode6b;
	uint8_t	svprnxiode7a;
	uint8_t	svprnxiode7b;
	uint8_t	svprnxiode8a;
	uint8_t	svprnxiode8b;
	uint8_t	ionoshperic;
};

// Response 8F 60 : DR Calibration and Status 
struct s_R8F60 {
	float	oscalef;	//  calibrated no. of pulses per mile output by odometer 
	float	glscalef;	//  ratio of actual gyro scale factor to nominal gyro scale factor 
	float	grscalef;
	uint8_t	flag;	//  1=digital, 2=analog 
};

// Response 8F 62 : GPS/DR Position/Velocity 
struct s_R8F62 {
	double	time_tag;	//  Time when position/velocity solution computed 
	double	latitude;	//  meters 
	double	longitude;	//  meters 
	double	altitude;	//  meters 
	double	clockbias;	//  seconds 
	int16_t	possource;	//  table 3-174 
	double	evolocity;	//  m/sec 
	double	nvelocity;	//  m/sec 
	double	uvelocity;	//  m/sec 
	double	clockbiasvel;
	uint8_t	velsource;	//  flag: 0=vel fix, east-north-up (ENU), 1=XYZ Cartesian (ECEF) 
	uint8_t	shstatus;	//  flags 
	uint8_t	svcount;	//  No. of satellites used in position solution computation 
	uint8_t	svprn[101];	//  Pseudorandom number of satellite included in solution 
};

// Response 8F 64 : Firmware Version and Configuration 
struct s_R8F64 {
	uint8_t	rpttype;	//  0x00: Reports the boad config and intermediate frequency (fixme more types) 
	uint8_t	Board[253];	//  config string + IF config string and checksum (varies) 
};

// Response 8F 6B : Last Gyroscope Readings 
struct s_R8F6B {
	double	gheading;	//  Gyro heading 
	double	ghrate;	//  Gyro heading rate (radians) 
	double	ghrbias;	//  Gyro heading rate bias (deg/sec) 
	double	timetag;	//  hh:mm:ss Time when gyroscope reading is computed 
	double	gdutycycle;	//  fraction of 1, representing the heading sensor output 
	uint8_t	health;	//  flags (0=no errors) 
	double	gscalef1;	//  Gyro scale factor 1 
	double	gscalef2;
	float	gvar;	//  Gyro variance (radians^2) 
	float	hcorr;	//  Heading correction (radians) 
};

// Response 8F 6D : Last Odometer Readings 
struct s_R8F6D {
	uint8_t	lastseccount;	//  Pulses counted during last second of reading 
	double	speed;	//  miles/hour 
	double	samptime;	//  Elapsed time while pulses counted (seconds) 
	double	timetag;	//  hh:mm:ss 
	float	scalefactor;	//  Calibrated no. of pulses per mile output by odometer 
	float	variance;	//  Difference in pulses per mile between calibrated scale factor and most recent computation 
	uint8_t	health;	//  flags 
	uint8_t	adswitch;	//  flags 
	uint8_t	potvalue;	//  Sensitivity of odometer input circuit 
	uint8_t	ostatus;	//  0=bad, 1=healthy 
};

// Response 8F 6F : Firmware Version Name 
struct s_R8F6F {
	uint8_t	versname[20];	//  Name assigned to receiver's firmware 
	int16_t	checksum;
};

// Response 8F 70 : Beacon Channel Status Report 
struct s_R8F70 {
	uint8_t	channels1;	//  No. of beacon channels (2) 
	int16_t	frequency1;	//  Beacon channel 0 frequency when searching 
	uint8_t	mode1;	//  flags 
	uint8_t	status1;	//  flag 
	uint8_t	rtcmused1;	//  1=source of differential corrections else =0 (not) 
	uint8_t	snr1;	//  Beacon signal to noise ratio 
	uint8_t	inputlevel1;	//  Beacon channel 0 electromagnitic field intensity (0-255) 
	uint8_t	rateindex1;	//  flags 
	uint8_t	lockind1;	//  Beacon channel 0 lock signal quality 
	uint8_t	carrieroff1;	//  Beacon channel 0 frequency difference between transmitter and receiver oscillators 
	uint8_t	bitrateoff1;	//  Difference between calculated bit rate and theoritical 
	uint8_t	worderrorrate1;	//  No. of RTCM word errors in last 255 words on beacon channel 0 
	uint8_t	health1;	//  Beacon channel 0 health status 
	int16_t	frequency2;	//  Beacon channel 0 frequency when searching 
	uint8_t	mode2;	//  flags 
	uint8_t	status2;	//  flag 
	uint8_t	rtcmused2;	//  1=source of differential corrections else =0 (not) 
	uint8_t	snr2;	//  Beacon signal to noise ratio 
	uint8_t	inputlevel2;	//  Beacon channel 0 electromagnitic field intensity (0-255) 
	uint8_t	rateindex2;	//  flags 
	uint8_t	lockind2;	//  Beacon channel 0 lock signal quality 
	uint8_t	carrieroff2;	//  Beacon channel 0 frequency difference between transmitter and receiver oscillators 
	uint8_t	bitrateoff2;	//  Difference between calculated bit rate and theoritical 
	uint8_t	worderrorrate2;	//  No. of RTCM word errors in last 255 words on beacon channel 0 
	uint8_t	health2;	//  Beacon channel 0 health status 
	int16_t	checksum;
};

// Response 81 71 : DGPS Station Database 
struct s_R8171 {
	uint8_t	recordx;	//  Index No. of database record (0-9) 
	int16_t	stationid;	//  Station identification no. 
	int16_t	frequency;	//  Beacon frequency or 0 if satellite database is used 
	uint8_t	modulationrate;	//  flags 
	uint8_t	health;	//  flags 
	float	latitude;	//  Latitude of reference station (radians) 
	float	longitude;	//  Longitude (radians) 
	float	distance;	//  Computed distance to reference station (meters) 
	float	range;	//  Range of transmission (meters, beacon only) 
	int32_t	seconds;	//  No of seconds since database record was updated 
	uint8_t	uscgx;	//  Index of the beacon frequency used in auto range mode (USCG Index) 
	int16_t	checksum;
};

// Response 8F 73 : Beacon Channel Control Acknowledgement 
struct s_R8F73 {
	int16_t	checksum;
};

// Response 8F 74 : Clear Beacon Database 
struct s_R8F74 {
	int16_t	checksum;
};

// Response 8F 75 : FFT Start 
struct s_R8F75 {
	int16_t	checksum;
};

// Response 8F 76 : FFT Stop 
struct s_R8F76 {
	int16_t	checksum;
};

// Response 8F 77 : FFT Report Packet, First 
struct s_R8F77 {
	uint8_t	pageno;	//  (FIXME -- intermediate and last pages) 
	double	frequency;	//  Center frequency of narrowband/wideband spectral plot in Hz 
	double	binsize;	//  FFT bin size in Hz 
	uint8_t	inpsquared;	//  flags 
	uint8_t	integrations;	//  flags 
	int16_t	nobins;	//  flags 
	float	maxlevel;	//  Maximum power level 
	uint8_t	avgpower[99];	//  Averaged power sample of first to last bins of first report page 
	int16_t	checksum;
};

// Response 8F 78 : RTCM 
struct s_R8F78 {
	uint8_t	channel;	//  Channel 0/1 
	int32_t	time;	//  Transmit time in ms since the start of the week 
	uint8_t	rtcmbytes[245];	//  Total no. of bytes (0-255) contained in the report (FIXME checksum too) 
};

// Response 8F 79 : Beacon Station Attributes 
struct s_R8F79 {
	int16_t	checksum;
};

// Response 8F 7A : Beacon Station Attibutes 
struct s_R8F7A {
	uint8_t	attribute[84];	//  flag 
	int16_t	checksum;
};

// Response 8F 7B : DGPS Receiver RAM Configuration Block 
struct s_R8F7B {
	uint8_t	portno;	//  0=Port A, 1=Port B 
	uint8_t	name[20];	//  Software name 
	uint8_t	majorvers;
	uint8_t	minorvers;
	uint8_t	month;
	uint8_t	day;
	uint8_t	year;	//  Year - 1900 : software release year 
	uint8_t	head;	//  0x55: Start of receiver config block 
	uint8_t	verion;
	uint8_t	serialno[32];
	uint8_t	rcvrday;
	uint8_t	rcvrmonth;
	int16_t	rcvryear;
	int16_t	superpktopt;	//  1=Superpacket available, else 0 
	uint8_t	inputprotocolb;	//  flags 
	uint8_t	outputprotocolb;	//  flags 
	uint8_t	inputbaudb;	//  flags 
	uint8_t	outputbaudb;	//  flags 
	uint8_t	parityb;	//  flags 
	uint8_t	databitsb;	//  0=5..3=8 
	uint8_t	stopbitsb;	//  0=1, 1=2 
	uint8_t	reserved1b;
	uint8_t	inputprotocola;	//  flags 
	uint8_t	outputprotocola;	//  flags 
	uint8_t	inputbauda;	//  flags 
	uint8_t	outputbauda;	//  flags 
	uint8_t	paritya;	//  flags 
	uint8_t	databitsa;	//  0=5..3=8 
	uint8_t	stopbitsa;	//  0=1, 1=2 
	uint8_t	reserved1a;
	uint8_t	productid;
	uint8_t	pvfilter;	//  flags 
	uint8_t	carrierphase;	//  flags 
	uint8_t	pps1flag;	//  flags 
	float	antgain;	//  decibels 
	uint8_t	eventflag;	//  flags 
	uint8_t	beaconenb;	//  1=enabled, else 0 
	uint8_t	maxposrate;	//  flags 
	uint8_t	beaconattr;	//  flags 
	uint8_t	rtcmtimeout;	//  seconds 
	uint8_t	scorpiodecode;	//  1=available, else 0 
	uint8_t	basestation;	//  flag 
	uint8_t	everest;	//  1=available, else 0 
	uint8_t	modemctl;	//  1=available, else 0 
	uint8_t	reserved2;
	int16_t	desubsc;	//  D&E subscription week no. 
	uint8_t	guidance;	//  flags 
	uint8_t	satdiffprov;	//  flags 
	uint8_t	language;	//  flags 
	uint8_t	hardware;	//  flags 
	int16_t	tnlsubsc;	//  TNL subscription week number 
	uint8_t	displayunits;	//  0=metric, 1=U.S. 
	uint8_t	disstream;	//  zero 
	uint8_t	reserved3[40];
	int16_t	endblk;	//  0xAA55 
	int16_t	checksum1;	//  of bytes 27-152 
	int16_t	checksum2;	//  of bytes 0-154 
};

// Response 8F 7C : DGPS Receiver Configuration Block 
struct s_R8F7C {
	int16_t	checksum;
};

// Response 8F 7E : Satellite Line-of-Sight (LOS) 
struct s_R8F7E {
	uint8_t	count;	//  Total number of bytes in packet 
	double	usertime;	//  User time of data, in seconds of the week 
	uint8_t	chanstatus0;	//  Status of satellite tracking channel (flags) 
	uint8_t	chanstatus1;	//  flags 
	double	pseudorange;
	float	deltapseudorange;
	float	pseudorangevariance;
	float	deltapseudovariance;
	double	svxpos;	//  X Position in ECEF coordinates 
	double	svypos;	//  Y Position 
	double	svzpos;	//  Z Position 
	float	svxdelta;
	float	svydelta;
	float	svzdelta;
	float	ionoshpericcorr;	//  Ionospheric correction 
	int16_t	ephemura;	//  Ephemeris URA word 
	float	deltapseudoint;	//  Delta Pseudo Range Interval 
	int16_t	checksum;
};

// Response 8F 7F : DGPS Receiver ROM Configuration 
struct s_R8F7F {
	uint8_t	portno;	//  Port B=0, Port A=1 
	uint8_t	name[20];	//  Software name 
	uint8_t	majorvers;
	uint8_t	minorvers;
	uint8_t	month;
	uint8_t	day;
	uint8_t	year;	//  Year - 1900 
	uint8_t	head;	//  Beginning of receiver configuration block (0x55) 
	uint8_t	version;
	uint8_t	serialno[32];
	uint8_t	rcvrday;
	uint8_t	rcvrmonth;
	int16_t	rcvryear;
	int16_t	superpktopt;	//  1=avail, else 0 
	uint8_t	inpprotocolb;	//  flags 
	uint8_t	outprotocolb;	//  flags 
	uint8_t	inputbaudb;	//  flags 
	uint8_t	outputbaudb;	//  flags 
	uint8_t	parityb;	//  flags 
	uint8_t	databitsb;
	uint8_t	stopbitsb;
	uint8_t	reserved1;
	uint8_t	inpprotocola;
	uint8_t	outprotocola;
	uint8_t	inpbauda;
	uint8_t	outbauda;
	uint8_t	paritya;
	uint8_t	databitsa;
	uint8_t	stopbitsa;
	uint8_t	reserved2;
	uint8_t	prodid;
	uint8_t	pvfilter;	//  1=enabled else 0 
	uint8_t	carrierphase;	//  1=enabled else 0 
	uint8_t	pps1flag;	//  1=enabled else 0 
	float	antgain;	//  decibels 
	uint8_t	eventflag;
	uint8_t	beaconenb;	//  1=enabled else 0 
	uint8_t	maxposrate;	//  flags 
	uint8_t	beaconattr;	//  flags 
	uint8_t	rtcmtimeout;
	uint8_t	scorpiodecode;	//  1=avail else 0 
	uint8_t	basestation;	//  flags 
	uint8_t	everest;	//  1=avail else 0 
	uint8_t	modemctl;	//  1=avail else 0 
	uint8_t	reserved3;
	int16_t	desubscr;	//  D&E subscription week number 
	uint8_t	guidance;	//  flags 
	uint8_t	satdiffprov;	//  flags 
	uint8_t	language;	//  flags 
	uint8_t	hardware;	//  flags 
	int16_t	tnlsubsc;	//  TNL subscription week number 
	uint8_t	dispunits;	//  0=metric, 1=U.S. 
	uint8_t	disstream;	//  zero 
	uint8_t	reserved4[40];
	int16_t	endcode;	//  0xAA55 
	int16_t	checksum1;
	int16_t	checksum2;
};
#endif

#endif // TSIP_HPP

// End tsip.hpp
