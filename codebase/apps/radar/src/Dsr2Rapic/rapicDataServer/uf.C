/***********************************************************************************
**
Take rapicradar structure from ufMgr and write out an elevation of the volume
in UF format.
 
Written godknowswhen.
Modified by Sanjeev ???
Modified by M. Whimpey ???
Modified by S.Dance 2 Nov 2001 for inclusion into Rapic V420b

************************************************************************************/

#include <uf.h>
#include <string.h>
#include <strings.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/file.h>
#include <ctype.h>

#include <algorithm>

bool writeRecByteLength = true;

inline bool littleEndian()
{
  long one= 1;
  return (*((char *)(&one)));
}

bool isLittleEndian = littleEndian();

template <typename T>
inline T networkByteOrder(const T& rhs)
{
  if (isLittleEndian)
    {
      T lhs = rhs;
      unsigned char* pData = reinterpret_cast<unsigned char*>(&lhs);
      std::reverse(pData, pData + sizeof(T));
      return lhs;
    }
  else
    return rhs;
}

inline short networkByteOrderShort(short sh)
{
  return networkByteOrder(sh);
}

inline int networkByteOrderInt(int in)
{
  return networkByteOrder(in);
}

static char const cvsid[] = "$Id: uf.C,v 1.2 2008/02/29 04:44:54 dixon Exp $";

static struct ufmatch {
    char 	*ufField;
    char 	*rapField;
    int  	ufIndex;
    int  	rapIndex;
    float 	ufScale;
    float 	rapScale;
    float	rapBias;
}fieldMatch [] = {

  {"CZ", FLDREFL,  UFCZ,   RAPICCZ ,	 16.0,   2.0,  64.0},
  {"VR", FLDVEL	,  UFVR,   RAPICVEL,	 16.0, 127.0, 128.0},
  {"SW", FLDWID	,  UFSW,   RAPICWID,	 16.0, 127.0, 128.0},
  {"UZ", FLDUREFL, UFUZ,   RAPICUZ ,	 16.0,   2.0,  64.0},
  {"ZD", FLDZDR	,  UFZD,   RAPICZDR,	 16.0, 127.0, 128.0},
  {"",	 FLDNONE,  UFNONE, RAPICNONE,     0.0,   0.0,   0.0}

};


uf::uf(bool set_debug)
{
 int ii;
 debug = (int) set_debug;
 fdout = -1;
 zeroData();
 for ( ii = 0; strlen(fieldMatch[ii].ufField) > 0 ; ii++ )
   fieldMatch[ii].ufIndex = UFNONE;
}

uf::~uf()
{
  if ( fdout > 0 )
      close(fdout);
  fdout = -1 ;
  zeroData();
}

bool uf::openVolume(char *path,char *name)
{
  char fullname[256];
  
     if ( fdout > 0) closeVolume();
     strcpy(fullname,path);
     strcat(fullname,"/");
     strcat(fullname,name);
     fdout = open(fullname, O_CREAT | O_TRUNC | O_RDWR, 0666);
     maxfields = UFZD;
     if ( debug )
         fprintf(stdout,"uf::fullname %s\n",fullname);
     if ( fdout == -1 )
       {
	 fprintf(stdout,"\nuf::openVolume FAILED opening %s \n",fullname);
	 perror(0);
	 return false;
       }
     //          exitUf(ERRWRITEOPEN,fullname);
     zeroData();
     if ( debug )fprintf(stdout,"\nuf::openVolume OK %s \n",fullname);
     spos = lseek(fdout,0L,SEEK_CUR);
     return true;
}

int uf::closeVolume()
{
 int nret = -1;
 int ii;
 
   if ( fdout > 0)
      nret = close(fdout);
   fdout = -1;
   if ( debug )
	for ( ii = 0; strlen(fieldMatch[ii].ufField) > 0 ; ii++ )
	    fprintf(stdout,"uf::%s %d %d \n",
		    fieldMatch[ii].rapField,
		    fieldMatch[ii].rapIndex,
		    fieldMatch[ii].ufIndex);  
   zeroData();
   for ( ii = 0; strlen(fieldMatch[ii].ufField) > 0 ; ii++ )
         fieldMatch[ii].ufIndex = UFNONE;
   maxfields = UFZD;
   for ( ii = 0; ii < 32; ii++ )
     Fhead[ii] = 0;
   if ( debug )fprintf(stdout,"uf::CloseVolume\n");
   return nret;
}

