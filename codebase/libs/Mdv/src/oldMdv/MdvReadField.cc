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
// MdvReadField.cc
//
// Class for handling access to Mdv fields
//
// Mike Dixon, RAP, NCAR,
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// May 1999
//
//////////////////////////////////////////////////////////
//
// See <Mdv/mdv/MdvReadField.hh> for details.
//
///////////////////////////////////////////////////////////

#include <Mdv/mdv/MdvRead.hh>
#include <Mdv/mdv/mdv_read.h>
#include <toolsa/mem.h>
#include <toolsa/toolsa_macros.h>
#include <cassert>
using namespace std;

////////////////////////////////////////////////////////////////////////
// Default constructor
//
// If you use this constructor you must call init() before using object
//

MdvReadField::MdvReadField()
  

{

  _initDone = false;
  _headersRead = false;

}

/////////////////////////////
// Primary constructor
//

MdvReadField::MdvReadField(const MdvRead *mdv, const int field_num)
  

{

  init(mdv, field_num);

}

/////////////////////////////
// Copy constructor
//

MdvReadField::MdvReadField(const MdvReadField &other)

{
  *this = other;
}

/////////////////////////////
// Destructor

MdvReadField::~MdvReadField()

{

  _freeData();

}

///////////////////////////////////////////////////////////////
// init - this must be called before the object is used.
// It is called automatically by the primary constructor.
//

void MdvReadField::init(const MdvRead *mdv, const int field_num)

{

  _mdv = mdv;
  _headersRead = false;
  _fieldNum = field_num;

  MEM_zero(_fieldHeader);
  MEM_zero(_vlevelHeader);
  MEM_zero(_grid);

  _plane1D = NULL;
  _plane2D = NULL;
  _vol1D = NULL;
  _vol2D = NULL;
  _vol3D = NULL;
  _planeIsEncoded = false;
  _volIsEncoded = false;

  _initDone = true;

}

/////////////////////////////
// Assignment
//

void MdvReadField::operator=(const MdvReadField &other)
  

{

  init(other._mdv, other._fieldNum);

  _headersRead = other._headersRead;
  _fieldHeader = other._fieldHeader;
  _vlevelHeader = other._vlevelHeader;
  _grid = other._grid;

  // plane data

  _planeIsEncoded = other._planeIsEncoded;
  _planeReturnDataType = other._planeReturnDataType;
  _planeElemSize = other._planeElemSize;
  
  if (_planeIsEncoded) {

    _encodedPlaneBuf = other._encodedPlaneBuf;

    if (other._plane1D) {
      _plane1D = _encodedPlaneBuf.getBufPtr();
    } else {
      _plane1D = NULL;
    }
    _plane2D = NULL;

  } else {

    if (other._plane2D) {
      _plane2D =
	(void **) umalloc2(_fieldHeader.ny, _fieldHeader.nx, _planeElemSize);
      _plane1D = _plane2D[0];

      memcpy(_plane1D, other._plane1D,
	     _fieldHeader.ny * _fieldHeader.nx * _planeElemSize);
    } else {
      _plane2D = NULL;
      _plane1D = NULL;
    }

  }

  // vol data

  _volIsEncoded = other._volIsEncoded;
  _volReturnDataType = other._volReturnDataType;
  _volElemSize = other._volElemSize;
  _encodedPlaneSizes = other._encodedPlaneSizes;
  int nz = _fieldHeader.nz;

  if (_volIsEncoded) {

    _encodedVolBuf = other._encodedVolBuf;

    if (other._vol1D) {
      _vol1D = _encodedVolBuf.getBufPtr();
      _vol2D = (void **) umalloc (nz * sizeof(void *));
      int offset = 0;
      for (int iz = 0; iz < nz; iz++) {
	_vol2D[iz] = ((char *) _encodedVolBuf.getBufPtr()) + offset;
	offset += _encodedPlaneSizes[iz];
      }
    } else {
      _vol1D = NULL;
      _vol2D = NULL;
    }
    _vol3D = NULL;

  } else {

    if (other._vol3D) {
      int ny = _fieldHeader.ny;
      int nx = _fieldHeader.nx;
      _vol3D = (void ***) umalloc3(nz, ny, nx, _volElemSize);
      _vol2D = (void **) umalloc(_fieldHeader.nz * sizeof(void *));
      for (int iz = 0; iz < nz; iz++) {
	_vol2D[iz] = _vol3D[iz][0];
      }
      _vol1D = _vol3D[0][0];
      memcpy(_vol1D, other._vol1D, nz * ny * nx * _volElemSize);
    } else {
      _vol3D = NULL;
      _vol2D = NULL;
      _vol1D = NULL;
    }

  }

}

