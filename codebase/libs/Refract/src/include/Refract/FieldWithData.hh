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
 *
 * @file FieldWithData.hh
 *
 * @class FieldWithData
 * Struct like class to hold a field, the header, and pointer to data
 *
 */

#ifndef FieldWithData_HH
#define FieldWithData_HH

#include <Mdv/DsMdvx.hh>
#include <Mdv/Mdvx.hh>
#include <Mdv/MdvxPjg.hh>
#include <Mdv/MdvxField.hh>
#include <string>

class FieldDataPair;

/** 
 * @class FieldWithData
 */

class FieldWithData
{
 public:

  /**
   * Empty constructor
   */
  inline FieldWithData(void) : _field(NULL), _data(NULL) {}

  /**
   * Make copy of input field, set all values to input, changing name and unit
   *
   * @param[in] f  Field to make copy of
   * @param[in] name  New name
   * @param[in] units  New units
   * @param[in] value  Value to put everywhere
   */
  FieldWithData(const MdvxField *f, const std::string &name,
		const std::string &units, double value);

  /**
   * Make copy of input field, including values, changing name and unit
   *
   * @param[in] f  Field to make copy of
   * @param[in] name  New name
   * @param[in] units  New units
   */
  FieldWithData(const MdvxField *f, const std::string &name,
		const std::string &units);

  /**
   * Make copy of input field, including values, changing name and unit
   *
   * @param[in] f  Field to make copy of
   * @param[in] name  New name
   * @param[in] units  New units
   */
  FieldWithData(const FieldWithData &f, const std::string &name,
		const std::string &units);

  /**
   * Weighted average of two inputs
   * @param[in] inp0
   * @param[in] w0
   * @param[in] inp1
   * @param[in] w1
   */
  FieldWithData(const FieldWithData &inp0, double w0,
		const FieldWithData &inp1, double w1);

  /**
   * Make copy of input field, including values
   *
   * @param[in] f  Field to make copy of
   */
  FieldWithData(const MdvxField *f);

  /**
   * Destructor
   */
  inline ~FieldWithData(void) {}

  /**
   * @return elevation angle 
   */
  inline fl32 getElev(void) const  {return _field->getVlevelHeader().level[0];}

  /**
   * @return scan size
   */
  inline int scanSize(void) const {return _field_hdr.nx*_field_hdr.ny;}

  /**
   * @return number of azimuths
   */
  inline int numAzimuth(void) const {return _field_hdr.ny;}

  /**
   * @return number of gates
   */
  inline int numGates(void) const {return _field_hdr.nx;}

  /**
   * @return gate spacing from field header
   */
  inline double gateSpacing(void) const {return _field_hdr.grid_dx;}

  /**
   * Create a projection from the field header
   * @return  the projection
   */
  MdvxPjg createProj(void) const;

  /**
   * @brief Create a blank MDV field with the given field name, whose 
   * contents match that of the local object as regards projection, elevation.
   *
   * @param[in] name  The field name
   *
   * @return Returns a pointer to the new field on success, 0 on failure.
   *         The calling method takes ownership of the pointer and must
   *         delete it when no longer needed.
   */
  MdvxField *createMatchingBlankField(const std::string &name) const;

  /**
   * Create a MDV field that matches the local one exactly and return
   * Note this does change the bad/missing value to RefractInput::INVALID
   * Note data values are not set to match local
   *
   * @return Returns a pointer to the new field on success, 0 on failure.
   *         The calling method takes ownership of the pointer and must
   *         delete it when no longer needed.
   */
  MdvxField *createMatchingField(void) const;

  /**
   * Create a MDV field that matches the local one and return
   * Note this does change the bad/missing value to RefractInput::INVALID
   * Note this does change the name and units to inputs
   * Note data values are not set to match local
   *
   * @param[in] name   New field name
   * @param[in] units  New units
   *
   * @return Returns a pointer to the new field on success, 0 on failure.
   *         The calling method takes ownership of the pointer and must
   *         delete it when no longer needed.
   */
  MdvxField *createMatchingField(const std::string &name,
				 const std::string &units) const;

