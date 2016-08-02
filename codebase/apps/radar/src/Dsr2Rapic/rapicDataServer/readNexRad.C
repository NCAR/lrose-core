/*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
 ** A demo program for reading China CINRAD/SA radar base data.
 ** Copyright Beijing METSTAR Radar CO., Ltd. (2005)
 ** All rights reserved. Licenced use only.
 ** Do not copy or distribute without authorization.
*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*/

/* Notwithstanding the above,  adapting to generate rapic file.
   S.Dance,  3 feb 2006
*/

/* 
   Make with makeNexRad in titandev2:/home/sandy/src/nowcast_sep05/rapic/utils
   ie:
------------------------------------------------------------------------
g++ -Dlinux -DLINUX -DSTDCPPHEADERS -DNO_XWIN_GUI -D_REENTRANT -g -Wall -DSTDCPPHEADERS -DHAVE_MALLINFO -DCHECK_RECSIZE_BUG -DUSE_DEFRAG_FN_ORIDE -DUSE_SIGACTION -DCOMPILE_READNEXRAD_MAIN -I/usr/include/bsd -I/usr/include/c++/3.2.2 -I../include -I/home/sandy/src/nowcast/rapic/include/ct_include.6.7 -DNO_XWIN_GUI -c -o readNexRad.o ../rapic/readNexRad.C

g++ -o readNexRad -O2 -pipe -march=i386 -mcpu=i686 -fno-strict-aliasing -pipe -L/usr/X11R6/lib expbuff.o radlcnvt.o radlpnt.o rdrglb.o scanmng.o threadobj.o replay.o version.o levelTable.o rdrscan.o juldate.o rdrutils.o utils.o siteinfo.o latlong.o log.o rdrxlat.o histogram.o freelist.o rdrfilter.o rdrInvent.o signals.o spinlock.o rxdevice.o txdevice.o commmng.o ipnamerescache.o comms.o db.o NexRad.o NexRadMgr.o NexRadStnHdlr.o uf.o ufStnHdlr.o ufMgr.o readNexRad.o rpEventTimer.o  -L/lib -L../lib -lpthread -lctreestd -lm 
------------------------------------------------------------------------

   run 3DRapic with  /opt/bin/run_3drapicbeta.sh
   .ini files are in /radar/3D-Rapic-beta/

   run lots based on runNexRad.sh.

*/

#include "rdrscan.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "uiftovrc.h"
#include <signal.h>
#include <sys/types.h>
#include <unistd.h>
#include <time.h>
#include <sys/stat.h>
#include "siteinfo.h"
#include "rdr.h"
#include "utils.h"
#include "rdrutils.h"
#include "radlcnvt.h"
#include "readNexRad.h"
#include <netinet/in.h>
#include <math.h>


readNexRad::readNexRad()
{
  HIST_SIZE = 100;

  RADIAN = 3.14159/180.;
  debug = 0;
  createdScan = NULL;
  dest_array = NULL;
  dest_array_size = 0;
  dest_xdim = dest_ydim = 0;
}

readNexRad::~readNexRad()
{
  for (int x = 0; x < int(nexradScans.size()); x++)
    delete nexradScans[x];  // deallocate nexradScanBuffs
  if (dest_array)
    delete[] dest_array;
}

unsigned short readNexRad::ftohs(int rtype, short in) 
{
  if (rtype == NXR_NEXRAD) return ntohs(in);
  else return (unsigned short) in;
}

short readNexRad::ftohss(int rtype, short in) 
{
  if (rtype == NXR_NEXRAD) return ntohs(in);
  else return (short) in;
}

unsigned long readNexRad::ftohl(int rtype, long in) 
{
  if (rtype == NXR_NEXRAD) return ntohl(in);
  else return in;
}

void readNexRad::newElevScan(int elindex, nexradRadial &newradial)
{
  nexradScanBuff *newscan = NULL;
  if ((elindex+1) >= int(nexradScans.size()))
    {
      int prevsize = nexradScans.size();
      nexradScans.resize(elindex+1);
      for (int i = prevsize; i < (elindex+1); i++)
	{
	  std::cout << "readNexRad::newElevScan - adding new elev scan #" << 
	    i << endl;
	  newscan = new nexradScanBuff(newradial);
	  nexradScans[i] = newscan;
	}
    }
  nexradScans[elindex]->reset(newradial);
}

