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
// ** DISCLAIMER: THIS SOFTWARE IS PROVIDED ::AS IS" AND WITHOUT ANY EXPRESS  
// ** OR IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED      
// ** WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.    
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
//////////////////////////////////////////////////////////
// MdvxField.cc
//
// Class for handling Mdvx fields
//
// Mike Dixon, RAP, NCAR,
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// August 1999
//
//////////////////////////////////////////////////////////
//
// See <Mdv/MdvxField.hh> for details.
//
///////////////////////////////////////////////////////////

#include <Mdv/MdvxField.hh>
#include <Mdv/MdvxProj.hh>
#include <Mdv/MdvxStdAtmos.hh>
#include <dataport/bigend.h>
#include <toolsa/toolsa_macros.h>
#include <toolsa/compress.h>
#include <toolsa/umisc.h>
#include <toolsa/str.h>
#include <toolsa/pjg.h>
#include <euclid/PjgMath.hh>
#include <math.h>
#include <iostream>
#include <iomanip>
#include <limits>
#include <cerrno>
using namespace std;

#define PSEUDO_RADIUS 8533.0
// #define DEBUG_PRINT

////////////////////////////////////////////////////////////////////////
// Basic constructor - only a master header reference
//

MdvxField::MdvxField()

{
  
  MEM_zero(_fhdr);
  MEM_zero(_vhdr);
  _fhdrFile = NULL;
  _vhdrFile = NULL;
  
}

/////////////////////////////
// Copy constructor
//

MdvxField::MdvxField(const MdvxField &rhs)
     
{
  MEM_zero(_fhdr);
  MEM_zero(_vhdr);
  _fhdrFile = NULL;
  _vhdrFile = NULL;
  _copy(rhs);
}

////////////////////////////////////////////////////////////////////
//
// Constucts object from field header and data parts.
//
// If the data pointer is not NULL, space is allocated for it and the
// data is copied in.
//
// If the data pointer is NULL, space is allocated for data based on
// the volume_size in the field header. In this case,
// if init_with_missing is true, the array will be initialized with
// the missing_data_value in the header.
//
// NOTE: for this to work, the following items in the 
//       header must be set:
//
//    nx, ny, nz, volume_size, data_element_nbytes,
//    bad_data_value, missing_data_value
//
//  In addition, for INT8 and INT16, set:
//    scale, bias
//
// All constructors leave internal fields uncompressed,
// and set write-pending compression appropriately.

MdvxField::MdvxField(const Mdvx::field_header_t &f_hdr,
		     const Mdvx::vlevel_header_t &v_hdr,
		     const void *vol_data /* = NULL*/,
		     bool init_with_missing /* = false*/,
		     bool compute_min_and_max /* = true*/,
                     bool alloc_mem /* = true*/ )
  
{

  setHdrsAndVolData(f_hdr, v_hdr, vol_data,
                    init_with_missing,
                    compute_min_and_max,
                    alloc_mem);

}

/////////////////////////////////////////////////////////////////////
// Constructor for a handle for a single plane, given a handle which
// has the entire data volume.
// The data for the plane is copied from the volume.
//
// All constructors leave internal fields uncompressed,
// and set write-pending compression appropriately.

MdvxField::MdvxField(const MdvxField &rhs, int plane_num)
  
{

  // copy headers

  _fhdr = rhs._fhdr;
  _vhdr = rhs._vhdr;
  if (rhs._fhdrFile != NULL) {
    _fhdrFile = new Mdvx::field_header_t;
    *_fhdrFile = *rhs._fhdrFile;
  } else {
    _fhdrFile = NULL;
  }
  if (rhs._vhdrFile) {
    _vhdrFile = new Mdvx::vlevel_header_t;
    *_vhdrFile= *rhs._vhdrFile;
  } else {
    _vhdrFile = NULL;
  }

  // copy the data

  _volBuf = rhs._volBuf;

  // uncompress as appropriate
  
  int compression_type = _fhdr.compression_type;
  size_t nBytesIn = 
    _fhdr.nx * _fhdr.ny * _fhdr.nz  * _fhdr.data_element_nbytes;
  if (decompress()) {
    // data decompression error
    if (nBytesIn == _volBuf.getLen()) {
      cerr << "WARNING - MdvxField single-plane copy constructor" << endl;
      cerr << "  Decompression error" << endl;
      cerr << "  Field name: " << _fhdr.field_name << endl;
      cerr << "  Size matched uncompressed size" << endl;
      cerr << "  Assuming not compressed" << endl;
      _fhdr.compression_type = Mdvx::COMPRESSION_NONE;
    } else {
      cerr << "WARNING - MdvxField single-plane copy constructor" << endl;
      cerr << "  Decompression error" << endl;
      cerr << "  Field name: " << _fhdr.field_name << endl;
      _fhdr.compression_type = Mdvx::COMPRESSION_NONE;
      _fhdr.compression_type = Mdvx::COMPRESSION_NONE;
      _fhdr.transform_type = Mdvx::DATA_TRANSFORM_NONE;
      _fhdr.scaling_type = Mdvx::SCALING_NONE;
      _fhdr.volume_size = 0;
      _fhdr.data_element_nbytes = 0;
      _fhdr.nx = 0;
      _fhdr.ny = 0;
      _fhdr.nz = 0;
      _volBuf.clear();
    }
    return;
  }

  if (_fhdr.proj_type == Mdvx::PROJ_VSECTION) {
    
    // vert section data

    if (_fhdr.ny > 1) {
      // copy the plane
      MemBuf tmpBuf;
      int64_t nbytesPlane = _fhdr.nx * _fhdr.nz * _fhdr.data_element_nbytes;
      int64_t planeOffset =  plane_num * nbytesPlane;
      void *planePtr = (ui08 *) _volBuf.getPtr() + planeOffset;
      tmpBuf.add(planePtr, nbytesPlane);
      _volBuf = tmpBuf;
      _fhdr.ny = 1;
    }

  } else {

    // horiz data
    
    if (_fhdr.nz > 1) {
      // copy the plane
      MemBuf tmpBuf;
      int64_t nbytesPlane = _fhdr.nx * _fhdr.ny * _fhdr.data_element_nbytes;
      int64_t planeOffset =  plane_num * nbytesPlane;
      void *planePtr = (ui08 *) _volBuf.getPtr() + planeOffset;
      tmpBuf.add(planePtr, nbytesPlane);
      _volBuf = tmpBuf;
      _fhdr.nz = 1;
    }
    
  }

  // set deferred compression

  if (compression_type == Mdvx::COMPRESSION_NONE) {
    requestCompression(Mdvx::COMPRESSION_NONE);
  } else {
    // we only use GZIP for compression now
    requestCompression(Mdvx::COMPRESSION_GZIP);
  }
  
}

////////////////////////////////////////////////////////////////////
//
// Constucts object for single plane from field header and data parts.
//
// If the plane data pointer is not NULL, space is allocated for it
// and the data is copied in. Data should not be compressed.
//
// If the plane data pointer is NULL, space is allocated for data based
// on the volume_size in the field header.
//
// NOTE: for this to work, the following items in the 
//       header must be set:
//
//    nx, ny, nz, volume_size, data_element_nbytes,
//    bad_data_value, missing_data_value
//
//  In addition, for INT8 and INT16, set:
//    scale, bias

MdvxField::MdvxField(int plane_num,
		     int64_t plane_size,
		     const Mdvx::field_header_t &f_hdr,
		     const Mdvx::vlevel_header_t &v_hdr,
		     const void *plane_data /* = NULL*/ )
  
{

  setHdrsAndPlaneData(plane_num, plane_size,
                      f_hdr, v_hdr, plane_data);

}

/////////////////////////////
// Destructor

MdvxField::~MdvxField()

{

  _planeData.erase(_planeData.begin(), _planeData.end());
  _planeSizes.erase(_planeSizes.begin(), _planeSizes.end());
  _planeOffsets.erase(_planeOffsets.begin(), _planeOffsets.end());
  _volBuf.free();
  
  if (_fhdrFile != NULL) {
    delete _fhdrFile;
  }
  if (_vhdrFile != NULL) {
    delete _vhdrFile;
  }

}

/////////////////////////////
// Assignment
//

MdvxField &MdvxField::operator=(const MdvxField &rhs)
  

{
  return _copy(rhs);
}

//////////////////////////////////////////////////
// _copy - used by copy constructor and operator =
//

MdvxField &MdvxField::_copy(const MdvxField &rhs)

{

  if (&rhs == this) {
    return *this;
  }

  _fhdr = rhs._fhdr;
  _vhdr = rhs._vhdr;

  if (_fhdrFile) {
    delete _fhdrFile;
    _fhdrFile = NULL;
  }

  if (_vhdrFile) {
    delete _vhdrFile;
    _vhdrFile = NULL;
  }

  if (rhs._fhdrFile != NULL) {
    _fhdrFile = new Mdvx::field_header_t;
    *_fhdrFile = *rhs._fhdrFile;
  } else {
    _fhdrFile = NULL;
  }
  if (rhs._vhdrFile) {
    _vhdrFile = new Mdvx::vlevel_header_t;
    *_vhdrFile= *rhs._vhdrFile;
  } else {
    _vhdrFile = NULL;
  }

  _volBuf = rhs._volBuf;

  if (rhs._planeSizes.size() > 0) {
    setPlanePtrs();
  }

  return *this;
  
}

//////////////////////////////////////////////////////////////////////
// setting the volume data
//
// The data must be uncompressed.
// Scaling type, scale and bias only apply to INT8 and INT16 encoding.

void MdvxField::setVolData(const void *vol_data,
			   int64_t volume_size,
			   Mdvx::encoding_type_t encoding_type,
			   Mdvx::scaling_type_t scaling_type
                           /* = Mdvx::SCALING_ROUNDED */,
			   double scale /* = 1.0*/,
			   double bias /* = 0.0*/ )

{

  _fhdr.data_element_nbytes = Mdvx::dataElementSize(encoding_type);
  ssize_t expectedVolSize =
    _fhdr.nz * _fhdr.ny * _fhdr.nx * _fhdr.data_element_nbytes;

  if (volume_size != expectedVolSize) {
    cerr << "WARNING - MdvxField::setVolData" << endl;
    cerr << "  Volume size is: " << volume_size << endl;
    cerr << "  Should be: " << expectedVolSize << endl;
    cerr << "  Field name: " << _fhdr.field_name << endl;
    cerr << "  Volume size: " << _fhdr.volume_size << endl;
    cerr << "  element nbytes: " << _fhdr.data_element_nbytes << endl;
  }

  _volBuf.free();
  _volBuf.add(vol_data, volume_size);
  _fhdr.volume_size = volume_size;
  _fhdr.encoding_type = encoding_type;
  _fhdr.scaling_type = scaling_type;
  _fhdr.scale = scale;
  _fhdr.bias = bias;

  // Reset the min and max so they get recomputed.
  _fhdr.min_value = 0.0;
  _fhdr.max_value = 0.0;

  // Check for NaN, infinity values.
  _check_finite(_volBuf.getPtr());
  
}

void MdvxField::setVolData(const void *vol_data,
			   int nz, int ny, int nx,
			   Mdvx::encoding_type_t encoding_type,
			   Mdvx::scaling_type_t scaling_type
                           /* = Mdvx::SCALING_ROUNDED */,
			   double scale /* = 1.0*/,
			   double bias /* = 0.0*/ )

{

  _fhdr.data_element_nbytes = Mdvx::dataElementSize(encoding_type);
  _fhdr.nz = nz;
  _fhdr.ny = ny;
  _fhdr.nx = nx;

  int64_t volume_size = nz * ny * nx * _fhdr.data_element_nbytes;
  setVolData(vol_data, volume_size,
             encoding_type, scaling_type,
             scale, bias);

  if (volume_size == 0) {
    cerr << "  nz: " << nz << endl;
    cerr << "  ny: " << ny << endl;
    cerr << "  nx: " << nx << endl;
  }
  
}


////////////////////////////////////////////////////////////////////
//
// Set object state from field header and data parts.
//
// If the data pointer is not NULL, space is allocated for it and the
// data is copied in.
//
// If the data pointer is NULL, space is allocated for data based on
// the volume_size in the field header. In this case,
// if init_with_missing is true, the array will be initialized with
// the missing_data_value in the header.
//
// NOTE: for this to work, the following items in the 
//       header must be set:
//
//    nx, ny, nz, volume_size, data_element_nbytes,
//    bad_data_value, missing_data_value
//
//  In addition, for INT8 and INT16, set:
//    scale, bias
//
// All constructors leave internal fields uncompressed,
// and set write-pending compression appropriately.

void MdvxField::setHdrsAndVolData(const Mdvx::field_header_t &f_hdr,
                                  const Mdvx::vlevel_header_t &v_hdr,
                                  const void *vol_data /* = NULL*/,
                                  bool init_with_missing /* = false*/,
                                  bool compute_min_and_max /* = true*/,
                                  bool alloc_mem /* = true*/ )
  
{
  
  _fhdr = f_hdr;
  _vhdr = v_hdr;
  _fhdrFile = NULL;
  _vhdrFile = NULL;
  _volBuf.clear();
  
  // check the element size and volume_size

  if (_fhdr.data_element_nbytes != 1 &&
      _fhdr.data_element_nbytes != 2 &&
      _fhdr.data_element_nbytes != 4) {
    cerr << "ERROR - MdvxField::MdvxField" << endl;
    cerr << "  You must set data_element_nbytes to 1, 2 or 4" << endl;
  }

  if (_fhdr.compression_type == Mdvx::COMPRESSION_NONE) {
    int64_t expected_volume_size =
      _fhdr.nx * _fhdr.ny * _fhdr.nz * _fhdr.data_element_nbytes;
    if (_fhdr.volume_size != expected_volume_size) {
      cerr << "WARNING - MdvxField::MdvxField" << endl;
      cerr << "  Field name: " << _fhdr.field_name << endl;
      cerr << "  Volume size incorrect: " << _fhdr.volume_size << endl;
      cerr << "  Should be: " << expected_volume_size << endl;
      cerr << "  _fhdr.nx: " << _fhdr.nx << endl;
      cerr << "  _fhdr.ny: " << _fhdr.ny << endl;
      cerr << "  _fhdr.nz: " << _fhdr.nz << endl;
      cerr << "  _fhdr.data_element_nbytes: "
	   << _fhdr.data_element_nbytes << endl;
    }
    if (_fhdr.volume_size == 0) {
      cerr << "WARNING - MdvxField::MdvxField" << endl;
      cerr << "  Field name: " << _fhdr.field_name << endl;
      cerr << "  Volume size: " << _fhdr.volume_size << endl;
    }
  }

  // check nz does not exceed max allowable

  if (_fhdr.nz > MDV_MAX_VLEVELS) {

    cerr << "WARNING - MdvxField::MdvxField" << endl;
    cerr << "  Field name: " << _fhdr.field_name << endl;
    cerr << "    nz: " << _fhdr.nz << endl;
    cerr << "  This exceeds max allowable value: " << MDV_MAX_VLEVELS << endl;
    cerr << "  Will be adjusted to: " << MDV_MAX_VLEVELS << endl;
    _fhdr.nz = MDV_MAX_VLEVELS;
    _fhdr.volume_size =
      _fhdr.nx * _fhdr.ny * _fhdr.nz * _fhdr.data_element_nbytes;

  }

  // add vol data

  if (vol_data != NULL) {

    // add the data

    _volBuf.add(vol_data, _fhdr.volume_size);

  } else if (alloc_mem) {

    // prepare buffer for use
    
    _volBuf.prepare(_fhdr.volume_size);
    
    if (init_with_missing) {
      switch (_fhdr.encoding_type) {
        case Mdvx::ENCODING_INT8: {
          ui08 missing = (ui08) _fhdr.missing_data_value;
          memset(_volBuf.getPtr(), missing, _fhdr.volume_size);
          _fhdr.data_element_nbytes = 1;
          break;
        }
        case Mdvx::ENCODING_INT16: {
          int64_t npts = _fhdr.nx * _fhdr.ny * _fhdr.nz;
          if (npts <= (int64_t) (_fhdr.volume_size / sizeof(ui16))) {
            ui16 missing = (ui16) _fhdr.missing_data_value;
            ui16 *val = (ui16 *) _volBuf.getPtr();
            for (int64_t i = 0; i < npts; i++, val++) {
              *val = missing;
            }
          }
          _fhdr.data_element_nbytes = 2;
          break;
        }
        case Mdvx::ENCODING_FLOAT32: {
          int64_t npts = _fhdr.nx * _fhdr.ny * _fhdr.nz;
          if (npts <= (int64_t) (_fhdr.volume_size / sizeof(fl32))) {
            fl32 missing = (fl32) _fhdr.missing_data_value;
            fl32 *val = (fl32 *) _volBuf.getPtr();
            for (int64_t i = 0; i < npts; i++, val++) {
              *val = missing;
            }
          }
          _fhdr.data_element_nbytes = 4;
          break;
        }
        case Mdvx::ENCODING_RGBA32: {
          int64_t npts = _fhdr.nx * _fhdr.ny * _fhdr.nz;
          if (npts <= (int64_t) (_fhdr.volume_size / sizeof(ui32))) {
            ui32 missing = (ui32) _fhdr.missing_data_value;
            ui32 *val = (ui32 *) _volBuf.getPtr();
            for (int64_t i = 0; i < npts; i++, val++) {
              *val = missing;
            }
          }
          _fhdr.data_element_nbytes = 4;
          break;
        }
      }
      
      _fhdr.compression_type = Mdvx::COMPRESSION_NONE;
      _fhdr.transform_type = Mdvx::DATA_TRANSFORM_NONE;
      _fhdr.scaling_type = Mdvx::SCALING_NONE;
      _fhdr.volume_size =
        _fhdr.data_element_nbytes * _fhdr.nx * _fhdr.ny * _fhdr.nz;
      
    } // if (init_with_missing)
    
  } // else if (alloc_mem)

  if (_volbufSizeValid() && _fhdr.volume_size > 0 && vol_data != NULL) {
    
    // uncompress if needed
    // compression is deferred until write
    
    int compression_type = _fhdr.compression_type;
    if (compression_type != Mdvx::COMPRESSION_NONE) {
      decompress();
      requestCompression(compression_type);
    }
    
    // Check for NaN, infinity values.
    
    _check_finite(_volBuf.getPtr());
    
    // compute min and max
    
    if (compute_min_and_max) {
      computeMinAndMax(true);
    }

  }

}

////////////////////////////////////////////////////////////////////
//
// Set object status for single plane from field header and data parts.
//
// If the plane data pointer is not NULL, space is allocated for it
// and the data is copied in. Data should not be compressed.
//
// If the plane data pointer is NULL, space is allocated for data based
// on the volume_size in the field header.
//
// NOTE: for this to work, the following items in the 
//       header must be set:
//
//    nx, ny, nz, volume_size, data_element_nbytes,
//    bad_data_value, missing_data_value
//
//  In addition, for INT8 and INT16, set:
//    scale, bias

void MdvxField::setHdrsAndPlaneData(int plane_num,
                                    int64_t plane_size,
                                    const Mdvx::field_header_t &f_hdr,
                                    const Mdvx::vlevel_header_t &v_hdr,
                                    const void *plane_data /* = NULL*/ )
  
{

  _fhdr = f_hdr;
  MEM_zero(_vhdr);
  _vhdr.level[0] = v_hdr.level[plane_num];
  _vhdr.type[0] = v_hdr.type[plane_num];

  _fhdr.nz = 1;
  _fhdr.grid_minz = f_hdr.grid_minz + plane_num * f_hdr.grid_dz;
  _fhdr.volume_size = plane_size * _fhdr.data_element_nbytes;

  _fhdrFile = NULL;
  _vhdrFile = NULL;

  _volBuf.clear();
  
  if (plane_data == NULL) {
    _volBuf.prepare(plane_size);
    _fhdr.compression_type = Mdvx::COMPRESSION_NONE;
  } else {
    _volBuf.add(plane_data, plane_size);
  }
    
  requestCompression(Mdvx::COMPRESSION_NONE);

}

//////////////////////////////////////////////////////////////////////
// clearing the volume data
//
// The data must be uncompressed.
// Sets all of the data values to missing.

void MdvxField::clearVolData()
{

  if (_fhdr.compression_type != Mdvx::COMPRESSION_NONE) {
    cerr << "WARNING - MdvxField::clearVolData" << endl;
    cerr << "   Data is compressed, cannot clear" << endl;
    return;
  }

  if (!_volbufSizeValid()) {
    // no op
    return;
  }
  
  switch (_fhdr.encoding_type) {

    case Mdvx::ENCODING_INT8 :
      _clearVolDataInt8();
      break;
      
    case Mdvx::ENCODING_INT16 :
      _clearVolDataInt16();
      break;
      
    case Mdvx::ENCODING_FLOAT32 :
      _clearVolDataFloat32();
      break;
      
    case Mdvx::ENCODING_RGBA32 :
      _clearVolDataRgba32();
      break;
    
    case Mdvx::ENCODING_ASIS :
      void *data = _volBuf.getPtr();
      memset(data, 0, _fhdr.volume_size);
      // cerr << "WARNING - MdvxField::clearVolData" << endl;
      // cerr << "   Encoding set to ENCODING_ASIS" << endl;
      // cerr << "   Data not changed" << endl;
      break;

  } /* endswitch - _fhdr.encoding_type */
  
  return;

}

//////////////////////////////////////////////////////////////////////
// clearing the INT8 volume data
//
// The data must be uncompressed.
// Sets all of the data values to missing.

void MdvxField::_clearVolDataInt8()
{
  ui08 *data = (ui08 *)_volBuf.getPtr();
  int64_t num_values = _fhdr.volume_size / _fhdr.data_element_nbytes;
  
  for (int64_t i = 0; i < num_values; ++i)
    data[i] = (ui08)_fhdr.missing_data_value;
}


//////////////////////////////////////////////////////////////////////
// clearing the INT16 volume data
//
// The data must be uncompressed.
// Sets all of the data values to missing.

void MdvxField::_clearVolDataInt16()
{
  ui16 *data = (ui16 *)_volBuf.getPtr();
  int64_t num_values = _fhdr.volume_size / _fhdr.data_element_nbytes;
  
  for (int64_t i = 0; i < num_values; ++i)
    data[i] = (ui16)_fhdr.missing_data_value;
}


//////////////////////////////////////////////////////////////////////
// clearing the FLOAT32 volume data
//
// The data must be uncompressed.
// Sets all of the data values to missing.

void MdvxField::_clearVolDataFloat32()
{
  fl32 *data = (fl32 *)_volBuf.getPtr();
  int64_t num_values = _fhdr.volume_size / _fhdr.data_element_nbytes;
  
  for (int64_t i = 0; i < num_values; ++i)
    data[i] = _fhdr.missing_data_value;
}


//////////////////////////////////////////////////////////////////////
// clearing the RGBA32 volume data
//
// The data must be uncompressed.
// Sets all of the data values to missing.

void MdvxField::_clearVolDataRgba32()
{
  ui32 *data = (ui32 *)_volBuf.getPtr();
  int64_t num_values = _fhdr.volume_size / _fhdr.data_element_nbytes;
  
  for (int64_t i = 0; i < num_values; ++i)
    data[i] = (ui32)_fhdr.missing_data_value;
}


//////////////////////////////////////////////////////////////
// isCompressed()
//
// Returns true of field is compressed, false otherwise

bool MdvxField::isCompressed() const

{
  if (_fhdr.compression_type >= Mdvx::COMPRESSION_RLE &&
      _fhdr.compression_type < Mdvx::COMPRESSION_TYPES_N) {
    // check buffer len
    size_t storedLen = _volBuf.getLen();
    size_t expectedLen = (_fhdr.nx * _fhdr.ny * _fhdr.nz *
                          _fhdr.data_element_nbytes);
    if (storedLen == expectedLen) {
      // not compressed
      _fhdr.compression_type = Mdvx::COMPRESSION_NONE;
      return false;
    } else {
      return true;
    }
  } else {
    return false;
  }
}

bool MdvxField::isCompressed(const Mdvx::field_header_t &fhdr)

{
  if (fhdr.compression_type >= Mdvx::COMPRESSION_RLE &&
      fhdr.compression_type < Mdvx::COMPRESSION_TYPES_N) {
    return true;
  } else {
    return false;
  }
}

///////////////////////////////////////////////////////////////////////////
// convertType()
//
// Convert field data to an output encoding type, specifying
// compression and scaling.
//
// The field object must contain a complete field header and data.
//
// Supported types are:
//   Mdvx::ENCODING_MDV_ASIS  - no change
//   Mdvx::ENCODING_INT8
//   Mdvx::ENCODING_INT16
//   Mdvx::ENCODING_FLOAT32
//
// If ENCODING_RGBA32 is used, compression is applied but type conversion is not.
//
// Supported compression types are:
//   Mdvx::COMPRESSION_ASIS - no change
//   Mdvx::COMPRESSION_NONE - uncompressed
//   Mdvx::COMPRESSION_RLE - see <toolsa/compress.h>
//   Mdvx::COMPRESSION_LZO - see <toolsa/compress.h>
//   Mdvx::COMPRESSION_ZLIB - see <toolsa/compress.h>
//   Mdvx::COMPRESSION_BZIP - see <toolsa/compress.h>
//   Mdvx::COMPRESSION_GZIP - see <toolsa/compress.h>
//   Mdvx::COMPRESSION_GZIP_VOL - GZIP with single buffer for vol
//
// Scaling types apply only to conversions to int types (INT8 and INT16)
//
// Supported scaling types are:
//   Mdvx::SCALING_ROUNDED
//   Mdvx::SCALING_INTEGRAL
//   Mdvx::SCALING_DYNAMIC
//   Mdvx::SCALING_SPECIFIED
//
// For Mdvx::SCALING_DYNAMIC, the scale and bias is determined from the
// dynamic range of the data.
//
// For Mdvx::SCALING_INTEGRAL, the operation is similar to
// Mdvx::SCALING_DYNAMIC, except that the scale and bias are
// constrained to integral values.
// 
// For Mdvx::SCALING_SPECIFIED, the specified scale and bias are used.
//
// Output scale and bias are ignored for conversions to float, and
// for Mdvx::SCALING_DYNAMIC and Mdvx::SCALING_INTEGRAL.
//
// Returns 0 on success, -1 on failure.
//
// On success, the volume data is converted, and the header is adjusted
// to reflect the changes.

int MdvxField::convertType
  (Mdvx::encoding_type_t output_encoding /* = Mdvx::ENCODING_FLOAT32*/,
   Mdvx::compression_type_t output_compression /* = Mdvx::COMPRESSION_NONE*/,
   Mdvx::scaling_type_t output_scaling /* = Mdvx::SCALING_DYNAMIC*/,
   double output_scale /* = 1.0*/,
   double output_bias /* = 0.0*/ )
     
{

  clearErrStr();
  char errstr[128];

  // check for consistency
  
  if (_fhdr.encoding_type == Mdvx::ENCODING_RGBA32) {

    if (output_encoding != Mdvx::ENCODING_RGBA32 &&
	output_encoding != Mdvx::ENCODING_ASIS) {
      _errStr += "ERROR - MdvxField::convertType()\n";
      sprintf(errstr, "  Cannot convert between RBGA32 and other types\n");
      _errStr += errstr;
      return -1;
    }

  } else {

    if (_fhdr.encoding_type != Mdvx::ENCODING_ASIS &&
	_fhdr.encoding_type != Mdvx::ENCODING_INT8 &&
	_fhdr.encoding_type != Mdvx::ENCODING_INT16 &&
	_fhdr.encoding_type != Mdvx::ENCODING_FLOAT32) {
      _errStr += "ERROR - MdvxField::convertType()\n";
      sprintf(errstr, "  Input encoding type %d not supported\n",
	      _fhdr.encoding_type);
      _errStr += errstr;
      return -1;
    }
    
    if (output_encoding != Mdvx::ENCODING_ASIS &&
	output_encoding != Mdvx::ENCODING_INT8 &&
	output_encoding != Mdvx::ENCODING_INT16 &&
	output_encoding != Mdvx::ENCODING_FLOAT32) {
      _errStr += "ERROR - MdvxField::convertType()\n";
      sprintf(errstr, "  Output encoding type %d not supported\n",
	      output_encoding);
      _errStr += errstr;
      return -1;
    }
    
  } // if (_fhdr.encoding_type == Mdvx::ENCODING_RGBA32)
  
  if (output_compression < Mdvx::COMPRESSION_ASIS ||
      output_compression >= Mdvx::COMPRESSION_TYPES_N) {
    _errStr += "ERROR - MdvxField::convertType()\n";
    sprintf(errstr, "  Output compression type %d not supported\n",
	    output_compression);
    _errStr += errstr;
    return -1;
  }

  if (output_scaling < Mdvx::SCALING_NONE ||
      output_scaling > Mdvx::SCALING_SPECIFIED) {
    _errStr += "ERROR - MdvxField::convertType()\n";
    sprintf(errstr, "  Output scaling type %d not supported\n",
	    output_scaling);
    _errStr += errstr;
    return -1;
  }

  if (!_volbufSizeValid()) {
    _errStr += "ERROR - MdvxField::convertType()\n";
    _errStr += "  volBuf not allocated\n";
    return -1;
  }
  
  // for floats ensure the scale and bias are neutral

  if (_fhdr.encoding_type == Mdvx::ENCODING_FLOAT32) {
    _fhdr.scale = 1.0;
    _fhdr.bias = 0.0;
  }

  // for ASIS encoding, set output_encoding accordingly
  
  if (output_encoding == Mdvx::ENCODING_ASIS) {
    output_encoding =
      (Mdvx::encoding_type_t) _fhdr.encoding_type;
  }

  // for ASIS compression, set output_compression accordingly
  
  if (output_compression == Mdvx::COMPRESSION_ASIS) {
    output_compression =
      (Mdvx::compression_type_t) _fhdr.compression_type;
  }

  // set nbytes and min/max in case not set

  _set_data_element_nbytes();

  // check for trivial case - input and output types are the same
  
  if (output_encoding == _fhdr.encoding_type) {

    if (output_compression == _fhdr.compression_type) {

      if (computeMinAndMax()) {
	return -1;
      }
      return 0;
      
    } else {

      // just change compression, not encoding type
      
      if (decompress()) {
	return -1;
      }
      
      if (computeMinAndMax()) {
	return -1;
      }
      requestCompression(output_compression);
      return 0;

    }

  }

  // decompress
  
  if (decompress()) {
    return -1;
  }
  
  if (computeMinAndMax()) {
    return -1;
  }

  // change encoding - switch on types
  
  if (_fhdr.encoding_type == Mdvx::ENCODING_INT8) {
    
    if (output_encoding == Mdvx::ENCODING_INT16) {
      
      _int8_to_int16(output_scaling,
		     output_scale,
		     output_bias);

    } else if (output_encoding == Mdvx::ENCODING_FLOAT32) {
      
      _int8_to_float32();
      
    }

  } else if (_fhdr.encoding_type == Mdvx::ENCODING_INT16) {
    
    if (output_encoding == Mdvx::ENCODING_INT8) {
      
      _int16_to_int8(output_scaling,
		     output_scale,
		     output_bias);
      
    } else if (output_encoding == Mdvx::ENCODING_FLOAT32) {

      _int16_to_float32();
      
    }

  } else if (_fhdr.encoding_type == Mdvx::ENCODING_FLOAT32) {

    _check_finite(_volBuf.getPtr());
      
    if (output_encoding == Mdvx::ENCODING_INT8) {
      
      if (output_scaling == Mdvx::SCALING_SPECIFIED) {
	_float32_to_int8(output_scale, output_bias);
      } else {
	_float32_to_int8(output_scaling);
      }
      
    } else if (output_encoding == Mdvx::ENCODING_INT16) {

      if (output_scaling == Mdvx::SCALING_SPECIFIED) {
	_float32_to_int16(output_scale, output_bias);
      } else {
	_float32_to_int16(output_scaling);
      }
      
    }

  }

  // set compression type
  
  requestCompression(output_compression);
      
  return 0;

}
		       
