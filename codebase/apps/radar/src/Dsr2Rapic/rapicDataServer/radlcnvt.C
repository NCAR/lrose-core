#ifdef RAPICTXMODS
#include "..\stdafx.h"   // helps speed compile through pre-compiled headers
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif
#endif

/*


radl_cnvt.c


*/

#include "rdr.h"
#include "rdrscan.h"
#include <ctype.h>
#include <string.h>
#include "radlcnvt.h"
#include "rdrutils.h"
#include <iostream>
#include <sstream>

#ifdef _DEBUG
void CheckBinaryIntegrity(unsigned char LastByte, unsigned char ThisByte, float Az, float El, e_data_type data_type);
#endif

// lookup table used for decoding delta ASCII radials
// the 0xdddd value is used to designate digit values used for RLE purposes
// delta pairs are encoded with the MSB (bit 15) set, otherwise value is a 
// direct absolute level.

unsigned short XLAT_16L[256] = {

  /*00*/	 0xff,   0xff,   0xff,   0xff,   0xff,   0xff,   0xff,   0xff,
  /*08*/	 0xff,   0xff,   0xff,   0xff,   0xff,   0xff,   0xff,   0xff,
  /*10*/	 0xff,   0xff,   0xff,   0xff,   0xff,   0xff,   0xff,   0xff,
  /*18*/	 0xff,   0xff,   0xff,   0xff,   0xff,   0xff,   0xff,   0xff,
  /*20*/	 0xff,   0xfdfd, 0x0010, 0xff,   0xfd03, 0xff,   0x8303, 0x0011,
  /*28*/	 0xfd02, 0x8302, 0x0012, 0x8100, 0x0013, 0xff00, 0x8000, 0xfdfe,
  /*30*/	 0x0ddd, 0x0ddd, 0x0ddd, 0x0ddd, 0x0ddd, 0x0ddd, 0x0ddd, 0x0ddd,
  /*38*/	 0x0ddd, 0x0ddd, 0x0014, 0x0015, 0x80ff, 0x0016, 0x8001, 0x0017,
  /*40*/	 0x83fd, 0x0000, 0x0001, 0x0002, 0x0003, 0x0004, 0x0005, 0x0006,
  /*48*/	 0x0007, 0x0008, 0x0009, 0x000A, 0x000B, 0x000C, 0x000D, 0x000E,
  /*50*/	 0x000F, 0x0018, 0x0019, 0xff02, 0x8002, 0x8102, 0x8202, 0xff03,
  /*58*/	 0x8003, 0x8103, 0x001a, 0xfefd, 0x83fe, 0x82fd, 0x001b, 0x001c,
  /*60*/	 0xff,   0xfffd, 0x80fd, 0x81fd, 0xfefe, 0xfffe, 0x80fe, 0x81fe,
  /*68*/	 0x82fe, 0xfdff, 0xfeff, 0xffff, 0x81ff, 0x82ff, 0x83ff, 0xfd00,
  /*70*/	 0xfe00, 0x8200, 0x8300, 0xfd01, 0xfe01, 0xff01, 0x8101, 0x8201,
  /*78*/	 0x8301, 0xfe02, 0x001d, 0xfe03, 0x001e, 0x8203, 0x001f, 0xff,
  /*80*/	 0x0020, 0x0021, 0x0022, 0x0023, 0x0024, 0x0025, 0x0026, 0x0027, 
  /*88*/	 0x0028, 0x0029, 0x002a, 0x002b, 0x002c, 0x002d, 0x002e, 0x002f, 
  /*90*/	 0x0030, 0x0031, 0x0032, 0x0033, 0x0034, 0x0035, 0x0036, 0x0037, 
  /*98*/	 0x0038, 0x0039, 0x003a, 0x003b, 0x003c, 0x003d, 0x003e, 0x003f, 
  /*a0*/	 0x0040, 0x0041, 0x0042, 0x0043, 0x0044, 0x0045, 0x0046, 0x0047, 
  /*a8*/	 0x0048, 0x0049, 0x004a, 0x004b, 0x004c, 0x004d, 0x004e, 0x004f, 
  /*b0*/	 0x0050, 0x0051, 0x0052, 0x0053, 0x0054, 0x0055, 0x0056, 0x0057, 
  /*b8*/	 0x0058, 0x0059, 0x005a, 0x005b, 0x005c, 0x005d, 0x005e, 0x005f, 
  /*c0*/	 0x0060, 0x0061, 0x0062, 0x0063, 0x0064, 0x0065, 0x0066, 0x0067, 
  /*c8*/	 0x0068, 0x0069, 0x006a, 0x006b, 0x006c, 0x006d, 0x006e, 0x006f, 
  /*d0*/	 0x0070, 0x0071, 0x0072, 0x0073, 0x0074, 0x0075, 0x0076, 0x0077, 
  /*d8*/	 0x0078, 0x0079, 0x007a, 0x007b, 0x007c, 0x007d, 0x007e, 0x007f, 
  /*e0*/	 0x0080, 0x0081, 0x0082, 0x0083, 0x0084, 0x0085, 0x0086, 0x0087, 
  /*e8*/	 0x0088, 0x0089, 0x008a, 0x008b, 0x008c, 0x008d, 0x008e, 0x008f, 
  /*f0*/	 0x0090, 0x0091, 0x0092, 0x0093, 0x0094, 0x0095, 0x0096, 0x0097, 
  /*f8*/	 0x0098, 0x0099, 0x009a, 0x009b, 0x009c, 0x009d, 0x009e, 0x009f 
};

