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

#include <iostream>
#include <stdlib.h>
#include <cstdio>

#include <toolsa/umisc.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include "hiResRadialFile.hh"
#include "GraphicProductMsg.hh"
#include "EchoTopsProduct.hh"
#include "DiffReflProduct.hh"
#include "SpecDiffPhaseProduct.hh"
#include "StormPrecipProduct.hh"
#include "HydroClassProduct.hh"
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
#include "orpg_product.h"

const fl32 hiResRadialFile::_missingFl32 = -999.0;


hiResRadialFile::hiResRadialFile(const char *filePath, bool byteSwap, 
				 bool debug):
  _nidsFile(filePath),
  _byteSwap(byteSwap),
  _debug(debug),
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
  _linkedVectorPacket(false)
{
  _contours.resize(4);
  if (_debug)
  {
    cerr << "hiResRadialFile class instantiated for " << _nidsFile << endl;
  }
}

//
// Destructor - frees memory.
//
hiResRadialFile::~hiResRadialFile()
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
	 

  return;
}

int hiResRadialFile::decode() 
{
  //
  // Open NIDS file
  //
  FILE *fp = fopen(_nidsFile.c_str(), "r");
  if (fp == NULL)
  {
    
    cerr << "hiResRadialFile::decode(): Could not open " << _nidsFile.c_str() 
	 << ". No processing of file." << endl;
    return 1;
  }

  //
  // Read Message Header Block
  //
  GraphicProductMsg::msgHdrBlk_t msgHdr;

  if (GraphicProductMsg::readMessageHeaderBlock(msgHdr, fp, _byteSwap,
						_debug) )
  {
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
  switch (msgHdr.msgCode){
  case 78:
    _product = new OneHourPrecipTotal(fp,  _byteSwap, _debug);
    _runLengthEncoding = true;
    _gateSpacing = 2;
    break;
  case 80:
    _product = new StormTotalPrecip(fp,  _byteSwap, _debug);
    _runLengthEncoding = true;
    _gateSpacing = 2;
    break;
  case 94:
    _product = new BaseReflProduct(fp,  _byteSwap, _debug);
    _gateSpacing = 1;
    break;
  case 99:
    _product = new BaseVelProduct(fp,  _byteSwap, _debug);
    _gateSpacing = .25;
    break;
  case 134:
    _product = new VilProduct(fp,  _byteSwap, _debug);
    _gateSpacing = 1;
    break;
  case 135:
    _product = new EchoTopsProduct(fp,  _byteSwap, _debug);
    _gateSpacing = 1;
    break;
  case 138:
    _product = new StormPrecipProduct(fp,  _byteSwap, _debug);
    _gateSpacing = 1;
    break;
  case 159: 
    _product = new DiffReflProduct(fp,  _byteSwap, _debug);
    _gateSpacing = .25;
    break;  
  case 161:
    _product = new CorrCoef(fp,  _byteSwap, _debug);
    _gateSpacing = .25;
    break;
  case 163:
    _product = new SpecDiffPhaseProduct(fp,  _byteSwap, _debug);
    _gateSpacing = .25;
    break;
  case 165: 
    _product = new HydroClassProduct(fp,  _byteSwap, _debug);
    _gateSpacing = .25;
    break;
  case 166:
    _product = new MeltingLayer(fp,  _byteSwap, _debug);
    _linkedVectorPacket = true;
    break;
  case 170: 
    _product = new DigitalAccumArray(fp,  _byteSwap, _debug);
    _gateSpacing = .25;
    break;  
  case 172: 
    _product = new DigitalTotalAccum(fp,  _byteSwap, _debug);
    _gateSpacing = .25;
    break;    
  case 176:
    _product = new DigitalPrecipRate(fp, _byteSwap, _debug);
    _gateSpacing = .25;
    _genericRadialFormat = true;
    break;

  default:
    cerr << "hiResRadialFile::decode(): Product decoder for product " 
	 <<  msgHdr.msgCode << " not found. End of processing." << endl;
    return 1;  
  }
 
  //
  // Read Product Decscription Block from Graphic Product Message. See ICD 
  // Figure3-6 (Sheets 2 and 6)
  //
  if (_product->readFromFile() == Product::PRODUCT_FAILURE)
  {
    cerr << "hiResRadialFile::decode(): Failure to read Product Description "
	 << "block. End of processing." << endl;
    
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
  unsigned char *decompressedBuf;

  if (_getDecompressedData(&decompressedBuf, fp))
  {
      //
      // Not successful
      //
      return 1;
  }

  //
  // Close file
  //
  fclose(fp);

  //
  // Initialize offset with which to go through rest of data in memory
  //
  unsigned long offset = 0;

  //
  // Read Symbology Block Header, advance offset to radial data
  //
  GraphicProductMsg::prodSymbBlk_t psb;

  GraphicProductMsg::readSymbologyBlockHdr(psb, decompressedBuf, offset, 
					   _byteSwap, _debug);
  
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
    _decodeRadialMsg(decompressedBuf, offset);
  }
  else if (_genericRadialFormat)
  {
    //
    // Radial data packet code 28
    //
    _decodeGenericMsg( decompressedBuf, offset);
  }
  else
  {
    //
    // Radial data packet code 16
    //
    _decodeDigitalRadialMsg(decompressedBuf, offset);
  }
    
  free(decompressedBuf);

  return 0;
}

int hiResRadialFile::_decompress( unsigned char *compressedBuf,  
				  unsigned char **decompressedBuf,
				  unsigned int numBytes)
{
  if ( _product->isCompressed() )
  {
    if(_debug)
    {
      cerr << "hiResRadialFile::decompressData(): Data is compressed." << endl;
    }

    //
    // Get the size
    //
    unsigned int size = _product->getUncompProdSize();

    //
    // Allocate the memory
    //
    *decompressedBuf = (unsigned char *)malloc(size);

    if (*decompressedBuf == NULL)
    {
	cerr << "hiResRadialFile::decompressData(): Memory allocation failed "
	     << "for " << _product->getUncompProdSize() << " bytes. Exiting."  
	     << endl;

	exit(-1);
    }

    //
    // Run decompression method
    //
    int error = BZ2_bzBuffToBuffDecompress((char *) *decompressedBuf, &size,
					   (char *) compressedBuf,numBytes, 
					   0, 0);
    if (error)
    {
      free(*decompressedBuf);
      
      cerr << "hiResRadialFile::compressData():Decompression failed for " 
	   << _nidsFile.c_str() << endl;

      return 1;
    }
    
    if(_debug)
    {
      cerr << "hiResRadialFile::compressData(): Data decompressed successfully" 
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
      cerr << "hiResRadialFile::compressData(): Data was not compressed" << endl;
    }

    *decompressedBuf = (unsigned char *) malloc(numBytes);
    
    if (*decompressedBuf == NULL)
    {
      cerr << "hiResRadialFile::decode():Memory allocation failed for " 
	   << _nidsFile.c_str() << endl;

      exit(-1);
    }

    memcpy(*decompressedBuf, compressedBuf, numBytes);
  }
 
  return 0;
}

//////////////////// Remap to cart ////////////

void hiResRadialFile::remapToCart(double delta, double dist){

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
    cerr << "Cartesian data malloc failed, nxy= " << _nxy;
  
    exit(-1);
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

  return;
}

int  hiResRadialFile::_getAzIndex( fl32 az )
{
  //
  // Ensure azimuth is between 0,359
  // 
  fl32 azimuth = az;
  
  if (azimuth < 0.0) 
  {
    azimuth += 360.0;
  }
   
  if (azimuth > 359.95)
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
   
  if (iazIndex > _nRadials -1)
  { 
    iazIndex = _nRadials -1;
  }
    
  return iazIndex;
}

double  hiResRadialFile::_getMaxDist(){

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

double hiResRadialFile::_decodeData(ui08 x){
  //
  // Returns a floating point physical value given a byte.
  // This is a rather involved encoding scheme.
  double val=0.0;
  
  val = _product->convertData(x);
 
  return val;

}

void hiResRadialFile::_decodeData(ui08 x, ui08 &run, fl32 &val)
{

  _product->convertRLEData(x,run,val);

}

void hiResRadialFile::_decodeLinkedVectors(unsigned char* decompressedBuf, 
					   unsigned long &offset)
{
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
	cerr << " hiResRadialFile::_decodeLinkedVectors(): Initial starting point " 
	     << "exists: I: " << initI << " J: " << initJ << endl;

	pair<si16,si16> contourPt(initI,initJ);

	_contours[j].push_back(contourPt);
      }
      else
      {
	cerr << " hiResRadialFile::_decodeLinkedVectors(): No initial "
	     << "starting point." << endl;
      }
      
    }
    
    
    if (_debug)
    {
      cerr << " hiResRadialFile::_decodeLinkedVectors(): number of vectors " 
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
	//cerr << " hiResRadialFile::_decodeLinkedVectors(): vector[" << i 
	// <<"] = ("<< I << ", " << J <<") " << endl;
      }
    }
    
  }
}

