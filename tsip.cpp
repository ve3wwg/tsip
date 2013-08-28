//////////////////////////////////////////////////////////////////////
// tsip.cpp -- TSIP Packet Encode/Decode
// Date: Wed Aug 21 19:42:11 2013
///////////////////////////////////////////////////////////////////////

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <assert.h>

#include "tsip.hpp"

RxPacket::RxPacket() {
	buf = 0;
	length = 0;
	offset = 0;
	state_sd = false;
}

void
RxPacket::load(uint8_t *buf,uint16_t buflen) {
	this->buf 	= buf;
	length		= buflen;
	offset		= 0;
}

uint16_t
RxPacket::id() {
	uint8_t id;
	uint8_t sub;

	if ( offset >= length )
		return 0x10;		// No ID
	if ( !get(id) )
		return 0x10;

	switch ( id ) {
	case 0x5F :
	case 0x8F :
	case 0xBB :
		if ( !get(sub) )
			return 0x10;
		return (uint16_t(id) << 8) | uint16_t(sub);
		break;
	default :
		break;
	}
	return id;
}

bool
RxPacket::get(uint8_t& byte) {
	if ( offset >= length )
		return false;
	byte = buf[offset++];
	return true;
}

uint16_t
RxPacket::get(uint8_t *buf,uint16_t count) {
	uint16_t rcount = 0;

	for ( ; count > 0 && offset < length; --count, ++rcount )
		*buf++ = this->buf[offset++];
	return rcount;
}

bool
RxPacket::get(int16_t& ival) {
	uint8_t bytes[2];

	if ( get(bytes,2) != 2 )
		return false;
	ival = ((short)bytes[0] << 8) | (short)bytes[1];
	return true;
}

bool
RxPacket::get(uint16_t& uval) {
	uint8_t bytes[2];

	if ( get(bytes,2) != 2 )
		return false;
	uval = ((uint16_t)bytes[0] << 8) | (uint16_t)bytes[1];
	return true;
}

bool
RxPacket::get(int32_t& ival) {
	uint8_t bytes[4];

	if ( get(bytes,4) != 4 )
		return false;
	
	ival	= ((int)bytes[0] << 24)
		| ((int)bytes[1] << 16)
		| ((int)bytes[2] <<  8)
		| ((int)bytes[3]);
	return true;
}

bool
RxPacket::get(uint32_t& uval) {
	uint8_t bytes[4];

	if ( get(bytes,4) != 4 )
		return false;
	
	uval	= ((uint32_t)bytes[0] << 24)
		| ((uint32_t)bytes[1] << 16)
		| ((uint32_t)bytes[2] <<  8)
		| ((uint32_t)bytes[3]);
	return true;
}

bool
RxPacket::get(int64_t& ival) {
	int32_t i32;

	if ( !get(i32) )
		return false;

	ival = (int64_t)i32 << 32;
	if ( !get(i32) )
		return false;
	ival |= i32;
	return true;
}

bool
RxPacket::get(uint64_t& uval) {
	uint32_t u32;

	if ( !get(u32) )
		return false;

	uval = (uint64_t)u32 << 32;
	if ( !get(u32) )
		return false;
	uval |= u32;
	return true;
}

bool
RxPacket::get(float& fval) {
	union	{
		int32_t	i32;
		float	f32;
	} u;

	if ( !get(u.i32) )
		return false;
	fval = u.f32;
	return true;
}

bool
RxPacket::get(double& fval) {
	union	{
		int64_t	i64;
		double	f64;
	} u;

	if ( !get(u.i64) )
		return false;
	fval = u.f64;
	return true;
}

bool
RxPacket::get(s_R40& recd) {
	
	if ( !get(recd.satellite) )
		return false;
	if ( !get(recd.t_zc) )
		return false;
	if ( !get(recd.week_no) )
		return false;
	if ( !get(recd.eccentricity) )
		return false;
	if ( !get(recd.t_oa) )
		return false;
	if ( !get(recd.i_o) )
		return false;
	if ( !get(recd.omega_dot) )
		return false;
	if ( !get(recd.sqrt_a) )
		return false;
	if ( !get(recd.omega_o) )
		return false;
	if ( !get(recd.omega) )
		return false;
	if ( !get(recd.m_o) )
		return false;
	return true;
}

bool
RxPacket::get(s_R41& recd) {
	if ( !get(recd.time) )
		return false;
	if ( !get(recd.week) )
		return false;
	if ( !get(recd.offset) )
		return false;
	return true;
}

bool
RxPacket::get(s_R42& recd) {
	if ( !get(recd.x) )
		return false;
	if ( !get(recd.y) )
		return false;
	if ( !get(recd.z) )
		return false;

	if ( !state_sd ) {
		if ( !get(recd.u.time_of_fix1) )
			return false;
	} else	{
		if ( !get(recd.u.time_of_fix2) )
			return false;
	}

	return true;
}