char TestRadial[] = 
"%262B+13+5+7+6+7+5+7+8+6+6+6+5+7+15dejk2-77ll6l4l5l2l5%263B+12+7+6+6+7+6+7+6+6+7+6+6+6+15dejk2-85+-5l15%264B+13+5+7+7+6+6+7+5+9+4+7+5+6+16djjk1-99l9l%265B+13+5+7+6+7+7+5+6l+5+7+5+7+5+16djjp2-78l8l19l"
;

void dump_radl(s_radl* radl) {
  int	x;
  char	c;
  for (x=0; x<radl->data_size; x++) {
    c = radl->data[x];
    if (c<=9) putchar(c+'0');
    else
      if ((c>=10) && (c<=15)) putchar(c-10+'a');
      else putchar('*');
  }
  putchar('\n');
}

// read angle of radial, either %ddd or %ddd.t
// returns no of chars used
int rd_radl_angle(char* instring,rdr_angle& angle) {
  int	degrees = 0;
  int tenths = 0;
  int	n = 0;
  int args = 0;
  float el;
  int tmofs;

  angle = 0;
  //  if (sscanf(instring,"%%%d%n.%d%n",&degrees,&n,&tenths,&n) > 0)
  //    angle = (degrees * 10) + tenths;
  if ((args = sscanf(instring,"%*c%d%n.%d%n,%f,%d=%n",&degrees,&n,&tenths,&n, &el, &tmofs, &n)) > 0)
    angle = (degrees * 10) + tenths;
  return n;
}

bool null_radl(char* instring) {
  rdr_angle	angle;

  if (instring[rd_radl_angle(instring,angle)] == 0)
    return true;
  else 
    return false;
}

/*
 * Decode an ASCII string into an sradl structure
 */

int RLE_16L_radl(char* instring, int ip_length, s_radl* radl, int maxval) {
  short  temp;
  short  first;
  short  second;
  uchar ch = 0;
  int  runcount;
  uchar  last = 0;
  int  n;
  uchar* ipbuffer;
  bool done = false;
  //  char 	anglbuff[4];
  bool debug = false;
    
  //memcpy(anglbuff,ipbuffer,4);
  //ipbuffer = TestRadial;
  //memcpy(ipbuffer,anglbuff,4);
  ipbuffer = (uchar*)instring;
  if ((n = rd_radl_angle((char*)ipbuffer,radl->az)) > 0)
    ipbuffer += n;
  else 
    return -1;					// bad angle, fail convert
  radl->data_size = 0;				// init size
  do {
    ch = *ipbuffer;
    ipbuffer++;
    temp = XLAT_16L[ch];
    if (temp == 0xff)
      break;
    if(temp == 0x0ddd) {				// process runcount - 0xdddd code used to identify digits
      runcount = ch - '0';
      while(XLAT_16L[*ipbuffer] == 0x0ddd) {		// 0xdddd code used to identify digits
	runcount *= 10;
	runcount += (*ipbuffer - '0');
	ipbuffer++;					// next char
      }
      if ((radl->data_size + runcount) > radl->buffsize) {  // limit run length to avail space
	runcount = (radl->buffsize - radl->data_size);
	done = true;
      }
      memset(radl->data+radl->data_size,last,runcount);
      radl->data_size += runcount;
    }
    else {
      if (temp & 0x8000) {	     /* delta encoded */
	second = temp & 0xff;
        first = (temp >> 8) & 0xff;
	if ((first & 0xf0) != 0xf0)	// if not negative
	  first &= 0x7f;			// strip delta flag from value
	last += first;
	if (radl->data_size < radl->buffsize-1)
	  {
	    radl->data[radl->data_size] = last;
	    radl->data_size++;
	    last += second;
	    radl->data[radl->data_size] = last;
	    radl->data_size++;
	  }
	else
	  done = true;
      }
      else {			     /* absolute value */
	last = temp;
	if (last <= maxval) {
	  if (radl->data_size < radl->buffsize)
	    {
	      radl->data[radl->data_size] = char(last);
	      radl->data_size++;
	    }
	  else
	    done = true;
	}
      }
    }
    if (last > maxval) {
      if (debug) {
	printf("RLE_16 ERR n=%d ch=%c ip=%s\n",n,ch,instring);
	dump_radl(radl);
      }
      done = true;    // bad value, terminate conversion
      last = maxval;
    }
  } while(!done);
  //if (debug) dump_radl(radl);
  return 0;
}

char    A2NXlat[49] = {        0x00,0x01,0x02,0x03,0x04,0x05,0x06,
			       0x10,0x11,0x12,0x13,0x14,0x15,0x16,
			       0x20,0x21,0x22,0x23,0x24,0x25,0x26,
			       0x30,0x31,0x32,0x33,0x34,0x35,0x36,
			       0x40,0x41,0x42,0x43,0x44,0x45,0x46,
			       0x50,0x51,0x52,0x53,0x54,0x55,0x56,
			       0x60,0x61,0x62,0x63,0x64,0x65,0x66 };

/*
 * Decode an ASCII string into an sradl structure
 */