void uf::setUfParams(struct rapicradar *radPtr,struct scan *scanPtr,int ngates)
{
 time_t bintim = 0;
 struct tm *stm;
 int degrees,minutes,seconds;
 char tmpnm[35];
 int i, j;
 float freq,prf;
 int ii;

   if ( radPtr == NULL )
        return;
   sweepno     = 1;
   rdata = *radPtr;
   strncpy ((char *)&Buffer[0], "UF", 2);
   Buffer[2]  = networkByteOrderShort(46);
   Buffer[3]  = networkByteOrderShort(60);
   Buffer[4]  = networkByteOrderShort(60);
   Buffer[5]  = networkByteOrderShort(0);
   Buffer[6]  = networkByteOrderShort(1);
   Buffer[58] = networkByteOrderShort(2);
   strncpy ((char *)&Buffer[31], "UT", 2);
   strncpy ((char *)&Buffer[45], "RAPIC2UF", 8);
   strncpy ((char *)&Buffer[54], "RAPIC   ", 8);

   strncpy ((char *)&Buffer[40], "RAPIC   ", 8);
   for ( ii = 0; ii < int(strlen(rdata.stationname)); ii++ )
          rdata.stationname[ii] = toupper(scanPtr->station[ii]);
   for ( ii = 0; ii < int(strlen(rdata.radarname)); ii++ )
        rdata.radarname[ii] = toupper(rdata.radarname[ii]);

   /*  get rid of '-' character in radar name, so name can fit
       in 8 character field for radar name in UF file.   */
   j = 0;
   for (i=0;;i++)
     {
       if (rdata.radarname[i] != '-')
	 {
	   tmpnm[j] = rdata.radarname[i];
	   if (rdata.radarname[i] == '\0') break;
	   j++;
	 }
     }
   strncpy ((char *)&Buffer[10], tmpnm, 8);
   strncpy ((char *)&Buffer[14], rdata.stationname, 8);
   bintim = time(0);
   stm = localtime (&bintim);
   Buffer[37] = networkByteOrderShort(stm->tm_year);
   Buffer[38] = networkByteOrderShort(stm->tm_mon + 1);
   Buffer[39] = networkByteOrderShort(stm->tm_mday);
   Buffer[44] = networkByteOrderShort(-32768);
   todegminsec(scanPtr->latitude,degrees,minutes,seconds);
   Buffer[18] = networkByteOrderShort(degrees);
   Buffer[19] = networkByteOrderShort(minutes);
   Buffer[20] = networkByteOrderShort(seconds *  64);
   todegminsec(scanPtr->longitude,degrees,minutes,seconds);
   Buffer[21] = networkByteOrderShort(degrees);
   Buffer[22] = networkByteOrderShort(minutes);
   Buffer[23] = networkByteOrderShort(seconds *  64);
   Buffer[24] = networkByteOrderShort(rdata.altitude);
   Buffer[34] = networkByteOrderShort(1);
   if ( volType == VOL_RHI )
     Buffer[34] = networkByteOrderShort(3);
   Buffer[36] = networkByteOrderShort(-32768);
   Buffer[49] = networkByteOrderShort(-32768);
   Buffer[50] = networkByteOrderShort(-32768);
   Buffer[51] = networkByteOrderShort(scanPtr->hour);
   Buffer[52] = networkByteOrderShort(scanPtr->min);
   Buffer[53] = networkByteOrderShort(scanPtr->sec);
   Vu         = scanPtr->nyq;
   freq = (LIGHTVEL  ) / ((double )rdata.wavelength * 0.001) ;
   prf  = (Vu * freq * 4.0 / LIGHTVEL) + 0.5;
   if ( debug )
   fprintf(stdout,"uf::Vu %f, wlen(mm) %d, freq(Mhz) %6.2f, calc prf %d, ht %d \n",
                  Vu,rdata.wavelength,freq/10.0e6,(int )prf,(int )rdata.altitude);
   Fhead[6] = networkByteOrderShort(-32768);
   Fhead[7] = networkByteOrderShort(-32768);
   Fhead[8] = networkByteOrderShort(-32768);
   Fhead[9] = networkByteOrderShort(-32768);
   Fhead[8] = networkByteOrderShort(short(scanPtr->angres * 64.0));
   Fhead[9] = Fhead[8];
   Fhead[10] = networkByteOrderShort(3);
   Fhead[11] = networkByteOrderShort(short(LIGHTVEL * 64 / (freq * 100)));
   Fhead[12] = networkByteOrderShort(100);
   strncpy ((char *)&Fhead[13], "  ", 2);
   Fhead[14] = networkByteOrderShort(-32768);
   Fhead[15] = networkByteOrderShort(-32768);
   strncpy ((char *)&Fhead[16], "  ", 2);
   Fhead[17] = networkByteOrderShort(short(1000000.0 / prf));
   Fhead[18] = networkByteOrderShort(16);
   for ( ii = 20; ii < 32; ii++ )
         Fhead[ii] = networkByteOrderShort(-32768);
   Fhead[27]   = networkByteOrderShort(0);
   Fhead[28]   = networkByteOrderShort(0);
   Fhead[29]   = networkByteOrderShort(0);
   Buffer[60]  = networkByteOrderShort(1);
   Fhead[5]    = networkByteOrderShort(short(ngates)); // scanPtr->numgates;
   if ( debug )fprintf(stdout,"uf::numgates %d \n",networkByteOrderShort(Fhead[5]));
}