  /**
   * Create a MDV field that matches the local one and return
   * Note this does change the bad/missing value to RefractInput::INVALID
   * Note this does change the name and units to inputs
   * Note this does set all values to a single input value

   * @param[in] name   New field name
   * @param[in] units  New units
   * @param[in] initialValue  The value to give all gridpoints
   *
   * @return Returns a pointer to the new field on success, 0 on failure.
   *         The calling method takes ownership of the pointer and must
   *         delete it when no longer needed.
   */
  MdvxField *createMatchingField(const std::string &name,
				 const std::string &units,
				 double initialValue) const;

  /**
   * Make a copy of local MdvField and change name and units to inputs
   * Includes copy of data values
   *
   * @param[in] name   New field name
   * @param[in] units  New units
   *
   * @return Returns a pointer to the new field on success, 0 on failure.
   *         The calling method takes ownership of the pointer and must
   *         delete it when no longer needed.
   */
  MdvxField *createMatchingFieldWithData(const std::string &name,
					 const std::string &units) const;

  /**
   * Make a copy of local MdvField. Includes copy of data values
   *
   * @return Returns a pointer to the new field on success, 0 on failure.
   *         The calling method takes ownership of the pointer and must
   *         delete it when no longer needed.
   */
  MdvxField *createMatchingFieldWithData(void) const;

  /**
   * @return a copy of the local MdvxField pointer (a new MdvxField)
   */
  MdvxField *fieldCopy(void) const;

  /**
   * @return true if data is bad or missing at an index
   * @param[in] index
   */
  bool isBadAtIndex(int index) const;

  /**
   * @return true if local gate spacing from field header not equal to input
   * @param[in] wanted   The gate spacing that is expected
   */
  bool wrongGateSpacing(double wanted) const;

  /**
   * Set data at all points to 0.0
   */
  void setAllZero(void) const;

  /**
   * Create a vector that is at each point the product of the local
   * data and the input count array
   * @param[in] count  An array of values
   *
   * @return the products.
   */
  std::vector<double> productVector(int *count) const;

  /**
   * At each point where a mask has a certain value, change local data
   * to a replacement value
   * @param[in] mask  The mask grid
   * @param[in] maskValue  The value to check for
   * @param[in] replaceValue  The value to replace with
   */
  void mask(const FieldWithData &mask, double maskValue, double replaceValue);

  /**
   * At each point where a count is <=0, set local data to replacement value
   * @param[in] count  the array
   * @param[in] replaceValue  The value to replace with
   */
  void maskWhenCountNonPositive(const int *count, double replaceValue);
  
  /**
   * Strength is the mean signal to noise ratio divided by the count
   * Set local data to strength using inputs
   *
   * @param[in] meanSnr  The grid of signal to noise ratio
   * @param[in] count  The count array
   * @param[in] imask  An array that is nonzero at all point where you set
   *                   the strength.  THis is to agree with original code.
   * @param[in] invalid  The value to set strength to where can't set strength
   */
  void setStrength(const FieldWithData &meanSnr, const int *count,
		   const std::vector<int> &imask, double invalid);

  /**
   * Set local data to NCP, derived from inputs
   * @param[in] sumAB
   * @param[in] denom  
   * @param[in] rMin
   * @param[in] count
   * @param[in] strength
   * @param[in] fluctSnr
   */
  void setNcp(const FieldDataPair &sumAB, const std::vector<double> &denom,
	      int rMin,
	      const int *count, const FieldWithData &strength,
	      const float *fluctSnr);

  /**
   * Set local data to phase error, derived from inputs
   * @param[in] ncp
   * @param[in] imask
   * @param[in] rMin
   * @param[in] very_large
   */
  void setPhaseErr(const FieldWithData &ncp, const std::vector<int> &imask,
		   int rMin, double very_large);

  /**
   * Create and return an vector with absolute value of local values,
   * except where invalid or 0, in which case set to -1
   * @return vector of values
   * @param[in] invalid  The invalid data value
   */
  std::vector<double> normalizationVector(double invalid) const;
  // double *normalizationArray(double invalid) const;

  /**
   * Eliminate main lobe contamination in local data using inputs
   *
   * @param[in] rmin  Minimum compute radius index
   * @param[in] contamin_pow  Contamination power
   * @param[in] strengthField   Strength grid
   */
  void mainlobeElimination(int rmin, double contamin_pow,
			   const FieldWithData &strengthField);