bool
RxPacket::get(s_R43& recd) {

	if ( !get(recd.x_velocity) )
		return false;
	if ( !get(recd.y_velocity) )
		return false;
	if ( !get(recd.z_velocity) )
		recd.z_velocity = 0.0;
	if ( !get(recd.bias_rate) )
		recd.bias_rate = 0.0;
	if ( state_sd ) {
		if ( !get(recd.u.time_of_fix1) )
			recd.u.time_of_fix1 = 0.0;
	} else	{
		if ( !get(recd.u.time_of_fix2) )
			recd.u.time_of_fix2 = 0.0;
	}
	return true;
}

bool
RxPacket::get(s_R46& recd) {
	uint8_t b;

	if ( !get(b) )
		return false;
	recd.status = Status46(b);
	if ( !get(recd.u.error_code) )
		recd.u.error_code = 0;
	return true;
}

bool
RxPacket::get(s_R48& recd) {
	uint16_t mlen;

	mlen = get(recd.message,sizeof recd.message);
	if ( mlen < sizeof recd.message )
		recd.message[mlen] = 0;
	return true;
}

bool
RxPacket::get(s_R49& recd) {
	uint16_t dlen;

	dlen = get(&recd.health[0],32);
	for ( ; dlen < 32; ++dlen )
		recd.health[dlen] = 0;
	return true;
}

bool
RxPacket::get(s_R4A& recd) {

	if ( !get(recd.latitude) )
		return false;
	if ( !get(recd.longitude) )
		return false;
	if ( !get(recd.altitude) )
		recd.altitude = 0;
	if ( !get(recd.clock_bias) )
		recd.clock_bias = 0;

	if ( state_sd ) {
		if ( !get(recd.u.time_of_fix1) )
			recd.u.time_of_fix1 = 0.0;
	} else	{
		if ( !get(recd.u.time_of_fix2) )
			recd.u.time_of_fix2 = 0.0;
	}
	return true;
}

bool
RxPacket::get(s_R4B& recd) {

	if ( !get(recd.machine_id) )
		return false;
	if ( !get(recd.u1.raw_status1) )
		recd.u1.raw_status1 = 0;
	if ( !get(recd.status2) )
		recd.status2 = 0;
	return true;
}

bool
RxPacket::get(s_R4C& recd) {

	if ( !get(recd.dynamics_code) )
		return false;
	if ( !get(recd.elevation_mask) )
		return false;
	if ( !get(recd.signal_level_mask) )
		return false;
	if ( !get(recd.pdop_mask) )
		return false;		
	if ( !get(recd.podp_switch) )
		return false;
	return 0;
}

bool
RxPacket::get(s_R56& recd) {

	if ( !get(recd.eastvel)
	  || !get(recd.northvel)
	  || !get(recd.upvel)
	  || !get(recd.clock_bias_rate) )
	  	return false;

	if ( !state_sd )
		return get(recd.u.time_of_fix1);
	return get(recd.u.time_of_fix2);
}

bool
RxPacket::get(s_R57& recd) {
	return !get(recd.info_src)
	  || !get(recd.track_mode)
	  || !get(recd.fix_time)
	  || !get(recd.fix_week);
}

bool
RxPacket::get(s_R59& recd) {
	uint8_t n;

	if ( !get(recd.operation) )
		return false;

	n = get(recd.sv_flags,sizeof recd.sv_flags);
	for ( ; n < sizeof recd.sv_flags; ++n )
		recd.sv_flags[n] = 0;
	return true;
}

bool
RxPacket::get(s_R5A& recd) {

	if ( !get(recd.sv_prn) )
		return false;
	if ( !get(recd.samplength) )
		return false;
	if ( !get(recd.siglevel) )
		return false;
	if ( !get(recd.code_phase) )
		return false;
	if ( !get(recd.doppler) )
		return false;
	if ( !get(recd.time) )
		return false;
	return true;
}

bool
RxPacket::get(s_R5B& recd) {
	if ( !get(recd.sv_prn) )
		return false;
	if ( !get(recd.coltime) )
		return false;
	if ( !get(recd.health) )
		return false;
	if ( !get(recd.iode) )
		return false;
	if ( !get(recd.t_oe) )
		recd.t_oe = 0.0;
	if ( !get(recd.fit_ival_flag) )
		recd.fit_ival_flag = 0;
	if ( !get(recd.ura) )
		recd.ura = 0.0;
	return true;
}

bool
RxPacket::get(s_R6D& recd) {
	if ( !get(recd.fixmode) )
		return false;
	if ( !get(recd.pdop) )
		return false;
	if ( !get(recd.hdop) )
		return false;
	if ( !get(recd.vdop) )
		return false;
	if ( !get(recd.tdop) )
		return false;

	memset(recd.sv_prn,0,sizeof recd.sv_prn);
	recd.n = get(recd.sv_prn,sizeof recd.sv_prn);
	return true;
}

bool
RxPacket::get(s_R82& recd) {
	uint8_t b;

	if ( !get(b) )
		return false;
	recd.mode = Mode82(b);

	if ( get(b) )
		recd.rtcm_vers = RtcmVers(b);
	else	recd.rtcm_vers = RtcmVers(0);

	if ( !get(recd.refstatn) )
		recd.refstatn = 0;

	return true;
}

