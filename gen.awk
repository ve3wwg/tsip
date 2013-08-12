BEGIN	{
		sizes["uint8_t"] = 1
		sizes["int16_t"] = 2
		sizes["int32_t"] = 4
		sizes["float"] = 4
		sizes["double"] = 8

		subrt["uint8_t"] = "inp_getb"
		subrt["int16_t"] = "inp_geti16"
		subrt["int32_t"] = "inp_geti32"
		subrt["float"] = "inp_getf32"
		subrt["double"] = "inp_getf64"

		putrt["uint8_t"] = "out_putb"
		putrt["int16_t"] = "out_puti16"
		putrt["int32_t"] = "out_puti32"
		putrt["float"] = "out_putf32"
		putrt["double"] = "out_putf64"

		Fieldx = -1
		Next_Byte = 0

		struct = "rstruct.h"
		print "/* Response structures */" >struct
		subr="decode.c"
		print "/* Decode subroutines */" >subr

		rgen = "rgen.c"
		print "/* Generate responses */" >rgen
		print "#include <stdio.h>" >>rgen
		print "#include <stdlib.h>" >>rgen
		print "#include \"tsip.h\"" >>rgen
		print "#include \"rstruct.h\"\n" >>rgen
		print "static FILE *data = 0;" >>rgen
		print "static FILE *opkt = 0;\n" >>rgen
		print "static int state_sd = 0;" >>rgen
		print "static uint8_t buf[512];" >>rgen
		print "static s_outpkt octl;\n" >>rgen
		print "void rgen_init() {" >>rgen
		print "\tdata = fopen(\"rtest.dat\",\"w\");" >>rgen
		print "\topkt = fopen(\"rpkt.dat\",\"w\");" >>rgen
		print "}\n" >>rgen

		print "static void oinit() {" >>rgen
		print "\tout_init(&octl,buf,sizeof buf);" >>rgen
		print "}\n" >>rgen

		rchk = "rchk.c"
		print "/* Generated check responses pgm */" >rchk
		print "#include <stdio.h>" >>rchk
		print "#include <stdlib.h>" >>rchk
		print "#include <assert.h>" >>rchk
		print "#include \"tsip.h\"" >>rchk
		print "#include \"rstruct.h\"\n" >>rchk
		print "static FILE *data = 0;" >>rchk
		print "static FILE *ipkt = 0;\n" >>rchk
		print "static int state_sd = 0;" >>rchk
		print "static uint8_t buf[512];" >>rchk
		print "static s_inpkt ictl;\n" >>rchk
		print "void rchk_init() {" >>rchk
		print "\tdata = fopen(\"rtest.dat\",\"r\");" >>rchk
		print "\tipkt = fopen(\"rpkt.dat\",\"r\");" >>rchk
		print "}\n" >>rchk

		print "static void iinit() {" >>rchk
		print "\tuint8_t b;\n" >>rchk
		print "\tinp_init(&ictl,buf,sizeof buf);" >>rchk
		print "\tdo\t{" >>rchk
		print "\t\tif ( !fread(&b,1,1,ipkt) )" >>rchk
		print "\t\t\texit(3);" >>rchk
		print "\t} while ( !inp_addb(&ictl,b) );" >>rchk
		print "}\n" >>rchk

		MAXF = 64
		subrx = 1
		subrs[subrx] = ""
}
$1 == "R" {
	if ( Fieldx >= 0 ) {
		if ( Subid == "-" )
			print "/* Response " Id " :" Title " */" >>struct
		else	print "/* Response " Id " " Subid " :" Title " */" >>struct

		if ( Subid == "-" ) {
			typedef = "s_R" Id
			subr_name = "decode_R" Id
		} else	{
			typedef = "s_R" Id Subid
			subr_name = "decode_R" Id Subid
		}
		
		subrs[subrx] = subr_name;
		++subrx

		print "int16_t" >>subr
		print subr_name "(s_inpkt *in," typedef " *recd) {" >>subr

		print "void\nrgen_" subr_name "() {" >>rgen
		print "\t" typedef " r;" >>rgen
		print "\tint x;\n" >>rgen
		print "\t(void)x;\n" >>rgen
		print "\toinit();\n" >>rgen

		print "void\nrchk_" subr_name "() {" >>rchk
		print "\t" typedef " r, v;" >>rchk
		print "\tint x, rc;\n" >>rchk
		print "\t(void)x;\n" >>rchk
		print "\tiinit();\n" >>rchk
		print "\tfread(&v,sizeof v,1,data);\n" >>rchk

		print "typedef struct {" >>struct
#		print "\tuint8_t\tid;"
#		if ( Subid != "-" )
#			print "\tuint8_t\tsubid;"
	
		for ( x=0; x<MAXF; ++x ) {
			n = split(Fields[x],a,"|")
			if ( !n ) break;
			Byte = a[1]
			if ( Byte == Byte+0 ) {
				dim = ""
			} else	{
				n = split(Byte,b,"-")
				Byte = b[1]
				ToByte = b[2]
				if ( ToByte == "n" )
					ToByte = 256 - Byte
				dim = ToByte - Byte + 1
			}
			Datatype = a[2]
			n = split(Datatype,c,":")
			if ( n > 1 ) {
				for ( y=1; y<n; ++y ) {
					if ( c[y] == "single" )
						c[y] = "float"
					else	c[y] = "double"
					Datatypes[y] = c[y]
				}
				state = c[n]
				union = n-1
			} else	{
				union = 0
			}

			Name = a[3]
			Desc = a[4]
#			print "Byte=" Byte ", Datatype=" Datatype ", Name=" Name ", " Desc
			if ( Desc != "" )
				comment = "\t/* " Desc " */"
			else	comment = ""
			if ( !union ) {
				if ( dim == "" ) {
					print "\t" Datatype "\t" Name ";" comment >>struct
					if ( Datatype == "uint8_t" ) {
						print "\trc = " subrt[Datatype] "(in,&recd->" Name ",1);" >>subr
						print "\tr." Name " = rand() % 256;" >>rgen
						print "\t" putrt[Datatype] "(&octl,&r." Name ",1);" >>rgen
						print "\trc = " subrt[Datatype] "(&ictl,&r." Name ",1);" >>rchk
						print "\tassert(!rc);" >>rchk
						print "\tassert(r." Name " == v." Name ");" >>rchk
					} else	{
						print "\trc = " subrt[Datatype] "(in,&recd->" Name ");" >>subr
						print "\tr." Name " = rand() % 32768;" >>rgen
						print "\t" putrt[Datatype] "(&octl,r." Name ");" >>rgen
						print "\trc = " subrt[Datatype] "(&ictl,&r." Name ");" >>rchk
						print "\tassert(!rc);" >>rchk
						print "\tassert(r." Name " == v." Name ");" >>rchk
					}
				} else	{
					print "\t" Datatype "\t" Name "[" dim "];" comment >>struct
					print "\trc = " subrt[Datatype] "(in,&recd->" Name "," dim ");" >>subr
					print "\tfor ( x=0; x<" dim "; ++x )" >>rgen
					if ( Datatype == "uint8_t" )
						M = 256;
					else	M = 32768;

					print "\t\tr." Name "[x] = rand() % " M ";" >>rgen
					print "\tout_putb(&octl,r." Name "," dim ");" >>rgen
					print "\trc = inp_getb(&ictl,r." Name "," dim ");" >>rchk
					print "\tassert(!rc);" >>rchk
					print "\tfor ( x=0; x<" dim "; ++x )" >>rchk
					print "\t\tassert(r." Name "[x] == v." Name "[x]);;" >>rchk
				}
			} else	{
				print "\tunion {\t" comment >>struct
				print "\tif ( " state " )" >>subr
				print "\tif ( " state " ) {" >>rgen
				print "\tif ( " state " ) {" >>rchk
				for ( y=1; y <= union; ++y ) {
					print "\t  " Datatypes[y] "\t" Name y ";" >>struct
					if ( y > 1 ) {
						print "\telse" >>subr
						print "\telse {" >>rgen
						print "\telse {" >>rchk
					}
					print "\t\trc = " subrt[Datatypes[y]] "(in,&recd->u." Name y ");" >>subr
					print "\t\tr.u." Name y " = rand() % 32768;" >>rgen
					print "\t\t" putrt[Datatypes[y]] "(&octl,r.u." Name y ");" >>rgen
					print "\t\trc = " subrt[Datatypes[y]] "(&ictl,&r.u." Name y ");" >>rchk
					print "\t\tassert(!rc);" >>rchk
					print "\t\tassert(r.u." Name y " == v.u." Name y ");" >>rchk
					print "\t}" >>rgen
					print "\t}" >>rchk
				}
				print "\t}\tu;" >>struct
			}
			print "\tif ( rc < 0 ) return rc;" >>subr
		}
		print "} " typedef ";\n" >>struct

		print "\treturn 0;" >>subr
		print "}\n" >>subr

		print "\tfwrite(&r,sizeof r,1,data);" >>rgen
		print "\tout_close(&octl);" >>rgen
		print "\tx = out_length(&octl);" >>rgen
		print "\tfwrite(buf,x,1,opkt);" >>rgen
		print "}\n" >>rgen

		print "}\n" >>rchk
	}

	for ( x=0; x<MAXF; ++x ) {
		if ( Fields[x] != "" ) {
			Fields[x] = ""
		}
	}

	Type = "R"
	Id = toupper($2)
	Subid = toupper($3)
	Title = ""
	for ( x=4; x<NF; ++x ) {
		Title = Title " " $x
	}
	Fieldx = 0
	if ( Subid == "-" )
		Next_Byte = 0
	else	Next_Byte = 1
}
$1 == ":" {
	Byte = $2
	Datatype = $3
	if ( Datatype == "byte" )
		Datatype = "uint8_t"
	else if ( Datatype == "integer" )
		Datatype = "int16_t"
	else if ( Datatype == "long" )
		Datatype = "int32_t"
	else if ( Datatype == "single" )
		Datatype = "float"
	else if ( Datatype == "double" )
		Datatype = "double"

	Name = $4
	Desc = ""
	for ( x=5; x<=NF; ++x ) {
		Desc = Desc " " $x
	}
	Fields[Fieldx] = Byte "|" Datatype "|" Name "|" Desc

	if ( Byte+0 != Byte ) {
		n = split(Byte,a,"-")
		Byte = a[1]
		ToByte = a[2]
		M = ToByte - Byte + 1
	} else	M = 1

	if ( Byte != Next_Byte ) {
		printf("Line %04d error: Expected byte %d, but got %s instead\n",FNR,Next_Byte,Byte)
		exit(1);
	}

	Fieldx = Fieldx + 1

	Next_Byte = Byte + M * sizes[Datatype]
}
END	{
	print "\nint main(int argc,char **argv) {\n" >>rgen
	print "\trgen_init();" >>rgen
	for ( x=1; subrs[x] != ""; ++x ) {
		print "\trgen_" subrs[x] "();" >>rgen
	}
	print "\tfclose(opkt);" >>rgen
	print "\tfclose(data);" >>rgen
	print "\tputs(\"Data generated.\");" >>rgen
	print "\treturn 0;\n}" >>rgen

	print "\nint main(int argc,char **argv) {\n" >>rchk
	print "\trchk_init();" >>rchk
	for ( x=1; subrs[x] != ""; ++x ) {
		print "\trchk_" subrs[x] "();" >>rchk
	}
	print "\tfclose(ipkt);" >>rchk
	print "\tfclose(data);" >>rchk
	print "\tputs(\"Rdata ok!\");" >>rchk
	print "\treturn 0;\n}" >>rchk
}

