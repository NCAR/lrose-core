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
#include "GraphicProductMsg.hh"

using namespace std;


#include <iostream>
#include <stdlib.h>
#include <cstdio>
#include <cstring>
#include <toolsa/umisc.h>
#include "Swap.hh"

int GraphicProductMsg::readMessageHeaderBlock(msgHdrBlk_t &msgHdr, FILE *fp,
					      bool byteSwap, bool debug, bool hasHdr = false) 
{
  char buf[100];

  if (hasHdr)
  {
    fread(&buf[0],1,30,fp);
  }

  //
  // Read message header into struct
  //
  if (1 != fread(&msgHdr, sizeof(msgHdr), 1, fp))
  { 
    cerr << "GraphicProductMsg::decode(): Unable to read message header." 
	 <<  " No processing of file." << endl;
    
    return 1;
  }
  
  //
  // Byte swap
  //
  if(byteSwap)
  {
    _swapMsgHdrBlk(&msgHdr);
  }

  //
  // Print
  //
  if (debug)
  {
    printMsgHdrBlk(msgHdr);
  }
 
  return 0;
}

int GraphicProductMsg::readProdDescr(productDescription_t &graphProdDesc, 
				    FILE *fp, bool byteSwap, bool debug)
{
  //
  // Read the graphic product message
  //
  if (1 != fread(&graphProdDesc, sizeof(productDescription_t), 1, fp))
  {
    if (debug)
    {
      cerr << "Product::readFromFile(): Failed to read graphic "
	   << "product message block. " << endl;
    }
    return 1;
  }

  //
  // Byte swap header
  //
  if(byteSwap)
  {
    _swapProdDescr(&graphProdDesc);
  }

  if(debug)
  {
    printProdDescr(graphProdDesc);
  }

  return 0;
}

void  GraphicProductMsg::readSymbologyBlockHdr(prodSymbBlk_t &psb,
					       unsigned char *decompressedBuf,
					       long unsigned int &offset,
					       bool byteSwap,
					       bool debug)
{
  //
  // Read symbology block header into struct
  // 
  memcpy(&psb, decompressedBuf+offset, sizeof(psb)); 

  //
  // Advance buffer offset appropriately
  //
  offset+=sizeof(psb);
  
  //
  // Byte swap header
  //
  if(byteSwap)
  {
    _swapProdSymbBlk( &psb ); 
  }

  //
  // Print
  //
  if (debug)
  {
    printPrdSymbBlk( psb );
  }
}

void GraphicProductMsg::readGraphAlphaNumHdr(graphicAlphaNumericBlk_t &g, 
					     unsigned char *decompressedBuf, 
					     unsigned long int &offset,
					     bool byteSwap,
					     bool debug)
{      
  //
  // Read graphic alphanumeric header into a struct
  //
  memcpy(&g, decompressedBuf+offset, sizeof(g)); 

  //
  // Advance offset appropriately
  //
  offset+=sizeof(g);
      
  //
  // Byte swap
  //
  if(byteSwap)
  {
    _swapGraphicAlphaNumericHdr( &g );
  }
   
  //
  // Print
  //
  if (debug) 
  {
    printGraphicAlphaNumericHdr( g );
  }     
}

void GraphicProductMsg::readRadialHdr( radial_t &r, 
				       unsigned char *decompressedBuf, 
				       long  unsigned int &offset,
				       bool byteSwap,
				       bool debug)
{
  //
  // Read radial header into struct
  //
  memcpy(&r, decompressedBuf+offset, sizeof(r)); 

  //
  // Advance offset appropriately
  //
  offset+=sizeof(r);
 
  //
  // Byte swap
  //
  if(byteSwap)
  {
    _swapRadial( &r );
  }

  //
  // Print
  //
  if(debug)
  {
    printRadial( r );
  }
}

void GraphicProductMsg::readSmallRadHdr( smallRadBlock_t & sb, 
					 unsigned char *decompressedBuf, 
					 unsigned long int &offset,
					 bool byteSwap,
					 bool debug)
{      
  //
  // Read small radial header into struct
  //
  memcpy(&sb, decompressedBuf+offset, sizeof(sb)); 

  //
  // Advance offset appropriately
  //
  offset+=sizeof(sb);
      
  //
  // Byte swap
  //
  if(byteSwap)
  {
    _swapSmallRadBlock( &sb );
  }
   
  //
  // Print
  //
  if (debug) 
  {
    //printSmallRadBlock( sb );
  }     
}