bool
RxPacket::get(s_R47& recd) {

	if ( !get(recd.count) )
		return false;

	for ( uint16_t x=0; x<recd.count && x < 12; ++x ) {
		if ( !get(recd.sat[x].prn) || !get(recd.sat[x].siglevel) ) {
			recd.count = x;
			return false;
		}
	}
	return true;
}

bool
RxPacket::get(s_R4D& recd) {
	return get(recd.offset);
}

bool
RxPacket::get(s_R4E& recd) {
	return get(recd.yn);
}


bool
RxPacket::get(s_R4F& recd) {

	if ( !get(recd.a0) )
		return false;
	if ( !get(recd.a1) )
		return false;
	if ( !get(recd.delta_t_ls) )
		return false;
	if ( !get(recd.tot) )
		return false;
	if ( !get(recd.wn_t) )
		return false;
	if ( !get(recd.wn_lsf) )
		return false;
	if ( !get(recd.dn) )
		return false;
	if ( !get(recd.delta_t_lsf) )
		return false;
	return true;
}

bool
RxPacket::get(s_R54& recd) {

	if ( !get(recd.bias) )
		return false;
	if ( !get(recd.bias_rate) )
		return false;
	if ( state_sd ) {
		if ( !get(recd.u.time_of_fix1) )
			return false;
	} else	{
		if ( !get(recd.u.time_of_fix2) )
			return false;
	}
	return true;
}

bool
RxPacket::get(s_R55& recd) {

	if ( !get(recd.position) )
		return false;
	if ( !get(recd.velocity) )
		return false;
	if ( !get(recd.timing) )
		return false;
	if ( !get(recd.auxiliary) )
		return false;
	return true;
}

bool
RxPacket::get(s_R58& recd) {
	uint8_t typ;

	if ( !get(recd.operation) )
		return false;
	if ( !get(typ) )
		return false;
	recd.datatype = s_R58::Type(typ);
	if ( !get(recd.sv_prn) )
		return false;
	if ( !get(recd.n) )
		return false;

	switch ( recd.datatype ) {
	case s_R58::NotUsed :
		break;
	case s_R58::Almanac :
		for ( uint8_t x=0; x<recd.n; ++x ) {
			if ( !get(recd.u.s2.t_oa_raw) )
				return false;
			if ( !get(recd.u.s2.sv_health) )
				return false;
			if ( !get(recd.u.s2.e) )
				return false;
			if ( !get(recd.u.s2.t_oa) )
				return false;
			if ( !get(recd.u.s2.i_o) )
				return false;
			if ( !get(recd.u.s2.omegadot) )
				return false;
			if ( !get(recd.u.s2.sqrt_a) )
				return false;
			if ( !get(recd.u.s2.omega_0) )
				return false;
			if ( !get(recd.u.s2.omega) )
				return false;
			if ( !get(recd.u.s2.m_0) )
				return false;
			if ( !get(recd.u.s2.a_f0) )
				return false;
			if ( !get(recd.u.s2.a_f1) )
				return false;
			if ( !get(recd.u.s2.axis) )
				return false;
			if ( !get(recd.u.s2.n) )
				return false;
			if ( !get(recd.u.s2.omega_n) )
				return false;
			if ( !get(recd.u.s2.odot_n) )
				return false;
			if ( !get(recd.u.s2.t_zc) )
				return false;
			if ( !get(recd.u.s2.weeknum) )
				return false;
			if ( !get(recd.u.s2.wn_oa) )
				return false;
		}
		break;
	case s_R58::Health :
		for ( uint8_t x=0; x<recd.n; ++x ) {
			if ( !get(recd.u.s3.weekno) )
				return false;
			if ( get(recd.u.s3.sv_health,32) != 32 )
				return false;
			if ( !get(recd.u.s3.t_oa) )
				return false;
			if ( !get(recd.u.s3.cur_t_oa) )
				return false;
			if ( !get(recd.u.s3.cur_weekno) )
				return false;
		}
		break;
	case s_R58::Ionosphere :
		for ( uint8_t x=0; x<recd.n; ++x ) {
			if ( get(recd.u.s4.compressed,8) != 8 )
				return false;
			if ( !get(recd.u.s4.alpha_0) )
				return false;
			if ( !get(recd.u.s4.alpha_1) )
				return false;
			if ( !get(recd.u.s4.alpha_2) )
				return false;
			if ( !get(recd.u.s4.alpha_3) )
				return false;
			if ( !get(recd.u.s4.beta_0) )
				return false;
			if ( !get(recd.u.s4.beta_1) )
				return false;
			if ( !get(recd.u.s4.beta_2) )
				return false;
			if ( !get(recd.u.s4.beta_3) )
				return false;
		}
		break;
	case s_R58::UTC :
		for ( uint8_t x=0; x<recd.n; ++x ) {
			if ( get(recd.u.s5.compressed,13) != 13 )
				return false;
			if ( !get(recd.u.s5.a_0) )
				return false;
			if ( !get(recd.u.s5.a_1) )
				return false;
			if ( !get(recd.u.s5.delta_t_ls) )
				return false;
			if ( !get(recd.u.s5.t_ot) )
				return false;
			if ( !get(recd.u.s5.wn_t) )
				return false;
			if ( !get(recd.u.s5.wn_lsf) )
				return false;
			if ( !get(recd.u.s5.dn) )
				return false;
			if ( !get(recd.u.s5.delta_t_lsf) )
				return false;
		}
		break;
	case s_R58::Ephemeris :
		for ( uint8_t x=0; x<recd.n; ++x ) {
			if ( !get(recd.u.s6.sv_prn) )
				return false;
			if ( !get(recd.u.s6.t_ephem) )
				return false;
			if ( !get(recd.u.s6.weekno) )
				return false;
			if ( !get(recd.u.s6.codel2) )
				return false;
			if ( !get(recd.u.s6.l2pdata) )
				return false;
			if ( !get(recd.u.s6.svacc_raw) )
				return false;
			if ( !get(recd.u.s6.sv_health) )
				return false;
			if ( !get(recd.u.s6.iodc) )
				return false;
			if ( !get(recd.u.s6.t_gd) )
				return false;
			if ( !get(recd.u.s6.t_oc) )
				return false;
			if ( !get(recd.u.s6.a_f2) )
				return false;
			if ( !get(recd.u.s6.a_f1) )
				return false;
			if ( !get(recd.u.s6.a_f0) )
				return false;
			if ( !get(recd.u.s6.svacc) )
				return false;
			if ( !get(recd.u.s6.iode) )
				return false;
			if ( !get(recd.u.s6.fit_ival) )
				return false;
			if ( !get(recd.u.s6.c_rs) )
				return false;
			if ( !get(recd.u.s6.delta_n) )
				return false;
			if ( !get(recd.u.s6.m_0) )
				return false;
			if ( !get(recd.u.s6.c_uc) )
				return false;
			if ( !get(recd.u.s6.e) )
				return false;
			if ( !get(recd.u.s6.c_us) )
				return false;
			if ( !get(recd.u.s6.sqrt_a) )
				return false;
			if ( !get(recd.u.s6.t_oe) )
				return false;
			if ( !get(recd.u.s6.c_ic) )
				return false;
			if ( !get(recd.u.s6.omega_0) )
				return false;
			if ( !get(recd.u.s6.c_is) )
				return false;
			if ( !get(recd.u.s6.i_o) )
				return false;
			if ( !get(recd.u.s6.c_rc) )
				return false;
			if ( !get(recd.u.s6.omega) )
				return false;
			if ( !get(recd.u.s6.omegadot) )
				return false;
			if ( !get(recd.u.s6.idot) )
				return false;
			if ( !get(recd.u.s6.axis) )
				return false;
			if ( !get(recd.u.s6.n) )
				return false;
			if ( !get(recd.u.s6.r1me2) )
				return false;
			if ( !get(recd.u.s6.omega_n) )
				return false;
			if ( !get(recd.u.s6.odot_n) )
				return false;
		}
		break;
	default :
		return false;
	}

	return true;
}

