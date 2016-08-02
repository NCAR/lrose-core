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
#include <bzlib.h>
#include <cmath>
#include <zlib.h>
#include <iostream>
#include <stdlib.h>
#include <cstdio>
#include <toolsa/pmu.h>
#include <toolsa/umisc.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include "NidsFile.hh"
#include "GraphicProductMsg.hh"
#include "EchoTopsProduct.hh"
#include "DiffReflProduct.hh"
#include "SpecDiffPhaseProduct.hh"
#include "StormPrecipProduct.hh"
#include "HydroClassProduct.hh"
#include "HybridHydroClass.hh"
#include "VilProduct.hh"
#include "OneHourPrecipTotal.hh"
#include "StormTotalPrecip.hh"
#include "DigitalPrecipRate.hh"
#include "DigitalAccumArray.hh"
#include "DigitalTotalAccum.hh"
#include "CorrCoef.hh"
#include "BaseReflProduct.hh"
#include "BaseVelProduct.hh"
#include "MeltingLayer.hh"
#include "Mesocyclone.hh"
#include "SRVel.hh"
#include "TornadoVortexSig.hh"
#include "HailIndex.hh"
#include "TdwrRefl.hh"
#include "DigitalHybridRefl.hh"
#include "orpg_product.h"

const fl32 NidsFile::_missingFl32 = -999.0;

map < string, int >
NidsFile::_scanMap = {{"VEL1",1}, {"VEL2",2}, {"VEL3",3}, {"VEL4",4},
		      {"BREF1",1},{"BREF2",2}, {"BREF3",3}, {"BREF4",4},
		      {"N0S",1}, {"N1S",2}, {"N2S",3}, {"N3S",4},
		      {"N0X",1}, {"NAX",2}, {"N1X",3}, {"NBX",4}, {"N2X",5}, {"N3X",6},
		      {"N0C",1}, {"NAC",2}, {"N1C",3}, {"NBC",4}, {"N2C",5}, {"N3C",6},
		      {"N0K",1}, {"NAK",2}, {"N1K",3}, {"NBK",4}, {"N2K",5}, {"N3K",6},
		      {"N0H",1}, {"NAH",2}, {"N1H",3}, {"NBH",4}, {"N2H",5}, {"N3H",6},
		      {"TR0",1}, {"TR1",2}, {"TR2",3},
		      {"TV0",1}, {"TV1",2}, {"TV2",3}};


NidsFile::NidsFile(const char *filePath, bool byteSwap, 
		   bool debug, bool hasHdr):
	       
  _nidsFile(filePath),
  _byteSwap(byteSwap),
  _debug(debug),
  _hasExtraHdr(hasHdr),
  _product(NULL),
  _lat(0),
  _lon(0),
  _alt(0),
  _time(0),
  _elevAngle(0),
  _nRadials(0),
  _nGates(0),
  _gateSpacing(0),
  _firstGateDist(0),
  _delAz(0),
  _radialFl32s(NULL),
  _cartData(NULL),
  _nxy(0),
  _dxy(0),
  _runLengthEncoding(false),
  _genericRadialFormat(false),
  _linkedVectorPacket(false),
  _hasGraphicSymbols(false)
{
  _contours.resize(4);

  unsigned found = _nidsFile.find_last_of(".");

  _fileSuffix =  _nidsFile.substr(found+1);	

  if (_debug)
  {
    cerr << "NidsFile class instantiated for " << _nidsFile << endl;
  }
}

//
// Destructor - frees memory.
//
NidsFile::~NidsFile()
{
  if (_radialFl32s != NULL)
  {
    free(_radialFl32s);
  }

  if (_cartData != NULL)
  {
    free(_cartData);
  }
  
  if(_product != NULL)
  {
    delete _product;
  }
  
  for (int i = 0; i < _contours.size(); i++)
  {
    _contours[i].clear();
  }

  _hailSymbols.clear();

  _tvsSymbols.clear();
  
  _mesocycSymbols.clear();

  _stormIDs.clear();
  
  return;
}
int NidsFile::getTiltIndex()
{
  if ( _scanMap.find(_fileSuffix) == _scanMap.end() )
       return 0;
  else
      return _scanMap[_fileSuffix];
}

bool NidsFile::isNextTilt(const string prevSuffix)
{
  // sanity check for previous file suffix in the map
  if ( _scanMap.find(prevSuffix) != _scanMap.end())
  {
  
    //
    // For vel, srvel, reflectivity:
    //
    switch( _msgCode)
    {
    case 56: 
    case 94:
    case 99:
    case 181:
    case 182:
      if( _scanMap[_fileSuffix] == _scanMap[prevSuffix] +1)
	return true;
      else
	return false;
      break;

    case 159:
    case 161:
    case 163:
    case 165:
      if( _scanMap[prevSuffix] == 1 && (_scanMap[_fileSuffix] == 2 || _scanMap[_fileSuffix] == 3))
	return true;
      else if ( _scanMap[prevSuffix] == 2 && _scanMap[_fileSuffix] == 3 )
	return true;
      else if ( _scanMap[prevSuffix] == 3 && (_scanMap[_fileSuffix] == 4 || _scanMap[_fileSuffix] == 5))
	return true;
      else if ( _scanMap[_fileSuffix] == _scanMap[prevSuffix] +1 )
	return true;
      else
	return false;
      break;
    default:
      return false;
    }
  }
  else
  {
    return false;
  }
}

