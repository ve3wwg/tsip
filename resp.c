/* resp.c
 * Warren W. Gay VE3WWG		Sun Aug  4 22:59:40 2013
 *
 * Parse TSIP responses
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <assert.h>

#include "tsip.h"

int32_t
response(s_inpkt *in) {
	uint8_t id;
	uint8_t subid;
	uint16_t rc;
	ReportID rpt_id = None;

	rc = inp_getbytes(in,&id,1);
	if ( rc )
		return rc;

	switch ( (ReportID)id ) {
	case R1A :
	case R6A :
	case R6E :
	case R7B :
	case R7D :
	case R87 :
	case R88 :
	case R8B :
	case R8D :
	case R8F :
	case RB0 :
	case RBB :
		rc = inp_getbytes(in,&subid,1);
		if ( rc )
			return rc;
		rpt_id = (ReportID) ((int)id << 8) | (int)subid;
		break;
	case None :
	case R13 :
	case R3D :
	case R40 :
	case R41 :
	case R42 :
	case R43 :
	case R44 :
	case R45 :
	case R46 :
	case R47 :
	case R48 :
	case R49 :
	case R4A :
	case R4B :
	case R4C :
	case R4D :
	case R4E :
	case R4F :
	case R53 :
	case R54 :
	case R55 :
	case R56 :
	case R57 :
	case R58 :
	case R59 :
	case R5A :
	case R5B :
	case R5C :
	case R60 :
	case R61 :
	case R6D :
	case R70 :
	case R76 :
	case R78 :
	case R82 :
	case R83 :
	case R84 :
	case R85 :
	case RBC :
	default :
		rpt_id = (ReportID) id;
	}

	if ( rpt_id == None )
		return rpt_id;

	switch ( rpt_id ) {
	case R13 :
		rc = decode_R13(in,u.resp_R13);
		break;
	case R1A00 :
		rc = decode_R1A00(in,u.resp_R1A00);
		break;
	case R3D :
		rc = decode_R3D(in,u.resp_R3D);
		break;
	case R40 :
		rc = decode_R40(in,u.resp_R40);
		break;
	case R41 :
		rc = decode_R41(in,u.resp_R41);
		break;
	case R42 :
		rc = decode_R42(in,u.resp_R42);
		break;
	case R43 :
		rc = decode_R43(in,u.resp_R43);
		break;
	case R44 :
		rc = decode_R44(in,u.resp_R44);
		break;
	case R45 :
		rc = decode_R45(in,u.resp_R45);
		break;
	case R46 :
		rc = decode_R46(in,u.resp_R46);
		break;
	case R47 :
		rc = decode_R47(in,u.resp_R47);
		break;
	case R48 :
		rc = decode_R48(in,u.resp_R48);
		break;
	case R49 :
		rc = decode_R49(in,u.resp_R49);
		break;
	case R4A :
		rc = decode_R4A(in,u.resp_R4A);
		break;
	case R4B :
		rc = decode_R4B(in,u.resp_R4B);
		break;
	case R4C :
		rc = decode_R4C(in,u.resp_R4C);
		break;
	case R4D :
		rc = decode_R4D(in,u.resp_R4D);
		break;
	case R4E :
		rc = decode_R4E(in,u.resp_R4E);
		break;
	case R4F :
		rc = decode_R4F(in,u.resp_R4F);
		break;
	case R53 :
		rc = decode_R53(in,u.resp_R53);
		break;
	case R54 :
		rc = decode_R54(in,u.resp_R54);
		break;
	case R55 :
		rc = decode_R55(in,u.resp_R55);
		break;
	case R56 :
		rc = decode_R56(in,u.resp_R56);
		break;
	case R57 :
		rc = decode_R57(in,u.resp_R57);
		break;
	case R58 :
		rc = decode_R58(in,u.resp_R58);
		break;
	case R59 :
		rc = decode_R59(in,u.resp_R59);
		break;
	case R5A :
		rc = decode_R5A(in,u.resp_R5A);
		break;
	case R5B :
		rc = decode_R5B(in,u.resp_R5B);
		break;
	case R5C :
		rc = decode_R5C(in,u.resp_R5C);
		break;
	case R60 :
		rc = decode_R60(in,u.resp_R60);
		break;
	case R61 :
		rc = decode_R61(in,u.resp_R61);
		break;
	case R6A00 :
		rc = decode_R6A00(in,u.resp_R6A00);
		break;
	case R6A01 :
		rc = decode_R6A01(in,u.resp_R6A01);
		break;
	case R6D :
		rc = decode_R6D(in,u.resp_R6D);
		break;
	case R6E01 :
		rc = decode_R6E01(in,u.resp_R6E01);
		break;
	case R70 :
		rc = decode_R70(in,u.resp_R70);
		break;
	case R76 :
		rc = decode_R76(in,u.resp_R76);
		break;
	case R78 :
		rc = decode_R78(in,u.resp_R78);
		break;
	case R7B00 :
		rc = decode_R7B00(in,u.resp_R7B00);
		break;
	case R7B04 :
		rc = decode_R7B04(in,u.resp_R7B04);
		break;
	case R7B05 :
		rc = decode_R7B05(in,u.resp_R7B05);
		break;
	case R7B06 :
		rc = decode_R7B06(in,u.resp_R7B06);
		break;
	case R7B80 :
		rc = decode_R7B80(in,u.resp_R7B80);
		break;
	case R7B84 :
		rc = decode_R7B84(in,u.resp_R7B84);
		break;
	case R7B85 :
		rc = decode_R7B85(in,u.resp_R7B85);
		break;
	case R7B86 :
		rc = decode_R7B86(in,u.resp_R7B86);
		break;
	case R7D00 :
		rc = decode_R7D00(in,u.resp_R7D00);
		break;
	case R7D01 :
		rc = decode_R7D01(in,u.resp_R7D01);
		break;
	case R7D02 :
		rc = decode_R7D02(in,u.resp_R7D02);
		break;
	case R7D03 :
		rc = decode_R7D03(in,u.resp_R7D03);
		break;
	case R7D05 :
		rc = decode_R7D05(in,u.resp_R7D05);
		break;
	case R7D06 :
		rc = decode_R7D06(in,u.resp_R7D06);
		break;
	case R7D09 :
		rc = decode_R7D09(in,u.resp_R7D09);
		break;
	case R7D7F :
		rc = decode_R7D7F(in,u.resp_R7D7F);
		break;
	case R82 :
		rc = decode_R82(in,u.resp_R82);
		break;
	case R83 :
		rc = decode_R83(in,u.resp_R83);
		break;
	case R84 :
		rc = decode_R84(in,u.resp_R84);
		break;
	case R85 :
		rc = decode_R85(in,u.resp_R85);
		break;
	case R8700 :
		rc = decode_R8700(in,u.resp_R8700);
		break;
	case R8701 :
		rc = decode_R8701(in,u.resp_R8701);
		break;
	case R8702 :
		rc = decode_R8702(in,u.resp_R8702);
		break;
	case R8703 :
		rc = decode_R8703(in,u.resp_R8703);
		break;
	case R8704 :
		rc = decode_R8704(in,u.resp_R8704);
		break;
	case R8705 :
		rc = decode_R8705(in,u.resp_R8705);
		break;
	case R8706 :
		rc = decode_R8706(in,u.resp_R8706);
		break;
	case R8708 :
		rc = decode_R8708(in,u.resp_R8708);
		break;
	case R8709 :
		rc = decode_R8709(in,u.resp_R8709);
		break;
	case R870A :
		rc = decode_R870A(in,u.resp_R870A);
		break;
	case R877D :
		rc = decode_R877D(in,u.resp_R877D);
		break;
	case R877E :
		rc = decode_R877E(in,u.resp_R877E);
		break;
	case R877F :
		rc = decode_R877F(in,u.resp_R877F);
		break;
	case R8800 :
		rc = decode_R8800(in,u.resp_R8800);
		break;
	case R8801 :
		rc = decode_R8801(in,u.resp_R8801);
		break;
	case R8802 :
		rc = decode_R8802(in,u.resp_R8802);
		break;
	case R8803 :
		rc = decode_R8803(in,u.resp_R8803);
		break;
	case R8804 :
		rc = decode_R8804(in,u.resp_R8804);
		break;
	case R8808 :
		rc = decode_R8808(in,u.resp_R8808);
		break;
	case R887F :
		rc = decode_R887F(in,u.resp_R887F);
		break;
	case R8B00 :
		rc = decode_R8B00(in,u.resp_R8B00);
		break;
	case R8B01 :
		rc = decode_R8B01(in,u.resp_R8B01);
		break;
	case R8B02 :
		rc = decode_R8B02(in,u.resp_R8B02);
		break;
	case R8B03 :
		rc = decode_R8B03(in,u.resp_R8B03);
		break;
	case R8D00 :
		rc = decode_R8D00(in,u.resp_R8D00);
		break;
	case R8D01 :
		rc = decode_R8D01(in,u.resp_R8D01);
		break;
	case R8D02 :
		rc = decode_R8D02(in,u.resp_R8D02);
		break;
	case R8D03 :
		rc = decode_R8D03(in,u.resp_R8D03);
		break;
	case R8D04 :
		rc = decode_R8D04(in,u.resp_R8D04);
		break;
	case R8F20 :
		rc = decode_R8F20(in,u.resp_R8F20);
		break;
	case R8F60 :
		rc = decode_R8F60(in,u.resp_R8F60);
		break;
	case R8F62 :
		rc = decode_R8F62(in,u.resp_R8F62);
		break;
	case R8F64 :
		rc = decode_R8F64(in,u.resp_R8F64);
		break;
	case R8F6B :
		rc = decode_R8F6B(in,u.resp_R8F6B);
		break;
	case R8F6D :
		rc = decode_R8F6D(in,u.resp_R8F6D);
		break;
	case R8F6F :
		rc = decode_R8F6F(in,u.resp_R8F6F);
		break;
	case R8F70 :
		rc = decode_R8F70(in,u.resp_R8F70);
		break;
	case R8F71 :
		rc = decode_R8F71(in,u.resp_R8F71);
		break;
	case R8F72 :
		rc = decode_R8F72(in,u.resp_R8F72);
		break;
	case R8F73 :
		rc = decode_R8F73(in,u.resp_R8F73);
		break;
	case R8F74 :
		rc = decode_R8F74(in,u.resp_R8F74);
		break;
	case R8F75 :
		rc = decode_R8F75(in,u.resp_R8F75);
		break;
	case R8F76 :
		rc = decode_R8F76(in,u.resp_R8F76);
		break;
	case R8F77 :
		rc = decode_R8F77(in,u.resp_R8F77);
		break;
	case R8F78 :
		rc = decode_R8F78(in,u.resp_R8F78);
		break;
	case R8F79 :
		rc = decode_R8F79(in,u.resp_R8F79);
		break;
	case R8F7A :
		rc = decode_R8F7A(in,u.resp_R8F7A);
		break;
	case R8F7B :
		rc = decode_R8F7B(in,u.resp_R8F7B);
		break;
	case R8F7C :
		rc = decode_R8F7C(in,u.resp_R8F7C);
		break;
	case R8F7D :
		rc = decode_R8F7D(in,u.resp_R8F7D);
		break;
	case R8F7E :
		rc = decode_R8F7E(in,u.resp_R8F7E);
		break;
	case R8F7F :
		rc = decode_R8F7F(in,u.resp_R8F7F);
		break;
	case R8F80 :
		rc = decode_R8F80(in,u.resp_R8F80);
		break;
	case R8F81 :
		rc = decode_R8F81(in,u.resp_R8F81);
		break;
	case R8F82 :
		rc = decode_R8F82(in,u.resp_R8F82);
		break;
	case R8F84 :
		rc = decode_R8F84(in,u.resp_R8F84);
		break;
	case R8F85 :
		rc = decode_R8F85(in,u.resp_R8F85);
		break;
	case R8F86 :
		rc = decode_R8F86(in,u.resp_R8F86);
		break;
	case R8F87 :
		rc = decode_R8F87(in,u.resp_R8F87);
		break;
	case R8F88 :
		rc = decode_R8F88(in,u.resp_R8F88);
		break;
	case R8F89 :
		rc = decode_R8F89(in,u.resp_R8F89);
		break;
	case R8F8A :
		rc = decode_R8F8A(in,u.resp_R8F8A);
		break;
	case R8F8B :
		rc = decode_R8F8B(in,u.resp_R8F8B);
		break;
	case R8F8E :
		rc = decode_R8F8E(in,u.resp_R8F8E);
		break;
	case R8F8F :
		rc = decode_R8F8F(in,u.resp_R8F8F);
		break;
	case R8F90 :
		rc = decode_R8F90(in,u.resp_R8F90);
		break;
	case R8F91 :
		rc = decode_R8F91(in,u.resp_R8F91);
		break;
	case R8F92 :
		rc = decode_R8F92(in,u.resp_R8F92);
		break;
	case R8F94 :
		rc = decode_R8F94(in,u.resp_R8F94);
		break;
	case R8F95 :
		rc = decode_R8F95(in,u.resp_R8F95);
		break;
	case R8F96 :
		rc = decode_R8F96(in,u.resp_R8F96);
		break;
	case R8F97 :
		rc = decode_R8F97(in,u.resp_R8F97);
		break;
	case R8F98 :
		rc = decode_R8F98(in,u.resp_R8F98);
		break;
	case R8F9A :
		rc = decode_R8F9A(in,u.resp_R8F9A);
		break;
	case RB080 :
		rc = decode_RB080(in,u.resp_RB080);
		break;
	case RB081 :
		rc = decode_RB081(in,u.resp_RB081);
		break;
	case RB082 :
		rc = decode_RB082(in,u.resp_RB082);
		break;
	case RB0C0 :
		rc = decode_RB0C0(in,u.resp_RB0C0);
		break;
	case RB0C1 :
		rc = decode_RB0C1(in,u.resp_RB0C1);
		break;
	case RB0C2 :
		rc = decode_RB0C2(in,u.resp_RB0C2);
		break;
	case RB0C3 :
		rc = decode_RB0C3(in,u.resp_RB0C3);
		break;
	case RB0C4 :
		rc = decode_RB0C4(in,u.resp_RB0C4);
		break;
	case RBB00 :
		rc = decode_RBB00(in,u.resp_RBB00);
		break;
	case RBC :
		rc = decode_RBC(in,u.resp_RBC);
		break;

	case None :
		assert(0);

	/* These have subids */
	case R1A :
	case R6A :
	case R6E :
	case R7B :
	case R7D :
	case R87 :
	case R88 :
	case R8B :
	case R8D :
	case R8F :
	case RB0 :
	case RBB :
		assert(0);	/* Should not get here */
	}

	return rpt_id;
}	