void uf::setnumFields(char *fldname,int &srcfld,int &dstfld)
{
   int ii;
   
    srcfld = RAPICNONE;
    dstfld = UFNONE;
    
    for ( ii = 0; strlen(fieldMatch[ii].ufField) > 0; ii++ )
    {
       if ( strncasecmp(fldname,
                        fieldMatch[ii].rapField,
			strlen(fieldMatch[ii].rapField)) == 0 )
       {
	    fieldMatch[ii].ufIndex = fieldMatch[ii].rapIndex;
	    srcfld = fieldMatch[ii].rapIndex;
	    dstfld = fieldMatch[ii].ufIndex;
	    return ; 
       }
    }
}

void uf::debugFields()
{
  int ii;
  
  //if ( debug )
  fprintf(stdout,"uf::debugFields\n");
  
  for ( ii = 0; strlen(fieldMatch[ii].ufField) > 0 ; ii++ )
    fprintf(stdout,"uf::%s %d %d \n",
	    fieldMatch[ii].rapField,
	    fieldMatch[ii].rapIndex,
	    fieldMatch[ii].ufIndex);  
}

void uf::setScanData(struct scan *scanPtr)
{
  long rlen = 0;
  
   if ( fdout == -1 || scanPtr == NULL ) return;
   volType = VOL_PPI;
   if ( scanPtr->scantype == RHI )
         volType = VOL_RHI;
   if ( strncmp(scanPtr->product,"RHI",3) == 0 )
         volType = VOL_RHI;
   if ( debug )
   fprintf(stdout,"uf::scanType %d scanProduct %s volType %d PPI %d RHI %d \n",
           scanPtr->scantype,scanPtr->product,volType,PPI,RHI);
   fangind = 33;
   vangind = 32;
   rlen = spos;
   writeData(fdout,scanPtr,rlen);
}