int NidsFile::decode() 
{
  PMU_auto_register("NidsFile::decode()");

  //
  // Open NIDS file
  //
  FILE *fp = fopen(_nidsFile.c_str(), "r");
  if (fp == NULL)
  {
    
    cerr << "NidsFile::decode(): Could not open " << _nidsFile.c_str() 
	 << ". No processing of file." << endl;
    return 1;
  }

  //
  // Read Message Header Block
  //
  GraphicProductMsg::msgHdrBlk_t msgHdr;

  if (GraphicProductMsg::readMessageHeaderBlock(msgHdr, fp, _byteSwap,
						_debug, _hasExtraHdr) )
  {
    if(fp)
      fclose(fp);
    
    //
    // Not successful
    //
    return 1;
  }

  //
  // Instantiate a Product object based on the message code. Graphic product 
  // message information is contained in this object.
  // Message codes can be found in the See ICD For the RPG for Class 1 User
  // in Table III. This product will be used to do product specific decoding 
  // of radial data. 
  // Note that gate spacing is set. Gate spacing is not found in ICD. 
  // Most gate spacing information was found in 
  // http://www.roc.noaa.gov/WSR88D/DualPol/documentation.aspx
  //
  // last tilt in volume string taken 
  // from:http://www.ncdc.noaa.gov/oa/radar/productsdetail.html
  // 
 
  _msgCode = msgHdr.msgCode;

  switch (_msgCode){
  case 32:
     _product = new DigitalHybridRefl(fp,  _byteSwap, _debug);
     _gateSpacing = 1;
     _runLengthEncoding = false;
     _startOfVolStr = "DHR";
     _endOfVolStr = "DHR";
     break;
  case 56:
     _product = new SRVel(fp,  _byteSwap, _debug);
     _gateSpacing = 1;
     _runLengthEncoding = true;
     _startOfVolStr = "N0S";
     _endOfVolStr = "N3S";
     break;
  case 59:
     _product = new HailIndex(fp,  _byteSwap, _debug);
     _gateSpacing = .25;
     _hasGraphicSymbols = true;
     _endOfVolStr = "NHI";
     _startOfVolStr = _endOfVolStr;
     break;
  case 61:
     _product = new TornadoVortexSig(fp,  _byteSwap, _debug);
     _gateSpacing = .25;
     _hasGraphicSymbols = true;
     _endOfVolStr = "NTV";
     _startOfVolStr = _endOfVolStr;
     break;
  case 78:
    _product = new OneHourPrecipTotal(fp,  _byteSwap, _debug);
    _runLengthEncoding = true;
    _gateSpacing = 2;
    _endOfVolStr = "N1P";
    _startOfVolStr = _endOfVolStr;
    break;
  case 80:
    _product = new StormTotalPrecip(fp,  _byteSwap, _debug);
    _runLengthEncoding = true;
    _gateSpacing = 2;
    _endOfVolStr = "NTP";
    _startOfVolStr = _endOfVolStr;
    break;
  case 94:
  case 19:
    _product = new BaseReflProduct(fp,  _byteSwap, _debug);
    _gateSpacing = 1;
    _startOfVolStr = "BREF1";
    _endOfVolStr = "BREF4";
    break;
  case 99:
    _product = new BaseVelProduct(fp,  _byteSwap, _debug);
    _gateSpacing = .25;
    _startOfVolStr = "VEL1";
    _endOfVolStr = "VEL4";
    break;
  case 134:
    _product = new VilProduct(fp,  _byteSwap, _debug);
    _gateSpacing = 1;
    _endOfVolStr = "DVL";
    _startOfVolStr = _endOfVolStr;
    break;
  case 135:
    _product = new EchoTopsProduct(fp,  _byteSwap, _debug);
    _gateSpacing = 1;
    _endOfVolStr = "EET";
    _startOfVolStr = _endOfVolStr;
    break;
  case 138:
    _product = new StormPrecipProduct(fp,  _byteSwap, _debug);
    _gateSpacing = 1;
    _endOfVolStr = "DSP";
    _startOfVolStr = _endOfVolStr;
    break;
  case 141:
    _product = new Mesocyclone(fp,  _byteSwap, _debug);
    _gateSpacing = .25;
    _hasGraphicSymbols = true;
    _endOfVolStr = "NMD";
    _startOfVolStr = _endOfVolStr;
    break;
  case 159: 
    _product = new DiffReflProduct(fp,  _byteSwap, _debug);
    _gateSpacing = .25;
    _startOfVolStr = "N0X"; 
    _endOfVolStr = "N3X";
    break;  
  case 161:
    _product = new CorrCoef(fp,  _byteSwap, _debug);
    _gateSpacing = .25;
    _startOfVolStr = "N0C";
    _endOfVolStr = "N3C";
    break;
  case 163:
    _product = new SpecDiffPhaseProduct(fp,  _byteSwap, _debug);
    _gateSpacing = .25;
    _startOfVolStr = "N0K";
    _endOfVolStr = "N3K";
    break;
  case 165: 
    _product = new HydroClassProduct(fp,  _byteSwap, _debug);
    _gateSpacing = .25;
    _startOfVolStr = "N0H";
    _endOfVolStr = "N3H";
    break;
  case 166:
    _product = new MeltingLayer(fp,  _byteSwap, _debug);
    _linkedVectorPacket = true;
    _startOfVolStr = "N0M";
    _endOfVolStr = "N3M";
    break;
  case 170: 
    _product = new DigitalAccumArray(fp,  _byteSwap, _debug);
    _gateSpacing = .25;
    _endOfVolStr = "DAA";
    _startOfVolStr = _endOfVolStr;
    break;  
  case 172: 
    _product = new DigitalTotalAccum(fp,  _byteSwap, _debug);
    _gateSpacing = .25;
    _endOfVolStr = "DTA";
    _startOfVolStr = _endOfVolStr;
    break;    
  case 176:
    _product = new DigitalPrecipRate(fp, _byteSwap, _debug);
    _gateSpacing = .25;
    _genericRadialFormat = true;
    _endOfVolStr = "DPR";
    _startOfVolStr = _endOfVolStr;
    break;
  case 177:
    _product = new HybridHydroClass(fp, _byteSwap, _debug);
    _gateSpacing = .25;
    _endOfVolStr = "HHC";
    _startOfVolStr = _endOfVolStr;
    break;
  case 181:
    _product = new TdwrRefl(fp,  _byteSwap, _debug);
    _gateSpacing = .15;
    _runLengthEncoding = true;
    _startOfVolStr = "TR0";
    _endOfVolStr = "TR2";
    break;
  case 182:
    _product = new BaseVelProduct(fp,  _byteSwap, _debug);
    _gateSpacing = .15;
    _startOfVolStr = "TV0";
    _endOfVolStr = "TV2";
    break;
  case 186:
    _product = new BaseReflProduct(fp,  _byteSwap, _debug);
    _gateSpacing = .3;
    _endOfVolStr = "TZL";
    _startOfVolStr = _endOfVolStr;
    break;

  default:
    
    cerr << "NidsFile::decode(): Product decoder for product " 
	 <<  msgHdr.msgCode << " not found. End of processing." << endl;
    
    if(fp)
      fclose(fp);
 
    return 1;  
  }
 
  //
  // Read Product Decscription Block from Graphic Product Message. See ICD 
  // Figure3-6 (Sheets 2 and 6)
  //
  PMU_auto_register("Reading Graphic product message from file");

  if (_product->readFromFile(_hasExtraHdr) == Product::PRODUCT_FAILURE)
  {
    cerr << "NidsFile::decode(): Failure to read Product Description "
	 << "block. End of processing." << endl;
   
    if(fp)
      fclose(fp);

    return 1;
  }

  if ( _product->graphProdDesc.prodDependent30  &&
       (msgHdr.msgCode == 170 || msgHdr.msgCode == 172))
  {
    cerr << "Product contains no data. (Null prodcut flag set to true)" << endl;

    if(fp)
      fclose(fp);
    
    return 1;
  }

  //
  // Set members from product description block
  // (lat, lon, alt, time etc.)
  //
  _setProductDescrMembers();

  //
  // The rest of file may be compressed. Get decompressed data.
  //
  unsigned char *decompressedBuf; // = NULL;

  unsigned int bufSize = 0;

  if (_getDecompressedData(&decompressedBuf, fp, bufSize))
  {
    if(fp)
      fclose(fp);
    
    //
    // Not successful
    //
    return 1;
  }

  //
  // Close file
  //
  if(fp)
    fclose(fp);

  //
  // Initialize offset with which to go through rest of data in memory
  //
  long unsigned int offset = 0;

  GraphicProductMsg::prodSymbBlk_t psb;
  psb.dataLayerLen = 0;
  GraphicProductMsg::readSymbologyBlockHdr(psb, decompressedBuf, offset, 
					   _byteSwap, _debug);

  
  if ( bufSize - sizeof(psb) < psb.dataLayerLen) 
  {
    cerr << "NidsFile::decode(): WARNING: The memory allocated is less "
	 << "than the memory that is specified in the Product Symbology "
	 << "block header." << endl;
    cerr << "size allocated: " << bufSize << " size needed: " 
	 << psb.dataLayerLen << endl;
    cerr << "The file causing the issue is: " <<  _nidsFile.c_str() << endl; 
  }
  int ret = 0;

  //
  // Decode data packets
  //
  if(_linkedVectorPacket)
  {
    _decodeLinkedVectors(decompressedBuf, offset);
  }
  else if(_runLengthEncoding)
  {
    //
    // Radial data packet code "AF1F"
    //
    ret = _decodeRadialMsg(decompressedBuf, offset, bufSize);
  }
  else if (_genericRadialFormat)
  {
    //
    // Radial data packet code 28
    //
    ret = _decodeGenericMsg( decompressedBuf, offset);
  }
  else if (_hasGraphicSymbols) 
  {
    _decodeGraphicSymbols(decompressedBuf, offset, psb.dataLayerLen);
  }
  else
  {
    //
    // Radial data packet code 16
    //
    ret = _decodeDigitalRadialMsg(decompressedBuf, offset);
  }
    
  free(decompressedBuf);

  return ret;
}