void hiResRadialFile::_decodeRadialMsg(unsigned char* decompressedBuf, 
				       unsigned long &offset)
{
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
  _allocateRadialSpace(_nGates, _nRadials, &_radialFl32s );
 
  int RLEhalfWords = r.nBytes;
 
  double azimuth = double(r.az)/10.0;

  //
  // Loop through all radials
  //
  for (int i=0; i < _nRadials; i++){

    //
    // We will count bytes for bookkeepping
    //
    unsigned long startOffset =  offset; 
    
    //
    // Number of bytes in radial
    //
    int numBytes = RLEhalfWords * 16;

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
    // First pass through loop
    //
    bool first = true;

    //
    // Set all gate values using Run Length Decoding
    //
    for (int j=0; j < _nGates;)
    {     
      //
      // Number of gates that will get value
      //
      ui08 run = 0;

      //
      // Value assigned to gates
      //
      fl32 val;

      _decodeData(decompressedBuf[offset], run, val);
	
      if (run > 0)
      {
	for(int irun = 0; irun < run; irun++)
	{
	  _radialFl32s[iazIndex * _nGates + j]=val;
	  
	  j++;
	}	

	//
	// Keep track of max and min values for debugging
	//
	_trackMaxMin(first, max, min, val);
      }
      
      offset++;
    }

    unsigned long endOffset  = offset;

    //
    // There may be extra bytes due to data packing
    //
    offset += (numBytes - ((endOffset - startOffset)*8))/8 ;

    if (_debug)
    {
      if (first)
      {
	cerr << "  All radial data is missing." << endl;
      } 
      else 
      {
	cerr << "  Values run from " << min << " to " << max << endl;
      }
    }

    if (i <  r.nRadials -1)
    {
      GraphicProductMsg::smallRadBlock_t smallRadHdr;

      GraphicProductMsg::readSmallRadHdr(smallRadHdr, decompressedBuf, offset, 
					 _byteSwap, _debug);

      RLEhalfWords = smallRadHdr.num;

      _delAz= (smallRadHdr.dAz)/10;

      azimuth = double(smallRadHdr.az)/10.0;
    }
  }
}