void GraphicProductMsg::readGenericHdr( genericHeader_t &genHdr,  
					unsigned char *decompressedBuf,
					unsigned long int &offset,
					bool byteSwap,
					bool debug)
{
  //
  // Read generic header into struct
  //
  memcpy(&genHdr, decompressedBuf+offset, sizeof(genHdr)); 

  //
  // Advance offset appropriately
  //
  offset+=sizeof(genHdr);

  //
  // Byte swap
  //
  if(byteSwap)
  {
   _swapGenericHdr( &genHdr ); 
  }

  //
  // Print
  //
  if(debug)
  {
    printGenericHdr(genHdr); 
  }
}

void GraphicProductMsg::readSetColorLevelHdr( setColorLevelHdr_t &cHdr, 
					      unsigned char *decompressedBuf, 
					      unsigned long int &offset,
					      bool byteSwap,
					      bool debug)
{      
  //
  // Read linked vector header into a struct
  //
  memcpy(&cHdr, decompressedBuf+offset, sizeof(cHdr)); 

  //
  // Advance offset appropriately
  //
  offset+=sizeof(cHdr);
      
  //
  // Byte swap
  //
  if(byteSwap)
  {
    _swapSetColorLevelHdr( &cHdr );
  }
   
  //
  // Print
  //
  if (debug) 
  {
    printSetColorLevelHdr(cHdr);
  }     
}

void GraphicProductMsg::readLinkedVectorHdr( linkedVectorHeader_t &v, 
					     unsigned char *decompressedBuf, 
					     unsigned long int &offset,
					     bool byteSwap,
					     bool debug)
{      

  //
  // Read linked vector header into a struct
  //
  memcpy(&v, decompressedBuf+offset, sizeof(v)); 

  //
  // Advance offset appropriately
  //
  offset+=sizeof(v);
      
  //
  // Byte swap
  //
  if(byteSwap)
  {
    _swapLinkedVectorHdr( &v );
  }
   
  //
  // Print
  //
  if (debug) 
  {
    printLinkedVectorHdr( v );
  }     
}


void GraphicProductMsg::readSpecialGraphicSymbolHdr(specialGraphicSymbolPacket_t &g, 
					  unsigned char *decompressedBuf, 
					  unsigned long int &offset,
					  bool byteSwap,
					  bool debug)
{

  //
  // Read special graphic symbol header into a struct
  //
  memcpy(&g, decompressedBuf+offset, sizeof(g)); 

  //
  // Advance offset appropriately
  //
  offset+=sizeof(g);
      
  //
  // Byte swap
  //
  if(byteSwap)
  {
    _swapSpecialGraphicSymbolHdr( &g );
  }
   
  //
  // Print
  //
  if (debug) 
  {
    printSpecialGraphicSympbolHdr( g );
  }     
}

void GraphicProductMsg::readGraphicPageHdr(graphicPageHdr_t &g, 
					   unsigned char *decompressedBuf, 
					   unsigned long int &offset,
					   bool byteSwap,
					   bool debug)
{
  //
  // Read special graphic symbol header into a struct
  //
  memcpy(&g, decompressedBuf+offset, sizeof(g)); 

  //
  // Advance offset appropriately
  //
  offset+=sizeof(g);
      
  //
  // Byte swap
  //
  if(byteSwap)
  {
    _swapGraphicPageHdr( &g );
  }
   
  //
  // Print
  //
  if (debug) 
  {
    printGraphicPageHdr( g );
  }     
}

void  GraphicProductMsg::readTextPacket8(textPacket8_t &t, 
					 unsigned char *decompressedBuf, 
					 unsigned long int &offset,
					 bool byteSwap,
					 bool debug)
{
   //
   // Read data into storm id struct
   //
   memcpy(&t, decompressedBuf+offset, sizeof(t)); 

   //
   // Advance offset appropriately
   //
   offset+=sizeof(t);
   
   //
   // Byte swap
   //
   if(byteSwap)
   {
     _swapTextPacket8(&t);
   }
   
   //
   // Print
   //
   if (debug) 
   {
     printTextPacket8(t);
   }     
 }


