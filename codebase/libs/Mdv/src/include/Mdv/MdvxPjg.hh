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
/************************************************************************
 * MdvxPjg.hh: class implementing projective geometry transformations
 *             based on Mdvx data.
 *
 * If you use the default constructor, the projection will be set 
 * to latlon. You must call one of the init() functions if you want
 * alternative behavior.
 *
 * RAP, NCAR, Boulder CO
 *
 * April 2001
 *
 * Nancy Rehak
 *
 ************************************************************************/

#ifndef MdvxPjg_hh
#define MdvxPjg_hh

#include <euclid/Pjg.hh>
#include <Mdv/Mdvx.hh>
using namespace std;

class MdvxPjg : public Pjg
{

public:

  /**********************************************************************
   * Constructors.
   */

  MdvxPjg();
  MdvxPjg(const Mdvx &mdvx);
  MdvxPjg(const Mdvx::master_header_t &mhdr,
	  const Mdvx::field_header_t &fhdr);
  MdvxPjg(const Mdvx::field_header_t &fhdr);
  MdvxPjg(const Mdvx::coord_t &coord);
  MdvxPjg(const MdvxPjg &rhs);
  MdvxPjg(const Pjg &pjg);
  

  /**********************************************************************
   * Destructor.
   */

  virtual ~MdvxPjg();
  
  /**********************************************************************
   * supported() - Check that the underlying projection is supported.
   *
   * Return true is proj type is supported, false otherwise.
   *
   * Useful for checking if the constructor was given data which
   * can be used by this class.
   */

  bool supported();

  // assignment
  
  MdvxPjg & operator=(const MdvxPjg &rhs);

  ////////////////////////////
  // Initialization methods //
  ////////////////////////////

  /**********************************************************************
   * init() - Initialize the object from the given information.
   */

  void init(const Mdvx &mdvx);
  void init(const Mdvx::master_header_t &mhdr,
	    const Mdvx::field_header_t &fhdr);
  void init(const Mdvx::field_header_t &fhdr);
  void init(const Mdvx::coord_t &coord);
  void init(const Pjg &pjg);
  

  /**********************************************************************
   * initFlat() - Initialize flat earth projection.
   */

  void initFlat(const double origin_lat, const double origin_lon,
		const double rotation = 0.0,
		const int nx = 1, const int ny = 1, const int nz = 1,
		const double dx = 1.0, const double dy = 1.0,
		const double dz = 1.0,
		const double minx = 0.0, const double miny = 0.0,
		const double minz = 0.0);
  
  /**********************************************************************
   * initLc2() - Initialize lambert conformal projection with two lats.
   */

  void initLc2(const double origin_lat, const double origin_lon,
	       const double lat1, const double lat2,
	       const int nx = 1, const int ny = 1, const int nz = 1,
	       const double dx = 1.0, const double dy = 1.0,
	       const double dz = 1.0,
	       const double minx = 0.0, const double miny = 0.0,
	       const double minz = 0.0);
  
  /**********************************************************************
   * initLatlon() - Initialize latlon projection.
   */

  void initLatlon(const int nx = 1, const int ny = 1, const int nz = 1,
		  const double dx = 1.0, const double dy = 1.0,
		  const double dz = 1.0,
		  const double minx = 0.0, const double miny = 0.0,
		  const double minz = 0.0);
  
  /**********************************************************************
   * initPolarRadar() - Initialize polar radar projection.
   */

  void initPolarRadar(const double origin_lat, const double origin_lon,
		      const int nx = 1, const int ny = 1, const int nz = 1,
		      const double dx = 1.0, const double dy = 1.0,
		      const double dz = 1.0,
		      const double minx = 0.0, const double miny = 0.0,
		      const double minz = 0.0);
  

  /**********************************************************************
   * initPolarStereo() - Initialize polar stereographic projection.
   */

  void initPolarStereo(const double tangent_lon,
		       const PjgTypes::pole_type_t pt = PjgTypes::POLE_NORTH,
		       const double central_scale = 1.0,
		       const int nx = 1, const int ny = 1, const int nz = 1,
		       const double dx = 1.0, const double dy = 1.0,
		       const double dz = 1.0,
		       const double minx = 0.0, const double miny = 0.0,
		       const double minz = 0.0);
  
  /**********************************************************************
   * initObliqueStereo() - Initialize oblique stereographic projection.
   */

