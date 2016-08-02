// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
// ** Copyright UCAR (c) 1990 - 2016                                         
// ** University Corporation for Atmospheric Research (UCAR)                 
// ** National Center for Atmospheric Research (NCAR)                        
// ** Boulder, Colorado, USA                                                 
// ** BSD licence applies - redistribution and use in source and binary      
// ** forms, with or without modification, are permitted provided that       
// ** the following conditions are met:                                      
// ** 1) If the software is modified to produce derivative works,            
// ** such modified software should be clearly marked, so as not             
// ** to confuse it with the version available from UCAR.                    
// ** 2) Redistributions of source code must retain the above copyright      
// ** notice, this list of conditions and the following disclaimer.          
// ** 3) Redistributions in binary form must reproduce the above copyright   
// ** notice, this list of conditions and the following disclaimer in the    
// ** documentation and/or other materials provided with the distribution.   
// ** 4) Neither the name of UCAR nor the names of its contributors,         
// ** if any, may be used to endorse or promote products derived from        
// ** this software without specific prior written permission.               
// ** DISCLAIMER: THIS SOFTWARE IS PROVIDED "AS IS" AND WITHOUT ANY EXPRESS  
// ** OR IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED      
// ** WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.    
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
//
// Read a high res radial data file. Niles Oien.
//
#include "hiResRadialFile.hh"

#include <bzlib.h>
#include <cmath>

#include <iostream>
#include <stdlib.h>
#include <cstdio>

