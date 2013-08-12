#include <stdio.h>
#include <assert.h>

#include "tsip.h"

int
main(int argc,char **argv) {
	FILE *out = fopen("tsip.dat","w");
	FILE *in = 0;
	s_outpkt outctl;
	s_inpkt inctl;
	uint8_t buf[256], b;
	uint16_t length;
	int16_t rc;
	int r;

	assert(out);
	out_init(&outctl,buf,sizeof buf);

	rc = out_putb(&outctl,(uint8_t *)"(First Packet test)",19);
	assert(!rc);

	rc = out_puti16(&outctl,(int16_t)0x1234);
	assert(!rc);

	rc = out_puti32(&outctl,0x56789ABC);
	assert(!rc);

	rc = out_putf32(&outctl,23.008);
	assert(!rc);

	rc = out_putf64(&outctl,95.001);
	assert(!rc);

	rc = out_checksum(&outctl);
	assert(!rc);

	rc = out_close(&outctl);
	assert(!rc);

	length = out_length(&outctl);
	fwrite(buf,length,1,out);
	fclose(out);
	out = 0;

	in = fopen("tsip.dat","r");
	assert(in);

	inp_init(&inctl,buf,sizeof buf);

	do	{
		r = fread(&b,1,1,in);
		assert(r >= 0);
		if ( !r ) {
			rc = 0;
			puts("<EOF>");
			break;
		}
		rc = inp_addb(&inctl,b);
		if ( rc < 0 )
			printf("Error: %d\n",rc);
	} while ( rc <= 0 );

	fclose(in);

	if ( rc > 0 ) {
		char temp[20];
		int16_t i16;
		int32_t i32;
		float f;
		double d;

		printf("Packet length is %u bytes.\n",(unsigned)rc);
		printf("Checksum: %s\n",inp_checksum(&inctl) ? "matched" : "failed");

		rc = inp_getb(&inctl,(uint8_t *)temp,19);
		assert(!rc);

		temp[20] = 0;
		printf("Got string '%s'\n",temp);
	
		rc = inp_geti16(&inctl,&i16);
		assert(!rc);	
		printf("Got i16 = 0x%04X\n",i16);

		rc = inp_geti32(&inctl,&i32);
		assert(!rc);	
		printf("Got i32 = 0x%08X\n",i32);

		rc = inp_getf32(&inctl,&f);
		assert(!rc);	
		printf("Got f   = %f\n",f);

		rc = inp_getf64(&inctl,&d);
		assert(!rc);	
		printf("Got d   = %lf\n",d);
	}

	return 0;
}