int RLE_6L_radl(char* ipbuffer, int ip_length, s_radl* radl) {
  unsigned char    ThisByte;
  int     RptCount;
  float	  ftemp;
  int     n;
  bool done = false;

  radl->data_size = 0;				// init size
  radl->az = 0;
  if (sscanf(ipbuffer," %%%f%n",&ftemp,&n) == 1)	// get radial angle
    radl->az = int(ftemp * 10);
  else return -1;					// bad angle, fail convert
  ipbuffer += n;					// skip past % and angle parts
  RptCount = 0;
  while (!done) {
    ThisByte = *ipbuffer;
    if (ThisByte >= 'A' && ThisByte <= 'Y') 
      ThisByte -= 'A'; 
    else { 
      if (ThisByte >= 'a' && ThisByte <= 'x')  
	ThisByte -= 'H'; 
      else 
        ThisByte = 0xff;
    }
    if (ThisByte != 0xff) {
      ThisByte = A2NXlat[ThisByte];
      RptCount = 0;
      while (isdigit(*(ipbuffer+1))) {
        RptCount = (RptCount * 10) + (*(ipbuffer+1)-'0');
        ipbuffer++;
      }
      RptCount++;
      if ((radl->data_size + (RptCount*2)) > radl->buffsize) {
        RptCount = (radl->buffsize - radl->data_size) / 2;
        done = true;
      }
      while (RptCount--) {
        radl->data[radl->data_size] = ThisByte & 0x0f;
        radl->data[radl->data_size+1] = ThisByte >> 4;
        radl->data_size +=2;
      }
    }
    else 
      done = true;
    if (radl->data_size == radl->buffsize)
      done = true;
    ipbuffer++;
  }
  return 0;
}

/*
  const BYTE TESTRADIAL[40] = {
  39, 0, 0, 0, 0, 1, 2, 3, 4, 5, 5, 4, 6, 9, 14, 13, 12, 2, 2, 2, 2, 2, 4, 6, 10,
  11, 12, 12, 11, 11, 10, 9, 2, 0, 0, 0, 0, 0, 1, 2

  };
*/

uchar absolute[160] = {
  'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P', 
  '\"','\'','*', ',', ':', ';', '=', '\?','Q', 'R', 'Z', '^', '_', 'z', '|', '~',  
  0x80, 0x81, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87, 
  0x88, 0x89, 0x8A, 0x8B, 0x8C, 0x8D, 0x8E, 0x8F, 
  0x90, 0x91, 0x92, 0x93, 0x94, 0x95, 0x96, 0x97, 
  0x98, 0x99, 0x9A, 0x9B, 0x9C, 0x9D, 0x9E, 0x9F, 
  0xA0, 0xA1, 0xA2, 0xA3, 0xA4, 0xA5, 0xA6, 0xA7, 
  0xA8, 0xA9, 0xAA, 0xAB, 0xAC, 0xAD, 0xAE, 0xAF, 
  0xB0, 0xB1, 0xB2, 0xB3, 0xB4, 0xB5, 0xB6, 0xB7, 
  0xB8, 0xB9, 0xBA, 0xBB, 0xBC, 0xBD, 0xBE, 0xBF, 
  0xC0, 0xC1, 0xC2, 0xC3, 0xC4, 0xC5, 0xC6, 0xC7, 
  0xC8, 0xC9, 0xCA, 0xCB, 0xCC, 0xCD, 0xCE, 0xCF, 
  0xD0, 0xD1, 0xD2, 0xD3, 0xD4, 0xD5, 0xD6, 0xD7, 
  0xD8, 0xD9, 0xDA, 0xDB, 0xDC, 0xDD, 0xDE, 0xDF, 
  0xE0, 0xE1, 0xE2, 0xE3, 0xE4, 0xE5, 0xE6, 0xE7, 
  0xE8, 0xE9, 0xEa, 0xEb, 0xEc, 0xEd, 0xEe, 0xEf, 
  0xF0, 0xF1, 0xF2, 0xF3, 0xF4, 0xF5, 0xF6, 0xF7, 
  0xF8, 0xF9, 0xFa, 0xFb, 0xFc, 0xFd, 0xFe, 0xFf
};