bool
RxPacket::get(s_R5C& recd) {

	if ( !get(recd.sv_prn) )
		return false;
	if ( !get(recd.chan_slot) )
		return false;
	if ( !get(recd.aquisflag) )
		return false;
	if ( !get(recd.ephemflag) )
		return false;
	if ( !get(recd.siglevel) )
		return false;
	if ( !get(recd.gps_time) )
		return false;
	if ( !get(recd.elevation) )
		return false;
	if ( !get(recd.azimuth) )
		return false;
	if ( !get(recd.oldmeas) )
		return false;
	if ( !get(recd.intmsec) )
		return false;
	if ( !get(recd.baddata) )
		return false;
	if ( !get(recd.datacol) )
		return false;
	return true;
}

bool
RxPacket::get(s_R5F11& recd) {
	return get(recd.status);
}

bool
RxPacket::get(s_R83& recd) {

	if ( !get(recd.x) )
		return false;
	if ( !get(recd.y) )
		return false;
	if ( !get(recd.z) )
		return false;
	if ( !get(recd.clock_bias) )
		return false;
	if ( state_sd ) {
		if ( !get(recd.u.time_of_fix1) )
			return false;
	} else	{
		if ( !get(recd.u.time_of_fix2) )
			return false;
	}
	return true;
}

bool
RxPacket::get(s_R84& recd) {

	if ( !get(recd.latitude) )
		return false;
	if ( !get(recd.longitude) )
		return false;
	if ( !get(recd.altitude) )
		return false;
	if ( !get(recd.clock_bias) )
		return false;
	if ( state_sd ) {
		if ( !get(recd.u.time_of_fix1) )
			return false;
	} else	{
		if ( !get(recd.u.time_of_fix2) )
			return false;
	}
	return false;
}