void GraphicProductMsg::readStormID(stormID_t &s, 
				    unsigned char *decompressedBuf, 
				    unsigned long int &offset,
				    bool byteSwap,
				    bool debug)
 {
   //
   // Read data into storm id struct
   //
   memcpy(&s, decompressedBuf+offset, sizeof(s)); 

  //
  // Advance offset appropriately
  //
  offset+=sizeof(s);
      
  //
  // Byte swap
  //
  if(byteSwap)
  {
    _swapStormID( &s );
  }
   
  //
  // Print
  //
  if (debug) 
  {
    printStormID( s);
  }     
 }

void GraphicProductMsg::readHdaHail(hdaHail_t &h, 
				    unsigned char *decompressedBuf, 
				    unsigned long int &offset,
				    bool byteSwap,
				    bool debug)
 {
   //
   // Read data into storm id struct
   //
   memcpy(&h, decompressedBuf+offset, sizeof(h)); 

  //
  // Advance offset appropriately
  //
  offset+=sizeof(h);
      
  //
  // Byte swap
  //
  if(byteSwap)
  {
    _swapHdaHail( &h );
  }
   
  //
  // Print
  //
  if (debug) 
  {
    printHdaHail(h);
  }     
 }

void GraphicProductMsg::readMesocyclone(mesocyclone_t &m, 
					unsigned char *decompressedBuf, 
					unsigned long int &offset,
					bool byteSwap,
					bool debug)
{
  //
  // Read data into storm id struct
  //
  memcpy(&m, decompressedBuf+offset, sizeof(m)); 
  
  //
  // Advance offset appropriately
  //
  offset+=sizeof(m);
      
  //
  // Byte swap
  //
  if(byteSwap)
  {
    _swapMesocyclone( &m );
  }
   
  //
  // Print
  //
  if (debug) 
  {
    printMesocyclone(m);
  }     
 }

void  GraphicProductMsg::readTextPacket1(textPacket1_t &t, 
					 unsigned char *decompressedBuf, 
					 unsigned long int &offset,
					 bool byteSwap,
					 bool debug)
{
   //
   // Read data into storm id struct
   //
   memcpy(&t, decompressedBuf+offset, sizeof(t)); 

   //
   // Advance offset appropriately
   //
   offset+=sizeof(t);
   
   //
   // Byte swap
   //
   if(byteSwap)
   {
     _swapTextPacket1(&t);
   }
   
   //
   // Print
   //
   if (debug) 
   {
     printTextPacket1(t);
   }     
 }

void GraphicProductMsg::printMsgHdrBlk(msgHdrBlk_t m)
{
  cerr << "Message header block:" << endl;

  cerr << " Message code : " << m.msgCode << endl;
   time_t t = (m.msgDate-1) * 86400 + m.msgTime;

  cerr << " Message time : " << utimstr( t ) << endl;
  
  cerr << " Message length : " << m.msgLen << endl;
  
  cerr << " Message source : " << m.msgSrc << endl;
  
  cerr << " Message dest : " << m.msgDest << endl;
  
  cerr << " Number of blocks : " << m.nBlcks << endl;

  if (m.blckDiv == -1)
  {
    cerr << " Message block divider is valid";
  }
  else
  {
    cerr << " Message block divider is not valid";
  }

  cerr << endl << endl;

  return;
}