uchar Delta[256] = {
  /******************************************************************************/
  /*					 FIRST BIN                                                        */
  /*																																						*/
  /*            0    1    2    3  4  5  6  7  8  9  A  B  C    D    E    F      */
  /*									                                                          */
  /*     0x */  '.', '>', 'T', 'X', 0, 0, 0, 0, 0, 0, 0, 0, 0, 'b', 'f', '<',
  /*     1x */  '+', 'v', 'U', 'Y', 0, 0, 0, 0, 0, 0, 0, 0, 0, 'c', 'g', 'l',
  /*     2x */  'q', 'w', 'V', '}', 0, 0, 0, 0, 0, 0, 0, 0, 0, ']', 'h', 'm',
  /* S   3x */  'r', 'x', ')', '&', 0, 0, 0, 0, 0, 0, 0, 0, 0, '@','\\', 'n',
  /* E   4x */	  0,   0,   0,   0, 0, 0, 0, 0, 0, 0, 0, 0, 0,   0,   0,	 0,
  /* C   5x */	  0,   0,   0,   0, 0, 0, 0, 0, 0, 0, 0, 0, 0,   0,   0,	 0,
  /* O   6x */	  0,   0,   0,   0, 0, 0, 0, 0, 0, 0, 0, 0, 0,   0,   0,	 0,
  /* N   7x */	  0,   0,   0,   0, 0, 0, 0, 0, 0, 0, 0, 0, 0,   0,   0,	 0,
  /* D   8x */	  0,   0,   0,   0, 0, 0, 0, 0, 0, 0, 0, 0, 0,   0,   0,	 0,
  /*     9x */	  0,   0,   0,   0, 0, 0, 0, 0, 0, 0, 0, 0, 0,   0,   0,	 0,
  /* B   Ax */	  0,   0,   0,   0, 0, 0, 0, 0, 0, 0, 0, 0, 0,   0,   0,	 0,
  /* I   Bx */	  0,   0,   0,   0, 0, 0, 0, 0, 0, 0, 0, 0, 0,   0,   0,	 0,
  /* N   Cx */	  0,   0,   0,   0, 0, 0, 0, 0, 0, 0, 0, 0, 0,   0,   0,	 0,
  /*     Dx */  'o', 's', '(', '$', 0, 0, 0, 0, 0, 0, 0, 0, 0, '!', '/', 'i',
  /*     Ex */  'p', 't', 'y', '{', 0, 0, 0, 0, 0, 0, 0, 0, 0, '[', 'd', 'j',
  /*     Fx */  '-', 'u', 'S', 'W', 0, 0, 0, 0, 0, 0, 0, 0, 0, 'a', 'e', 'k'
};

int DeltaASCII(uchar *Radial, uchar *OutputRadl, int length, int maxopsize)
{
  int  count;
  unsigned char oldval;
  unsigned char newval;
  int  delta;
  bool first;
  int	 deltapair;
  int  RunCount;
  char NumAsStr[16];
  int	 bytesToAdd;
  uchar *saveOutputRadl = (uchar *)OutputRadl;

  if (!(Radial && OutputRadl)) {
    if (OutputRadl)
      *OutputRadl++ = 0; // ensure final string is terminated!
    return 0;; 
  }
  oldval = (uchar)200;	// ensure first time in lays down an absolute
  first = true;
  RunCount = 0;
  for(count = 0; count < length; count++) {
    newval = *Radial++;		// get next i/p value
    delta = newval - oldval;	// get difference to last
		
    // ***** OUTSIDE +-3 allowable deviation
    if(delta > 3 || delta < -3) {	
      
      // we must establish an absolute value if OUTSIDE allowable deviation 

      // check and dump any prior run of a value
      if(RunCount != 0)  {
	bytesToAdd = sprintf(NumAsStr, "%d", RunCount);
	RunCount = 0;
	maxopsize -= bytesToAdd;
	if(maxopsize <= 0) {
	  first = true;
	  break;
	}
	strcpy((char *)OutputRadl, NumAsStr);
        OutputRadl += bytesToAdd;
      }
			
      // check if prior delta was OK but not now
      if(!first) {								
        *OutputRadl++ = absolute[oldval]; // write absolute for prior value
	maxopsize--;
	if(maxopsize <= 0) {
	  first = true;
	  break;
	}
      }
			
      // write absolute for 'out of range' value
      *OutputRadl++ = absolute[newval]; 
      maxopsize--;
      if(maxopsize <= 0) {
	first = true;
	break;
      }
      first = true;			// reset for a fresh delta pair
    }

    // ************ WITHIN +-3 allowable deviation
    else {			    
      // if not creating a delta pair, inc run count for a zero delta
      if(delta == 0 && first) 
        RunCount++;

      else {
        // check if delta pair started 
        if(first) { // NO
          first = false;			// initially assume a new delta pair
          // check if an existing run count => this value is different
          if(RunCount != 0) {
            // if run count == 1 => prior delta was actually a zero - encode a delta pair
            if(RunCount == 1) { 
              *OutputRadl++ = Delta[delta & 0xf]; // encode 0*
              first = true;		// dont start a new delta pair - we just wrote one!
	      maxopsize--;
	      if(maxopsize <= 0) {
		first = true;
		RunCount = 0;
		break;
	      }
            }
            else {           // Run count > 1 => dump run count
	      bytesToAdd = sprintf(NumAsStr, "%d", RunCount);
	      maxopsize -= bytesToAdd;
	      if(maxopsize <= 0) {
		first = true;
		RunCount = 0;
		break;
	      }
	      strcpy((char *)OutputRadl, NumAsStr);
              OutputRadl += bytesToAdd;
	    }
            
            RunCount = 0;			// flush run count
          }
          if(!first)
            // NOTE must mask to prevent sign extension and -ve array indexing BAD!
            deltapair = (delta & 0xf) << 4; // create first part of delta pair
        }
        else {     // delta pair started - finish it off
          deltapair |= (delta & 0x0f);
          *OutputRadl++ = Delta[deltapair]; // lookup encoding value for this pair
          first = true;			  // reset for new delta pair
	  maxopsize--;
	  if(maxopsize <= 0) {
	    RunCount = 0;
	    break;
	  }
	}
      }
    }
    oldval = newval;
  }
  if(!first) { 			      // pick up possible started delta pair 
    *OutputRadl++ = absolute[newval]; // and whack it onto the end of line as an absolute
    maxopsize--;
    if(maxopsize <= 0) {
      OutputRadl--;
      *OutputRadl = 0;		      // ensure final string is terminated!
      return (int(OutputRadl - saveOutputRadl));;
    }
  }
  if(RunCount != 0)	{	     // whack possible runcount onto end of line
    OutputRadl += sprintf((char *)OutputRadl, "%d", RunCount);
  }

  *OutputRadl++ = 0;  // ensure final string is terminated!
    
  // calc output line length
  return (int(OutputRadl - saveOutputRadl));
}