/////////////////////////////////////////////////////////////////
// This routine calls convertType(), with ROUNDED scaling.
// See convertType().
// 
// Returns 0 on success, -1 on failure.
//
int MdvxField::convertRounded
  (Mdvx::encoding_type_t output_encoding,
   Mdvx::compression_type_t output_compression /* = Mdvx::COMPRESSION_NONE*/ )
  
{
  
  int iret = convertType(output_encoding,
			 output_compression,
			 Mdvx::SCALING_ROUNDED, 0.0, 0.0);
  if (iret) {
    _errStr += "ERROR - MdvxField::convertRounded.\n";
    return -1;
  }

  return 0;
    
}
     
/////////////////////////////////////////////////////////////////
// This routine calls convertType(), with INTEGRAL scaling.
// See convertType().
// 
// Returns 0 on success, -1 on failure.
//
int MdvxField::convertIntegral
  (Mdvx::encoding_type_t output_encoding,
   Mdvx::compression_type_t output_compression /* = Mdvx::COMPRESSION_NONE*/ )
  
{
  
  int iret = convertType(output_encoding,
			 output_compression,
			 Mdvx::SCALING_INTEGRAL, 0.0, 0.0);
  if (iret) {
    _errStr += "ERROR - MdvxField::convertIntegral.\n";
    return -1;
  }

  return 0;
    
}
     
/////////////////////////////////////////////////////////////////
// This routine calls convertType(), with DYNAMIC scaling.
// See convertType().
// 
// Returns 0 on success, -1 on failure.
//
int MdvxField::convertDynamic
  (Mdvx::encoding_type_t output_encoding,
   Mdvx::compression_type_t output_compression /* = Mdvx::COMPRESSION_NONE*/ )
  
{
  
  int iret = convertType(output_encoding,
			 output_compression,
			 Mdvx::SCALING_DYNAMIC, 0.0, 0.0);
  if (iret) {
    _errStr += "ERROR - MdvxField::convertDynamic.\n";
    return -1;
  }

  return 0;
    
}
     
///////////////////////////////////////////////////////////////////////////
// transform2Log()
//
// Transform field data from linear to natural log values.
//
// The field object must contain a complete field header and data.
//
// Returns 0 on success, -1 on failure.
//
// On success, the volume data is converted, and the header is adjusted
// to reflect the changes.

int MdvxField::transform2Log()
     
{
  
  clearErrStr();

  if (_fhdr.transform_type == Mdvx::DATA_TRANSFORM_LOG ||
      _fhdr.encoding_type == Mdvx::ENCODING_RGBA32) {
    // no-op
    return 0;
  }

  if (_fhdr.transform_type != Mdvx::DATA_TRANSFORM_NONE) {
    _errStr += "ERROR - MdvxField::transform2Log\n";
    _errStr += "  Data must be linear before calling this routine.\n";
    return -1;
  }

  // save current state

  Mdvx::encoding_type_t encoding_type =
    (Mdvx::encoding_type_t) _fhdr.encoding_type;
  Mdvx::compression_type_t compression_type =
    (Mdvx::compression_type_t) _fhdr.compression_type;

  // transform to float uncompressed

  if (convertType(Mdvx::ENCODING_FLOAT32, Mdvx::COMPRESSION_NONE)) {
    _errStr += "ERROR - MdvxField::transform2Log\n";
    _errStr += "  Cannnot convert to fl32 uncompressed.\n";
    return -1;
  }

  // find the min value above zero, set a zero value 
  // 3 orders of magnitude below that

  fl32 missing = _fhdr.missing_data_value;
  int64_t nn = _volBuf.getLen() / sizeof(fl32);
  fl32 fmin = 1.0e30;
  bool allMissing = true;
  
  fl32 *ff = (fl32 *) _volBuf.getPtr();
  for (int64_t i = 0; i < nn; i++, ff++) {
    fl32 fff = *ff;
    if (fff != missing && fff > 0) {
      allMissing = false;
      if (fff < fmin) {
	fmin = fff;
      }
    }
  } // i
  fl32 fzero = fmin / 1000.0;

  if (!allMissing) {

    // transform to natural log
    
    ff = (fl32 *) _volBuf.getPtr();
    for (int64_t i = 0; i < nn; i++, ff++) {
      fl32 fff = *ff;
      if (fff != missing) {
	if (fff <= 0) {
	  *ff = fzero;
	} else {
	  *ff = log(fff);
	}
      }
    }
    
    // set headers accordingly
    
    computeMinAndMax(true);
    _fhdr.transform_type = Mdvx::DATA_TRANSFORM_LOG;
    memset(_fhdr.transform, 0, MDV_TRANSFORM_LEN);
    STRncopy(_fhdr.transform, "log", MDV_TRANSFORM_LEN);

  }

  // convert to original state
  
  if (convertType(encoding_type, compression_type)) {
    _errStr += "ERROR - MdvxField::transform2Log\n";
    _errStr += "  Cannnot convert to original encoding and compression.\n";
    return -1;
  }

  return 0;

}

///////////////////////////////////////////////////////////////////////////
// transform2Linear()
//
// Transform field data from natural log to linear values.
//
// The field object must contain a complete field header and data.
//
// Returns 0 on success, -1 on failure.
//
// On success, the volume data is converted, and the header is adjusted
// to reflect the changes.

int MdvxField::transform2Linear()
     
{
  
  clearErrStr();

  if (_fhdr.transform_type == Mdvx::DATA_TRANSFORM_NONE ||
      _fhdr.encoding_type == Mdvx::ENCODING_RGBA32) {
    // no-op
    return 0;
  }

  if (_fhdr.transform_type != Mdvx::DATA_TRANSFORM_LOG) {
    _errStr += "ERROR - MdvxField::transform2Linear\n";
    _errStr += "  Data must be log before calling this routine.\n";
    return -1;
  }

  // save current state

  Mdvx::encoding_type_t encoding_type =
    (Mdvx::encoding_type_t) _fhdr.encoding_type;
  Mdvx::compression_type_t compression_type =
    (Mdvx::compression_type_t) _fhdr.compression_type;

  // transform to float uncompressed

  if (convertType(Mdvx::ENCODING_FLOAT32, Mdvx::COMPRESSION_NONE)) {
    _errStr += "ERROR - MdvxField::transform2Linear\n";
    _errStr += "  Cannnot convert to fl32 uncompressed.\n";
    return -1;
  }

  // transform to linear from log

  fl32 missing = _fhdr.missing_data_value;
  fl32 *ff = (fl32 *) _volBuf.getPtr();
  int64_t nn = _volBuf.getLen() / sizeof(fl32);
  for (int64_t i = 0; i < nn; i++, ff++) {
    fl32 fff = *ff;
    if (fff != missing) {
      *ff = exp(fff);
    }
  }

  // set headers accordingly

  computeMinAndMax(true);
  _fhdr.transform_type = Mdvx::DATA_TRANSFORM_NONE;
  memset(_fhdr.transform, 0, MDV_TRANSFORM_LEN);
  STRncopy(_fhdr.transform, "none", MDV_TRANSFORM_LEN);

  // convert to original state
  
  if (convertType(encoding_type, compression_type)) {
    _errStr += "ERROR - MdvxField::transform2Linear\n";
    _errStr += "  Cannnot convert to original encoding and compression.\n";
    return -1;
  }

  return 0;

}

///////////////////////////////////////////////////////////////////////////
// negate()
//
// Convert each byte value to its negative value.
// The field object must contain a complete field header and data.
//
// This will also convert to linear if (a) the data is stored as
// logs and (b) convert_to_linear is true.
//
// Returns 0 on success, -1 on failure.
//
// On success, the volume data is converted, and the header is adjusted
// to reflect the changes.

int MdvxField::negate(bool convert_to_linear /* = false*/ )
     
{

  clearErrStr();

  if (_fhdr.encoding_type == Mdvx::ENCODING_RGBA32) {
    // no-op
    return 0;
  }

  // save current state

  Mdvx::encoding_type_t encoding_type =
    (Mdvx::encoding_type_t) _fhdr.encoding_type;
  Mdvx::compression_type_t compression_type =
    (Mdvx::compression_type_t) _fhdr.compression_type;
  
  bool convertToLinear = false;
  if (_fhdr.transform_type == Mdvx::DATA_TRANSFORM_LOG &&
      convert_to_linear) {
    convertToLinear = true;
  }

  // transform to float uncompressed

  if (convertType(Mdvx::ENCODING_FLOAT32, Mdvx::COMPRESSION_NONE)) {
    _errStr += "ERROR - MdvxField::transform2Linear\n";
    _errStr += "  Cannnot convert to fl32 uncompressed.\n";
    return -1;
  }

  // negate the values
  
  fl32 missing = _fhdr.missing_data_value;
  fl32 *ff = (fl32 *) _volBuf.getPtr();
  int64_t nn = _volBuf.getLen() / sizeof(fl32);
  for (int64_t i = 0; i < nn; i++, ff++) {
    fl32 fff = *ff;
    if (fff != missing) {
      if (convertToLinear) {
	*ff = -exp(fff);
      } else {
	*ff = -fff;
      }
    }
  }

  // set headers accordingly
  
  computeMinAndMax(true);
  if (convertToLinear) {
    _fhdr.transform_type = Mdvx::DATA_TRANSFORM_NONE;
    memset(_fhdr.transform, 0, MDV_TRANSFORM_LEN);
    STRncopy(_fhdr.transform, "none", MDV_TRANSFORM_LEN);
  }

  // convert to original state
  
  if (convertType(encoding_type, compression_type)) {
    _errStr += "ERROR - MdvxField::transform2Linear\n";
    _errStr += "  Cannnot convert to original encoding and compression.\n";
    return -1;
  }

  return 0;

}

///////////////////////////////////////////////////////////////////////
// Compute the composite (max at multiple levels) given lower and
// upper vlevel limits.
//
// Returns 0 on success, -1 on failure.

int MdvxField::convert2Composite(double lower_vlevel,
				 double upper_vlevel)

{
  
  if (_fhdr.encoding_type == Mdvx::ENCODING_RGBA32) {
    // no-op
    return 0;
  }

  int lowerPlane, upperPlane;
   
  computePlaneLimits(lower_vlevel, upper_vlevel,
		     lowerPlane, upperPlane);
  
  return convert2Composite(lowerPlane, upperPlane);
	  
}

///////////////////////////////////////////////////////////////////////
// Compute the composite (max at multiple levels) given lower and
// upper plane numbers.
//
// If lower_plane_num is -1, it is set to the lowest plane in the vol.
// If upper_plane_num is -1, it is set to the highest plane in the vol.
// 
// Returns 0 on success, -1 on failure.
//
  
int MdvxField::convert2Composite(int lower_plane_num /* = -1*/,
				 int upper_plane_num /* = -1*/ )
  
{

  clearErrStr();

  if (_fhdr.encoding_type == Mdvx::ENCODING_RGBA32) {
    // no-op
    return 0;
  }

  if (_fhdr.nz < 1)
  {
    fprintf(stderr, "ERROR - MdvxField::convert2Composite\n");
    fprintf(stderr, "  _fhdr.nz < 1\n");
    return -1;
  }

  int lowerPlaneNum = lower_plane_num;
  int upperPlaneNum = upper_plane_num;

  if (lowerPlaneNum < 0) {
    lowerPlaneNum = 0;
  } else if (lowerPlaneNum > _fhdr.nz - 1) {
    lowerPlaneNum = _fhdr.nz - 1;
  }
  if (upperPlaneNum == -1) {
    upperPlaneNum = _fhdr.nz - 1;
  } else if (upperPlaneNum < -1) {
    upperPlaneNum = 0;
  } else if (upperPlaneNum > _fhdr.nz - 1) {
    upperPlaneNum = _fhdr.nz - 1;
  }

  if (lowerPlaneNum > upperPlaneNum) {
    fprintf(stderr, "WARNING - MdvxField::convert2Composite\n");
    fprintf(stderr, "  Lower plane is above upper plane - switching\n");
    int tmp_plane_num = lowerPlaneNum;
    lowerPlaneNum = upperPlaneNum;
    upperPlaneNum = tmp_plane_num;
  }

  int64_t npoints_plane = _fhdr.nx * _fhdr.ny;

  // make sure we have decompressed data

  int compression_type = _fhdr.compression_type;
  bool recompress = false;
  if (isCompressed()) {
    if (decompress()) {
      fprintf(stderr, "ERROR - MdvxField::convert2Composite\n");
      fprintf(stderr, "  Error decompressing volume\n");
      return -1;
    }
    recompress = true;
  }

  // create working buffer

  MemBuf workBuf;

  // create composite depending on type

  switch (_fhdr.encoding_type) {
    
    case Mdvx::ENCODING_INT8: {
      int64_t nbytes_comp = npoints_plane * sizeof(ui08);
      ui08 *comp = (ui08 *) workBuf.prepare(nbytes_comp);
      if (comp == NULL) {
        fprintf(stderr, "ERROR - MdvxField::convert2Composite\n");
        fprintf(stderr, "  Error allocating memory\n");
        return -1;
      }
      for (int64_t i = 0; i < npoints_plane; ++i)
        comp[i] = (ui08) _fhdr.missing_data_value;
      for (int j = lowerPlaneNum; j <= upperPlaneNum; j++) {
        ui08 *c = comp;
        ui08 *v = (ui08 *) _volBuf.getPtr() + j * npoints_plane;
        for (int64_t i = 0; i < npoints_plane; i++, c++, v++) {
          ui08 val = *v;
          if (val == _fhdr.missing_data_value || val == _fhdr.bad_data_value)
            continue;
          if (*c == _fhdr.missing_data_value || *c == _fhdr.bad_data_value ||
              val > *c) {
            *c = val;
          }
        } // i */
      } // j */
      break;
    }

    case Mdvx::ENCODING_INT16: {
      int64_t nbytes_comp = npoints_plane * sizeof(ui16);
      ui16 *comp = (ui16 *) workBuf.prepare(nbytes_comp);
      if (comp == NULL) {
        fprintf(stderr, "ERROR - MdvxField::convert2Composite\n");
        fprintf(stderr, "  Error allocating memory\n");
        return -1;
      }
      for (int64_t i = 0; i < npoints_plane; ++i)
        comp[i] = (ui16) _fhdr.missing_data_value;
      for (int j = lowerPlaneNum; j <= upperPlaneNum; j++) {
        ui16 *v = (ui16 *) _volBuf.getPtr() + j * npoints_plane;
        ui16 *c = comp;
        for (int64_t i = 0; i < npoints_plane; i++, c++, v++) {
          ui16 val = *v;
          if (val == _fhdr.missing_data_value || val == _fhdr.bad_data_value)
            continue;
          if (*c == _fhdr.missing_data_value || *c == _fhdr.bad_data_value ||
              val > *c) {
            *c = val;
          }
        } // i */
      } // j */
      break;
    }

    case Mdvx::ENCODING_FLOAT32: {
      int64_t nbytes_comp = npoints_plane * sizeof(fl32);
      fl32 *comp = (fl32 *) workBuf.prepare(nbytes_comp);
      if (comp == NULL) {
        fprintf(stderr, "ERROR - MdvxField::convert2Composite\n");
        fprintf(stderr, "  Error allocating memory\n");
        return -1;
      }
      fl32 *c = comp;
      for (int64_t i = 0; i < npoints_plane; i++, c++) {
        *c = _fhdr.missing_data_value;
      }
      for (int j = lowerPlaneNum; j <= upperPlaneNum; j++) {
        fl32 *v = (fl32 *) _volBuf.getPtr() + j * npoints_plane;
        c = comp;
        for (int64_t i = 0; i < npoints_plane; i++, c++, v++) {
          fl32 val = *v;
          if (val == _fhdr.missing_data_value || val == _fhdr.bad_data_value)
            continue;
          if (*c == _fhdr.missing_data_value || *c == _fhdr.bad_data_value ||
              val > *c) {
            *c = val;
          }
        } // i */
      } // j */
      break;
    }

  } // switch */

  // copy data from work buf

  _volBuf = workBuf;

  // adjust header
  
  _fhdr.nz = 1;
  _fhdr.data_dimension = 2;
  _fhdr.grid_dz = 0;
  _fhdr.grid_minz = _vhdr.level[lowerPlaneNum];
  _fhdr.vlevel_type = Mdvx::VERT_TYPE_COMPOSITE;

  _fhdr.volume_size = _fhdr.nx * _fhdr.ny * _fhdr.nz
    * _fhdr.data_element_nbytes;
  
  if (_fhdr.volume_size == 0) {
    cerr << "WARNING - MdvxField::convert2Composite" << endl;
    cerr << "  Field name: " << _fhdr.field_name << endl;
    cerr << "  Volume size: " << _fhdr.volume_size << endl;
  }

  MEM_zero(_vhdr);
  _vhdr.level[0] = _fhdr.grid_minz;
  _vhdr.type[0] = Mdvx::VERT_TYPE_COMPOSITE;

  STRconcat(_fhdr.field_name_long, "_composite", MDV_LONG_FIELD_LEN);

  if (recompress) {
    requestCompression(compression_type);
  }

  return 0;

}
     
///////////////////////////////////////////////////////////////////
// Convert vlevel type if possible
//
// Conversion between the following types are supported:
//
//   VERT_TYPE_Z
//   VERT_TYPE_PRESSURE
//   VERT_FLIGHT_LEVEL

void MdvxField::convertVlevelType(Mdvx::vlevel_type_t req_vlevel_type)

{

  // check - can we do the conversion?
  
  bool failure = false;

  if (!_volbufSizeValid()) {
    cerr << "ERROR: MdvxField::convertVlevelType" << endl;
    cerr << "  volBuf not allocted" << endl;
    failure = true;
  }

  if (req_vlevel_type != Mdvx::VERT_TYPE_Z &&
      req_vlevel_type != Mdvx::VERT_TYPE_PRESSURE &&
      req_vlevel_type != Mdvx::VERT_FLIGHT_LEVEL) {
    cerr << "WARNING: MdvxField::convertVlevelType" << endl;
    cerr << "  Requested vlevel type: "
	 << Mdvx::vertType2Str(req_vlevel_type) << endl;
    failure = true;
  }

  Mdvx::vlevel_type_t file_vlevel_type =
    (Mdvx::vlevel_type_t) _fhdr.vlevel_type;
  if (file_vlevel_type != Mdvx::VERT_TYPE_Z &&
      file_vlevel_type != Mdvx::VERT_TYPE_PRESSURE &&
      file_vlevel_type != Mdvx::VERT_FLIGHT_LEVEL) {
    cerr << "WARNING: MdvxField::convertVlevelType" << endl;
    cerr << "  Requested vlevel type: "
	 << Mdvx::vertType2Str(req_vlevel_type) << endl;
    cerr << "  File vlevel type: "
	 << Mdvx::vertType2Str(file_vlevel_type) << endl;
    failure = true;
  }

  if (failure) {
    cerr << "  Field: " << _fhdr.field_name << endl;
    cerr << "  Only the following are supported for conversion:" << endl;
    cerr << "    VERT_TYPE_Z" << endl;
    cerr << "    VERT_TYPE_PRESSURE" << endl;
    cerr << "    VERT_TYPE_FLIGHT_LEVEL" << endl;
    cerr << "  Conversion will NOT be done" << endl;
    cerr << endl;
    return;
  }

  // no-op?
  
  if (req_vlevel_type == file_vlevel_type) {
    return;
  }

  // do the conversion for the vlevels
  
  _convert_vlevels(file_vlevel_type, req_vlevel_type,
                   _fhdr, _vhdr);

  if (_fhdrFile != NULL && _vhdrFile != NULL) {
    _convert_vlevels(file_vlevel_type, req_vlevel_type,
                     *_fhdrFile, *_vhdrFile);
  }

}

///////////////////////////////////////////////////////////////////////
// Compute the vertical section for the data volume, given the 
// waypts, nsamples and lookup table object. If the lookup table has not
// been initialized it is computed. If the waypts, n_samples or
// grid geometry has changed the lookup table is recomputed.
//
// If fill_missing is set and fewer than 20 % of the grid points in a
// plane are missing, the missing vals are filled in by copying in
// from adjacent points.
//
// Returns 0 on success, -1 on failure.

int MdvxField::convert2Vsection(const Mdvx::master_header_t &mhdr,
                                const vector<Mdvx::vsect_waypt_t> &waypts,
				int n_samples,
				MdvxVsectLut &lut,
				bool fill_missing /* = false*/,
				bool interp /* = true*/,
				bool specify_vlevel_type /* = false */,
				Mdvx::vlevel_type_t vlevel_type /* = Mdvx::VERT_TYPE_Z */,
				bool do_final_convert /* = true */)
  
{

  clearErrStr();

  if (waypts.size() < 1) {
    _errStr += "ERROR - MdvxField::convert2Vsection\n";
    _errStr += "  0 waypoints specified\n";
    return -1;
  }

  // compute lookup table - does not recompute if not needed

  MdvxProj proj(_fhdr);
  proj.setConditionLon2Origin(true);
  if (interp) {
    lut.computeWeights(waypts, n_samples, proj);
  } else {
    lut.computeOffsets(waypts, n_samples, proj);
  }
  
  // save state

  Mdvx::encoding_type_t encoding_type =
    (Mdvx::encoding_type_t) _fhdr.encoding_type;
  Mdvx::compression_type_t compression_type =
    (Mdvx::compression_type_t) _fhdr.compression_type;
  Mdvx::scaling_type_t scaling_type =
    (Mdvx::scaling_type_t) _fhdr.scaling_type;
  double scale = _fhdr.scale;
  double bias = _fhdr.bias;

  // convert to float and uncompress as required

  if (encoding_type == Mdvx::ENCODING_RGBA32) {
    if (convertType(Mdvx::ENCODING_ASIS, Mdvx::COMPRESSION_NONE)) {
      _errStr += "ERROR - MdvxField::convert2Vsection\n";
      return -1;
    }
  } else {
    if (convertType(Mdvx::ENCODING_FLOAT32, Mdvx::COMPRESSION_NONE)) {
      _errStr += "ERROR - MdvxField::convert2Vsection\n";
      return -1;
    }
  }

  const vector<Mdvx::vsect_samplept_t> &samplePts = lut.getSamplePts();
  int64_t nSamplePoints = samplePts.size();
  
  // set up working buffer

  MemBuf workBuf;
  int64_t nBytes = nSamplePoints * _fhdr.nz * _fhdr.data_element_nbytes;
  workBuf.prepare(nBytes);

  // compute the vert section

  if (_fhdr.proj_type == Mdvx::PROJ_POLAR_RADAR) {
    _computeVsectionPolarRadar(lut, proj, workBuf);
  } else if (encoding_type == Mdvx::ENCODING_RGBA32) {
    _computeVsectionRGBA(lut, workBuf);
  } else {
    _computeVsection(lut, interp, workBuf);
  }
  
  if (_fhdr.proj_type == Mdvx::PROJ_POLAR_RADAR ||
      _fhdr.vlevel_type == Mdvx::VERT_TYPE_ELEV) {
    
    // ppi or polar radar data - convert elev angles to height

    _vsectionElev2Ht(mhdr, proj, lut, interp, workBuf);

  }
  
  if (fill_missing) {
    _vsection_fill_missing(_fhdr.encoding_type,
                           _fhdr.missing_data_value,
                           workBuf.getPtr(), nSamplePoints, _fhdr.nz);
  }

  // convert vlevel type if needed

  if (specify_vlevel_type) {
    convertVlevelType(vlevel_type);
  }
  
  // copy the work buf to the vol buf
  
  _volBuf = workBuf;

  // set the header appropriately

  _fhdr.proj_type = Mdvx::PROJ_VSECTION;
  _fhdr.volume_size = _volBuf.getLen();

  _fhdr.nx = nSamplePoints;
  _fhdr.grid_dx = lut.getDxKm();
  _fhdr.grid_minx = 0.0;

  _fhdr.ny = 1;
  _fhdr.grid_dy = 1.0;
  _fhdr.grid_miny = 0.0;

  if (samplePts.size() > 0) {
    _fhdr.proj_origin_lat = samplePts[0].lat;
    _fhdr.proj_origin_lon = samplePts[0].lon;
  } else if (waypts.size() > 0) {
    _fhdr.proj_origin_lat = waypts[0].lat;
    _fhdr.proj_origin_lon = waypts[0].lon;
  } else {
    _fhdr.proj_origin_lat = 0.0;
    _fhdr.proj_origin_lon = 0.0;
  }

  // For Polar or PPI radar data, the cross section is interpolated onto
  // a cartesian grid, with km MSL on the vertical scale.

  if(_fhdr.proj_type == Mdvx::PROJ_POLAR_RADAR ||
     _fhdr.vlevel_type == Mdvx::VERT_TYPE_ELEV) {
    _fhdr.vlevel_type = Mdvx::VERT_TYPE_Z;
  }

  computeMinAndMax();

  if (do_final_convert) {
    if (convertType(encoding_type, compression_type,
		    scaling_type, scale, bias)) {
      _errStr += "ERROR - MdvxField::convert2Vsection\n";
      return -1;
    }
  }

  return 0;

}

///////////////////////////////////////////////////////////////////////
// Compute to a single RHI.
//
// The RHI in the file is stored as: (x = range, y = elevation, z = azimuth)
// The Single RHI stored as (x = range, y = az, z = elevation), in order
// to match the vsection format.
//
// Returns 0 on success, -1 on failure.

int MdvxField::convert2SingleRhi(const Mdvx::master_header_t &mhdr,
				 int rhiIndex,
				 const vector<Mdvx::vsect_waypt_t> &waypts,
				 MdvxVsectLut &lut,
				 bool do_final_convert /* = true */)
  
{
  
  clearErrStr();

  // convert to float and uncompress as required
  
  Mdvx::encoding_type_t encoding_type =
    (Mdvx::encoding_type_t) _fhdr.encoding_type;
  Mdvx::compression_type_t compression_type =
    (Mdvx::compression_type_t) _fhdr.compression_type;
  if (encoding_type == Mdvx::ENCODING_RGBA32) {
    _errStr += "ERROR - MdvxField::convert2SingleRhi\n";
    _errStr += "  ENCODING_RGBA32 not applicable for RHI conversion";
    return -1;
  }
  if (convertType(Mdvx::ENCODING_FLOAT32, Mdvx::COMPRESSION_NONE)) {
    _errStr += "ERROR - MdvxField::convert2SingleRhi\n";
    return -1;
  }

  // compute sample points

  lut.computeSamplePts(waypts, _fhdr.nx);

  // just get the vlevel we need - this is actually one RHI
  // because the vlevels represent azimuth
  
  convert2Composite(rhiIndex, rhiIndex);
  
  // set the header appropriately
  
  _fhdr.proj_type = Mdvx::PROJ_RHI_RADAR;
  _fhdr.vlevel_type = Mdvx::VERT_TYPE_AZ;
  
  computeMinAndMax();
  
  if (do_final_convert) {
    if (convertType(encoding_type, compression_type)) {
      _errStr += "ERROR - MdvxField::convert2SingleRhi\n";
      return -1;
    }
  }

  return 0;


}

///////////////////////////////////////////////////////////////////////
// Compute a vertical section from an RHI.
//
// The RHI is stored as: (x = range, y = elevation, z = azimuth)
// The Vsection is stored as (x = range, y = az, z = ht)
//
// Returns 0 on success, -1 on failure.

int MdvxField::convertRhi2Vsect(const Mdvx::master_header_t &mhdr,
				int rhiIndex,
				const vector<Mdvx::vsect_waypt_t> &waypts,
				MdvxVsectLut &lut,
				bool do_final_convert /* = true */)
  