bool
RxPacket::get(s_R8F41& recd) {

	if ( !get(recd.serprefix) )
		return false;
	if ( !get(recd.serialno) )
		return false;
	if ( !get(recd.year) )
		return false;	
	if ( !get(recd.month) )
		return false;	
	if ( !get(recd.day) )
		return false;
	if ( !get(recd.hour) )
		return false;
	if ( !get(recd.oscoffset) )
		return false;
	if ( !get(recd.testcode) )
		return false;
	return true;
}

bool
RxPacket::get(s_R8F42& recd) {

	if ( !get(recd.optsprefix) )
		return false;
	if ( !get(recd.pnextension) )
		return false;
	if ( !get(recd.csnpref) )
		return false;
	if ( !get(recd.caseser) )
		return false;
	if ( !get(recd.prodno) )
		return false;
	if ( !get(recd.reserved1) )
		return false;
	if ( !get(recd.machid) )
		return false;
	if ( !get(recd.reserved2) )
		return false;
	return true;
}

bool
RxPacket::get(s_R8FA5& recd) {
	return     get(recd.u.flags)
		&& get(recd.mbz);
}

bool
RxPacket::get(s_R8FAB& recd) {

	if ( !get(recd.tow) )
		return false;
	if ( !get(recd.weekno) )
		return false;
	if ( !get(recd.utc_offset) )
		return false;
	if ( !get(recd.timing_flags.raw) )
		return false;
	if ( !get(recd.seconds) )
		return false;
	if ( !get(recd.minutes) )
		return false;
	if ( !get(recd.hours) )
		return false;
	if ( !get(recd.mday) )
		return false;
	if ( !get(recd.month) )
		return false;
	if ( !get(recd.year) )
		return false;
	return true;
}

bool
RxPacket::get(s_R1C81& recd) {

	if ( !get(recd.reserved1) )
		return false;
	if ( !get(recd.major_firm) )
		return false;
	if ( !get(recd.minor_firm) )
		return false;
	if ( !get(recd.build_no) )
		return false;
	if ( !get(recd.month) )
		return false;
	if ( !get(recd.day) )
		return false;
	if ( !get(recd.year) )
		return false;
	if ( !get(recd.length) )
		return false;

	uint16_t len = get(recd.prodname,sizeof recd.prodname);
	if ( len > sizeof recd.prodname )
		recd.length = sizeof recd.prodname;	
	else if ( len < sizeof recd.prodname )
		recd.length = len;	
	if ( recd.length < sizeof recd.prodname )
		recd.prodname[recd.length] = 0;
	return true;
}

bool
RxPacket::get(s_R1C83& recd) {

	if ( !get(recd.serialno) )
		return false;
	if ( !get(recd.day) )
		return false;
	if ( !get(recd.month) )
		return false;
	if ( !get(recd.year) )
		return false;
	if ( !get(recd.hour) )
		return false;
	if ( !get(recd.hardw_code) )
		return false;
	if ( !get(recd.length) )
		return false;

	uint16_t len = get(recd.hardw_id,sizeof recd.hardw_id);
	if ( len > sizeof recd.hardw_id )
		recd.length = sizeof recd.hardw_id;	
	else if ( len < sizeof recd.hardw_id )
		recd.length = len;	
	if ( recd.length < sizeof recd.hardw_id )
		recd.hardw_id[recd.length] = 0;
	return true;
}

bool
RxPacket::get(s_R45& recd) {
	if ( !get(recd.major) )
		return false;
	if ( !get(recd.minor) )
		return false;
	if ( !get(recd.month) )
		return false;
	if ( !get(recd.day) )
		return false;
	if ( !get(recd.year) )
		return false;
	if ( !get(recd.major2) )
		return false;
	if ( !get(recd.minor2) )
		return false;
	if ( !get(recd.month2) )
		return false;
	if ( !get(recd.day2) )
		return false;
	return get(recd.year2);
}

bool
RxPacket::get(s_RBB00& recd) {
	return     get(recd.opdim)
		&& get(recd.dgps_mode)
		&& get(recd.dyn_mode)
		&& get(recd.sol_mode)
		&& get(recd.elev_mask)
		&& get(recd.amu_mask)
		&& get(recd.pdop_mask)
		&& get(recd.pdop_switch)
		&& get(recd.dgps_age)
		&& get(recd.foliage_mode)
		&& get(recd.reserved1)
		&& get(recd.reserved2)
		&& get(recd.meas_rate)
		&& get(recd.posfx_rate);
}


//////////////////////////////////////////////////////////////////////
// Encoding a Packet
//////////////////////////////////////////////////////////////////////

TxPacket::TxPacket() {
	buf = 0;
	maxlen = buflen = 0;
}

void
TxPacket::open(uint8_t *buf,uint16_t maxlen) {
	this->buf = buf;
	this->maxlen = maxlen;
	buflen = 0;
}

bool
TxPacket::put_asis(uint8_t byte) {
	if ( buflen < maxlen ) {
		buf[buflen++] = byte;
		return true;
	}
	return false;
}