int NidsFile::_decompress( unsigned char *compressedBuf,  
			   unsigned char **decompressedBuf,
			   unsigned int numBytes,
			   unsigned int &bufSize)
{

  PMU_auto_register("NidsFile::_decompress");

  if ( _product->isCompressed() )
  {
    if(_debug)
    {
      cerr << "NidsFile::decompressData(): Data is compressed." << endl;
    }

    //
    // Get the size
    //
    bufSize = _product->getUncompProdSize();

    //
    // Allocate the memory
    //
    *decompressedBuf = (unsigned char *)malloc(bufSize);

    if (*decompressedBuf == NULL)
    {
	cerr << "NidsFile::decompressData(): Memory allocation failed "
	     << "for " << _product->getUncompProdSize() << " bytes. Exiting."  
	     << endl;

	exit(1);
    }
   else
   {
      memset(*decompressedBuf, 0, bufSize);
   }

    //
    // Run decompression method
    //
    int error = BZ2_bzBuffToBuffDecompress((char *) *decompressedBuf, &bufSize,
					   (char *) compressedBuf, numBytes, 
					   0, 0);
    if (error != BZ_OK)
    {
      free(*decompressedBuf);
      switch(error) 
      {
      case BZ_CONFIG_ERROR:

      	 cerr << "NidsFile::compressData():BZ_CONFIG_ERROR (library miscompiled) Decompression failed for " 
	      << _nidsFile.c_str() << " error code = " << error <<  endl;
         break;
     case BZ_PARAM_ERROR:
         cerr << "NidsFile::compressData():BZ_PARAM_ERROR Decompression failed for "
              << _nidsFile.c_str() << " error code = " << error <<  endl;
         break;
     case BZ_MEM_ERROR:
         cerr << "NidsFile::compressData():BZ_MEM_ERROR Decompression failed for "
              << _nidsFile.c_str() << " error code = " << error <<  endl;
         break;
     case BZ_OUTBUFF_FULL:
         cerr << "NidsFile::compressData():BZ_OUTBUFF_FULL Decompression failed for "
              << _nidsFile.c_str() << " error code = " << error <<  endl;
          break;
     default:
         cerr << "NidsFile::compressData(): Unknown error code. Decompression failed for "
              << _nidsFile.c_str() << " error code = " << error <<  endl;
          break;
      }
      return 1;
    }
   
    if(_debug)
    {
      cerr << "NidsFile::compressData(): Data decompressed successfully" 
	   << endl;
      return 0; 
    }
  }
  else 
  { 
    //
    // Data were not compressed, just allocate space and copy them across
    //
    if(_debug)
    {
      cerr << "NidsFile::compressData(): Data was not compressed" << endl;
    }

    *decompressedBuf = (unsigned char *) malloc(numBytes);
    
    if (*decompressedBuf == NULL)
    {
      cerr << "NidsFile::decode():Memory allocation failed for " 
	   << _nidsFile.c_str() << " Exiting." << endl;

      exit(1);
    }

    memcpy(*decompressedBuf, compressedBuf, numBytes);

    bufSize = numBytes;
  }
 
  return 0;
}