bool readNexRad::ReadNexradData(char *filename, char *radtype,
				 int station, bool createRapicScan)
{
  FILE *fp=0;
  int scan_count = 0;
  int elev_count = 0;
  int radial_count = 0;
	
	
//   int FstBin=0,LstBin,BinNum;
//   float CurAz,CurEl;
//   int ElIndex,AzIndex,BnIndex;
//   int ptrPos;
//   int RBinNum =0, VBinNum =0;

  int ElIndex;
  float CurEl;

  bool fileEndFlag = false;

  bool VolBeg=false;
  bool VolEnd=false;

  int rtype = NXR_CINRAD;  //radar type, NEXRAD or CINRAD
	
  int elevHist[HIST_SIZE];
  float elevValues[HIST_SIZE];
  int elevHistCount = 0;

  int rfd = -1;		     
	
	

  fp = fopen(filename,"rb");
  if(fp==0) {
    fprintf(stderr,"readNexRad::ReadNexradData: Error opening file %s\n",
	    filename);
    return false;
  }
	
  if (!strcmp(radtype,"NEXRAD")) 
    {
      if (debug) printf("Radar type is NEXRAD\n");
      oneRadial.nrtype = NRT_NEXRAD;
      rtype = NXR_NEXRAD;	  
    }
  else
    oneRadial.nrtype = NRT_CINRAD;

//   while (!fileEndFlag)
//     {
//       if (oneRadial.readRadial(fp))
// 	{
// 	  radial_count++;
// 	  if (debug >= 2) oneRadial.dumpHeader(debug);
// 	  fileEndFlag = oneRadial.endOfVol() || feof(fp);
// 	}
//       else
// 	fileEndFlag = true;
//     }
//   return true;
	    
  //Initialize array
  for(ElIndex=0; ElIndex<int(nexradScans.size()); ElIndex++)
    {
      nexradScans[ElIndex]->clear(VALUE_INVALID);
    }//end el

  //init histogram
  for (int i=0;i<100;i++) {
    elevHist[i] = 0;
    elevValues[i] = 0.0;
    elevHistCount = 0;
  }

  bool radlReadOK = false;
  do
    {
      //Initialize flags
      if (!(radlReadOK = oneRadial.readRadial(fp)))
	{
	  fprintf(stdout, "readNexRad::ReadNexradData: "
		  "Failed reading radial from file - %s - Aborting read\n",
		  filename);
	  break;
	}
      fileEndFlag = !radlReadOK || feof(fp) || oneRadial.endOfVol();
      radial_count++;
      if (debug >= 2) oneRadial.dumpHeader(debug);
		
      //Start a volume scan
      if(oneRadial.header.radlStatus() == NRS_BEGIN_VOL) 
	{
	  ElIndex=0;
	  newElevScan(ElIndex, oneRadial);
	  CurEl = oneRadial.header.el();
	  
	  VolBeg = true;
	  if (debug) printf("VCP number is %3d\n",
			    oneRadial.header.vcpNum);

	  //assumes julian date is days from 1/1/70,  poss, year + day count???
	  startTime = oneRadial.header.radlTime();
	  if (debug) printf("julian date is %d, unix time %s\n",
			    oneRadial.header.radlJulDate,
			    ctime(&startTime));
			
	}
      //Find the beginning of the volume scan, like skip RDA status data.
      if(!VolBeg) continue;		

      CurEl = oneRadial.header.el();

      //Start an elevation
      if((oneRadial.header.radlStatus() == NRS_NEW_ELEV) || 
	 (oneRadial.header.radlStatus() == NRS_END_VOL))  
	{
	  //looks like the Chinese radar bounces around when it starts a new elevation,  often starting
	  //0.1 degrees higher than its usual value.  Should I take a histogram to get the best elevation?  Yes.
	  if((rtype == NXR_NEXRAD && CurEl-nexradScans[ElIndex]->elev > 0.1) || 
	     (rtype == NXR_CINRAD && CurEl-nexradScans[ElIndex]->elev > 0.4))
	    {//different elevation angle - set prev elev from histogram value
	      nexradScans[ElIndex]->elevFromHist = 
		getBestElev(elevHist, elevValues, elevHistCount);

	      //reset histogram
	      for (int i=0;i<100;i++) {
		elevHist[i] = 0;
		elevValues[i] = 0.0;
		elevHistCount = 0;
	      }
	      ElIndex++;
	      newElevScan(ElIndex, oneRadial);
	    }
	  if (debug) 
	    {
	      if (oneRadial.header.radlStatus() == NRS_NEW_ELEV)
		printf("Elevation %3d  (%5.2f Degree) start...\n",
		       ElIndex+1,CurEl);
	      else if (oneRadial.header.radlStatus() == NRS_END_VOL)
		printf("END OF VOLUME DETECTED - Final Elev Count=%d\n",
		       ElIndex+1);
	    }
		
	}

      addElevHist(CurEl, elevHist, elevValues, &elevHistCount);
			
      // set last elev from histogram value
      if(oneRadial.endOfVol()) {
	nexradScans[ElIndex]->elevFromHist = 
	  getBestElev(elevHist, 
		      elevValues, elevHistCount);
	VolEnd=true;
      }
      
      if (nexradScans[ElIndex])
	nexradScans[ElIndex]->addRadl(oneRadial);
      
		
    }
  while(fileEndFlag==0 && !VolEnd);

  fclose(fp);
  if ((nexradScans.size() == 0) ||
      (!nexradScans[0]))
    return false;

  // finished reading nexrad into nexradScans buffers
  
      

  //assumes julian date is days from 1/1/70,  poss, year + day count???
  lastTime = oneRadial.radlTime();

  //now write out rapic data!!
  if (debug) fprintf(stdout,"Radial count is %d\n",radial_count);
  //open file
  if (!createRapicScan)
    {
      strcat(filename,".rapic");
      rfd = open(filename, O_RDWR+O_CREAT, 0664);
      if (rfd < 0) {
	fprintf(stderr, "readNexRad::ReadNexradData FAILED opening %s \n",
		filename);
	perror(0);
      }
      else fprintf(stdout, 
		   "readNexRad::ReadNexradData opening output RAPIC file %s\n",
		   filename);
    }

  //recompute elevation count to eliminate bad ones  SD 17May07
  int total_elev_count = ElIndex+1;
  int bad_elev_count = 0;
  int total_scan_count = 0;
  for(ElIndex=0; ElIndex<total_elev_count; ElIndex++)
    {
      if (!nexradScans[ElIndex] ||
	  (nexradScans[ElIndex]->elevFromHist == VALUE_INVALID))
	bad_elev_count++;
      else
	{
	  if (nexradScans[ElIndex]->reflBins)
	    total_scan_count++;
	  if (nexradScans[ElIndex]->doppBins)
	    total_scan_count += 2;  // vel & spwid
	}
    }
  elev_count = total_elev_count - bad_elev_count;
  
	
  if (debug) fprintf(stdout,"Total good elevations is %d, total scans is %d, bad elevations is %d\n",
		     elev_count, total_scan_count, bad_elev_count);

  scan_count = 1;  //label of scan starts at 1
  
  for(ElIndex=0; ElIndex<total_elev_count; ElIndex++)
    {
      //skip bad elevations  SD 17May07
      if (!nexradScans[ElIndex] ||
	  (nexradScans[ElIndex]->elevFromHist == VALUE_INVALID))
	continue;
      nexradScans[ElIndex]->fillMissingRadials();
      scan_count += 
	createScanProducts(rfd, ElIndex, elev_count,
			   scan_count, total_scan_count,
			   station, createRapicScan);
    }
	

  if(!VolEnd && VolBeg)
    {
      fprintf(stderr,"Error! Incomplete Volume Scan\n");
      return false;
    }

  NumValidCuts = ElIndex+1;
  if (!createRapicScan && (rfd >= 0))
    close(rfd);
  return true;
}

// void readNexRad::fillMissingRadials(bool *flag, float *data, int elev_count, int numGates) 
// {
  
//   //fill in any missing radials provided they are singletons, ie have immediate valid neighbours.
//   int precAz = 0;
//   int nextAz = 0;
//   int ElIndex, AzIndex, BnIndex;
  
  
//   for(ElIndex=0; ElIndex<elev_count; ElIndex++)
//     {
//       for (AzIndex=0;AzIndex<MaxRads;AzIndex++) {
// 	precAz = AzIndex -1;
// 	nextAz = AzIndex + 1;
// 	if (precAz <0) precAz = 359;
// 	if (nextAz > 359) nextAz = 0;
	  