#include <toolsa/umisc.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
//
// Destructor - frees memory.
//
hiResRadialFile::~hiResRadialFile(){
  if (_radialBytes != NULL) free(_radialBytes);
  if (_radialFl32s != NULL) free(_radialFl32s);
  if (_cartData != NULL) free(_cartData);
  return;
}
//
// Constructor - makes a copy of a pointer to the TDRP parameters and
// reads file.
//
hiResRadialFile::hiResRadialFile(const char *FilePath, Params *P, bool verbose){

  if (verbose) cerr << "hiResRadialFile class instantiated for " << FilePath << endl;

  _params = P;

  // Set data members to default values which will
  // be returned if the file read fails.
  _missingFl32=-999.0;
  _missingByte=0;
  _radialFl32s = NULL;
  _radialBytes = NULL;
  _cartData = NULL;
  _nGates = 0;
  _nRadials=0;
  _noaaProdCode=0;
  _lat=0.0;
  _lon=0.0;
  _alt=0.0;
  _gateSpacing=0.0;
  _firstGateDist=0.0;
  _time=0L;
  _delAz=0.0;
  _nxy=0;
  _dxy=0.0;
  _hw31=0;
  _hw32=0;
  _hw33=0;

  FILE *fp = fopen(FilePath, "r");
  if (fp == NULL){
    if (verbose) cerr << "Could not open " << FilePath << endl;
    return;
  }

  msgHdrBlk_t m;
  if (1 != fread(&m, sizeof(m), 1, fp)){
    if (verbose) cerr << "Unable to read message header from " << FilePath << endl;
    return;
  }
  _swapMsgHdrBlk(&m);
  if (verbose) _printMsgHdrBlk(m);


  graphicProductMsg_t g;
  if (1 != fread(&g, sizeof(g), 1, fp)){
    if (verbose) cerr << "Failed to read graphics block from " << FilePath << endl;
    return;
  }
  _swapGraphicProductMsg(&g);
  if (verbose) _printGraphicProductMsg(g);

  // Need to decode halfword values. These are needed to translate data
  // into physical values.

  _swap2(&g.prodDependent31);
  _hw31 = (fl32) g.prodDependent31;

  _swap2(&g.prodDependent32);
  _hw32 = (fl32) g.prodDependent32;

  _swap2(&g.prodDependent33);
  _hw33 = (fl32) g.prodDependent33;


  if (verbose){
    cerr << "Halfword values used to obtain physical values :" << endl;
    cerr << " hw31 : " << _hw31 << endl;
    cerr << " hw32 : " << _hw32 << endl;   
    cerr << " hw33 : " << _hw33 << endl;
  }


  if (g.symbOff==0 ){
    if (verbose) cerr << "No symbology block found in " << FilePath << endl;
    return;
  }

  if (fseek(fp, 2*g.symbOff, SEEK_SET)){
    if (verbose)  cerr << "Seek to symbology block failed for " << FilePath << endl;
    return;
  }

  struct stat buf;
  if (stat(FilePath, &buf)){
    // Highly unlikely as the file is open, but I should check.
    if (verbose)  cerr << "Failed to stat " << FilePath << endl;
    return;
  }

  // The rest of the file is a bzip2 compressed buffer of radials, read it in
  // to memory and close the file.
  //
  unsigned bytesToRead =  buf.st_size -  2*g.symbOff;
  unsigned char *compressedBuf = (unsigned char *) malloc(bytesToRead);
  if (compressedBuf == NULL){
    cerr << "Malloc fail" << endl;
    exit(-1);
  }

  if (bytesToRead != fread(compressedBuf, sizeof(unsigned char), bytesToRead, fp)){
    if (verbose) cerr << "Main data read failed for " << FilePath << endl;
    return;
  }

  fclose(fp);

  // If the first two bytes of the buffer are "BZ" then it is compressed.
  bool dataCompressed = false;
  if (0==strncmp((char *)compressedBuf, (char *)"BZ", 2)) dataCompressed = true;

  unsigned char *decompressedBuffer;

  if (verbose){
    if (dataCompressed)
      cerr << "Data are compressed" << endl;
    else 
      cerr << "Data not compressed" << endl;
  }

  if (dataCompressed){

    // Uncompress the data - keep doubling the size of the output
    // buffer until we have enough space. Initial guess of compression
    // factor of 200 generally gets it on the first try.
    int sizeFactor = 1;
    int error = 0;
    unsigned int olength;

    do {
      
      olength = 200*bytesToRead*sizeFactor;
      
      decompressedBuffer = (unsigned char *)malloc(olength);
      if (decompressedBuffer == NULL){
	cerr << "malloc failed for size " << olength << endl;
	exit(-1);
      }
      
#ifdef BZ_CONFIG_ERROR
      error = BZ2_bzBuffToBuffDecompress((char *) decompressedBuffer, &olength,
					 (char *) compressedBuf, bytesToRead, 0, 0);
#else
      error = bzBuffToBuffDecompress((char *) decompressedBuffer, &olength,
				     (char *) compressedBuf, bytesToRead, 0, 0);
#endif
      
      if (error == BZ_OUTBUFF_FULL){
	free(decompressedBuffer);
	sizeFactor *= 2;
	if (sizeFactor > 10){
	  if (verbose) cerr << "Crazy allocation size attemped for " << FilePath << ", giving up" << endl;
	  return;
	}
      }

    } while (error == BZ_OUTBUFF_FULL); // Keep doubling the size of the output buffer until it's enough.

    if (error){
      free(decompressedBuffer);
      if (verbose) cerr << "Decompression failed for " << FilePath << endl;
      return;
    }

    if (verbose) cerr << "Exited decompression with sizeFactor=" << sizeFactor << ", outlen=" << olength << endl << endl;

  } else { // Data were not compressed, just allocate space and copy them across
    decompressedBuffer = (unsigned char *) malloc(bytesToRead);
    if (decompressedBuffer == NULL){
      cerr << "Malloc fail" << endl;
      exit(-1);
    }
    memcpy(decompressedBuffer, compressedBuf, bytesToRead);
  }

  free(compressedBuf);

  unsigned long offset =0L; // Offset to go through what we have in memory

  prodSymbBlk_t psb;
  memcpy(&psb, decompressedBuffer+offset, sizeof(psb)); offset+=sizeof(psb);
  _swapProdSymbBlk( &psb ); if (verbose) _printPrdSymbBlk( psb );


  radial_t r;
  memcpy(&r, decompressedBuffer+offset, sizeof(r)); offset+=sizeof(r);
  _swapRadial( &r ); if (verbose) _printRadial( r );


  //
  // At this point we can be pretty sure the file is OK.
  // Set internal data members after allocating space for data.
  //
  _nGates = r.nBins;
  _nRadials=r.nRadials;

  //
  // The number of bytes per radial may be greater than the number
  // of gates due to packing issues.
  //
  int numBytes = r.nBytes;

  _radialBytes = (ui08 *)malloc(_nGates*_nRadials*sizeof(ui08));
  _radialFl32s = (fl32 *)malloc(_nGates*_nRadials*sizeof(fl32));
  if ((_radialBytes == NULL) || (_radialFl32s == NULL)){
    cerr << "Malloc failed." << endl;
    exit(-1);
  }

  // Initialize radials to missing.
  for (int k=0; k < _nGates*_nRadials; k++){
    _radialFl32s[k]=_missingFl32;
  }

  
  _noaaProdCode=m.msgCode;

  if (_noaaProdCode!=138){
    cerr << "Wrong product code : " <<  _noaaProdCode << endl;
    return;
  }

  _time = (g.volScanDate-1) * 86400 + g.volScanTime;
  _lat = double(g.lat/1000.0);
  _lon = double(g.lon/1000.0);
  _alt = double(g.elevFeet)*0.3;
  _delAz = double(r.delAz)/10.0;


  _gateSpacing = 1.0; // In Km. Actually not in the file, have to deduce from product code.

  _firstGateDist = r.firstIndex*_gateSpacing;

  double azimuth = double(r.az)/10.0;

  for (int i=0; i < _nRadials; i++){

    if (azimuth < 0.0) azimuth += 360.0;
    if (azimuth > 359.95) azimuth -= 360.0;

    int iazIndex = (int)rint( azimuth / _delAz );
    
    if (iazIndex < 0) iazIndex = 0;   
    if (iazIndex > _nRadials -1) iazIndex = _nRadials -1;

    if ((iazIndex < 0) || (iazIndex > _nRadials - 1)) continue; // This should not happen but if it does it's a seg fault

    double min=0.0, max=0.0;
    int first = 1;
    for (int j=0; j < _nGates; j++){
	  
      if ((decompressedBuffer[offset] > 1) && (decompressedBuffer[offset] < 255)){ // Data value is not missing.
	double val = _decodeData(decompressedBuffer[offset]);
	if (first){
	  first=0;
	  min = val; max = val;
	} else {
	  if (val > max) max = val;
	  if (val < min) min = val;
	}
	_radialBytes[iazIndex * _nGates + j]=decompressedBuffer[offset];
	_radialFl32s[iazIndex * _nGates + j]=val;
      } else { // Data value is missing.
	_radialBytes[iazIndex * _nGates + j]=_missingByte;
	_radialFl32s[iazIndex * _nGates + j]=_missingFl32;
      }
      offset++;
    }

    offset += (numBytes-_nGates); // Because there may be trailing bytes - numBytes != nGates, necessarily

    if (verbose){
      if (first){
	cerr << "  No non-missing data in radial." << endl;
      } else {
	cerr << "  Values run from " << min << " to " << max << endl;
      }
    }

    if (i <  r.nRadials -1){
      smallBlock_t sb;
      memcpy(&sb, decompressedBuffer+offset, sizeof(sb)); offset+=sizeof(sb);
      _swapSmallBlock( &sb ); if (verbose) _printSmallBlock( sb );
      numBytes = sb.num; // Probably the same as what we already read in the radial header but since it's here, use it
      azimuth = double(sb.az)/10.0;
    }
  }

  free(decompressedBuffer);

  return;

}