/**** SIX LEVEL ASCII encoding algorithm ****/

const char SixLevelEncode[64] =	{		// convert nibble to ASCII 
  /**********************************************/
  /*					          1st BIN                 */
  /*																						*/
  /*            0   1   2   3   4   5   6   7   */
  /*								                            */
  /* 2   0x */ 'A','B','C','D','E','F','G','A',
  /* n   1x */ 'H','I','J','K','L','M','N','A',
  /* d   2x */ 'O','P','Q','R','S','T','U','A',
  /*     3x */ 'V','W','X','Y','a','b','c','A',
  /* B   4x */ 'd','e','f','g','h','i','j','A',
  /* I   5x */ 'k','l','m','n','o','p','q','A',
  /* N   6x */ 'r','s','t','u','v','w','x','A',
  /*     7x */ 'A','A','A','A','A','A','A','A'
};

int SixLevelASCII(unsigned char *Radial, char *OutputRadl, int length, int maxopsize) 
{
  unsigned char newpair;
  unsigned char lastpair = 0xff;		// impossible value to achieve with 6 level data
  int	RunCount = 0;
  char NumAsStr[16];
  int bytesToAdd;
  bool safebreak;
  char *saveOutputRadl = OutputRadl;

  while(length > 1) {  // note > 1 will work for odd length radials
    newpair = *Radial++ & 0x7;
    newpair |= (*Radial++ & 0x07) << 3; // build a six bit value for indexing into lookup table
    length -= 2;
    safebreak = false;
    if((newpair & 0x07) == 0x07) 
      break; 
    if((newpair & 0x38) == 0x38)
      break; 
    if(newpair == lastpair)
      RunCount++;
    else {
      if(RunCount) { 
	bytesToAdd = sprintf(NumAsStr, "%d", RunCount);
	RunCount = 0;
	maxopsize -= bytesToAdd;
	if(maxopsize <= 0) {
	  length = 0;
	  break;
	}
	strcpy(OutputRadl, NumAsStr);
	OutputRadl += bytesToAdd;
      }
      *OutputRadl++ = SixLevelEncode[newpair];
      maxopsize--;
      if(maxopsize <= 0) {
	length = 0;
	break;
      }
    }
    lastpair = newpair;
  }
  if(RunCount) // pick up possible trailing runcount
    OutputRadl += sprintf(OutputRadl, "%d", RunCount);
  if(length == 1) { // odd length radial, pick up the final level
    newpair = *Radial++ & 0x7;
    *OutputRadl++ = SixLevelEncode[newpair];
  }
  *OutputRadl++ = 0;  // terminate end of string

  // calc output line length
  return (int(OutputRadl - saveOutputRadl));
}


/****************************************************************************/
/* binary radial encoding / decoding routines                               */
/* 
   Since radar data invariably has lots of nulled bins, it makes more sense 
   to compress these nulls in preference to attempting to detect runs of 
   intensity levels.

   The method used to achieve this Null Suppression is to include a run 
   count after any occurence of a 0x00 value in the data stream. This is 
   wasteful on isolated nulls but these are usually the exception rather 
   than the norm. 
   A single null is therefore encoded as:  0x00 0x01

   The end of radial is marked by 0x00 0x00 which also serves as a useful sync 
   marker if data corruption should occur.

   A data section is implicit by writing data until a zero is encountered,
   at which stage the zero is written then a run count is laid down.

   The start of every radial holds a 16 bit count of how many bytes follow, 
   including the count bytes, to describe the entire radial.


   Examples:
   ~~~~~~~~
   A sample radial always starts as below: (spaces added for clarity)

   @ XXX HI LO
   |  |  |  |
   |  |  |  '----------- LS byte for radial length
   |  |  '-------------- MS byte for radial length
   |  '----------------- radial's angle - ASCII character string
   |                       (XXX for azimuth, XX.X for elevation)
   '-------------------- start of binary radial character marker


   A run count of 24 null bins would be represented as follows:

   0x00 0x18 
   |
   '----------- count of 24 

*/