void GraphicProductMsg::printProdDescr(productDescription_t prodDescr)
{

  cerr << "Graphic product message block :" << endl;

  cerr << " Lat, lon, alt (feet) : " << double(prodDescr.lat/1000.0) << ", " 
       << double(prodDescr.lon/1000.0) << ", " << prodDescr.elevFeet 
       << endl;
  cerr << " Product code : " <<  prodDescr.prodCode << endl;
  cerr << " Operational mode (0 = maint, 1=clear air, 2=weather) : " 
       << prodDescr.opMode << endl;
  cerr << " VCP : " << prodDescr.vcp << endl;
  cerr << " Sequence number : " <<  prodDescr.seqNum << endl;
  cerr << " Volume number " <<  prodDescr.volScanNum << endl;

  time_t t = (prodDescr.volScanDate-1) * 86400 + prodDescr.volScanTime;
  cerr << " Volume time " << utimstr( t ) << endl;

  t = (prodDescr.genProdDate-1) * 86400 + prodDescr.genProdTime;
  cerr << " Product generation time " << utimstr( t ) << endl;

  cerr << " Elevation number : " << prodDescr.elNum << endl;
  cerr << " Version : " << (int)prodDescr.version << endl;
  cerr << " Spot blank (1=on, 0=off) : " <<  (int)prodDescr.spotBlank << endl;
  cerr << " Symb block offset, halfwords (0 = not present) : " 
       << prodDescr.symbOff << endl;
  cerr << " Graphic block offset, halfwords (0 = not present) : " 
       << prodDescr.grphOff << endl;
  cerr << " Tabular block offset, halfwords (0 = not present) : " 
       << prodDescr.tabOff << endl;
  cerr << endl;
  return;
}

void GraphicProductMsg::printPrdSymbBlk(prodSymbBlk_t p){

  cerr << "Product symbology block : " << endl;

  cerr << " Div (should be -1) : " << p.div << endl;

  cerr << " ID (should be 1) : " << p.id << endl;

  cerr << " Length (bytes) of block : " << p.len << endl;

  cerr << " Number of layers : " << p.nLayers << endl;

  cerr << " Layer div (should be -1) : " << p.layerDiv << endl;

  cerr << " Length of data layer (bytes) : " << p.dataLayerLen << endl << endl;

  return;
}

void GraphicProductMsg::printGraphicAlphaNumericHdr(graphicAlphaNumericBlk_t g)
{
  cerr << endl << "Graphic Alpha Numeric Block: " << endl;

  cerr << " Div (should be -1) : " << g.div << endl;

  cerr << " ID (should be 2) : " << g.id << endl;

  cerr << " Length (bytes) of block : " << g.len << endl;

  cerr << " Number of pages : " << g.nPages << endl;
}

void GraphicProductMsg::printRadial(radial_t r){

  cerr << "Radial header block : " << endl;

  cerr << " Data packet code: " << r.msgCode << endl;
  
  cerr << " First index : " << r.firstIndex << endl;
  
  cerr << " Number of bins : " << r.nBins << endl;
  
  cerr << " I,J center : " << r.iCent << ", " << r.jCent << endl;

  if (r.msgCode == 16)
  {
    double angle = acos(double(r.rangeScaleFactor)/1000.0);
    
    angle = angle * 180.0 / 3.1415927;
    
    cerr << " Range scale factor : " << double(r.rangeScaleFactor)/1000.0 
	 << " angle " << angle << endl;
    
    cerr << " Number of bytes: " << r.nBytes << endl;
  }

  if (r.msgCode == 44831)
  {
    cerr << " Range scale factor(number of pixels per range bin): " 
	 << double(r.rangeScaleFactor)/1000.0 << endl;
    
    cerr << " Number of RLE 16 bit HW per radial: " << r.nBytes << endl;
  }

  cerr << " Number of radials : " << r.nRadials << endl;
  
  cerr << " Azimuth : " << double(r.az)/10.0 << endl;
  
  cerr << " Delta azimuth : " << double(r.delAz)/10.0 << endl << endl;

  return;
}

void GraphicProductMsg::printSmallRadBlock(smallRadBlock_t b)
{
  cerr << endl << "Small radial header block found : " << endl;

  cerr << " Num: " << b.num << endl;
  
  cerr << " Azimuth: " << double(b.az)/10.0 << endl;
  
  cerr << " deltaAz: " << double(b.dAz)/10.0 << endl << endl;

  return;
}

void GraphicProductMsg::printGenericHdr(genericHeader_t h)
{
  cerr << endl << "Generic header: " << endl;

  cerr << " Packet code : " << h.code << endl;
  
  cerr << " numBytes( in serialized data): " <<   h.numBytes << endl;
  
  return;
}