void hiResRadialFile::_decodeDigitalRadialMsg(unsigned char* decompressedBuf, 
					      unsigned long &offset)
{

  GraphicProductMsg::radial_t r;
  
  GraphicProductMsg::readRadialHdr(r, decompressedBuf, offset, _byteSwap, 
				   _debug);

  if ( r.msgCode != 16)
  {
    cerr << "hiResRadialFile::_decodeDigitalRadialMsg(): WARNING: Message "
	 << "packet code != 16 as expected." << endl;
    return;
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
  _allocateRadialSpace(_nGates, _nRadials, &_radialFl32s );

  //
  // Get the number of bytes per radial. This may be greater than the number
  // of gates due to packing issues.
  //
  int numBytes = r.nBytes;
 
  double azimuth = double(r.az)/10.0;

  //
  // Loop through all radials
  //
  for (int i=0; i < _nRadials; i++){


    int iazIndex = _getAzIndex(azimuth);

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
	_trackMaxMin(first, max, min, val);
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

    //
    // Now adjust offset to next radial appropriately. There may be trailing 
    // bytes  due to data packing ( numBytes != nGates, necessarily)
    //
    offset += (numBytes-_nGates); 

    // if (_debug)
    // {
    //   if (first)
    //   {
    // 	cerr << "  All missing data in radial." << endl;
    //   } 
    //   else 
    //   {
    // 	cerr << "  Values run from " << min << " to " << max << endl;
    //   }
    // }

    if (i <  r.nRadials -1)
    {
      GraphicProductMsg::smallRadBlock_t smallRadHdr;

      GraphicProductMsg::readSmallRadHdr(smallRadHdr,  decompressedBuf, offset,
					 _byteSwap, _debug);
      
      numBytes = smallRadHdr.num;
      
      azimuth = double(smallRadHdr.az)/10.0;
    }
  }
}

void hiResRadialFile::_decodeGenericMsg(unsigned char* decompressedBuf, unsigned long &offset)
{
  
  GraphicProductMsg::genericHeader_t h;

  GraphicProductMsg::readGenericHdr( h, decompressedBuf, offset, _byteSwap,
				     _debug);
  
  RPGP_product_t *p;

  int ret = RPGP_product_deserialize((char*)decompressedBuf + offset, h.numBytes, (void**)&p);
  
  if ( ret)
  {
    cerr << "hiResRadialFile::_decodeGenericMsg(): Deserializing "
	 << " was unsuccessful. No more processing" << endl; 
    
    return;
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
    cerr << "hiResRadialFile::_decodeGenericMsg(): Freeing memory for "
	 << " RPGP_product_t was unsuccessful. May have a memory leak." 
	 << endl; 
    
    return;
  }
  
}
 
int hiResRadialFile::_getDecompressedData(unsigned char **decompressedBuf, 
					  FILE *fp)
{
  //
  // Stat the file to get the number of bytes in file
  //
  struct stat inputFile;
 
  if (stat(_nidsFile.c_str(), &inputFile))
  {
    cerr << "hiResRadialFile::decode():Failed to stat " << _nidsFile.c_str() 
	 << endl;

    return 1;
  }

  //
  // Get offset to the radial data from the Product object. The units of 
  // the offset are in 2 byte half words.
  // See (Product Description Block in ICD Figure 3-6)
  //
  unsigned int bytesToRead = inputFile.st_size -( 2 *_product->graphProdDesc.symbOff);

  //
  // Create a buffer and copy radial data to buffer
  //
  unsigned char *dataBuf = (unsigned char *) malloc(bytesToRead);

  if (dataBuf == NULL)
  {
    cerr << "hiResRadialFile::decode(): Failed to allocate memory. End of " 
	 << "processing. Exiting." << endl;

    exit(-1);
  }

  unsigned int numBytesRead = fread(dataBuf, sizeof(unsigned char), 
				    bytesToRead, fp);
  
  if (bytesToRead != numBytesRead)
  {
    if (_debug) cerr << "hiResRadialFile::decode(): Read of radial data bytes "
		     << "failed for " << _nidsFile << ". End of processing" 
		     << endl;
    return 1;
  }

  //
  // Decompress data if necessary
  //  
  if ( _decompress(dataBuf,decompressedBuf,numBytesRead) )
  {
    //
    // Decompression failed
    //
    return 1;
  }

  free (dataBuf);

  return 0;
}


void hiResRadialFile::_setProductDescrMembers()
{
  _time = (_product->graphProdDesc.volScanDate-1) * 86400 + 
    _product->graphProdDesc.volScanTime;

  _lat = double(_product->graphProdDesc.lat/1000.0);
  
  _lon = double(_product->graphProdDesc.lon/1000.0);
  
  _alt = double(_product->graphProdDesc.elevFeet)*0.3; 

  _elevAngle = _product->getElevAngle();

  _volNum = _product->graphProdDesc.volScanNum;
}

void hiResRadialFile::_allocateRadialSpace( int _nGates, int _nRadials, 
					    fl32 **radialData )
{  
  //
  // Allocate space for radial data
  //
  *radialData = (fl32 *)malloc(_nGates*_nRadials*sizeof(fl32));

  if (radialData == NULL)
  {
    cerr << " hiResRadialFile::_decodeDigitalRadialMsg(): Malloc failed." 
	 << " Exiting" << endl;

    exit(-1);
  }

  //
  // Initialize radials to missing.
  //
  for (int k=0; k < _nGates*_nRadials; k++)
  {
    (*radialData)[k] = _missingFl32;
  }
}

 void hiResRadialFile::_trackMaxMin(bool &first, float &max, float &min, float val)
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

void hiResRadialFile::getContourPt (int contourNum, int ptIndex, si16 &i, si16 &j)
{
  if ( _contours[contourNum].size() > ptIndex )
  {
    i =  _contours[contourNum][ptIndex].first;
    
    j = _contours[contourNum][ptIndex].second;
  }
}