{
  
  clearErrStr();
  
  // convert to float and uncompress as required
  
  Mdvx::encoding_type_t encoding_type =
    (Mdvx::encoding_type_t) _fhdr.encoding_type;
  Mdvx::compression_type_t compression_type =
    (Mdvx::compression_type_t) _fhdr.compression_type;
  if (encoding_type == Mdvx::ENCODING_RGBA32) {
    _errStr += "ERROR - MdvxField::convertRhi2Vsect\n";
    _errStr += "  ENCODING_RGBA32 not applicable for RHI conversion";
    return -1;
  }
  if (convertType(Mdvx::ENCODING_FLOAT32, Mdvx::COMPRESSION_NONE)) {
    _errStr += "ERROR - MdvxField::convertRhi2Vsect\n";
    return -1;
  }

  // compute sample points

  lut.computeSamplePts(waypts, _fhdr.nx);

  // just get the vlevel we need - this is actually one RHI
  // because the vlevels represent azimuth
  
  convert2Composite(rhiIndex, rhiIndex);
  double azimuth = _vhdr.level[0];
  
  // compute array of sines and cosines for elev angles
  
  double *elev = new double[_fhdr.ny];
  double *sinElev = new double[_fhdr.ny];
  double *cosElev = new double[_fhdr.ny];
  double minCosElev = 1.0;
  for (int iy = 0; iy < _fhdr.ny; iy++) {
    elev[iy] = _fhdr.grid_miny + iy * _fhdr.grid_dy;
    double rad = elev[iy] * DEG_TO_RAD;
    EG_sincos(rad, &sinElev[iy], &cosElev[iy]);
    if (cosElev[iy] < minCosElev) {
      minCosElev = cosElev[iy];
    }
  }

  // compute the vertical dimensions in km
  
  double maxRange = _fhdr.grid_minx + _fhdr.nx * _fhdr.grid_dx;
  double twiceRadius = PSEUDO_RADIUS * 2.0;
  double sensorHt = mhdr.sensor_alt;
  double maxHt = (sensorHt + maxRange * sinElev[_fhdr.ny-1] +
		  maxRange * maxRange / twiceRadius);
  if (maxHt > 25.0) {
    maxHt = 25.0;
  }
  double dz = _round_dz((maxHt - sensorHt) / 120.0);
  int nz = (int) ((maxHt - sensorHt) / dz) + 2;
  if (nz > MDV_MAX_VLEVELS) {
    nz = MDV_MAX_VLEVELS;
  }
  double minz = ((int) (sensorHt / dz)) * dz;
  
  // compute x origin

  double minX = _fhdr.grid_minx;
  int nX = _fhdr.nx;
  if (minCosElev < 0) {
    double min_x = maxRange * minCosElev;
    int nNeg = floor(fabs(min_x) / _fhdr.grid_dx + 1.5);
    minX -= nNeg * _fhdr.grid_dx;
    nX += nNeg;
  }
  
  // set up working buffer, initialize to missing vals
  
  MemBuf workBuf;
  int64_t nGridOut = nX * nz;
  int64_t nBytes = nGridOut * _fhdr.data_element_nbytes;
  workBuf.prepare(nBytes);
  
  if ( workBuf.getPtr() == NULL) {
    _errStr += "ERROR - MdvxField::convertRhi2Vsect\n";
    delete[] elev;
    delete[] sinElev;
    delete[] cosElev;
    return -1;
  }

  fl32 missing = (fl32) _fhdr.missing_data_value;
  fl32 *vval = (fl32 *) workBuf.getPtr();
  for (int64_t i = 0;  i < nGridOut; i++, vval++) {
    *vval = missing;
  }

  // interp onto output grid

  fl32 *in = (fl32 *) _volBuf.getPtr();
  fl32 *out = (fl32 *) workBuf.getPtr();
  
  for (int iz = 0; iz < nz; iz++) { // height
    
    double ht = minz + iz * dz;
    double gndRange = minX;
    
    for (int ix = 0; ix < nX;
	 ix++, gndRange += _fhdr.grid_dx, out++) { // range

      double range = sqrt(gndRange * gndRange + ht * ht);
      int irange = (int) ((range - _fhdr.grid_minx) / _fhdr.grid_dx + 0.5);
      if (irange >= _fhdr.nx) {
	continue;
      }

      double zcorr = (range * range) / twiceRadius;
      double zht = ht - zcorr;
      double el = asin(zht/range) * RAD_TO_DEG;
      if (gndRange < 0) {
        el += 90.0;
      }
      int iel = (int) ((el - _fhdr.grid_miny) / _fhdr.grid_dy);
      
      if (iel >= 0 && iel < _fhdr.ny - 1) {
	
	// between 2 elevations
	
	double wt = (elev[iel+1] - el) / _fhdr.grid_dy;
	fl32 *below = in + iel * _fhdr.nx + irange;
	fl32 *above = below + _fhdr.nx;
	*out = (wt * (*below)) + ((1.0 - wt) * (*above));
	
      } else if (iel == -1) {
	
	// just below lowest elevation, copy from lowest elev
	
	*out = in[irange];
	
      } else if (iel == _fhdr.ny - 1) {

	// just above upper elev, copy from upper elev
	
	*out = in[iel * _fhdr.nx + irange];
	
      }

    } // ix

  } // iz

  // copy the work buf to the vol buf
  
  _volBuf = workBuf;
  
  // set the header appropriately

  _fhdr.proj_type = Mdvx::PROJ_RHI_RADAR;
  _fhdr.volume_size = _volBuf.getLen();

  _fhdr.ny = 1;
  _fhdr.grid_dy = 1.0;
  _fhdr.grid_miny = azimuth;

  _fhdr.nz = nz;
  _fhdr.grid_dz = (fl32) dz;
  _fhdr.grid_minz = (fl32) minz;
  _fhdr.dz_constant = true;

  _fhdr.vlevel_type = Mdvx::VERT_TYPE_Z;
  memset(_vhdr.level, 0, MDV_MAX_VLEVELS * sizeof(fl32));
  memset(_vhdr.type, 0, MDV_MAX_VLEVELS * sizeof(si32));

  for (int iz = 0; iz < nz; iz++) {
    _vhdr.level[iz] = minz + iz * dz;
    _vhdr.type[iz] = Mdvx::VERT_TYPE_Z;
  }
  
  computeMinAndMax();

  if (do_final_convert) {
    if (convertType(encoding_type, compression_type)) {
      _errStr += "ERROR - MdvxField::convertRhi2Vsect\n";
      delete[] elev;
      delete[] sinElev;
      delete[] cosElev;

      return -1;
    }
  }
  delete[] elev;
  delete[] sinElev;
  delete[] cosElev;
  
  return 0;

}

///////////////////////////////////////////////////////////////////////
// Vertical section computation, all projections except polar radar

void MdvxField::_computeVsection(MdvxVsectLut &lut,
				 bool interp,
				 MemBuf &workBuf)

{

  const vector<Mdvx::vsect_samplept_t> &samplePts = lut.getSamplePts();
  int64_t nSamplePoints = samplePts.size();
  int64_t nGridPoints = nSamplePoints * _fhdr.nz;
  
  // initialize the work buffer to missing vals
  
  fl32 missing = (fl32) _fhdr.missing_data_value;
  fl32 *vval = (fl32 *) workBuf.getPtr();
  for (int64_t i = 0;  i < nGridPoints; i++, vval++) {
    *vval = missing;
  }
  
  fl32 *in = (fl32 *) _volBuf.getPtr();
  fl32 *out = (fl32 *) workBuf.getPtr();
  int64_t npointsPlane = _fhdr.nx * _fhdr.ny;
  
  if (interp) {
    
    // use weights for interpolation
    
    const vector<MdvxVsectLutEntry> &entries = lut.getWeights();
    
    for (int64_t ii = 0; ii < nSamplePoints; ii++) {
      
      const MdvxVsectLutEntry entry = entries[ii];
      
      if (entry.set && entry.wts[0] > 0 && entry.wts[1] > 0 &&
	  entry.wts[2] > 0 && entry.wts[3] > 0) {

	for (int64_t iz = 0; iz < _fhdr.nz; iz++) {
	  fl32 v0 = in[iz * npointsPlane + entry.offsets[0]];
	  fl32 v1 = in[iz * npointsPlane + entry.offsets[1]];
	  fl32 v2 = in[iz * npointsPlane + entry.offsets[2]];
	  fl32 v3 = in[iz * npointsPlane + entry.offsets[3]];
	  if (v0 != missing && v1 != missing &&
 	      v2 != missing && v3 != missing) {
	    double vv = (v0 * entry.wts[0] +
			 v1 * entry.wts[1] +
			 v2 * entry.wts[2] +
			 v3 * entry.wts[3]);
	    out[iz * nSamplePoints + ii] = vv;
	  } // if (v0 != missing && v1 != missing ...
	} // iz
	
      } //   if (entry.wts[0] > 0 ...
      
    } // ii
	  
  } else {

    // no interpolation - nearest neighbor
    
    const vector<int64_t> &offsets = lut.getOffsets();
    
    for (int ii = 0; ii < nSamplePoints; ii++) {
      if (offsets[ii] >= 0) {
	for (int64_t iz = 0; iz < _fhdr.nz; iz++) {
	  fl32 vv = in[iz * npointsPlane + offsets[ii]];
	  out[iz * nSamplePoints + ii] = vv;
	} // iz
      } // if (offsets[ii] >= 0)
    } // ii

  } // if (interp) 
  
  return;

}

///////////////////////////////////////////////////////////////////////
// Vertical section computation for RGBA

void MdvxField::_computeVsectionRGBA(MdvxVsectLut &lut,
				     MemBuf &workBuf)
  
{
  
  const vector<Mdvx::vsect_samplept_t> &samplePts = lut.getSamplePts();
  int64_t nSamplePoints = samplePts.size();
  int64_t nGridPoints = nSamplePoints * _fhdr.nz;
  
  // initialize the work buffer to missing vals
  
  ui32 missing = (ui32) _fhdr.missing_data_value;
  ui32 *vval = (ui32 *) workBuf.getPtr();
  for (int64_t i = 0;  i < nGridPoints; i++, vval++) {
    *vval = missing;
  }
  
  ui32 *in = (ui32 *) _volBuf.getPtr();
  ui32 *out = (ui32 *) workBuf.getPtr();
  int64_t npointsPlane = _fhdr.nx * _fhdr.ny;
  
  // nearest neighbor
    
  const vector<int64_t> &offsets = lut.getOffsets();
    
  for (int64_t ii = 0; ii < nSamplePoints; ii++) {
    if (offsets[ii] >= 0) {
      for (int64_t iz = 0; iz < _fhdr.nz; iz++) {
	ui32 vv = in[iz * npointsPlane + offsets[ii]];
	out[iz * nSamplePoints + ii] = vv;
      } // iz
    } // if (offsets[ii] >= 0)
  } // ii
  
  return;

}

///////////////////////////////////////////////////////////////////////
// Vertical section computation, polar radar projection

void MdvxField::_computeVsectionPolarRadar(const MdvxVsectLut &lut,
					   const MdvxProj &proj,
					   MemBuf &workBuf)

{

  const vector<Mdvx::vsect_samplept_t> &samplePts = lut.getSamplePts();
  int64_t nSamplePoints = samplePts.size();
  int64_t nGridPoints = nSamplePoints * _fhdr.nz;
  
  // initialize the work buffer to missing vals
  
  fl32 missing = (fl32) _fhdr.missing_data_value;
  fl32 *vval = (fl32 *) workBuf.getPtr();
  for (int64_t i = 0;  i < nGridPoints; i++, vval++) {
    *vval = missing;
  }
  
  fl32 *in = (fl32 *) _volBuf.getPtr();
  fl32 *out = (fl32 *) workBuf.getPtr();
  int64_t npointsPlane = _fhdr.nx * _fhdr.ny;
  
  for (size_t ii = 0; ii < samplePts.size(); ii++) {
    
    int ix, iy;
    if (proj.latlon2xyIndex(samplePts[ii].lat, samplePts[ii].lon,
			    ix, iy) == 0) {
      
      for (int64_t iz = 0; iz < _fhdr.nz; iz++) {

	// compute the gate number correction for the elevation angle
	
        double elevDeg = _vhdr.level[iz];
        if (fabs(elevDeg) > 89.0) {
          elevDeg = 89.0;
        }
	double cosel = cos(elevDeg * DEG_TO_RAD);
	int64_t ixz = (int) ((double) ix / cosel + 0.5);
	int64_t offset = (int64_t) iy * _fhdr.nx + ixz;
        if (offset > npointsPlane - 1) {
          offset = npointsPlane - 1;
        }
	
	fl32 vv = in[iz * npointsPlane + offset];
	out[iz * nSamplePoints + ii] = vv;

      } // iz

    } // if (proj.latlon2xyIndex( ...

  } // ii

   
  return;

}

///////////////////////////////////////////////////////////////////////
// Vertical section modification for PPI

void MdvxField::_vsectionElev2Ht(const Mdvx::master_header_t &mhdr,
                                 const MdvxProj &proj,
				 const MdvxVsectLut &lut,
				 bool interp,
				 MemBuf &workBuf)

{

  const vector<Mdvx::vsect_samplept_t> &samplePts = lut.getSamplePts();
  int64_t nSamplePoints = samplePts.size();
  
  // precompute the sin and cos of the elevation angles
  
  int nElev = _fhdr.nz;
  double *sinElev = new double[nElev];
  double *cosElev = new double[nElev];

  if(_fhdr.dz_constant == 0) {
    for (int ielev = 0; ielev < nElev; ielev++) {
      double rad = _vhdr.level[ielev] * DEG_TO_RAD;
      EG_sincos(rad, &sinElev[ielev], &cosElev[ielev]);
    }
  } else { // Handle single plane or regular spaced data
    double elev = _fhdr.grid_minz;
    for(int ielev = 0; ielev < nElev; ielev++) {
      double rad = elev * DEG_TO_RAD;
      EG_sincos(rad, &sinElev[ielev], &cosElev[ielev]);
      elev += _fhdr.grid_dz;
    }
  }

  // compute the range from the radar to the sample points
  // In both ppi and polar cases, the radar is at the proj origin
  
  double *gndRange = new double[nSamplePoints];
  double maxGndRange = 0.0;
  
  for (int64_t ii = 0; ii < nSamplePoints; ii++) {
    double xx, yy;
    PJGLatLon2DxDy(_fhdr.proj_origin_lat, _fhdr.proj_origin_lon,
		   samplePts[ii].lat, samplePts[ii].lon,
		   &xx, &yy);
    double thisRange = sqrt(xx * xx + yy * yy);
    gndRange[ii] = thisRange;
    maxGndRange = MAX(maxGndRange, thisRange);
  } // ii

  // compute the vertical dimensions

  double maxSlantRange = maxGndRange / cosElev[nElev-1];
  double twiceRadius = PSEUDO_RADIUS * 2.0;
  double sensorHt = mhdr.sensor_alt;
  double maxHt = (sensorHt + maxSlantRange * sinElev[nElev-1] +
		  maxSlantRange * maxSlantRange / twiceRadius);
  if (maxHt > 30.0) {
    maxHt = 30.0;
  }
  double dz = _round_dz((maxHt - sensorHt) / 100.0);
  int nz = (int) ((maxHt - sensorHt) / dz) + 2;
  if (nz > MDV_MAX_VLEVELS) {
    nz = MDV_MAX_VLEVELS;
  }
  double minz = ((int) (sensorHt / dz)) * dz;

  // set up interpolation buffer
  
  MemBuf interpBuf;
  int64_t nBytes = nSamplePoints * nz * _fhdr.data_element_nbytes;
  interpBuf.prepare(nBytes);
  if (interpBuf.getPtr() == NULL){
    _errStr += "ERROR - MdvxField::vsectionElev2Ht\n";
    _errStr += "  MemBuf mem allocation error";
    delete[] sinElev;
    delete[] cosElev;
    delete[] gndRange;
    return;
  }
  // initialize the interp buffer to missing vals
  
  fl32 missing = (fl32) _fhdr.missing_data_value;
  fl32 *vval = (fl32 *) interpBuf.getPtr();
  for (int64_t i = 0;  i < nSamplePoints * nz; i++, vval++) {
    *vval = missing;
  }
  
  // loop through the sample points to load up new vertical section
  
  fl32 *in = (fl32 *) workBuf.getPtr();
  fl32 *out = (fl32 *) interpBuf.getPtr();
  
  for (int64_t ii = 0; ii < nSamplePoints; ii++) {
    
    // compute the heights of the PPIs at this point
    
    double *ppiHt = new double[nElev];
    for (int ielev = 0; ielev < nElev; ielev++) {
      double slantRange = gndRange[ii] / cosElev[ielev];
      ppiHt[ielev] =
	sensorHt + (slantRange * sinElev[ielev] +
		    slantRange * slantRange / twiceRadius);
    } // ielev
    
    if (interp) {
      
      // interpolate between ppi heights
      
      for (int ielev = 1; ielev < nElev; ielev++) {
	int istart = (int) ((ppiHt[ielev - 1] - minz) / dz);
	if (istart < 0) {
	  istart = 0;
	}
	int iend = (int) ((ppiHt[ielev] - minz) / dz);
	if (iend > nz - 1) {
	  iend = nz - 1;
	}
	fl32 in1 = in[(ielev - 1) * nSamplePoints + ii];
	fl32 in2 = in[ielev * nSamplePoints + ii];
	double low = ppiHt[ielev - 1];
	double high = ppiHt[ielev];
	double delta = high - low;
	if (in1 != missing && in2 != missing) {
	  for (int jj = istart; jj <= iend; jj++) {
	    double ht = minz + jj * dz;
	    double wt = (ht - low) / delta;
	    out[jj * nSamplePoints + ii] =
	      (fl32) (in1 * (1.0 - wt) + in2 * wt);
	  }
	}
      } // ielev
      
    } else {
      
      // no interpolation

      for (int iz = 0; iz < nz; iz++) {
	
	double zz = minz + iz * dz;
	
	if (zz < (ppiHt[0] - dz) || zz > (ppiHt[nElev-1] + dz)) {
	  continue;
	}
	
	int MatchIelev = -1;
	double matchError = 1.0e9;
	for (int ielev = 0; ielev < nElev; ielev++) {
	  double error = fabs(zz - ppiHt[ielev]);
	  if (error < matchError) {
	    MatchIelev = ielev;
	    matchError = error;
	  }
	}
	
	if (MatchIelev >= 0) {
	  out[iz * nSamplePoints + ii] = in[MatchIelev * nSamplePoints + ii];
	  // out[iz * nSamplePoints + ii] = ppiHt[MatchIelev];
	}
	
      } // iz

    } // if (interp)

    delete[] ppiHt;

  } // ii

  // copy the interp buf across to the work buf

  workBuf = interpBuf;

  // set the vlevels accordingly

  _fhdr.vlevel_type = Mdvx::VERT_TYPE_Z;
  for (int iz = 0; iz < nz; iz++) {
    _vhdr.level[iz] = minz + iz * dz;
    _vhdr.type[iz] = Mdvx::VERT_TYPE_Z;
  }
  _fhdr.nz = nz;
  _fhdr.grid_minz = minz;
  _fhdr.grid_dz = dz;
  _fhdr.dz_constant = true;

  delete[] sinElev;
  delete[] cosElev;
  delete[] gndRange;

}


///////////////////////////////////////////////////////////////////////
// Remap onto selected projections.
//
// If the lookup table has not been initialized it is computed.
// If the projection geometry has changed the lookup table is recomputed.
//
// Returns 0 on success, -1 on failure.

// Remap onto a latlon projection.
// Returns 0 on success, -1 on failure.

int MdvxField::remap2Latlon(MdvxRemapLut &lut,
			    int nx, int ny,
			    double minx, double miny,
			    double dx, double dy)

{

  clearErrStr();

  // create target proj object

  Mdvx::coord_t coords;
  MEM_zero(coords);
  coords.proj_type = Mdvx::PROJ_LATLON;
  coords.nx = nx;
  coords.ny = ny;
  coords.minx = minx;
  coords.miny = miny;
  coords.dx = dx;
  coords.dy = dy;
  MdvxProj projTarget(coords);

  // do the remapping

  if (remap(lut, projTarget)) {
    _errStr += "ERROR - MdvxField::remap2Latlon\n";
    return -1;
  }

  return 0;
  
}

// Remap onto a flat projection.
// Returns 0 on success, -1 on failure.

int MdvxField::remap2Flat(MdvxRemapLut &lut,
			  int nx, int ny,
			  double minx, double miny,
			  double dx, double dy,
			  double origin_lat, double origin_lon,
			  double rotation,
                          double false_northing /* = 0.0 */,
                          double false_easting /* = 0.0 */)

{

  clearErrStr();

  // create target proj object

  Mdvx::coord_t coords;
  MEM_zero(coords);
  coords.proj_type = Mdvx::PROJ_FLAT;
  coords.nx = nx;
  coords.ny = ny;
  coords.nz = _fhdr.nz;
  coords.minx = minx;
  coords.miny = miny;
  coords.minz = _fhdr.grid_minz;
  coords.dx = dx;
  coords.dy = dy;
  coords.dz = _fhdr.grid_dz;
  coords.proj_origin_lat = origin_lat;
  coords.proj_origin_lon = origin_lon;
  coords.proj_params.flat.rotation = rotation;
  coords.false_northing = false_northing;
  coords.false_easting = false_easting;
  MdvxProj projTarget(coords);

  // do the remapping

  if (remap(lut, projTarget)) {
    _errStr += "ERROR - MdvxField::remap2Flat\n";
    return -1;
  }

  return 0;
  
}

// Remap onto a lambert conformal projection.
// Returns 0 on success, -1 on failure.

int MdvxField::remap2LambertConf(MdvxRemapLut &lut,
				 int nx, int ny,
				 double minx, double miny,
				 double dx, double dy,
				 double origin_lat, double origin_lon,
				 double lat1, double lat2,
                                 double false_northing /* = 0.0 */,
                                 double false_easting /* = 0.0 */)

{

  clearErrStr();

  // create target proj object

  Mdvx::coord_t coords;
  MEM_zero(coords);
  coords.proj_type = Mdvx::PROJ_LAMBERT_CONF;
  coords.nx = nx;
  coords.ny = ny;
  coords.minx = minx;
  coords.miny = miny;
  coords.dx = dx;
  coords.dy = dy;
  coords.proj_origin_lat = origin_lat;
  coords.proj_origin_lon = origin_lon;
  coords.proj_params.lc2.lat1 = lat1;
  coords.proj_params.lc2.lat2 = lat2;
  coords.false_northing = false_northing;
  coords.false_easting = false_easting;
  MdvxProj projTarget(coords);

  // do the remapping

  if (remap(lut, projTarget)) {
    _errStr += "ERROR - MdvxField::remap2LambertConf\n";
    return -1;
  }

  return 0;
  
}

// Remap onto a polar stereographic projection.
// Returns 0 on success, -1 on failure.

int MdvxField::remap2PolarStereo(MdvxRemapLut &lut,
				 int nx, int ny,
				 double minx, double miny,
				 double dx, double dy,
				 double origin_lat, double origin_lon,
				 double tangent_lon, 
				 Mdvx::pole_type_t poleType,
				 double central_scale,
                                 double false_northing /* = 0.0 */,
                                 double false_easting /* = 0.0 */,
				 double lad /* = 90.0 */)
  
{
  
  clearErrStr();

  // create target proj object

  Mdvx::coord_t coords;
  MEM_zero(coords);
  coords.proj_type = Mdvx::PROJ_POLAR_STEREO;
  coords.nx = nx;
  coords.ny = ny;
  coords.minx = minx;
  coords.miny = miny;
  coords.dx = dx;
  coords.dy = dy;
  coords.proj_origin_lon = tangent_lon;
  coords.proj_params.ps.tan_lon = tangent_lon;
  if (poleType == Mdvx::POLE_NORTH) {
    coords.proj_params.ps.pole_type = 0;
    coords.proj_origin_lat = 90;
  } else {
    coords.proj_params.ps.pole_type = 1;
    coords.proj_origin_lat = -90;
  }
  coords.proj_params.ps.central_scale = central_scale;
  coords.false_northing = false_northing;
  coords.false_easting = false_easting;
  coords.proj_params.ps.lad = lad;
  MdvxProj projTarget(coords);
  
  // do the remapping

  if (remap(lut, projTarget)) {
    _errStr += "ERROR - MdvxField::remap2PolarStereo\n";
    return -1;
  }

  return 0;
  
}

// Remap onto an oblique stereographic projection.
// Returns 0 on success, -1 on failure.

int MdvxField::remap2ObliqueStereo(MdvxRemapLut &lut,
				   int nx, int ny,
				   double minx, double miny,
				   double dx, double dy,
				   double origin_lat, double origin_lon,
				   double tangent_lat, double tangent_lon,
                                   double central_scale /* = 1.0 */,
                                   double false_northing /* = 0.0 */,
                                   double false_easting /* = 0.0 */)
  
{
  
  clearErrStr();

  // create target proj object

  Mdvx::coord_t coords;
  MEM_zero(coords);
  coords.proj_type = Mdvx::PROJ_OBLIQUE_STEREO;
  coords.nx = nx;
  coords.ny = ny;
  coords.minx = minx;
  coords.miny = miny;
  coords.dx = dx;
  coords.dy = dy;
  coords.proj_origin_lat = origin_lat;
  coords.proj_origin_lon = origin_lon;
  coords.proj_params.os.tan_lat = tangent_lat;
  coords.proj_params.os.tan_lon = tangent_lon;
  coords.proj_params.os.central_scale = central_scale;
  coords.false_northing = false_northing;
  coords.false_easting = false_easting;
  MdvxProj projTarget(coords);
  
  // do the remapping

  if (remap(lut, projTarget)) {
    _errStr += "ERROR - MdvxField::remap2ObliqueStereo\n";
    return -1;
  }

  return 0;
  
}

// Remap onto a MERCATOR projection.
// Returns 0 on success, -1 on failure.

int MdvxField::remap2Mercator(MdvxRemapLut &lut,
			      int nx, int ny,
			      double minx, double miny,
			      double dx, double dy,
			      double origin_lat, double origin_lon,
                              double false_northing /* = 0.0 */,
                              double false_easting /* = 0.0 */)
  
{
  
  clearErrStr();

  // create target proj object

  Mdvx::coord_t coords;
  MEM_zero(coords);
  coords.proj_type = Mdvx::PROJ_MERCATOR;
  coords.nx = nx;
  coords.ny = ny;
  coords.minx = minx;
  coords.miny = miny;
  coords.dx = dx;
  coords.dy = dy;
  coords.proj_origin_lat = origin_lat;
  coords.proj_origin_lon = origin_lon;
  coords.false_northing = false_northing;
  coords.false_easting = false_easting;
  MdvxProj projTarget(coords);
  
  // do the remapping

  if (remap(lut, projTarget)) {
    _errStr += "ERROR - MdvxField::remap2Mercator\n";
    return -1;
  }

  return 0;
  
}

// Remap onto a TRANSVERSE MERCATOR projection.
// Returns 0 on success, -1 on failure.

int MdvxField::remap2TransverseMercator(MdvxRemapLut &lut,
					int nx, int ny,
					double minx, double miny,
					double dx, double dy,
					double origin_lat, double origin_lon,
					double central_scale,
                                        double false_northing /* = 0.0 */,
                                        double false_easting /* = 0.0 */)
  
{
  
  clearErrStr();

  // create target proj object

  Mdvx::coord_t coords;
  MEM_zero(coords);
  coords.proj_type = Mdvx::PROJ_TRANS_MERCATOR;
  coords.nx = nx;
  coords.ny = ny;
  coords.minx = minx;
  coords.miny = miny;
  coords.dx = dx;
  coords.dy = dy;
  coords.proj_origin_lat = origin_lat;
  coords.proj_origin_lon = origin_lon;
  coords.proj_params.tmerc.central_scale = central_scale;
  coords.false_northing = false_northing;
  coords.false_easting = false_easting;
  MdvxProj projTarget(coords);
  
  // do the remapping

  if (remap(lut, projTarget)) {
    _errStr += "ERROR - MdvxField::remap2TransverseMercator\n";
    return -1;
  }

  return 0;
  
}

// Remap onto a ALBERS EQUAL AREA CONIC projection.
// Returns 0 on success, -1 on failure.

int MdvxField::remap2Albers(MdvxRemapLut &lut,
			    int nx, int ny,
			    double minx, double miny,
			    double dx, double dy,
			    double origin_lat, double origin_lon,
			    double lat1, double lat2,
                            double false_northing /* = 0.0 */,
                            double false_easting /* = 0.0 */)

{

  clearErrStr();

  // create target proj object

  Mdvx::coord_t coords;
  MEM_zero(coords);
  coords.proj_type = Mdvx::PROJ_ALBERS;
  coords.nx = nx;
  coords.ny = ny;
  coords.minx = minx;
  coords.miny = miny;
  coords.dx = dx;
  coords.dy = dy;
  coords.proj_origin_lat = origin_lat;
  coords.proj_origin_lon = origin_lon;
  coords.proj_params.albers.lat1 = lat1;
  coords.proj_params.albers.lat2 = lat2;
  coords.false_northing = false_northing;
  coords.false_easting = false_easting;
  MdvxProj projTarget(coords);

  // do the remapping

  if (remap(lut, projTarget)) {
    _errStr += "ERROR - MdvxField::remap2Albers\n";
    return -1;
  }

  return 0;
  
}

// Remap onto a, LAMBERT AZIMUTHAL EQUAL AREA projection.
// Returns 0 on success, -1 on failure.

int MdvxField::remap2LambertAzimuthal(MdvxRemapLut &lut,
				      int nx, int ny,
				      double minx, double miny,
				      double dx, double dy,
				      double origin_lat, double origin_lon,
                                      double false_northing /* = 0.0 */,
                                      double false_easting /* = 0.0 */)
  
{
  
  clearErrStr();

  // create target proj object

  Mdvx::coord_t coords;
  MEM_zero(coords);
  coords.proj_type = Mdvx::PROJ_LAMBERT_AZIM;
  coords.nx = nx;
  coords.ny = ny;
  coords.minx = minx;
  coords.miny = miny;
  coords.dx = dx;
  coords.dy = dy;
  coords.proj_origin_lat = origin_lat;
  coords.proj_origin_lon = origin_lon;
  coords.false_northing = false_northing;
  coords.false_easting = false_easting;
  MdvxProj projTarget(coords);
  
  // do the remapping

  if (remap(lut, projTarget)) {
    _errStr += "ERROR - MdvxField::remap2LambertAzimuthal\n";
    return -1;
  }

  return 0;
  
}

// Remap onto a VERT_PERSP (satellite view) projection.
// Returns 0 on success, -1 on failure.

int MdvxField::remap2VertPersp(MdvxRemapLut &lut,
                               int nx, int ny,
                               double minx, double miny,
                               double dx, double dy,
                               double origin_lat, double origin_lon,
                               double persp_radius,
                               double false_northing /* = 0.0 */,
                               double false_easting /* = 0.0 */)
  
{

  clearErrStr();

  // create target proj object

  Mdvx::coord_t coords;
  MEM_zero(coords);
  coords.proj_type = Mdvx::PROJ_VERT_PERSP;
  coords.nx = nx;
  coords.ny = ny;
  coords.minx = minx;
  coords.miny = miny;
  coords.dx = dx;
  coords.dy = dy;
  coords.proj_origin_lat = origin_lat;
  coords.proj_origin_lon = origin_lon;
  coords.proj_params.vp.persp_radius = persp_radius;
  coords.false_northing = false_northing;
  coords.false_easting = false_easting;
  MdvxProj projTarget(coords);
  
  // do the remapping

  if (remap(lut, projTarget)) {
    _errStr += "ERROR - MdvxField::remap2VertPersp\n";
    return -1;
  }

  return 0;
  
}

///////////////////////////////////////////////////////////////////////
// Auto remap onto a latlon projection, without specifying
// grid information.
//
// The function will determine grid information from the
// projection information of the existing grid.
//
// If the lookup table has not been initialized it is computed.
// If the projection geometry has changed the lookup table is recomputed.
//
// Returns 0 on success, -1 on failure.

int MdvxField::autoRemap2Latlon(MdvxRemapLut &lut)

{

  clearErrStr();

  // trivial case

  if (_fhdr.proj_type == Mdvx::PROJ_LATLON) {
    return 0;
  }

  // get current projection

  MdvxProj projCurrent(_fhdr);
  projCurrent.setConditionLon2Origin(true);

  double originLat = _fhdr.proj_origin_lat;
  double originLon = _fhdr.proj_origin_lon;

  // compute the latlon bounding box

  double minLat = originLat;
  double minLon = originLon;

  double maxLat = originLat;
  double maxLon = originLon;

  double minx = _fhdr.grid_minx;
  double miny = _fhdr.grid_miny;
  double maxx = minx + _fhdr.nx * _fhdr.grid_dx;
  double maxy = miny + _fhdr.ny * _fhdr.grid_dy;

  // SW corner

  double lat, lon;
  projCurrent.xy2latlon(minx, miny, lat, lon);
  minLat = MIN(minLat, lat);
  minLon = MIN(minLon, lon);
  maxLat = MAX(maxLat, lat);
  maxLon = MAX(maxLon, lon);

  // NW corner
  
  projCurrent.xy2latlon(minx, maxy, lat, lon);
  minLat = MIN(minLat, lat);
  minLon = MIN(minLon, lon);
  maxLat = MAX(maxLat, lat);
  maxLon = MAX(maxLon, lon);

  // NE corner
  
  projCurrent.xy2latlon(maxx, maxy, lat, lon);
  minLat = MIN(minLat, lat);
  minLon = MIN(minLon, lon);
  maxLat = MAX(maxLat, lat);
  maxLon = MAX(maxLon, lon);

  // SE corner
  
  projCurrent.xy2latlon(maxx, miny, lat, lon);
  minLat = MIN(minLat, lat);
  minLon = MIN(minLon, lon);
  maxLat = MAX(maxLat, lat);
  maxLon = MAX(maxLon, lon);

  // W edge
  
  projCurrent.xy2latlon(minx, (miny + maxy) / 2.0, lat, lon);
  minLat = MIN(minLat, lat);
  minLon = MIN(minLon, lon);
  maxLat = MAX(maxLat, lat);
  maxLon = MAX(maxLon, lon);

  // E edge
  
  projCurrent.xy2latlon(maxx, (miny + maxy) / 2.0, lat, lon);
  minLat = MIN(minLat, lat);
  minLon = MIN(minLon, lon);
  maxLat = MAX(maxLat, lat);
  maxLon = MAX(maxLon, lon);

  // N edge
  
  projCurrent.xy2latlon((minx + maxx) / 2.0, maxy, lat, lon);
  minLat = MIN(minLat, lat);
  minLon = MIN(minLon, lon);
  maxLat = MAX(maxLat, lat);
  maxLon = MAX(maxLon, lon);

  // S edge
  
  projCurrent.xy2latlon((minx + maxx) / 2.0, miny, lat, lon);
  minLat = MIN(minLat, lat);
  minLon = MIN(minLon, lon);
  maxLat = MAX(maxLat, lat);
  maxLon = MAX(maxLon, lon);

  // compute dlat and dlon

  double dLat = _fhdr.grid_dy / KM_PER_DEG_AT_EQ;
  double dLon = (_fhdr.grid_dx / KM_PER_DEG_AT_EQ) / cos(maxLat * DEG_TO_RAD);
  if (dLon < (dLat * 0.25)) {
    dLon = dLat * 0.25;
  }

  // set oversampling ratio

  dLat *= 0.75;
  dLon *= 0.75;

  // compute nx and ny
  
  int nLon = (int) ((maxLon - minLon) / dLon + 1.0);
  int nLat = (int) ((maxLat - minLat) / dLat + 1.0);

  // do the remapping

  return remap2Latlon(lut, nLon, nLat, minLon, minLat, dLon, dLat);

}