int EncodeBinaryRadl(unsigned char *Radial, unsigned char *OutputRadl, int length, int maxopsize)
{
  int opsize = 0;
  unsigned char nullruncount = 0;
  unsigned char oneruncount = 0;
  unsigned char latestval = 1;							// ensure initialised non zero in case of zero length input ray
  unsigned char *pSizeLoc = OutputRadl;    // remember location of start of output radial
  bool bAddVal = true;

  if (Radial == NULL || OutputRadl == NULL) 
    return 0; 

  OutputRadl += 2 * sizeof(unsigned char); // step over length marker bytes for now
  bool bMaxLenDetected = false;

  for(int count = 0; count < length; count++) {
    bAddVal = true;                 // by default add new bytes to output
    latestval = *Radial++;					// get next i/p value

    if(latestval == 0x00) {            // is it a null ?    
      if(oneruncount) {                // have we been counting ones up to this byte ?
        *OutputRadl++ = oneruncount;   // YES, write out the count value reached
        opsize++;                   
        oneruncount = 0;               // reset run count - reached end of a null section
      }
      nullruncount++;                   // account for it
      if(nullruncount == 1) {           // and make sure that we do write out the initial 0x00
        latestval = 0x00;          
      }
      else if(nullruncount == 255) {    // check that we have not exhausted our run count
        latestval = nullruncount;       // count will be added to output later
        nullruncount = 0;               // reset run count to which will also force next input zero to be written to output
      }  
      else
        bAddVal = false;            // prevent writing out new value just yet
    }
    else if(latestval == 0x01) {     // typical data code for spectral width moment
      if(nullruncount) {                // have we been counting nulls up to this byte ?
        *OutputRadl++ = nullruncount;   // YES, write out the count value reached
        opsize++;                   
        nullruncount = 0;               // reset run count - reached end of a null section
      }
      oneruncount++;                   // account for it
      if(oneruncount == 1) {           // and make sure that we do write out the initial 0x01
        latestval = 0x01;          
      }
      else if(oneruncount == 255) {    // check that we have not exhausted our run count
        latestval = oneruncount;       // count will be added to output later
        oneruncount = 0;               // reset run count to which will also force next input zero to be written to output
      }  
      else
        bAddVal = false;            // prevent writing out new value just yet
    }
    else {                          // we don't have a null or a one
      if(nullruncount) {                // have we been counting nulls up to this byte ?
        *OutputRadl++ = nullruncount;   // YES, write out the count value reached
        opsize++;                   
        nullruncount = 0;               // reset run count - reached end of a null section
      }
      if(oneruncount) {                // have we been counting nulls up to this byte ?
        *OutputRadl++ = oneruncount;   // YES, write out the count value reached
        opsize++;                   
        oneruncount = 0;               // reset run count - reached end of a null section
      }
      // NOTE: we will write out the non-zero value later
    }

    if(opsize >= maxopsize) {					// check o/p size
      bMaxLenDetected = true;			
      break;
    }

    if(bAddVal) {
      *OutputRadl++ = latestval;    // write out the latest value 
      opsize++;                     // and bump the byte counter                 
      if(opsize >= maxopsize) {
	bMaxLenDetected = true;			
	break;
      }
    }
  }

  if(!bMaxLenDetected) {
    if(nullruncount) {                // have we been counting nulls up to this byte ?
      latestval = nullruncount;       // remember last value written
      *OutputRadl++ = nullruncount;   // YES, write out the count value reached
      opsize++;                   // and bump the byte counter
      nullruncount = 0;               // reset run count
    }
    if(oneruncount) {                // have we been counting nulls up to this byte ?
      latestval = oneruncount;       // remember last value written
      *OutputRadl++ = oneruncount;   // YES, write out the count value reached
      opsize++;                   // and bump the byte counter
      oneruncount = 0;               // reset run count
    }

    *OutputRadl++ = 0;            // write line terminating nulls 
    opsize++;
    if(latestval) {
      *OutputRadl++ = 0;          // only add second null if last data was not zero
      opsize++;
    }
  }
  else {													// force end null's onto over length o/p radial
    OutputRadl--;
    *OutputRadl-- = 0;
    *OutputRadl-- = 0;
  }

  *pSizeLoc++ = (unsigned char)((opsize >> 8) & 0xff);
  *pSizeLoc++ = (unsigned char)(opsize & 0xff);

  return opsize + 2;            // include size bytes for returned size
}

bool checkForEmbeddedMSSG(unsigned char* buffer, int length)
{
  char* ptr = (char *)buffer;
  char* mssgstr = 0;
  int len = length;

  while (len && !mssgstr)
    {
      if (*ptr == 'M')
	{
	  mssgstr = strstr(ptr, "MSSG:");
	  if (mssgstr)
	    fprintf(stderr, "\ncheckForEmbeddedMSSG DETECTED RADIAL ERROR - %s\n", mssgstr);
	}
      ptr++;
      len--;
    }
  if (!mssgstr)
    {
      ptr = (char *)buffer;
      len = length;
      while (len && !mssgstr)
	{
	  if (*ptr == 'E')
	    {
	      mssgstr = strstr(ptr, "END STATUS");
	      if (mssgstr)
		fprintf(stderr, "\ncheckForEmbeddedMSSG DETECTED RADIAL ERROR - %s\n", mssgstr);
	    }
	  ptr++;
	  len--;
	}
    }
  return (mssgstr != 0);
}

void printRadlBuffer(unsigned char* buffer, int length)
{
  char* ptr = (char *)buffer;

  fputc('\n', stderr);
  while (length--)
    fputc(*(ptr++), stderr);
  fputc('\n', stderr);
}