  /**
   * Eliminate range side lobe contamination in local data using inputs
   *
   * @param[in] rMin  Minimum compute radius index
   * @param[in] contamin_pow  Contamination power
   * @param[in] veryLarge  The very large data value
   * @param[in] strengthField   Strength grid
   */
  void rangeSidelobeElimination(int rMin, double contamin_pow, double veryLarge,
				const FieldWithData &strengthField);

  /**
   * Eliminate 360 side lobe contamination in local data using inputs
   *
   * @param[in] rMin  Minimum compute radius index
   * @param[in] sideLobePower
   * @param[in] strengthField   Strength grid
   * @param[in] oldSnrField
   */
  void sidelobe360Elimination(int rMin, double sideLobePower,
			      const FieldWithData &strengthField,
			      const FieldWithData &oldSnrField);

  /**
   * Set local data to phase error, using inputs
   * @param[in] data  
   * @param[in] very_large
   */
  void setPhaseError(const FieldWithData &data, double very_large);

  /**
   * Create and return a quality array using local SNR data and inputs
   * @param[in] qual Quality data 
   * @param[in] quality_from_width  True if quality is derived from width
   * @param[in] quality_from_cpa    The other option
   * @param[in] thresh_width
   * @param[in] abrupt_factor
   *
   * @return array owned by caller
   */
  std::vector<double>
  setQualityVector(const FieldWithData &qual, bool quality_from_width,
		   bool quality_from_cpa, double thresh_width,
		   double abrupt_factor) const;

  /**
   * Create and return phase error array using local phase error data and inputs
   * @param[in,out] quality Quality values, which can be modified
   * @param[in] av_iq  Average IQ
   * @param[in] minR  Minimum  compute radius index
   *
   * @return array owned by caller
   */
  std::vector<double>
  setPhaseErVector(std::vector<double> &quality, const FieldDataPair &av_iq,
		   int minR) const;

  /**
   * Set data to absolute value of input, where the input is not invalid
   *
   * @param[in] inp
   * @param[in] invalid
   */
  void setAbsValue(const FieldWithData &inp, double invalid);
  
  /**
   * @return x (r) index for an input 2d index for local grid dimensions
   * @param[in] index  Index into grid interpreted as 1d
   */
  int rIndex(int index) const;

  /**
   * @return x (r) index for an input 2d index for particular grid dimensions
   * @param[in] index  Index into grid interpreted as 1d
   * @param[in] numAz  Number of azimuth (y) indices
   * @param[in] numGates  Number of gates (x) indices
   */
  static int rIndex(int index, int numAz, int numGates);

  /**
   * @brief Create a blank MDV field with the given field name, projection,
   * and elevation
   *
   * @param[in] proj The data projection.
   * @param[in] field_name The field name.
   * @param[in] elevation The elevation angle of the tilt we are processing.
   *
   * @return Returns a pointer to the new field on success, 0 on failure.
   *         The calling method takes ownership of the pointer and must
   *         delete it when no longer needed.
   */
  static MdvxField *createBlankField(const MdvxPjg &proj,
				     const std::string &field_name,
				     double elevation);
  
  /**
   * operator[], no bounds checking
   * @return reference to data at an index
   * @param[in] i
   */
  inline fl32 &operator[](size_t i) {return _data[i];}

  /**
   * operator[], no bounds checking
   * @return reference to data at an index
   * @param[in] i
   */
  inline const fl32 &operator[](size_t i) const {return _data[i];}

  /**
   * @return copy of the field header for this data
   */
  inline Mdvx::field_header_t getFieldHeader(void) const {return _field_hdr;}

  /**
   * @return pointer to the data
   */
  inline fl32 *getDataPtr(void) {return _data;}

  /**
   * @return const pointer to the data
   */
  inline const fl32 *getDataPtr(void) const {return _data;}

  /**
   * @return const pointer to the field
   */
  const MdvxField *getFieldPtrConst(void) const {return _field;}

 private:

  std::string _name;                     /**< field name */
  MdvxField *_field;                     /**< pointer to MdvxField */
  Mdvx::field_header_t _field_hdr;       /**< the field header, synched */
  fl32 * _data;                          /**< the data, synched */

  MdvxField *_createMatchingField(const MdvxField *f, const std::string &name,
				  const std::string &units) const;
  static double _sideCorrection(double near_pow, double contamin_pow);
};


#endif