//
// Private Methods
//
float uf::rapic2uz(struct scan *scanPtr,unsigned char cdat)
//hopefully returns physical dBZ
{
  // float dat = (float )cdat;
  if ( cdat == 0 ) return (float )-32768;
  if (!scanPtr->dbmlvl) fprintf(stderr,"uf::rapic2uz: ERROR; No level table\n");
  return scanPtr->dbmlvl[cdat]; 
}
float uf::rapic2vr(struct scan *scanPtr,unsigned char cdat,int levels)
//hopefully returns physical m/sec
{
  // float dat = (float )cdat;
  if ( cdat == 0 ) return (float )-32768;
  return (float)((cdat-levels/2)*scanPtr->nyq/(levels/2-1));
}
float uf::rapic2sw(unsigned char cdat) //legacy
{
 float dat = (float )cdat;
  if ( cdat == 0 ) return (float )-32768;
  return (float )(dat)/256.0;
}
float uf::rapic2zdr(unsigned char cdat) //legacy
{
 float dat = (float )cdat;
  if ( cdat == 0 ) return (float )-32768;
  return (float )((dat - 64.0)/2.0);
}

void uf::todegminsec(float lat,int &deg,int &min,int &sec)
{
 int sign = 1;
 // int ss;
 
   deg = min = sec = 0;
   if ( lat < 0 )
   {
      lat = -lat;
      sign = -1;
   }
   deg = int(lat);
   min = int(lat * 60) % 60;
   sec = int((lat * 3600) + 0.5) % 60;
   /* you've got to be kidding!!!
   ss  = int((lat - deg )* 3600 + 0.5);
   while ( ss >= 60 )
   {
     min++;
     ss -= 60;
   }
   sec = ss;
   */
   deg *= sign;
   min *= sign;
   sec *= sign;
   if ( debug )
     {
       printf("\n degrees = %d",deg);
       printf("\n minutes = %d",min);
       printf("\n seconds = %d",sec);
     }
}