// 	if (!flag[ElIndex*MaxRads+AzIndex] && flag[ElIndex*MaxRads+precAz] && 
// 	    flag[ElIndex*MaxRads+nextAz]) { 
// 	  //interpolate data into missing radial 
// 	  for (BnIndex=0;BnIndex<numGates;BnIndex++) {
// 	    if (data[ElIndex*MaxRads*RGatesMem+precAz*RGatesMem+BnIndex] != 
// 		VALUE_INVALID &&
// 		data[ElIndex*MaxRads*RGatesMem+precAz*RGatesMem+BnIndex] != 
// 		VALUE_RANFOLD &&
// 		data[ElIndex*MaxRads*RGatesMem+nextAz*RGatesMem+BnIndex] != 
// 		VALUE_INVALID &&
// 		data[ElIndex*MaxRads*RGatesMem+nextAz*RGatesMem+BnIndex] != 
// 		VALUE_RANFOLD)
// 	      data[ElIndex*MaxRads*RGatesMem+AzIndex*RGatesMem+BnIndex] = 
// 		(data[ElIndex*MaxRads*RGatesMem+precAz*RGatesMem+BnIndex] + 
// 		 data[ElIndex*MaxRads*RGatesMem+nextAz*RGatesMem+BnIndex])/2.0;
// 	  }
// 	}
//       }
//     }
// }


void readNexRad::addElevHist(float elev, int elevHist[], float elevValues[], int *elevCount) 
{
  int ind = 0;
  
  //get index of elev
  for (ind=0;ind<*elevCount;ind++) {
    if (elev == elevValues[ind]) break;
  }
  if (ind >= *elevCount && ind < HIST_SIZE-1) {
    *elevCount = ind+1;
    elevValues[ind] = elev;
  }
  if (ind >= HIST_SIZE-1) 
    fprintf(stderr, 
	    "readNexRad::addElevHist: ERROR, histogram array size exceeded\n");
  
  elevHist[ind]++;
}

float readNexRad::getBestElev(int elevHist[], float elevValues[], int elevCount) 
{
  //return the most common elevation in histogram
  int maxCount = -1;
  int indexOfMaxSoFar = 0;
  
  if (elevCount == 0) {
    fprintf(stderr,"readNexRad::getBestElev: ERROR, histogram size is zero\n");
    return -999.0;
  }
  
  
  for (int ind=0;ind<elevCount;ind++) {
    if (elevHist[ind] > maxCount){
      maxCount = elevHist[ind];
      indexOfMaxSoFar = ind;
    }
  }
  return elevValues[indexOfMaxSoFar];
}
  

void readNexRad::derefCreatedScan()
{
  if (createdScan && 
      createdScan->ShouldDelete(this, "readNexRad::createScanProduct"))
    delete createdScan;
  createdScan = NULL;
} 

int readNexRad::createScanProducts(int rfd,
				   int elindex, int total_tilts,
				   int scan_count, int total_scans,
				   int station, bool createRapicScan)
{
  int scans_added = 0;
  float rngtofirstgate;
  
  nexradScanBuff *scanbuff = nexradScans[elindex];
  if (!scanbuff) return scans_added;
  if (scanbuff->reflBins) 
    {
      rngtofirstgate = scanbuff->rngToFirstReflGate;
      if (rngtofirstgate < 0)
	rngtofirstgate *= -1.0;
      createScanProduct(rfd, 
			scanbuff->reflData.begin(),
			e_refl,
			scanbuff->numRadls, 
			scanbuff->reflBins,  
			scanbuff->reflGateSize,
			int(round(rngtofirstgate)),
			scanbuff->nyquist,
			elindex+1,
			total_tilts,
			scan_count++,
			total_scans,
			scanbuff->elevFromHist,
			scanbuff->elevTime,
			station, createRapicScan);
      scans_added++;
    }
  if (scanbuff->doppBins) 
    {
      rngtofirstgate = scanbuff->rngToFirstDoppGate;
      if (rngtofirstgate < 0)
	rngtofirstgate *= -1.0;
      createScanProduct(rfd, 
			scanbuff->velData.begin(),
			e_vel,
			scanbuff->numRadls, 
			scanbuff->doppBins,
			scanbuff->doppGateSize,
			int(round(rngtofirstgate)),
			scanbuff->nyquist,
			elindex+1,
			total_tilts,
			scan_count++,
			total_scans,
			scanbuff->elevFromHist,
			scanbuff->elevTime,
			station, createRapicScan);
      scans_added++;
      createScanProduct(rfd, 
			scanbuff->spwidData.begin(),
			e_spectw,
			scanbuff->numRadls, 
			scanbuff->doppBins,
			scanbuff->doppGateSize,
			int(round(rngtofirstgate)),
			scanbuff->nyquist,
			elindex+1,
			total_tilts,
			scan_count++,
			total_scans,
			scanbuff->elevFromHist,
			scanbuff->elevTime,
			station, createRapicScan);
      scans_added++;
    }
  return scans_added;
}