void GraphicProductMsg::printSetColorLevelHdr( setColorLevelHdr_t cH)
{
  cerr << endl << "Color Level Header: " << endl;

  cerr << " Packet code hex 0x0802 (dec 2050) : " << cH.code << endl;
  
  cerr << "color flag (2 == true , else false): " <<   cH.colorIndicator<< endl;
  
  cerr << "color value " << cH.colorValue << endl;
  
  return;
}

void GraphicProductMsg::printLinkedVectorHdr(linkedVectorHeader_t v)
{
  cerr << endl << "Linked Contour Vector Header: " << endl;

  cerr << " Packet code 0x0E03 (dec 3587) : " << v.code << endl;
  
  return;
}


void GraphicProductMsg::printSpecialGraphicSympbolHdr(specialGraphicSymbolPacket_t g)
{
   cerr << endl << "Special Graphics Symbol Header: " << endl;

   cerr << " Packet code   : " << g.code << endl;
   
   cerr << " Number of bytes: " << g.numBytes << endl;
}

void GraphicProductMsg::printGraphicPageHdr(graphicPageHdr_t g)
{
  cerr << endl << "Graphics Page Header: " << endl;

  cerr << " page number: " << g.pageNum << endl;

  cerr << " page length: " << g.pageLen << endl;
}

void GraphicProductMsg::printStormID(stormID_t s)
{
  cerr << endl << "Storm ID: " << endl;

  cerr << " location: (" << s.I<< ", " << s.J << ")" << endl;

  cerr << " char1: " << s.char1 << " char2: " << s.char2 << endl;
}

void GraphicProductMsg::printHdaHail(hdaHail_t h)
{
  cerr << endl << "HDA Hail: " << endl;

  cerr << " location: (" << h.I<< ", " << h.J << ")" << endl;

  cerr << " probOfHail (-999 probability indicates cell exceeds processing range): ";

  if (h.probOfHail >=0)
  {
    cerr << h.probOfHail  << endl;
  }
  else
  {
    cerr << h.probOfHail << endl;
  }

  cerr << " probOfSevereHail (-999 probability indicates cell exceeds processing range): "; 
  
  if( h.probOfSevereHail >= 0)
  {
    cerr << h.probOfSevereHail  << endl;
  }
  else
  {
    cerr << h.probOfSevereHail << endl;
  }

  cerr << " maxHailSize(inches): " << h.maxHailSize << endl;
}

void GraphicProductMsg::printMesocyclone(mesocyclone_t m)
{
  cerr << endl << "Mesocyclone (location of 0,0 with radius 0 indicates no mesocyclone): " 
       << endl;
  
  cerr << " location: (" << m.I<< ", " << m.J << ")" << endl;
  cerr << " radius: " <<  (float) m.radius * .25 << endl;

  switch (  m.type )
  {
  case 1: 
  case 2:
    cerr << " type: " << m.type  << " =  mesocyclone (extrapolated)" << endl;
    break;

  case 3:
  case 4:
    cerr << " type: " << m.type  << " = mesocyclone(persistent, new, or "
	 << "increasing)" << endl;
    break;

  case 5:
    cerr << " type: " << m.type  << " = TVS (extrapolated)" << endl;
    break;
  
  case 6:
    cerr << " type: " << m.type  << " = ETVS (extrapolated)" << endl;
    break;	

  case 7:
    cerr << " type: " << m.type  << " = TVS (persistent, new, or "
	 << "increasing)" << endl;
    break;

  case 8:
    cerr << " type: " << m.type  << " = ETVS (persistent, new, or "
	 << "increasing)" << endl;
    break;

  case 9:
    cerr << " type: " << m.type  << " = MDA Circulation with " 
	 << "Strength Rank >= 5 and Base Height <= 1km ARL or Base "
	 << "on the lowest elevation angle" << endl;
    break;

  case 10:
    cerr << " type: " << m.type  << " = MDA Circulation with " 
	 << "Strength Rank >= 5 and Base Height > 1km ARL AND Base "
	 << "is on the lowest elevation angle" << endl;
    break;

  case 11:
   cerr << " type: " << m.type  << " = MDA Circulation with " 
	 << "Strength Rank < 5" << endl;
   break; 

  default:
    cerr << " type: " << m.type  << " Unrecognized type!" << endl;
    
  } 
}

