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
//////////////////////////////////////////////////////////
// MdvVsectionField.cc
//
// Class for representing MDV sert sections.
// Used by MdvVsection.
//
// Mike Dixon, RAP, NCAR,
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// May 1999
//
//////////////////////////////////////////////////////////
//
// See <Mdv/mdv/MdvVsectionField.hh> for details.
//
///////////////////////////////////////////////////////////

#include <iostream>
#include <Mdv/mdv/MdvVsectionField.hh>
#include <toolsa/mem.h>
using namespace std;

////////////////////////////////////////////////////////////////////////
// Default constructor
//
// If you use this constructor you must call init() before using object
//

MdvVsectionField::MdvVsectionField()
  

{

  _initDone = false;

}

/////////////////////////////
// Primary constructor
//

MdvVsectionField::MdvVsectionField(const int field_num)
  

{

  init(field_num);

}

/////////////////////////////
// Copy constructor
//

MdvVsectionField::MdvVsectionField(const MdvVsectionField &other)

{
  *this = other;
}

/////////////////////////////
// Destructor

MdvVsectionField::~MdvVsectionField()

{
  freeData();
}

///////////////////////////////////////////////////////////////
// init - this must be called before the object is used.
// It is called automatically by the primary constructor.
//

void MdvVsectionField::init(const int field_num)

{

  _fieldNum = field_num;

  MEM_zero(_fieldHeader);
  MEM_zero(_vlevelHeader);
  
  _dataElemSize = 1;
  _data1D = NULL;
  _data2D = NULL;

  _initDone = true;

}

/////////////////////////////
// Assignment
//

void MdvVsectionField::operator=(const MdvVsectionField &other)
  

{

  init(other._fieldNum);

  _fieldHeader = other._fieldHeader;
  _vlevelHeader = other._vlevelHeader;

  if (other._data2D) {
    allocData(other._nLevels, other._nPts, other._dataElemSize);
    memcpy(_data1D, other._data1D, _nLevels * _nPts * _dataElemSize);
  }

  _initDone = other._initDone;

}

///////////////////////////////////////
// Alloc memory for vsection

void MdvVsectionField::allocData(int n_levels, int n_pts,
				 int elem_size, int encoding_type /*= MDV_INT8*/)

{

  if (_data2D) {
    ufree2((void **) _data2D);
  }
  _nLevels = n_levels;
  _nPts = n_pts;
  _dataElemSize = elem_size;
  _dataEncodingType = encoding_type;
  _data2D =
    (void **) umalloc2(_nLevels, _nPts, _dataElemSize);
  _data1D = _data2D[0];

}

///////////////////////////////////////
// Free up memory associated with grids

void MdvVsectionField::freeData()

{

  if (_data2D) {
    ufree2((void **) _data2D);
  }
  _data2D = NULL;
  _data1D = NULL;

}

/////////////////////////
// print summary

void MdvVsectionField::printSummary(ostream &out)

{

  out << ">> Vsection field <<" << endl;
  
  out << "  Field num: " << _fieldNum << endl;
  out << "  Field name: " << _fieldHeader.field_name << endl;

  out << "  Data elem size: " << _dataElemSize << endl;

  switch (_dataEncodingType) {
  case MDV_INT8:
    out << "  Encoding type: MDV_INT8" << endl;
    break;
  case MDV_INT16:
    out << "  Encoding type: MDV_INT16" << endl;
    break;
  case MDV_FLOAT32:
    out << "  Encoding type: MDV_FLOAT32" << endl;
    break;
  default:
    out << "  Encoding type: unknown" << endl;
    break;
  }

  out << "  N levels: " << _nLevels << endl;
  out << "  N points: " << _nPts << endl;

  out << "  Vlevels: " << endl;
  for (int i = 0; i < _nLevels; i++) {
    out << "    " << i << ": " << _vlevelHeader.vlevel_params[i] << endl;
  }
  
  out << endl;

}

/////////////////////////
// print data

void MdvVsectionField::printData(ostream &out)

{

  out << ">> Vsection field data <<" << endl;

  for (int i = 0; i < _nLevels; i++) {

    out << "  -->> Vlevel: " << _vlevelHeader.vlevel_params[i] << endl;
    out << "  -->> Data: ";
    
    switch (_dataEncodingType) {

    case MDV_INT8:
      {
	ui08 *ptr = (ui08 *) _data2D[i];
	for (int j = 0; j < _nPts; j++, ptr++) {
	  float val = (*ptr) * _fieldHeader.scale + _fieldHeader.bias;
	  out << val << " ";
	}
      }
      break;

    case MDV_INT16:
      {
	ui16 *ptr = (ui16 *) _data2D[i];
	for (int j = 0; j < _nPts; j++, ptr++) {
	  float val = (*ptr) * _fieldHeader.scale + _fieldHeader.bias;
	  out << val << " ";
	}
      }
      break;

    case MDV_FLOAT32:
      {
	fl32 *ptr = (fl32 *) _data2D[i];
	for (int j = 0; j < _nPts; j++, ptr++) {
	  float val = *ptr;
	  out << val << " ";
	}
      }
      break;

    default:
      out << "  Encoding type: unknown" << endl;
      break;
      
    }

    out << endl;

  } // i

}