///////////////////////////////////////
// Free up memory associated with grids

void MdvReadField::_freeData()

{

  _freePlane();
  _freeVol();

}

/////////////////////////////////////////
// Free up memory associated with volume

void MdvReadField::_freeVol()

{

  if (_volIsEncoded) {
    
    if (_vol2D) {
      ufree(_vol2D);
    }
    _encodedVolBuf.free();
    _vol1D = NULL;
 
    _encodedPlaneSizes.erase(_encodedPlaneSizes.begin(),
			     _encodedPlaneSizes.end());
    
  } else {

    if (_vol3D) {
      ufree3((void ***) _vol3D);
      ufree(_vol2D);
      _vol3D = NULL;
      _vol2D = NULL;
      _vol1D = NULL;
    }

  }

}

///////////////////////////////////////
// Free up memory associated with plane

void MdvReadField::_freePlane()

{

  if (_planeIsEncoded) {

    _encodedPlaneBuf.free();
    _plane1D = NULL;

  } else {

    if (_plane2D) {
      ufree2((void **) _plane2D);
      _plane2D = NULL;
      _plane1D = NULL;
    }
    
  }

}

///////////////////////////////////////////////////
// read field header and vlevel header if included
//
// Returns 0 on success,  -1 on failure

int MdvReadField::_readHeaders()
  
{

  assert (_initDone);

  // only read once

  if (_headersRead) {
    return (0);
  }

  if (!_mdv->_fp) {
    cerr << "ERROR - MdvReadField::readHeaders" << endl;
    cerr << "  File not open" << endl;
    return (-1);
  }

  /*
   * Read the field header.
   */
  
  if (MDV_load_field_header(_mdv->_fp,
			    &_fieldHeader, _fieldNum) != MDV_SUCCESS) {
    cerr << "ERROR - MdvReadField::readHeaders" << endl;
    cerr << "  Cannot load field header, field_num: " << _fieldNum << endl;
    cerr << "  File path '" << _mdv->_filePath << "'" << endl;
    return (-1);
  }

  if (_mdv->_masterHeader.vlevel_included) {
    if (MDV_load_vlevel_header(_mdv->_fp, &_vlevelHeader,
			       (MDV_master_header_t *) &_mdv->_masterHeader,
			       _fieldNum) != MDV_SUCCESS) {
      cerr << "ERROR - MdvReadField::readHeaders" << endl;
      cerr << "  Cannot load vlevel header, field_num: " << _fieldNum << endl;
      cerr << "  File path '" << _mdv->_filePath << "'" << endl;
      return (-1);
    }
  }

  MDV_load_grid_from_hdrs((MDV_master_header_t *) &_mdv->_masterHeader,
			  &_fieldHeader, &_grid);

  _headersRead = true;
  
  return (0);

}

///////////////////////////////////////////////////
// read plane of data given the plane_num
//
// If fhdr is non-NULL, it is filled out with the values applicable
// after the read and any relevant conversions.
//
// Returns 0 on success, -1 on failure

int MdvReadField::_readPlane(const int plane_num,
			     const int return_data_type,
			     MDV_field_header_t *fhdr /* = NULL*/ )
  