///////////////////////////////////////////////////////////////////////
// Remap onto specified projection.
//
// If the lookup table has not been initialized it is computed.
// If the projection geometry has changed the lookup table is recomputed.
//
// Returns 0 on success, -1 on failure.

int MdvxField::remap(MdvxRemapLut &lut,
		     MdvxProj &proj_target)
  
{

  // uncompress as required
  
  int compression_type = _fhdr.compression_type;
  if (decompress()) {
    _errStr += "ERROR - MdvxField::remap\n";
    return -1;
  }

  // create source proj object
  
  MdvxProj projSource(_fhdr);
  int64_t nPointsSourcePlane = _fhdr.nx * _fhdr.ny;
  int64_t nBytesSourcePlane = nPointsSourcePlane * _fhdr.data_element_nbytes;

  // compute lookup table - this will only recompute if the source or
  // target projection has changed.
  
  lut.computeOffsets(projSource, proj_target);

  // set up working buffer

  MemBuf workBuf;
  const Mdvx::coord_t &targetCoords = proj_target.getCoord();
  int64_t nPointsTargetPlane = targetCoords.nx * targetCoords.ny;
  int64_t nPointsTargetVol = nPointsTargetPlane * _fhdr.nz;
  int64_t nBytesTargetPlane = nPointsTargetPlane * _fhdr.data_element_nbytes;
  int64_t nBytesTargetVol = nBytesTargetPlane * _fhdr.nz;
  workBuf.prepare(nBytesTargetVol);

  if ( workBuf.getPtr() == NULL) {
    _errStr += "ERROR - MdvxField::remap\n";
    return -1;
  }

  // zero out the work buffer

  switch (_fhdr.encoding_type) {
    
    case Mdvx::ENCODING_INT8: {
      ui08 missing = (ui08) _fhdr.missing_data_value;
      ui08 *vval = (ui08 *) workBuf.getPtr();
      for (int64_t i = 0;  i < nPointsTargetVol; i++, vval++) {
        *vval = missing;
      }
      break;
    }

    case Mdvx::ENCODING_INT16: {
      ui16 missing = (ui16) _fhdr.missing_data_value;
      ui16 *vval = (ui16 *) workBuf.getPtr();
      for (int64_t i = 0;  i < nPointsTargetVol; i++, vval++) {
        *vval = missing;
      }
      break;
    }

    case Mdvx::ENCODING_FLOAT32: {
      fl32 missing = (fl32) _fhdr.missing_data_value;
      fl32 *vval = (fl32 *) workBuf.getPtr();
      for (int64_t i = 0;  i < nPointsTargetVol; i++, vval++) {
        *vval = missing;
      }
      break;
    }

    case Mdvx::ENCODING_RGBA32: {
      ui32 missing = (ui32) _fhdr.missing_data_value;
      ui32 *vval = (ui32 *) workBuf.getPtr();
      for (int64_t i = 0;  i < nPointsTargetVol; i++, vval++) {
        *vval = missing;
      }
      break;
    }

  } // switch (_fhdr.encoding_type)

  // remap into the work buffer
  
  ui08 *source = (ui08 *) _volBuf.getPtr();
  ui08 *target = (ui08 *) workBuf.getPtr();

  int64_t nOffsets = lut.getNOffsets();
  const int64_t *sourceOffsets = lut.getSourceOffsets();
  const int64_t *targetOffsets = lut.getTargetOffsets();
  
  for (int64_t i = 0; i < nOffsets; i++, sourceOffsets++, targetOffsets++) {
    
    int64_t soff = *sourceOffsets * _fhdr.data_element_nbytes;
    int64_t toff = *targetOffsets * _fhdr.data_element_nbytes;
    
    for (int64_t iz = 0; iz < _fhdr.nz;
	 iz++, soff += nBytesSourcePlane, toff += nBytesTargetPlane) {
      memcpy(target + toff, source + soff, _fhdr.data_element_nbytes);
    }

  } // i
  
  // set the field header appropriately

  proj_target.syncXyToFieldHdr(_fhdr);

  // copy the work buf to the vol buf

  _volBuf = workBuf;

  // set compression

  requestCompression(compression_type);

  return 0;

}


///////////////////////////////////////////////////////////////////////
// Remap to specified type, using the existing grid parameters to
// estimate the best target grid.
//
// NOTE - NOT IMPLEMENTED YET.
//
// If the lookup table has not been initialized it is computed.
// If the projection geometry has changed the lookup table is recomputed.
//
// Returns 0 on success, -1 on failure.

int MdvxField::remap(MdvxRemapLut &lut,
		     Mdvx::projection_type_t proj_type)

{
  return -1;
}

///////////////////////////////////////////////////////////////////////
// Decimate to max grid cell count
//
// Returns 0 on success, -1 on failure.

int MdvxField::decimate(int64_t max_nxy)
  
{

  // for polar data, use special method

  if (_fhdr.proj_type == Mdvx::PROJ_POLAR_RADAR) {
    return _decimate_radar_horiz(max_nxy);
  }

  // Use a pixel averaging technique
  if(_fhdr.encoding_type == Mdvx::ENCODING_RGBA32) {
    return _decimate_rgba(max_nxy);
  }

  // check if we need to decimate
  
  double full_nxy = _fhdr.nx * _fhdr.ny;
  if (max_nxy <= 0 || full_nxy < max_nxy) {
    return 0;
  }
  int64_t sparsity = (int64_t) (sqrt(full_nxy / (double) max_nxy) + 1);
  if (sparsity < 2) {
    return 0;
  }
  
  // uncompress as required
  
  int compression_type = _fhdr.compression_type;
  bool recompress = false;
  if (isCompressed()) {
    if (decompress()) {
      _errStr += "ERROR - MdvxField::decimate\n";
      return -1;
    }
    recompress = true;
  }

  // compute sizes

  int elSize = _fhdr.data_element_nbytes;
  int jumpSize = sparsity * elSize;
  
  int64_t startPos = (sparsity - 1) / 2;
  int outNx = (_fhdr.nx - startPos - 1) / sparsity + 1;
  int outNy = (_fhdr.ny - startPos - 1) / sparsity + 1;

  int64_t nBytesSourceRow = _fhdr.nx * elSize;
  int64_t nBytesSourcePlane = nBytesSourceRow * _fhdr.ny;
  
  int64_t nBytesTargetRow = outNx * elSize;
  int64_t nBytesTargetPlane = nBytesTargetRow * outNy;
  int64_t nBytesTargetVol = nBytesTargetPlane * _fhdr.nz;

  // set up working buffer
  
  MemBuf targetBuf;
  targetBuf.prepare(nBytesTargetVol);
  
  // loop through z, decimating into target buffer
  
  ui08 *sourceVol = (ui08 *) _volBuf.getPtr();
  ui08 *targetVol = (ui08 *) targetBuf.getPtr();
  
  for (int iz = 0; iz < _fhdr.nz; iz++) {
    
    ui08 *sourcePlane = sourceVol + iz * nBytesSourcePlane;
    ui08 *targetPlane = targetVol + iz * nBytesTargetPlane;
    
    // loop through y

    for (int iny = startPos, outy = 0; outy < outNy;
	 iny += sparsity, outy++) {
      
      ui08 *source = sourcePlane + iny * nBytesSourceRow + startPos * elSize;
      ui08 *target = targetPlane + outy * nBytesTargetRow;

      // loop through x

      for (int outx = 0; outx < outNx;
	   outx++, source += jumpSize, target += elSize) {
	memcpy(target, source, elSize);
      } // outx
      
    } // outy

  } // iz
  
  // set the field header appropriately
  
  _fhdr.volume_size = nBytesTargetVol;
  _fhdr.nx = outNx;
  _fhdr.ny = outNy;
  _fhdr.grid_minx += startPos * _fhdr.grid_dx;
  _fhdr.grid_miny += startPos * _fhdr.grid_dy;
  _fhdr.grid_dx *= sparsity;
  _fhdr.grid_dy *= sparsity;

  // copy the target buf to the vol buf

  _volBuf = targetBuf;

  if (recompress) {
    requestCompression(compression_type);
  }

  return 0;

}

//////////////////////////////////////////////////
// compute the plane number from the vlevel value
//

int MdvxField::computePlaneNum(double vlevel)

{

  int planeNum = 0;

  // check for data order
  bool ascending;
  if (_vhdr.level[0] <= _vhdr.level[_fhdr.nz - 1]) {
    ascending = true;
  } else {
    ascending = false;
  }
  
  // check for limiting conditions
  
  if (ascending) { // ascending order
    if (vlevel < _vhdr.level[0])  return (0); 
    if (vlevel > _vhdr.level[_fhdr.nz - 1]) return (_fhdr.nz - 1);
    
  } else { // descending order
    if (vlevel > _vhdr.level[0])  return (0); 
    if (vlevel < _vhdr.level[_fhdr.nz - 1])  return (_fhdr.nz - 1); 
  }
  
  // get plane num
  double minDiff = 1.0e99;
  for (int i = 0; i < _fhdr.nz; i++) {
    double diff = fabs(vlevel - _vhdr.level[i]);
    if (diff < minDiff) {
      planeNum = i;
      minDiff = diff;
    } 
  } // i


  return (planeNum);
}

///////////////////////////////////////////////////////
// compute the plane limits for a pair of vlevel values
//

void MdvxField::computePlaneLimits(double vlevel1,
				   double vlevel2,
				   int &lower_plane,
				   int &upper_plane)
  
{

  // get plane nums

  int p1 = computePlaneNum(vlevel1);
  int p2 = computePlaneNum(vlevel2);

  // if they are the same, we are done

  if (p1 == p2) {
    lower_plane = p1;
    upper_plane = p2;
    return;
  }

  // reorder as necessary

  if (p1 < p2 ) {
    lower_plane = p1;
    upper_plane = p2;
  } else {
    lower_plane = p2;
    upper_plane = p1;
  }

  // sanity check on upper_plane and lower_plane values insures no out-of-bounds array access 
  if (lower_plane < 0)
    lower_plane = 0;
  if (upper_plane < 0)
    upper_plane = 0;
 
  
  // check for data order
  
  bool ascending;
  if (_vhdr.level[0] <= _vhdr.level[_fhdr.nz - 1]) {
    ascending = true;
  } else {
    ascending = false;
  }
  double vmin = MIN(vlevel1, vlevel2);
  double vmax = MAX(vlevel1, vlevel2);

  // check that we have not included unwanted planes

  if (ascending) {

    // ascending order

    if (_vhdr.level[lower_plane] < vmin) {
      lower_plane++;
    }
    if (_vhdr.level[upper_plane] > vmax) {
      upper_plane--;
    }

  } else {

    // descending order

    if (_vhdr.level[lower_plane] > vmax) {
      lower_plane++;
    }
    if (_vhdr.level[upper_plane] < vmin) {
      upper_plane--;
    }

  }

  return;
  
}

////////////////////////////////////////////
// Convert data buffer from BE byte ordering

void MdvxField::buffer_from_BE(void *buf, int64_t buflen,
			       int encoding_type)
     
{

  switch (encoding_type) {
    case Mdvx::ENCODING_INT8:
      // no need to swap byte data
      return;
      break;
    case Mdvx::ENCODING_INT16:
      BE_to_array_16(buf, buflen);
      break;
    case Mdvx::ENCODING_FLOAT32:
      BE_to_array_32(buf, buflen);
      break;
    case Mdvx::ENCODING_RGBA32:
      break;
  }
}

////////////////////////////////////////////
// Convert data buffer to BE byte ordering

void MdvxField::buffer_to_BE(void *buf, int64_t buflen,
			     int encoding_type)
  
{

  switch (encoding_type) {
    case Mdvx::ENCODING_INT8:
      // no need to swap byte data
      return;
      break;
    case Mdvx::ENCODING_INT16:
      BE_from_array_16(buf, buflen);
      break;
    case Mdvx::ENCODING_FLOAT32:
      BE_from_array_32(buf, buflen);
      break;
    case Mdvx::ENCODING_RGBA32:
      break;
  }
}

////////////////////////////////////////////
//
// Convert volume buffer from BE byte ordering

void MdvxField::_data_from_BE(const Mdvx::field_header_t &fhdr,
                              void *buf,
                              int64_t buflen)

{
  
  if (isCompressed(fhdr)) {
    // compressed data is left in BE
    return;
  }

  buffer_from_BE(buf, buflen, fhdr.encoding_type);

}

////////////////////////////////////////////////////////////////////////////
//
// Print the contents of the MdvxField object headers
//
// print_file_headers: if true and the file headers exist, print them.

void MdvxField::printHeaders(ostream &out,
			     bool print_file_headers /* = false*/ ) const
  
{
  
  Mdvx::printFieldHeader(_fhdr, out);
  Mdvx::printVlevelHeader(_vhdr, _fhdr.nz, _fhdr.field_name, out);

  if (print_file_headers) {
    if (_fhdrFile != NULL) {
      out << "======================================================" << endl;
      out << "   Field header as in file" << endl;
      out << "======================================================" << endl;
      Mdvx::printFieldHeader(*_fhdrFile, out);
    }
    
    if (_vhdrFile != NULL && _fhdrFile != NULL) {
      out << "======================================================" << endl;
      out << "   Vlevel header as in file" << endl;
      out << "======================================================" << endl;
      Mdvx::printVlevelHeader(*_vhdrFile, _fhdrFile->nz,
			      _fhdrFile->field_name, out);
    }

  }

}

////////////////////////////////////////////////////////////////////////////
//
// Print the contents of the MdvxField object
//
// print_native: if true, type is preserved.
//               if false, printed as floats.
//
// print_labels: if true, a label will be printed for each plane.
//
// pack_duplicates: if true, duplicates will be printed once with a 
//                           repeat count.
//                  if false, all values will be printed.
//
// print_file_headers: if true and the file headers exist, print them.
//
// n_lines_data: number of data lines to print. If -1, print all.

void MdvxField::print(ostream &out,
		      bool print_native /* = true */,
		      bool print_labels /* = true */,
		      bool pack_duplicates /* = true */,
		      bool print_file_headers /* = false */,
		      int n_lines_data /* = -1 */ ) const
  
{
  
  printHeaders(out, print_file_headers);
  printVoldata(out, print_native, print_labels, pack_duplicates, false, n_lines_data);

}

////////////////////////////////////////////////////////////////////////////
//
// Print the volume data part of MdvxField object.
//
// print_native: if true, type is preserved.
//               if false, printed as floats.
//
// print_labels: if true, a label will be printed for each plane.
//
// pack_duplicates: if true, duplicates will be printed once with a 
//                           repeat count.
//                  if false, all values will be printed.
//
// printCanonical: print in full resolution
//
// n_lines_data: number of data lines to print. If -1, print all.

void MdvxField::printVoldata(ostream &out,
			     bool print_native /* = true */,
			     bool print_labels /* = true */,
			     bool pack_duplicates /* = true */,
                             bool printCanonical /* = false */,
                             int n_lines_data /* = -1 */ ) const
     
{

  // make local copy of object

  MdvxField copy(*this);
  
  // uncompress if needed

  if (copy.isCompressed()) {
    if (copy.decompress()) {
      out << "ERROR - MdvxField::printVoldata" << endl;
      out << "  Cannot decompress field, name: " << getFieldName() << endl;
      out << _errStr << endl;
      return;
    }
  }

  // convert if needed

  if (!print_native) {
    copy.convertType(Mdvx::ENCODING_FLOAT32);
  }

  if (pack_duplicates) {
    copy._print_voldata_packed(out, print_labels, printCanonical, n_lines_data);
  } else {
    copy._print_voldata_verbose(out, print_labels);
  }

}

////////////////////////////////////////////////////////////////////////////
//
// Print time-height profile data

void MdvxField::printTimeHeightData(ostream &out,
				    const vector<time_t> &times,
				    bool print_native /* = true*/)
  
{
  
  // make local copy of object

  MdvxField copy(*this);
  
  // uncompress if needed

  if (copy.isCompressed()) {
    if (copy.decompress()) {
      out << "ERROR - MdvxField::printTimeHeightData" << endl;
      out << "  Cannot decompress field, name: " << getFieldName() << endl;
      out << _errStr << endl;
      return;
    }
  }

  // convert if needed

  if (!print_native) {
    copy.convertType(Mdvx::ENCODING_FLOAT32);
  }
  
  copy._print_time_height(out, times);

}

///////////////////
// set() functions

// setting the entire headers

void MdvxField::setFieldHeader(const Mdvx::field_header_t &fhdr)
{
  _fhdr = fhdr;
}

void MdvxField::setVlevelHeader(const Mdvx::vlevel_header_t &vhdr)
{
  _vhdr = vhdr;
}

// setting the entire headers - file version

void MdvxField::setFieldHeaderFile(const Mdvx::field_header_t &fhdr)
{
  if (_fhdrFile) delete _fhdrFile;
  _fhdrFile = new Mdvx::field_header_t(fhdr);
}

void MdvxField::setVlevelHeaderFile(const Mdvx::vlevel_header_t &vhdr)
{
  if (_vhdrFile) delete _vhdrFile;
  _vhdrFile = new Mdvx::vlevel_header_t(vhdr);
}

// setting individual parts of the header

void MdvxField::setFieldName(const string &name)
{
  STRncopy(_fhdr.field_name, name.c_str(), MDV_SHORT_FIELD_LEN );
}

void MdvxField::setFieldNameLong(const string &nameLong)
{
  STRncopy(_fhdr.field_name_long, nameLong.c_str(), MDV_LONG_FIELD_LEN );
}

void MdvxField::setUnits(const string &units)
{
  STRncopy(_fhdr.units, units.c_str(), MDV_UNITS_LEN );
}

void MdvxField::setTransform(const string &transform)
{
  STRncopy(_fhdr.transform, transform.c_str(), MDV_TRANSFORM_LEN );
}

void MdvxField::setFieldName(const char *name)
{
  STRncopy(_fhdr.field_name, name, MDV_SHORT_FIELD_LEN );
}

void MdvxField::setFieldNameLong(const char *nameLong)
{
  STRncopy(_fhdr.field_name_long, nameLong, MDV_LONG_FIELD_LEN );
}

void MdvxField::setUnits(const char *units)
{
  STRncopy(_fhdr.units, units, MDV_UNITS_LEN );
}

void MdvxField::setTransform(const char *transform)
{
  STRncopy(_fhdr.transform, transform, MDV_TRANSFORM_LEN );
}

void MdvxField::setFieldName(const string &name,
                             Mdvx::field_header_t &fhdr)
{
  STRncopy(fhdr.field_name, name.c_str(), MDV_SHORT_FIELD_LEN );
}

void MdvxField::setFieldNameLong(const string &nameLong,
                                 Mdvx::field_header_t &fhdr)
{
  STRncopy(fhdr.field_name_long, nameLong.c_str(), MDV_LONG_FIELD_LEN );
}

void MdvxField::setUnits(const string &units, Mdvx::field_header_t &fhdr)
{
  STRncopy(fhdr.units, units.c_str(), MDV_UNITS_LEN );
}

void MdvxField::setTransform(const string &transform,
                             Mdvx::field_header_t &fhdr)
{
  STRncopy(fhdr.transform, transform.c_str(), MDV_TRANSFORM_LEN );
}

///////////////////////
// protected functions


///////////////////////////////////////
// Set the plane pointers into the data
//
// This must be called before using the following plane functions:
//    getPlane();
//    getPlaneSize()
//    getPlaneOffset()

void MdvxField::setPlanePtrs() const

{

  if (!_volbufSizeValid()) {
    // no op
    cerr << "WARNING - MdvxField::setPlanePtrs()" << endl;
    cerr << "  Data buffer not set, field: " << getFieldName() << endl;
    return;
  }
  
  int64_t nz = _fhdr.nz;

  _planeData.erase(_planeData.begin(), _planeData.end());
  _planeSizes.erase(_planeSizes.begin(), _planeSizes.end());
  _planeOffsets.erase(_planeOffsets.begin(), _planeOffsets.end());
  _planeData.reserve(nz);
  _planeSizes.reserve(nz);
  _planeOffsets.reserve(nz);

  if (!isCompressed()) {

    // not compressed

    int64_t size = _fhdr.nx * _fhdr.ny * _fhdr.data_element_nbytes;

    for (int64_t i = 0; i < nz; i++) {
      int64_t offset = i * size;
      _planeSizes[i] = size;
      _planeOffsets[i] = offset;
      _planeData[i] = (void *) ((ui08 *) _volBuf.getPtr() + _planeOffsets[i]);
    }
    
  } else {

    si32* be_offsets = (si32*) _volBuf.getPtr();
    si32* be_sizes = be_offsets + nz;

    for (int i = 0; i < nz; i++) {
      _planeSizes[i] = BE_to_si32(be_sizes[i]);
      _planeOffsets[i] = BE_to_si32(be_offsets[i]);
      _planeData[i] = (void *) ((ui08 *) _volBuf.getPtr() + _planeOffsets[i]);
    }

  } // if (!isCompressed())

}

////////////////////////////
// INT8 to INT16 conversion

void MdvxField::_int8_to_int16(int output_scaling,
			       double output_scale,
			       double output_bias)
     
{

  _int8_to_float32();
  if (output_scaling == Mdvx::SCALING_SPECIFIED) {
    _float32_to_int16(output_scale, output_bias);
  } else {
    _float32_to_int16(output_scaling);
  }

}

///////////////////////////
// INT16 to INT8 conversion

void MdvxField::_int16_to_int8(int output_scaling,
			       double output_scale,
			       double output_bias)
     
{

  _int16_to_float32();
  if (output_scaling == Mdvx::SCALING_SPECIFIED) {
    _float32_to_int8(output_scale, output_bias);
  } else {
    _float32_to_int8(output_scaling);
  }

}

//////////////////////////////
// INT8 to FLOAT32 conversion

void MdvxField::_int8_to_float32()
     
{

  // copy the volume buffer

  MemBuf copyBuf(_volBuf);

  // allocate the output buffer

  int64_t npoints = _fhdr.nx * _fhdr.ny * _fhdr.nz;
  int64_t output_size = npoints * sizeof(fl32);
  _volBuf.prepare(output_size);
  
  // convert data
  
  ui08 *in = (ui08 *) copyBuf.getPtr();
  fl32 *out = (fl32 *) _volBuf.getPtr();
  fl32 scale = _fhdr.scale;
  fl32 bias = _fhdr.bias;

  double threshold_val;
  if (_fhdr.min_value == _fhdr.max_value) {
    threshold_val = 0.0;
  } else {
    threshold_val = fabs(scale * 0.05);
  }

  ui08 byte_missing_val = (ui08) _fhdr.missing_data_value;
  ui08 byte_bad_val = (ui08) _fhdr.bad_data_value;

  fl32 float_missing_val = _fhdr.missing_data_value * scale + bias;
  fl32 float_bad_val = _fhdr.bad_data_value * scale + bias;
  
  for (int64_t i = 0; i < npoints; i++, in++, out++) {
    if (*in == byte_missing_val) {
      *out = float_missing_val;
    } else if (*in == byte_bad_val) {
      *out = float_bad_val;
    } else {
      fl32 outval = ((fl32) *in * scale + bias);
      if (fabs(outval) < threshold_val) {
	*out = 0.0;
      } else {
	*out = outval;
      }
    }
  }

  // adjust header

  _fhdr.volume_size = output_size;
  _fhdr.encoding_type = Mdvx::ENCODING_FLOAT32;
  _fhdr.scaling_type = Mdvx::SCALING_NONE;
  _fhdr.data_element_nbytes = 4;
  _fhdr.missing_data_value = float_missing_val;
  _fhdr.bad_data_value = float_bad_val;
  _fhdr.scale = 1.0;
  _fhdr.bias = 0.0;
  
}

//////////////////////////////
// INT16 to FLOAT32 conversion

void MdvxField::_int16_to_float32()

{

  // copy the volume buffer

  MemBuf copyBuf(_volBuf);

  // allocate the output buffer

  int64_t npoints = _fhdr.nx * _fhdr.ny * _fhdr.nz;
  int64_t output_size = npoints * sizeof(fl32);
  _volBuf.prepare(output_size);
  
  // convert data
  
  ui16 *in = (ui16 *) copyBuf.getPtr();
  fl32 *out = (fl32 *) _volBuf.getPtr();
  fl32 scale = _fhdr.scale;
  fl32 bias = _fhdr.bias;

  double threshold_val;
  if (_fhdr.min_value == _fhdr.max_value) {
    threshold_val = 0.0;
  } else {
    threshold_val = fabs(scale * 0.05);
  }

  ui16 short_missing_val = (ui16) _fhdr.missing_data_value;
  ui16 short_bad_val = (ui16) _fhdr.bad_data_value;

  fl32 float_missing_val = _fhdr.missing_data_value * scale + bias;
  fl32 float_bad_val = _fhdr.bad_data_value * scale + bias;
  
  for (int64_t i = 0; i < npoints; i++, in++, out++) {
    if (*in == short_missing_val) {
      *out = float_missing_val;
    } else if (*in == short_bad_val) {
      *out = float_bad_val;
    } else {
      fl32 outval = ((fl32) *in * scale + bias);
      if (fabs(outval) < threshold_val) {
	*out = 0.0;
      } else {
	*out = outval;
      }
    }
  }

  // adjust header
  
  _fhdr.volume_size = output_size;
  _fhdr.encoding_type = Mdvx::ENCODING_FLOAT32;
  _fhdr.scaling_type = Mdvx::SCALING_NONE;
  _fhdr.data_element_nbytes = 4;
  _fhdr.missing_data_value =
    _fhdr.missing_data_value * scale + bias;
  _fhdr.bad_data_value =
    _fhdr.bad_data_value * scale + bias;
  _fhdr.scale = 1.0;
  _fhdr.bias = 0.0;
  
}

//////////////////////////////
// FLOAT32 to INT8 conversion
//
// Rounded or dynamic

void MdvxField::_float32_to_int8(int output_scaling)

{

  // set missing and bad

  fl32 in_missing = _fhdr.missing_data_value;
  fl32 in_bad =  _fhdr.bad_data_value;
  ui08 out_missing = 0;
  ui08 out_bad;
  if (in_missing == in_bad) {
    out_bad = out_missing;
  } else {
    out_bad = 1;
  }

  // compute scale and bias
  
  double scale, bias;

  if (_fhdr.max_value == _fhdr.min_value) {
    
    // scale = 1.0;
    // bias = _fhdr.min_value - 4.0;

    if (_fhdr.max_value == 0.0) {
      scale = 1.0;
    } else {
      scale = fabs(_fhdr.max_value);
    }
    bias = _fhdr.min_value - 4.0 * scale;
    
  } else {
    
    double range = _fhdr.max_value - _fhdr.min_value;
    scale = range / 250.0;
    
    if (output_scaling == Mdvx::SCALING_ROUNDED) {
      scale = _round_up(scale);
      bias = _fhdr.min_value - scale * 4.0;
      bias = floor(bias / scale) * scale;
    } else if (output_scaling == Mdvx::SCALING_INTEGRAL) {
      scale = floor (scale + 1.0);
      bias = _fhdr.min_value - scale * 4.0;
      bias = floor(bias / scale) * scale;
    } else {
      bias = _fhdr.min_value - scale * 4.0;
    }
    
  }
  
  // copy the volume buffer

  MemBuf copyBuf(_volBuf);

  // allocate the output buffer

  int64_t npoints = _fhdr.nx * _fhdr.ny * _fhdr.nz;
  int64_t output_size = npoints * sizeof(ui08);
  _volBuf.prepare(output_size);

  // convert data
  
  fl32 *in = (fl32 *) copyBuf.getPtr();
  ui08 *out = (ui08 *) _volBuf.getPtr();
  int64_t nBad = 0;

  for (int64_t i = 0; i < npoints; i++, in++, out++) {
    fl32 in_val = *in;
    if (in_val == in_missing) {
      *out = out_missing;
    } else if (in_val == in_bad) {
      *out = out_bad;
    } else {
      int out_val = (int) ((in_val - bias) / scale + 0.49999);
      if (out_val > 255) {
        nBad++;
 	*out = 255;
      } else if (out_val < 3) {
        nBad++;
 	*out = 3;
      } else {
	*out = (ui08) out_val;
      }
    }
  } // i */

  if (nBad > 0) {
    cerr << "ERROR - MdvxField::_float32_to_int8" << endl;
    cerr << "  Out of range data found, field: " << getFieldName() << endl;
    cerr << "  n points: " << nBad << endl;
    cerr << "  Replaced with min or max values as appropriate" << endl;
  }

  // adjust header
  
  _fhdr.volume_size = output_size;
  _fhdr.encoding_type = Mdvx::ENCODING_INT8;
  _fhdr.scaling_type = output_scaling;
  _fhdr.data_element_nbytes = 1;
  _fhdr.missing_data_value = out_missing;
  _fhdr.bad_data_value = out_bad;
  _fhdr.scale = (fl32) scale;
  _fhdr.bias = (fl32) bias;
  
}

////////////////////////////////////////////////////////
// FLOAT32 to INT8 conversion
//
// specified scale and bias

void MdvxField::_float32_to_int8(double output_scale,
				 double output_bias)
  