void GraphicProductMsg::printTextPacket1(textPacket1_t t)
{
  cerr << endl << "Text Packet Code 1: " << endl;
  
  cerr << "code: " << t.code << endl;

  cerr << "numBytes: " << t.numBytes << endl;

  cerr << "starting screen I: " << t.I << " starting screen J: " << t.J << endl;
}

void GraphicProductMsg::printTextPacket8(textPacket8_t t)
{
  cerr << endl << "Text Packet Code 8: " << endl;
  
  cerr << "code: " << t.code << endl;

  cerr << "numBytes: " << t.numBytes << endl;

  cerr << "coordinate I: " << t.I << " coordinate J: " << t.J << endl;
}

void GraphicProductMsg::_swapMsgHdrBlk(msgHdrBlk_t *m)
{
  Swap::swap2( &m->msgCode);

  Swap::swap2( &m->msgDate);

  Swap::swap4( &m->msgTime);

  Swap::swap4( &m->msgLen);

  Swap::swap2( &m->msgSrc);

  Swap::swap2( &m->msgDest);

  Swap::swap2( &m->nBlcks);

  Swap::swap2( &m->blckDiv);

  return;
}

void GraphicProductMsg::_swapProdDescr(productDescription_t *prodDescr)
{
  Swap::swap4( &prodDescr->lat);
  
  Swap::swap4( &prodDescr->lon);
  
  Swap::swap2( &prodDescr->elevFeet);
  
  Swap::swap2( &prodDescr->prodCode);
  
  Swap::swap2( &prodDescr->opMode);
  
  Swap::swap2( &prodDescr->vcp);
  
  Swap::swap2( &prodDescr->seqNum);
  
  Swap::swap2( &prodDescr->volScanNum);
  
  Swap::swap2( &prodDescr->volScanDate);
  
  Swap::swap4( &prodDescr->volScanTime);
  
  Swap::swap2( &prodDescr->genProdDate);
  
  Swap::swap4( &prodDescr->genProdTime);
  
  Swap::swap2( &prodDescr->elNum);
  
  Swap::swap4( &prodDescr->symbOff);
  
  Swap::swap4( &prodDescr->grphOff);
 
  Swap::swap4( &prodDescr->tabOff);
  
  return;
}

void GraphicProductMsg::_swapProdSymbBlk(prodSymbBlk_t *p)
{
  Swap::swap2( &p->div);

  Swap::swap2( &p->id);

  Swap::swap4( &p->len);

  Swap::swap2( &p->nLayers);

  Swap::swap2( &p->layerDiv);

  Swap::swap4( &p->dataLayerLen);

  return;
}

void GraphicProductMsg::_swapGraphicAlphaNumericHdr(graphicAlphaNumericBlk_t *g)
{
  Swap::swap2(&g->div);
  
  Swap::swap2(&g->id);

  Swap::swap4(&g->len);

  Swap::swap2(&g->nPages);

}

void GraphicProductMsg::_swapRadial(radial_t *r)
{
  Swap::swap2( &r->msgCode);
  
  Swap::swap2( &r->firstIndex);
  
  Swap::swap2( &r->nBins);
  
  Swap::swap2( &r->iCent);
  
  Swap::swap2( &r->jCent);
  
  Swap::swap2( &r->rangeScaleFactor);
  
  Swap::swap2( &r->nRadials);
  
  Swap::swap2( &r->nBytes);
  
  Swap::swap2( &r->az);
  
  Swap::swap2( &r->delAz);
  
  return;
}

void GraphicProductMsg::_swapSmallRadBlock(smallRadBlock_t *b)
{
  Swap::swap2( &b->num);

  Swap::swap2( &b->az);

  Swap::swap2( &b->dAz);

  return;
}

void GraphicProductMsg::_swapGenericHdr(genericHeader_t *h)
{
  Swap::swap2( &h->code);
  
  Swap::swap2( &h->notUsed);
  
  Swap::swap4( &h->numBytes);
 
  return;
}

void GraphicProductMsg::_swapSetColorLevelHdr(setColorLevelHdr_t *cH)
{
   Swap::swap2( &cH->code);
   
   Swap::swap2( &cH->colorIndicator);

   Swap::swap2( &cH->colorValue);
   
  return;
}