//////////////////// Remap to cart ////////////

int NidsFile::remapToCart(double delta, double dist){


  PMU_auto_register("NidsFile::_remapToCart");

  double deltaToUse = delta;

  if (deltaToUse < 0.0) 
  {
    deltaToUse = _gateSpacing;
  }

  double distToUse=dist;

  if (distToUse < 0.0) 
  { 
    distToUse = _getMaxDist();
  }

  _dxy = deltaToUse;
  
  _nxy = (int)rint( 2.0 * distToUse / _dxy );

  _cartData = (fl32 *) malloc(_nxy * _nxy * sizeof(fl32));

  if (_cartData == NULL)
  {
    cerr << "NidsFile::remapToCart: Cartesian data malloc failed. Exiting.\n";
  
    exit(1);
  }

  double pi = acos( -1.0 );

  for (int ix=0; ix < _nxy; ix++)
  {
    for (int iy=0; iy < _nxy; iy++)
    {

      // Initialize data to missing

      _cartData[iy * _nxy + ix] = _missingFl32;

      // Figure out dist, az from (nx/2, ny/2) to
      // (ix, iy)

      double ewDist = double(ix - _nxy/2) * _dxy;
    
      double nsDist = double(iy - _nxy/2) * _dxy;

      double polarDist = sqrt(ewDist*ewDist + nsDist*nsDist);
      
      int radialIndex = (int)rint((polarDist - _firstGateDist)/_gateSpacing);
      
      if (radialIndex > _nGates)
      {
	//
	// Out of radar range
	//
	continue; 
      }

      //
      // Get index of closest azimuth
      //
      double az = 0.0;
      
      if ((nsDist == 0.0) && (ewDist == 0.0))
      {
	az = 0.0;
      }
      else
      {
	az = atan2(nsDist, ewDist);
      }

      az = 180.0*az/pi;
      
      az = 90.0-az;
      
      if (az < 0.0)
      {
	az += 360.0;
      }
      //
      //_getAzIndex( az );
      //
      int azIndex =  (int)rint(az/_delAz);

      if (azIndex < 0) 
      {
	azIndex = 0;
      }
      if (azIndex > _nRadials-1)
      {
	azIndex = _nRadials-1;
      }

      //
      // Move data from polar grid to cartesian grid
      //
      _cartData[iy * _nxy + ix] = _radialFl32s[azIndex * _nGates + radialIndex];
    }
  }

  return 0;
}

