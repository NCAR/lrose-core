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
//////////////////////////////////////////////////////////////////////
//  Ncxx C++ classes for NetCDF4
//
//  Copied from code by:
//
//    Lynton Appel, of the Culham Centre for Fusion Energy (CCFE)
//    in Oxfordshire, UK.
//    The netCDF-4 C++ API was developed for use in managing
//    fusion research data from CCFE's innovative MAST
//    (Mega Amp Spherical Tokamak) experiment.
// 
//  Offical NetCDF codebase is at:
//
//    https://github.com/Unidata/netcdf-cxx4
//
//  Modification for LROSE made by:
//
//    Mike Dixon, EOL, NCAR
//    P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
//  The base code makes extensive use of exceptions.
//
//  December 2016
//
//////////////////////////////////////////////////////////////////////

#include <string>
#include <Ncxx/NcxxGroup.hh>
#include <netcdf.h>

#ifndef NcxxFileClass
#define NcxxFileClass


//!  C++ API for netCDF4.

/*!
  Class represents a netCDF root group.
  The Ncfile class is the same as the NcxxGroup class with the additional functionality for opening
  and closing files.
*/
class NcxxFile : public NcxxGroup
{
public:

  enum FileMode
    {
      read,	//!< File exists, open read-only.
      write,      //!< File exists, open for writing.
      replace,	//!< Create new file, even if already exists.
      newFile	//!< Create new file, fail if already exists.
    };

  enum FileFormat
    {
      classic,    //!< Classic format, classic data model
      classic64,  //!< 64-bit offset format, classic data model
      nc4,        //!< (default) netCDF-4/HDF5 format, enhanced data model
      nc4classic  //!< netCDF-4/HDF5 format, classic data model
    };


  /*! Constructor generates a \ref isNull "null object". */
  NcxxFile();

  /*!
    Opens a netCDF file.
    \param filePath    Name of netCDF optional path.
    \param fMode       The file mode:
    - 'read'    File exists, open for read-only.
    - 'write'   File exists, open for writing.
    - 'replace' Create new file, even it already exists.
    - 'newFile' Create new file, fail it exists already.
  */
  NcxxFile(const std::string& filePath, FileMode fMode);
  /*!
    Opens a netCDF file.
    \param filePath    Name of netCDF optional path.
    \param fMode       The file mode:
    - 'read'    File exists, open for read-only.
    - 'write'   File exists, open for writing.
    - 'replace' Create new file, even it already exists.
    - 'newFile' Create new file, fail it exists already.
  */
  void open(const std::string& filePath, FileMode fMode);

  /*!
    Creates a netCDF file of a specified format.
    \param filePath    Name of netCDF optional path.
    \param fMode       The file mode:
    - 'replace' Create new file, even it already exists.
    - 'newFile' Create new file, fail it exists already.
  */
  NcxxFile(const std::string& filePath, FileMode fMode, FileFormat fFormat);
  /*!
    Creates a netCDF file of a specified format.
    \param filePath    Name of netCDF optional path.
    \param fMode       The file mode:
    - 'replace' Create new file, even it already exists.
    - 'newFile' Create new file, fail it exists already.
  */
  void open(const std::string& filePath, FileMode fMode, FileFormat fFormat);
     
  //! Close a file before destructor call
  void close();

  /*! destructor */
  virtual ~NcxxFile(); //closes file and releases all resources

  //! Synchronize an open netcdf dataset to disk
  void sync();

  //! Leave define mode, used for classic model
  void enddef();

  //! get the path in use
  string getPathInUse() const { return _pathInUse; }
     
private:

  string _pathInUse;
  FileMode _mode;
  FileFormat _format;

  /* Do not allow definition of NcxxFile involving copying any NcxxFile or NcxxGroup.
     Because the destructor closes the file and releases al resources such 
     an action could leave NcxxFile objects in an invalid state */
  NcxxFile& operator =(const NcxxGroup & rhs);
  NcxxFile& operator =(const NcxxFile & rhs);
  NcxxFile(const NcxxGroup& rhs);
  NcxxFile(const NcxxFile& rhs);
     
     
};

#endif
