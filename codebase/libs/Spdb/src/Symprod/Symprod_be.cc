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
/////////////////////////////////////////////////////////////////////////
// Symprod_be.cc
//
// Byte-swapping for Symbolic product class
//
// Mike Dixon, from Nancy Rehak
// RAP, NCAR, Boulder, CO, 80307, USA
//
// Dec 1999
//
////////////////////////////////////////////////////////////////////////
//
// The toBE() routines convert from host to Big-endian byte order.
// The fromBE() routines convert from Big-endian to host byte order.
//
////////////////////////////////////////////////////////////////////////

#include <Spdb/Symprod.hh>
#include <dataport/bigend.h>
using namespace std;

///////////////////////////////////////////////////////////////////////
// prodHdrToBE()
//

void Symprod::prodHdrToBE(prod_hdr_props_t *prod_hdr)
  
{
  BE_from_array_32(prod_hdr, sizeof(prod_hdr_props_t) - SYMPROD_LABEL_LEN);
}

///////////////////////////////////////////////////////////////////////
// prodHdrFromBE()
//

void Symprod::prodHdrFromBE(prod_hdr_props_t *prod_hdr)
{
  BE_to_array_32(prod_hdr, sizeof(prod_hdr_props_t) - SYMPROD_LABEL_LEN);
}

///////////////////////////////////////////////////////////////////////
// objHdrToBE()
//

void Symprod::objHdrToBE(obj_hdr_t *obj_hdr)
{
  BE_from_array_32(&obj_hdr->object_type,
		   (char *) &obj_hdr->color - (char *) &obj_hdr->object_type);
  wptToBE(&obj_hdr->centroid);
}


///////////////////////////////////////////////////////////////////////
// objHdrFromBE()
//

void Symprod::objHdrFromBE(obj_hdr_t *obj_hdr)
{
  BE_to_array_32(&obj_hdr->object_type,
		 (char *) &obj_hdr->color - (char *) &obj_hdr->object_type);
  wptFromBE(&obj_hdr->centroid);
}


///////////////////////////////////////////////////////////////////////
// textToBE()
//

void Symprod::textToBE(text_props_t *props)
{
  BE_from_array_32(props, sizeof(text_props_t) - SYMPROD_FONT_NAME_LEN);
}

///////////////////////////////////////////////////////////////////////
// textFromBE()
//

void Symprod::textFromBE(text_props_t *props)
{
  BE_to_array_32(props, sizeof(text_props_t) - SYMPROD_FONT_NAME_LEN);
}

///////////////////////////////////////////////////////////////////////
// polylineToBE()
//

void Symprod::polylineToBE(polyline_props_t *props)
{
  BE_from_array_32(props, sizeof(polyline_props_t));
}

///////////////////////////////////////////////////////////////////////
// polylineFromBE()
//

void Symprod::polylineFromBE(polyline_props_t *props)

{
  BE_to_array_32(props, sizeof(polyline_props_t));
}

///////////////////////////////////////////////////////////////////////
// iconlineToBE()
//

void Symprod::iconlineToBE(iconline_props_t *props)
{
  BE_from_array_32(props, sizeof(iconline_props_t));
}

///////////////////////////////////////////////////////////////////////
// iconlineFromBE()
//

void Symprod::iconlineFromBE(iconline_props_t *props)

{
  BE_to_array_32(props, sizeof(iconline_props_t));
}

///////////////////////////////////////////////////////////////////////
// strokedIconToBE()
//

void Symprod::strokedIconToBE(stroked_icon_props_t *props)
{
  BE_from_array_32(props, sizeof(stroked_icon_props_t));
}

///////////////////////////////////////////////////////////////////////
// strokedIconFromBE()
//

void Symprod::strokedIconFromBE(stroked_icon_props_t *props)
{
  BE_to_array_32(props, sizeof(stroked_icon_props_t));
}

///////////////////////////////////////////////////////////////////////
// namedIconToBE()
//

void Symprod::namedIconToBE(named_icon_props_t *props)
{
  BE_from_array_32(&props->num_icons, sizeof(si32));
}

///////////////////////////////////////////////////////////////////////
// namedIconFromBE()
//

void Symprod::namedIconFromBE(named_icon_props_t *props)
{
  BE_to_array_32(&props->num_icons, sizeof(si32));
}

///////////////////////////////////////////////////////////////////////
// bitmapIconToBE()
//

void Symprod::bitmapIconToBE(bitmap_icon_props_t *props)
{
  BE_from_array_32(props, sizeof(bitmap_icon_props_t));
}

///////////////////////////////////////////////////////////////////////
// bitmapIconFromBE()
//

void Symprod::bitmapIconFromBE(bitmap_icon_props_t *props)
{
  BE_to_array_32(props, sizeof(bitmap_icon_props_t));
}

///////////////////////////////////////////////////////////////////////
// arcToBE()

void Symprod::arcToBE(arc_props_t *props)
{
  BE_from_array_32(props, sizeof(arc_props_t));
}

///////////////////////////////////////////////////////////////////////
// arcFromBE()
//

void Symprod::arcFromBE(arc_props_t *props)
{
  BE_to_array_32(props, sizeof(arc_props_t));
}

///////////////////////////////////////////////////////////////////////
// rectangleToBE()

void Symprod::rectangleToBE(rectangle_props_t *props)
{
  BE_from_array_32(props, sizeof(rectangle_props_t));
}

///////////////////////////////////////////////////////////////////////
// rectangleFromBE()
//

void Symprod::rectangleFromBE(rectangle_props_t *props)
{
  BE_to_array_32(props, sizeof(rectangle_props_t));
}

///////////////////////////////////////////////////////////////////////
// chunkToBE()

void Symprod::chunkToBE(chunk_props_t *props)
{
  BE_from_array_32(props, sizeof(chunk_props_t));
}

///////////////////////////////////////////////////////////////////////
// chunkFromBE()
//

void Symprod::chunkFromBE(chunk_props_t *props)
{
  BE_to_array_32(props, sizeof(chunk_props_t));
}

///////////////////////////////////////////////////////////////////////
// bboxToBE()
//

void Symprod::bboxToBE(bbox_t *bbox)
{
  BE_from_array_32(bbox, sizeof(bbox_t));
}

///////////////////////////////////////////////////////////////////////
// boxFromBE()

void Symprod::boxFromBE(bbox_t *bbox)
{
  BE_to_array_32(bbox, sizeof(bbox_t));
}

///////////////////////////////////////////////////////////////////////
// wptToBE()
//

void Symprod::wptToBE(wpt_t *wpt)
{
  BE_from_array_32(wpt, sizeof(wpt_t));
}

///////////////////////////////////////////////////////////////////////
// wptFromBE()
//

void Symprod::wptFromBE(wpt_t *wpt)
{
  BE_to_array_32(wpt, sizeof(wpt_t));
}

///////////////////////////////////////////////////////////////////////
// pptToBE()
//

void Symprod::pptToBE(ppt_t *ppt)
{
  BE_from_array_32(ppt, sizeof(ppt_t));
}

///////////////////////////////////////////////////////////////////////
// pptFromBE()
//

void Symprod::pptFromBE(ppt_t *ppt)
{
  BE_to_array_32(ppt, sizeof(ppt_t));
}