int DecodeBinaryRadl(unsigned char* ipbuffer, int ip_length, s_radl* radl)
{
  unsigned char ThisByte;
  unsigned char LastByte;
  int bytestofollow = 0;
  bool firstpass = true;
  float Az, El;
  int   secs, n;
  char errstr[512];
  int	lengthcount = 0;
  unsigned char* ptr;
  int Errno = 0;
  bool debug = false;
  
  bool lenBad1, lenBad2, lencountok;	
  // 2 interpretations of length, either inc or excl length bytes
  // At this stage, make test lenient and allow either
  
  radl->data_size = 0;				// init size
  radl->az = 0;
  if (sscanf((char*)ipbuffer,"@%f,%f,%d=%n",&Az, &El, &secs, &n) == 3) {	// get radial angle
    radl->az = int(Az * 10);
    radl->el = int(El * 10);
  } 
  else 
    { 
      if (debug)
	{
	  sprintf(errstr, "DecodeBinaryRadl - Bad Radl Header=%16s", 
		  ipbuffer);
	  RapicLog(errstr, LOG_CRIT+LOG_CDATA);
	}
      return -1;			// bad angle, fail convert
    }
  if (n > 18)
    {
      if (debug)
	{
	  sprintf(errstr, "DecodeBinaryRadl - Bad Radl Header length count=%d - %16s", 
		  n, ipbuffer);
	  RapicLog(errstr, LOG_CRIT+LOG_CDATA);
	}
      return -1;					// bad angle, fail convert
    }
  ipbuffer += n;		// skip past @ and angle parts
  bytestofollow = *ipbuffer++;
  bytestofollow <<= 8;
  bytestofollow += *ipbuffer++;
  /*
    lencountok = (bytestofollow+n+2 == ip_length) || // Kurnell style
    (bytestofollow+n == ip_length);    // CPol style
  */
  lencountok = (bytestofollow+n) <= ip_length;	// use relaxed test, only detect CPol friendly overrun
  if (!lencountok)
    {
      if (debug)
	{
	  sprintf(errstr, "DecodeBinaryRadl - Length count overrun - ip_length=%d"
		  " Length from radial=%d(**%d**), az/el=%d/%d", 
		  ip_length, bytestofollow+n+2, bytestofollow+n, radl->az/10, radl->el/10);
	  RapicLog(errstr, LOG_CRIT+LOG_CDATA);
	}
      bytestofollow = ip_length - n - 2;
      Errno = -2;
    }
  // Kurnell style length (DOESN'T include length bytes)
  lenBad1 = (*(ipbuffer+bytestofollow-1) ||	// ipbuffer already inc'd past 2 count chars
	     *(ipbuffer+bytestofollow-2));
  // CPol style length (DOES include length bytes)
  lenBad2 = (*(ipbuffer+bytestofollow-3) ||	// ipbuffer already inc'd past 2 count chars
	     *(ipbuffer+bytestofollow-4));
  if (lenBad1 && lenBad2) 	// 2 NULLS not found at either type length
    {
      ptr = ipbuffer;
      lengthcount = 2;    // if it finds nulls 1st time then length is already 4
      while ((lengthcount <= ip_length-n) && (*ptr || *(ptr+1)))
	{   // look for 2 nulls
	  ptr++;
	  lengthcount++;
	}
      if (debug)
	{
	  sprintf(errstr, "DecodeBinaryRadl - Error detecting 2 nulls at length=%d"
		  ", last-3=%d, last-2=%d, last-1=%d, last=%d - nulls found at length=%d, az/el=%d/%d"
		  " ip_length=%d", 
		  bytestofollow, 
		  *(ipbuffer+bytestofollow-4), 
		  *(ipbuffer+bytestofollow-3), 
		  *(ipbuffer+bytestofollow-2), 
		  *(ipbuffer+bytestofollow-1), 
		  lengthcount, radl->az/10, radl->el/10, ip_length);
	  RapicLog(errstr, LOG_CRIT+LOG_CDATA);
	}
      if (!checkForEmbeddedMSSG(ipbuffer, bytestofollow) && debug)
	printRadlBuffer(ipbuffer, bytestofollow);
      bytestofollow = ip_length - n - 2;
      Errno = -2;
    }
  while (bytestofollow--) {
    ThisByte = *ipbuffer++;     // read byte from buffer
    if(firstpass) {
      firstpass = false;        // we can't use LastByte until after the first pass through
      radl->data[radl->data_size++] = ThisByte;
      LastByte = ThisByte;
    }
    else {
#ifdef _DEBUG
      // check and flag suspect encoded rays
      CheckBinaryIntegrity(LastByte, ThisByte, Az, El, radl->data_type);
#endif

      if((ThisByte | LastByte) == 0) {
        radl->data_size--;        // bump size down after we know we have got the end marker
	if (!((bytestofollow == 0) || (bytestofollow == 2)))  // allow both radial length style
	  {
	    if (debug)
	      {
		sprintf(errstr, "DecodeBinaryRadl - Detected nulls while runcount > 0 (%d)", 
			bytestofollow);
		RapicLog(errstr, LOG_CRIT+LOG_CDATA);
	      }
	    bytestofollow = 0;  // terminate loop
	    Errno = -2;
	  }
        break;
      }
      if(LastByte == 0x00) {      // ThisByte is actually a run count if previous value was zero
        LastByte = 0xff;          // prevent recursion into this routine
        while(--ThisByte) {       // generate as many zero's as required
          radl->data[radl->data_size++] = 0x00;
          if(radl->data_size >= radl->buffsize) // safeguard against excessive length radials
            break;
        }
      }
      else if(LastByte == 0x01) {
        LastByte = 0xff;          // prevent recursion into this routine
        while(--ThisByte) {       // generate as many one's as required
          radl->data[radl->data_size++] = 0x01;
          if(radl->data_size >= radl->buffsize) // safeguard against excessive length radials
            break;
        }
      }
      else {                      // non-zero prior value - this must be a data byte
        radl->data[radl->data_size++] = ThisByte;
        LastByte = ThisByte;
      }

    }
    if(radl->data_size >= radl->buffsize) // safeguard against excessive length radials
      break;
  }
  return Errno;
}