void readNexRad::createScanProduct(int rfd, 
				   float_vec_iter data_iter,
				   e_data_type type, 
				   int numRadls, 
				   int numGates, int gateSize, 
				   int rngToFirstGate,
				   float nyquist,
				   int tilt_num, int total_tilts,
				   int scan_count,int total_scans,
				   float ElevAngle,time_t scanTime,
				   int station, bool createRapicScan)
  //create rapic scan data from nexrad
{
  //bool addMoment = true;
  char headerStr[100];
  int headerStrLen = 0;
  float reflMin = -31.5;   //dBz
  float reflMax = 95.5;
  int numLevels = 256;
  float reflInc = (reflMax - reflMin)/(numLevels -2);
  
  if (debug) printf("first gate range %d, numGates %d, gatesize %d"
		    ", data type %d\n"
		    "tilt num %d of %d scan num %d of %d angle %f\n",
		    rngToFirstGate,numGates,gateSize,
		    type,tilt_num,total_tilts,
		    scan_count, total_scans, ElevAngle);
  
  rdr_scan *invented_scan = 0;  

  // biggest required dest_array is kept until instance deleted

  // check if need bigger array, if so delete existing
  if ((numGates * numRadls) > dest_array_size)
    {
      if (dest_array)
	delete[] dest_array;
      dest_array_size = dest_xdim = dest_ydim = 0;
    }

  // allocate array if required
  if (!dest_array) {
    dest_array_size = numGates * numRadls;
    dest_array = new unsigned char[dest_array_size];
  }
  
  dest_xdim = numGates;
  dest_ydim = numRadls;
  memset(dest_array, 0, numGates * numRadls);  // del reqd space in dest_array
    
  //access data so: data_iter[AzIndex][BnIndex]
  int temp;
    
  for (int az = 0;az<numRadls;az++) {
    //convert radial into compressed rapic format
    for (int gate = 0; gate<numGates;gate++) {
      switch (type) {
      case e_refl:
	if (*data_iter < reflMin || 
	    *data_iter > reflMax ||
	    *data_iter == VALUE_INVALID) //not nec. since -999 < reflMin
	  dest_array[az*numGates+gate] = (unsigned char)0;
	else {
	  temp = (int)((*data_iter - reflMin)/reflInc + 1);
	  dest_array[az*numGates+gate] = (unsigned char)(temp & 0xff);
	}
	break;
      case e_vel:
      case e_spectw:
	if (*data_iter == 0 || 
	    *data_iter == 1 ||
	    *data_iter == VALUE_RANFOLD ||
	    *data_iter == VALUE_INVALID)
	  temp = 0;
	else 
	  {
	    float nyq_fraction = *data_iter / nyquist;
	    temp = (int)((nyq_fraction*(numLevels/2-1)) + numLevels/2);
	  }
// 	else temp = (int)((*data_iter*(numLevels/2-1))/nyquist + numLevels/2);
	dest_array[az*numGates+gate] = (unsigned char)(temp & 0xff);
	break;
      default:
	fprintf(stderr,"createScanProduct: ERROR, unknown data type %d\n",type);
	dest_array[az*numGates+gate] = (unsigned char)0;
      }
      data_iter++;
    }
    //got radial of uncompressed rapic data, 
  }
  //got scan of umcompressed rapic data
  
  if (createRapicScan)
    {
      if (scan_count == 1)
	{
	  derefCreatedScan();
	  createdScan = new rdr_scan(this, "readNexRad::createScanProduct");
	  invented_scan = createdScan;
	}
      else
	{
	  invented_scan = 
	    new rdr_scan(NULL, "readNexRad::createScanProduct", createdScan);
	}
    }
  else
    invented_scan = new rdr_scan(NULL, "readNexRad::createScanProduct");
  //invented_scan->set_dflt(srcscan);  replace with values from China

  invented_scan->station = station;
  invented_scan->FirstTm = startTime;
  invented_scan->LastTm  = lastTime;
  invented_scan->scan_time_t = scanTime;
  UnixTime2DateTime(invented_scan->scan_time_t,invented_scan->year,
		    invented_scan->month,invented_scan->day,
		    invented_scan->hour,invented_scan->min,invented_scan->sec);

  memset(&invented_scan->rdr_params, 0, sizeof(invented_scan->rdr_params));
  strcpy(invented_scan->rdr_params.name,"Chinese radar");	// distinctive default
  //may need other params like: wavelength 10ths of centimetres; rdr_angle, az_beamwidth, el_beamwidth; in 10ths of degree
  invented_scan->scan_type = VOL;
  invented_scan->data_type = type;		
  invented_scan->bfdata_type = bf_none;
  invented_scan->radltype = SIMPLE;
  invented_scan->data_fmt = RLE_8BIT;	
  invented_scan->NumLevels = numLevels ;
  invented_scan->angle_res = 10;				// default 1deg res.
  invented_scan->frequency = 0;
  invented_scan->rng_res = gateSize;				// meters
  invented_scan->start_rng = rngToFirstGate;
  invented_scan->max_rng = (rngToFirstGate+(gateSize*numGates))/1000;// km
  invented_scan->num_radls = 360;				// none yet
  invented_scan->radl_pos = 0;
  invented_scan->vol_scans = total_scans;
  invented_scan->vol_tilts = total_tilts;
  //invented_scan->num_scans = total_scans;
  invented_scan->volume_id = 0;      // id=-1, no explicit id defined
  if (ElevAngle != VALUE_INVALID)
    invented_scan->set_angle = int(round(ElevAngle*10.0));      //elevation angle in 10th of degree. Round to nearest .1 deg
  else
    invented_scan->set_angle = -99;  // use -9.9deg as invalid
  invented_scan->prf = 0;
  invented_scan->completed_tilts = tilt_num;  //??
  invented_scan->previous_tilt_angle = -9999;
  memset(&(invented_scan->comp_params), 0, sizeof(invented_scan->comp_params));
  invented_scan->ScanSetComplete = FALSE;
  invented_scan->ScanFinished = FALSE;
  invented_scan->nyquist = nyquist;
  invented_scan->ShownToScanClients = FALSE;
  invented_scan->data_source = RADARCONVERT;

  //readjust scan counts for new moment
  //assume there is velocity,  but that we dont know number of elevations
  invented_scan->vol_scan_no = scan_count;
    
  if (!createRapicScan)  // not needed if creating rdr_scan instance instead of file
    {
      //write out /IMAGE headers, not used by rapic, but by rapic2keyvals.pl
      if (scan_count == 1)   //test for first call, so root scan
	{
	  headerStrLen = sprintf(headerStr,"/IMAGE: %4d %02d%02d%02d%02d%02d\n/RXTIME:\n/IMAGESCANS: %d\n/IMAGESIZE:\n",
				 invented_scan->station,
				 invented_scan->year%100,
				 invented_scan->month,
				 invented_scan->day,
				 invented_scan->hour,
				 invented_scan->min,
				 invented_scan->vol_scans);
	  if (!write(rfd,headerStr,headerStrLen)) 
	    fprintf(stderr, "rdrInvent.createScanProduct: ERROR writing /IMAGE: line\n");
	
	  //finish with end header /IMAGEHEADER END:
	  if (!write(rfd,"/IMAGEHEADER END:\n\0",18)) 
	    fprintf(stderr, "rdrInvent.createScanProduct: ERROR writing : /IMAGEHEADER END: line\n");
	}    //end if rootscan 
      invented_scan->WriteDataFrom2DPolarArray(rfd, dest_array, 
					       dest_xdim, dest_ydim); 
    }
  else
    { 
      if (createdScan)
	{
	  createdScan->EncodeDataFrom2DPolarArray(dest_array,
						  dest_xdim, dest_ydim,
						  invented_scan);
	  createdScan->vol_tilts = 
	    createdScan->completedTilts();       // update tilt count
	  invented_scan->vol_tilt_no = 
	    createdScan->thisTilt(invented_scan); // set tilt no in scan
	}
    }    
  if ((!createRapicScan || (scan_count > 1)) &&
      (invented_scan->ShouldDelete(NULL,  "readNexRad::createScanProduct")))
    delete invented_scan;
  
  if (dest_array) {
    delete[] dest_array;
    dest_array = 0;
  }
  return ;
}

void nexradTitleBlock::clear()
{
  strcpy(fname_root, "undef   ");
  memset(fname_ext, '0', 3);
  julDate = mSeconds = 0;
  memset(unused, '\0', 4);
}

char* nexradRadlStatusStrings[] =
  {
    "New Elev", "Intermediate Radl", "End Elev",
    "New Vol", "End Vol", "undef"
  };

char* nexradHeader::radlStatusStr()
{
  if (_radlStatus <= 4)
    return nexradRadlStatusStrings[_radlStatus];
  else
    return nexradRadlStatusStrings[5];
}

