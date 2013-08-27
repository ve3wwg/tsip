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
RxPacket::get(s_R57& recd) {

	if ( !get(recd.info_src) )
		return false;
	if ( !get(recd.diag_code) )
		return false;
	if ( !get(recd.fix_time) )
		return false;
	if ( !get(recd.fix_week) )
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



#if 0


int16_t
decode_R13(s_inpkt *in,s_R13 *recd) {
	rc = inp_getb(in,&recd->packet_id,1);
	if ( rc < 0 ) return rc;
	rc = inp_getb(in,&recd->contents,255);
	if ( rc < 0 ) return rc;
	return 0;
}

int16_t
decode_R1A00(s_inpkt *in,s_R1A00 *recd) {
	rc = inp_getb(in,&recd->data,255);
	if ( rc < 0 ) return rc;
	return 0;
}

int16_t
decode_R3D(s_inpkt *in,s_R3D *recd) {
	rc = inp_getb(in,&recd->output_baud_rate,1);
	if ( rc < 0 ) return rc;
	rc = inp_getb(in,&recd->input_baud_rate,1);
	if ( rc < 0 ) return rc;
	rc = inp_getb(in,&recd->parity_bits,1);
	if ( rc < 0 ) return rc;
	rc = inp_getb(in,&recd->stop_flow,1);
	if ( rc < 0 ) return rc;
	rc = inp_getb(in,&recd->out_protocol,1);
	if ( rc < 0 ) return rc;
	rc = inp_getb(in,&recd->in_protocol,1);
	if ( rc < 0 ) return rc;
	return 0;
}

int16_t
decode_R44(s_inpkt *in,s_R44 *recd) {
	rc = inp_getb(in,&recd->flag,1);
	if ( rc < 0 ) return rc;
	rc = inp_getb(in,&recd->satellite1,1);
	if ( rc < 0 ) return rc;
	rc = inp_getb(in,&recd->satellite2,1);
	if ( rc < 0 ) return rc;
	rc = inp_getb(in,&recd->satellite3,1);
	if ( rc < 0 ) return rc;
	rc = inp_getb(in,&recd->satellite4,1);
	if ( rc < 0 ) return rc;
	rc = inp_getf32(in,&recd->pdop);
	if ( rc < 0 ) return rc;
	rc = inp_getf32(in,&recd->hdop);
	if ( rc < 0 ) return rc;
	rc = inp_getf32(in,&recd->vdop);
	if ( rc < 0 ) return rc;
	rc = inp_getf32(in,&recd->tdop);
	if ( rc < 0 ) return rc;
	return 0;
}

int16_t
decode_R5E(s_inpkt *in,s_R5E *recd) {
	rc = inp_getb(in,&recd->prevmeas,1);
	if ( rc < 0 ) return rc;
	rc = inp_getb(in,&recd->oldmeas,1);
	if ( rc < 0 ) return rc;
	return 0;
}

int16_t
decode_R5F(s_inpkt *in,s_R5F *recd) {
	rc = inp_getb(in,&recd->hex0x02,1);
	if ( rc < 0 ) return rc;
	rc = inp_getb(in,&recd->message,255);
	if ( rc < 0 ) return rc;
	return 0;
}

int16_t
decode_R60(s_inpkt *in,s_R60 *recd) {
	rc = inp_geti16(in,&recd->zcount);
	if ( rc < 0 ) return rc;
	rc = inp_getb(in,&recd->station_health,1);
	if ( rc < 0 ) return rc;
	rc = inp_getb(in,&recd->verstype,1);
	if ( rc < 0 ) return rc;
	rc = inp_getb(in,&recd->scale1,1);
	if ( rc < 0 ) return rc;
	rc = inp_geti16(in,&recd->prc1);
	if ( rc < 0 ) return rc;
	rc = inp_getb(in,&recd->rrc1,1);
	if ( rc < 0 ) return rc;
	rc = inp_getb(in,&recd->iode1,1);
	if ( rc < 0 ) return rc;
	rc = inp_getb(in,&recd->scale2,1);
	if ( rc < 0 ) return rc;
	rc = inp_geti16(in,&recd->prc2);
	if ( rc < 0 ) return rc;
	rc = inp_getb(in,&recd->rrc2,1);
	if ( rc < 0 ) return rc;
	rc = inp_getb(in,&recd->iode2,1);
	if ( rc < 0 ) return rc;
	rc = inp_getb(in,&recd->scale3,1);
	if ( rc < 0 ) return rc;
	rc = inp_geti16(in,&recd->prc3);
	if ( rc < 0 ) return rc;
	rc = inp_getb(in,&recd->rrc3,1);
	if ( rc < 0 ) return rc;
	rc = inp_getb(in,&recd->iode3,1);
	if ( rc < 0 ) return rc;
	rc = inp_getb(in,&recd->scale4,1);
	if ( rc < 0 ) return rc;
	rc = inp_geti16(in,&recd->prc4);
	if ( rc < 0 ) return rc;
	rc = inp_getb(in,&recd->rrc4,1);
	if ( rc < 0 ) return rc;
	rc = inp_getb(in,&recd->iode4,1);
	if ( rc < 0 ) return rc;
	rc = inp_getb(in,&recd->scale5,1);
	if ( rc < 0 ) return rc;
	rc = inp_geti16(in,&recd->prc5);
	if ( rc < 0 ) return rc;
	rc = inp_getb(in,&recd->rrc5,1);
	if ( rc < 0 ) return rc;
	rc = inp_getb(in,&recd->iode5,1);
	if ( rc < 0 ) return rc;
	rc = inp_getb(in,&recd->scale6,1);
	if ( rc < 0 ) return rc;
	rc = inp_geti16(in,&recd->prc6);
	if ( rc < 0 ) return rc;
	rc = inp_getb(in,&recd->rrc6,1);
	if ( rc < 0 ) return rc;
	rc = inp_getb(in,&recd->iode6,1);
	if ( rc < 0 ) return rc;
	rc = inp_getb(in,&recd->scale7,1);
	if ( rc < 0 ) return rc;
	rc = inp_geti16(in,&recd->prc7);
	if ( rc < 0 ) return rc;
	rc = inp_getb(in,&recd->rrc7,1);
	if ( rc < 0 ) return rc;
	rc = inp_getb(in,&recd->iode7,1);
	if ( rc < 0 ) return rc;
	rc = inp_getb(in,&recd->scale8,1);
	if ( rc < 0 ) return rc;
	rc = inp_geti16(in,&recd->prc8);
	if ( rc < 0 ) return rc;
	rc = inp_getb(in,&recd->rrc8,1);
	if ( rc < 0 ) return rc;
	rc = inp_getb(in,&recd->iode8,1);
	if ( rc < 0 ) return rc;
	rc = inp_getb(in,&recd->scale9,1);
	if ( rc < 0 ) return rc;
	rc = inp_geti16(in,&recd->prc9);
	if ( rc < 0 ) return rc;
	rc = inp_getb(in,&recd->rrc9,1);
	if ( rc < 0 ) return rc;
	rc = inp_getb(in,&recd->iode9,1);
	if ( rc < 0 ) return rc;
	rc = inp_getb(in,&recd->scale10,1);
	if ( rc < 0 ) return rc;
	rc = inp_geti16(in,&recd->prc10);
	if ( rc < 0 ) return rc;
	rc = inp_getb(in,&recd->rrc10,1);
	if ( rc < 0 ) return rc;
	rc = inp_getb(in,&recd->iode10,1);
	if ( rc < 0 ) return rc;
	rc = inp_getb(in,&recd->scale11,1);
	if ( rc < 0 ) return rc;
	rc = inp_geti16(in,&recd->prc11);
	if ( rc < 0 ) return rc;
	rc = inp_getb(in,&recd->rrc11,1);
	if ( rc < 0 ) return rc;
	rc = inp_getb(in,&recd->iode11,1);
	if ( rc < 0 ) return rc;
	rc = inp_getb(in,&recd->scale12,1);
	if ( rc < 0 ) return rc;
	rc = inp_geti16(in,&recd->prc12);
	if ( rc < 0 ) return rc;
	rc = inp_getb(in,&recd->rrc12,1);
	if ( rc < 0 ) return rc;
	rc = inp_getb(in,&recd->iode12,1);
	if ( rc < 0 ) return rc;
	return 0;
}

int16_t
decode_R61(s_inpkt *in,s_R61 *recd) {
	rc = inp_geti16(in,&recd->zcount);
	if ( rc < 0 ) return rc;
	rc = inp_getb(in,&recd->vers,1);
	if ( rc < 0 ) return rc;
	rc = inp_getb(in,&recd->scale1,1);
	if ( rc < 0 ) return rc;
	rc = inp_geti16(in,&recd->dprc1);
	if ( rc < 0 ) return rc;
	rc = inp_getb(in,&recd->scale2,1);
	if ( rc < 0 ) return rc;
	rc = inp_geti16(in,&recd->dprc2);
	if ( rc < 0 ) return rc;
	rc = inp_getb(in,&recd->scale3,1);
	if ( rc < 0 ) return rc;
	rc = inp_geti16(in,&recd->dprc3);
	if ( rc < 0 ) return rc;
	rc = inp_getb(in,&recd->scale4,1);
	if ( rc < 0 ) return rc;
	rc = inp_geti16(in,&recd->dprc4);
	if ( rc < 0 ) return rc;
	rc = inp_getb(in,&recd->scale5,1);
	if ( rc < 0 ) return rc;
	rc = inp_geti16(in,&recd->dprc5);
	if ( rc < 0 ) return rc;
	rc = inp_getb(in,&recd->scale6,1);
	if ( rc < 0 ) return rc;
	rc = inp_geti16(in,&recd->dprc6);
	if ( rc < 0 ) return rc;
	rc = inp_getb(in,&recd->scale7,1);
	if ( rc < 0 ) return rc;
	rc = inp_geti16(in,&recd->dprc7);
	if ( rc < 0 ) return rc;
	rc = inp_getb(in,&recd->scale8,1);
	if ( rc < 0 ) return rc;
	rc = inp_geti16(in,&recd->dprc8);
	if ( rc < 0 ) return rc;
	rc = inp_getb(in,&recd->scale9,1);
	if ( rc < 0 ) return rc;
	rc = inp_geti16(in,&recd->dprc9);
	if ( rc < 0 ) return rc;
	rc = inp_getb(in,&recd->scale10,1);
	if ( rc < 0 ) return rc;
	rc = inp_geti16(in,&recd->dprc10);
	if ( rc < 0 ) return rc;
	rc = inp_getb(in,&recd->scale11,1);
	if ( rc < 0 ) return rc;
	rc = inp_geti16(in,&recd->dprc11);
	if ( rc < 0 ) return rc;
	rc = inp_getb(in,&recd->scale12,1);
	if ( rc < 0 ) return rc;
	rc = inp_geti16(in,&recd->dprc12);
	if ( rc < 0 ) return rc;
	return 0;
}

int16_t
decode_R6A00(s_inpkt *in,s_R6A00 *recd) {
	rc = inp_geti32(in,&recd->fixtimetag);
	if ( rc < 0 ) return rc;
	rc = inp_getb(in,&recd->sats,1);
	if ( rc < 0 ) return rc;
	rc = inp_getb(in,&recd->satid1,1);
	if ( rc < 0 ) return rc;
	rc = inp_getf32(in,&recd->rc1);
	if ( rc < 0 ) return rc;
	rc = inp_getf32(in,&recd->rrc1);
	if ( rc < 0 ) return rc;
	rc = inp_getb(in,&recd->satid2,1);
	if ( rc < 0 ) return rc;
	rc = inp_getf32(in,&recd->rc2);
	if ( rc < 0 ) return rc;
	rc = inp_getf32(in,&recd->rrc2);
	if ( rc < 0 ) return rc;
	rc = inp_getb(in,&recd->satid3,1);
	if ( rc < 0 ) return rc;
	rc = inp_getf32(in,&recd->rc3);
	if ( rc < 0 ) return rc;
	rc = inp_getf32(in,&recd->rrc3);
	if ( rc < 0 ) return rc;
	rc = inp_getb(in,&recd->satid4,1);
	if ( rc < 0 ) return rc;
	rc = inp_getf32(in,&recd->rc4);
	if ( rc < 0 ) return rc;
	rc = inp_getf32(in,&recd->rrc4);
	if ( rc < 0 ) return rc;
	rc = inp_getb(in,&recd->satid5,1);
	if ( rc < 0 ) return rc;
	rc = inp_getf32(in,&recd->rc5);
	if ( rc < 0 ) return rc;
	rc = inp_getf32(in,&recd->rrc5);
	if ( rc < 0 ) return rc;
	rc = inp_getb(in,&recd->satid6,1);
	if ( rc < 0 ) return rc;
	rc = inp_getf32(in,&recd->rc6);
	if ( rc < 0 ) return rc;
	rc = inp_getf32(in,&recd->rrc6);
	if ( rc < 0 ) return rc;
	rc = inp_getb(in,&recd->satid7,1);
	if ( rc < 0 ) return rc;
	rc = inp_getf32(in,&recd->rc7);
	if ( rc < 0 ) return rc;
	rc = inp_getf32(in,&recd->rrc7);
	if ( rc < 0 ) return rc;
	rc = inp_getb(in,&recd->satid8,1);
	if ( rc < 0 ) return rc;
	rc = inp_getf32(in,&recd->rc8);
	if ( rc < 0 ) return rc;
	rc = inp_getf32(in,&recd->rrc8);
	if ( rc < 0 ) return rc;
	rc = inp_getb(in,&recd->satid9,1);
	if ( rc < 0 ) return rc;
	rc = inp_getf32(in,&recd->rc19);
	if ( rc < 0 ) return rc;
	rc = inp_getf32(in,&recd->rrc9);
	if ( rc < 0 ) return rc;
	rc = inp_getb(in,&recd->satid10,1);
	if ( rc < 0 ) return rc;
	rc = inp_getf32(in,&recd->rc110);
	if ( rc < 0 ) return rc;
	rc = inp_getf32(in,&recd->rrc10);
	if ( rc < 0 ) return rc;
	rc = inp_getb(in,&recd->satid11,1);
	if ( rc < 0 ) return rc;
	rc = inp_getf32(in,&recd->rc111);
	if ( rc < 0 ) return rc;
	rc = inp_getf32(in,&recd->rrc11);
	if ( rc < 0 ) return rc;
	rc = inp_getb(in,&recd->satid12,1);
	if ( rc < 0 ) return rc;
	rc = inp_getf32(in,&recd->rc112);
	if ( rc < 0 ) return rc;
	rc = inp_getf32(in,&recd->rrc12);
	if ( rc < 0 ) return rc;
	return 0;
}

int16_t
decode_R6A01(s_inpkt *in,s_R6A01 *recd) {
	rc = inp_getb(in,&recd->outen,1);
	if ( rc < 0 ) return rc;
	rc = inp_getb(in,&recd->reserved1,1);
	if ( rc < 0 ) return rc;
	rc = inp_getb(in,&recd->reserved2,1);
	if ( rc < 0 ) return rc;
	return 0;
}

int16_t
decode_R6E01(s_inpkt *in,s_R6E01 *recd) {
	rc = inp_getb(in,&recd->enable,1);
	if ( rc < 0 ) return rc;
	rc = inp_getb(in,&recd->outival,1);
	if ( rc < 0 ) return rc;
	return 0;
}

int16_t
decode_R6F01(s_inpkt *in,s_R6F01 *recd) {
	rc = inp_getb(in,&recd->preamble,1);
	if ( rc < 0 ) return rc;
	rc = inp_geti16(in,&recd->length);
	if ( rc < 0 ) return rc;
	rc = inp_getf64(in,&recd->recvtime);
	if ( rc < 0 ) return rc;
	rc = inp_getf64(in,&recd->clkoffset);
	if ( rc < 0 ) return rc;
	rc = inp_getb(in,&recd->nsats,1);
	if ( rc < 0 ) return rc;
	rc = inp_getb(in,&recd->sv_prn1,1);
	if ( rc < 0 ) return rc;
	rc = inp_getb(in,&recd->flagsa1,1);
	if ( rc < 0 ) return rc;
	rc = inp_getb(in,&recd->flagsb1,1);
	if ( rc < 0 ) return rc;
	rc = inp_getb(in,&recd->elevation_angle1,1);
	if ( rc < 0 ) return rc;
	rc = inp_geti16(in,&recd->azimuth1);
	if ( rc < 0 ) return rc;
	rc = inp_getb(in,&recd->snrx241,1);
	if ( rc < 0 ) return rc;
	rc = inp_getf64(in,&recd->pseudorange1);
	if ( rc < 0 ) return rc;
	rc = inp_getf64(in,&recd->phase1);
	if ( rc < 0 ) return rc;
	rc = inp_getf32(in,&recd->doppler1);
	if ( rc < 0 ) return rc;
	rc = inp_getb(in,&recd->sv_prn2,1);
	if ( rc < 0 ) return rc;
	rc = inp_getb(in,&recd->flagsa2,1);
	if ( rc < 0 ) return rc;
	rc = inp_getb(in,&recd->flagsb2,1);
	if ( rc < 0 ) return rc;
	rc = inp_getb(in,&recd->elevation_angle2,1);
	if ( rc < 0 ) return rc;
	rc = inp_geti16(in,&recd->azimuth2);
	if ( rc < 0 ) return rc;
	rc = inp_getb(in,&recd->snrx242,1);
	if ( rc < 0 ) return rc;
	rc = inp_getf64(in,&recd->pseudorange2);
	if ( rc < 0 ) return rc;
	rc = inp_getf64(in,&recd->phase2);
	if ( rc < 0 ) return rc;
	rc = inp_getf32(in,&recd->doppler2);
	if ( rc < 0 ) return rc;
	rc = inp_getb(in,&recd->sv_prn3,1);
	if ( rc < 0 ) return rc;
	rc = inp_getb(in,&recd->flagsa3,1);
	if ( rc < 0 ) return rc;
	rc = inp_getb(in,&recd->flagsb3,1);
	if ( rc < 0 ) return rc;
	rc = inp_getb(in,&recd->elevation_angle3,1);
	if ( rc < 0 ) return rc;
	rc = inp_geti16(in,&recd->azimuth3);
	if ( rc < 0 ) return rc;
	rc = inp_getb(in,&recd->snrx243,1);
	if ( rc < 0 ) return rc;
	rc = inp_getf64(in,&recd->pseudorange3);
	if ( rc < 0 ) return rc;
	rc = inp_getf64(in,&recd->phase3);
	if ( rc < 0 ) return rc;
	rc = inp_getf32(in,&recd->doppler3);
	if ( rc < 0 ) return rc;
	rc = inp_getb(in,&recd->sv_prn4,1);
	if ( rc < 0 ) return rc;
	rc = inp_getb(in,&recd->flagsa4,1);
	if ( rc < 0 ) return rc;
	rc = inp_getb(in,&recd->flagsb4,1);
	if ( rc < 0 ) return rc;
	rc = inp_getb(in,&recd->elevation_angle4,1);
	if ( rc < 0 ) return rc;
	rc = inp_geti16(in,&recd->azimuth4);
	if ( rc < 0 ) return rc;
	rc = inp_getb(in,&recd->snrx244,1);
	if ( rc < 0 ) return rc;
	rc = inp_getf64(in,&recd->pseudorange4);
	if ( rc < 0 ) return rc;
	rc = inp_getf64(in,&recd->phase4);
	if ( rc < 0 ) return rc;
	rc = inp_getf32(in,&recd->doppler4);
	if ( rc < 0 ) return rc;
	rc = inp_getb(in,&recd->sv_prn5,1);
	if ( rc < 0 ) return rc;
	rc = inp_getb(in,&recd->flagsa5,1);
	if ( rc < 0 ) return rc;
	rc = inp_getb(in,&recd->flagsb5,1);
	if ( rc < 0 ) return rc;
	rc = inp_getb(in,&recd->elevation_angle5,1);
	if ( rc < 0 ) return rc;
	rc = inp_geti16(in,&recd->azimuth5);
	if ( rc < 0 ) return rc;
	rc = inp_getb(in,&recd->snrx245,1);
	if ( rc < 0 ) return rc;
	rc = inp_getf64(in,&recd->pseudorange5);
	if ( rc < 0 ) return rc;
	rc = inp_getf64(in,&recd->phase5);
	if ( rc < 0 ) return rc;
	rc = inp_getf32(in,&recd->doppler5);
	if ( rc < 0 ) return rc;
	rc = inp_getb(in,&recd->sv_prn6,1);
	if ( rc < 0 ) return rc;
	rc = inp_getb(in,&recd->flagsa6,1);
	if ( rc < 0 ) return rc;
	rc = inp_getb(in,&recd->flagsb6,1);
	if ( rc < 0 ) return rc;
	rc = inp_getb(in,&recd->elevation_angle6,1);
	if ( rc < 0 ) return rc;
	rc = inp_geti16(in,&recd->azimuth6);
	if ( rc < 0 ) return rc;
	rc = inp_getb(in,&recd->snrx246,1);
	if ( rc < 0 ) return rc;
	rc = inp_getf64(in,&recd->pseudorange6);
	if ( rc < 0 ) return rc;
	rc = inp_getf64(in,&recd->phase6);
	if ( rc < 0 ) return rc;
	rc = inp_getf32(in,&recd->doppler6);
	if ( rc < 0 ) return rc;
	rc = inp_getb(in,&recd->sv_prn7,1);
	if ( rc < 0 ) return rc;
	rc = inp_getb(in,&recd->flagsa7,1);
	if ( rc < 0 ) return rc;
	rc = inp_getb(in,&recd->flagsb7,1);
	if ( rc < 0 ) return rc;
	rc = inp_getb(in,&recd->elevation_angle7,1);
	if ( rc < 0 ) return rc;
	rc = inp_geti16(in,&recd->azimuth7);
	if ( rc < 0 ) return rc;
	return 0;
}

int16_t
decode_R70(s_inpkt *in,s_R70 *recd) {
	rc = inp_getb(in,&recd->dynfilter,1);
	if ( rc < 0 ) return rc;
	rc = inp_getb(in,&recd->statfilter,1);
	if ( rc < 0 ) return rc;
	rc = inp_getb(in,&recd->altfilter,1);
	if ( rc < 0 ) return rc;
	rc = inp_getb(in,&recd->reserved,1);
	if ( rc < 0 ) return rc;
	return 0;
}

int16_t
decode_R76(s_inpkt *in,s_R76 *recd) {
	rc = inp_getb(in,&recd->fixtype,1);
	if ( rc < 0 ) return rc;
	return 0;
}

int16_t
decode_R78(s_inpkt *in,s_R78 *recd) {
	rc = inp_geti16(in,&recd->maxprcage);
	if ( rc < 0 ) return rc;
	return 0;
}

int16_t
decode_R7B00(s_inpkt *in,s_R7B00 *recd) {
	rc = inp_getb(in,&recd->interval,1);
	if ( rc < 0 ) return rc;
	rc = inp_getb(in,&recd->msgmask,4);
	if ( rc < 0 ) return rc;
	return 0;
}

int16_t
decode_R7B06(s_inpkt *in,s_R7B06 *recd) {
	rc = inp_getb(in,&recd->message,1);
	if ( rc < 0 ) return rc;
	rc = inp_getb(in,&recd->optflags,1);
	if ( rc < 0 ) return rc;
	rc = inp_getb(in,&recd->precision,1);
	if ( rc < 0 ) return rc;
	return 0;
}

int16_t
decode_R7B80(s_inpkt *in,s_R7B80 *recd) {
	rc = inp_getb(in,&recd->interval,1);
	if ( rc < 0 ) return rc;
	rc = inp_getb(in,&recd->msgmask,4);
	if ( rc < 0 ) return rc;
	return 0;
}

int16_t
decode_R7B84(s_inpkt *in,s_R7B84 *recd) {
	rc = inp_getb(in,&recd->contents,1);
	if ( rc < 0 ) return rc;
	rc = inp_getb(in,&recd->msglist,253);
	if ( rc < 0 ) return rc;
	return 0;
}

int16_t
decode_R7B85(s_inpkt *in,s_R7B85 *recd) {
	rc = inp_getb(in,&recd->houroffset,1);
	if ( rc < 0 ) return rc;
	rc = inp_getb(in,&recd->minoffset,1);
	if ( rc < 0 ) return rc;
	return 0;
}

int16_t
decode_R7B86(s_inpkt *in,s_R7B86 *recd) {
	rc = inp_getb(in,&recd->message,1);
	if ( rc < 0 ) return rc;
	rc = inp_getb(in,&recd->optflags,1);
	if ( rc < 0 ) return rc;
	rc = inp_getb(in,&recd->precision,1);
	if ( rc < 0 ) return rc;
	return 0;
}

int16_t
decode_R7D00(s_inpkt *in,s_R7D00 *recd) {
	rc = inp_getb(in,&recd->asap_rate,1);
	if ( rc < 0 ) return rc;
	return 0;
}

int16_t
decode_R7D01(s_inpkt *in,s_R7D01 *recd) {
	rc = inp_getb(in,&recd->opt1flags,1);
	if ( rc < 0 ) return rc;
	rc = inp_getb(in,&recd->opt2flags,1);
	if ( rc < 0 ) return rc;
	return 0;
}

int16_t
decode_R7D03(s_inpkt *in,s_R7D03 *recd) {
	rc = inp_geti16(in,&recd->max_age);
	if ( rc < 0 ) return rc;
	return 0;
}

int16_t
decode_R7D05(s_inpkt *in,s_R7D05 *recd) {
	rc = inp_geti16(in,&recd->cts_delay);
	if ( rc < 0 ) return rc;
	return 0;
}

int16_t
decode_R7D06(s_inpkt *in,s_R7D06 *recd) {
	rc = inp_geti16(in,&recd->rts_delay);
	if ( rc < 0 ) return rc;
	return 0;
}

int16_t
decode_R7D09(s_inpkt *in,s_R7D09 *recd) {
	rc = inp_getb(in,&recd->port,1);
	if ( rc < 0 ) return rc;
	rc = inp_getb(in,&recd->protocol,1);
	if ( rc < 0 ) return rc;
	rc = inp_getb(in,&recd->interval,1);
	if ( rc < 0 ) return rc;
	rc = inp_getb(in,&recd->reserved,8);
	if ( rc < 0 ) return rc;
	rc = inp_geti16(in,&recd->checksum);
	if ( rc < 0 ) return rc;
	return 0;
}

int16_t
decode_R85(s_inpkt *in,s_R85 *recd) {
	rc = inp_getb(in,&recd->sv_prn,1);
	if ( rc < 0 ) return rc;
	rc = inp_getb(in,&recd->stats_code,1);
	if ( rc < 0 ) return rc;
	rc = inp_getb(in,&recd->stn_health,1);
	if ( rc < 0 ) return rc;
	rc = inp_getb(in,&recd->sv_health,1);
	if ( rc < 0 ) return rc;
	rc = inp_getb(in,&recd->iode1,1);
	if ( rc < 0 ) return rc;
	rc = inp_getb(in,&recd->iode2,1);
	if ( rc < 0 ) return rc;
	rc = inp_getf32(in,&recd->zcount);
	if ( rc < 0 ) return rc;
	rc = inp_getf32(in,&recd->prc);
	if ( rc < 0 ) return rc;
	rc = inp_getf32(in,&recd->rrc);
	if ( rc < 0 ) return rc;
	rc = inp_getf32(in,&recd->deltaprc);
	if ( rc < 0 ) return rc;
	return 0;
}

int16_t
decode_R8700(s_inpkt *in,s_R8700 *recd) {
	rc = inp_getb(in,&recd->control,1);
	if ( rc < 0 ) return rc;
	return 0;
}

int16_t
decode_R8701(s_inpkt *in,s_R8701 *recd) {
	rc = inp_getb(in,&recd->options1,1);
	if ( rc < 0 ) return rc;
	rc = inp_getb(in,&recd->options2,1);
	if ( rc < 0 ) return rc;
	return 0;
}

int16_t
decode_R8702(s_inpkt *in,s_R8702 *recd) {
	rc = inp_getb(in,&recd->version,1);
	if ( rc < 0 ) return rc;
	return 0;
}

int16_t
decode_R8703(s_inpkt *in,s_R8703 *recd) {
	rc = inp_getb(in,&recd->type,1);
	if ( rc < 0 ) return rc;
	rc = inp_getf64(in,&recd->x);
	if ( rc < 0 ) return rc;
	rc = inp_getf64(in,&recd->y);
	if ( rc < 0 ) return rc;
	rc = inp_getf64(in,&recd->z);
	if ( rc < 0 ) return rc;
	return 0;
}

int16_t
decode_R8704(s_inpkt *in,s_R8704 *recd) {
	rc = inp_geti16(in,&recd->id);
	if ( rc < 0 ) return rc;
	return 0;
}

int16_t
decode_R8705(s_inpkt *in,s_R8705 *recd) {
	rc = inp_getb(in,&recd->length,1);
	if ( rc < 0 ) return rc;
	rc = inp_getb(in,&recd->text,253);
	if ( rc < 0 ) return rc;
	return 0;
}

int16_t
decode_R8706(s_inpkt *in,s_R8706 *recd) {
	rc = inp_getb(in,&recd->type,1);
	if ( rc < 0 ) return rc;
	rc = inp_getb(in,&recd->interval,1);
	if ( rc < 0 ) return rc;
	return 0;
}

int16_t
decode_R8708(s_inpkt *in,s_R8708 *recd) {
	rc = inp_getb(in,&recd->type,1);
	if ( rc < 0 ) return rc;
	rc = inp_geti16(in,&recd->stationid);
	if ( rc < 0 ) return rc;
	rc = inp_geti16(in,&recd->zcount);
	if ( rc < 0 ) return rc;
	rc = inp_geti16(in,&recd->deltatime);
	if ( rc < 0 ) return rc;
	rc = inp_getb(in,&recd->sequno,1);
	if ( rc < 0 ) return rc;
	rc = inp_getb(in,&recd->length,1);
	if ( rc < 0 ) return rc;
	rc = inp_getb(in,&recd->health,1);
	if ( rc < 0 ) return rc;
	rc = inp_getb(in,&recd->svcount,1);
	if ( rc < 0 ) return rc;
	rc = inp_getb(in,&recd->svprn,233);
	if ( rc < 0 ) return rc;
	return 0;
}

int16_t
decode_R8709(s_inpkt *in,s_R8709 *recd) {
	rc = inp_getb(in,&recd->type,1);
	if ( rc < 0 ) return rc;
	rc = inp_getf32(in,&recd->x);
	if ( rc < 0 ) return rc;
	rc = inp_getf32(in,&recd->y);
	if ( rc < 0 ) return rc;
	rc = inp_getf32(in,&recd->z);
	if ( rc < 0 ) return rc;
	return 0;
}

int16_t
decode_R870A(s_inpkt *in,s_R870A *recd) {
	rc = inp_getb(in,&recd->type,1);
	if ( rc < 0 ) return rc;
	rc = inp_geti16(in,&recd->interval);
	if ( rc < 0 ) return rc;
	rc = inp_geti16(in,&recd->offset);
	if ( rc < 0 ) return rc;
	return 0;
}

int16_t
decode_R877D(s_inpkt *in,s_R877D *recd) {
	rc = inp_getb(in,&recd->id,1);
	if ( rc < 0 ) return rc;
	rc = inp_getb(in,&recd->length,1);
	if ( rc < 0 ) return rc;
	rc = inp_getb(in,&recd->first,1);
	if ( rc < 0 ) return rc;
	return 0;
}

int16_t
decode_R877E(s_inpkt *in,s_R877E *recd) {
	rc = inp_getb(in,&recd->warning,1);
	if ( rc < 0 ) return rc;
	return 0;
}

int16_t
decode_R877F(s_inpkt *in,s_R877F *recd) {
	rc = inp_getb(in,&recd->none,1);
	if ( rc < 0 ) return rc;
	return 0;
}

int16_t
decode_R8800(s_inpkt *in,s_R8800 *recd) {
	rc = inp_getb(in,&recd->mode,1);
	if ( rc < 0 ) return rc;
	return 0;
}

int16_t
decode_R8801(s_inpkt *in,s_R8801 *recd) {
	rc = inp_getb(in,&recd->options1,1);
	if ( rc < 0 ) return rc;
	rc = inp_getb(in,&recd->options2,1);
	if ( rc < 0 ) return rc;
	return 0;
}

int16_t
decode_R8802(s_inpkt *in,s_R8802 *recd) {
	rc = inp_getb(in,&recd->version,1);
	if ( rc < 0 ) return rc;
	return 0;
}

int16_t
decode_R8803(s_inpkt *in,s_R8803 *recd) {
	rc = inp_getb(in,&recd->type,1);
	if ( rc < 0 ) return rc;
	rc = inp_getf64(in,&recd->x);
	if ( rc < 0 ) return rc;
	rc = inp_getf64(in,&recd->y);
	if ( rc < 0 ) return rc;
	rc = inp_getf64(in,&recd->z);
	if ( rc < 0 ) return rc;
	return 0;
}

int16_t
decode_R8804(s_inpkt *in,s_R8804 *recd) {
	rc = inp_geti16(in,&recd->id);
	if ( rc < 0 ) return rc;
	return 0;
}

int16_t
decode_R8805(s_inpkt *in,s_R8805 *recd) {
	rc = inp_getb(in,&recd->length,1);
	if ( rc < 0 ) return rc;
	rc = inp_geti16(in,&recd->id);
	if ( rc < 0 ) return rc;
	rc = inp_geti32(in,&recd->time);
	if ( rc < 0 ) return rc;
	rc = inp_getb(in,&recd->text,241);
	if ( rc < 0 ) return rc;
	return 0;
}

int16_t
decode_R8808(s_inpkt *in,s_R8808 *recd) {
	rc = inp_getb(in,&recd->type,1);
	if ( rc < 0 ) return rc;
	rc = inp_geti16(in,&recd->stnid);
	if ( rc < 0 ) return rc;
	rc = inp_geti16(in,&recd->zcount);
	if ( rc < 0 ) return rc;
	rc = inp_geti16(in,&recd->deltatime);
	if ( rc < 0 ) return rc;
	rc = inp_getb(in,&recd->seqno,1);
	if ( rc < 0 ) return rc;
	rc = inp_getb(in,&recd->length,1);
	if ( rc < 0 ) return rc;
	rc = inp_getb(in,&recd->health,1);
	if ( rc < 0 ) return rc;
	rc = inp_getb(in,&recd->svcount,1);
	if ( rc < 0 ) return rc;
	rc = inp_getb(in,&recd->svprn,233);
	if ( rc < 0 ) return rc;
	return 0;
}

int16_t
decode_R887F(s_inpkt *in,s_R887F *recd) {
	rc = inp_getb(in,&recd->fixme,1);
	if ( rc < 0 ) return rc;
	return 0;
}

int16_t
decode_R8B00(s_inpkt *in,s_R8B00 *recd) {
	rc = inp_getb(in,&recd->enabled,1);
	if ( rc < 0 ) return rc;
	rc = inp_getb(in,&recd->reserved1,4);
	if ( rc < 0 ) return rc;
	rc = inp_geti16(in,&recd->checksum);
	if ( rc < 0 ) return rc;
	return 0;
}

int16_t
decode_R8B01(s_inpkt *in,s_R8B01 *recd) {
	rc = inp_getb(in,&recd->enabled,1);
	if ( rc < 0 ) return rc;
	rc = inp_getb(in,&recd->reserved1,4);
	if ( rc < 0 ) return rc;
	rc = inp_geti16(in,&recd->checksum);
	if ( rc < 0 ) return rc;
	return 0;
}

int16_t
decode_R8B02(s_inpkt *in,s_R8B02 *recd) {
	rc = inp_geti32(in,&recd->fixtime);
	if ( rc < 0 ) return rc;
	rc = inp_getb(in,&recd->datavalid,1);
	if ( rc < 0 ) return rc;
	rc = inp_getf32(in,&recd->rms);
	if ( rc < 0 ) return rc;
	rc = inp_getf32(in,&recd->sigmaeast);
	if ( rc < 0 ) return rc;
	rc = inp_getf32(in,&recd->sigmanorth);
	if ( rc < 0 ) return rc;
	rc = inp_getf32(in,&recd->coven);
	if ( rc < 0 ) return rc;
	rc = inp_getf32(in,&recd->sigmaup);
	if ( rc < 0 ) return rc;
	rc = inp_getb(in,&recd->axesvalid,1);
	if ( rc < 0 ) return rc;
	rc = inp_getf32(in,&recd->semimajor);
	if ( rc < 0 ) return rc;
	rc = inp_getf32(in,&recd->semiminor);
	if ( rc < 0 ) return rc;
	rc = inp_getf32(in,&recd->orientation);
	if ( rc < 0 ) return rc;
	rc = inp_getf32(in,&recd->unitvariance);
	if ( rc < 0 ) return rc;
	rc = inp_geti16(in,&recd->epochs);
	if ( rc < 0 ) return rc;
	rc = inp_geti16(in,&recd->dof);
	if ( rc < 0 ) return rc;
	rc = inp_geti16(in,&recd->flags);
	if ( rc < 0 ) return rc;
	rc = inp_getb(in,&recd->reserved,4);
	if ( rc < 0 ) return rc;
	rc = inp_geti16(in,&recd->checksum);
	if ( rc < 0 ) return rc;
	return 0;
}

int16_t
decode_R8B03(s_inpkt *in,s_R8B03 *recd) {
	rc = inp_geti32(in,&recd->fixtime);
	if ( rc < 0 ) return rc;
	rc = inp_getb(in,&recd->datavalid,1);
	if ( rc < 0 ) return rc;
	rc = inp_getf32(in,&recd->rms);
	if ( rc < 0 ) return rc;
	rc = inp_getf32(in,&recd->vcv_xx);
	if ( rc < 0 ) return rc;
	rc = inp_getf32(in,&recd->vcv_xy);
	if ( rc < 0 ) return rc;
	rc = inp_getf32(in,&recd->vcv_xz);
	if ( rc < 0 ) return rc;
	rc = inp_getf32(in,&recd->vcv_yy);
	if ( rc < 0 ) return rc;
	rc = inp_getf32(in,&recd->vcv_yz);
	if ( rc < 0 ) return rc;
	rc = inp_getf32(in,&recd->vcv_zz);
	if ( rc < 0 ) return rc;
	rc = inp_getf32(in,&recd->unitvariance);
	if ( rc < 0 ) return rc;
	rc = inp_geti16(in,&recd->epochs);
	if ( rc < 0 ) return rc;
	rc = inp_geti16(in,&recd->dof);
	if ( rc < 0 ) return rc;
	rc = inp_geti16(in,&recd->flags);
	if ( rc < 0 ) return rc;
	rc = inp_getb(in,&recd->reserved,4);
	if ( rc < 0 ) return rc;
	rc = inp_geti16(in,&recd->checksum);
	if ( rc < 0 ) return rc;
	return 0;
}

int16_t
decode_R8D00(s_inpkt *in,s_R8D00 *recd) {
	rc = inp_getb(in,&recd->stopcode,1);
	if ( rc < 0 ) return rc;
	rc = inp_getb(in,&recd->status,1);
	if ( rc < 0 ) return rc;
	return 0;
}

int16_t
decode_R8D01(s_inpkt *in,s_R8D01 *recd) {
	rc = inp_getb(in,&recd->options1,1);
	if ( rc < 0 ) return rc;
	rc = inp_getb(in,&recd->options2,1);
	if ( rc < 0 ) return rc;
	return 0;
}

int16_t
decode_R8D02(s_inpkt *in,s_R8D02 *recd) {
	rc = inp_getb(in,&recd->type,1);
	if ( rc < 0 ) return rc;
	rc = inp_getb(in,&recd->options1,1);
	if ( rc < 0 ) return rc;
	rc = inp_getb(in,&recd->options2,1);
	if ( rc < 0 ) return rc;
	return 0;
}

int16_t
decode_R8D03(s_inpkt *in,s_R8D03 *recd) {
	rc = inp_getb(in,&recd->type,1);
	if ( rc < 0 ) return rc;
	rc = inp_geti32(in,&recd->number);
	if ( rc < 0 ) return rc;
	rc = inp_geti32(in,&recd->duration);
	if ( rc < 0 ) return rc;
	rc = inp_getf64(in,&recd->x);
	if ( rc < 0 ) return rc;
	rc = inp_getf64(in,&recd->y);
	if ( rc < 0 ) return rc;
	rc = inp_getf64(in,&recd->z);
	if ( rc < 0 ) return rc;
	rc = inp_geti32(in,&recd->time);
	if ( rc < 0 ) return rc;
	return 0;
}

int16_t
decode_R8D04(s_inpkt *in,s_R8D04 *recd) {
	rc = inp_getb(in,&recd->type,1);
	if ( rc < 0 ) return rc;
	rc = inp_geti32(in,&recd->number);
	if ( rc < 0 ) return rc;
	rc = inp_geti32(in,&recd->duration);
	if ( rc < 0 ) return rc;
	rc = inp_getf32(in,&recd->xe);
	if ( rc < 0 ) return rc;
	rc = inp_getf32(in,&recd->yn);
	if ( rc < 0 ) return rc;
	rc = inp_getf32(in,&recd->zu);
	if ( rc < 0 ) return rc;
	rc = inp_geti32(in,&recd->time);
	if ( rc < 0 ) return rc;
	return 0;
}

int16_t
decode_R8F20(s_inpkt *in,s_R8F20 *recd) {
	rc = inp_getb(in,&recd->key,1);
	if ( rc < 0 ) return rc;
	rc = inp_geti16(in,&recd->evelocity);
	if ( rc < 0 ) return rc;
	rc = inp_geti16(in,&recd->nvolocity);
	if ( rc < 0 ) return rc;
	rc = inp_geti16(in,&recd->uvelocity);
	if ( rc < 0 ) return rc;
	rc = inp_geti32(in,&recd->weektime);
	if ( rc < 0 ) return rc;
	rc = inp_geti32(in,&recd->latitude);
	if ( rc < 0 ) return rc;
	rc = inp_geti32(in,&recd->longitude);
	if ( rc < 0 ) return rc;
	rc = inp_geti32(in,&recd->altitude);
	if ( rc < 0 ) return rc;
	rc = inp_getb(in,&recd->reserved1,3);
	if ( rc < 0 ) return rc;
	rc = inp_getb(in,&recd->posfixflags,1);
	if ( rc < 0 ) return rc;
	rc = inp_getb(in,&recd->svcount,1);
	if ( rc < 0 ) return rc;
	rc = inp_getb(in,&recd->utcoffset,1);
	if ( rc < 0 ) return rc;
	rc = inp_geti16(in,&recd->week);
	if ( rc < 0 ) return rc;
	rc = inp_getb(in,&recd->svprnxiode1a,1);
	if ( rc < 0 ) return rc;
	rc = inp_getb(in,&recd->svprnxiode1b,1);
	if ( rc < 0 ) return rc;
	rc = inp_getb(in,&recd->svprnxiode2a,1);
	if ( rc < 0 ) return rc;
	rc = inp_getb(in,&recd->svprnxiode2b,1);
	if ( rc < 0 ) return rc;
	rc = inp_getb(in,&recd->svprnxiode3a,1);
	if ( rc < 0 ) return rc;
	rc = inp_getb(in,&recd->svprnxiode3b,1);
	if ( rc < 0 ) return rc;
	rc = inp_getb(in,&recd->svprnxiode4a,1);
	if ( rc < 0 ) return rc;
	rc = inp_getb(in,&recd->svprnxiode4b,1);
	if ( rc < 0 ) return rc;
	rc = inp_getb(in,&recd->svprnxiode5a,1);
	if ( rc < 0 ) return rc;
	rc = inp_getb(in,&recd->svprnxiode5b,1);
	if ( rc < 0 ) return rc;
	rc = inp_getb(in,&recd->svprnxiode6a,1);
	if ( rc < 0 ) return rc;
	rc = inp_getb(in,&recd->svprnxiode6b,1);
	if ( rc < 0 ) return rc;
	rc = inp_getb(in,&recd->svprnxiode7a,1);
	if ( rc < 0 ) return rc;
	rc = inp_getb(in,&recd->svprnxiode7b,1);
	if ( rc < 0 ) return rc;
	rc = inp_getb(in,&recd->svprnxiode8a,1);
	if ( rc < 0 ) return rc;
	rc = inp_getb(in,&recd->svprnxiode8b,1);
	if ( rc < 0 ) return rc;
	rc = inp_getb(in,&recd->ionoshperic,1);
	if ( rc < 0 ) return rc;
	return 0;
}

int16_t
decode_R8F60(s_inpkt *in,s_R8F60 *recd) {
	rc = inp_getf32(in,&recd->oscalef);
	if ( rc < 0 ) return rc;
	rc = inp_getf32(in,&recd->glscalef);
	if ( rc < 0 ) return rc;
	rc = inp_getf32(in,&recd->grscalef);
	if ( rc < 0 ) return rc;
	rc = inp_getb(in,&recd->flag,1);
	if ( rc < 0 ) return rc;
	return 0;
}

int16_t
decode_R8F62(s_inpkt *in,s_R8F62 *recd) {
	rc = inp_getf64(in,&recd->time_tag);
	if ( rc < 0 ) return rc;
	rc = inp_getf64(in,&recd->latitude);
	if ( rc < 0 ) return rc;
	rc = inp_getf64(in,&recd->longitude);
	if ( rc < 0 ) return rc;
	rc = inp_getf64(in,&recd->altitude);
	if ( rc < 0 ) return rc;
	rc = inp_getf64(in,&recd->clockbias);
	if ( rc < 0 ) return rc;
	rc = inp_geti16(in,&recd->possource);
	if ( rc < 0 ) return rc;
	rc = inp_getf64(in,&recd->evolocity);
	if ( rc < 0 ) return rc;
	rc = inp_getf64(in,&recd->nvelocity);
	if ( rc < 0 ) return rc;
	rc = inp_getf64(in,&recd->uvelocity);
	if ( rc < 0 ) return rc;
	rc = inp_getf64(in,&recd->clockbiasvel);
	if ( rc < 0 ) return rc;
	rc = inp_getb(in,&recd->velsource,1);
	if ( rc < 0 ) return rc;
	rc = inp_getb(in,&recd->shstatus,1);
	if ( rc < 0 ) return rc;
	rc = inp_getb(in,&recd->svcount,1);
	if ( rc < 0 ) return rc;
	rc = inp_getb(in,&recd->svprn,101);
	if ( rc < 0 ) return rc;
	return 0;
}

int16_t
decode_R8F64(s_inpkt *in,s_R8F64 *recd) {
	rc = inp_getb(in,&recd->rpttype,1);
	if ( rc < 0 ) return rc;
	rc = inp_getb(in,&recd->Board,253);
	if ( rc < 0 ) return rc;
	return 0;
}

int16_t
decode_R8F6B(s_inpkt *in,s_R8F6B *recd) {
	rc = inp_getf64(in,&recd->gheading);
	if ( rc < 0 ) return rc;
	rc = inp_getf64(in,&recd->ghrate);
	if ( rc < 0 ) return rc;
	rc = inp_getf64(in,&recd->ghrbias);
	if ( rc < 0 ) return rc;
	rc = inp_getf64(in,&recd->timetag);
	if ( rc < 0 ) return rc;
	rc = inp_getf64(in,&recd->gdutycycle);
	if ( rc < 0 ) return rc;
	rc = inp_getb(in,&recd->health,1);
	if ( rc < 0 ) return rc;
	rc = inp_getf64(in,&recd->gscalef1);
	if ( rc < 0 ) return rc;
	rc = inp_getf64(in,&recd->gscalef2);
	if ( rc < 0 ) return rc;
	rc = inp_getf32(in,&recd->gvar);
	if ( rc < 0 ) return rc;
	rc = inp_getf32(in,&recd->hcorr);
	if ( rc < 0 ) return rc;
	return 0;
}

int16_t
decode_R8F6D(s_inpkt *in,s_R8F6D *recd) {
	rc = inp_getb(in,&recd->lastseccount,1);
	if ( rc < 0 ) return rc;
	rc = inp_getf64(in,&recd->speed);
	if ( rc < 0 ) return rc;
	rc = inp_getf64(in,&recd->samptime);
	if ( rc < 0 ) return rc;
	rc = inp_getf64(in,&recd->timetag);
	if ( rc < 0 ) return rc;
	rc = inp_getf32(in,&recd->scalefactor);
	if ( rc < 0 ) return rc;
	rc = inp_getf32(in,&recd->variance);
	if ( rc < 0 ) return rc;
	rc = inp_getb(in,&recd->health,1);
	if ( rc < 0 ) return rc;
	rc = inp_getb(in,&recd->adswitch,1);
	if ( rc < 0 ) return rc;
	rc = inp_getb(in,&recd->potvalue,1);
	if ( rc < 0 ) return rc;
	rc = inp_getb(in,&recd->ostatus,1);
	if ( rc < 0 ) return rc;
	return 0;
}

int16_t
decode_R8F6F(s_inpkt *in,s_R8F6F *recd) {
	rc = inp_getb(in,&recd->versname,20);
	if ( rc < 0 ) return rc;
	rc = inp_geti16(in,&recd->checksum);
	if ( rc < 0 ) return rc;
	return 0;
}

int16_t
decode_R8F70(s_inpkt *in,s_R8F70 *recd) {
	rc = inp_getb(in,&recd->channels1,1);
	if ( rc < 0 ) return rc;
	rc = inp_geti16(in,&recd->frequency1);
	if ( rc < 0 ) return rc;
	rc = inp_getb(in,&recd->mode1,1);
	if ( rc < 0 ) return rc;
	rc = inp_getb(in,&recd->status1,1);
	if ( rc < 0 ) return rc;
	rc = inp_getb(in,&recd->rtcmused1,1);
	if ( rc < 0 ) return rc;
	rc = inp_getb(in,&recd->snr1,1);
	if ( rc < 0 ) return rc;
	rc = inp_getb(in,&recd->inputlevel1,1);
	if ( rc < 0 ) return rc;
	rc = inp_getb(in,&recd->rateindex1,1);
	if ( rc < 0 ) return rc;
	rc = inp_getb(in,&recd->lockind1,1);
	if ( rc < 0 ) return rc;
	rc = inp_getb(in,&recd->carrieroff1,1);
	if ( rc < 0 ) return rc;
	rc = inp_getb(in,&recd->bitrateoff1,1);
	if ( rc < 0 ) return rc;
	rc = inp_getb(in,&recd->worderrorrate1,1);
	if ( rc < 0 ) return rc;
	rc = inp_getb(in,&recd->health1,1);
	if ( rc < 0 ) return rc;
	rc = inp_geti16(in,&recd->frequency2);
	if ( rc < 0 ) return rc;
	rc = inp_getb(in,&recd->mode2,1);
	if ( rc < 0 ) return rc;
	rc = inp_getb(in,&recd->status2,1);
	if ( rc < 0 ) return rc;
	rc = inp_getb(in,&recd->rtcmused2,1);
	if ( rc < 0 ) return rc;
	rc = inp_getb(in,&recd->snr2,1);
	if ( rc < 0 ) return rc;
	rc = inp_getb(in,&recd->inputlevel2,1);
	if ( rc < 0 ) return rc;
	rc = inp_getb(in,&recd->rateindex2,1);
	if ( rc < 0 ) return rc;
	rc = inp_getb(in,&recd->lockind2,1);
	if ( rc < 0 ) return rc;
	rc = inp_getb(in,&recd->carrieroff2,1);
	if ( rc < 0 ) return rc;
	rc = inp_getb(in,&recd->bitrateoff2,1);
	if ( rc < 0 ) return rc;
	rc = inp_getb(in,&recd->worderrorrate2,1);
	if ( rc < 0 ) return rc;
	rc = inp_getb(in,&recd->health2,1);
	if ( rc < 0 ) return rc;
	rc = inp_geti16(in,&recd->checksum);
	if ( rc < 0 ) return rc;
	return 0;
}

int16_t
decode_R8F73(s_inpkt *in,s_R8F73 *recd) {
	rc = inp_geti16(in,&recd->checksum);
	if ( rc < 0 ) return rc;
	return 0;
}

int16_t
decode_R8F74(s_inpkt *in,s_R8F74 *recd) {
	rc = inp_geti16(in,&recd->checksum);
	if ( rc < 0 ) return rc;
	return 0;
}

int16_t
decode_R8F75(s_inpkt *in,s_R8F75 *recd) {
	rc = inp_geti16(in,&recd->checksum);
	if ( rc < 0 ) return rc;
	return 0;
}

int16_t
decode_R8F76(s_inpkt *in,s_R8F76 *recd) {
	rc = inp_geti16(in,&recd->checksum);
	if ( rc < 0 ) return rc;
	return 0;
}

int16_t
decode_R8F77(s_inpkt *in,s_R8F77 *recd) {
	rc = inp_getb(in,&recd->pageno,1);
	if ( rc < 0 ) return rc;
	rc = inp_getf64(in,&recd->frequency);
	if ( rc < 0 ) return rc;
	rc = inp_getf64(in,&recd->binsize);
	if ( rc < 0 ) return rc;
	rc = inp_getb(in,&recd->inpsquared,1);
	if ( rc < 0 ) return rc;
	rc = inp_getb(in,&recd->integrations,1);
	if ( rc < 0 ) return rc;
	rc = inp_geti16(in,&recd->nobins);
	if ( rc < 0 ) return rc;
	rc = inp_getf32(in,&recd->maxlevel);
	if ( rc < 0 ) return rc;
	rc = inp_getb(in,&recd->avgpower,99);
	if ( rc < 0 ) return rc;
	rc = inp_geti16(in,&recd->checksum);
	if ( rc < 0 ) return rc;
	return 0;
}

int16_t
decode_R8F78(s_inpkt *in,s_R8F78 *recd) {
	rc = inp_getb(in,&recd->channel,1);
	if ( rc < 0 ) return rc;
	rc = inp_geti32(in,&recd->time);
	if ( rc < 0 ) return rc;
	rc = inp_getb(in,&recd->rtcmbytes,245);
	if ( rc < 0 ) return rc;
	return 0;
}

int16_t
decode_R8F79(s_inpkt *in,s_R8F79 *recd) {
	rc = inp_geti16(in,&recd->checksum);
	if ( rc < 0 ) return rc;
	return 0;
}

int16_t
decode_R8F7A(s_inpkt *in,s_R8F7A *recd) {
	rc = inp_getb(in,&recd->attribute,84);
	if ( rc < 0 ) return rc;
	rc = inp_geti16(in,&recd->checksum);
	if ( rc < 0 ) return rc;
	return 0;
}

int16_t
decode_R8F7B(s_inpkt *in,s_R8F7B *recd) {
	rc = inp_getb(in,&recd->portno,1);
	if ( rc < 0 ) return rc;
	rc = inp_getb(in,&recd->name,20);
	if ( rc < 0 ) return rc;
	rc = inp_getb(in,&recd->majorvers,1);
	if ( rc < 0 ) return rc;
	rc = inp_getb(in,&recd->minorvers,1);
	if ( rc < 0 ) return rc;
	rc = inp_getb(in,&recd->month,1);
	if ( rc < 0 ) return rc;
	rc = inp_getb(in,&recd->day,1);
	if ( rc < 0 ) return rc;
	rc = inp_getb(in,&recd->year,1);
	if ( rc < 0 ) return rc;
	rc = inp_getb(in,&recd->head,1);
	if ( rc < 0 ) return rc;
	rc = inp_getb(in,&recd->verion,1);
	if ( rc < 0 ) return rc;
	rc = inp_getb(in,&recd->serialno,32);
	if ( rc < 0 ) return rc;
	rc = inp_getb(in,&recd->rcvrday,1);
	if ( rc < 0 ) return rc;
	rc = inp_getb(in,&recd->rcvrmonth,1);
	if ( rc < 0 ) return rc;
	rc = inp_geti16(in,&recd->rcvryear);
	if ( rc < 0 ) return rc;
	rc = inp_geti16(in,&recd->superpktopt);
	if ( rc < 0 ) return rc;
	rc = inp_getb(in,&recd->inputprotocolb,1);
	if ( rc < 0 ) return rc;
	rc = inp_getb(in,&recd->outputprotocolb,1);
	if ( rc < 0 ) return rc;
	rc = inp_getb(in,&recd->inputbaudb,1);
	if ( rc < 0 ) return rc;
	rc = inp_getb(in,&recd->outputbaudb,1);
	if ( rc < 0 ) return rc;
	rc = inp_getb(in,&recd->parityb,1);
	if ( rc < 0 ) return rc;
	rc = inp_getb(in,&recd->databitsb,1);
	if ( rc < 0 ) return rc;
	rc = inp_getb(in,&recd->stopbitsb,1);
	if ( rc < 0 ) return rc;
	rc = inp_getb(in,&recd->reserved1b,1);
	if ( rc < 0 ) return rc;
	rc = inp_getb(in,&recd->inputprotocola,1);
	if ( rc < 0 ) return rc;
	rc = inp_getb(in,&recd->outputprotocola,1);
	if ( rc < 0 ) return rc;
	rc = inp_getb(in,&recd->inputbauda,1);
	if ( rc < 0 ) return rc;
	rc = inp_getb(in,&recd->outputbauda,1);
	if ( rc < 0 ) return rc;
	rc = inp_getb(in,&recd->paritya,1);
	if ( rc < 0 ) return rc;
	rc = inp_getb(in,&recd->databitsa,1);
	if ( rc < 0 ) return rc;
	rc = inp_getb(in,&recd->stopbitsa,1);
	if ( rc < 0 ) return rc;
	rc = inp_getb(in,&recd->reserved1a,1);
	if ( rc < 0 ) return rc;
	rc = inp_getb(in,&recd->productid,1);
	if ( rc < 0 ) return rc;
	rc = inp_getb(in,&recd->pvfilter,1);
	if ( rc < 0 ) return rc;
	rc = inp_getb(in,&recd->carrierphase,1);
	if ( rc < 0 ) return rc;
	rc = inp_getb(in,&recd->pps1flag,1);
	if ( rc < 0 ) return rc;
	rc = inp_getf32(in,&recd->antgain);
	if ( rc < 0 ) return rc;
	rc = inp_getb(in,&recd->eventflag,1);
	if ( rc < 0 ) return rc;
	rc = inp_getb(in,&recd->beaconenb,1);
	if ( rc < 0 ) return rc;
	rc = inp_getb(in,&recd->maxposrate,1);
	if ( rc < 0 ) return rc;
	rc = inp_getb(in,&recd->beaconattr,1);
	if ( rc < 0 ) return rc;
	rc = inp_getb(in,&recd->rtcmtimeout,1);
	if ( rc < 0 ) return rc;
	rc = inp_getb(in,&recd->scorpiodecode,1);
	if ( rc < 0 ) return rc;
	rc = inp_getb(in,&recd->basestation,1);
	if ( rc < 0 ) return rc;
	rc = inp_getb(in,&recd->everest,1);
	if ( rc < 0 ) return rc;
	rc = inp_getb(in,&recd->modemctl,1);
	if ( rc < 0 ) return rc;
	rc = inp_getb(in,&recd->reserved2,1);
	if ( rc < 0 ) return rc;
	rc = inp_geti16(in,&recd->desubsc);
	if ( rc < 0 ) return rc;
	rc = inp_getb(in,&recd->guidance,1);
	if ( rc < 0 ) return rc;
	rc = inp_getb(in,&recd->satdiffprov,1);
	if ( rc < 0 ) return rc;
	rc = inp_getb(in,&recd->language,1);
	if ( rc < 0 ) return rc;
	rc = inp_getb(in,&recd->hardware,1);
	if ( rc < 0 ) return rc;
	rc = inp_geti16(in,&recd->tnlsubsc);
	if ( rc < 0 ) return rc;
	rc = inp_getb(in,&recd->displayunits,1);
	if ( rc < 0 ) return rc;
	rc = inp_getb(in,&recd->disstream,1);
	if ( rc < 0 ) return rc;
	rc = inp_getb(in,&recd->reserved3,40);
	if ( rc < 0 ) return rc;
	rc = inp_geti16(in,&recd->endblk);
	if ( rc < 0 ) return rc;
	rc = inp_geti16(in,&recd->checksum1);
	if ( rc < 0 ) return rc;
	rc = inp_geti16(in,&recd->checksum2);
	if ( rc < 0 ) return rc;
	return 0;
}

int16_t
decode_R8F7C(s_inpkt *in,s_R8F7C *recd) {
	rc = inp_geti16(in,&recd->checksum);
	if ( rc < 0 ) return rc;
	return 0;
}

int16_t
decode_R8F7E(s_inpkt *in,s_R8F7E *recd) {
	rc = inp_getb(in,&recd->count,1);
	if ( rc < 0 ) return rc;
	rc = inp_getf64(in,&recd->usertime);
	if ( rc < 0 ) return rc;
	rc = inp_getb(in,&recd->chanstatus0,1);
	if ( rc < 0 ) return rc;
	rc = inp_getb(in,&recd->chanstatus1,1);
	if ( rc < 0 ) return rc;
	rc = inp_getf64(in,&recd->pseudorange);
	if ( rc < 0 ) return rc;
	rc = inp_getf32(in,&recd->deltapseudorange);
	if ( rc < 0 ) return rc;
	rc = inp_getf32(in,&recd->pseudorangevariance);
	if ( rc < 0 ) return rc;
	rc = inp_getf32(in,&recd->deltapseudovariance);
	if ( rc < 0 ) return rc;
	rc = inp_getf64(in,&recd->svxpos);
	if ( rc < 0 ) return rc;
	rc = inp_getf64(in,&recd->svypos);
	if ( rc < 0 ) return rc;
	rc = inp_getf64(in,&recd->svzpos);
	if ( rc < 0 ) return rc;
	rc = inp_getf32(in,&recd->svxdelta);
	if ( rc < 0 ) return rc;
	rc = inp_getf32(in,&recd->svydelta);
	if ( rc < 0 ) return rc;
	rc = inp_getf32(in,&recd->svzdelta);
	if ( rc < 0 ) return rc;
	rc = inp_getf32(in,&recd->ionoshpericcorr);
	if ( rc < 0 ) return rc;
	rc = inp_geti16(in,&recd->ephemura);
	if ( rc < 0 ) return rc;
	rc = inp_getf32(in,&recd->deltapseudoint);
	if ( rc < 0 ) return rc;
	rc = inp_geti16(in,&recd->checksum);
	if ( rc < 0 ) return rc;
	return 0;
}

int16_t
decode_R8F7F(s_inpkt *in,s_R8F7F *recd) {
	rc = inp_getb(in,&recd->portno,1);
	if ( rc < 0 ) return rc;
	rc = inp_getb(in,&recd->name,20);
	if ( rc < 0 ) return rc;
	rc = inp_getb(in,&recd->majorvers,1);
	if ( rc < 0 ) return rc;
	rc = inp_getb(in,&recd->minorvers,1);
	if ( rc < 0 ) return rc;
	rc = inp_getb(in,&recd->month,1);
	if ( rc < 0 ) return rc;
	rc = inp_getb(in,&recd->day,1);
	if ( rc < 0 ) return rc;
	rc = inp_getb(in,&recd->year,1);
	if ( rc < 0 ) return rc;
	rc = inp_getb(in,&recd->head,1);
	if ( rc < 0 ) return rc;
	rc = inp_getb(in,&recd->version,1);
	if ( rc < 0 ) return rc;
	rc = inp_getb(in,&recd->serialno,32);
	if ( rc < 0 ) return rc;
	rc = inp_getb(in,&recd->rcvrday,1);
	if ( rc < 0 ) return rc;
	rc = inp_getb(in,&recd->rcvrmonth,1);
	if ( rc < 0 ) return rc;
	rc = inp_geti16(in,&recd->rcvryear);
	if ( rc < 0 ) return rc;
	rc = inp_geti16(in,&recd->superpktopt);
	if ( rc < 0 ) return rc;
	rc = inp_getb(in,&recd->inpprotocolb,1);
	if ( rc < 0 ) return rc;
	rc = inp_getb(in,&recd->outprotocolb,1);
	if ( rc < 0 ) return rc;
	rc = inp_getb(in,&recd->inputbaudb,1);
	if ( rc < 0 ) return rc;
	rc = inp_getb(in,&recd->outputbaudb,1);
	if ( rc < 0 ) return rc;
	rc = inp_getb(in,&recd->parityb,1);
	if ( rc < 0 ) return rc;
	rc = inp_getb(in,&recd->databitsb,1);
	if ( rc < 0 ) return rc;
	rc = inp_getb(in,&recd->stopbitsb,1);
	if ( rc < 0 ) return rc;
	rc = inp_getb(in,&recd->reserved1,1);
	if ( rc < 0 ) return rc;
	rc = inp_getb(in,&recd->inpprotocola,1);
	if ( rc < 0 ) return rc;
	rc = inp_getb(in,&recd->outprotocola,1);
	if ( rc < 0 ) return rc;
	rc = inp_getb(in,&recd->inpbauda,1);
	if ( rc < 0 ) return rc;
	rc = inp_getb(in,&recd->outbauda,1);
	if ( rc < 0 ) return rc;
	rc = inp_getb(in,&recd->paritya,1);
	if ( rc < 0 ) return rc;
	rc = inp_getb(in,&recd->databitsa,1);
	if ( rc < 0 ) return rc;
	rc = inp_getb(in,&recd->stopbitsa,1);
	if ( rc < 0 ) return rc;
	rc = inp_getb(in,&recd->reserved2,1);
	if ( rc < 0 ) return rc;
	rc = inp_getb(in,&recd->prodid,1);
	if ( rc < 0 ) return rc;
	rc = inp_getb(in,&recd->pvfilter,1);
	if ( rc < 0 ) return rc;
	rc = inp_getb(in,&recd->carrierphase,1);
	if ( rc < 0 ) return rc;
	rc = inp_getb(in,&recd->pps1flag,1);
	if ( rc < 0 ) return rc;
	rc = inp_getf32(in,&recd->antgain);
	if ( rc < 0 ) return rc;
	rc = inp_getb(in,&recd->eventflag,1);
	if ( rc < 0 ) return rc;
	rc = inp_getb(in,&recd->beaconenb,1);
	if ( rc < 0 ) return rc;
	rc = inp_getb(in,&recd->maxposrate,1);
	if ( rc < 0 ) return rc;
	rc = inp_getb(in,&recd->beaconattr,1);
	if ( rc < 0 ) return rc;
	rc = inp_getb(in,&recd->rtcmtimeout,1);
	if ( rc < 0 ) return rc;
	rc = inp_getb(in,&recd->scorpiodecode,1);
	if ( rc < 0 ) return rc;
	rc = inp_getb(in,&recd->basestation,1);
	if ( rc < 0 ) return rc;
	rc = inp_getb(in,&recd->everest,1);
	if ( rc < 0 ) return rc;
	rc = inp_getb(in,&recd->modemctl,1);
	if ( rc < 0 ) return rc;
	rc = inp_getb(in,&recd->reserved3,1);
	if ( rc < 0 ) return rc;
	rc = inp_geti16(in,&recd->desubscr);
	if ( rc < 0 ) return rc;
	rc = inp_getb(in,&recd->guidance,1);
	if ( rc < 0 ) return rc;
	rc = inp_getb(in,&recd->satdiffprov,1);
	if ( rc < 0 ) return rc;
	rc = inp_getb(in,&recd->language,1);
	if ( rc < 0 ) return rc;
	rc = inp_getb(in,&recd->hardware,1);
	if ( rc < 0 ) return rc;
	rc = inp_geti16(in,&recd->tnlsubsc);
	if ( rc < 0 ) return rc;
	rc = inp_getb(in,&recd->dispunits,1);
	if ( rc < 0 ) return rc;
	rc = inp_getb(in,&recd->disstream,1);
	if ( rc < 0 ) return rc;
	rc = inp_getb(in,&recd->reserved4,40);
	if ( rc < 0 ) return rc;
	rc = inp_geti16(in,&recd->endcode);
	if ( rc < 0 ) return rc;
	rc = inp_geti16(in,&recd->checksum1);
	if ( rc < 0 ) return rc;
	rc = inp_geti16(in,&recd->checksum2);
	if ( rc < 0 ) return rc;
	return 0;
}

int16_t
decode_R8171(s_inpkt *in,s_R8171 *recd) {
	rc = inp_getb(in,&recd->recordx,1);
	if ( rc < 0 ) return rc;
	rc = inp_geti16(in,&recd->stationid);
	if ( rc < 0 ) return rc;
	rc = inp_geti16(in,&recd->frequency);
	if ( rc < 0 ) return rc;
	rc = inp_getb(in,&recd->modulationrate,1);
	if ( rc < 0 ) return rc;
	rc = inp_getb(in,&recd->health,1);
	if ( rc < 0 ) return rc;
	rc = inp_getf32(in,&recd->latitude);
	if ( rc < 0 ) return rc;
	rc = inp_getf32(in,&recd->longitude);
	if ( rc < 0 ) return rc;
	rc = inp_getf32(in,&recd->distance);
	if ( rc < 0 ) return rc;
	rc = inp_getf32(in,&recd->range);
	if ( rc < 0 ) return rc;
	rc = inp_geti32(in,&recd->seconds);
	if ( rc < 0 ) return rc;
	rc = inp_getb(in,&recd->uscgx,1);
	if ( rc < 0 ) return rc;
	rc = inp_geti16(in,&recd->checksum);
	if ( rc < 0 ) return rc;
	return 0;
}


#endif

// End tsip.cpp