bool
TxPacket::put(uint8_t byte) {
	if ( buflen < maxlen ) {
		if ( byte != 0x10 ) {
			buf[buflen++] = byte;
		} else	{
			buf[buflen++] = 0x10;
			if ( buflen >= maxlen )
				return false;
			buf[buflen++] = 0x10;
		}
		return true;
	}
	return false;
}

bool
TxPacket::put(const uint8_t *buf,uint16_t len) {
	while ( len > 0 && put(*buf++) )
		--len;
	return len <= 0;
}

bool
TxPacket::put(int16_t ival) {
	uint8_t buf[2];

	buf[0] = ((uint16_t) ival) >> 8;
	buf[1] = ((uint16_t) ival) & 0xFF;
	return put(buf,sizeof buf);
}

bool
TxPacket::put(int32_t ival) {
	uint8_t buf[4];

	buf[0] = (((uint32_t) ival) >> 24) & 0xFF;
	buf[1] = (((uint32_t) ival) >> 16) & 0xFF;
	buf[2] = (((uint32_t) ival) >>  8) & 0xFF;
	buf[3] = ((uint32_t) ival) & 0xFF;
	return put(buf,sizeof buf);
}

bool
TxPacket::put(int64_t ival) {
	uint8_t buf[8];

	buf[0] = (((uint64_t) ival) >> 56) & 0xFF;
	buf[1] = (((uint64_t) ival) >> 48) & 0xFF;
	buf[2] = (((uint64_t) ival) >> 40) & 0xFF;
	buf[3] = (((uint64_t) ival) >> 32) & 0xFF;
	buf[4] = (((uint64_t) ival) >> 24) & 0xFF;
	buf[5] = (((uint64_t) ival) >> 16) & 0xFF;
	buf[6] = (((uint64_t) ival) >>  8) & 0xFF;
	buf[7] = ((uint64_t) ival) & 0xFF;
	return put(buf,sizeof buf);
}

bool
TxPacket::put(float fval) {
	union	{
		int32_t	i32;
		float	f32;
	} u;

	u.f32 = fval;
	return put(u.i32);
}

bool
TxPacket::put(double fval) {
	union	{
		int64_t	i64;
		double	f64;
	} u;
	u.f64 = fval;
	return put(u.i64);
}

bool
TxPacket::command(uint16_t id) {

	put_asis(uint8_t(0x10));
	if ( !(id & 0xFF00) )
		return put(uint8_t(id));	// Normal command ID

	uint8_t b, b2;
	b = id >> 8;		// Command ID
	b2 = id & 0xFF;		// Sub ID
	return put(b) && put(b2);
}

bool
TxPacket::close() {
	return put_asis(0x10) && put_asis(0x03);
}

//////////////////////////////////////////////////////////////////////
// 1C01 -- Request Software Version Information
// Responses:
//	R1C81
//////////////////////////////////////////////////////////////////////

bool
TxPacket::C1C01() {
	return command(0x1C01) && close();
}

//////////////////////////////////////////////////////////////////////
// 1C03 -- Hardware Version Information
// Responses:
//	R1C83
//////////////////////////////////////////////////////////////////////

bool
TxPacket::C1C03() {
	return command(0x1C03) && close();
}

//////////////////////////////////////////////////////////////////////
// 1E	-- Initiate Cold or Factory Reset
//
// Arguments:
//	type:	'K' -	Cold reset
//		'F' -	Factory reset
//		'C' - 	Compatibility reset
//		'N' -	Clear navigation EEPROM
//		'R' -	Set EEPROM to factory config and reset
//////////////////////////////////////////////////////////////////////

bool
TxPacket::C1E(char type) {
	return command(0x1E) && put(uint8_t(type)) && close();
}

//////////////////////////////////////////////////////////////////////
// 1F	-- Software Versions Request
// Response:
//	R45 
//////////////////////////////////////////////////////////////////////

bool
TxPacket::C1F() {
	return command(0x1F) && close();
}

//////////////////////////////////////////////////////////////////////
// 20	-- Almanac Request
// Response:
//	R40
//////////////////////////////////////////////////////////////////////

bool
TxPacket::C20() {
	return command(0x20) && close();
}

//////////////////////////////////////////////////////////////////////
// 21	-- Current Time Request
// Response:
//	R41
//////////////////////////////////////////////////////////////////////

bool
TxPacket::C21() {
	return command(0x21) && close();
}

//////////////////////////////////////////////////////////////////////
// 23	-- Initial Position (XYZ Cartesian ECEF) Command
//////////////////////////////////////////////////////////////////////

bool
TxPacket::C23(float x,float y,float z) {
	return command(0x21)
		&& put(x)
		&& put(y)
		&& put(z)
		&& close();
}

//////////////////////////////////////////////////////////////////////
// 24	-- GPS Receiver Position Fix Mode Request
// Response:
//	R6D
//////////////////////////////////////////////////////////////////////

bool
TxPacket::C24() {
	return command(0x24) && close();
}

//////////////////////////////////////////////////////////////////////
// 25	-- Initiate Soft Reset / Self Test Command
// Response:
//	R45
//////////////////////////////////////////////////////////////////////