void uf::writeData(int fdout,struct scan *scanPtr,long &rlen)
{
int iaz,sfield,ifield,igate;
int num ,field_len;
short oldvalu,atemp;
double scale,TV;
long pos;
float  maxdbz  = -99999, mindbz  = 99999;
float  maxvel  = -99999, minvel  = 99999;
float  maxudbz = -99999, minudbz = 99999;
float  maxwid  = -99999, minwid  = 99999;
float imaxdbz  = -99999,imindbz  = 99999;
float imaxvel  = -99999,iminvel  = 99999;
float imaxudbz = -99999,iminudbz = 99999;
float imaxwid  = -99999,iminwid  = 99999;
float dbz,vel,wid;
unsigned char inp;
int rlen_nw;

int hour,min,sec,hh,mm,ss;

    rlen = 0;
    //num = maxfields;
    num = 2;  //SD, 1st cut of field manip
    
    if ( scanPtr == NULL || fdout <= 0 ) return;
    Buffer[9] = networkByteOrderShort(sweepno);
    sweepno++;
    Buffer[35] = networkByteOrderShort(short(scanPtr->elev * 64.0));
    Buffer[33] = networkByteOrderShort(short(scanPtr->elev * 64.0));
    Buffer[7]  = networkByteOrderShort(0);
    Buffer[25] = networkByteOrderShort((scanPtr->year-1900) % 100);
    Buffer[26] = networkByteOrderShort(scanPtr->month);
    Buffer[27] = networkByteOrderShort(scanPtr->day);
    Buffer[28] = networkByteOrderShort(scanPtr->hour);
    Buffer[29] = networkByteOrderShort(scanPtr->min);
    Buffer[30] = networkByteOrderShort(scanPtr->sec);
    field_len = networkByteOrderShort(Fhead[5]);
    if ( debug )fprintf(stdout,"uf::Elev %f \n",scanPtr->elev);
    hh = networkByteOrderShort(Buffer[28]);
    mm = networkByteOrderShort(Buffer[29]);
    ss = networkByteOrderShort(Buffer[30]);
    for ( iaz = 0; iaz < scanPtr->nazimuths; iaz++ )
    {
         hour       = hh;
	 min        = mm;
	 sec        = ss + scanPtr->gatedata[iaz].time;
	 if ( sec > 59 )
	 {
	      sec -= 60;
	      min++;
	      if ( min > 59 )
	      {
	           min -= 60;
		   hour++;
	      }
	 }
         Buffer[28] = networkByteOrderShort(hour);
         Buffer[29] = networkByteOrderShort(min);
         Buffer[30] = networkByteOrderShort(sec);
/*	 Buffer[fangind] = scanPtr->gatedata[iaz].elev * 64; */
	 maxdbz  = -99999*100.0,mindbz  = 99999*100.0;
	 maxvel  = -99999*100.0,minvel  = 99999*100.0;
	 maxudbz = -99999*100.0,minudbz = 99999*100.0;
	 maxwid  = -99999*100.0,minwid  = 99999*100.0;
	 imaxdbz  = -99999*100.0,imindbz  = 99999*100.0;
	 imaxvel  = -99999*100.0,iminvel  = 99999*100.0;
	 imaxudbz = -99999*100.0,iminudbz = 99999*100.0;
	 imaxwid  = -99999*100.0,iminwid  = 99999*100.0;
         Buffer[7] = networkByteOrderShort(networkByteOrderShort(Buffer[7])+1);
	 Buffer[vangind] = networkByteOrderShort(short(scanPtr->gatedata[iaz].azimuth * 64));
	 //if ( debug )
	 //      fprintf(stdout,"uf::Az %4.1f el %4.1f %.2d:%.2d:%.2d %d\n",scanPtr->gatedata[iaz].azimuth,
	 //             scanPtr->gatedata[iaz].elev,hour,min,sec ,(int )scanPtr->gatedata[iaz].time);
	 Fhead[2] = networkByteOrderShort(scanPtr->startrange / 1000);
	 Fhead[3] = networkByteOrderShort(0);
	 Fhead[4] = networkByteOrderShort(scanPtr->rngres);
	 Buffer[5] = networkByteOrderShort(networkByteOrderShort(Buffer[5])+1);
	 Buffer[8] = networkByteOrderShort(1);
	 sfield = -1;
	 for ( ifield = 0; ifield < num ; ifield++ )
	 {
	     scale = fieldMatch[ifield].ufScale;
	     if ( fieldMatch[ifield].ufIndex == UFNONE ) continue;
	     sfield++;
	     scale = fieldMatch[ifield].ufScale;
	     strncpy((char *)&Buffer[62+2*sfield], fieldMatch[ifield].ufField,2);
	     if ( iaz == 0 && debug )
	     {
	        fprintf(stdout,"uf:: Field %2s src index %d gates %d ifield %d sfield %d uf scale %f\n",
		                fieldMatch[ifield].ufField,
		                sfield,field_len,ifield,sfield,scale);
	     }
	     Buffer[63+2*sfield] = networkByteOrderShort((62 + 2 * num + sfield*(32+field_len)) + 1);
	     if ( ifield == RAPICVEL )
	     {
	        oldvalu = short(scale);
	        atemp = short(Vu * scale);
		Fhead[19] = networkByteOrderShort(atemp);
	     }
	     memcpy((char *)&Buffer[62 + 2 *num + sfield*(32+field_len)],(char *)Fhead,64);
	     if ( ifield == RAPICVEL ) 
	          Fhead[19] = networkByteOrderShort(oldvalu);
	     if ( iaz == 0 && debug)
	       fprintf(stdout,"uf::Input Field '%s' outPutField '%s' numgates %d Elev %f Velsca %d\n",
	                       fieldMatch[ifield].rapField,fieldMatch[ifield].ufField,
			        networkByteOrderShort(Fhead[5]),(float )Buffer[fangind]/64.0,networkByteOrderShort(Fhead[19]));
	     Buffer[62 + 2 * num + sfield * (32 + field_len)] = 
	            networkByteOrderShort(62 + 2 * num + 32 + sfield *(32 + field_len) + 1);
	     Buffer[63 + 2 * num + sfield * (32 + field_len)] = networkByteOrderShort(short(scale));
	     for ( igate = 0; igate < field_len; igate++ )
	           Buffer[62 + 2*num + 32 + sfield*(32+field_len) + igate] = networkByteOrderShort(0);
	     for ( igate = 0; igate < scanPtr->gatedata[iaz].ngates; igate++ )
	     {
	         inp = scanPtr->gatedata[iaz].data[ifield][igate];
		 if ( inp != 0 )
		 {
		     if ( ifield == RAPICUZ || ifield == RAPICCZ )
		     {
			 dbz =    rapic2uz(scanPtr,scanPtr->gatedata[iaz].data[ifield][igate]);
			 Buffer[62 + 2*num + 32 + sfield*(32+field_len) + igate] =  networkByteOrderShort(short(dbz*scale));
			 dbz =  networkByteOrderShort(Buffer[62 + 2*num + 32 + sfield*(32+field_len) + igate]);
			 if ( ifield == RAPICUZ ) 
			 {
			     if ( dbz > UNDEFFIELD ) minudbz = min(minudbz,dbz);
			     maxudbz = max(maxudbz,dbz);
			     iminudbz = min(iminudbz,inp);
			     imaxudbz = max(imaxudbz,inp);
			 }
			 else
			 {
			     if ( dbz > UNDEFFIELD ) mindbz = min(mindbz,dbz);
			     maxdbz = max(maxdbz,dbz);
			     imindbz = min(imindbz,inp);
			     imaxdbz = max(imaxdbz,inp);
			 }
		     }
		     if ( ifield == RAPICVEL )
		     {
			 vel = rapic2vr(scanPtr,scanPtr->gatedata[iaz].data[ifield][igate],scanPtr->fieldlevel[ifield]);

			 TV =  vel;
			 vel = 
			 Buffer[62 + 2*num + 32 + sfield*(32+field_len) + igate] = networkByteOrderShort(short(TV * scale));
			 if ( vel > UNDEFFIELD ) minvel = min(minvel,vel);
			 maxvel = max(maxvel,vel);
			 iminvel = min(iminvel,inp);
			 imaxvel = max(imaxvel,inp);
		     }
		     if ( ifield == RAPICWID )
		     {
			 TV =  Vu*rapic2sw(scanPtr->gatedata[iaz].data[ifield][igate]);
			 wid = 
			 Buffer[62 + 2*num + 32 + sfield*(32+field_len) + igate] = networkByteOrderShort(short(TV * scale));
			 if ( wid > UNDEFFIELD ) minwid = min(minwid,wid);
			 maxwid = max(maxwid,wid);
			 iminwid = min(iminwid,inp);
			 imaxwid = max(imaxwid,inp);
		     }
		     if ( ifield == RAPICZDR )
		     {
			 Buffer[62 + 2*num + 32 + sfield*(32+field_len) + igate] = 
			       networkByteOrderShort(short(rapic2zdr(scanPtr->gatedata[iaz].data[ifield][igate])*scale));
		     }
		 }
		 else
		      Buffer[62 + 2*num + 32 + sfield*(32+field_len) + igate] = networkByteOrderShort(-32768);
	     } // For each gate
	     for ( igate = scanPtr->gatedata[iaz].ngates; igate < field_len ; igate++ )
	           Buffer[62 + 2*num + 32 + sfield*(32+field_len) + igate] = networkByteOrderShort(-32768);
	 } // For each Field 
	 Buffer[59] = networkByteOrderShort(sfield + 1);
	 Buffer[61] = networkByteOrderShort(sfield + 1);
	 rlen = (62 + 2*num + (sfield+1)*(32 + field_len));
	 Buffer[1] = networkByteOrderShort(rlen);
	 rlen *= 2;
         //write(fdout,(char *)&rlen,sizeof(long)); // dummy write. SD This damages the file!!

         pos = lseek(fdout,0L,SEEK_CUR);	    // remember the pos
	 if (writeRecByteLength)                    // write length of record in bytes before each record
	   {
	     rlen_nw = networkByteOrderInt(rlen);
	     write(fdout, &rlen_nw, sizeof(int));
	   }
	 write(fdout,(char *)Buffer,rlen);
	 if (writeRecByteLength)                    // write length of record in bytes after each record as well
	   {
	     write(fdout, &rlen_nw, sizeof(int));
	   }
         //write(fdout,(char *)&pos,sizeof(size_t)); // maintain back pointer. SD ditto!
	 /*
	 if ( debug ) 
	      fprintf(stdout,"mindbz %f maxdbz %f minvel %f maxvel %f nazimuths %d\n",
			      mindbz/16.0,maxdbz/16.0,minvel/16.0,maxvel/16.0,iaz);
	 if ( debug ) 
	      fprintf(stdout,"minudbz %f maxudbz %f minwid %f maxwid %f nazimuths %d\n",
			      minudbz/16.0,maxudbz/16.0,minwid/16.0,maxwid/16.0,iaz);
	 if ( debug ) 
	      fprintf(stdout,"imindbz %f imaxdbz %f iminvel %f imaxvel %f nazimuths %d\n",
			      imindbz/16.0,imaxdbz/16.0,iminvel/16.0,imaxvel/16.0,iaz);
	 if ( debug ) 
	      fprintf(stdout,"iminudbz %f imaxudbz %f iminwid %f imaxwid %f nazimuths %d\n",
			      iminudbz/16.0,imaxudbz/16.0,iminwid/16.0,imaxwid/16.0,iaz);
	 */
    } // For each ray  
}