void nexradHeader::dumpHeader(int debug) 
{
  if ((debug >= 3) ||
      ((debug == 2) && (radlNumber == 1)))
    printf("     mssgSize;                  %d  \n"
	   "     chID_mssgType              %d  \n"
	   "     seqID;                     %d  \n"                     
	   "     mssgJulianDate;            %d  \n"
	   "     mssgMSeconds;              %d  \n"
	   "     mssgSegments;              %d  \n"
	   "     mssgSegment;               %d  \n"
	   "     radlMSeconds;              %d  \n"
	   "     radlJulDate;               %d  \n"
	   "     unambRange;                %1.1f\n"
	   "     az;                        %d  \n"
	   "     real Az;                   %1.3f\n"
	   "     radlNumber;                %d  \n"
	   "     radlStatus;                %d - %s\n"
	   "     el;                        %d  \n"
	   "     real el;                   %f  \n"
	   "     elNumber;                  %d  \n"
	   "     rngToFirstReflGate;        %dm \n"
	   "     rngToFirstDoppGate;        %dm \n"
	   "     reflGateSize;              %dm \n"
	   "     doppGateSize;              %dm \n"
	   "     reflNumGates;              %d  \n"
	   "     doppNumGates;              %d  \n"
	   "     cutSectorNumber;           %d  \n"
	   "     calConst;                  %d \n"
	   "     reflPtr;                   %d  \n"
	   "     velPtr;                    %d  \n"
	   "     spwidPtr;                  %d  \n"
	   "     velRes;                    %1.1f\n"
	   "     vcpNum;                    %d  \n"
	   "     reflPtrArch;               %d  \n"
	   "     velPtrArch;                %d  \n"
	   "     spwidPtrArch;              %d  \n"
	   "     nyquist;                   %1.1f\n"
	   "     atmAttenFactor;            %f  \n"
	   "     rngAmbigThresh;            %f  \n"
	   "     CircleTotal;               %d  \n\n",
           mssgSizeBytes,
	   chID_mssgType,
	   seqID,
	   mssgJulDate,
	   mssgMSeconds,
	   mssgSegments,
	   mssgSegment,
	   radlMSeconds,
	   radlJulDate,
	   unambRange(),
	   _az,
	   az(),
	   radlNumber,
	   _radlStatus, radlStatusStr(),
	   _el, 
	   el(),
	   elNumber,
	   rngToFirstReflGate,
	   rngToFirstDoppGate,
	   reflGateSize,
	   doppGateSize,
	   reflNumGates,
	   doppNumGates,
	   cutSectorNumber,
	   calConst,
	   reflPtr,
	   velPtr,
	   spwidPtr,
	   velRes(),
	   vcpNum,
	   reflPtrArch,
	   velPtrArch,
	   spwidPtrArch,
	   nyquist(),
	   atmAttenFactor(),
	   rngAmbigThresh(),
	   circleTotal);
  else if (debug >= 2)
    printf("     Az=%f  El=%f RadialNumber=%d ElNumber=%d\n",
	   az(),
	   el(),
	   radlNumber,
	   elNumber);
}

void nexradRadial::dumpHeader(int debug) 
{
  if (headerValid)
    header.dumpHeader(debug);
}

void nexradHeader::fixByteOrder(nexradType nrtype, int maxNumGates)
{
  mssgSizeBytes = ftohs(nrtype, mssgSizeBytes);
  seqID = ftohs(nrtype, seqID);
  mssgJulDate = ftohs(nrtype, mssgJulDate);
  mssgMSeconds = ftohl(nrtype, mssgMSeconds);
  mssgSegments = ftohs(nrtype, mssgSegments);
  mssgSegment = ftohs(nrtype, mssgSegment);
  radlMSeconds = ftohl(nrtype, radlMSeconds);
  radlJulDate = ftohs(nrtype, radlJulDate);
  _unambRange = ftohs(nrtype, _unambRange);
  _az = ftohs(nrtype, _az);
  radlNumber = ftohs(nrtype, radlNumber);
  _radlStatus = ftohs(nrtype, _radlStatus);
  _el = ftohs(nrtype, _el);
  elNumber = ftohs(nrtype, elNumber);
  rngToFirstReflGate = ftohss(nrtype, rngToFirstReflGate);
  rngToFirstDoppGate = ftohss(nrtype, rngToFirstDoppGate);
  reflGateSize = ftohs(nrtype, reflGateSize);
  doppGateSize = ftohs(nrtype, doppGateSize);
  reflNumGates = ftohs(nrtype, reflNumGates);
  if (reflNumGates > maxNumGates)
    {
      fprintf(stdout, "nexradHeader::fixByteOrder - Limiting reflNumGates "
	      "to %d from %d\n",
	      maxNumGates, reflNumGates);
      reflNumGates = maxNumGates;
    }      
  doppNumGates = ftohs(nrtype, doppNumGates);
  if (doppNumGates > maxNumGates)
    {
      fprintf(stdout, "nexradHeader::fixByteOrder - Limiting doppNumGates "
	      "to %d from %d\n",
	      maxNumGates, doppNumGates);
      doppNumGates = maxNumGates;
    }
  cutSectorNumber = ftohs(nrtype, cutSectorNumber);
  calConst = ftohl(nrtype, calConst);
  reflPtr = ftohs(nrtype, reflPtr);
  velPtr = ftohs(nrtype, velPtr);
  spwidPtr = ftohs(nrtype, spwidPtr);
  _velRes = ftohs(nrtype, _velRes);
  vcpNum = ftohs(nrtype, vcpNum);
  reflPtrArch = ftohs(nrtype, reflPtrArch);
  velPtrArch = ftohs(nrtype, velPtrArch);
  spwidPtrArch = ftohs(nrtype, spwidPtrArch);
  _nyquist = ftohs(nrtype, _nyquist);
  _atmAttenFactor = ftohs(nrtype, _atmAttenFactor);
  _rngAmbigThresh = ftohs(nrtype, _rngAmbigThresh);
  circleTotal = ftohs(nrtype, circleTotal);
}

bool nexradHeader::sanityCheckOK()
{
  if (!((_velRes == 2) || (_velRes == 4)))
    {
      fprintf(stdout, "nexradHeader::sanityCheckOK failed - "
	      "velRes (should be 2 or 4)=%d\n",
	      _velRes);
      return false;
    }
  else if (_radlStatus > 4)
    {
      fprintf(stdout, "nexradHeader::sanityCheckOK failed - "
	      "Bad radlStatus>4=%d\n",
	      _radlStatus);
      return false;
    }
  else if (reflNumGates > 4096)
    {
      fprintf(stdout, "nexradHeader::sanityCheckOK failed - "
	      "reflNumGates too large=%d\n",
	      reflNumGates);
      return false;
    }
  else if (doppNumGates > 4096)
    {
      fprintf(stdout, "nexradHeader::sanityCheckOK failed - "
	      "doppNumGates too large=%d\n",
	      doppNumGates);
      return false;
    }
  else
    return true;
}