{

  // check whether to put missing and bad at lower end or
  // upper end

  fl32 in_missing = _fhdr.missing_data_value;
  fl32 in_bad =  _fhdr.bad_data_value;
  ui08 out_missing = 0;
  ui08 out_bad = 0;

  int min_byte_val =
    (int) ((_fhdr.min_value - output_bias) / output_scale + 0.49999);
  int max_byte_val =
    (int) ((_fhdr.max_value - output_bias) / output_scale + 0.49999);

  if (min_byte_val > 1) {

    if (in_missing == in_bad) {
      out_missing = 0;
      out_bad = 0;
    } else {
      out_missing = 0;
      out_bad = 1;
    }

  } else if (max_byte_val < 254) {

    if (in_missing == in_bad) {
      out_missing = 255;
      out_bad = 255;
    } else {
      out_missing = 255;
      out_bad = 254;
    }

  } else {

#ifdef DEBUG_PRINT
    cerr << "WARNING - MdvxField::_float32_to_int8." << endl;
    cerr << "  Inconsistent scale, bias and data values." << endl;
    cerr << "  output_scale: " << output_scale << endl;
    cerr << "  output_bias: " << output_bias << endl;
    cerr << "  min_val: " << _fhdr.min_value << endl;
    cerr << "  max_val: " << _fhdr.max_value << endl;
    cerr << "  Using 0 for missing and 1 for bad value" << endl;
    cerr << "  Data which maps to byte val of < 2 will be set to 2" << endl;
#endif
    
  } // if (min_byte_val >= 2) 

  // copy the volume buffer

  MemBuf copyBuf(_volBuf);

  // allocate the output buffer

  int64_t npoints = _fhdr.nx * _fhdr.ny * _fhdr.nz;
  int64_t output_size = npoints * sizeof(ui08);
  _volBuf.prepare(output_size);

  // convert data
  
  fl32 *in = (fl32 *) copyBuf.getPtr();
  ui08 *out = (ui08 *) _volBuf.getPtr();

  for (int64_t i = 0; i < npoints; i++, in++, out++) {
    fl32 in_val = *in;
    if (in_val == in_missing) {
      *out = out_missing;
    } else if (in_val == in_bad) {
      *out = out_bad;
    } else {
      int out_val = (int) ((in_val - output_bias) / output_scale + 0.49999);
      if (out_missing == 0) {
	if (out_val > 255) {
	  *out = 255;
	} else if (out_val < 2) {
	  *out = 2;
	} else {
	  *out = (ui08) out_val;
	}
      } else {
	if (out_val > 253) {
	  *out = 253;
	} else if (out_val < 0) {
	  *out = 0;
	} else {
	  *out = (ui08) out_val;
	}
      }
    }
  } // i */

  // adjust header
  
  _fhdr.volume_size = output_size;
  _fhdr.encoding_type = Mdvx::ENCODING_INT8;
  _fhdr.scaling_type = Mdvx::SCALING_SPECIFIED;
  _fhdr.data_element_nbytes = 1;
  _fhdr.missing_data_value = out_missing;
  _fhdr.bad_data_value = out_bad;
  _fhdr.scale = (fl32) output_scale;
  _fhdr.bias = (fl32) output_bias;
  
}

///////////////////////////////
// FLOAT32 to INT16 conversion
//
// Rounded or dynamic

void MdvxField::_float32_to_int16(int output_scaling)
  
{

  // set missing and bad

  fl32 in_missing = _fhdr.missing_data_value;
  fl32 in_bad =  _fhdr.bad_data_value;
  ui16 out_missing = 0;
  ui16 out_bad;
  if (in_missing == in_bad) {
    out_bad = out_missing;
  } else {
    out_bad = 1;
  }

  // compute scale and bias

  double scale, bias;

  if (_fhdr.max_value == _fhdr.min_value) {
    
    scale = 1.0;
    bias = _fhdr.min_value - 20.0;
    
  } else {
    
    double range = _fhdr.max_value - _fhdr.min_value;
    scale = range / 65500.0;
    
    if (output_scaling == Mdvx::SCALING_ROUNDED) {
      scale = _round_up(scale);
      bias = _fhdr.min_value - scale * 20.0;
      bias = floor(bias / scale) * scale;
    } else if (output_scaling == Mdvx::SCALING_INTEGRAL) {
      scale = floor (scale + 1.0);
      bias = _fhdr.min_value - scale * 20.0;
      bias = floor(bias / scale) * scale;
    } else {
      bias = _fhdr.min_value - scale * 20.0;
    }
    
  }
  
  // copy the volume buffer
  
  MemBuf copyBuf(_volBuf);

  // allocate the output buffer
  
  int64_t npoints = _fhdr.nx * _fhdr.ny * _fhdr.nz;
  int64_t output_size = npoints * sizeof(ui16);
  _volBuf.prepare(output_size);

  // convert data
  
  fl32 *in = (fl32 *) copyBuf.getPtr();
  ui16 *out = (ui16 *) _volBuf.getPtr();
  int64_t nBad = 0;

  for (int64_t i = 0; i < npoints; i++, in++, out++) {
    fl32 in_val = *in;
    if (in_val == in_missing) {
      *out = out_missing;
    } else if (in_val == in_bad) {
      *out = out_bad;
    } else {
      int out_val = (int) ((in_val - bias) / scale + 0.49999);
      if (out_val > 65535) {
        nBad++;
 	*out = 65535;
      } else if (out_val < 20) {
        nBad++;
 	*out = 20;
      } else {
	*out = (ui16) out_val;
      }
    }
  } // i */

  if (nBad > 0) {
    cerr << "ERROR - MdvxField::_float32_to_int16" << endl;
    cerr << "  Out of range data found, field: " << getFieldName() << endl;
    cerr << "  n points: " << nBad << endl;
    cerr << "  Replaced with min or max values as appropriate" << endl;
  }

  // adjust header
  
  _fhdr.volume_size = output_size;
  _fhdr.encoding_type = Mdvx::ENCODING_INT16;
  _fhdr.scaling_type = output_scaling;
  _fhdr.data_element_nbytes = 2;
  _fhdr.missing_data_value = out_missing;
  _fhdr.bad_data_value = out_bad;
  _fhdr.scale = (fl32) scale;
  _fhdr.bias = (fl32) bias;
  
}

///////////////////////////////
// FLOAT32 to INT16 conversion
//
// Specified scale and bias

void MdvxField::_float32_to_int16(double output_scale,
				  double output_bias)
  
{

  // check whether to put missing and bad at lower end or
  // upper end

  fl32 in_missing = _fhdr.missing_data_value;
  fl32 in_bad =  _fhdr.bad_data_value;
  ui16 out_missing = 0;
  ui16 out_bad = 0;

  int min_byte_val =
    (int) ((_fhdr.min_value - output_bias) / output_scale + 0.49999);
  int max_byte_val =
    (int) ((_fhdr.max_value - output_bias) / output_scale + 0.49999);

  if (min_byte_val > 1) {

    if (in_missing == in_bad) {
      out_missing = 0;
      out_bad = 0;
    } else {
      out_missing = 0;
      out_bad = 1;
    }

  } else if (max_byte_val < 65534) {

    if (in_missing == in_bad) {
      out_missing = 65535;
      out_bad = 65535;
    } else {
      out_missing = 65535;
      out_bad = 65534;
    }

  } else {

#ifdef DEBUG_PRINT
    cerr << "WARNING - MdvxField::_float32_to_int16." << endl;
    cerr << "  Inconsistent scale, bias and data values." << endl;
    cerr << "  output_scale: " << output_scale << endl;
    cerr << "  output_bias: " << output_bias << endl;
    cerr << "  min_val: " << _fhdr.min_value << endl;
    cerr << "  max_val: " << _fhdr.max_value << endl;
    cerr << "  Using 0 for missing and 1 for bad value" << endl;
    cerr << "  Data which maps to byte val of < 2 will be set to 2" << endl;
#endif
    
  } // if (min_byte_val >= 2) 

  // copy the volume buffer
  
  MemBuf copyBuf(_volBuf);

  // allocate the output buffer

  int64_t npoints = _fhdr.nx * _fhdr.ny * _fhdr.nz;
  int64_t output_size = npoints * sizeof(ui16);
  _volBuf.prepare(output_size);

  // convert data
  
  fl32 *in = (fl32 *) copyBuf.getPtr();
  ui16 *out = (ui16 *) _volBuf.getPtr();

  for (int64_t i = 0; i < npoints; i++, in++, out++) {
    fl32 in_val = *in;
    if (in_val == in_missing) {
      *out = out_missing;
    } else if (in_val == in_bad) {
      *out = out_bad;
    } else {
      int out_val = (int) ((in_val - output_bias) / output_scale + 0.49999);
      if (out_missing == 0) {
	if (out_val > 65535) {
	  *out = 65535;
	} else if (out_val < 2) {
	  *out = 2;
	} else {
	  *out = (ui16) out_val;
	}
      } else {
	if (out_val > 65533) {
	  *out = 65533;
	} else if (out_val < 0) {
	  *out = 0;
	} else {
	  *out = (ui16) out_val;
	}
      }
    }
  } // i */

  // adjust header
  
  _fhdr.volume_size = output_size;
  _fhdr.encoding_type = Mdvx::ENCODING_INT16;
  _fhdr.scaling_type = Mdvx::SCALING_SPECIFIED;
  _fhdr.data_element_nbytes = 2;
  _fhdr.missing_data_value = out_missing;
  _fhdr.bad_data_value = out_bad;
  _fhdr.scale = (fl32) output_scale;
  _fhdr.bias = (fl32) output_bias;
  
}

///////////////////////////////////////////////////////////////////
// constrain the data in the vertical, based on read limits set in
// the mdvx object

void MdvxField::constrainVertical(const Mdvx &mdvx)

{

  if (!_volbufSizeValid()) {
    cerr << "WARNING - MdvxField::constrainVertical()" << endl;
    cerr << "  Data buffer not set, field: " << getFieldName() << endl;
    return;
  }
  
  int minPlane, maxPlane;
  
  if (mdvx._readPlaneNumLimitsSet) {
    minPlane = mdvx._readMinPlaneNum;
    maxPlane = mdvx._readMaxPlaneNum;
  } else {
    computePlaneLimits(mdvx._readMinVlevel, mdvx._readMaxVlevel,
		       minPlane, maxPlane);
  }

  // swap if necessary

  if (minPlane > maxPlane) {
    int tmpPlane = minPlane;
    minPlane = maxPlane;
    maxPlane = tmpPlane;
  }

  // Sanity check on _fhdr.nz value insures no out-of-bounds array access
  if (_fhdr.nz < 1) {
    _fhdr.nz = 1;
  }

  if (minPlane < 0) {
    minPlane = 0;
  }
  if (minPlane > _fhdr.nz - 1) {
    minPlane = _fhdr.nz - 1;
  }
  if (maxPlane < 0) {
    maxPlane = 0;
  }
  if (maxPlane > _fhdr.nz - 1) {
    maxPlane = _fhdr.nz - 1;
  }

  int outNz = maxPlane - minPlane + 1;

  // if compressed with GZIP_VOL, decompress
  
  if (isCompressed() && ta_gzip_buffer(_volBuf.getPtr())) {
    _decompressGzipVol();
  }
  
  if (isCompressed()) {

    // compressed data
    
    MemBuf outBuf;
    ui32 outPlaneOffsets[MDV_MAX_VLEVELS];
    ui32 outPlaneSizes[MDV_MAX_VLEVELS];
    
    // assemble out buffers plane-by-plane
    
    ui32 *be_in_offsets = (ui32 *) _volBuf.getPtr();
    ui32 *be_in_sizes = be_in_offsets + _fhdr.nz;
    
    for (int iz = 0; iz < outNz; iz++) {
      
      ui32 in_offset = BE_to_ui32(be_in_offsets[iz + minPlane]);
      ui32 in_size = BE_to_ui32(be_in_sizes[iz + minPlane]);
      
      outPlaneOffsets[iz] = BE_from_ui32(outBuf.getLen());
      outPlaneSizes[iz] = BE_from_ui32(in_size);
      
      void *outData =
	((ui08 *) _volBuf.getPtr() +
	 2 * _fhdr.nz * sizeof(ui32) + in_offset);
      outBuf.add(outData, in_size);
      
    }

    // assemble volume buffer
    
    _volBuf.free();
    _volBuf.add(outPlaneOffsets, outNz * sizeof(ui32));
    _volBuf.add(outPlaneSizes, outNz * sizeof(ui32));
    _volBuf.add(outBuf.getPtr(), outBuf.getLen());

    // set volume size

    _fhdr.volume_size = _volBuf.getLen();

  } else {

    // uncompressed data
    
    MemBuf outBuf;
    int64_t outOffset = (_fhdr.nx * _fhdr.ny *
                         _fhdr.data_element_nbytes * minPlane);
    int64_t outSize = _fhdr.nx * _fhdr.ny * outNz * _fhdr.data_element_nbytes;
    void *outData = ((ui08 *) _volBuf.getPtr() + outOffset);
    outBuf.add(outData, outSize);
    _volBuf = outBuf;
    _fhdr.volume_size = _volBuf.getLen();
    
  }

  // update headers

  _fhdr.nz = outNz;
  for (int i = 0; i < outNz; i++) {
    _vhdr.level[i] = _vhdr.level[i + minPlane];
  }
  for (int i = outNz; i < maxPlane; i++) {
    _vhdr.level[i] = 0.0;
  }
  _fhdr.grid_minz = _vhdr.level[0];

  computeMinAndMax();

}

///////////////////////////////////////////////////////////////////
// For latlon grids, might need to shift the lon domain
//

void MdvxField::_check_lon_domain(double read_min_lon,
				  double read_max_lon)

{

  double dLon = _fhdr.grid_dx;
  double dataMinLon = _fhdr.grid_minx;
  double dataMaxLon = dataMinLon + dLon * (_fhdr.nx - 1);
  
  // check if the request can be satisfied with existing
  // domain, or by shifting the domain

  if (dataMinLon <= read_min_lon &&
      dataMaxLon >= read_max_lon) {
    // request is inside data
    return;
  } else if (dataMinLon >= read_min_lon &&
	     dataMaxLon <= read_max_lon) {
    // data is inside request
    return;
  } else if ((dataMinLon + 360.0) <= read_min_lon &&
	     (dataMaxLon + 360.0) >= read_max_lon) {
    // request is inside data shifted right
    _fhdr.grid_minx += 360.0;
    return;
  } else if ((dataMinLon + 360.0) >= read_min_lon &&
	     (dataMaxLon + 360.0) <= read_max_lon) {
    // data shifted right is inside request
    _fhdr.grid_minx += 360.0;
    return;
  } else if ((dataMinLon - 360.0) <= read_min_lon &&
	     (dataMaxLon - 360.0) >= read_max_lon) {
    // request is inside data shifted left
    _fhdr.grid_minx -= 360.0;
    return;
  } else if ((dataMinLon - 360.0) >= read_min_lon &&
	     (dataMaxLon - 360.0) <= read_max_lon) {
    // data shifted left is inside request
    _fhdr.grid_minx -= 360.0;
    return;
  }

  // move the data into the desired read range
  
  int replyMinIlon = (int) ((read_min_lon - dataMinLon) / dLon);
  int replyMaxIlon = (int) ((read_max_lon - dataMinLon) / dLon) + 1;
  int replyNLon = (replyMaxIlon - replyMinIlon) + 1;
  double replyMinLon = dataMinLon + replyMinIlon * dLon;

  // compute lookup table for each longitude in output grid

  int *lut = new int[replyNLon];
  
  for (int ix = 0; ix < replyNLon; ix++) {
    
    double lon = replyMinLon + ix * dLon;

    if (lon >= dataMinLon && lon <= dataMaxLon) {
      lut[ix] = (int) ((lon - dataMinLon) / dLon + 0.5);
    } else if (lon + 360.0 >= dataMinLon && lon + 360.0 <= dataMaxLon) {
      lut[ix] = (int) ((lon + 360.0 - dataMinLon) / dLon + 0.5);
    } else if (lon - 360.0 >= dataMinLon && lon - 360.0 <= dataMaxLon) {
      lut[ix] = (int) ((lon - 360.0 - dataMinLon) / dLon + 0.5);
    } else {
      lut[ix] = -1;
    }

  } // ix
  
  // copy into a working buffer
  
  MemBuf workBuf;
  workBuf.reserve(replyNLon * _fhdr.ny * _fhdr.nz *
		  _fhdr.data_element_nbytes);
  
  if (workBuf.getPtr() == NULL ) {
    cerr << "ERROR - MdvxField::_check_lon_domain" << endl;
    cerr << "  MemBuf allocation error " << endl;
    delete [] lut;
    return;
  }

  for (int iz = 0; iz < _fhdr.nz; iz++) {
    for (int iy = 0; iy < _fhdr.ny; iy++) {
      
      if (_fhdr.encoding_type == Mdvx::ENCODING_INT8) {
	
	ui08 *in = ((ui08 *) _volBuf.getPtr()) +
	  iz * _fhdr.ny * _fhdr.nx + iy * _fhdr.nx;

	ui08 *out = (ui08 *) workBuf.getPtr() +
	  iz * _fhdr.ny * replyNLon + iy * replyNLon;
	
	ui08 missing = (ui08) _fhdr.missing_data_value;
	
	for (int ix = 0; ix < replyNLon; ix++) {
	  int index = lut[ix];
	  if (index < 0) {
	    out[ix] = missing;
	  } else {
	    out[ix] = in[index];
	  }
	} // ix

      } else if (_fhdr.encoding_type == Mdvx::ENCODING_INT16) {
	
	ui16 *in = ((ui16 *) _volBuf.getPtr()) +
	  iz * _fhdr.ny * _fhdr.nx + iy * _fhdr.nx;

	ui16 *out = (ui16 *) workBuf.getPtr() +
	  iz * _fhdr.ny * replyNLon + iy * replyNLon;
	
	ui16 missing = (ui16) _fhdr.missing_data_value;
	
	for (int ix = 0; ix < replyNLon; ix++) {
	  int index = lut[ix];
	  if (index < 0) {
	    out[ix] = missing;
	  } else {
	    out[ix] = in[index];
	  }
	} // ix

	
      } else if (_fhdr.encoding_type == Mdvx::ENCODING_FLOAT32) {
	
	fl32 *in = ((fl32 *) _volBuf.getPtr()) +
	  iz * _fhdr.ny * _fhdr.nx + iy * _fhdr.nx;

	fl32 *out = (fl32 *) workBuf.getPtr() +
	  iz * _fhdr.ny * replyNLon + iy * replyNLon;
	
	fl32 missing = (fl32) _fhdr.missing_data_value;
	
	for (int ix = 0; ix < replyNLon; ix++) {
	  int index = lut[ix];
	  if (index < 0) {
	    out[ix] = missing;
	  } else {
	    out[ix] = in[index];
	  }
	} // ix
	
      }
      
    } // iy
  } // iz
  
  // copy the working buffer to the volBuf

  _volBuf = workBuf;
  
  // reset header variables
  
  _fhdr.nx = replyNLon;
  _fhdr.grid_minx = replyMinLon;
  _fhdr.volume_size = _volBuf.getLen(); 

  delete[] lut;

}

///////////////////////////////////////////////////////////////////
// constrain the data in the horizontal, based on read limits set in
// the mdvx object
//
// Returns 0 on success, -1 on failure.

void MdvxField::constrainHorizontal(const Mdvx &mdvx)

{
  
  if (!_volbufSizeValid()) {
    // no-op
    cerr << "WARNING - MdvxField::constrainHorizontal()" << endl;
    cerr << "  Data buffer not set, field: " << getFieldName() << endl;
    return;
  }
  
  // find the index limits for the bounding box. Each corner must
  // be tested because the projection may cause the sides to be
  // skewed in x,y space

  MdvxProj proj(_fhdr);
  if (!proj.supported()) {
    return;
  }

  // check proj type is supported
  
  if (proj.getProjType() == Mdvx::PROJ_POLAR_RADAR) {
    _constrain_radar_horiz(mdvx);
  }

  if (proj.getProjType() == Mdvx::PROJ_POLAR_RADAR ||
      proj.getProjType() == Mdvx::PROJ_POLAR_STEREO ||
      proj.getProjType() == Mdvx::PROJ_OBLIQUE_STEREO) {
    return;
  }
  
  double xx, yy, lat, lon;
  lat = mdvx._readMinLat;
  lon = mdvx._readMinLon;
  proj.latlon2xy(lat, lon, xx, yy);

  double minX = xx;
  double maxX = xx;
  double minY = yy;
  double maxY = yy;

  lat = mdvx._readMinLat;
  lon = mdvx._readMaxLon;
  proj.latlon2xy(lat, lon, xx, yy);

  minX = MIN(minX, xx);
  maxX = MAX(maxX, xx);
  minY = MIN(minY, yy);
  maxY = MAX(maxY, yy);
		       
  lat = mdvx._readMaxLat;
  lon = mdvx._readMinLon;
  proj.latlon2xy(lat, lon, xx, yy);

  minX = MIN(minX, xx);
  maxX = MAX(maxX, xx);
  minY = MIN(minY, yy);
  maxY = MAX(maxY, yy);
		       
  lat = mdvx._readMaxLat;
  lon = mdvx._readMaxLon;
  proj.latlon2xy(lat, lon, xx, yy);

  minX = MIN(minX, xx);
  maxX = MAX(maxX, xx);
  minY = MIN(minY, yy);
  maxY = MAX(maxY, yy);
		       
  lat = (mdvx._readMinLat + mdvx._readMaxLat) / 2.0;
  lon = mdvx._readMinLon;
  proj.latlon2xy(lat, lon, xx, yy);

  minX = MIN(minX, xx);
  maxX = MAX(maxX, xx);
  minY = MIN(minY, yy);
  maxY = MAX(maxY, yy);
		       
  lat = (mdvx._readMinLat + mdvx._readMaxLat) / 2.0;
  lon = mdvx._readMaxLon;
  proj.latlon2xy(lat, lon, xx, yy);

  minX = MIN(minX, xx);
  maxX = MAX(maxX, xx);
  minY = MIN(minY, yy);
  maxY = MAX(maxY, yy);
		       
  lat = mdvx._readMinLat;
  lon = (mdvx._readMinLon + mdvx._readMaxLon) / 2.0;
  proj.latlon2xy(lat, lon, xx, yy);

  minX = MIN(minX, xx);
  maxX = MAX(maxX, xx);
  minY = MIN(minY, yy);
  maxY = MAX(maxY, yy);
		       
  lat = mdvx._readMaxLat;
  lon = (mdvx._readMinLon + mdvx._readMaxLon) / 2.0;
  proj.latlon2xy(lat, lon, xx, yy);

  minX = MIN(minX, xx);
  maxX = MAX(maxX, xx);
  minY = MIN(minY, yy);
  maxY = MAX(maxY, yy);

  // check for no overlap and clipping between zoom and original data

  double grid_maxx = _fhdr.grid_minx + (_fhdr.nx * _fhdr.grid_dx);
  double grid_maxy = _fhdr.grid_miny + (_fhdr.ny * _fhdr.grid_dy);
  
  if (minX > grid_maxx || maxX < _fhdr.grid_minx ||
      minY > grid_maxy || maxY < _fhdr.grid_miny) {
    _fhdr.zoom_no_overlap = 1;
  }

  if (minX > _fhdr.grid_minx || maxX < grid_maxx ||
      minY > _fhdr.grid_miny || maxY < grid_maxy) {
    _fhdr.zoom_clipped = 1;
  }
  
  // convert to grid indices

  int minIx, maxIx, minIy, maxIy;
  proj.xy2xyIndex(minX, minY, minIx, minIy);
  proj.xy2xyIndex(maxX, maxY, maxIx, maxIy);

  // If we have a grid that is inverted (dx < 0 or dy < 0), then our calculated
  // indices will be in the wrong order.  Make sure that min < max before
  // continuing on.

  if (minIx > maxIx)
  {
    int temp = minIx;
    minIx = maxIx;
    maxIx = temp;
  }
  
  if (minIy > maxIy)
  {
    int temp = minIy;
    minIy = maxIy;
    maxIy = temp;
  }
  
  // add a boundary of 2 grid element on each side to allow for contouring

  minIy -= 2;
  if (minIy < 0) {
    minIy = 0;
  }
  minIx -= 2;
  if (minIx < 0) {
    minIx = 0;
  }
  maxIx += 2;
  if (maxIx > _fhdr.nx - 1) {
    maxIx = _fhdr.nx - 1;
  }
  maxIy += 2;
  if (maxIy > _fhdr.ny - 1) {
    maxIy = _fhdr.ny - 1;
  }

  int nyOut = maxIy - minIy + 1;
  int nxOut = maxIx - minIx + 1;
  
  // pack the bounded data into a working buffer

  MemBuf workBuf;
  
  int64_t nbytesLineIn = _fhdr.nx * _fhdr.data_element_nbytes;
  int64_t nbytesPlaneIn = nbytesLineIn * _fhdr.ny;
  int64_t nbytesLineOut = nxOut * _fhdr.data_element_nbytes;
  
  for (int iz = 0; iz < _fhdr.nz; iz++) {
    int64_t offset = (iz * nbytesPlaneIn) +
      (minIy * _fhdr.nx + minIx) * _fhdr.data_element_nbytes;
    for (int iy = minIy; iy <= maxIy; iy++, offset += nbytesLineIn) {
      void *ptr = ((ui08 *) _volBuf.getPtr() + offset);
      workBuf.add(ptr, nbytesLineOut);
    } // iy
  } // iz

  // copy the working buffer to the volBuf

  _volBuf = workBuf;

  // reset header variables

  _fhdr.nx = nxOut;
  _fhdr.ny = nyOut;
  _fhdr.grid_minx += minIx * _fhdr.grid_dx;
  _fhdr.grid_miny += minIy * _fhdr.grid_dy;
  _fhdr.volume_size = _volBuf.getLen(); 

}

///////////////////////////////////////////////////////////////////
// for polar radar,
// constrain the data in the horizontal, based on read limits set in
// the mdvx object
//
// Returns 0 on success, -1 on failure.

int MdvxField::_constrain_radar_horiz(const Mdvx &mdvx)