{

  if (_readHeaders()) {
    return (-1);
  }

  // free previous allocation

  _freePlane();
  
  // read the plane - tmp plane is allocated by MDV_get_plane_size
  // and must be copied to permanent pos and freed

  int plane_size;
  MDV_field_header_t loc_fhdr = _fieldHeader;
  int output_encoding = return_data_type;
  int output_compression = MDV_COMPRESSION_NONE;
  if (return_data_type == MDV_PLANE_RLE8) {
    output_encoding = MDV_INT8;
    output_compression = MDV_COMPRESSION_RLE;
  }
  
  void *plane = MDV_read_field_plane(_mdv->_fp, &loc_fhdr,
				     output_encoding, output_compression,
				     MDV_SCALING_ROUNDED, 0.0, 0.0, 
				     plane_num, &plane_size);

  if (fhdr != NULL) {
    *fhdr = loc_fhdr;
  }

  if (plane == NULL) {
    cerr << "ERROR - MdvReadField::readPlane" << endl;
    cerr << "  Cannot read plane_num " << plane_num
	 << ", field_num " << _fieldNum << endl;
    cerr << "  File path '" << _mdv->_filePath << "'" << endl;
    return (-1);
  }

  _planeReturnDataType = return_data_type;
  
  if (_planeReturnDataType == MDV_PLANE_RLE8) {

    _planeIsEncoded = true;
    
    // for encoded data, store in buffer and set pointer to it

    _encodedPlaneBuf.add(plane, plane_size);
    _plane1D = _encodedPlaneBuf.getBufPtr();
    ufree(plane);
    
  } else {
    
    _planeIsEncoded = false;

    if (_planeReturnDataType == MDV_FLOAT32) {
      _planeElemSize = 4;
    } else if (_planeReturnDataType == MDV_INT16) {
      _planeElemSize = 2;
    } else if (_planeReturnDataType == MDV_INT8) {
      _planeElemSize = 1;
    } else {
      cerr << "ERROR - MdvReadField::readPlane" << endl;
      cerr << "  Bad return type code: " << _planeReturnDataType << endl;
      ufree(plane);
      return (-1);
    }

    int expected_size =
      _fieldHeader.nx * _fieldHeader.ny * _planeElemSize;
    
    if (expected_size != plane_size) {
      cerr << "ERROR - MdvReadField::readPlane" << endl;
      cerr << "  Incorrect plane size, plane_num " << plane_num
	   << ", field_num " << _fieldNum << endl;
      cerr << "  Expected size: " << expected_size << endl;
      cerr << "  Size in file: " << plane_size << endl;
      cerr << "  File path '" << _mdv->_filePath << "'" << endl;
      ufree(plane);
      return (-1);
    }
    
    _plane2D =
      (void **) umalloc2(_fieldHeader.ny, _fieldHeader.nx, _planeElemSize);
    _plane1D = _plane2D[0];
    memcpy(_plane1D, plane, plane_size);
    ufree(plane);
    
  }

  _planeNum = plane_num;
  if (_mdv->_masterHeader.vlevel_included) {
    _planeVlevel = _vlevelHeader.vlevel_params[_planeNum];
  } else {
    _planeVlevel = _fieldHeader.grid_minz + _planeNum * _fieldHeader.grid_dz;
  }

  return (0);
  
}

//////////////////////////////////////////////////////////
// Read plane of data given the desired vlevel.
// Actual plane_num used may be retrieved by getPlaneNum()
// Actual plane vlevel used may be retrieved by getPlaneVlevel()
//
// If fhdr is non-NULL, it is filled out with the values applicable
// after the read and any relevant conversions.
//
// Returns 0 on success, -1 on failure

int MdvReadField::_readPlane(const double vlevel,
			     const int return_data_type,
			     MDV_field_header_t *fhdr /* = NULL*/ )
  
{

  assert (_initDone);

  int planeNum;

  if (_mdv->_masterHeader.vlevel_included) {

    planeNum = 0;
    double actual = _vlevelHeader.vlevel_params[0];
    double minError = fabs(vlevel - actual);

    for (int iz = 1; iz < _fieldHeader.nz; iz++) {
      actual = _vlevelHeader.vlevel_params[iz];
      double error = fabs(vlevel - actual);
      if (error < minError) {
	planeNum = iz;
	minError = error;
      }
      if (actual > vlevel) {
	break;
      }
    } // iz
    
  } else {

    planeNum = (int) ((vlevel - _fieldHeader.grid_minz) /
		      _fieldHeader.grid_dz + 0.5);

  }

  if (planeNum < 0) {
    planeNum = 0;
  } else if (planeNum > _fieldHeader.nz - 1) {
    planeNum = _fieldHeader.nz - 1;
  }

  return(_readPlane(planeNum, return_data_type, fhdr));

}

//////////////////////////////////////////////////////
// read composite - this is the max value at any point.
//
// Encoded return types are not supported.
//
// If fhdr is non-NULL, it is filled out with the values applicable
// after the read and any relevant conversions.
//
// Returns 0 on success, -1 on failure

int MdvReadField::_readComposite(const int return_data_type /* = MDV_INT8*/,
				 MDV_field_header_t *fhdr /* = NULL*/ )
  
