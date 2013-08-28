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
// Response 0xBB -- Set Primary Receiver Configuration
//////////////////////////////////////////////////////////////////////

struct s_RBB00 {
	uint8_t	opdim;		// Operating dimension
	uint8_t	dgps_mode;	// DGPS mode
	uint8_t	dyn_mode;	// Dynamics mode
	uint8_t	sol_mode;	// Solution mode (1==overdetermine fix)
	float	elev_mask;	// Elevation mask (default 0.1745 radians (10 deg))
	float	amu_mask;	// Min. signal level for fixes (default 4.0)
	float	pdop_mask;	// Max PDOP for fixes (8)
	float	pdop_switch;	// Selects 2D/3D mode (6)
	uint8_t	dgps_age;	// Max time to use a DGPS correction (secs) (30)
	uint8_t	foliage_mode;	// Zero
	uint8_t	reserved1;
	uint8_t	reserved2;
	uint8_t	meas_rate;	// Measurement rate (1 Hz) Supply Zero
	uint8_t	posfx_rate;	// Position fix rate (1 Hz) Supply Zero
	uint8_t	reserved3;
};

//////////////////////////////////////////////////////////////////////
// R8FA5 - Request Packet Broadcast Mask (also C8EA5)
//////////////////////////////////////////////////////////////////////

struct s_R8FA5 {
	union	{
		uint16_t flags;
		uint16_t x8F20 : 1;	// Enable 0x8F-20 on port
		uint16_t reserved : 4;
		uint16_t auto_tsip : 1;	// Enable auto TSIP outputs
		uint16_t x8FAB : 1;	// Enable 0x8F-AB primary timeing info 
		uint16_t x8FAC : 1;	// Enable 0x8F-AC supplemental timing
		uint16_t x8F0B_sya : 1;	// Synchronous 0x8F0B (1 Hz)
		uint16_t x8F0B_eva : 1;	// Event output (port A)
		uint16_t x8F0B_evb : 1;	// Event output port B
		uint16_t x8F0B_syb : 1;	// Synchronous 0x8F0B on port B
		uint16_t x8FAD_eva : 1;	// Event 0x8FAD on port A
		uint16_t x8FAD_syb : 1;	// Synchronous 0x8FAD on port B
		uint16_t x8FAD_evb : 1;	// Event 0x8FAD on port B
	} u;
	int16_t	mbz;
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
	bool get(s_R8FA5& recd);
	bool get(s_R8FAB& recd);
	bool get(s_RBB00& recd);

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

	bool C8EA5(s_R8FA5& parms);	// Set Packet Broadcast Mask

	bool CBB00(s_RBB00& parms);

	inline uint16_t size() { return buflen; }
};

#endif // TSIP_HPP

// End tsip.hpp