bool
TxPacket::C25() {
	return command(0x25) && close();
}

//////////////////////////////////////////////////////////////////////
// 26	-- Health Request
// Responses:
//	R46, R4B
//////////////////////////////////////////////////////////////////////

bool
TxPacket::C26() {
	return command(0x26) && close();
}

//////////////////////////////////////////////////////////////////////
// 27	-- Signal Levels Request
// Response:
//	R47
//////////////////////////////////////////////////////////////////////

bool
TxPacket::C27() {
	return command(0x27) && close();
}

//////////////////////////////////////////////////////////////////////
// 28	-- GPS System Message Request
// Response:
//	R48
//////////////////////////////////////////////////////////////////////

bool
TxPacket::C28() {
	return command(0x28) && close();
}

//////////////////////////////////////////////////////////////////////
// 29	-- Almanac Health Page Request
// Response:
//	R49
//////////////////////////////////////////////////////////////////////

bool
TxPacket::C29() {
	return command(0x29) && close();
}

//////////////////////////////////////////////////////////////////////
// 2A	-- Altitude for 2-D Mode Command
// Arguments:
//	alt_meters	altitude in meters, using the WGS-84 model
//			of the earth or MSL geoid altitude
//////////////////////////////////////////////////////////////////////

bool
TxPacket::C2A(int16_t alt_meters) {
	return command(0x2A)
		&& put(alt_meters)
		&& close();
}

//////////////////////////////////////////////////////////////////////
// 2A	-- Altitude for 2-D Mode Command
// Use the altitude of the previous 3-D fix (altitude-hold mode)
//////////////////////////////////////////////////////////////////////

bool
TxPacket::C2A() {
	return command(0x2A) && close();
}

//////////////////////////////////////////////////////////////////////
// 2A	-- Altitude for 2-D Mode Command
//
// Sending packet 0x2A with one data byte equal to 0xFF will
// cancel altitude-set mode and return the reference
// altitude to 0. The altitude setting is stored in
// non-volatile memory.
//////////////////////////////////////////////////////////////////////

bool
TxPacket::C2A_cancel() {
	return command(0x2A)
		&& put(uint8_t(0xFF))
		&& close();
}

//////////////////////////////////////////////////////////////////////
// 2B	-- Initial Position (Latitude, Longitude, Altitude) Command
//////////////////////////////////////////////////////////////////////

bool
TxPacket::C2B(float latitude,float longitude,float altitude) {
	return command(0x2B)
		&& put(latitude)
		&& put(longitude)
		&& put(altitude)
		&& close();
}

//////////////////////////////////////////////////////////////////////
// 2D	-- Oscillator Offset Request
// Response:
//	4D
//////////////////////////////////////////////////////////////////////

bool
TxPacket::C2D() {
	return command(0x2D) && close();
}

//////////////////////////////////////////////////////////////////////
// 2E	-- GPS Time Command
// Response:
//	4E
//////////////////////////////////////////////////////////////////////

bool
TxPacket::C2E(float gps_time,int16_t weekno) {
	return command(0x2E)
		&& put(gps_time)
		&& put(weekno)
		&& close();
}

//////////////////////////////////////////////////////////////////////
// 2F	-- UTC Parameters Request
// Response:
//	4F
//////////////////////////////////////////////////////////////////////

bool
TxPacket::C2F() {
	return command(0x2F) && close();
}

//////////////////////////////////////////////////////////////////////
// 31	-- Accurate Initial Position (XYZ Cartesian ECEF) Command
//////////////////////////////////////////////////////////////////////

bool
TxPacket::C31(float x,float y,float z) {
	return command(0x31)
		&& put(x)
		&& put(y)
		&& put(z)
		&& close();
}

//////////////////////////////////////////////////////////////////////
// 32	-- Accurate Initial Position (Latitude, Longitude, Altitude) Command
//////////////////////////////////////////////////////////////////////

bool
TxPacket::C32(float latitude,float longitude,float altitude) {
	return command(0x32)
		&& put(latitude)
		&& put(longitude)
		&& put(altitude)
		&& close();
}

//////////////////////////////////////////////////////////////////////
// 34	-- Satellite Number For One-Satellite Mode Command
// Argument:
//	prn	-- 0=auto select
//		   1-32 choosen satellite to use
//////////////////////////////////////////////////////////////////////

bool
TxPacket::C34(uint8_t prn) {
	return command(0x34)
		&& put(prn)
		&& close();
}

//////////////////////////////////////////////////////////////////////
// 35	-- I/O Option Flags Command
// Arguments:
//////////////////////////////////////////////////////////////////////