void uf::exitUf(errs ind,char *msg2)
{
  if (  ind >= 0 && ind < ERRLAST )
  {
         fprintf(stderr,"uf::ERROR: %s %s Quitting\n",
	         errors[ind].msg,msg2);
	 exit(-1);  
  }
  fprintf(stderr,"uf::ERROR: %s %s Quitting\n",
	  "BUG-->" ,msg2);
  exit(-1);    
}

void uf::zeroData()
{

  memset((void *)&ufId,0,sizeof(ufHeader));
  memset((void *)&maHeaderData  , 0,sizeof(maHeader ));
  memset((void *)&opHeaderData  , 0,sizeof(opHeader ));
  memset((void *)&luHeaderData  , 0,sizeof(luHeader ));
  memset((void *)&fldHeaderData , 0,sizeof(fldHeader));
  memset((void *)&daHeaderData  , 0,sizeof(daHeader ));
  memset((void *)&firstFieldHeaderData  , 0,sizeof(firstFieldHeader ));
  
}

void uf::dumpData(struct voldata *vol)
{
  struct scan * scptr;
  struct gate * gptr;
  unsigned char *cptr;
  int ii,jj,kk,ll;
  int sz;
  int maxf ;
  float faz;
  long zero = 0;
  FILE*           dbfd;
  char   dbname[100] = "dump_scandata";
  
  printf("uf::dumpData: start dump to %s\n",dbname);
  
  dbfd   = fopen(dbname,"w");
  if ( dbfd == NULL )
  {
     fprintf(stderr,"uf::dumpData: Error in Opening the File %s\n",dbname);
     perror("uf::dumpData:");
     
     exit(-1);
  }
  
   maxf = min(MAXFIELDS,RAPICZDR);
   for ( ii = 0; ii < vol->nelevations;ii += 1)
   {
      scptr = &vol->scans[ii];
      
      if ( vol->scans[ii].field != RAPICNONE )
      {
         for ( jj = 0; jj < scptr->nazimuths; /* MAXAZS; */ jj++ )
	 {
	     gptr = &scptr->gatedata[jj];
	     faz   = gptr->azimuth;
	     sz   = gptr->ngates;
	     for ( kk = 0; kk < maxf; kk++ )
	     {
	        if ( sz > 0 )
		{
		    cptr = &gptr->data[kk][0];
		    zero = 0;
		    for ( ll = 0; ll < sz; ll++ )
		    {
		       zero += (cptr[ll]&0xFF);
		    }
		    if ( zero )
		    {
			fprintf(dbfd,"@%4.1f %.4d %.2d %4.1f ",
				faz,sz,kk,gptr->elev); // ii);
			cptr = &gptr->data[kk][0];
			for ( ll = 0; ll < sz; ll++ )
			{
			   zero += (cptr[ll]&0xFF);
			   fprintf(dbfd,"%x",cptr[ll]&0xFF);
			}
			fprintf(dbfd,"\n"); 
		    }
		}  
	     }	 
	 }
      }
   }
   
   fclose(dbfd);
   printf("uf::dumpData: end dump to %s\n",dbname);

}