int  NidsFile::_getAzIndex( fl32 az )
{

  //
  // Ensure azimuth is between 0,359
  // 
  fl32 azimuth = az;
  
  if (azimuth < 0.0) 
  {
    azimuth += 360.0;
  }
   
  if (azimuth >= 360.00)
  { 
    azimuth -= 360.0;
  }
  
  //
  // Get integer index
  //
  int iazIndex = (int)rint( azimuth / _delAz );

  if (iazIndex < 0)
  { 
    iazIndex = 0;
  }

  
   
  if (iazIndex > _nRadials-1)
  { 
    iazIndex = 0;
  }
    
  return iazIndex;
}

double  NidsFile::_getMaxDist(){

  int maxGateIndex = 0;

  for (int i=0; i < _nRadials; i++)
  {
    for (int j=maxGateIndex; j < _nGates; j++)
    {
      if (_radialFl32s[i * _nGates + j] !=  _missingFl32) 
      {
	maxGateIndex = j;
      }
    }
  }

  return _firstGateDist + maxGateIndex * _gateSpacing;

}

double NidsFile::_decodeData(ui08 x){

  //
  // Returns a floating point physical value given a byte.
  // This is a rather involved encoding scheme.
  double val=0.0;
  
  val = _product->convertData(x);
 
  return val;

}

double NidsFile::_decodeData(ui16 x){

  //
  // Returns a floating point physical value given a byte.
  // This is a rather involved encoding scheme.
  double val=0.0;

  val = _product->convertData(x);

  return val;

}


void NidsFile::_decodeData(ui08 x, ui08 &run, fl32 &val)
{

  _product->convertRLEData(x,run,val);

}

void NidsFile::_decodeLinkedVectors(unsigned char* decompressedBuf, 
					   long unsigned int &offset)
{

  PMU_auto_register("NidsFile::_decodeLinkedVectors");

  //
  // Get the 4 Melting Layer contours-- we know this linked contour 
  // product is for the melting layer
  //
  for (int j = 0; j < 4; j++)
  {
    //
    // Decode set color level packet
    //
    GraphicProductMsg::setColorLevelHdr_t cH;
    
    GraphicProductMsg::readSetColorLevelHdr(cH, decompressedBuf, offset, 
					    _byteSwap, _debug);
    
    if(cH.code !=  (si16) 0x0802 ) // dec 2050
    {
      cerr << "Warning: Set Color Level Header value not expected. Code = "
	 << cH.code << ", 0802 Hex (2050 dec) expected" << endl;    
    return;
    }
    
    if( cH.colorIndicator == 2)
    {
      _colorValue = cH.colorValue;
    }
    else
    {
      //
      // Not set
      //
      _colorValue = -1;
    }
    
    //
    // Decode linked contour vector header
    //
    GraphicProductMsg::linkedVectorHeader_t v;
    
    GraphicProductMsg::readLinkedVectorHdr(v, decompressedBuf, offset, 
					   _byteSwap, _debug);
    
    if ( v.code != (si16)0x0E03) // dec 3587
    {
      cerr << "Warning: Linked Contour Vector packet code not expected. Code = "
	   << v.code << ", 0E03 Hex (3587 dec) expected." << endl;
      
      return;
    } 
    
    si16 initI;
    
    si16 initJ;
    
    bool gotStart = GraphicProductMsg::getContourInitPt(decompressedBuf, offset, 
							_byteSwap, _debug, initI,
							initJ);
    
    si16 numVec = GraphicProductMsg::getContourNumVectors(decompressedBuf, 
							  offset, _byteSwap, 
							  _debug);
    if (_debug) 
    { 
      if (gotStart)
      {
	cerr << " NidsFile::_decodeLinkedVectors(): Initial starting point " 
	     << "exists: I: " << initI << " J: " << initJ << endl;

	pair<si16,si16> contourPt(initI,initJ);

	_contours[j].push_back(contourPt);
      }
      else
      {
	cerr << " NidsFile::_decodeLinkedVectors(): No initial "
	     << "starting point." << endl;
      }
      
    }
    
    
    if (_debug)
    {
      cerr << " NidsFile::_decodeLinkedVectors(): number of vectors " 
	   << numVec << ". number of bytes = "<<  numVec *4 << endl;
    }
    
    for (int i = 0; i < numVec; i++)
    {
      si16 I, J;
      
      GraphicProductMsg::getContourVectorPt(decompressedBuf, offset, _byteSwap, 
					    _debug,  I, J);
      pair<si16,si16> contourPt(I,J);

      _contours[j].push_back(contourPt);
      
      if (_debug)
      {
	//cerr << " NidsFile::_decodeLinkedVectors(): vector[" << i 
	// <<"] = ("<< I << ", " << J <<") " << endl;
      }
    }
    
  }
}

