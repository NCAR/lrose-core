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
/**
 * @file GenPolyGrid.hh
 * @brief extends GenPoly class to include a specific conversion to/from grids
 *        using MdvxProj
 * @class GenPolyGrid
 * @brief extends GenPoly class to include a specific conversion to/from grids
 *        using MdvxProj
 *
 * Uses storage in the base class for one specific double value, the 'score'
 */

# ifndef    GENPOLYGRID_H
# define    GENPOLYGRID_H

#include <rapformats/GenPoly.hh>
class Grid2d;
class GridAlgs;
class MdvxProj;
class MdvxField;
class Mdvx;
class DsMdvx;

class GenPolyGrid : public GenPoly
{
 public:

  /**
   * Constructor
   */
  GenPolyGrid(void);

  /**
   * Construct from base class
   * @param[in] m  Base class object
   */
  GenPolyGrid(const GenPoly &m);

  /**
   * Copy constructor
   * @param[in] m  
   */
  GenPolyGrid(const GenPolyGrid &m);

  /**
   * Destructor
   */
  virtual ~GenPolyGrid(void);

  /**
   * Operator=
   * @param[in] g
   */
  GenPolyGrid & operator=(const GenPolyGrid &g);

  /**
   * Operator==
   * @param[in] g
   */
  bool operator==(const GenPolyGrid &g) const;

  /**
   * Debug print
   * @param[in] full  If true invoke GenPoly::print()
   *                  otherwise a brief print with id, time, number of points
   */
  void print(const bool full) const;

  /**
   * Debug print
   * @param[in] indent  Indentation character string
   * @param[in] full  If true invoke GenPoly::print()
   *                  otherwise a brief print with id, time, number of points
   */
  void print(const char *indent, const bool full) const;

  /**
   * Set polygon vertex and other values using an input grid and a projection.
   * One vertex per 'edge' grid point. 
   *
   * This method builds up vertices along the edge of the clump, one vertex
   * per edge point.
   *
   * @param[in] t  Time to use
   * @param[in] t_expire  Expire time to use
   * @param[in] id  I.d. value to use
   * @param[in] grid  grid assumed to have one contiguous clump of
   *                  non-missing grid points, missing everywhere else
   * @param[in] proj projection information for the grid
   *
   * @return true for success
   */
  bool set(const time_t &t, const time_t &t_expire, const int id,
	   const GridAlgs &grid, const MdvxProj &proj);

  /**
   * Set polygon vertex and other values using an input grid and a projection,
   * treating each grid point as the centerpoint of a box. For example the
   * polygon for one grid point would consist of 4 vertices at the box
   * corners that surround the point.  
   *
   * This method builds up vertices along the edge of the clump using this
   * interpretation of the grid points, and there can be up to 4 vertices
   * per edge gridpoint.
   *
   * @param[in] t  Time to use
   * @param[in] t_expire  Expire time to use
   * @param[in] id  I.d. value to use
   * @param[in] grid  grid assumed to have one contiguous clump of
   *                  non-missing grid points, missing everywhere else
   * @param[in] proj projection info for the grid
   *
   * @return true for success
   */
  bool setBoxes(const time_t &t, const time_t &t_expire, const int id,
		const GridAlgs &grid, const MdvxProj &proj);

  /**
   * set polygon values from input grid, with smoothing of vertices after.
   * One vertex per 'edge' grid point.
   *
   * @param[in] t  Time to use
   * @param[in] t_expire  Expire time to use
   * @param[in] id  I.d. value to use
   * @param[in] grid  grid with one clump of non-missing points
   * @param[in] proj  projection info for the grid
   * @param[in] npt_smooth  max number of polygon vertices to smooth
   * @param[in] max_pcnt_smooth  max percentage of total vertices to smooth
   *
   * @return true for success
   *
   * The smoothing is through averaging of latitude/longitudes.
   */
  bool setAndSmooth(const time_t &t, const time_t &t_expire, const int id,
		    const GridAlgs &grid, const MdvxProj &proj,
		    const int npt_smooth, const double max_pcnt_smooth);
  
