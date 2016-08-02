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
// mdv/MdvReadField.hh
//
// Class for reading MDV fields. Used by MdvRead.
//
// Mike Dixon, RAP, NCAR,
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// May 1999
//
///////////////////////////////////////////////////////////
//
// For a given field, you may read the following:
//
//   1. field header and vlevel header.
//   2. A given plane.
//   3. The entire volume.
//
// You cannot use the read routines in the class directly. You need to
// call them via the routines in the MdvRead object.
// The read routines are private to this class.
//
// However, the public member functions of the class give you access
// to the headers and data once they have been read.
//
// Header representation.
//
//   getFieldNum() returns the field number of this object.
//   getFieldHeader() returns a reference to the field header.
//   getVlevelHeader() returns a reference to the vlevel header.
//   getGrid() returns a reference to an mdv_grid_t struct. This struct
//     holds an alternative representation of the grid information to
//     be found in the field header. It is primarily included to make
//     it easier to convert old code to use this class.
//
// Data representation.
//
// 1. Plane.
//
// Data from a plane is read into contiguous memory, accessed via
// the function getPlane1D().
//
// You may also access the data as a 2D array, using getPlane2D().
// The dereferencing is [row][col] or [y][x].
//
// planeIsEncoded() returns true if the plane was returned encoded
// (for example as MDV_PLANE_RLE8). If encoded, the 2D array is not
// relevant and is set to NULL.
//
// getPlaneNum() and getPlaneVlevel() return info on the latest plane read.
//
// 2. Volume.
//
// Data from a volume is read into contiguous memory, accessed via
// the function getVol1D().
//
// You may also access the data as a 2D array, using getVol2D().
// The dereferencing is [vlevel][row*col] or [z][row*col].
//
// You may also access the data as a 3D array, using getVol3D().
// The dereferencing is [vlevel][row][col] or [z][y][x].
//
// volIsEncoded() returns true if the vol was returned encoded
// (for example as MDV_VOL_RLE8). If encoded, the 3D array is not
// relevant and is set to NULL.
//
////////////////////////////////////////////////////////////////////////////

#ifndef MdvReadField_hh
#define MdvReadField_hh

#include <Mdv/mdv/mdv_file.h>
#include <Mdv/mdv/mdv_grid.h>
#include <toolsa/MemBuf.hh>
#include <string>
#include <vector>
using namespace std;

class MdvRead;

class MdvReadField {

friend class MdvRead;

public:

  // default constructor
  // If this is used, you must call init() before using the object
  
  MdvReadField();

  // primary constructor
  
  MdvReadField(const MdvRead *mdv, const int field_num);

  // copy constructor

  MdvReadField(const MdvReadField &other);

  // destructor

  virtual ~MdvReadField();

  // initialization - must br called if the default constructor is used
  
  void init(const MdvRead *mdv, const int field_num);

  // assignment
  
  void operator=(const MdvReadField &other);

  // access to data members

  int                   getFieldNum() const { return (_fieldNum); }
  MDV_field_header_t &  getFieldHeader()    { return (_fieldHeader); }
  MDV_vlevel_header_t & getVlevelHeader()   { return (_vlevelHeader); }
  mdv_grid_t &          getGrid()           { return (_grid); }

  int    getPlaneNum()    const   { return (_planeNum); }
  double getPlaneVlevel() const   { return (_planeVlevel); }
  void  *getPlane1D()             { return (_plane1D); }
  void **getPlane2D()             { return (_plane2D); }
  bool   planeIsEncoded()   const { return (_planeIsEncoded); }
  int    getPlaneElemSize() const { return (_planeElemSize); }

  void *getVol1D()            { return (_vol1D); }
  void **getVol2D()           { return (_vol2D); }
  void ***getVol3D()          { return (_vol3D); }
  bool volIsEncoded()   const { return (_volIsEncoded); }
  int  getVolElemSize() const { return (_volElemSize); }

protected:
  
  bool _initDone;

  const MdvRead *_mdv;

  bool _headersRead;
  int _fieldNum;
  
  MDV_field_header_t _fieldHeader;
  MDV_vlevel_header_t _vlevelHeader;
  mdv_grid_t _grid;

  // Data from readPlane.
  // If returned non-encoded, 1D data is contiguous,
  //   and 2D array points into it.
  //   2D array points to start of each row.
  // If data is returned encoded, 2D array is not relevant.

  bool _planeIsEncoded;
  int _planeReturnDataType;
  int _planeElemSize;
  int _planeNum;
  double _planeVlevel;
  
  void *_plane1D;
  void **_plane2D;

  // Data from readVol.
  // If returned non-encoded, 1D data is contiguous,
  //   and 2D and 3D arrays point into it.
  //   2D array points to start of each plane.
  //   3D array points to start of each row.
  // If data is returned encoded, 3D array is not relevant.
  
  bool _volIsEncoded;
  int _volReturnDataType;
  int _volElemSize;
  
  void *_vol1D;
  void **_vol2D;
  void ***_vol3D;
  vector<int> _encodedPlaneSizes;

  // memory buffers is used for encoded data

  MemBuf _encodedPlaneBuf;
  MemBuf _encodedVolBuf;

  ////////////////////////////////
  // read field and vlevel headers
  //
  // Returns 0 on success,  -1 on failure

  int _readHeaders();

  /////////////////////////////////////////
  // read plane of data given the plane num
  // If fhdr is non-NULL, it is filled out with the values applicable
  // after the read and any relevant conversions.
  // Returns 0 on success, -1 on failure

  int _readPlane(const int plane_num,
		 const int return_data_type,
		 MDV_field_header_t *fhdr = NULL);

  //////////////////////////////////////////////////////////
  // Read plane of data given the desired vlevel.
  // If fhdr is non-NULL, it is filled out with the values applicable
  // after the read and any relevant conversions.
  // Actual plane_num used may be retrieved by getPlaneNum()
  // Actual plane vlevel used may be retrieved by getPlaneVlevel()
  //
  // Returns 0 on success, -1 on failure
  
  int _readPlane(const double plane_vlevel,
		 const int return_data_type,
		 MDV_field_header_t *fhdr = NULL);
  
  //////////////////////////////////////////////////////
  // read composite - this is the max value at any point.
  // Only non-compressed types supported.
  // If fhdr is non-NULL, it is filled out with the values applicable
  // after the read and any relevant conversions.
  // Returns 0 on success, -1 on failure

  int _readComposite(const int return_data_type = MDV_INT8,
		     MDV_field_header_t *fhdr = NULL);
  
  ////////////////////////////////
  // read data volume
  // Only non-compressed types supported.
  // If fhdr is non-NULL, it is filled out with the values applicable
  // after the read and any relevant conversions.
  // Returns 0 on success, -1 on failure
  
  int _readVol(const int return_data_type,
	       MDV_field_header_t *fhdr = NULL);
  
  //////////////////
  // free functions

  void _freeData();
  void _freePlane();
  void _freeVol();

private:

};

#endif