bool nexradRadial::readHeader(FILE *nexradfile)
{
  if (!nexradfile)
    return false;
  int headersize = sizeof(header);
  if (headersize != 128)
    {
      fprintf(stderr, "nexradRadial::readHeader - Compile error\n"
	      " sizeof(header) = %d, it should be 128\n"
	      " Aborting nexrad file read\n",
	      headersize);
      return false;
    }
  int readSize = fread(&header, 1, headersize, nexradfile);
  if (readSize < headersize)
    return false;
  header.fixByteOrder(nrtype, maxNumGates);
  if (!header.sanityCheckOK())
    return false;
  return true;
}

bool nexradRadial::readData(FILE *nexradfile)
{
  long endHeaderPos = ftell(nexradfile);
  int readSize = 0;
  if (header.reflNumGates > ReflBuffSize)
    {
      if (ReflData) delete[] ReflData;
      ReflBuffSize = header.reflNumGates;
      ReflData = new unsigned char[ReflBuffSize];
    }
  if (header.reflPtr)
    {
      fseek(nexradfile, endHeaderPos + header.reflPtr - 100, SEEK_SET);
      readSize = fread(ReflData, 1, header.reflNumGates, nexradfile);
      if (readSize < header.reflNumGates)
	return false;
    }
  if (header.doppNumGates > VelBuffSize)
    {
      if (VelData) delete[] VelData;
      VelBuffSize = header.doppNumGates;
      VelData = new unsigned char[VelBuffSize];
    }
  if (header.velPtr)
    {
      fseek(nexradfile, endHeaderPos + header.velPtr - 100, SEEK_SET);
      readSize = fread(VelData, 1, header.doppNumGates, nexradfile);
      if (readSize < header.doppNumGates)
	return false;
    }
  if (header.doppNumGates > SWBuffSize)
    {
      if (SWData) delete[] SWData;
      SWBuffSize = header.doppNumGates;
      SWData = new unsigned char[SWBuffSize];
    }
  if (header.spwidPtr)
    {
      fseek(nexradfile, endHeaderPos + header.spwidPtr - 100, SEEK_SET);
      readSize = fread(SWData, 1, header.doppNumGates, nexradfile);
      if (readSize < header.doppNumGates)
	return false;
    }
  return true;
}

bool nexradRadial::readRadial(FILE *nexradfile)
{
  if (!nexradfile)
    return false;
  int readSize = 0;
  long startpos = ftell(nexradfile);

  titleBlockValid = false;
  if (nrtype == NRT_NEXRAD)
    {
      readSize = fread(&titleBlock, 1, sizeof(titleBlock), nexradfile);
      if (readSize < int(sizeof(titleBlock)))
	return false;
      titleBlockValid = true;
    }

  headerValid = false;
  if (!readHeader(nexradfile))
    return false;
  headerValid = true;

  if (!readData(nexradfile))
    return false;
  // if not end of vol, point to next radial
  if (!endOfVol())
    fseek(nexradfile, startpos + header.mssgSizeBytes, SEEK_SET);
  return true;
}

bool nexradRadial::endOfVol()
{
  return (header.radlStatus() == NRS_END_VOL);
}

nexradRadial::nexradRadial()
{
  titleBlockValid = headerValid = false;
  nrtype = NRT_NEXRAD;
  ReflData =
    VelData = 
    SWData = NULL;
  ReflBuffSize =
    VelBuffSize = 
    SWBuffSize = 0;
  maxNumGates = 2048;
}

nexradRadial::~nexradRadial()
{
  if (ReflData)
    delete[] ReflData;
  if (VelData)
    delete[] VelData;
  if (SWData)
    delete[] SWData;
}

unsigned short nexradHeader::ftohs(nexradType nrtype, short in) 
{
  if (nrtype == NRT_NEXRAD) return ntohs(in);
  else return (unsigned short) in;
}

short nexradHeader::ftohss(nexradType nrtype, short in) 
{
  if (nrtype == NRT_NEXRAD) return ntohs(in);
  else return (short) in;
}

unsigned long nexradHeader::ftohl(nexradType nrtype, long in) 
{
  if (nrtype == NRT_NEXRAD) return ntohl(in);
  else return in;
}

nexradScanBuff::nexradScanBuff(int reflbins, int doppbins, int numradls)
{
  init(reflbins, doppbins, numradls);
}

nexradScanBuff::nexradScanBuff(nexradRadial &newradial, int numradls)
{
  init(newradial.header.reflNumGates, 
       newradial.header.doppNumGates, numradls);
}

nexradScanBuff::~nexradScanBuff()
{
}
  
void nexradScanBuff::init(int reflbins, int doppbins, int numradls)
{
  elev = elevFromHist = 0.0;
  elevTime = 0;
  reflBins = doppBins = numRadls = 0;
  reflGateSize =   
    doppGateSize = 0;
  rngToFirstReflGate =   
    rngToFirstDoppGate = 0;
  nyquist = 0;
  resize(reflbins, doppbins, numradls);
  lastClearVal = 0.0;
  clear();
}

void nexradScanBuff::resize(int reflbins, int doppbins, int numradls)
{
  if (numradls > 0)  // resize both refl and dopp buffs
    {
      numRadls = numradls;
      resizeRefl(reflbins, numradls);
      resizeDopp(doppbins, numradls);
    }
  else
    {
      if ((reflbins > 0) &&
	  (reflbins != reflBins))
	resizeRefl(reflbins, numradls);
      if ((doppbins > 0) &&
	  (doppbins != doppBins))
	resizeDopp(doppbins, numradls);
    }
}

void nexradScanBuff::resizeNumRadls(int numradls)
{
  if (numradls > 0)  // resize both refl and dopp buffs
    {
      resizeRefl(reflBins, numradls);
      resizeDopp(doppBins, numradls);
      numRadls = numradls;
    }
}  

void nexradScanBuff::resizeRefl(int reflbins, int numradls)
{
  if (((reflbins > 0) &&
       (reflbins != reflBins)) ||
      ((numradls > 0)))  // always set vals if numradls defined
    {
      if (reflbins > 0)
	reflBins = reflbins;
      if (numradls > 0)
	{
	  numRadls = numradls;
	  reflRadlFlags.resize(numRadls);
	}
      if ((numRadls * reflBins) > int(reflData.size()))  // only ever increase size
	reflData.resize(numRadls * reflBins);
    }
}

void nexradScanBuff::resizeDopp(int doppbins, int numradls)
{
  if (((doppbins > 0) &&
       (doppbins != doppBins)) ||
      ((numradls > 0)))  // always set vals if numradls defined
    {
      if (doppbins > 0)
	doppBins = doppbins;
      if (numradls > 0)
	{
	  numRadls = numradls;
	  velRadlFlags.resize(numRadls);
	  spwidRadlFlags.resize(numRadls);
	}
      if ((numRadls * doppBins) > int(velData.size()))  // only ever increase size
	velData.resize(numRadls * doppBins);
      if ((numRadls * doppBins) > int(spwidData.size()))  // only ever increase size
	spwidData.resize(numRadls * doppBins);
    }
}