{

  MdvxProj proj;
  proj.initAzimEquiDist(_fhdr.proj_origin_lat, _fhdr.proj_origin_lon, 0);

  double xx, yy, lat, lon;
  lat = mdvx._readMinLat;
  lon = mdvx._readMinLon;
  proj.latlon2xy(lat, lon, xx, yy);
  
  double minX = xx;
  double maxX = xx;
  double minY = yy;
  double maxY = yy;

  lat = mdvx._readMinLat;
  lon = mdvx._readMaxLon;
  proj.latlon2xy(lat, lon, xx, yy);

  minX = MIN(minX, xx);
  maxX = MAX(maxX, xx);
  minY = MIN(minY, yy);
  maxY = MAX(maxY, yy);
		       
  lat = mdvx._readMaxLat;
  lon = mdvx._readMinLon;
  proj.latlon2xy(lat, lon, xx, yy);

  minX = MIN(minX, xx);
  maxX = MAX(maxX, xx);
  minY = MIN(minY, yy);
  maxY = MAX(maxY, yy);
		       
  lat = mdvx._readMaxLat;
  lon = mdvx._readMaxLon;
  proj.latlon2xy(lat, lon, xx, yy);

  minX = MIN(minX, xx);
  maxX = MAX(maxX, xx);
  minY = MIN(minY, yy);
  maxY = MAX(maxY, yy);
		       
  bool radarIsVisible = false;
  if (minX <= 0 && maxX >= 0 &&
      minY <= 0 && maxY >= 0) {
    radarIsVisible = true;
  }
  bool keepAllAngles = false;
  if (radarIsVisible) {
    keepAllAngles = true;
  }

#ifdef DEBUG_PRINT
  cerr << "constrain_radar_horiz: minX: " << minX << endl;
  cerr << "constrain_radar_horiz: maxX: " << maxX << endl;
  cerr << "constrain_radar_horiz: minY: " << minY << endl;
  cerr << "constrain_radar_horiz: maxY: " << maxY << endl;
  cerr << "constrain_radar_horiz: ny: " << _fhdr.ny << endl;
  cerr << "constrain_radar_horiz: grid_miny: " << _fhdr.grid_miny << endl;
  cerr << "constrain_radar_horiz: nx: " << _fhdr.nx << endl;
  cerr << "constrain_radar_horiz: grid_minx: " << _fhdr.grid_minx << endl;
#endif

  // compute max range and azimith limits

  double minRange = 9999;
  double maxRange = 0;
  double minAz = 0;
  double maxAz = 360.0;
  
  if (radarIsVisible) {

#ifdef DEBUG_PRINT
    cerr << "constrain_radar_horiz: radarIsVisible" << endl;
#endif

    // radar origin is in view

    double rangeSW = sqrt((minX * minX) + (minY * minY));
    maxRange = rangeSW;

    double rangeNW = sqrt((minX * minX) + (maxY * maxY));
    if (rangeNW > maxRange) maxRange = rangeNW;

    double rangeNE = sqrt((maxX * maxX) + (maxY * maxY));
    if (rangeNE > maxRange) maxRange = rangeNE;

    double rangeSE = sqrt((maxX * maxX) + (minY * minY));
    if (rangeSE > maxRange) maxRange = rangeSE;

    minRange = _fhdr.grid_minx;
    
  } else if (maxX < 0 && maxY < 0) {

    // SW quadrant

#ifdef DEBUG_PRINT
    cerr << "constrain_radar_horiz: SW quadrant" << endl;
#endif

    minRange = sqrt((maxX * maxX) + (maxY * maxY));
    maxRange = sqrt((minX * minX) + (minY * minY));
    minAz = atan2(maxX, minY) * RAD_TO_DEG + 360.0;
    maxAz = atan2(minX, maxY) * RAD_TO_DEG + 360.0;
    
  } else if (maxX < 0 && minY > 0) {

    // NW quadrant

#ifdef DEBUG_PRINT
    cerr << "constrain_radar_horiz: NW quadrant" << endl;
#endif

    minRange = sqrt((maxX * maxX) + (minY * minY));
    maxRange = sqrt((minX * minX) + (maxY * maxY));
    minAz = atan2(minX, minY) * RAD_TO_DEG + 360.0;
    maxAz = atan2(maxX, maxY) * RAD_TO_DEG + 360.0;
    
  } else if (minX > 0 && minY > 0) {

    // NE quadrant

#ifdef DEBUG_PRINT
    cerr << "constrain_radar_horiz: NE quadrant" << endl;
#endif

    minRange = sqrt((minX * minX) + (minY * minY));
    maxRange = sqrt((maxX * maxX) + (maxY * maxY));
    minAz = atan2(minX, maxY) * RAD_TO_DEG;
    maxAz = atan2(maxX, minY) * RAD_TO_DEG;
    
  } else if (minX > 0 && maxY < 0) {

    // SE quadrant

#ifdef DEBUG_PRINT
    cerr << "constrain_radar_horiz: SE quadrant" << endl;
#endif

    minRange = sqrt((minX * minX) + (maxY * maxY));
    maxRange = sqrt((maxX * maxX) + (minY * minY));
    minAz = atan2(maxX, maxY) * RAD_TO_DEG;
    maxAz = atan2(minX, minY) * RAD_TO_DEG;
    
  } else if (maxY < 0) {

    // S sector

#ifdef DEBUG_PRINT
    cerr << "constrain_radar_horiz: S sector" << endl;
#endif

    minRange = -maxY;
    maxRange = sqrt((minX * minX) + (minY * minY));
    double maxRange2 = sqrt((maxX * maxX) + (minY * minY));
    if (maxRange2 > maxRange) maxRange = maxRange2;
    minAz = atan2(maxX, maxY) * RAD_TO_DEG;
    maxAz = atan2(minX, maxY) * RAD_TO_DEG + 360;
    
  } else if (maxX < 0) {

    // W sector

#ifdef DEBUG_PRINT
    cerr << "constrain_radar_horiz: W sector" << endl;
#endif

    minRange = -maxX;
    maxRange = sqrt((minX * minX) + (minY * minY));
    double maxRange2 = sqrt((minX * minX) + (maxY * maxY));
    if (maxRange2 > maxRange) maxRange = maxRange2;
    minAz = atan2(maxX, minY) * RAD_TO_DEG + 360;
    maxAz = atan2(maxX, maxY) * RAD_TO_DEG + 360;
    
  } else if (minY > 0) {

    // N sector
    
#ifdef DEBUG_PRINT
    cerr << "constrain_radar_horiz: N sector" << endl;
#endif

    minRange = minY;
    maxRange = sqrt((minX * minX) + (maxY * maxY));
    double maxRange2 = sqrt((maxX * maxX) + (maxY * maxY));
    if (maxRange2 > maxRange) maxRange = maxRange2;
    minAz = atan2(minX, minY) * RAD_TO_DEG + 360;
    maxAz = atan2(maxX, minY) * RAD_TO_DEG;
    
  } else if (minX > 0) {

    // E sector
    
#ifdef DEBUG_PRINT
    cerr << "constrain_radar_horiz: E sector" << endl;
#endif

    minRange = minX;
    maxRange = sqrt((maxX * maxX) + (maxY * maxY));
    double maxRange2 = sqrt((maxX * maxX) + (minY * minY));
    if (maxRange2 > maxRange) maxRange = maxRange2;
    minAz = atan2(minX, maxY) * RAD_TO_DEG;
    maxAz = atan2(minX, minY) * RAD_TO_DEG;
    
  }
  
#ifdef DEBUG_PRINT
  cerr << "constrain_radar_horiz: minAz: " << minAz << endl;
  cerr << "constrain_radar_horiz: maxAz: " << maxAz << endl;
#endif

  // compute indexes for azimuth limits

  int minAzIndex = 0;
  int maxAzIndex = _fhdr.ny - 1;
  int nAz = _fhdr.ny;
  int n360 = (int) (360.0 / _fhdr.grid_dy + 0.5);

  if (!radarIsVisible) {
    
    minAzIndex = (int) ((minAz - _fhdr.grid_miny) / _fhdr.grid_dy + 0.5);
    maxAzIndex = (int) ((maxAz - _fhdr.grid_miny) / _fhdr.grid_dy + 0.5);

    // widen to ensure coverage
    
    minAzIndex -= 2;
    maxAzIndex += 2;

    // ensure positive indices

    if (minAzIndex < 0) minAzIndex += n360;
    if (maxAzIndex < 0) maxAzIndex += n360;
    
    if (minAzIndex > maxAzIndex) {

      // region spans the missing sector,
      // so we need to keep all angles
      
      keepAllAngles = true;
      minAzIndex = 0;
      maxAzIndex = _fhdr.ny - 1;
      nAz = _fhdr.ny;

    } else {
      
      if (minAzIndex >= _fhdr.ny) {
        minAzIndex = _fhdr.ny - 1;
      }
      if (maxAzIndex >= _fhdr.ny) {
        maxAzIndex = _fhdr.ny - 1;
      }
      
      nAz = maxAzIndex - minAzIndex + 1;
      
    }
    
#ifdef DEBUG_PRINT
    cerr << "constrain_radar_horiz: minAzIndex: " << minAzIndex << endl;
    cerr << "constrain_radar_horiz: maxAzIndex: " << maxAzIndex << endl;
    cerr << "constrain_radar_horiz: nAz: " << nAz << endl;
#endif

  }
  
  // compute indexes for range limits
  
  int minRangeIndex =
    (int) ((minRange - _fhdr.grid_minx) / _fhdr.grid_dx + 0.5);
  int maxRangeIndex =
    (int) ((maxRange - _fhdr.grid_minx) / _fhdr.grid_dx + 0.5);

  // widen to ensure coverage.  Note that we have to make sure that the
  // minRangeIndex value is in the 0-nx range.  This covers the case where
  // the radar data is completely outside of the requested region.  In this
  // case, we'll return a minimal amount of data, but not an error.  (Without
  // this check, we were ending up with minRangeIndex > maxRangeIndex which
  // caused a core dump when we requested a MemBuf with a negative size
  // further down in the code.)

  minRangeIndex -= 2;
  if (minRangeIndex < 0) minRangeIndex = 0;
  if (minRangeIndex >= _fhdr.nx) minRangeIndex = _fhdr.nx - 1;
  maxRangeIndex += 2;
  if (maxRangeIndex >= _fhdr.nx) maxRangeIndex = _fhdr.nx - 1;
  int nRange = maxRangeIndex - minRangeIndex + 1;
  
  // return now if no clipping is required

  if (keepAllAngles && nRange == _fhdr.nx) {
    return 0;
  }

  // set clipping flag

  _fhdr.zoom_clipped = 1;
  
  // pack the bounded data into a working buffer

  MemBuf workBuf;
  
  int64_t nbytesBeamIn = _fhdr.nx * _fhdr.data_element_nbytes;
  int64_t nbytesPlaneIn = nbytesBeamIn * _fhdr.ny;
  int64_t nbytesBeamOut = nRange * _fhdr.data_element_nbytes;
  
  for (int iz = 0; iz < _fhdr.nz; iz++) {
    
    for (int iaz = 0; iaz < nAz; iaz++) {
      
      int azIndex = (iaz + minAzIndex + _fhdr.ny) % _fhdr.ny;
      int64_t offset = (iz * nbytesPlaneIn) +
        (azIndex * _fhdr.nx + minRangeIndex) * _fhdr.data_element_nbytes;
      void *ptr = ((ui08 *) _volBuf.getPtr() + offset);
      workBuf.add(ptr, nbytesBeamOut);
      
    } // iaz

  } // iz

  // copy the working buffer to the volBuf

  _volBuf = workBuf;

  // reset header variables

  _fhdr.nx = nRange;
  _fhdr.grid_minx += minRangeIndex * _fhdr.grid_dx;
  _fhdr.ny = nAz;
  _fhdr.grid_miny += minAzIndex * _fhdr.grid_dy;
  _fhdr.volume_size = _volBuf.getLen(); 

#ifdef DEBUG_PRINT
  cerr << "-->> constrain_radar_horiz: nRange: " << _fhdr.nx << endl;
  cerr << "-->> constrain_radar_horiz: nAz: " << _fhdr.ny << endl;
  cerr << "-->> constrain_radar_horiz: minRange: " << _fhdr.grid_minx << endl;
  cerr << "-->> constrain_radar_horiz: minAz: " << _fhdr.grid_miny << endl;
#endif

  return 0;

}

///////////////////////////////////////////////////////////////////////
// Decimate to max grid cell count for polar radar data
//
// Returns 0 on success, -1 on failure.

int MdvxField::_decimate_radar_horiz(int64_t max_nxy)
  
{

  // assume we need 2 pixels to show a gate

  double pixelsPerGate = 2;
  int maxNgates = (int) (max_nxy / pixelsPerGate);

  // check if we need to decimate
  
  double fullNgates = _fhdr.nx * _fhdr.ny;

  double pointRatio = (double) fullNgates / (double) maxNgates;
  double azRes = 360.0 / _fhdr.ny;
  double relRatio = 3.5 * azRes;
  double azRatio = sqrt(pointRatio / relRatio) + 1.0;
  int azSparsity = (int) azRatio;
  double rangeRatio = (pointRatio / azRatio) + 1.0;
  int rangeSparsity = (int) rangeRatio;
  int sparsity = rangeSparsity * azSparsity;

#ifdef DEBUG_PRINT
  cerr << "====== _decimate_radar_horiz =====" << endl;
  cerr << "  pointRatio: " << pointRatio << endl;
  cerr << "  relRatio: " << relRatio << endl;
  cerr << "  azRes: " << azRes << endl;
  cerr << "  azRatio: " << azRatio << endl;
  cerr << "  azSparsity: " << azSparsity << endl;
  cerr << "  rangeRatio: " << rangeRatio << endl;
  cerr << "  rangeSparsity: " << rangeSparsity << endl;
  cerr << "  sparsity: " << sparsity << endl;
#endif

  if (sparsity < 2) {
    return 0;
  }
  
  // uncompress as required
  
  int compression_type = _fhdr.compression_type;
  bool recompress = false;
  if (isCompressed()) {
    if (decompress()) {
      _errStr += "ERROR - MdvxField::decimate\n";
      return -1;
    }
    recompress = true;
  }

  // compute sizes

  int elemSize = _fhdr.data_element_nbytes;
  int elemJumpSize = rangeSparsity * elemSize;
  
  int azStartPos = (azSparsity - 1) / 2;
  int rangeStartPos = (rangeSparsity - 1) / 2;
  int outNx = (_fhdr.nx - rangeStartPos - 1) / rangeSparsity + 1;
  int outNy = (_fhdr.ny - azStartPos - 1) / azSparsity + 1;

  int64_t nBytesSourceRow = _fhdr.nx * elemSize;
  int64_t nBytesSourcePlane = nBytesSourceRow * _fhdr.ny;
  
  int64_t nBytesTargetRow = outNx * elemSize;
  int64_t nBytesTargetPlane = nBytesTargetRow * outNy;
  int64_t nBytesTargetVol = nBytesTargetPlane * _fhdr.nz;

  // set up working buffer
  
  MemBuf targetBuf;
  targetBuf.prepare(nBytesTargetVol);
  
  // loop through z, decimating into target buffer
  
  ui08 *sourceVol = (ui08 *) _volBuf.getPtr();
  ui08 *targetVol = (ui08 *) targetBuf.getPtr();
  
  for (int iz = 0; iz < _fhdr.nz; iz++) {
    
    ui08 *sourcePlane = sourceVol + iz * nBytesSourcePlane;
    ui08 *targetPlane = targetVol + iz * nBytesTargetPlane;
    
    // loop through y

    for (int iny = azStartPos, outy = 0; outy < outNy;
	 iny += azSparsity, outy++) {
      
      ui08 *source = sourcePlane + iny * nBytesSourceRow + rangeStartPos * elemSize;
      ui08 *target = targetPlane + outy * nBytesTargetRow;

      // loop through x

      for (int outx = 0; outx < outNx;
	   outx++, source += elemJumpSize, target += elemSize) {
	memcpy(target, source, elemSize);
      } // outx
      
    } // outy

  } // iz
  
  // set the field header appropriately
  
  _fhdr.volume_size = nBytesTargetVol;
  _fhdr.nx = outNx;
  _fhdr.ny = outNy;
  _fhdr.grid_minx += rangeStartPos * _fhdr.grid_dx;
  _fhdr.grid_miny += azStartPos * _fhdr.grid_dy;
  _fhdr.grid_dx *= rangeSparsity;
  _fhdr.grid_dy *= azSparsity;

  // copy the target buf to the vol buf

  _volBuf = targetBuf;

  if (recompress) {
    requestCompression(compression_type);
  }

  return 0;

}

///////////////////////////////////////////////////////////////////////
// Decimate to max grid cell count using pixel averaging
// Returns 0 on success, -1 on failure.

int MdvxField::_decimate_rgba(int64_t max_nxy)
  
{
  // check if we need to decimate
  double full_nxy = _fhdr.nx * _fhdr.ny;
  if (max_nxy <= 0 || full_nxy < max_nxy) {
    return 0;
  }
  double ratio = sqrt(full_nxy / max_nxy);
  if (ratio < 1.5) {
    return 0;
  }
  
  // uncompress as required
  int compression_type = _fhdr.compression_type;
  bool recompress = false;
  if (isCompressed()) {
    if (decompress()) {
      _errStr += "ERROR - MdvxField::decimate\n";
      return -1;
    }
    recompress = true;
  }

  // compute sizes
  int elSize = _fhdr.data_element_nbytes;
  int nPixSourcePlane = _fhdr.nx * _fhdr.ny;
  int outNx = (_fhdr.nx / ratio) + 1;
  int outNy = (_fhdr.ny / ratio) + 1;
  double x_ratio = (double)_fhdr.nx / outNx;
  double y_ratio = (double)_fhdr.ny / outNy;
  int x_pix_count = x_ratio + 1; // Pixels input in X direction per output pixel
  int y_pix_count = y_ratio + 1; // Pixels input in Y direction per output pixel
  
  int64_t nPixTargetRow = outNx;
  int64_t nPixTargetPlane = nPixTargetRow * outNy;
  int64_t nBytesTargetVol = nPixTargetPlane * _fhdr.nz * elSize;

  int64_t count = 0;
  int y_loc,x_loc;
  uint r_val,g_val,b_val,a_val;
  uint r_sum,g_sum,b_sum,a_sum;

  // set up working buffer
  MemBuf targetBuf;
  targetBuf.prepare(nBytesTargetVol);
  if ( targetBuf.getPtr() == NULL){
    _errStr += "ERROR - MdvxField::decimate\n";
    return -1;
  }
 
 
  // loop through z, decimating into target buffer
  ui32 *sourceVol = (ui32 *) _volBuf.getPtr();
  ui32 *targetVol = (ui32 *) targetBuf.getPtr();
  
  for (int iz = 0; iz < _fhdr.nz; iz++) {
    ui32 *sourcePlane = sourceVol + iz * nPixSourcePlane;
    ui32 *targetPlane = targetVol + iz * nPixTargetPlane;
    ui32 *target = targetPlane; // Start of the output array
    
    // loop through y output rows
    for (int outy = 0; outy < outNy;  outy++) {
      // loop through x output columns
      for (int outx = 0; outx < outNx; outx++ ) {
        // compute the starting source Row for the averaging.
        y_loc = outy * y_ratio;
        if(y_loc >= _fhdr.ny)  y_loc = _fhdr.ny - 1;

        // compute the starting source Column for the averaging.
        x_loc = outx * x_ratio;
        if(x_loc >= _fhdr.nx) x_loc = _fhdr.nx -1;
         
        // Compute the Value for the output Pixel
        r_sum = 0;
        g_sum = 0;
        b_sum = 0;
        a_sum = 0;
        count = 0;
        // Loop through y_pix_count rows in input
        for(int iny = 0; iny < y_pix_count; iny++) {
          ui32 *source = sourcePlane + (y_loc * _fhdr.nx) + x_loc;
          // Sum x_pix_count columns from each row
          for(int inx = 0 ; inx < x_pix_count; inx++) {
            r_sum += (*source >> 24) & 0xFF;
            g_sum += (*source >> 16) & 0xFF;
            b_sum += (*source >> 8) & 0xFF;
            a_sum += *source & 0xFF;
            source++;  // move to the next input pixel
            count++;   // keep track of how many pixels are in the sums
          } // input data column loop
          y_loc++;
          if(y_loc >= _fhdr.ny ) y_loc = _fhdr.ny - 1;
        } // input data row loop

        if ( count != 0) {
          r_val =  (double) r_sum / count;
          g_val =  (double) g_sum / count;
          b_val =  (double) b_sum / count;
          a_val =  (double) a_sum / count;

          *target = (r_val << 24) + (g_val << 16) + (b_val << 8) + a_val;
        }
        else {
          *target = 0;
        }

        target++; // move to the next output pixel
      } 
    } 

  } // iz
  // set the field header appropriately
  
  _fhdr.volume_size = nBytesTargetVol;
  _fhdr.nx = outNx;
  _fhdr.ny = outNy;
  _fhdr.grid_minx += x_ratio * 0.5 * _fhdr.grid_dx;
  _fhdr.grid_miny += y_ratio * 0.5 * _fhdr.grid_dy;
  _fhdr.grid_dx *= x_ratio;
  _fhdr.grid_dy *= y_ratio;

  // copy the target buf to the vol buf

  _volBuf = targetBuf;

  if (recompress) {
    requestCompression(compression_type);
  }

  return 0;

}

///////////////////////////////////////////////////////////////
// compress the data volume if compression has previously
// been requested
//
// Compressed buffer is stored in BE byte order.
//
// returns 0 on success, -1 on failure

int MdvxField::compressIfRequested() const
{
  if (_fhdr.requested_compression == Mdvx::COMPRESSION_NONE) {
    return 0;
  }
  return compress(_fhdr.requested_compression);
}

int MdvxField::compressIfRequested64() const
{
  if (_fhdr.requested_compression == Mdvx::COMPRESSION_NONE) {
    return 0;
  }
  return compress64(_fhdr.requested_compression);
}

///////////////////////////////////////////////////////////////
// compress the data volume
//
// The compressed buffer comprises the following in order:
//   Array of nz ui32s: compressed plane offsets
//   Array of nz ui32s: compressed plane sizes
//   All of the compressed planes packed together.
//
// Compressed buffer is stored in BE byte order.
//
// returns 0 on success, -1 on failure

int MdvxField::compress(int compression_type) const

{

  if (!_volbufSizeValid()) {
    _errStr += "ERROR - MdvxField::compress.\n";
    _errStr +=  "  volBuf not allocated.\n";
    return -1;
  }
  
  // check if we are already properly compressed
  // now use gzip for all compression

  if (compression_type == Mdvx::COMPRESSION_ASIS) {
    return 0;
  }
  
  if (compression_type == Mdvx::COMPRESSION_GZIP_VOL &&
      _fhdr.compression_type == Mdvx::COMPRESSION_GZIP_VOL) {
    return 0;
  }

  if (compression_type == Mdvx::COMPRESSION_GZIP &&
      _fhdr.compression_type == Mdvx::COMPRESSION_GZIP) {
    return 0;
  }

  // uncompress

  if (decompress()) {
    return -1;
  }
  
  if (compression_type == Mdvx::COMPRESSION_NONE) {
    // no compression
    return 0;
  }

  if (compression_type == Mdvx::COMPRESSION_GZIP_VOL) {
    // special case
    return _compressGzipVol();
  }

  // proceed with gzip compression

  int nz = _fhdr.nz;
  int64_t npoints_plane = _fhdr.nx * _fhdr.ny;
  int64_t nbytes_plane = npoints_plane * _fhdr.data_element_nbytes;
  int64_t nbytes_vol = nbytes_plane * nz;
  int64_t next_offset = 0;

  // swap volume data to BE as appropriate

  buffer_to_BE(_volBuf.getPtr(), nbytes_vol, _fhdr.encoding_type);

  // create working buffer
  
  MemBuf workBuf;

  // compress plane-by-plane
  
  ui32 plane_offsets[MDV_MAX_VLEVELS];
  ui32 plane_sizes[MDV_MAX_VLEVELS];

  for (int iz = 0; iz < nz; iz++) {

    // only use GZIP compression - all others are deprecated
    
    void *uncompressed_plane = ((char *) _volBuf.getPtr() + iz * nbytes_plane);
    ui64 nbytes_compressed;
    void *compressed_plane = ta_compress(TA_COMPRESSION_GZIP,
                                         uncompressed_plane,
                                         nbytes_plane,
                                         &nbytes_compressed);
    
    if (compressed_plane == NULL) {
      _errStr += "ERROR - MdvxField::_compress.\n";
      _errStr +=  "  Compression failed.\n";
      return -1;
    }

    plane_offsets[iz] = next_offset;
    plane_sizes[iz] = nbytes_compressed;
    
    workBuf.add(compressed_plane, nbytes_compressed);
    ta_compress_free(compressed_plane);
    next_offset += nbytes_compressed;

  } // iz

  // swap plane offset and size arrays

  int64_t index_array_size = nz * sizeof(ui32);
  BE_from_array_32(plane_offsets, index_array_size);
  BE_from_array_32(plane_sizes, index_array_size);

  // assemble compressed buffer
  
  _volBuf.free();
  _volBuf.add(plane_offsets, index_array_size);
  _volBuf.add(plane_sizes, index_array_size);
  _volBuf.add(workBuf.getPtr(), workBuf.getLen());

  // adjust header

  _fhdr.compression_type = Mdvx::COMPRESSION_GZIP;
  _fhdr.volume_size = next_offset + 2 * index_array_size;

  return 0;

}

///////////////////////////////////////////////////////////////
// compress the data volume - 64-bit version
//
// The compressed buffer comprises the following in order:
//   Flags_64[2]: 2 x ui32: each with 0x64646464U - indicates 64 bit
//   Array of nz ui64s: compressed plane offsets
//   Array of nz ui64s: compressed plane sizes
//   All of the compressed planes packed together.
//
// Compressed buffer is stored in BE byte order.
//
// returns 0 on success, -1 on failure

int MdvxField::compress64(int compression_type) const

{

  if (!_volbufSizeValid()) {
    _errStr += "ERROR - MdvxField::compress64.\n";
    _errStr +=  "  volBuf not allocated.\n";
    return -1;
  }
  
  // check if we are already properly compressed
  // now use gzip for all compression

  if (compression_type == Mdvx::COMPRESSION_ASIS) {
    return 0;
  }
  
  if (compression_type == Mdvx::COMPRESSION_GZIP_VOL &&
      _fhdr.compression_type == Mdvx::COMPRESSION_GZIP_VOL) {
    return 0;
  }

  if (compression_type == Mdvx::COMPRESSION_GZIP &&
      _fhdr.compression_type == Mdvx::COMPRESSION_GZIP) {
    return 0;
  }

  // uncompress

  if (decompress()) {
    return -1;
  }
  
  if (compression_type == Mdvx::COMPRESSION_NONE) {
    // no compression
    return 0;
  }

  if (compression_type == Mdvx::COMPRESSION_GZIP_VOL) {
    // special case
    return _compressGzipVol();
  }

  // proceed with gzip compression

  ui32 flags64[2] = { MDV_FLAG_64, MDV_FLAG_64 };
  int nz = _fhdr.nz;
  int64_t npoints_plane = _fhdr.nx * _fhdr.ny;
  int64_t nbytes_plane = npoints_plane * _fhdr.data_element_nbytes;
  int64_t nbytes_vol = nbytes_plane * nz;
  int64_t next_offset = 0;

  // swap volume data to BE as appropriate

  buffer_to_BE(_volBuf.getPtr(), nbytes_vol, _fhdr.encoding_type);

  // create working buffer
  
  MemBuf workBuf;
  
  // compress plane-by-plane
  
  ui64 plane_offsets[MDV_MAX_VLEVELS];
  ui64 plane_sizes[MDV_MAX_VLEVELS];

  for (int iz = 0; iz < nz; iz++) {

    // only use GZIP compression - all others are deprecated
    
    void *uncompressed_plane = ((char *) _volBuf.getPtr() + iz * nbytes_plane);
    ui64 nbytes_compressed;
    void *compressed_plane = ta_compress(TA_COMPRESSION_GZIP,
                                         uncompressed_plane,
                                         nbytes_plane,
                                         &nbytes_compressed);
    
    if (compressed_plane == NULL) {
      _errStr += "ERROR - MdvxField::_compress.\n";
      _errStr +=  "  Compression failed.\n";
      return -1;
    }

    plane_offsets[iz] = next_offset;
    plane_sizes[iz] = nbytes_compressed;
    
    workBuf.add(compressed_plane, nbytes_compressed);
    ta_compress_free(compressed_plane);
    next_offset += nbytes_compressed;

  } // iz

  // swap plane offset and size arrays

  int64_t index_array_size = nz * sizeof(ui64);
  BE_from_array_64(plane_offsets, index_array_size);
  BE_from_array_64(plane_sizes, index_array_size);

  // assemble compressed buffer
  
  _volBuf.free();
  _volBuf.add(flags64, sizeof(flags64));
  _volBuf.add(plane_offsets, index_array_size);
  _volBuf.add(plane_sizes, index_array_size);
  _volBuf.add(workBuf.getPtr(), workBuf.getLen());

  // adjust header

  _fhdr.compression_type = Mdvx::COMPRESSION_GZIP;
  _fhdr.volume_size = next_offset + 2 * index_array_size;

  return 0;

}

///////////////////////////////////////////////////////////////
// compress the data volume into a single GZIP buffer
//
// Compressed buffer is stored in BE byte order.
//
// returns 0 on success, -1 on failure

int MdvxField::_compressGzipVol() const

{

  int64_t npoints_plane = _fhdr.nx * _fhdr.ny;
  int64_t nbytes_plane = npoints_plane * _fhdr.data_element_nbytes;
  int64_t nbytes_vol = nbytes_plane * _fhdr.nz;

  // swap volume data to BE as appropriate

  buffer_to_BE(_volBuf.getPtr(), nbytes_vol, _fhdr.encoding_type);

  // compress vol into single buffer
  
  void *uncompressed_vol = _volBuf.getPtr();
  ui64 nbytes_compressed;
  void *compressed_vol = ta_compress(TA_COMPRESSION_GZIP,
                                     uncompressed_vol,
                                     nbytes_vol,
                                     &nbytes_compressed);
  
  if (compressed_vol == NULL) {
    _errStr += "ERROR - MdvxField::_compressGzipVol.\n";
    _errStr +=  "  Compression failed.\n";
    return -1;
  }
  
  // load compressed buffer into volume buffer
  
  _volBuf.free();
  _volBuf.add(compressed_vol, nbytes_compressed);

  // free up

  ta_compress_free(compressed_vol);
  
  // adjust header

  _fhdr.compression_type = Mdvx::COMPRESSION_GZIP_VOL;
  _fhdr.volume_size = nbytes_compressed;

  return 0;

}

///////////////////////////////////////////////////////////////
// decompress the field
//
// Decompresses the volume buffer if necessary
//
// returns 0 on success, -1 on failure

int MdvxField::decompress() const

{

  if (!_volbufSizeValid()) {
    _errStr += "ERROR - MdvxField::decompress.\n";
    _errStr +=  "  volBuf not allocated.\n";
    return -1;
  }

  // check for compression
  
  if (!isCompressed()) {
    return 0;
  }

  // check for GzipVol compression

  if (ta_gzip_buffer(_volBuf.getPtr())) {
    return _decompressGzipVol();
  }

  // check for 64-bit

  ui32 flags64[2] = { MDV_FLAG_64, MDV_FLAG_64 };
  if (_volBuf.getLen() >= sizeof(flags64)) {
    memcpy(flags64, _volBuf.getPtr(), sizeof(flags64));
    if (flags64[0] == MDV_FLAG_64 &&
        flags64[1] == MDV_FLAG_64) {
      return _decompress64();
    }
  }
  
  int nz = _fhdr.nz;
  int64_t npoints_plane = _fhdr.nx * _fhdr.ny;
  int64_t nbytes_plane = npoints_plane * _fhdr.data_element_nbytes;
  int64_t nbytes_vol = nz * nbytes_plane;
  int64_t index_array_size =  nz * sizeof(ui32);

  ui32 *plane_offsets = (ui32 *) _volBuf.getPtr();
  ui32 *plane_sizes = plane_offsets + nz;

  // swap offsets and sizes

  BE_to_array_32(plane_offsets, index_array_size);
  BE_to_array_32(plane_sizes, index_array_size);

  // create work buffer

  MemBuf workBuf;
  
  for (int iz = 0; iz < nz; iz++) {

    char *compressed_plane = NULL;
    void *uncompressed_plane = NULL;
    ui64 nbytes_uncompressed = 0;
    ui32 this_offset = plane_offsets[iz] + 2 * index_array_size;

    // check for valid offset

    if (this_offset > _volBuf.getLen() - 1) {
      _errStr += "ERROR - MdvxField::decompress.\n";
      char errstr[1024];
      snprintf(errstr, 1024,
               "  Field, plane: %s, %d\n", getFieldName(), iz);
      _errStr += errstr;
      snprintf(errstr, 1024,
               "  Bad field offset: %ud\n", this_offset);
      _errStr += errstr;
      return -1;
    }

    compressed_plane = ((char *) _volBuf.getPtr() + this_offset);
    
    uncompressed_plane =
      ta_decompress(compressed_plane, &nbytes_uncompressed);
    
    if (uncompressed_plane == NULL) {
      _errStr += "ERROR - MdvxField::decompress.\n";
      _errStr +=  "  Field not compressed.\n";
      return -1;
    }

    if ((int) nbytes_uncompressed != nbytes_plane) {
      _errStr += "ERROR - MdvxField::decompress.\n";
      _errStr +=  "  Wrong number of bytes in plane.\n";
      char errstr[128];
      sprintf(errstr, "  %ld expected, %ld found.\n",
	      (long) nbytes_plane, (long) nbytes_uncompressed);
      _errStr += errstr;
      ta_compress_free(uncompressed_plane);
      return -1;
    }
    
    workBuf.add(uncompressed_plane, nbytes_plane);
    ta_compress_free(uncompressed_plane);

  } // iz

  // check
  
  if ((int) workBuf.getLen() != nbytes_vol) {
    _errStr += "ERROR - MdvxField::decompress.\n";
    _errStr +=  "  Wrong number of bytes in vol.\n";
    char errstr[128];
    sprintf(errstr, "  %ld expected, %ld found.\n",
	    (long) nbytes_vol, (long) workBuf.getLen());
    _errStr += errstr;
    return -1;
  }
  
  // copy work buf to volume buf
  
  _volBuf.reset();
  _volBuf.add(workBuf.getPtr(), nbytes_vol);

  // swap volume data from BE as appropriate

  buffer_from_BE(_volBuf.getPtr(), nbytes_vol, _fhdr.encoding_type);

  // update header

  _fhdr.compression_type = Mdvx::COMPRESSION_NONE;
  _fhdr.volume_size = nbytes_vol;

  return 0;

}

///////////////////////////////////////////////////////////////
// decompress a field compressed with 64-bit
// Decompresses the volume buffer if necessary
// returns 0 on success, -1 on failure

int MdvxField::_decompress64() const

{

  int nz = _fhdr.nz;
  int64_t npoints_plane = _fhdr.nx * _fhdr.ny;
  int64_t nbytes_plane = npoints_plane * _fhdr.data_element_nbytes;
  int64_t nbytes_vol = nz * nbytes_plane;
  int64_t index_array_size =  nz * sizeof(ui64);

  // get plane offsets and sizes
  // these follow the 64-bit flag

  ui32 flags64[2];
  char *offsetsStart = (char *) _volBuf.getPtr() + sizeof(flags64);
  
  ui64 *plane_offsets = (ui64 *) offsetsStart;
  ui64 *plane_sizes = plane_offsets + nz;

  // swap offsets and sizes

  BE_to_array_64(plane_offsets, index_array_size);
  BE_to_array_64(plane_sizes, index_array_size);

  // create work buffer

  MemBuf workBuf;
  
  for (int iz = 0; iz < nz; iz++) {
    
    char *compressed_plane;
    void *uncompressed_plane;
    ui64 nbytes_uncompressed;
    ui64 this_offset = plane_offsets[iz] + sizeof(flags64) + 2 * index_array_size;

    // check for valid offset

    if (this_offset > _volBuf.getLen() - 1) {
      _errStr += "ERROR - MdvxField::decompress64.\n";
      char errstr[1024];
      snprintf(errstr, 1024,
               "  Field, plane: %s, %d\n", getFieldName(), iz);
      _errStr += errstr;
      snprintf(errstr, 1024,
               "  Bad field offset: %lu\n", (unsigned long) this_offset);
      _errStr += errstr;
      return -1;
    }

    compressed_plane = ((char *) _volBuf.getPtr() + this_offset);
    
    uncompressed_plane =
      ta_decompress(compressed_plane, &nbytes_uncompressed);
    
    if (uncompressed_plane == NULL) {
      _errStr += "ERROR - MdvxField::decompress64.\n";
      _errStr +=  "  Field not compressed.\n";
      return -1;
    }

    if ((int) nbytes_uncompressed != nbytes_plane) {
      _errStr += "ERROR - MdvxField::decompress64.\n";
      _errStr +=  "  Wrong number of bytes in plane.\n";
      char errstr[1024];
      snprintf(errstr, 1024, "  %ld expected, %ld found.\n",
               (long) nbytes_plane, (long) nbytes_uncompressed);
      _errStr += errstr;
      ta_compress_free(uncompressed_plane);
      return -1;
    }
    
    workBuf.add(uncompressed_plane, nbytes_plane);
    ta_compress_free(uncompressed_plane);

  } // iz

  // check
  
  if ((int) workBuf.getLen() != nbytes_vol) {
    _errStr += "ERROR - MdvxField::decompress.\n";
    _errStr +=  "  Wrong number of bytes in vol.\n";
    char errstr[1024];
    snprintf(errstr, 1024, "  %ld expected, %ld found.\n",
             (long) nbytes_vol, (long) workBuf.getLen());
    _errStr += errstr;
    return -1;
  }
  
  // copy work buf to volume buf
  
  _volBuf.reset();
  _volBuf.add(workBuf.getPtr(), nbytes_vol);

  // swap volume data from BE as appropriate

  buffer_from_BE(_volBuf.getPtr(), nbytes_vol, _fhdr.encoding_type);

  // update header

  _fhdr.compression_type = Mdvx::COMPRESSION_NONE;
  _fhdr.volume_size = nbytes_vol;

  return 0;

}