bool
TxPacket::C35(
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
  bool aux_db_hz) {		// 1=Output db/Hz (0x47) vs AMU (0x5A/5C)
	union {
		uint8_t	raw;
		struct {
			uint8_t	pos_xyz_ecef_on : 1;
			uint8_t	lla_on          : 1;
			uint8_t	lla_alt         : 1;
			uint8_t	alt_input       : 1;
			uint8_t	pos_precision   : 1;
			uint8_t	r8F20_on        : 1;
			uint8_t	unused          : 2;
		} bits;
	} b0;
	union {
		uint8_t	raw;
		struct {
			uint8_t	vel_xyz_ecef_on : 1;
			uint8_t	vel_enu         : 1;
			uint8_t	unused          : 6;
		} bits;
	} b1;
	union {
		uint8_t	raw;
		struct {
			uint8_t	timing_utc      : 1;
			uint8_t	timing_comp     : 1;
			uint8_t	timing_fix      : 1;
			uint8_t	timing_sync     : 1;
			uint8_t	timing_minproj  : 1;
			uint8_t	unused          : 3;
		} bits;
	} b2;
	union {
		uint8_t	raw;
		struct {
			uint8_t	aux_raw         : 1;
			uint8_t	aux_smoothed    : 1;
			uint8_t unused1         : 1;
			uint8_t	aux_db_hz       : 1;
			uint8_t	unused          : 4;
		} bits;
	} b3;

	b0.raw 			= 0;
	b0.bits.pos_xyz_ecef_on	= pos_xyz_ecef_on;
	b0.bits.lla_on		= lla_on;
	b0.bits.lla_alt		= lla_alt;
	b0.bits.alt_input	= alt_input;
	b0.bits.pos_precision	= pos_precision;
	b0.bits.r8F20_on	= r8F20_on;

	b1.raw 			= 0;
	b1.bits.vel_xyz_ecef_on	= vel_xyz_ecef_on;
	b1.bits.vel_enu		= vel_enu;

	b2.raw			= 0;
	b2.bits.timing_utc	= timing_utc;
	b2.bits.timing_comp	= timing_comp;
	b2.bits.timing_fix	= timing_fix;
	b2.bits.timing_sync	= timing_sync;
	b2.bits.timing_minproj	= timing_minproj;

	b3.raw 			= 0;
	b3.bits.aux_raw		= aux_raw;
	b3.bits.aux_smoothed	= aux_smoothed;
	b3.bits.aux_db_hz	= aux_db_hz;

	return command(0x35)
		&& put(b0.raw)
		&& put(b1.raw)
		&& put(b2.raw)
		&& put(b3.raw)
		&& close();
}

//////////////////////////////////////////////////////////////////////
// 37	-- Last Position and Velocity Request
// Responses:
//	R57, R42/4A, R83/84, R43/56, R57, R54
//////////////////////////////////////////////////////////////////////

bool
TxPacket::C37() {
	return command(0x37) && close();
}

//////////////////////////////////////////////////////////////////////
// 3A	-- Last Raw Measurement Request
// Response:
//	R5A
//////////////////////////////////////////////////////////////////////

bool
TxPacket::C3A(uint8_t prn) {
	return command(0x3A)
		&& put(prn)
		&& close();
}

//////////////////////////////////////////////////////////////////////
// 3B	-- Satellite Ephemeris Status Request
// Response:
//	R5B
//////////////////////////////////////////////////////////////////////

bool
TxPacket::C3B(uint8_t prn) {
	return command(0x3B)
		&& put(prn)
		&& close();
}

//////////////////////////////////////////////////////////////////////
// 3C	-- Satellite Tracking Status Request
// Response:
//	R5C
//////////////////////////////////////////////////////////////////////

bool
TxPacket::C3C(uint8_t prn) {
	return command(0x3C)
		&& put(prn)
		&& close();
}

//////////////////////////////////////////////////////////////////////
// 3F11	-- EEPROM Segment Commands
// Response:
//	R5F11, 8FAC
//////////////////////////////////////////////////////////////////////

bool
TxPacket::C3F11() {
	return command(0x3F)
		&& put(0x11)
		&& close();
}

//////////////////////////////////////////////////////////////////////
// BB00	-- Set Primary Receiver Configuration
// Response:
//	RBB00
//////////////////////////////////////////////////////////////////////

bool
TxPacket::CBB00(s_RBB00& parms) {
	parms.reserved1 = 0;
	parms.reserved2 = 0;
	parms.reserved3 = 0;

	return command(0xBB00)
		&& put(parms.opdim)
		&& put(parms.dgps_mode)
		&& put(parms.dyn_mode)
		&& put(parms.sol_mode)
		&& put(parms.elev_mask)
		&& put(parms.amu_mask)
		&& put(parms.pdop_mask)
		&& put(parms.pdop_switch)
		&& put(parms.dgps_age)
		&& put(parms.foliage_mode)
		&& put(parms.reserved1)
		&& put(parms.reserved2)
		&& put(parms.meas_rate)
		&& put(parms.posfx_rate)
		&& put(parms.reserved3)
		&& close();
}

//////////////////////////////////////////////////////////////////////
// 8EA5	-- Set Packet Broadcast Mask
// Response:
//	R8FA5
//////////////////////////////////////////////////////////////////////

bool
TxPacket::C8EA5(s_R8FA5& parms) {

	parms.mbz = 0;
	return command(0x8EA5)
		&& put(parms.u.flags)
		&& put(parms.mbz)
		&& close();
}

// End tsip.cpp