int NidsFile::_decodeRadialMsg(unsigned char* decompressedBuf, 
			       long unsigned int &offset,
			       const long unsigned int bufSize)
{
  PMU_auto_register("NidsFile::_decodeRadialMsg");

  GraphicProductMsg::radial_t r;

  GraphicProductMsg::readRadialHdr(r, decompressedBuf, offset, _byteSwap,
				   _debug);
  
  //
  // Set radial data members
  //
  _nGates = r.nBins;

  _nRadials = r.nRadials;
  
  _delAz = double(r.delAz)/10.0;

  _firstGateDist = r.firstIndex*_gateSpacing;
  
  //
  // Make space for decoded data 
  //
  _allocateRadialSpace(_nGates, _nRadials, &_radialFl32s);


  int RLEhalfWords = r.nBytes;
 
  double azimuth = fmod( double(r.az)/10.0, 360);

  //
  // Loop through all radials
  //
 
  for (int i=0; i < _nRadials; i++)
  {

    //
    // We will count bytes for bookkeepping
    //
    long unsigned int startOffset =  offset; 
    
    //
    // Number of bytes in radial
    //
    int numBytes = RLEhalfWords * 2;
    
    //
    // Integer index of azimuth
    //
    int iazIndex = _getAzIndex( azimuth);

    //
    // Minimum data value
    //
    fl32 min = _missingFl32;
    
    //
    // Maximum data value
    //
    fl32 max = _missingFl32;
    
    //
    // Set gate to the first in radial or 0.
    // 
    int j = 0;
      
    for (int k = 0; k <  numBytes ; k++)
    {
      //
      // Number of gates that will get value
      //
      ui08 run = 0;
      
      //
      // Default value assigned to gates
      //
      fl32 val = _missingFl32;
      
      //
      // Check memory before since headers for RLE SRVEL have indicated 
      // more bytes than buffer(file) has 
      // 
      if ( offset < bufSize)
      {
	_decodeData(decompressedBuf[offset], run, val);
      }
      else
      {
	//
	// Return ok since RLE product SRVEL may have faulty info in headers
	// Size of data exceeds filesize). Partial files still decode properly
	// up until that point.
	//
	cerr << "NidsFile::_decodeRadialMsg(): WARNING not enough bytes in buffer "
	     << "to do requested read. File is: " <<  _nidsFile.c_str() <<  endl; 

	return 0;
      }
      
      if ((int)run > 0)
      {
	for(int irun = 0; irun < (int)run; irun++)
	{
	  _radialFl32s[iazIndex * _nGates + j]=val;
	  
	  j++;
	}	
      }

      offset++; 
    }
    
    if (i <  r.nRadials -1)
    {
      GraphicProductMsg::smallRadBlock_t smallRadHdr;

      //
      // Check memory before reading since headers for RLE SRVEL have 
      // indicated more bytes than buffer(file) has 
      // 
      if ( offset + sizeof(smallRadHdr) > bufSize)
      {
	//
	// Return ok since RLE product SRVEL may have faulty info in headers
	// (Size of data exceeds filesize). Partial files still decode properly
	// up until that point.
	//
	cerr << "NidsFile::_decodeRadialMsg(): WARNING not enough bytes in file "
	     << "to do read of small radial header block. The file is: " <<   _nidsFile.c_str() << endl; 

	return 0;
      }

      GraphicProductMsg::readSmallRadHdr(smallRadHdr, decompressedBuf, offset, 
					 _byteSwap, _debug);
      
      RLEhalfWords = smallRadHdr.num;
      
      azimuth = fmod( (double)(smallRadHdr.az)/10.0, 360 ) ;
    }
  }
  
  return 0;
  
}

int  NidsFile::_decodeDigitalRadialMsg(unsigned char* decompressedBuf, 
				       long unsigned int &offset)
{

  PMU_auto_register("NidsFile::_decodeDigitalRadialMsg");

  GraphicProductMsg::radial_t r;
  
  GraphicProductMsg::readRadialHdr(r, decompressedBuf, offset, _byteSwap, 
				   _debug);

  if ( r.msgCode != 16)
  {
    cerr << "NidsFile::_decodeDigitalRadialMsg(): WARNING: Message "
	 << "packet code != 16 as expected. No further decoding" << endl;
    return 1;
  }
  
  //
  // Set radial data members
  //
  _nGates = r.nBins;

  _nRadials = r.nRadials;
  
  _delAz = double(r.delAz)/10.0;

  _firstGateDist = r.firstIndex*_gateSpacing;
  
  //
  // Make space for decoded data 
  //
  _allocateRadialSpace(_nGates, _nRadials, &_radialFl32s);

  //
  // Get the number of bytes per radial. This may be greater than the number
  // of gates due to packing issues.
  //
  int numBytes = r.nBytes;
 
  double azimuth = fmod(double(r.az)/10.0, 360);

  //
  // Loop through all radials
  //
  for (int i=0; i < _nRadials; i++){

    //cerr << "azimuth: " << azimuth << endl;
	
    int iazIndex = _getAzIndex(azimuth);

    //cerr << "iazindex: " << iazIndex << endl;	

    //cerr << "_delAz: " << _delAz << endl;

    
    fl32 min = _missingFl32;

    fl32 max = _missingFl32;

    bool first = true;

    //
    // Loop through all gates for this radial
    //
    for (int j=0; j < _nGates; j++)
    {	  
      if ((decompressedBuf[offset] > 1) && (decompressedBuf[offset] < 255))
      {
	//
	// Data value is not missing.
	//
	double val = _decodeData(decompressedBuf[offset]);
	
	_radialFl32s[iazIndex * _nGates + j] = val;

	//
	// Keep track of max and min values for debugging
	//
	//_trackMaxMin(first, max, min, val);
      } 
      else 
      { 
	//
	// Data value is missing.
	//
	_radialFl32s[iazIndex * _nGates + j]=_missingFl32;
      
      }
      offset++;

    }
    
    // Because there may be trailing bytes - numBytes != nGates, necessarily
    
    offset += (numBytes-_nGates);

    if (i <  r.nRadials -1)
    {
      GraphicProductMsg::smallRadBlock_t smallRadHdr;

      GraphicProductMsg::readSmallRadHdr(smallRadHdr,  decompressedBuf, offset,
					 _byteSwap, _debug);
      
      numBytes = smallRadHdr.num;
      
      azimuth = fmod( double(smallRadHdr.az)/10.0, 360) ;
    }
  }

  return 0;
}