///////////////////////////////////////////////////////////////
// decompress field which has been compressed with GZIP_VOL.
// This has a single compressed buffer for the volume.
//
// returns 0 on success, -1 on failure

int MdvxField::_decompressGzipVol() const
  
{

  int64_t npoints_plane = _fhdr.nx * _fhdr.ny;
  int64_t nbytes_plane = npoints_plane * _fhdr.data_element_nbytes;
  int64_t nbytes_vol = _fhdr.nz * nbytes_plane;

  // uncompress buffer
  
  void *compressed_vol = _volBuf.getPtr();
  ui64 nbytes_uncompressed;
  void *uncompressed_vol =
    ta_decompress(compressed_vol, &nbytes_uncompressed);
    
  if (uncompressed_vol == NULL) {
    _errStr += "ERROR - MdvxField::_decompressGzipVol.\n";
    _errStr +=  "  Compression type not recognized.\n";
    return -1;
  }

  // check size

  if ((int) nbytes_uncompressed != nbytes_vol) {
    _errStr += "ERROR - MdvxField::_decompressGzipVol.\n";
    _errStr +=  "  Wrong number of bytes in vol.\n";
    char errstr[1024];
    snprintf(errstr, 1024, "  %ld expected, %ld found.\n",
             (long) nbytes_vol, (long) nbytes_uncompressed);
    _errStr += errstr;
    ta_compress_free(uncompressed_vol);
    return -1;
  }
  
  // copy work buf to volume buf
  
  _volBuf.reset();
  _volBuf.add(uncompressed_vol, nbytes_vol);

  // free up
  
  ta_compress_free(uncompressed_vol);
  
  // swap volume data from BE as appropriate
  
  buffer_from_BE(_volBuf.getPtr(), nbytes_vol, _fhdr.encoding_type);

  // update header

  _fhdr.compression_type = Mdvx::COMPRESSION_NONE;
  _fhdr.volume_size = nbytes_vol;
  
  return 0;

}

////////////////////////////
// _set_data_element_nbytes
//
// Set the number of bytes per element

void MdvxField::_set_data_element_nbytes()

{

  if (_fhdr.encoding_type == Mdvx::ENCODING_INT8) {
    _fhdr.data_element_nbytes = 1;
  } else if (_fhdr.encoding_type == Mdvx::ENCODING_INT16) {
    _fhdr.data_element_nbytes = 2;
  } else if (_fhdr.encoding_type == Mdvx::ENCODING_FLOAT32) {
    _fhdr.data_element_nbytes = 4;
  } else if (_fhdr.encoding_type == Mdvx::ENCODING_RGBA32) {
    _fhdr.data_element_nbytes = 4;
  }

}

//////////////////////
// computeMinAndMax
//
// Set min and max value for the field, if not already done
//
// Returns 0 on success, -1 on failure

int MdvxField::computeMinAndMax(bool force /* = false*/ ) const

{
  
  if (!_volbufSizeValid()) {
    _errStr += "ERROR - MdvxField::computeMinAndMax()\n";
    _errStr +=  "  volBuf not allocated.\n";
    return -1;
  }
  
  if (_fhdr.min_value != 0.0 || _fhdr.max_value != 0.0) {
    if (_fhdr.min_value_orig_vol == 0 && _fhdr.max_value_orig_vol == 0) {
      _fhdr.min_value_orig_vol = _fhdr.min_value;
      _fhdr.max_value_orig_vol = _fhdr.max_value;
    }
  }

  // don't do it if already done
  
  if (!force) {
    if ((_fhdr.min_value != 0.0 || _fhdr.max_value != 0.0) && 
        (_fhdr.min_value != _fhdr.max_value) &&
        !isfinite(_fhdr.min_value) &&
        !isfinite(_fhdr.max_value)) {
      return 0;
    }
  }

  // check we do not have nans or inf

  _check_finite(_volBuf.getPtr());

  // decompress data if necessary

  int compression_type = _fhdr.compression_type;
  bool wasCompressed = false;
  if (isCompressed()) {
    if (decompress()) {
      _errStr += "ERROR - MdvxField::computeMinAndMax\n";
      return -1;
    }
    wasCompressed = true;
  }

  int64_t npoints = _fhdr.nx * _fhdr.ny * _fhdr.nz;

  if (_fhdr.encoding_type == Mdvx::ENCODING_INT8) {
    
    ui08 *val = (ui08 *) _volBuf.getPtr();
    ui08 min_val = 255;
    ui08 max_val = 0;
    ui08 missing = (ui08) _fhdr.missing_data_value;
    ui08 bad = (ui08) _fhdr.bad_data_value;
    
    for (int64_t i = 0; i < npoints; i++, val++) {
      ui08 this_val = *val;
      if (this_val != missing && this_val != bad) {
	min_val = MIN(min_val, this_val);
	max_val = MAX(max_val, this_val);
      }
    }
    
    if (min_val <= max_val) {
      _fhdr.min_value =
	min_val * _fhdr.scale + _fhdr.bias;
      _fhdr.max_value =
	max_val * _fhdr.scale + _fhdr.bias;
    }

  } else if (_fhdr.encoding_type == Mdvx::ENCODING_INT16) {
    
    ui16 *val = (ui16 *) _volBuf.getPtr();
    ui16 min_val = 65535;
    ui16 max_val = 0;
    ui16 missing = (ui16) _fhdr.missing_data_value;
    ui16 bad = (ui16) _fhdr.bad_data_value;
   
    for (int64_t i = 0; i < npoints; i++, val++) {
      ui16 this_val = *val;
      if (this_val != missing && this_val != bad) {
	min_val = MIN(min_val, this_val);
	max_val = MAX(max_val, this_val);
      }
    }

    if (min_val <= max_val) {
      _fhdr.min_value =
	min_val * _fhdr.scale + _fhdr.bias;
      _fhdr.max_value =
	max_val * _fhdr.scale + _fhdr.bias;
    }

  } else if (_fhdr.encoding_type == Mdvx::ENCODING_FLOAT32) {

    fl32 *val = (fl32 *) _volBuf.getPtr();
    fl32 min_val = numeric_limits<float>::max(); //1.0e99;
    fl32 max_val = -1 * numeric_limits<float>::max(); //-1.0e99;
    fl32 missing = _fhdr.missing_data_value;
    fl32 bad = _fhdr.bad_data_value;
    
    for (int64_t i = 0; i < npoints; i++, val++) {
      fl32 this_val = *val;
      if (this_val != missing && this_val != bad) {
	min_val = MIN(min_val, this_val);
	max_val = MAX(max_val, this_val);
      }
    }

    if (min_val <= max_val) {
      _fhdr.min_value = min_val;
      _fhdr.max_value = max_val;
    }

  } else if (_fhdr.encoding_type == Mdvx::ENCODING_RGBA32) {
    
    ui32 *val = (ui32 *) _volBuf.getPtr();
    ui32 min_val = 0xffffffff;
    ui32 max_val = 0;
    ui32 missing = (ui32) _fhdr.missing_data_value;
    ui32 bad = (ui32) _fhdr.bad_data_value;
    
    for (int64_t i = 0; i < npoints; i++, val++) {
      ui32 this_val = *val;
      if (this_val != missing && this_val != bad) {
	min_val = MIN(min_val, this_val);
	max_val = MAX(max_val, this_val);
      }
    }
    
    if (min_val <= max_val) {
      _fhdr.min_value = min_val;
      _fhdr.max_value = max_val;
    }

  }

  if (_fhdr.min_value_orig_vol == 0 && _fhdr.max_value_orig_vol == 0) {
    _fhdr.min_value_orig_vol = _fhdr.min_value;
    _fhdr.max_value_orig_vol = _fhdr.max_value;
  }

  if (_fhdr.volume_size == 0) {
    cerr << "WARNING - MdvxField::computeMinAndMax" << endl;
    cerr << "  Field name: " << _fhdr.field_name << endl;
    cerr << "  Volume size: " << _fhdr.volume_size << endl;
  }
  
  // set compression back

  if (wasCompressed) {
    requestCompression(compression_type);
  }

  return 0;

}

//////////////////////////////////////////
// round a value up to the next 2, 5 or 10

double MdvxField::_round_up(double z)

{

  double _log2 = log10(2.0);
  double _log5 = log10(5.0);

  double l = log10(z);
  double L = l - (-1000.0);

  double integral;
  double fract = modf(L, &integral);

  double rfract;
  if (fract == 0.0) {
    rfract = 0.0;
  } else if (fract <= _log2) {
    rfract = _log2;
  } else if (fract <= _log5) {
    rfract = _log5;
  } else {
    rfract = 1.0;
  }

  double R = integral + rfract - 1000.0;
  
  double r = pow(10.0, R);

  return r;

}

//////////////////////////////////////////////////////////////////////////
//
// Read a field volume from a file. The data offset and size in the
// field header are used, therefore the field header must be correct.
//
// If mdvx is not NULL, the horizontal and vertical read parameters
// in the mdvx object are used to constrain the read.
//
// If fill_missing is set and fewer than 20 % of the grid points in a
// plane are missing, the missing vals are filled in by copying in
// from adjacent points.
//
// The volume data is read into the volBuf, and then swapped if appropriate.
//
// Returns 0 on success, -1 on failure.
//

int MdvxField::_read_volume(TaFile &infile,
			    Mdvx &mdvx,
			    bool fill_missing,
			    bool do_decimate,
			    bool do_final_convert,
			    MdvxRemapLut &remapLut,
			    bool is_vsection,
			    double vsection_min_lon,
			    double vsection_max_lon)
  
{

  clearErrStr();
  if (infile.fseek(_fhdr.field_data_offset, SEEK_SET)) {
    _errStr += "ERROR - MdvxField::_read_volume\n";
    _errStr += "  Cannot read field: ";
    _errStr += _fhdr.field_name;
    _errStr += "\n";
    return -1;
  }

  size_t volume_size = _fhdr.volume_size;
  void *buf = _volBuf.prepare(volume_size);

  if (infile.fread(buf, 1, volume_size) != volume_size) {
    _errStr += "ERROR - MdvxField::_read_volume\n";
    _errStr += "  Cannot read field: ";
    _errStr += _fhdr.field_name;
    _errStr += "\n";
    return -1;
  }

  // byte swap as needed

  _data_from_BE(_fhdr, _volBuf.getPtr(), _volBuf.getLen());

  // set headers exactly as in file

  setFieldHeaderFile(_fhdr);
  setVlevelHeaderFile(_vhdr);

  // convert according to the read request

  if (_apply_read_constraints(mdvx, fill_missing, do_decimate,
                              do_final_convert,
                              remapLut, is_vsection,
                              vsection_min_lon, vsection_max_lon)) {
    _errStr += "ERROR - MdvxField::_read_volume\n";
    return -1;
  }
    
  return 0;

}

//////////////////////////////////////////////////////////////////////////
//
// convert a field after reading in the data
//
// If fill_missing is set and fewer than 20 % of the grid points in a
// plane are missing, the missing vals are filled in by copying in
// from adjacent points.
//
// Returns 0 on success, -1 on failure.

int MdvxField::_apply_read_constraints(const Mdvx &mdvx,
                                       bool fill_missing,
                                       bool do_decimate,
                                       bool do_final_convert,
                                       MdvxRemapLut &remapLut,
                                       bool is_vsection,
                                       double vsection_min_lon,
                                       double vsection_max_lon)
  
{

  // set output compression type

  Mdvx::compression_type_t output_compression = mdvx._readCompressionType;
  if (output_compression == Mdvx::COMPRESSION_ASIS) {
    output_compression =
      (Mdvx::compression_type_t) _fhdr.compression_type;
  }

  // decompress if it makes sense
  
  if (mdvx._readComposite || mdvx._readHorizLimitsSet || mdvx._readRemapSet ||
      do_decimate || fill_missing || is_vsection ||
      (_fhdr.min_value == 0.0 && _fhdr.max_value == 0.0)) {
    if (decompress()) {
      _errStr += "ERROR - MdvxField::_apply_read_constraints\n";
      _errStr += "  Error decompressing volume\n";
      return -1;
    }
  }
  
  // compute min and max

  computeMinAndMax(true);
    
  // convert vlevel type if needed

  if (mdvx._readSpecifyVlevelType) {
    convertVlevelType(mdvx._readVlevelType);
  }
  
  // composite if needed

  if (mdvx._readComposite) {
    int iret;
    if (mdvx._readVlevelLimitsSet) {
      iret = convert2Composite(mdvx._readMinVlevel, mdvx._readMaxVlevel);
    } else if (mdvx._readPlaneNumLimitsSet) {
      iret = convert2Composite(mdvx._readMinPlaneNum, mdvx._readMaxPlaneNum);
    } else {
      iret = convert2Composite();
    }
    if (iret) {
      _errStr += "ERROR - MdvxField::_apply_read_constraints\n";
      return -1;
    }
    
  } else {
    // constrain in the vertical if needed
    
    if (mdvx._readVlevelLimitsSet || mdvx._readPlaneNumLimitsSet) {
      constrainVertical(mdvx);
    }

  } // if (mdvx._readComposite) 

  // For latlon grids, might need to shift the lon domain

  if (_fhdr.proj_type == Mdvx::PROJ_LATLON) {
    if (is_vsection) {
      _check_lon_domain(vsection_min_lon, vsection_max_lon);
    } else if (mdvx._readHorizLimitsSet) {
      _check_lon_domain(mdvx._readMinLon, mdvx._readMaxLon);
    }
  }

  // constrain in the horizontal if needed
  
  if (mdvx._readHorizLimitsSet && !is_vsection) {
    constrainHorizontal(mdvx);
  }

  // fill missing value as required

  if (fill_missing) {
    if (_planes_fill_missing()) {
      _errStr += "ERROR - MdvxField::_apply_read_constraints\n";
      return -1;
    }
  }

  // decimate if required

  if (do_decimate) {
    if (decimate(mdvx._readDecimateMaxNxy)) {
      _errStr += "ERROR - MdvxField::_apply_read_constraints.\n";
      _errStr += "  Decimating field.\n";
      return -1;
    }
  }
  
  // remap if required
  
  if (mdvx._readRemapSet) {
    MdvxProj proj(mdvx._readRemapCoords);
    if (remap(remapLut, proj)) {
      _errStr += "ERROR - MdvxField::_apply_read_constraints.\n";
      _errStr += "  Remapping field.\n";
      return -1;
    }
  }

  if (mdvx._readAutoRemap2LatLon) {
    if (autoRemap2Latlon(remapLut)) {
      _errStr += "ERROR - MdvxField::_apply_read_constraints.\n";
      _errStr += "  Remapping field.\n";
      return -1;
    }
  }
  
  // convert as required

  if (do_final_convert) {
    if (convertType(mdvx._readEncodingType,
		    output_compression,
		    mdvx._readScalingType,
		    mdvx._readScale,
		    mdvx._readBias)) {
      _errStr += "ERROR - MdvxField::_apply_read_constraints\n";
      return -1;
    }
  }

  return 0;

}

/////////////////////////////////////////////////////////////////////////
//
// Write field data volume to a file.
//
// The volume data is swapped as appropriate, and then written to
// the file.
//
// Passed in is 'this_offset', the starting offset for the write.
//
// Side effects:
//  1: field_data_offset is set in the field header.
//  2: arg next_offset is set - it is the offset for the next write.
//
// Returns 0 on success, -1 on failure.

int MdvxField::_write_volume(TaFile &outfile,
			     int64_t this_offset,
			     int64_t &next_offset) const

{

  clearErrStr();

  // compute min and max

  computeMinAndMax(true);

  // compress if previously requested

  compressIfRequested();

  // get sizes

  int64_t volume_size = _fhdr.volume_size;
  ui32 be_volume_size = BE_from_ui32(volume_size);

  // Set the constant values in the headers

  _fhdr.record_len1 = sizeof(Mdvx::field_header_t) - (2 * sizeof(si32));
  _fhdr.struct_id = Mdvx::FIELD_HEAD_MAGIC_COOKIE_64;
  _fhdr.record_len2 = _fhdr.record_len1;
  
  _vhdr.record_len1 = sizeof(Mdvx::vlevel_header_t) - (2 * sizeof(si32));
  _vhdr.struct_id = Mdvx::VLEVEL_HEAD_MAGIC_COOKIE_64;
  _vhdr.record_len2 = _vhdr.record_len1;
  
  // set data offset in header, and next offset

  _fhdr.field_data_offset = this_offset + sizeof(ui32);
  next_offset = this_offset + volume_size + 2 * sizeof(ui32);

  // make copy of the data

  MemBuf copyBuf = _volBuf;

  // swap data to BE byte ordering as appropriate
  
  if (!isCompressed()) {
    buffer_to_BE(copyBuf.getPtr(), copyBuf.getLen(), _fhdr.encoding_type);
  }

  // seek to starting offset

  if (outfile.fseek(this_offset, SEEK_SET) != 0) {
    _errStr += "ERROR - MdvxField::_write_volume.\n";
    char errstr[1024];
    snprintf(errstr, 1024,
             "  Seeking volume data at this_offset %ld\n", (long) this_offset);
    _errStr += errstr;
    _errStr += " Field name: ";
    _errStr += _fhdr.field_name;
    _errStr += "\n";
    return -1;
  }

  // Write the leading FORTRAN record
  
  ssize_t nwritten = outfile.fwrite(&be_volume_size, sizeof(be_volume_size), 1);
  if (nwritten != 1) {
    _errStr += "ERROR - MdvxField::writeVol\n";
    _errStr += "  Cannot write begin fortran len for field: ";
    _errStr += _fhdr.field_name;
    _errStr += "\n";
    return -1;
  }

  // Write out the data.

  nwritten = outfile.fwrite(copyBuf.getPtr(), 1, volume_size);
  if (nwritten != volume_size) {
    int errNum = errno;
    char errstr[1024];
    _errStr += "ERROR - MdvxField::writeVol\n";
    _errStr += string("  Cannot write data for field: ")
      + _fhdr.field_name + "\n";
    snprintf(errstr, 1024, "%ld", (long) copyBuf.getLen());
    _errStr += string("    copyBuf has ") + errstr + " bytes\n";
    snprintf(errstr, 1024, "%ld", (long) volume_size);
    _errStr += string("    should have ") + errstr + " bytes.\n";
    snprintf(errstr, 1024, "%ld", (long) nwritten);
    _errStr += string("    nwritten: ") + errstr + " bytes.\n";
    _errStr += strerror(errNum);
    _errStr += "\n";
    return -1;
  }
  
  // Write the trailing FORTRAN record to disk.
  
  if (outfile.fwrite(&be_volume_size, sizeof(be_volume_size), 1) != 1) {
    _errStr += "ERROR - MdvxField::writeVol\n";
    _errStr += "  Cannot write end fortran len for field: ";
    _errStr += _fhdr.field_name;
    _errStr += "\n";
    return -1;
  }

  return 0;

}

//////////////////////////////////////
// print volume data in verbose format

void MdvxField::_print_voldata_verbose(ostream &out,
				       bool print_labels)

{     
  
  int64_t npoints_plane = _fhdr.nx * _fhdr.ny;
  char outstr[32];
  
  switch (_fhdr.encoding_type) {

    case Mdvx::ENCODING_INT8: {
      ui08 *val = (ui08 *) _volBuf.getPtr();
      ui08 missing = (ui08) _fhdr.missing_data_value;
      ui08 bad = (ui08) _fhdr.bad_data_value;
      for (int iz = 0; iz < _fhdr.nz; iz++) {
        if (print_labels) {
          out << "INT8 data for plane " << iz << ":" << endl << endl;
        }
        for (int64_t i = 0; i < npoints_plane; i++, val++) {
          ui08 this_val = *val;
          if (this_val == bad) {
            out << "BAD ";
          } else if (this_val == missing) {
            out << "MISS ";
          } else {
            sprintf(outstr, "%3d ", *val);
            out << outstr;
          }
        }
        out << endl << endl;
      } // iz 
      break;
    }

    case Mdvx::ENCODING_INT16: {
      ui16 *val = (ui16 *) _volBuf.getPtr();
      ui16 missing = (ui16) _fhdr.missing_data_value;
      ui16 bad = (ui16) _fhdr.bad_data_value;
      for (int iz = 0; iz < _fhdr.nz; iz++) {
        if (print_labels) {
          out << "INT16 data for plane " << iz << ":" << endl << endl;
        }
        for (int64_t i = 0; i < npoints_plane; i++, val++) {
          ui16 this_val = *val;
          if (this_val == bad) {
            out << "BAD ";
          } else if (this_val == missing) {
            out << "MISS ";
          } else {
            sprintf(outstr, "%5d ", *val);
            out << outstr;
          }
        }
        out << endl << endl;
      } // iz
      break;
    }

    case Mdvx::ENCODING_FLOAT32: {
      fl32 *val = (fl32 *) _volBuf.getPtr();
      fl32 missing = _fhdr.missing_data_value;
      fl32 bad = _fhdr.bad_data_value;
      for (int iz = 0; iz < _fhdr.nz; iz++) {
        if (print_labels) {
          out << "FLOAT32 data for plane " << iz << ":" << endl << endl;
        }
        for (int64_t i = 0; i < npoints_plane; i++, val++) {
          fl32 this_val = *val;
          if (this_val == bad) {
            out << "BAD ";
          } else if (this_val == missing) {
            out << "MISS ";
          } else {
            if (fabs(*val) > 0.01) {
              sprintf(outstr, "%.3f ", *val);
            } else {
              sprintf(outstr, "%.3e ", *val);
            }
            out << outstr;
          }
        }
        out << endl << endl;
      } // iz
      break;
    }
  
    case Mdvx::ENCODING_RGBA32: {
      ui32 *val = (ui32 *) _volBuf.getPtr();
      ui32 missing = (ui32) _fhdr.missing_data_value;
      ui32 bad = (ui32) _fhdr.bad_data_value;
      for (int iz = 0; iz < _fhdr.nz; iz++) {
        if (print_labels) {
          out << "RGBA32 data for plane " << iz << ":" << endl << endl;
        }
        for (int64_t i = 0; i < npoints_plane; i++, val++) {
          ui32 this_val = *val;
          if (this_val == bad) {
            out << "BAD ";
          } else if (this_val == missing) {
            out << "MISS ";
          } else {
            sprintf(outstr, "%5x ", *val);
            out << outstr;
          }
        }
        out << endl << endl;
      } // iz
      break;
    }
  
  } // switch

}

/////////////////////////////////////
// print volume data in packed format

void MdvxField::_print_voldata_packed(ostream &out,
				      bool print_labels,
                                      bool printCanonical,
                                      int n_lines_print)

{     

  int64_t npoints_plane = _fhdr.nx * _fhdr.ny;
  int nlines = 0;

  switch (_fhdr.encoding_type) {
    
    case Mdvx::ENCODING_INT8: {
      ui08 *val = (ui08 *) _volBuf.getPtr();
      ui08 missing = (ui08) _fhdr.missing_data_value;
      ui08 bad = (ui08) _fhdr.bad_data_value;
    
      for (int iz = 0; iz < _fhdr.nz; iz++) {
        if (printCanonical) {
          out << "fullfield: encoding: INT8";
          out << "  field: " << _fhdr.field_name;
          out << "  plane: " << iz;
          out << endl;
        }
        if (print_labels) {
          out << endl;
          out << "npoints_plane: " << npoints_plane << endl;
          out << "INT8 data for plane " << iz << ":" << endl << endl;
        }
        int64_t printed = 0;
        int64_t count = 1;
        ui08 prev_val = *val;
        val++;
        for (int64_t i = 1; i < npoints_plane; i++, val++) {
          ui08 this_val = *val;
          if (this_val != prev_val) {
            if (printCanonical) out << "fulldata: ";
            _print_int8_packed(out, count, prev_val, bad, missing,
                               printCanonical);
            if (printCanonical) {
              out << endl;
              nlines++;
            }
            printed++;
            if (printed > 8) {
              if (!printCanonical) {
                out << endl;
                nlines++;
              }
              printed = 0;
            }
            prev_val = this_val;
            count = 1;
          } else {
            count++;
          }
          if (n_lines_print > 0 && nlines >= n_lines_print) {
            break;
          }
        } // i
        if (printCanonical) out << "fulldata: ";
        _print_int8_packed(out, count, prev_val, bad, missing, printCanonical);
        out << endl;
      } // iz

      break;
    }

    case Mdvx::ENCODING_INT16: {
      ui16 *val = (ui16 *) _volBuf.getPtr();
      ui16 missing = (ui16) _fhdr.missing_data_value;
      ui16 bad = (ui16) _fhdr.bad_data_value;
    
      for (int iz = 0; iz < _fhdr.nz; iz++) {
        if (printCanonical) {
          out << "fullfield: encoding: INT16";
          out << "  field: " << _fhdr.field_name;
          out << "  plane: " << iz;
          out << endl;
        }
        if (print_labels) {
          out << endl;
          out << "npoints_plane: " << npoints_plane << endl;
          out << "INT16 data for plane " << iz << ":" << endl << endl;
        }
        int printed = 0;
        int64_t count = 1;
        ui16 prev_val = *val;
        val++;
        for (int64_t i = 1; i < npoints_plane; i++, val++) {
          ui16 this_val = *val;
          if (this_val != prev_val) {
            if (printCanonical) out << "fulldata: ";
            _print_int16_packed(out, count, prev_val, bad, missing,
                                printCanonical);
            if (printCanonical) {
              out << endl;
              nlines++;
            }
            printed++;
            if (printed > 7) {
              if (!printCanonical) {
                out << endl;
                nlines++;
              }
              printed = 0;
            }
            prev_val = this_val;
            count = 1;
          } else {
            count++;
          }
          if (n_lines_print > 0 && nlines >= n_lines_print) {
            break;
          }
        } // i
        if (printCanonical) out << "fulldata: ";
        _print_int16_packed(out, count, prev_val, bad, missing, printCanonical);
        out << endl << endl;
      } // iz

      break;
    }
    
    case Mdvx::ENCODING_FLOAT32: {
    
      fl32 *val = (fl32 *) _volBuf.getPtr();
      fl32 missing = _fhdr.missing_data_value;
      fl32 bad = _fhdr.bad_data_value;
    
      for (int iz = 0; iz < _fhdr.nz; iz++) {
        if (printCanonical) {
          out << "fullfield: encoding: FLOAT32";
          out << "  field: " << _fhdr.field_name;
          out << "  plane: " << iz;
          out << endl;
        }

        if (print_labels) {
          out << endl;
          out << "npoints_plane: " << npoints_plane << endl;
          out << "FLOAT32 data for plane " << iz << ":" << endl << endl;
        }
        int printed = 0;
        int64_t count = 1;
        fl32 prev_val = *val;
        val++;
        for (int64_t i = 1; i < npoints_plane; i++, val++) {
          fl32 this_val = *val;
          if (this_val != prev_val) {
            if (printCanonical) out << "fulldata: ";
            _print_float32_packed(out, count, prev_val, bad, missing,
                                  printCanonical);
            if (printCanonical) {
              out << endl;
              nlines++;
            }
            printed++;
            if (printed > 6) {
              if (!printCanonical) {
                out << endl;
                nlines++;
              }
              printed = 0;
            }
            prev_val = this_val;
            count = 1;
          } else {
            count++;
          }
          if (n_lines_print > 0 && nlines >= n_lines_print) {
            break;
          }
        } // i
        if (printCanonical) out << "fulldata: ";
        _print_float32_packed(out, count, prev_val, bad, missing,
                              printCanonical);
        out << endl << endl;
      } // iz
    
      break;
    }
    
    case Mdvx::ENCODING_RGBA32: {
    
      ui32 *val = (ui32 *) _volBuf.getPtr();
      ui32 missing = (ui32) _fhdr.missing_data_value;
      ui32 bad = (ui32) _fhdr.bad_data_value;
    
      for (int iz = 0; iz < _fhdr.nz; iz++) {
        if (printCanonical) {
          out << "fullfield: encoding: RGBA32";
          out << "  field: " << _fhdr.field_name;
          out << "  plane: " << iz;
          out << endl;
        }
        if (print_labels) {
          out << "RGBA data for plane " << iz << ":" << endl << endl;
        }
        int printed = 0;
        int64_t count = 1;
        ui32 prev_val = *val;
        val++;
        for (int64_t i = 1; i < npoints_plane; i++, val++) {
          ui32 this_val = *val;
          if (this_val != prev_val) {
            if (printCanonical) out << "fulldata: ";
            _print_rgba32_packed(out, count, prev_val, bad, missing,
                                 printCanonical);
            if (printCanonical) {
              out << endl;
              nlines++;
            }
            printed++;
            if (printed > 6) {
              if (!printCanonical) {
                out << endl;
                nlines++;
              }
              printed = 0;
            }
            prev_val = this_val;
            count = 1;
          } else {
            count++;
          }
          if (n_lines_print > 0 && nlines >= n_lines_print) {
            break;
          }
        } // i
        if (printCanonical) out << "fulldata: ";
        _print_rgba32_packed(out, count, prev_val, bad, missing, printCanonical);
        out << endl << endl;
      } // iz
    
      break;
    }
    
  } // switch

}

/////////////////////////////////////////
// print int8 values in packed format

void MdvxField::_print_int8_packed(ostream &out, int count,
				   ui08 val, ui08 bad,
				   ui08 missing,
                                   bool printCanonical)

{

  char outstr[32];
  if (count > 1 || printCanonical) {
    out << count << "*";
  }
  if (val == missing) {
    out << "MISS ";
  } else if (val == bad) {
    out << "BAD ";
  } else {
    if (printCanonical) sprintf(outstr, "%d ", val);
    else sprintf(outstr, "%.3d ", val);
    out << outstr;
  }

}

/////////////////////////////////////////
// print float32 values in packed format

void MdvxField::_print_int16_packed(ostream &out, int count,
				    ui16 val, ui16 bad,
				    ui16 missing,
                                    bool printCanonical)
{

  char outstr[32];
  if (count > 1 || printCanonical) {
    out << count << "*";
  }
  if (val == missing) {
    out << "MISS ";
  } else if (val == bad) {
    out << "BAD ";
  } else {
    if (printCanonical) sprintf(outstr, "%d ", val);
    else sprintf(outstr, "%.5d ", val);
    out << outstr;
  }

}

