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

////////////////////////////////////////////////////////////////
// RenderContext.hh
//
// Lightweight object holding context for product rendering.
//
// Mike Dixon, RAP, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Jan 2000
//
////////////////////////////////////////////////////////////////

#ifndef RenderContext_HH
#define RenderContext_HH

#include <rapplot/gplot.h>
#include <Mdv/MdvxProj.hh>
using namespace std;

class ProductMgr;
class Product;
class SymprodRender;
class SymprodRenderObj;
class SymprodRenderText;
class SymprodRenderPolyline;
class SymprodRenderStrokedIcon;
class SymprodRenderNamedIcon;
class SymprodRenderBitmapIcon;
class SymprodRenderArc;
class SymprodRenderRectangle;
class SymprodRenderChunk;

///////////////////////////////////////////////////////////////
// class definition

class RenderContext

{

friend class ProductMgr;
friend class Product;
friend class SymprodRender;
friend class SymprodRenderObj;
friend class SymprodRenderText;
friend class SymprodRenderPolyline;
friend class SymprodRenderIconline;
friend class SymprodRenderStrokedIcon;
friend class SymprodRenderNamedIcon;
friend class SymprodRenderBitmapIcon;
friend class SymprodRenderArc;
friend class SymprodRenderRectangle;
friend class SymprodRenderChunk;

public:

  // default constructor
  
  RenderContext();
  
  // destructor
  
  virtual ~RenderContext();

protected:

private:

  // data members

  Colormap cmap;
  x_color_list_index_t *colorIndex;
  MdvxProj proj;
  int dev;
  gframe_t frame;
  psgc_t psgc;
  double iconScale;

};

#endif