int NidsFile::_decodeGenericMsg(unsigned char* decompressedBuf, long unsigned int &offset)
{

  PMU_auto_register("NidsFile::_decodeGenericMsg");
  
  GraphicProductMsg::genericHeader_t h;

  GraphicProductMsg::readGenericHdr( h, decompressedBuf, offset, _byteSwap,
				     _debug);
  
  RPGP_product_t *p;

  int ret = RPGP_product_deserialize((char*)decompressedBuf + offset, h.numBytes, (void**)&p);
  
  if ( ret)
  {
    cerr << "NidsFile::_decodeGenericMsg(): Deserializing "
	 << " was unsuccessful. No more processing" << endl; 
    
    return 1;
  }
  
  RPGP_radial_t *rptr = (RPGP_radial_t *) *(p->components);
  
  _nRadials=rptr->numof_radials;
  
  _firstGateDist = rptr->first_range/1000;
  
  RPGP_radial_data_t *radialData = (RPGP_radial_data_t *) (rptr->radials);
  
  _nGates = radialData->n_bins; 
  
  _delAz =  radialData->width;
  
  //
  // Make space for decoded data 
  //
  _allocateRadialSpace(_nGates, _nRadials, &_radialFl32s );

  for (int i = 0; i < _nRadials; i++) 
  {  
    RPGP_radial_data_t radial = (RPGP_radial_data_t)radialData[i];

    int iazIndex = _getAzIndex(radial.azimuth);
    
    for (int j = 0; j < _nGates; j++) 
    {    
      ui16 *binData =  (ui16*) radial.bins.data;
      
      _radialFl32s[iazIndex*_nGates + j] = _decodeData(binData[j]);
      
    }
  }

  //
  // Cleanup
  //
  ret = RPGP_product_free(p);

  if ( ret)
  {
    cerr << "NidsFile::_decodeGenericMsg(): Freeing memory for "
	 << " RPGP_product_t was unsuccessful. May have a memory leak." 
	 << endl; 
    
    return 1;
  }

  return 0;
  
}
 
int NidsFile::_getDecompressedData(unsigned char **decompressedBuf, 
				   FILE *fp, unsigned int &bufSize)
{

  PMU_auto_register("NidsFile::getDecompressedData");

  //
  // Stat the file to get the number of bytes in file
  //
  struct stat inputFile;
 
  if (stat(_nidsFile.c_str(), &inputFile))
  {
    cerr << "NidsFile::decode():Failed to stat " << _nidsFile.c_str() 
	 << endl;

    return 1;
  }

  //
  // Get offset to the graphical data from the Product object. The units of 
  // the offset are in 2 byte half words.
  // See (Product Description Block in ICD Figure 3-6)
  //
  int offset = 0;
  
  offset  =  2 *_product->graphProdDesc.symbOff;

  long unsigned int bytesToRead;

  if (_hasExtraHdr)
  {
    //
    // Product has the NCDC or the WMO 30 byte header
    //
    bytesToRead = inputFile.st_size - offset - 30;
  }
  else
  {
    bytesToRead = inputFile.st_size - offset;
  }

  //
  // Create a buffer and copy radial data to buffer
  //
  unsigned char *dataBuf = (unsigned char *) malloc(bytesToRead);


  if (dataBuf == NULL)
  {
    cerr << "NidsFile::decode(): Failed to allocate memory. End of " 
	 << "processing. Exiting." << endl;

    exit(1);
  }
  else
  {
     memset(dataBuf, 0, bytesToRead);
  }

  long unsigned int numBytesRead = fread(dataBuf, sizeof(unsigned char), 
				    bytesToRead, fp);
  
  if (bytesToRead != numBytesRead)
  {
    if (_debug) cerr << "NidsFile::decode(): Read of radial data bytes "
		     << "failed for " << _nidsFile << ". End of processing" 
		     << endl;
    return 1;
  }

  //
  // Decompress data if necessary
  //  
  if ( _decompress(dataBuf,decompressedBuf,numBytesRead, bufSize) )
  {
    //
    // Decompression failed
    //
    return 1;
  }

  free (dataBuf);

  return 0;
}


void NidsFile::_setProductDescrMembers()
{
  _time = (_product->graphProdDesc.volScanDate-1) * 86400 + 
    _product->graphProdDesc.volScanTime;

  _lat = double(_product->graphProdDesc.lat/1000.0);
  
  _lon = double(_product->graphProdDesc.lon/1000.0);
  
  _alt = double(_product->graphProdDesc.elevFeet)*0.3; 

  _elevAngle = _product->getElevAngle();

  _volNum = _product->graphProdDesc.volScanNum;

  _elNum = _product->graphProdDesc.elNum;
}