/////////////////////////////////////////
// print float32 values in packed format

void MdvxField::_print_float32_packed(ostream &out, int count,
				      fl32 val, fl32 bad,
				      fl32 missing,
                                      bool printCanonical)
{

  char outstr[32];
  if (count > 1 || printCanonical) {
    out << count << "*";
  }
  if (val == missing) {
    out << "MISS ";
  } else if (val == bad) {
    out << "BAD ";
  } else {
    if (printCanonical) {
      sprintf(outstr, "%.17E", val);
      out << outstr;
    }
    else {
      if (fabs(val) > 0.01) {
        sprintf(outstr, "%.3f ", val);
        out << outstr;
      } else if (val == 0.0) {
        out << "0.0 ";
      } else {
        sprintf(outstr, "%.3e ", val);
        out << outstr;
      }
    }
  }

}

/////////////////////////////////////////
// print rgba32 values in packed format

void MdvxField::_print_rgba32_packed(ostream &out, int count,
				     ui32 val, ui32 bad,
				     ui32 missing,
                                     bool printCanonical)

{

  char outstr[32];
  if (count > 1 || printCanonical) {
    out << count << "*";
  }
  if (val == missing) {
    out << "MISS ";
  } else if (val == bad) {
    out << "BAD ";
  } else {
    if (printCanonical) sprintf(outstr, "%x ", val);
    else sprintf(outstr, "%.5x ", val);
    out << outstr;
  }

}

/////////////////////////////////////
// print time height profile

void MdvxField::_print_time_height(ostream &out,
				   const vector<time_t> &times)

{     

  out << "TIME-HEIGHT PROFILE - field name: " << _fhdr.field_name << endl;
  out << endl;

  string htLabel = "Heights in ";
  htLabel += Mdvx::vertTypeZUnits(_fhdr.vlevel_type);
  htLabel += ": ";
  // out << left << setw(20) << htLabel << right;
  out << setw(20) << htLabel;
  for (int iz = 0; iz < _fhdr.nz; iz++) {
    out << setw(10) << _vhdr.level[iz];
  } // iz
  out << endl;

  out << "Times" << endl;
  int nTimes = _fhdr.nx;
  if (nTimes > (int) times.size()) {
    nTimes = (int) times.size();
  }

  for (int itime = 0; itime < nTimes; itime++) {

    // out << left << setw(20) << DateTime::strn(times[itime]) << right;
    out << setw(20) << DateTime::strn(times[itime]);

    for (int iz = 0; iz < _fhdr.nz; iz++) {
      int index = iz * _fhdr.nx + itime;

      switch (_fhdr.encoding_type) {

        case Mdvx::ENCODING_INT8: {
          ui08 val = ((ui08 *) _volBuf.getPtr())[index];
          ui08 missing = (ui08) _fhdr.missing_data_value;
          if (val == missing) {
            out << setw(10) << "****";
          } else {
            out << setw(10) << setprecision(4) << val;
          }
          break;
        } // INT8

        case Mdvx::ENCODING_INT16: {
          ui16 val = ((ui16 *) _volBuf.getPtr())[index];
          ui16 missing = (ui16) _fhdr.missing_data_value;
          if (val == missing) {
            out << setw(10) << "****";
          } else {
            out << setw(10) << setprecision(4) << val;
          }
          break;
        } // INT16
      
        case Mdvx::ENCODING_FLOAT32: {
          fl32 val = ((fl32 *) _volBuf.getPtr())[index];
          fl32 missing = (fl32) _fhdr.missing_data_value;
          if (val == missing) {
            out << setw(10) << "****";
          } else {
            out << setw(10) << setprecision(4) << val;
          }
          break;
        } // FLOAT32

      } // switch

    } // iz
    out << endl;

  } // itime

}

///////////////////////////////////////////////////////////////////
// If fewer than 20 % of the grid points in a plane are missing,
// the missing vals are filled in by copying in from adjacent points.
//
// Returns 0 on success, -1 on failure.

int MdvxField::_planes_fill_missing()

{

  if (decompress()) {
    _errStr += "ERROR - MdvxField::_planes_fill_missing\n";
    return -1;
  }

  for (int i = 0; i < _fhdr.nz; i++) {
    void *planeStart = (void *)
      ((char *) _volBuf.getPtr() + i * _fhdr.nx * _fhdr.data_element_nbytes);
    
    _plane_fill_missing(_fhdr.encoding_type,
			_fhdr.missing_data_value,
			planeStart, _fhdr.nx, _fhdr.ny);
      
  }

  return 0;

}

///////////////////////////////////////////////////////////////////////
// If fewer than 25 % of the grid points in a plane are missing,
// the missing vals are filled in by copying in from adjacent points.
// This can be useful for some rendering operations.

void MdvxField::_plane_fill_missing(int encoding_type,
				    fl32 missing_data_value,
				    void *array, int nx, int ny)

{

  switch (encoding_type) {
    
    case Mdvx::ENCODING_INT8: {

      ui08 missing = (ui08) missing_data_value;
      ui08 *a = (ui08 *) array;

      // check that less than 75 % is missing

      int64_t count = 0;
      for (int64_t i = 0; i < nx * ny; i++) {
        if (a[i] == missing) {
          count++;
        }
      }
      if ((double) count / (double) (nx * ny) > 0.75) {
        return;
      }
    
      // create pointer array for efficiency
    
      ui08 **aa = new ui08 *[ny];
      for (int iy = 0; iy < ny; iy++) {
        aa[iy] = a + iy * nx;
      }

      // loop through, copying adjacent values to replace missing ones

      int nloop = 0;
      int maxloops = count;
      bool done = false;
      while (!done) {
        done = true;
        for (int iy = 0; iy < ny; iy++) {
          for (int ix = 0; ix < nx; ix++) {
            if (aa[iy][ix] == missing) {
              if (iy < (ny - 1) && aa[iy + 1][ix] != missing) {
                aa[iy][ix] = aa[iy + 1][ix];
                iy++; // prevents filling in from one side only
                if (iy >= ny - 1) {
                  break;
                }
                done = false;
              } else if (ix < (nx - 1) && aa[iy][ix + 1] != missing) {
                aa[iy][ix] = aa[iy][ix + 1];
                ix++; // prevents filling in from one side only
                done = false;
              } else if (iy > 0 && aa[iy - 1][ix] != missing) {
                aa[iy][ix] = aa[iy - 1][ix];
                iy++; // prevents filling in from one side only
                if (iy >= ny - 1) {
                  break;
                }
                done = false;
              } else if (ix > 0 && aa[iy][ix - 1] != missing) {
                aa[iy][ix] = aa[iy][ix - 1];
                ix++; // prevents filling in from one side only
                done = false;
              }
            } // if (aa[iy][ix] == missing
          } // ix
        } // iy
        nloop++;
        if (nloop > maxloops) {
          delete [] aa;
          return;
        }
      } // while (!done)

      delete[] aa;

      break;

    } // case Mdvx::ENCODING_INT8

    case Mdvx::ENCODING_INT16: {


      ui16 missing = (ui16) missing_data_value;
      ui16 *a = (ui16 *) array;

      // check that less than 75 % is missing

      int64_t count = 0;
      for (int64_t i = 0; i < nx * ny; i++) {
        if (a[i] == missing) {
          count++;
        }
      }
      if ((double) count / (double) (nx * ny) > 0.75) {
        return;
      }
    
      // create pointer array for efficiency
    
      ui16 **aa = new ui16 *[ny];
      for (int iy = 0; iy < ny; iy++) {
        aa[iy] = a + iy * nx;
      }

      // loop through, copying adjacent values to replace missing ones

      int nloop = 0;
      int maxloops = count;
      bool done = false;
      while (!done) {
        done = true;
        for (int iy = 0; iy < ny; iy++) {
          for (int ix = 0; ix < nx; ix++) {
            if (aa[iy][ix] == missing) {
              if (iy < (ny - 1) && aa[iy + 1][ix] != missing) {
                aa[iy][ix] = aa[iy + 1][ix];
                iy++; // prevents filling in from one side only
                if (iy >= ny - 1) {
                  break;
                }
                done = false;
              } else if (ix < (nx - 1) && aa[iy][ix + 1] != missing) {
                aa[iy][ix] = aa[iy][ix + 1];
                ix++; // prevents filling in from one side only
                done = false;
              } else if (iy > 0 && aa[iy - 1][ix] != missing) {
                aa[iy][ix] = aa[iy - 1][ix];
                iy++; // prevents filling in from one side only
                if (iy >= ny - 1) {
                  break;
                }
                done = false;
              } else if (ix > 0 && aa[iy][ix - 1] != missing) {
                aa[iy][ix] = aa[iy][ix - 1];
                ix++; // prevents filling in from one side only
                done = false;
              }
            } // if (aa[iy][ix] == missing
          } // ix
        } // iy
        nloop++;
        if (nloop > maxloops) {
          delete [] aa;
          return;
        }
      } // while (!done)

      delete[] aa;

      break;

    } // case Mdvx::ENCODING_INT16

    case Mdvx::ENCODING_FLOAT32: {

      fl32 missing = (fl32) missing_data_value;
      fl32 *a = (fl32 *) array;

      // check that less than 75 % is missing

      int64_t count = 0;
      for (int64_t i = 0; i < nx * ny; i++) {
        if (a[i] == missing) {
          count++;
        }
      }

      if ((double) count / (double) (nx * ny) > 0.75) {
        return;
      }
    
      // create pointer array for efficiency
    
      fl32 **aa = new fl32 *[ny];
      for (int iy = 0; iy < ny; iy++) {
        aa[iy] = a + iy * nx;
      }

      // loop through, copying adjacent values to replace missing ones

      int nloop = 0;
      int maxloops = count;
      bool done = false;
      while (!done) {
        done = true;
        for (int iy = 0; iy < ny; iy++) {
          for (int ix = 0; ix < nx; ix++) {
            if (aa[iy][ix] == missing) {
              if (iy < (ny - 1) && aa[iy + 1][ix] != missing) {
                aa[iy][ix] = aa[iy + 1][ix];
                iy++; // prevents filling in from one side only
                if (iy >= ny - 1) {
                  break;
                }
                done = false;
              } else if (ix < (nx - 1) && aa[iy][ix + 1] != missing) {
                aa[iy][ix] = aa[iy][ix + 1];
                ix++; // prevents filling in from one side only
                done = false;
              } else if (iy > 0 && aa[iy - 1][ix] != missing) {
                aa[iy][ix] = aa[iy - 1][ix];
                iy++; // prevents filling in from one side only
                if (iy >= ny - 1) {
                  break;
                }
                done = false;
              } else if (ix > 0 && aa[iy][ix - 1] != missing) {
                aa[iy][ix] = aa[iy][ix - 1];
                ix++; // prevents filling in from one side only
                done = false;
              }
            } // if (aa[iy][ix] == missing
          } // ix
        } // iy
        nloop++;
        if (nloop > maxloops) {
          delete [] aa;
          return;
        }
      } // while (!done)

      delete[] aa;

      break;

    } // case Mdvx::ENCODING_FLOAT32

    case Mdvx::ENCODING_RGBA32: {
      // no-op
    }

  } // switch

}

///////////////////////////////////////////////////////////////////////
// If fewer than 25 % of the grid points in a vsection are missing,
// the missing vals are filled in by copying in from adjacent points
// at the same height.

void MdvxField::_vsection_fill_missing(int encoding_type,
                                       fl32 missing_data_value,
                                       void *array, int nx, int nz)
  
{

  switch (encoding_type) {
    
    case Mdvx::ENCODING_INT8: {

      ui08 missing = (ui08) missing_data_value;
      ui08 *a = (ui08 *) array;

      // check that less than 75 % is missing

      int64_t count = 0;
      for (int64_t i = 0; i < nx * nz; i++) {
        if (a[i] == missing) {
          count++;
        }
      }
      if ((double) count / (double) (nx * nz) > 0.75) {
        return;
      }
    
      // create pointer array for efficiency
    
      ui08 **aa = new ui08 *[nz];
      for (int iz = 0; iz < nz; iz++) {
        aa[iz] = a + iz * nx;
      }

      // check each X value to see if all data is missing at that point

      vector<bool> allMissing;
      for (int ix = 0; ix < nx; ix++) {
        bool all_missing = true;
        for (int iz = 0; iz < nz; iz++) {
          if (aa[iz][ix] != missing) {
            all_missing = false;
            break;
          }
        }
        allMissing.push_back(all_missing);
      }
          
      // loop through, copying adjacent values to replace missing ones

      int nloop = 0;
      int maxloops = count;
      bool done = false;
      while (!done) {
        done = true;
        for (int iz = 0; iz < nz; iz++) {
          for (int ix = 0; ix < nx; ix++) {
            if (allMissing[ix]) {
              continue;
            }
            if (aa[iz][ix] == missing) {
              if (ix < (nx - 1) && aa[iz][ix + 1] != missing) {
                aa[iz][ix] = aa[iz][ix + 1];
                ix++; // prevents filling in from one side only
                done = false;
              } else if (ix > 0 && aa[iz][ix - 1] != missing) {
                aa[iz][ix] = aa[iz][ix - 1];
                ix++; // prevents filling in from one side only
                done = false;
              }
            } // if (aa[iz][ix] == missing
          } // ix
        } // iz
        nloop++;
        if (nloop > maxloops) {
          delete [] aa;
          return;
        }
      } // while (!done)

      delete[] aa;

      break;

    } // case Mdvx::ENCODING_INT8

    case Mdvx::ENCODING_INT16: {


      ui16 missing = (ui16) missing_data_value;
      ui16 *a = (ui16 *) array;

      // check that less than 75 % is missing

      int64_t count = 0;
      for (int64_t i = 0; i < nx * nz; i++) {
        if (a[i] == missing) {
          count++;
        }
      }
      if ((double) count / (double) (nx * nz) > 0.75) {
        return;
      }
    
      // create pointer array for efficiency
    
      ui16 **aa = new ui16 *[nz];
      for (int iz = 0; iz < nz; iz++) {
        aa[iz] = a + iz * nx;
      }

      // check each X value to see if all data is missing at that point

      vector<bool> allMissing;
      for (int ix = 0; ix < nx; ix++) {
        bool all_missing = true;
        for (int iz = 0; iz < nz; iz++) {
          if (aa[iz][ix] != missing) {
            all_missing = false;
            break;
          }
        }
        allMissing.push_back(all_missing);
      }
          
      // loop through, copying adjacent values to replace missing ones

      int nloop = 0;
      int maxloops = count;
      bool done = false;
      while (!done) {
        done = true;
        for (int iz = 0; iz < nz; iz++) {
          for (int ix = 0; ix < nx; ix++) {
            if (allMissing[ix]) {
              continue;
            }
            if (aa[iz][ix] == missing) {
              if (ix < (nx - 1) && aa[iz][ix + 1] != missing) {
                aa[iz][ix] = aa[iz][ix + 1];
                ix++; // prevents filling in from one side only
                done = false;
              } else if (ix > 0 && aa[iz][ix - 1] != missing) {
                aa[iz][ix] = aa[iz][ix - 1];
                ix++; // prevents filling in from one side only
                done = false;
              }
            } // if (aa[iz][ix] == missing
          } // ix
        } // iz
        nloop++;
        if (nloop > maxloops) {
          delete [] aa;
          return;
        }
      } // while (!done)

      delete[] aa;

      break;

    } // case Mdvx::ENCODING_INT16

    case Mdvx::ENCODING_FLOAT32: {

      fl32 missing = (fl32) missing_data_value;
      fl32 *a = (fl32 *) array;

      // check that less than 75 % is missing

      int count = 0;
      for (int i = 0; i < nx * nz; i++) {
        if (a[i] == missing) {
          count++;
        }
      }

      if ((double) count / (double) (nx * nz) > 0.75) {
        return;
      }
    
      // create pointer array for efficiency
    
      fl32 **aa = new fl32 *[nz];
      for (int iz = 0; iz < nz; iz++) {
        aa[iz] = a + iz * nx;
      }

      // check each X value to see if all data is missing at that point

      vector<bool> allMissing;
      for (int ix = 0; ix < nx; ix++) {
        bool all_missing = true;
        for (int iz = 0; iz < nz; iz++) {
          if (aa[iz][ix] != missing) {
            all_missing = false;
            break;
          }
        }
        allMissing.push_back(all_missing);
      }
          
      // loop through, copying adjacent values to replace missing ones

      int nloop = 0;
      int maxloops = count;
      bool done = false;
      while (!done) {
        done = true;
        for (int iz = 0; iz < nz; iz++) {
          for (int ix = 0; ix < nx; ix++) {
            if (allMissing[ix]) {
              continue;
            }
            if (aa[iz][ix] == missing) {
              if (ix < (nx - 1) && aa[iz][ix + 1] != missing) {
                aa[iz][ix] = aa[iz][ix + 1];
                ix++; // prevents filling in from one side only
                done = false;
              } else if (ix > 0 && aa[iz][ix - 1] != missing) {
                aa[iz][ix] = aa[iz][ix - 1];
                ix++; // prevents filling in from one side only
                done = false;
              }
            } // if (aa[iz][ix] == missing
          } // ix
        } // iz
        nloop++;
        if (nloop > maxloops) {
          delete [] aa;
          return;
        }
      } // while (!done)

      delete[] aa;

      break;

    } // case Mdvx::ENCODING_FLOAT32

    case Mdvx::ENCODING_RGBA32: {
      // no-op
    }

  } // switch

}

///////////////////////////////////////
// do the conversion for the vlevels

void MdvxField::_convert_vlevels(Mdvx::vlevel_type_t file_vlevel_type,
                                 Mdvx::vlevel_type_t req_vlevel_type,
                                 Mdvx::field_header_t &fhdr,
                                 Mdvx::vlevel_header_t &vhdr)
  
{
  
  MdvxStdAtmos msa;

  if (file_vlevel_type == Mdvx::VERT_TYPE_Z) {
    
    if (req_vlevel_type == Mdvx::VERT_TYPE_PRESSURE) {
      for (int ii = 0; ii < fhdr.nz; ii++) {
	vhdr.level[ii] = msa.ht2pres(vhdr.level[ii] * 1000.0);
      }
    } else if (req_vlevel_type == Mdvx::VERT_FLIGHT_LEVEL) {
      for (int ii = 0; ii < fhdr.nz; ii++) {
	vhdr.level[ii] = msa.ht2flevel(vhdr.level[ii] * 1000.0);
      }
    }

  } else if (file_vlevel_type == Mdvx::VERT_TYPE_PRESSURE) {

    if (req_vlevel_type == Mdvx::VERT_TYPE_Z) {
      for (int ii = 0; ii < fhdr.nz; ii++) {
	vhdr.level[ii] = msa.pres2ht(vhdr.level[ii]) / 1000.0;
      }
    } else if (req_vlevel_type == Mdvx::VERT_FLIGHT_LEVEL) {
      for (int ii = 0; ii < fhdr.nz; ii++) {
	vhdr.level[ii] = msa.pres2flevel(vhdr.level[ii]);
      }
    }
    
  } else if (file_vlevel_type == Mdvx::VERT_FLIGHT_LEVEL) {

    if (req_vlevel_type == Mdvx::VERT_TYPE_Z) {
      for (int ii = 0; ii < fhdr.nz; ii++) {
	vhdr.level[ii] = msa.flevel2ht(vhdr.level[ii]) / 1000.0;
      }
    } else if (req_vlevel_type == Mdvx::VERT_TYPE_PRESSURE) {
      for (int ii = 0; ii < fhdr.nz; ii++) {
	vhdr.level[ii] = msa.flevel2pres(vhdr.level[ii]);
      }
    }
    
  }

  // set the vlevel types
  
  if (req_vlevel_type == Mdvx::VERT_TYPE_Z) {
    for (int ii = 0; ii < fhdr.nz; ii++) {
      vhdr.type[ii] = Mdvx::VERT_TYPE_Z;
    }
    fhdr.vlevel_type = Mdvx::VERT_TYPE_Z;
  } else if (req_vlevel_type == Mdvx::VERT_TYPE_PRESSURE) {
    for (int ii = 0; ii < fhdr.nz; ii++) {
      vhdr.type[ii] = Mdvx::VERT_TYPE_PRESSURE;
    }
    fhdr.vlevel_type = Mdvx::VERT_TYPE_PRESSURE;
  } else if (req_vlevel_type == Mdvx::VERT_FLIGHT_LEVEL) {
    for (int ii = 0; ii < fhdr.nz; ii++) {
      vhdr.type[ii] = Mdvx::VERT_FLIGHT_LEVEL;
    }
    fhdr.vlevel_type = Mdvx::VERT_FLIGHT_LEVEL;
  }

  // See if the change in altitude is constant

  fhdr.dz_constant = true;
  if (fhdr.nz < 3) {
    return;
  }

  double dz0 = fabs(vhdr.level[1] - vhdr.level[0]);
  for (int iz = 2; iz <  fhdr.nz; iz++) {
    double dz = vhdr.level[iz+1] - vhdr.level[iz];
    if (fabs(dz - dz0) > 0.0001) {
      fhdr.dz_constant = false;
      break;
    }
  }

}

///////////////////////////////////////////////////////////////////
// Check if DZ is constant
// Returns TRUE if dz constant, false otherwise.
// 
// Side effect: sets dz_constant in field header.

bool MdvxField::isDzConstant()

{

  _fhdr.dz_constant = true;
  if (_fhdr.nz < 3) {
    return true;
  }
  
  double dz0 = fabs(_vhdr.level[1] - _vhdr.level[0]);
  for (int iz = 2; iz < _fhdr.nz; iz++) {
    double dz = _vhdr.level[iz] - _vhdr.level[iz-1];
    if (fabs(dz - dz0) > 0.0001) {
      _fhdr.dz_constant = false;
      return false;
    }
  }

  return true;

}

///////////////////////////////////////////////////////////////////
// Set dz, difference between vlevels, to be constant
// If not already constant, data is remapped onto constant vlevels
// by copying from closest available vlevel.
// dz is computed as the smallest suitable delta z.

void MdvxField::setDzConstant(int nzMax /* = MDV32_MAX_VLEVELS */)

{

  if (isDzConstant()) {
    // already constant
    return;
  }

  // find the smallest dz

  double dzMin = fabs(_vhdr.level[1] - _vhdr.level[0]);
  for (int iz = 0; iz < _fhdr.nz - 1; iz++) {
    double dz = fabs(_vhdr.level[iz+1] - _vhdr.level[iz]);
    if (dz < dzMin) {
      dzMin = dz;
    }
  }

  // use dzMin as delta Z

  setDzConstant(dzMin, nzMax);

}

///////////////////////////////////////////////////////////////////
// Set dz, difference between vlevels, to be constant
// If not already constant, data is remapped onto constant vlevels
// by copying from closest available vlevel.

void MdvxField::setDzConstant(double dz, int nzMax /* = MDV32_MAX_VLEVELS */)

{

  if (isDzConstant()) {
    // already constant
    return;
  }

  if (nzMax > MDV_MAX_VLEVELS) {
    nzMax = MDV_MAX_VLEVELS;
  }

  // find the min and max z
  
  double minz = _vhdr.level[0];
  double maxz = minz;
  for (int iz = 0; iz < _fhdr.nz; iz++) {
    double zz = _vhdr.level[iz];
    if (zz < minz) {
      minz = zz;
    }
    if (zz > maxz) {
      maxz = zz;
    }
  }
  double zRange = maxz - minz;
  
  // compute the number of vlevels
  
  int nz = (int) floor((zRange / dz) + 0.5);
  if (nz > nzMax) {
    nz = nzMax;
    dz = zRange / (nz - 1.0);
  }

  // remap the vlevels

  remapVlevels(nz, minz, dz);

}

///////////////////////////////////////////////////////////////////
// remap vertical levels, using a constant dz.
// If not already constant, data is remapped onto constant vlevels
// using a nearest neighbor method

void MdvxField::remapVlevels(int nz, double minz, double dz)

{

  if (!_volbufSizeValid()) {
    // no op
    cerr << "WARNING - MdvxField::remapVlevels()" << endl;
    cerr << "  Data buffer not set, field: " << getFieldName() << endl;
    return;
  }
  
  // don't remap if less than 2 levels
  
  if (_fhdr.nz < 2) {
    return;
  }

  // constrain nz to legal value for MDV

  if (nz > MDV_MAX_VLEVELS) {
    nz = MDV_MAX_VLEVELS;
  }

  // decompress data if needed

  decompress();
  
  // find the min and max z in the data
  
  double minzData = _vhdr.level[0];
  double maxzData = minzData;
  for (int iz = 1; iz < _fhdr.nz; iz++) {
    double zz = _vhdr.level[iz];
    if (zz < minzData) {
      minzData = zz;
    }
    if (zz > maxzData) {
      maxzData = zz;
    }
  }

  // set the min and max remapped Z

  bool increasing = true;
  if (_vhdr.level[_fhdr.nz-1] - _vhdr.level[0] < 0) {
    increasing = false;
  }
  
  double minzRemapped = 
    minzData - (_vhdr.level[1] - _vhdr.level[0]) / 2.0;
  double maxzRemapped = 
    maxzData + (_vhdr.level[_fhdr.nz-1] - _vhdr.level[_fhdr.nz-2]) / 2.0;

  if (!increasing) {
    maxzRemapped = 
      maxzData + (_vhdr.level[0] - _vhdr.level[1]) / 2.0;
    minzRemapped = 
      minzData - (_vhdr.level[_fhdr.nz-2] - _vhdr.level[_fhdr.nz-1]) / 2.0;
  }

#ifdef DEBUG_PRINT
  cerr << "MdvxField::remapVlevels: nz, minz, dz: "
       << nz << ", " << minz << ", " << dz << endl;
  cerr << "  increasing: " << increasing << endl;
  cerr << "  minzData: " << minzData << endl;
  cerr << "  maxzData: " << maxzData << endl;
  cerr << "  minzRemapped: " << minzRemapped << endl;
  cerr << "  maxzRemapped: " << maxzRemapped << endl;
#endif

  // set the z values, constraining at bottom and top
  // so that we only extend the data by half a dz value at each extreme
  
  vector<double> zVals;
  double zz = minz;
  for (int ii = 0; ii < nz; ii++, zz += dz) {
    if (zz >= minzRemapped && zz <= maxzRemapped) {
      zVals.push_back(zz);
    }
  }

  if (zVals.size() < 1) {
    // no remapping
    cerr << "WARNING - MdvxField::remapVlevels" << endl;
    cerr << "  Field name: " << getFieldName() << endl;
    cerr << "  Requested nz, minz, dz: "
         << nz << ", " << minz << ", " << dz << endl;
    cerr << "  Existing nz, minz, maxz: "
         << _fhdr.nz << ", " << minzData << ", " << maxzData << endl;
    cerr << "  Remapping will not be done" << endl;
    return;
  }

  // reset params 

  minz = zVals[0];
  nz = zVals.size();

#ifdef DEBUG_PRINT
  cerr << "  Final nz, minz, dz: "
       << nz << ", " << minz << ", " << dz << endl;
#endif
  
  // load temporary output buffer with data for new levels
  
  MemBuf outBuf;
  int64_t nPointsPlane = _fhdr.nx * _fhdr.ny;
  int64_t nBytesPlane = nPointsPlane * _fhdr.data_element_nbytes;
  
  for (int ii = 0; ii < nz; ii++) {

    // get the closest plane index

    int iz0, iz1;
    computePlaneLimits(zVals[ii], zVals[ii], iz0, iz1);
    
#ifdef DEBUG_PRINT
    cerr << " Remapping ii, zz, plane index: "
         << ii << ", " << zVals[ii] << ", " << iz0 << endl;
#endif
    int64_t offset = iz0 * nBytesPlane;
    void *plane = ((ui08 *) _volBuf.getPtr() + offset);
    outBuf.add(plane, nBytesPlane);

  }
  
  // move data to volume buffer
  // this does a deep copy
  
  _volBuf = outBuf;
  outBuf.free();

  // set meta data

  _fhdr.volume_size = _volBuf.getLen();
  _fhdr.nz = nz;
  int vlevelType = _vhdr.type[0];
  MEM_zero(_vhdr);
  for (int ii = 0; ii < nz; ii++) {
    _vhdr.level[ii] = zVals[ii];
    _vhdr.type[ii] = vlevelType;
  }
  _fhdr.grid_minz = _vhdr.level[0];
  _fhdr.grid_dz = dz;

  _fhdr.dz_constant = true;

#ifdef DEBUG_PRINT
  cerr << "======================================" << endl;
  Mdvx::printVlevelHeader(_vhdr, nz, getFieldName(), cerr);
  cerr << "======================================" << endl;
#endif
  
}

///////////////////////////////////////////////////////////////////
// Round dz to reasonable value

double MdvxField::_round_dz(double dz)

{

  if (dz < 0.050) {
    return 0.050;
  } else if (dz < 0.1) {
    return 0.1;
  } else if (dz < 0.25) {
    return 0.25;
  } else if (dz < 0.5){
    return 0.5;
  } else  {
    return 1.0;
  }

}

///////////////////////////////////////////////////////////////////
// Check for NaNs and infinities. If any are found, replace them with the
// bad data value and print a warning message. This is only done for
// uncompressed, FLOAT32 encoded data.

void MdvxField::_check_finite(const void *vol_data) const

{

  // For uncompressed FLOAT_32 data, check for NaN's - if any
  // are found, print a message to that effect and replace the values
  // with the bad_data_value.
  //
  // Return now if the check is not applicable.
  
  if ((_fhdr.compression_type != Mdvx::COMPRESSION_NONE) ||
      (_fhdr.encoding_type != Mdvx::ENCODING_FLOAT32) ||
      (vol_data == NULL)) {
    return;
  }
  
  int numNans = 0;
  fl32 *floatData = (fl32 *) vol_data;
  fl32 bad = _fhdr.bad_data_value;

  for (int i=0; i <  _fhdr.nx *  _fhdr.ny *  _fhdr.nz; i++, floatData++) {

    // The finite() function returns a non-zero value if value is
    // neither infinite nor a "not-a-number" (NaN) value,  and  0
    // otherwise.

    if (!isfinite(*floatData)) {
      numNans++;
      *floatData = bad;
    }
    
  }

  // If any NaNs or infinites were found, say so.

  if (numNans > 0) {
    cerr << "WARNING - MdvxField::MdvxField" << endl;
    cerr << "  " << numNans << " NaNs found in data volume for field ";
    cerr << "  " << _fhdr.field_name << " (";
    cerr << 100.0*double(numNans)/double(_fhdr.nx *  _fhdr.ny *  _fhdr.nz);
    cerr << " % NaNs) - replaced with bad_data_value" << endl;
  }
  
  return;

}

////////////////////////////////////////
// check that volbuf is valid in size

bool MdvxField::_volbufSizeValid() const
{
  if (_fhdr.compression_type == Mdvx::COMPRESSION_NONE) {
    // not compressed, do we have sufficient storage
    if ((ssize_t) _volBuf.getLen() >= _fhdr.volume_size) {
      return true;
    }
  } else {
    // compressed
    if (_volBuf.getLen() > 0) {
      // some data
      return true;
    } else if (_fhdr.volume_size == 0) {
      // no data
      return true;
    }
  }
  return false;
}