//////////////// Data access methods.

int hiResRadialFile::getNradials(){
  return _nRadials;
}

int hiResRadialFile::getNgates(){
  return _nGates;
}

fl32 hiResRadialFile::getAz(int n){
  return n*_delAz;
}

ui08 *hiResRadialFile::getAllRadialBytes(){
  return _radialBytes;
}

fl32 *hiResRadialFile::getAllRadialFl32s(){
  return _radialFl32s;
}

ui08 *hiResRadialFile::getRadialBytes(int n){
  return _radialBytes + n * _nGates;
}

fl32 *hiResRadialFile::getRadialFl32s(int n){
  return _radialFl32s + n * _nGates;
}

fl32 hiResRadialFile::getLat(){
  return _lat;
}

fl32 hiResRadialFile::getLon(){
  return _lon;
}

fl32 hiResRadialFile::getAlt(){
  return _alt;
}

fl32 hiResRadialFile::getGateSpacing(){
  return _gateSpacing;
}

fl32 hiResRadialFile::getFirstGateDist(){
  return _firstGateDist;
}

fl32 hiResRadialFile::getMissingFl32(){
  return _missingFl32;
}

ui08 hiResRadialFile::getMissingByte(){
  return _missingByte;
}

time_t hiResRadialFile::getTime(){
  return _time;
}

int hiResRadialFile::getProdCode(){
  return _noaaProdCode;
}

fl32 hiResRadialFile::getDelAz(){
  return _delAz;
}

fl32 hiResRadialFile::getDxy(){
  return _dxy;
}

int hiResRadialFile::getNxy(){
  return _nxy;
}

fl32 *hiResRadialFile::getCartData(){
  return _cartData;
}


//////////////////// Remap to cart ////////////