  void initObliqueStereo(const double origin_lat, const double origin_lon,
			 const double tangent_lat, const double tangent_lon,
			 const int nx = 1, const int ny = 1, const int nz = 1,
			 const double dx = 1.0, const double dy = 1.0,
			 const double dz = 1.0,
			 const double minx = 0.0, const double miny = 0.0,
			 const double minz = 0.0);
  
  /**********************************************************************
   * initMercator() - Initialize Mercator projection.
   */

  void initMercator(const double origin_lat, const double origin_lon,
		  const int nx = 1, const int ny = 1, const int nz = 1,
		  const double dx = 1.0, const double dy = 1.0,
		  const double dz = 1.0,
		  const double minx = 0.0, const double miny = 0.0,
		  const double minz = 0.0);
  

  ////////////////////
  // Access methods //
  ////////////////////

  /**********************************************************************
   * setGridDims() - Set the current grid dimensions.
   */

  void setGridDims(const int nx, const int ny, const int nz)
  {
    Pjg::setGridDims(nx, ny, nz);

    _coord.nx = nx;
    _coord.ny = ny;
    _coord.nz = nz;
  }
  
  
  /**********************************************************************
   * setGridDeltas() - Set the current grid deltas.
   */

  void setGridDeltas(const double dx, const double dy, const double dz)
  {
    Pjg::setGridDeltas(dx, dy, dz);

    _coord.dx = dx;
    _coord.dy = dy;
    _coord.dz = dz;
  }
  
  
  /**********************************************************************
   * setGridMins() - Set the current grid minimums.
   */

  void setGridMins(const double minx, const double miny, const double minz)
  {
    Pjg::setGridMins(minx, miny, minz);

    _coord.minx = minx;
    _coord.miny = miny;
    _coord.minz = minz;
  }
  
  
  /**********************************************************************
   * setSensorPosn() - Set the sensor position.  Ht is in km MSL.
   */

  void setSensorPosn(double sensor_lat,
		     double sensor_lon,
		     double sensor_ht);
  

  /**********************************************************************
   * setDataOrdering() - Set the data ordering.  The default is
   *                     Mdvx::ORDER_XYZ.
   */

  inline void setDataOrdering(const Mdvx::grid_order_indices_t data_ordering)
  {
    _dataOrdering = data_ordering;
  }
  

  /**********************************************************************
   * getProjType() - Retrieve the Mdvx projection type.
   */

  int getProjType(void) const;
  

  /**********************************************************************
   * getCoord() - Retrieve the coord struct
   */

  const Mdvx::coord_t getCoord(void) const
  {
    return _coord;
  }
  

  /////////////////////////////
  // Synchronization methods //
  /////////////////////////////

  /**********************************************************************
   * syncToHdrs() - Synchronize master and field header with info from
   *                this object.
   */

  void syncToHdrs(Mdvx::master_header_t &mhdr,
		  Mdvx::field_header_t &fhdr) const;

  /**********************************************************************
   * syncToFieldHdr() - Synchronize field header with info from this
   *                    object.
   */

  void syncToFieldHdr(Mdvx::field_header_t &fhdr) const;
  
  /**********************************************************************
   * syncXyToFieldHdr() - Synchronize field header with (x,y) info from
   *                      this object.
   */

  void syncXyToFieldHdr(Mdvx::field_header_t &fhdr) const;
  

  //////////////////////////
  // Input/Output methods //
  //////////////////////////

  /**********************************************************************
   * print() - Print the object information to the given print stream.
   */

  void print(ostream &out) const;
  
  /**********************************************************************
   * printCoord() - Print the given coord structure to the given print
   *                stream.
   */

  static void printCoord(const Mdvx::coord_t &coord,
			 ostream &out);
  

  /////////////////////
  // Utility methods //
  /////////////////////

  /**********************************************************************
   * xyIndex2arrayIndex() - Computes the index into the data array.
   *
   * Returns the calculated array index on success, -1 on failure
   * (data outside grid).
   */

  int xyIndex2arrayIndex(const int ix, const int iy, const int iz = 0) const;
  

protected:

  ///////////////////////
  // Protected members //
  ///////////////////////

  bool _supported;
  
  Mdvx::coord_t _coord;
  Mdvx::grid_order_indices_t  _dataOrdering;
  

  ///////////////////////
  // Protected methods //
  ///////////////////////

  void _init();
  
  void _loadCoordFromFieldHdr(const Mdvx::field_header_t &fhdr);
  
  void _loadCoordFromMasterHdr(const Mdvx::master_header_t &mhdr);
  
private:

};

#endif