void NidsFile::_allocateRadialSpace( int _nGates, int _nRadials, 
					   fl32 **radialData )
{  
  //
  // Allocate space for radial data
  //
  *radialData = (fl32 *)malloc(_nGates*_nRadials*sizeof(fl32));

  if (*radialData == NULL)
  {
    cerr << " NidsFile::_decodeDigitalRadialMsg(): Malloc failed." 
	 << " Exiting" << endl;

    exit(1);
  }

  //
  // Initialize radials to missing.
  //
  for (int k=0; k < _nGates*_nRadials; k++)
  {
    (*radialData)[k] = _missingFl32;
  }
}

void NidsFile::_trackMaxMin(bool &first, float &max, float &min, float val)
{
   if (first)
   {
     min = val;
     
     max = val;
     
     first = false;
   } 
   else
   {
     if (val > max)
     { 
       max = val;
     }

     if (val < min)
     {
       min = val;
     }
   }
 }

void NidsFile::getContourPt (int contourNum, int ptIndex, si16 &i, si16 &j)
{
  if ( _contours[contourNum].size() > ptIndex )
  {
    i =  _contours[contourNum][ptIndex].first;
    
    j = _contours[contourNum][ptIndex].second;
  }
}


void NidsFile::_decodeGraphicSymbols( unsigned char* decompressedBuf, 
				      long unsigned int &offset, int dataLayerLen)
{ 

  PMU_auto_register("NidsFile::_decodeGraphicSymbols");

  int startOffset = offset;

  while ( offset - startOffset <  dataLayerLen)
  {
    GraphicProductMsg::specialGraphicSymbolPacket_t graphicSymbol;
    
    GraphicProductMsg::readSpecialGraphicSymbolHdr(graphicSymbol, decompressedBuf, 
						   offset, _byteSwap, _debug);

    if( graphicSymbol.code == 12 || graphicSymbol.code == 26)
    {
      int numTvsPts =  graphicSymbol.numBytes/4;
  
      for (int i = 0; i < numTvsPts; i++)
      {
	si16 I, J;
	
	GraphicProductMsg::getContourVectorPt(decompressedBuf, offset, _byteSwap, 
					      _debug,  I, J);
	
	pair<si16,si16> contourPt(I,J);
	
	_tvsSymbols.push_back(contourPt);
     
	if (_debug)
	{
	  cerr << " NidsFile::_decodeTvs(): tvsLoc[" << i 
	       <<"] = ("<< I << ", " << J <<") " << endl;
	}     
      }
    }

    else if (graphicSymbol.code == 19)
    {
      int numHailSymbols =  graphicSymbol.numBytes/10;
      
      for (int i = 0; i < numHailSymbols; i++)
      {	
	GraphicProductMsg::hdaHail_t hail;
	
	GraphicProductMsg::readHdaHail(hail, decompressedBuf, offset,_byteSwap, _debug);
	
	_hailSymbols.push_back(hail);   
      }
    }

    else if ( graphicSymbol.code == 15)
    {
      GraphicProductMsg::stormID_t stormID;

      GraphicProductMsg::readStormID(stormID, decompressedBuf, 
				     offset, _byteSwap, _debug);

      _stormIDs.push_back(stormID);
    }
    
    else if ( graphicSymbol.code == 20 )
    {
      GraphicProductMsg::mesocyclone_t mesocyclone;

      GraphicProductMsg::readMesocyclone(mesocyclone, decompressedBuf, 
					 offset, _byteSwap, _debug);

      _mesocycSymbols.push_back(mesocyclone);
    }
    else
    {
      cerr << "NidsFile::_decodeGraphicSymbols(): WARNING graphic product code " 
	   << graphicSymbol.code << " not recognized. Skipping data packet" << endl;
      
      offset += graphicSymbol.numBytes;
    }
  }    
}

void NidsFile::getTvsPt(int i, int &I, int &J)
{
  I = _tvsSymbols[i].first;

  J  = _tvsSymbols[i].second;

}

void NidsFile::getHailPt(int i, int &I, int &J)
{
  I = _hailSymbols[i].I;

  J = _hailSymbols[i].J;
}

void NidsFile::getMesocycPt(int i, int &I, int &J)
{
   I = _mesocycSymbols[i].I;

   J = _mesocycSymbols[i].J;
}

void NidsFile::getStormIDPt(int i, int &I, int &J)
{
  I = _stormIDs[i].I;

  J = _stormIDs[i].J;
}
const string NidsFile::getStormID(int i)
{

  string idStr("");

  idStr.push_back(_stormIDs[i].char1);

  idStr.push_back(_stormIDs[i].char2);

  return idStr;
}
const string NidsFile:: getMesocycTypeStr(int i)
{
  string typeStr;

   switch (  _mesocycSymbols[i].type )
   {
   case 1: 
   case 2:
       typeStr = "MesoExtrap";
     break;
     
   case 3:
   case 4:
     typeStr = "MesoNewIncr";
     break;
     
   case 5:
     typeStr = "TVSExtrap";
     break;
     
   case 6:
     typeStr = "ETVSExtrap";
     break;
     
   case 7:
     typeStr = "TVSNewIncr";
     break;
     
   case 8:
      typeStr = "ETVSNewIncr";
     break;
     
   case 9:
     typeStr = "MDA5+Base<=1km";
     break;
     
   case 10:
       typeStr = "MDA5+Base>1km";
     break;
     
   case 11:
      typeStr = "MDA5-";
     break; 
     
   default:
     typeStr = "MesoTypeUnknown";
  } 

   return typeStr;
}