/* routine decodes a binary encoded radial
 *
 * returns: number of valid bins in opbuffer  (-1 => error)
 */
int DecodeBinaryRadl(unsigned char* opbuffer, unsigned char *ipbuffer, int maxsize, int &angle);
            
int DecodeBinaryRadl(unsigned char* opbuffer, unsigned char *ipbuffer, int maxsize, int &angle)
{
  unsigned char ThisByte;
  unsigned char LastByte;
  int bytestofollow = 0;
  bool firstpass = true;
  float Az, El;
  int   n;
  int data_size;

  data_size = 0;				// init size
  angle = 0;
  if (sscanf((char*)ipbuffer,"@%f,%f,%d=",&Az, &El, &n) == 3) {	// get radial angle
    angle = int(Az * 10);
  } 
  else 
    return -1;					// bad angle, fail convert
  ipbuffer += 16;				// skip past @ and angle parts
  bytestofollow = *ipbuffer++;
  bytestofollow <<= 8;
  bytestofollow += *ipbuffer++;
  while (bytestofollow--) {
    ThisByte = *ipbuffer++;     // read byte from buffer
    if(firstpass) {
      firstpass = false;        // we can't use LastByte until after the first pass through
      opbuffer[data_size++] = ThisByte;
      LastByte = ThisByte;
    }
    else {
#ifdef _DEBUG
      // check and flag suspect encoded rays
      CheckBinaryIntegrity(LastByte, ThisByte, Az, El, eMatchAllDataTypes);
#endif

      if((ThisByte | LastByte) == 0) {
        data_size--;        // bump size down after we know we have got the end marker
        break;
      }
      if(LastByte == 0x00) {       // ThisByte is actually a run count if previous value was zero
        LastByte = 0xff;           // prevent recursion into this routine
        while(--ThisByte) {        // generate as many zero's as required
          opbuffer[data_size++] = 0x00;
          if(data_size >= maxsize) // safeguard against excessive length radials
            break;
        }
      }
      else if(LastByte == 0x01) {
        LastByte = 0xff;           // prevent recursion into this routine
        while(--ThisByte) {        // generate as many one's as required
          opbuffer[data_size++] = 0x01;
          if(data_size >= maxsize) // safeguard against excessive length radials
            break;
        }
      }
      else {                       // non-zero prior value - this must be a data byte
        opbuffer[data_size++] = ThisByte;
        LastByte = ThisByte;
      }
    }
    if(data_size >= maxsize) // safeguard against excessive length radials
      break;
  }
  return data_size;
}

#ifdef _DEBUG
void CheckBinaryIntegrity(unsigned char LastByte, unsigned char ThisByte, float Az, float El, e_data_type data_type)
{
  char  ErrStr[256];

  if(ThisByte == 0x00 && LastByte == 0x01) {
    switch(data_type) {
    case e_refl:
      sprintf(ErrStr, "DecodeBinaryRadial - Definite bad transition 01->00 [REFL] (Az=%.1f El=%.1f)\n", Az, El);
      break;
    case e_vel:
      sprintf(ErrStr, "DecodeBinaryRadial - Definite bad transition 01->00 [VEL] (Az=%.1f El=%.1f)\n", Az, El);
      break;
    case e_spectw:
      sprintf(ErrStr, "DecodeBinaryRadial - Definite bad transition 01->00 [SPW] (Az=%.1f El=%.1f)\n", Az, El);
      break;
    case e_rawrefl:
      sprintf(ErrStr, "DecodeBinaryRadial - Definite bad transition 01->00 [RAW] (Az=%.1f El=%.1f)\n", Az, El);
      break;
    default:
      sprintf(ErrStr, "DecodeBinaryRadial - Definite bad transition 01->00 [???] (Az=%.1f El=%.1f)\n", Az, El);
      break;
    }          
    RapicLog(ErrStr, LOG_CRIT+LOG_CDATA);
  }
  if(ThisByte == 0x01 && LastByte == 0x00) {
    switch(data_type) {
    case e_refl:
      sprintf(ErrStr, "DecodeBinaryRadial - Possible bad transition 00->01 [REFL] (Az=%.1f El=%.1f)\n", Az, El);
      break;
    case e_vel:
      sprintf(ErrStr, "DecodeBinaryRadial - Possible bad transition 00->01 [VEL] (Az=%.1f El=%.1f)\n", Az, El);
      break;
    case e_spectw:
      sprintf(ErrStr, "DecodeBinaryRadial - Possible bad transition 00->01 [SPW] (Az=%.1f El=%.1f)\n", Az, El);
      break;
    case e_rawrefl:
      sprintf(ErrStr, "DecodeBinaryRadial - Possible bad transition 00->01 [RAW] (Az=%.1f El=%.1f)\n", Az, El);
      break;
    default:
      sprintf(ErrStr, "DecodeBinaryRadial - Possible bad transition 00->01 [???] (Az=%.1f El=%.1f)\n", Az, El);
      break;
    }          
    RapicLog(ErrStr, LOG_WARNING+LOG_CDATA);
  }
}
#endif