  /**
   * fill grid values as a clump using polygon vertices to define clump edges
   *
   * @param[in] proj  grid projection info
   * @param[in,out] grid  Grid to write to, not cleared before writing to.
   * @param[out] npt  Number of grodpoints in the clump that was created
   *
   * @return true for success
   *
   * @note the clump value that is put into the grid is the object i.d.
   * @note Does NOT clear the grid to missing prior to filling in this data
   */
  bool get(const MdvxProj &proj, Grid2d &grid, int &npt) const;

  /**
   * @return number of vertices.
   *
   * This is a shorter method name, same as base class getNumVertices
   */
  inline int num(void) const {return getNumVertices();}

  /**
   * create an empty polygon.  (zero vertices)
   *
   * @param[in] id  Polygon id to give the empty polygon
   * @param[in] t   Time  to give the empty polygon
   */
  void fillEmpty(const int id, const time_t t);

  /**
   * @return true if the local object is an empty polygon (no vertices)
   */
  bool isEmpty(void) const;

  /**
   * @return true if the polygon vertices are the same as input.
   *
   * @param[in] p  Object to compare to
   */
  bool isRedundant(const GenPolyGrid &p) const;

  /**
   * set so there is no score value.
   *
   * @note The class supports one score
   */
  void setNoScore(void);

  /**
   * set so there is one score value with input name and units
   *
   * @param[in] score  The score to use
   * @param[in] name   The name to use
   * @param[in] units  The units to use
   *
   * @note The class supports one score
   */
  void setScore(const double score, const string &name, const string &units);

  /**
   * Retrieve the score
   *
   * @param[out] score the score
   * @return true if score value was present.
   *
   * @note The class supports one score
   */
  bool getScore(double &score) const;

  /**
   * @return fixed tag for this object, useful for XML
   */
  inline static string stateTag(void) {return "GenPolyGrid";}

  /**
   * Append state as XML to a string, indenting as indicated.
   *
   * @param[in] buf     string to append to
   * @param[in] indent  number of spaces to indent 
   * 
   * @Note Creates XML with state_tag() outer tag
   */
  void saveGenpolygridState(string &buf, int indent) const;

  /** 
   * read from XML string into local object
   *
   * @param[in] s      buffer to read 
   *
   * @return  true for success
   *
   * @note buf assumed to have content consistant with write method.
   *       Method looks for state_tag() XML block and parses that.
   */
  bool retrieveGenpolygridState(const string &s);

  /**
   * Static method to take an input Mdvx object and convert one field
   * to a Grid2d object.  Assumes the input Mdvx data is 2 dimensional.
   *
   * @param[in] M  The Mdvx object
   * @param[in] fieldName  The field name
   * @param[out] G  The Grid2d
   * @return  True if able to convert and build the grid
   *
   * @note This doesn't quite belong here, but for now it has been put into
   * this class
   */
  static bool mdvToGrid2d(const Mdvx &M, const std::string &fieldName,
			  Grid2d &G);

  /**
   * Static method to take an input DsMdvx object and clear out all fields,
   * adding in a new field for each Grid2d input.
   *
   * @param[in,out] D The DsMdvx object. It is assumed this object was used
   *                to successfully read in at least one field, which is still
   *                contained in the object on entry, and that this field has
   *                the same dimensions as the input grids.
   * @param[in] grids  The Grid2d data
   * @param[in] nU  The name/unit pairs. (first=name, second=units).
   *                For each Grid2d in grids, its name should match
   *                one of the names here.
   * @return  True if able to form the returned object
   *
   * The fields are given ENCODING_INT8 with COMPRESSION_GZIP
   *
   * @note This doesn't quite belong here, but for now it has been put into
   * this class
   */
  static bool
  grid2dToDsMdvx(DsMdvx &D, const std::vector<Grid2d> &grids,
		 const std::vector<std::pair<std::string,std::string> > &nU);

protected:
private:

  void _setInit(const string &name, const int id, const time_t &t,
		 const time_t &t_expire, const bool closed);

  static bool
  _addField(const Grid2d &grid,
	    const std::vector<std::pair<std::string,std::string> > &nU,
	    const MdvxField *field, DsMdvx &D);
};

# endif