void nexradScanBuff::reset(nexradRadial &newradial)
{
  resize(newradial.header.reflNumGates, newradial.header.doppNumGates);
  clear();
  elev = newradial.header.el();
  elevTime = newradial.header.radlTime();
}

void nexradScanBuff::clear()
{
  clear(lastClearVal);
}

void nexradScanBuff::clear(float clearval)
{
  float_vec_iter iter;
  int sz;
  if ((sz = reflData.size()))
    {
      iter = reflData.begin();
      for (int x = 0; x < sz; x++)
	{
	  *iter = clearval;
	  iter++;
	}
    }
  if ((sz = velData.size()))
    {
      iter = velData.begin();
      for (int x = 0; x < sz; x++)
	{
	  *iter = clearval;
	  iter++;
	}
    }
  if ((sz = spwidData.size()))
    {
      iter = spwidData.begin();
      for (int x = 0; x < sz; x++)
	{
	  *iter = clearval;
	  iter++;
	}
    }
  for (int az = 0; az < numRadls; az++)
    {
      reflRadlFlags[az] = 
	velRadlFlags[az] = 
	spwidRadlFlags[az] = false;
    }
  elev = elevFromHist = clearval;
  lastClearVal = clearval;
}

void nexradScanBuff::addRadl(nexradRadial &addradl)
{  
  float_vec_iter fl_iter;

  //Calculate azimuth angle and Azimuth Index
  int AzIndex = int(round(addradl.header.az()));
  if(AzIndex >= 360) AzIndex = AzIndex-360;
  
  
  //Save reflectivity data into the array
  if(addradl.hasRefl())
    {

// just copy all of the data at this point

//       if (oneRadial.header.reflGateSize)
// 	FstBin = int((oneRadial.header.rngToFirstReflGate/
// 		      oneRadial.header.reflGateSize)+0.5);
//       BinNum = oneRadial.header.reflNumGates;
//       if(FstBin<0)
// 	{
// 	  BinNum = FstBin+BinNum;
// 	  FstBin = -1*FstBin;
// 	}
//       LstBin = FstBin + BinNum;
//       //Save data
//       for(BnIndex=FstBin; BnIndex<LstBin; BnIndex++) 
// 	reflData[(AzIndex*reflBins) + BnIndex] = 
// 	  DecodeRef(oneRadial.ReflData[BnIndex]);

      if (addradl.header.reflNumGates != reflBins)
	resizeRefl(addradl.header.reflNumGates);

      // Copy all refl data
      if ((fl_iter = beginReflRadlAz(AzIndex)) != reflData.end())
	{
	  for(int BnIndex=0; BnIndex<reflBins; BnIndex++) 
	    {
	      *fl_iter = 
		decodeRefl(addradl.ReflData[BnIndex]);
	      fl_iter++;
	    }
	  reflRadlFlags[AzIndex] = true;
	}
      reflGateSize = (int)addradl.header.reflGateSize;
      rngToFirstReflGate = (short)addradl.header.rngToFirstReflGate; 
      //if its negative, cast to short to show it
      if (addradl.header.nyquist())
	nyquist = addradl.header.nyquist();
    }

  //Save velocity data into the array
  if(addradl.hasVel())
    {

// just copy all of the data at this point

//       //Get first bin, last bin, and number of bins
//       if (newradl.header.doppGateSize)
// 	FstBin = int((addradl.header.rngToFirstDoppGate/
// 		      addradl.header.doppGateSize)+0.5);
//       BinNum = addradl.header.GatesNumberOfDoppler;
//       if(FstBin<0)
// 	{
// 	  BinNum = FstBin+BinNum;
// 	  FstBin = -1*FstBin;
// 	}
//       LstBin = FstBin + BinNum;
//       ptrPos = addradl.header.PtrOfVelocity);
// //Save data
// for(BnIndex=FstBin; BnIndex<LstBin; BnIndex++) {
//   velData[(AzIndex * doppBins) + BnIndex] = 
//     DecodeVel(addradl.VelData[BnIndex],
// 	      addradl.header.velRes()));

      if (addradl.header.doppNumGates != doppBins)
	resizeDopp(addradl.header.doppNumGates);

      // Copy all vel data
      if ((fl_iter = beginVelRadlAz(AzIndex)) != velData.end())
	{
	  float scale_factor = addradl.header.velRes();
	  for(int BnIndex=0; BnIndex<doppBins; BnIndex++) 
	    {
	      *fl_iter = 
		decodeVel(addradl.VelData[BnIndex], scale_factor);
	      fl_iter++;
	    }
	  velRadlFlags[AzIndex] = true;
	}
      doppGateSize = (int)addradl.header.doppGateSize;
      rngToFirstDoppGate = (short)addradl.header.rngToFirstDoppGate; 
      //if its negative, cast to short to show it
      if (addradl.header.nyquist())
	nyquist = addradl.header.nyquist();
    }
			
  //Save spectrum width data into the array

// just copy all of the data at this point

  if(addradl.hasSpWid())
    {
//     //Get first bin, last bin, and number of bins
//     if (addradl.header.GateSizeOfDoppler))
//       FstBin = int(addradl.header.RangeToFirstGateOfDop)/
//   addradl.header.GateSizeOfDoppler)+0.5);
// BinNum = addradl.header.GatesNumberOfDoppler);
// if(FstBin<0)
//   {
//     BinNum = FstBin+BinNum;
//     FstBin = -1*FstBin;
//   }
// LstBin = FstBin + BinNum;
// ptrPos = addradl.header.PtrOfSpectrumWidth);
// //Save data
// for(BnIndex=FstBin; BnIndex<LstBin; BnIndex++)
//   WData[ElIndex][AzIndex][BnIndex] = 
//     DecodeSpw(addradl.header.Echodata[ptrPos+ptrOffset+BnIndex]);

      if (addradl.header.doppNumGates != doppBins)
	resizeDopp(addradl.header.doppNumGates);

      // Copy all vel data
      if ((fl_iter = beginSpWidRadlAz(AzIndex)) != spwidData.end())
	{
	  for(int BnIndex=0; BnIndex<doppBins; BnIndex++) 
	    {
	      *fl_iter = 
		decodeSpWid(addradl.SWData[BnIndex]);
	      fl_iter++;
	    }
	  spwidRadlFlags[AzIndex] = true;
	}
      doppGateSize = (int)addradl.header.doppGateSize;
      rngToFirstDoppGate = (short)addradl.header.rngToFirstDoppGate; 
      //if its negative, cast to short to show it
      if (addradl.header.nyquist())
	nyquist = addradl.header.nyquist();
    }

}