void hiResRadialFile::remapToCart(double delta, double dist){

  double deltaToUse = delta;
  if (deltaToUse < 0.0) deltaToUse = _gateSpacing;
  
  double distToUse=dist;
  if (distToUse < 0.0)  distToUse = _getMaxDist();

  _dxy = deltaToUse;
  _nxy = (int)rint( 2.0 * distToUse / _dxy );


  _cartData = (fl32 *) malloc(_nxy * _nxy * sizeof(fl32));
  if (_cartData == NULL){
    cerr << "Cartesian data malloc failed, nxy= " << _nxy;
    exit(-1);
  }

  double pi = acos( -1.0 );

  for (int ix=0; ix < _nxy; ix++){
    for (int iy=0; iy < _nxy; iy++){

      // Set data to missing

      _cartData[iy * _nxy + ix] = _missingFl32;

      // Figure out dist, az from (nx/2, ny/2) to
      // (ix, iy)

      double ewDist = double(ix - _nxy/2)*_dxy;
      double nsDist = double(iy - _nxy/2)*_dxy;

      double polarDist = sqrt(ewDist*ewDist + nsDist*nsDist);
      int radialIndex = (int)rint((polarDist - _firstGateDist)/_gateSpacing);
      if (radialIndex > _nGates) continue; // Out of radar range

      // Get index of closest azimuth
      double az=0.0;
      if ((nsDist == 0.0) && (ewDist == 0.0))
	az = 0.0;
      else
	az = atan2(nsDist, ewDist);


      az = 180.0*az/pi;
      az = 90.0-az;
      if (az < 0.0) az += 360.0;

      int azIndex = _getAzIndex( az );
      if (azIndex < 0) azIndex = 0;
      if (azIndex > _nRadials-1) azIndex = _nRadials-1;

      // Move data from polar grid to cartesian grid

      _cartData[iy * _nxy + ix] = _radialFl32s[ azIndex * _nGates + radialIndex ];

    }
  }

  return;
}

// Return index for a given azimuth. Azimuth must be on range 0..360.
int  hiResRadialFile::_getAzIndex( double az ){
  return (int)rint(az/_delAz);
}

//
// Get the maximum distance we have to care about when
// remapping to cartesian.
//
double  hiResRadialFile::_getMaxDist(){

  int maxGateIndex = 0;
  for (int i=0; i < _nRadials; i++){
    for (int j=maxGateIndex; j < _nGates; j++){
      if (_radialBytes[i * _nGates + j] !=  _missingByte) maxGateIndex = j;
    }
  }

  return _firstGateDist + maxGateIndex * _gateSpacing;

}


double hiResRadialFile::_decodeData(ui08 x){
  //
  // Returns a floating point physical value given a byte.
  double val = double(x)*double(_hw32) + double(_hw31);
  return val/100.0; // Go from hundredths of an inch to inches
}


//////// Swap, print routines for data structures from file

void hiResRadialFile::_swapMsgHdrBlk(msgHdrBlk_t *m){

  _swap2( &m->msgCode);
  _swap2( &m->msgDate);
  _swap4( &m->msgTime);
  _swap4( &m->msgLen);
  _swap2( &m->msgSrc);
  _swap2( &m->msgDest);
  _swap2( &m->nBlcks);
  _swap2( &m->blckDiv);

  return;

}

void hiResRadialFile::_printMsgHdrBlk(msgHdrBlk_t m){

  cerr << "Message header block:" << endl;
  cerr << " Message code : " << m.msgCode << endl;
  
  time_t t = (m.msgDate-1) * 86400 + m.msgTime;

  cerr << " Message time : " << utimstr( t ) << endl;
  cerr << " Message length : " << m.msgLen << endl;
  cerr << " Message source : " << m.msgSrc << endl;
  cerr << " Message dest : " << m.msgDest << endl;
  cerr << " Number of blocks : " << m.nBlcks << endl;
  if (m.blckDiv == -1)
    cerr << " Message block divider is valid";
  else
    cerr << " Message block divider is not valid";

  cerr << endl << endl;

  return;

}



void hiResRadialFile::_swapGraphicProductMsg(graphicProductMsg_t *g){

  _swap4( &g->lat);
  _swap4( &g->lon);
  _swap2( &g->elevFeet);
  _swap2( &g->prodCode);
  _swap2( &g->opMode);
  _swap2( &g->vcp);
  _swap2( &g->seqNum);
  _swap2( &g->volScanNum);
  _swap2( &g->volScanDate);
  _swap4( &g->volScanTime);
  _swap2( &g->genProdDate);
  _swap4( &g->genProdTime);
  _swap2( &g->elNum);
  _swap4( &g->symbOff );
  _swap4( &g->grphOff );
  _swap4( &g->tabOff );


  return;

}

