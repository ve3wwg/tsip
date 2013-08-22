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
#if 0
	if ( !get(recd.z_velocity) )
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
#endif
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
RxPacket::get(s_R4B& recd) {

	if ( !get(recd.machine_id) )
		return false;
#if 0
	if ( !get(recd.u1.raw_status1) )
		return false;
	if ( !get(recd.status2) )
		return false;
#endif
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
		return false;
	if ( !get(recd.fit_ival_flag) )
		return false;
	if ( !get(recd.ura) )
		return false;
	return true;
}

bool
RxPacket::get(s_R6D& recd) {
	uint16_t blen;

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
	blen = get(recd.sv_prn,sizeof recd.sv_prn);
	if ( blen < sizeof recd.sv_prn )
		return false;

	return true;
}

bool
RxPacket::get(s_R82& recd) {
	uint8_t b;

	if ( !get(b) )
		return false;
	recd.mode = Mode82(b);
#if 0
	if ( !get(b) )
		return false;
	recd.rtcm_vers = RtcmVers(b);
	if ( !get(recd.refstatn) )
		return false;
#endif
	return true;
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
TxPacket::close() {
	static const uint8_t ending[] = { 0x10, 0x03 };
	return put(ending,2);
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
decode_R45(s_inpkt *in,s_R45 *recd) {
	rc = inp_getb(in,&recd->nav_proc_major,1);
	if ( rc < 0 ) return rc;
	rc = inp_getb(in,&recd->nav_proc_minor,1);
	if ( rc < 0 ) return rc;
	rc = inp_getb(in,&recd->nav_proc_month,1);
	if ( rc < 0 ) return rc;
	rc = inp_getb(in,&recd->nav_proc_day,1);
	if ( rc < 0 ) return rc;
	rc = inp_getb(in,&recd->nav_proc_year,1);
	if ( rc < 0 ) return rc;
	rc = inp_getb(in,&recd->sig_proc_major,1);
	if ( rc < 0 ) return rc;
	rc = inp_getb(in,&recd->sig_proc_minor,1);
	if ( rc < 0 ) return rc;
	rc = inp_getb(in,&recd->sig_proc_month,1);
	if ( rc < 0 ) return rc;
	rc = inp_getb(in,&recd->sig_proc_day,1);
	if ( rc < 0 ) return rc;
	rc = inp_getb(in,&recd->sig_proc_year,1);
	if ( rc < 0 ) return rc;
	rc = inp_getb(in,&recd->bsd_serial_no,5);
	if ( rc < 0 ) return rc;
	rc = inp_getb(in,&recd->checksum,1);
	if ( rc < 0 ) return rc;
	rc = inp_geti16(in,&recd->revision);
	if ( rc < 0 ) return rc;
	rc = inp_getb(in,&recd->machine_id,1);
	if ( rc < 0 ) return rc;
	rc = inp_getb(in,&recd->config_length,1);
	if ( rc < 0 ) return rc;
	rc = inp_getb(in,&recd->channels,1);
	if ( rc < 0 ) return rc;
	rc = inp_getb(in,&recd->rtcm_input,1);
	if ( rc < 0 ) return rc;
	rc = inp_getb(in,&recd->rtcm_output,1);
	if ( rc < 0 ) return rc;
	rc = inp_getb(in,&recd->fix_rate,1);
	if ( rc < 0 ) return rc;
	rc = inp_getb(in,&recd->sync_meas,1);
	if ( rc < 0 ) return rc;
	rc = inp_getb(in,&recd->misc,1);
	if ( rc < 0 ) return rc;
	rc = inp_getb(in,&recd->nmea_output,1);
	if ( rc < 0 ) return rc;
	rc = inp_getb(in,&recd->pps1_output,1);
	if ( rc < 0 ) return rc;
	rc = inp_getb(in,&recd->product_id,1);
	if ( rc < 0 ) return rc;
	rc = inp_getb(in,&recd->reserved1,1);
	if ( rc < 0 ) return rc;
	rc = inp_getb(in,&recd->reserved2,64);
	if ( rc < 0 ) return rc;
	rc = inp_geti16(in,&recd->Checksum);
	if ( rc < 0 ) return rc;
	return 0;
}

int16_t
decode_R47(s_inpkt *in,s_R47 *recd) {
	rc = inp_getb(in,&recd->count,1);
	if ( rc < 0 ) return rc;
	rc = inp_getb(in,&recd->sat1,1);
	if ( rc < 0 ) return rc;
	rc = inp_getf32(in,&recd->signal_lev1);
	if ( rc < 0 ) return rc;
	rc = inp_getb(in,&recd->sat2,1);
	if ( rc < 0 ) return rc;
	rc = inp_getf32(in,&recd->signal_lev2);
	if ( rc < 0 ) return rc;
	rc = inp_getb(in,&recd->sat3,1);
	if ( rc < 0 ) return rc;
	rc = inp_getf32(in,&recd->signal_lev3);
	if ( rc < 0 ) return rc;
	rc = inp_getb(in,&recd->sat4,1);
	if ( rc < 0 ) return rc;
	rc = inp_getf32(in,&recd->signal_lev4);
	if ( rc < 0 ) return rc;
	rc = inp_getb(in,&recd->sat5,1);
	if ( rc < 0 ) return rc;
	rc = inp_getf32(in,&recd->signal_lev5);
	if ( rc < 0 ) return rc;
	rc = inp_getb(in,&recd->sat6,1);
	if ( rc < 0 ) return rc;
	rc = inp_getf32(in,&recd->signal_lev6);
	if ( rc < 0 ) return rc;
	rc = inp_getb(in,&recd->sat7,1);
	if ( rc < 0 ) return rc;
	rc = inp_getf32(in,&recd->signal_lev7);
	if ( rc < 0 ) return rc;
	rc = inp_getb(in,&recd->sat8,1);
	if ( rc < 0 ) return rc;
	rc = inp_getf32(in,&recd->signal_lev8);
	if ( rc < 0 ) return rc;
	rc = inp_getb(in,&recd->sat9,1);
	if ( rc < 0 ) return rc;
	rc = inp_getf32(in,&recd->signal_lev9);
	if ( rc < 0 ) return rc;
	rc = inp_getb(in,&recd->sat10,1);
	if ( rc < 0 ) return rc;
	rc = inp_getf32(in,&recd->signal_lev10);
	if ( rc < 0 ) return rc;
	rc = inp_getb(in,&recd->sat11,1);
	if ( rc < 0 ) return rc;
	rc = inp_getf32(in,&recd->signal_lev11);
	if ( rc < 0 ) return rc;
	rc = inp_getb(in,&recd->sat12,1);
	if ( rc < 0 ) return rc;
	rc = inp_getf32(in,&recd->signal_lev12);
	if ( rc < 0 ) return rc;
	return 0;
}

int16_t
decode_R49(s_inpkt *in,s_R49 *recd) {
	rc = inp_getb(in,&recd->health1,1);
	if ( rc < 0 ) return rc;
	rc = inp_getb(in,&recd->health2,1);
	if ( rc < 0 ) return rc;
	rc = inp_getb(in,&recd->health3,1);
	if ( rc < 0 ) return rc;
	rc = inp_getb(in,&recd->health4,1);
	if ( rc < 0 ) return rc;
	rc = inp_getb(in,&recd->health5,1);
	if ( rc < 0 ) return rc;
	rc = inp_getb(in,&recd->health6,1);
	if ( rc < 0 ) return rc;
	rc = inp_getb(in,&recd->health7,1);
	if ( rc < 0 ) return rc;
	rc = inp_getb(in,&recd->health8,1);
	if ( rc < 0 ) return rc;
	rc = inp_getb(in,&recd->health9,1);
	if ( rc < 0 ) return rc;
	rc = inp_getb(in,&recd->health10,1);
	if ( rc < 0 ) return rc;
	rc = inp_getb(in,&recd->health11,1);
	if ( rc < 0 ) return rc;
	rc = inp_getb(in,&recd->health12,1);
	if ( rc < 0 ) return rc;
	rc = inp_getb(in,&recd->health13,1);
	if ( rc < 0 ) return rc;
	rc = inp_getb(in,&recd->health14,1);
	if ( rc < 0 ) return rc;
	rc = inp_getb(in,&recd->health15,1);
	if ( rc < 0 ) return rc;
	rc = inp_getb(in,&recd->health16,1);
	if ( rc < 0 ) return rc;
	rc = inp_getb(in,&recd->health17,1);
	if ( rc < 0 ) return rc;
	rc = inp_getb(in,&recd->health18,1);
	if ( rc < 0 ) return rc;
	rc = inp_getb(in,&recd->health19,1);
	if ( rc < 0 ) return rc;
	rc = inp_getb(in,&recd->health20,1);
	if ( rc < 0 ) return rc;
	rc = inp_getb(in,&recd->health21,1);
	if ( rc < 0 ) return rc;
	rc = inp_getb(in,&recd->health22,1);
	if ( rc < 0 ) return rc;
	rc = inp_getb(in,&recd->health23,1);
	if ( rc < 0 ) return rc;
	rc = inp_getb(in,&recd->health24,1);
	if ( rc < 0 ) return rc;
	rc = inp_getb(in,&recd->health25,1);
	if ( rc < 0 ) return rc;
	rc = inp_getb(in,&recd->health26,1);
	if ( rc < 0 ) return rc;
	rc = inp_getb(in,&recd->health27,1);
	if ( rc < 0 ) return rc;
	rc = inp_getb(in,&recd->health28,1);
	if ( rc < 0 ) return rc;
	rc = inp_getb(in,&recd->health29,1);
	if ( rc < 0 ) return rc;
	rc = inp_getb(in,&recd->health30,1);
	if ( rc < 0 ) return rc;
	rc = inp_getb(in,&recd->health31,1);
	if ( rc < 0 ) return rc;
	rc = inp_getb(in,&recd->health32,1);
	if ( rc < 0 ) return rc;
	return 0;
}

int16_t
decode_R4A(s_inpkt *in,s_R4A *recd) {
	rc = inp_getf32(in,&recd->latitude);
	if ( rc < 0 ) return rc;
	rc = inp_getf32(in,&recd->longitude);
	if ( rc < 0 ) return rc;
	rc = inp_getf32(in,&recd->altitude);
	if ( rc < 0 ) return rc;
	rc = inp_getf32(in,&recd->clock_bias);
	if ( rc < 0 ) return rc;
	if ( state_sd )
		rc = inp_getf32(in,&recd->u.time_of_fix1);
	else
		rc = inp_getf64(in,&recd->u.time_of_fix2);
	if ( rc < 0 ) return rc;
	return 0;
}

int16_t
decode_R4C(s_inpkt *in,s_R4C *recd) {
	rc = inp_getb(in,&recd->dynamics_code,1);
	if ( rc < 0 ) return rc;
	rc = inp_getf32(in,&recd->elevation_mask);
	if ( rc < 0 ) return rc;
	rc = inp_getf32(in,&recd->signal_level_mask);
	if ( rc < 0 ) return rc;
	rc = inp_getf32(in,&recd->pdop_mask);
	if ( rc < 0 ) return rc;
	rc = inp_getf32(in,&recd->podp_switch);
	if ( rc < 0 ) return rc;
	return 0;
}

int16_t
decode_R4D(s_inpkt *in,s_R4D *recd) {
	rc = inp_getf32(in,&recd->offset);
	if ( rc < 0 ) return rc;
	return 0;
}

int16_t
decode_R4E(s_inpkt *in,s_R4E *recd) {
	rc = inp_getb(in,&recd->yn,1);
	if ( rc < 0 ) return rc;
	return 0;
}

int16_t
decode_R4F(s_inpkt *in,s_R4F *recd) {
	rc = inp_getf64(in,&recd->a0);
	if ( rc < 0 ) return rc;
	rc = inp_getf32(in,&recd->a1);
	if ( rc < 0 ) return rc;
	rc = inp_geti16(in,&recd->delta_t_ls);
	if ( rc < 0 ) return rc;
	rc = inp_getf32(in,&recd->tot);
	if ( rc < 0 ) return rc;
	rc = inp_geti16(in,&recd->wn_t);
	if ( rc < 0 ) return rc;
	rc = inp_geti16(in,&recd->wn_lsf);
	if ( rc < 0 ) return rc;
	rc = inp_geti16(in,&recd->dn);
	if ( rc < 0 ) return rc;
	rc = inp_geti16(in,&recd->delta_t_lsf);
	if ( rc < 0 ) return rc;
	return 0;
}

int16_t
decode_R53(s_inpkt *in,s_R53 *recd) {
	rc = inp_getf32(in,&recd->receiver);
	if ( rc < 0 ) return rc;
	rc = inp_getf32(in,&recd->reserved1);
	if ( rc < 0 ) return rc;
	rc = inp_getf32(in,&recd->reserved2);
	if ( rc < 0 ) return rc;
	rc = inp_getf32(in,&recd->voltage);
	if ( rc < 0 ) return rc;
	rc = inp_getf32(in,&recd->supplyv);
	if ( rc < 0 ) return rc;
	rc = inp_getf32(in,&recd->Antenna);
	if ( rc < 0 ) return rc;
	rc = inp_getf32(in,&recd->pwr25);
	if ( rc < 0 ) return rc;
	rc = inp_getf32(in,&recd->pwr50);
	if ( rc < 0 ) return rc;
	return 0;
}

int16_t
decode_R54(s_inpkt *in,s_R54 *recd) {
	rc = inp_getf32(in,&recd->bias);
	if ( rc < 0 ) return rc;
	rc = inp_getf32(in,&recd->bias_rate);
	if ( rc < 0 ) return rc;
	if ( state_sd )
		rc = inp_getf32(in,&recd->u.time_of_fix1);
	else
		rc = inp_getf64(in,&recd->u.time_of_fix2);
	if ( rc < 0 ) return rc;
	return 0;
}

int16_t
decode_R55(s_inpkt *in,s_R55 *recd) {
	rc = inp_getb(in,&recd->position,1);
	if ( rc < 0 ) return rc;
	rc = inp_getb(in,&recd->velocity,1);
	if ( rc < 0 ) return rc;
	rc = inp_getb(in,&recd->timing,1);
	if ( rc < 0 ) return rc;
	rc = inp_getb(in,&recd->auxiliary,1);
	if ( rc < 0 ) return rc;
	return 0;
}

int16_t
decode_R56(s_inpkt *in,s_R56 *recd) {
	rc = inp_getf32(in,&recd->eastvel);
	if ( rc < 0 ) return rc;
	rc = inp_getf32(in,&recd->northvel);
	if ( rc < 0 ) return rc;
	rc = inp_getf32(in,&recd->upvel);
	if ( rc < 0 ) return rc;
	rc = inp_getf32(in,&recd->clock_bias_rate);
	if ( rc < 0 ) return rc;
	if ( state_sd )
		rc = inp_getf32(in,&recd->u.time_of_fix1);
	else
		rc = inp_getf64(in,&recd->u.time_of_fix2);
	if ( rc < 0 ) return rc;
	return 0;
}

int16_t
decode_R57(s_inpkt *in,s_R57 *recd) {
	rc = inp_getb(in,&recd->info_src,1);
	if ( rc < 0 ) return rc;
	rc = inp_getb(in,&recd->diag_code,1);
	if ( rc < 0 ) return rc;
	rc = inp_getf32(in,&recd->fix_time);
	if ( rc < 0 ) return rc;
	rc = inp_geti16(in,&recd->fix_week);
	if ( rc < 0 ) return rc;
	return 0;
}

int16_t
decode_R58(s_inpkt *in,s_R58 *recd) {
	rc = inp_getb(in,&recd->operation,1);
	if ( rc < 0 ) return rc;
	rc = inp_getb(in,&recd->datatype,1);
	if ( rc < 0 ) return rc;
	rc = inp_getb(in,&recd->sv_prn,1);
	if ( rc < 0 ) return rc;
	rc = inp_getb(in,&recd->n,1);
	if ( rc < 0 ) return rc;
	rc = inp_getb(in,&recd->satdata,249);
	if ( rc < 0 ) return rc;
	return 0;
}

int16_t
decode_R59(s_inpkt *in,s_R59 *recd) {
	rc = inp_getb(in,&recd->operation,1);
	if ( rc < 0 ) return rc;
	rc = inp_getb(in,&recd->sv_flags,32);
	if ( rc < 0 ) return rc;
	return 0;
}

int16_t
decode_R5A(s_inpkt *in,s_R5A *recd) {
	rc = inp_getb(in,&recd->sv_prn,1);
	if ( rc < 0 ) return rc;
	rc = inp_getf32(in,&recd->samplength);
	if ( rc < 0 ) return rc;
	rc = inp_getf32(in,&recd->siglevel);
	if ( rc < 0 ) return rc;
	rc = inp_getf32(in,&recd->code_phase);
	if ( rc < 0 ) return rc;
	rc = inp_getf32(in,&recd->doppler);
	if ( rc < 0 ) return rc;
	rc = inp_getf64(in,&recd->time);
	if ( rc < 0 ) return rc;
	return 0;
}


int16_t
decode_R5C(s_inpkt *in,s_R5C *recd) {
	rc = inp_getb(in,&recd->sv_prn,1);
	if ( rc < 0 ) return rc;
	rc = inp_getb(in,&recd->chan_slot,1);
	if ( rc < 0 ) return rc;
	rc = inp_getb(in,&recd->aquisflag,1);
	if ( rc < 0 ) return rc;
	rc = inp_getb(in,&recd->ephemflag,1);
	if ( rc < 0 ) return rc;
	rc = inp_getf32(in,&recd->siglevel);
	if ( rc < 0 ) return rc;
	rc = inp_getf32(in,&recd->gps_time);
	if ( rc < 0 ) return rc;
	rc = inp_getf32(in,&recd->elevation);
	if ( rc < 0 ) return rc;
	rc = inp_getf32(in,&recd->azimuth);
	if ( rc < 0 ) return rc;
	rc = inp_getb(in,&recd->oldmeas,1);
	if ( rc < 0 ) return rc;
	rc = inp_getb(in,&recd->intmsec,1);
	if ( rc < 0 ) return rc;
	rc = inp_getb(in,&recd->baddata,1);
	if ( rc < 0 ) return rc;
	rc = inp_getb(in,&recd->datacol,1);
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
decode_R83(s_inpkt *in,s_R83 *recd) {
	rc = inp_getf64(in,&recd->x);
	if ( rc < 0 ) return rc;
	rc = inp_getf64(in,&recd->y);
	if ( rc < 0 ) return rc;
	rc = inp_getf64(in,&recd->z);
	if ( rc < 0 ) return rc;
	rc = inp_getf64(in,&recd->clock_bias);
	if ( rc < 0 ) return rc;
	if ( state_sd )
		rc = inp_getf32(in,&recd->u.time_of_fix1);
	else
		rc = inp_getf64(in,&recd->u.time_of_fix2);
	if ( rc < 0 ) return rc;
	return 0;
}

int16_t
decode_R84(s_inpkt *in,s_R84 *recd) {
	rc = inp_getf64(in,&recd->latitude);
	if ( rc < 0 ) return rc;
	rc = inp_getf64(in,&recd->longitude);
	if ( rc < 0 ) return rc;
	rc = inp_getf64(in,&recd->altitude);
	if ( rc < 0 ) return rc;
	rc = inp_getf64(in,&recd->clock_bias);
	if ( rc < 0 ) return rc;
	if ( state_sd )
		rc = inp_getf32(in,&recd->u.time_of_fix1);
	else
		rc = inp_getf64(in,&recd->u.time_of_fix2);
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
#endif

// End tsip.cpp