float nexradScanBuff::decodeRefl(unsigned char code)
{
  if(code==CODE_INVALID)		
    return VALUE_INVALID;
  else if(code==CODE_RANFOLD)	
    return VALUE_RANFOLD;
  else						
    return (float((code-2.0)/2.0-32.5));
}


float nexradScanBuff::decodeVel(unsigned char code, float scale_factor)
{
  if(code==CODE_INVALID)		
    return VALUE_INVALID;
  else if(code==CODE_RANFOLD)	
    return VALUE_RANFOLD;
  else	
    return (float((code-2)-127)) * scale_factor;
}


float nexradScanBuff::decodeSpWid(unsigned char code)
{
  if(code==CODE_INVALID)		
    return VALUE_INVALID;
  else if(code==CODE_RANFOLD)	
    return VALUE_RANFOLD;
  else						
    return (float((code-2.)/2.-63.5));
}

void nexradScanBuff::fillMissingRadials() 
{
  
  //fill in any missing radials provided they are singletons, ie have immediate valid neighbours.
  int precAz = 0;
  int nextAz = 0;
  int AzIndex, BnIndex;
  float_vec_iter prec_iter, this_iter, next_iter;
  bool iters_ok;

  for (AzIndex=0;AzIndex<numRadls;AzIndex++) 
    {
      precAz = AzIndex -1;
      nextAz = AzIndex + 1;
      if (precAz <0) precAz = 359;
      if (nextAz > 359) nextAz = 0;
	  
      if (!reflRadlFlags[AzIndex] && reflRadlFlags[precAz] && 
	  reflRadlFlags[nextAz]) 
	{ 
	  iters_ok = 
	    ((prec_iter = beginReflRadlAz(precAz)) != reflData.end()) &&
	    ((this_iter = beginReflRadlAz(AzIndex)) != reflData.end()) &&
	    ((next_iter = beginReflRadlAz(nextAz)) != reflData.end());
	  if (iters_ok) 
	    //interpolate data into missing radial 
	    for (BnIndex=0;BnIndex<reflBins;BnIndex++) 
	      {
		if (*prec_iter != VALUE_INVALID &&
		    *prec_iter != VALUE_RANFOLD &&
		    *next_iter != VALUE_INVALID &&
		    *next_iter != VALUE_RANFOLD)
		  *this_iter = (*prec_iter + *next_iter)/2.0;
		else
		  *this_iter == VALUE_INVALID;
		prec_iter++;
		this_iter++;
		next_iter++;
	      }
	}
      if (!velRadlFlags[AzIndex] && velRadlFlags[precAz] && 
	  velRadlFlags[nextAz]) 
	{ 
	  iters_ok = 
	    ((prec_iter = beginVelRadlAz(precAz)) != velData.end()) &&
	    ((this_iter = beginVelRadlAz(AzIndex)) != velData.end()) &&
	    ((next_iter = beginVelRadlAz(nextAz)) != velData.end());
	  if (iters_ok) 
	    //interpolate data into missing radial 
	    for (BnIndex=0;BnIndex<doppBins;BnIndex++) 
	      {
		if (*prec_iter != VALUE_INVALID &&
		    *prec_iter != VALUE_RANFOLD &&
		    *next_iter != VALUE_INVALID &&
		    *next_iter != VALUE_RANFOLD)
		  *this_iter = (*prec_iter + *next_iter)/2.0;
		else
		  *this_iter == VALUE_INVALID;
		prec_iter++;
		this_iter++;
		next_iter++;
	      }
	}
      if (!spwidRadlFlags[AzIndex] && spwidRadlFlags[precAz] && 
	  spwidRadlFlags[nextAz]) 
	{ 
	  iters_ok = 
	    ((prec_iter = beginSpWidRadlAz(precAz)) != spwidData.end()) &&
	    ((this_iter = beginSpWidRadlAz(AzIndex)) != spwidData.end()) &&
	    ((next_iter = beginSpWidRadlAz(nextAz)) != spwidData.end());
	  if (iters_ok) 
	    //interpolate data into missing radial 
	    for (BnIndex=0;BnIndex<doppBins;BnIndex++) 
	      {
		if (*prec_iter != VALUE_INVALID &&
		    *prec_iter != VALUE_RANFOLD &&
		    *next_iter != VALUE_INVALID &&
		    *next_iter != VALUE_RANFOLD)
		  *this_iter = (*prec_iter + *next_iter)/2.0;
		else
		  *this_iter == VALUE_INVALID;
		prec_iter++;
		this_iter++;
		next_iter++;
	      }
	}
    }
}


#ifdef COMPILE_READNEXRAD_MAIN

pid_t mainpid = 0;
bool quitRapicConvert = false;
bool debug=0;
//bool debug=true;
bool appClosing()
{
  return quitRapicConvert;
}


void HandleSignal(int signo, siginfo_t *siginfo, void *ptr)
{
    if (getpid() != mainpid)
	return;
    fprintf(stderr,"SIGNAL RECEIVED = %d\n",signo);
    switch (signo) 
    {
	case SIGHUP:
	case SIGINT:
	case SIGTERM:
	case SIGABRT:
	case SIGUSR1:
	    quitRapicConvert = true;
	    break;
    }	
}

int main( int argc, char **argv )
{

  if (argc < 4) {
    fprintf(stdout,"readNexRad ERROR: Should be 3 arguments\n");
    fprintf(stdout,"   Usage:  %s <cinrad or nexrad file name> <NEXRAD|CINRAD> <station number>\n",argv[0]);
    return false;
  }
  
  char filename[1000];
  char radtype[10];
  
  
  //char filename[]="archive2.001";
  strcpy(filename,argv[1]);
  strcpy(radtype,argv[2]);
  int station = 0;

  if (argc > 4)
    {
      if (strstr("debug", argv[4]))
	debug = 1;
    }

  if (!sscanf(argv[3],"%d",&station)) {
    fprintf(stderr,"readNexRad ERROR: station %s not recognised, should be a number\n",argv[3]);
    fprintf(stderr,"   Usage:  %s <cinrad or nexrad file name> <NEXRAD|CINRAD> <station number>\n",argv[0]);
    exit(-1);
    
  }
  printf("readNexRad: input station is %d\n",station);
  
  GetSiteInfo(DISABLE_COVERAGE_LOAD);
  
  bool res = true;

  if (debug) printf("Begin reading...\n");


  readNexRad *nexRadReader = new readNexRad();
  nexRadReader->debug = debug;
  res=nexRadReader->ReadNexradData(filename,radtype,station);

  if(res) {
    if (debug) printf("run OK	\n");
  }
  else fprintf(stderr,"readNexRad: Error reading data.\n");
	 

  if (debug) printf("Complete!\n");

  delete nexRadReader;

  return 0;
}

#endif