void GraphicProductMsg::_swapLinkedVectorHdr(linkedVectorHeader_t *v)
{
   Swap::swap2( &v->code);

  return;
}

void GraphicProductMsg::_swapSpecialGraphicSymbolHdr(specialGraphicSymbolPacket_t *g)
{
  Swap::swap2(&g->code);  
  
  Swap::swap2(&g->numBytes);
}


void GraphicProductMsg:: _swapGraphicPageHdr(graphicPageHdr_t *g)
{
  Swap::swap2(&g->pageNum);

  Swap::swap2(&g->pageLen);
}

void GraphicProductMsg::_swapStormID(stormID_t *s)
{
  Swap::swap2(&s->I);

  Swap::swap2(&s->J);
}

void GraphicProductMsg::_swapHdaHail(hdaHail_t *h)
{
  Swap::swap2(&h->I);

  Swap::swap2(&h->J);

  Swap::swap2(&h->probOfHail);

  Swap::swap2(&h->probOfSevereHail);

  Swap::swap2(&h->maxHailSize);
}

void GraphicProductMsg::_swapMesocyclone(mesocyclone_t *m)
{
  Swap::swap2(&m->I);

  Swap::swap2(&m->J);

   Swap::swap2(&m->type);

  Swap::swap2(&m->radius);  
}

void GraphicProductMsg::_swapTextPacket1(textPacket1_t *t)
{
  Swap::swap2(&t->code);

  Swap::swap2(&t->numBytes);
  
  Swap::swap2(&t->I);

  Swap::swap2(&t->J);
}

void GraphicProductMsg::_swapTextPacket8(textPacket8_t *t)
{
  Swap::swap2(&t->code);

  Swap::swap2(&t->numBytes);

  Swap::swap2(&t->textColor);

  Swap::swap2(&t->I);

  Swap::swap2(&t->J);

}

bool GraphicProductMsg::getContourInitPt(unsigned char *decompressedBuf, 
					 unsigned long int &offset, 
					 bool byteSwap, bool debug, 
					 si16 &startI, si16 &startJ)
{
  si16 initPtIndicator;
  
  memcpy(&initPtIndicator, decompressedBuf+offset, sizeof(si16));

  offset+=sizeof(initPtIndicator);

  si16 initI;
  
  memcpy(&initI,  decompressedBuf+offset, sizeof(si16));

  offset+=sizeof(initI);

  si16 initJ;

  memcpy(&initJ,  decompressedBuf+offset, sizeof(si16)); 

  offset+=sizeof(initJ);
  
  if(byteSwap)
  {
    Swap::swap2(&initPtIndicator);
    
    Swap::swap2(&initI);

    Swap::swap2(&initJ);
  }

  if(initPtIndicator == (si16)0x8000)
  {
    startI = initI;

    startJ =  initJ;

    return true;
  }
  else
  {
    return false;
  }
}

si16 GraphicProductMsg::getContourNumVectors(unsigned char *decompressedBuf, 
					     unsigned long int &offset, 
					     bool byteSwap, bool debug)
{
  si16 num;
  
  memcpy(&num,  decompressedBuf+offset, sizeof(si16));

  offset+=sizeof(num);

  if(byteSwap)
  {
    Swap::swap2(&num);
  }

  //
  // According to ICD, Number stored is number of vectors X 4 (since each vector is 
  // defined by 4 numbers: startI, startJ, endI, endJ)
  //
  return num/4;
}

void  GraphicProductMsg::getContourVectorPt(unsigned char *decompressedBuf, 
					  unsigned long int &offset, 
					  bool byteSwap, bool debug, 
					  si16 &I, si16 &J)
{

  si16 i;
  memcpy(&i,  decompressedBuf+offset, sizeof(si16));

  offset+=sizeof(i);

  si16 j;

  memcpy(&j,  decompressedBuf+offset, sizeof(si16)); 

  offset+=sizeof(j);
  
  if(byteSwap)
  {
    Swap::swap2(&i);

    Swap::swap2(&j);
  }
  
  I = i;

  J = j;

}