void hiResRadialFile::_printGraphicProductMsg(graphicProductMsg_t g){

  cerr << "Graphic product message block :" << endl;

  cerr << " Lat, lon, alt (feet) : " << double(g.lat/1000.0) << ", " << double(g.lon/1000.0) << ", " << g.elevFeet << endl;
  cerr << " Product code : " <<  g.prodCode << endl;
  cerr << " Operational mode (0 = maint, 1=clear air, 2=weather) : " << g.opMode << endl;
  cerr << " VCP : " << g.vcp << endl;
  cerr << " Sequence number : " <<  g.seqNum << endl;
  cerr << " Volume number " <<  g.volScanNum << endl;

  time_t t = (g.volScanDate-1) * 86400 + g.volScanTime;
  cerr << " Volume time " << utimstr( t ) << endl;

  t = (g.genProdDate-1) * 86400 + g.genProdTime;
  cerr << " Product generation time " << utimstr( t ) << endl;

  cerr << " Elevation number : " << g.elNum << endl;
  cerr << " Version : " << (int)g.version << endl;
  cerr << " Spot blank (1=on, 0=off) : " <<  (int)g.spotBlank << endl;
  cerr << " Symb block offset, halfwords (0 = not present) : " << g.symbOff << endl;
  cerr << " Graphic block offset, halfwords (0 = not present) : " << g.grphOff << endl;
  cerr << " Tabular block offset, halfwords (0 = not present) : " << g.tabOff << endl;

  cerr << endl;

  return;

}


void hiResRadialFile::_swapProdSymbBlk(prodSymbBlk_t *p){

  _swap2( &p->div);
  _swap2( &p->id);
  _swap4( &p->len);
  _swap2(  &p->nLayers);
  _swap2( &p->layerDiv);
  _swap4( &p->dataLayerLen);

  return;
}
 
void hiResRadialFile::_printPrdSymbBlk(prodSymbBlk_t p){

  cerr << "Product symbology block : " << endl;
  cerr << " Div (should be -1) : " << p.div << endl;
  cerr << " ID (should be 1) : " << p.id << endl;
  cerr << " Length (bytes) of block : " << p.len << endl;
  cerr << " Number of layers : " << p.nLayers << endl;
  cerr << " Layer div (should be -1) : " << p.layerDiv << endl;
  cerr << " Length of data layer (bytes) : " << p.dataLayerLen << endl << endl;

  return;
}

void hiResRadialFile::_swapRadial(radial_t *r){

    _swap2( &r->msgCode);
    _swap2( &r->firstIndex);
    _swap2( &r->nBins);
    _swap2( &r->iCent);
    _swap2( &r->jCent);
    _swap2( &r->rangeScaleFactor);
    _swap2( &r->nRadials);
    _swap2( &r->nBytes);
    _swap2( &r->az);
    _swap2( &r->delAz);

  return;

}

void hiResRadialFile::_printRadial(radial_t r){

  cerr << "Radial header block : " << endl;
  cerr << " Message code (should be 16) : " << r.msgCode << endl;
  cerr << " First index : " << r.firstIndex << endl;
  cerr << " Number of bins : " << r.nBins << endl;
  cerr << " I,J center : " << r.iCent << ", " << r.jCent << endl;

  double angle = acos(double(r.rangeScaleFactor)/1000.0);
  angle = angle * 180.0 / 3.1415927;

  cerr << " Range scale factor : " << double(r.rangeScaleFactor)/1000.0 << " angle " << angle << endl;
  cerr << " Number of radials : " << r.nRadials << endl;
  cerr << " Number of bytes : " << r.nBytes << endl;
  cerr << " Azimuth : " << double(r.az)/10.0 << endl;
  cerr << " Delta azimuth : " << double(r.delAz)/10.0 << endl << endl;

  return;
}

void hiResRadialFile::_swapSmallBlock(smallBlock_t *b){

  _swap2( &b->num);
  _swap2( &b->az);
  _swap2( &b->dAz);

  return;

}

void hiResRadialFile::_printSmallBlock(smallBlock_t b){

  cerr << endl << "Small block found : " << endl;
  cerr << " Num : " << b.num << endl;
  cerr << " Az : " << double(b.az)/10.0 << endl;
  cerr << " dAz : " << double(b.dAz)/10.0 << endl << endl;

  return;

}




void hiResRadialFile::_swap2(void *c){

  if (!(_params->byteSwap)) return;

  unsigned char *b = (unsigned char *) c;

  unsigned char t = *b;
  *b = *(b+1);
  *(b+1) = t;

  return;

}


void hiResRadialFile::_swap4(void *c){

  if (!(_params->byteSwap)) return;

  unsigned char *b = (unsigned char *) c;

  unsigned char t = *b;
  *b = *(b+3);
  *(b+3) = t;

  t = *(b+1);
  *(b+1) = *(b+2);
  *(b+2) = t;

  return;

}