{

  assert (_initDone);

  // check return type is not encoded

  if (return_data_type == MDV_PLANE_RLE8) {
    cerr << "ERROR - MdvReadField::readComposite" << endl;
    cerr << "  Encoded types are not supported." << endl;
    cerr << "  File path '" << _mdv->_filePath << "'" << endl;
    return (-1);
  }

  // read in the volume

  if (_readVol(return_data_type, fhdr)) {
    cerr << "ERROR - MdvReadField::readComposite" << endl;
    cerr << "  Cannot read volume." << endl;
    cerr << "  Field num: " << _fieldNum;
    cerr << "  File path '" << _mdv->_filePath << "'" << endl;
    return (-1);
  }

  // set byte size
  
  _planeIsEncoded = false;
  _planeReturnDataType = return_data_type;
  _planeElemSize = _volElemSize;

  // alloc the plane

  int nz = _fieldHeader.nz;
  int ny = _fieldHeader.ny;
  int nx = _fieldHeader.nx;

  _freePlane();
  _plane2D = (void **) umalloc2(ny, nx, _planeElemSize);
  _plane1D = _plane2D[0];

  // initialize with the first plane from the vol

  memcpy(_plane1D, _vol2D[0], ny * nx * _planeElemSize);

  // loop through the planes, computing the max, loading
  // up the composite plane

  for (int iz = 1; iz < nz; iz++) {

    if (_volReturnDataType == MDV_FLOAT32) {
      
      fl32 *cp = (fl32 *) _plane1D;
      fl32 *vp = (fl32 *) _vol2D[iz];
      for (int i = 0; i < ny * nx; i++, cp++, vp++) {
	*cp = MAX(*cp, *vp);
      }

    } else if (_volReturnDataType == MDV_INT16) {

      ui16 *cp = (ui16 *) _plane1D;
      ui16 *vp = (ui16 *) _vol2D[iz];
      for (int i = 0; i < ny * nx; i++, cp++, vp++) {
	*cp = MAX(*cp, *vp);
      }

    } else if (_volReturnDataType == MDV_INT8) {

      ui08 *cp = (ui08 *) _plane1D;
      ui08 *vp = (ui08 *) _vol2D[iz];
      for (int i = 0; i < ny * nx; i++, cp++, vp++) {
	*cp = MAX(*cp, *vp);
      }
      
    }

  } // iz

  // free up the volume

  _freeVol();

  return (0);
  
}


///////////////////////////////////////////////////
// read data volume
//
// If fhdr is non-NULL, it is filled out with the values applicable
// after the read and any relevant conversions.
//
// Returns 0 on success, -1 on failure

int MdvReadField::_readVol(const int return_data_type,
			   MDV_field_header_t *fhdr /* = NULL*/ )
  
{
  
  if (return_data_type == MDV_PLANE_RLE8) {
    cerr << "ERROR - MdvReadField::_readVol" << endl;
    cerr << "  Encoded types are not supported." << endl;
    cerr << "  File path '" << _mdv->_filePath << "'" << endl;
    return (-1);
  }

  if (_readHeaders()) {
    return (-1);
  }

  int nz = _fieldHeader.nz;

  // free previous mem

  _freeVol();
  
  _volReturnDataType = return_data_type;

  _volIsEncoded = false;
    
  if (_volReturnDataType == MDV_FLOAT32) {
    _volElemSize = 4;
  } else if (_volReturnDataType == MDV_INT16) {
    _volElemSize = 2;
  } else if (_volReturnDataType == MDV_INT8) {
    _volElemSize = 1;
  } else {
    cerr << "ERROR - MdvReadField::readVol" << endl;
    cerr << "  Bad return type code: " << _volReturnDataType << endl;
    return (-1);
  }
  
  // allocate 3D array
  
  _vol3D = (void ***) umalloc3(_fieldHeader.nz, _fieldHeader.ny,
			       _fieldHeader.nx, _volElemSize);
  
  // allocate and set 2D array
  
  _vol2D = (void **) umalloc(_fieldHeader.nz * sizeof(void *));
  
  for (int iz = 0; iz < nz; iz++) {
    _vol2D[iz] = _vol3D[iz][0];
  }
  
  // set 1D array
  
  _vol1D = _vol3D[0][0];
  
  int expected_vol_size =
    _fieldHeader.nx * _fieldHeader.ny * _fieldHeader.nz * _volElemSize;

  // read in the volume
  // vol data is temporary and must be copied to permanent pos and freed

  int vol_size;
  MDV_field_header_t loc_fhdr = _fieldHeader;
  
  void *vol = MDV_read_field_volume(_mdv->_fp, &loc_fhdr,
				    return_data_type, MDV_COMPRESSION_NONE,
				    MDV_SCALING_ROUNDED, 0.0, 0.0, 
				    &vol_size);

  if (vol == NULL) {
    cerr << "ERROR - MdvReadField::readVol" << endl;
    cerr << "  Cannot read volume, field_num " << _fieldNum << endl;
    cerr << "  File path '" << _mdv->_filePath << "'" << endl;
    return -1;
  }

  if (vol_size != expected_vol_size) {
    cerr << "ERROR - MdvReadField::readVol" << endl;
    cerr << "  Incorrect vol size read: " << vol_size << endl;
    cerr << "  Expected vol size : " << expected_vol_size << endl;
    cerr << "  Field_num " << _fieldNum << endl;
    cerr << "  File path '" << _mdv->_filePath << "'" << endl;
    ufree(vol);
    return -1;
  }

  // copy the field header if appropriate

  if (fhdr != NULL) {
    *fhdr = loc_fhdr;
  }

  // copy vol data to object
  
  memcpy(**_vol3D, vol, vol_size);
  ufree(vol);

  return (0);

}


