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
 * @file FieldDataPair.hh
 *
 * @class FieldDataPair
 *
 * Class for doing all of the data processing.
 *  
 */

#ifndef FieldDataPair_HH
#define FieldDataPair_HH

#include <Refract/VectorIQ.hh>
#include <Mdv/DsMdvx.hh>
#include <Mdv/MdvxField.hh>
#include <toolsa/toolsa_macros.h>
#include <cmath>
#include <string>

class FieldWithData;

/** 
 * @class FieldDataPair
 */

class FieldDataPair : public VectorIQ
{
 public:

  /**
   * @brief Constructor, empty
   */
  inline FieldDataPair(void) : _I(NULL), _Q(NULL), _scanSize(0), _idata(NULL),
			       _qdata(NULL) {}
  
  /**
   * @brief Destructor
   */
  virtual ~FieldDataPair (void);
  
  /**
   * Constructor.  Example data is used as a template.  Names and units are
   * changed to inputs.
   * @param[in] example Example data
   * @param[in] iName   new name for I
   * @param[in] iUnits  new units for I
   * @param[in] qName   new name for Q
   * @param[in] qUnits  new unites for Q
   */
  FieldDataPair(const FieldDataPair &example, const std::string &iName,
		const std::string &iUnits, const std::string &qName,
		const std::string &qUnits);

  /**
   * Constructor.  Example data is used as a template.  Names and units are
   * changed to inputs, and I and Q are set to a constant everywhere.
   * @param[in] example Example data
   * @param[in] iName   new name for I
   * @param[in] iUnits  new units for I
   * @param[in] ivalue  Value for all I
   * @param[in] qName   new name for Q
   * @param[in] qUnits  new unites for Q
   * @param[in] qvalue  Value for all Q
   */
  FieldDataPair(const FieldDataPair &example, const std::string &iName,
		const std::string &iUnits, double ivalue,
		const std::string &qName, const std::string &qUnits,
		double qvalue);

  /**
   * Constructor.  Input I and Q are used to set up local object
   * @param[in] I 
   * @param[in] Q 
   */
  FieldDataPair(const FieldWithData &I, const FieldWithData &Q);

  /**
   * Constructor.  Input I and Q are used to set up local object
   * @param[in] I 
   * @param[in] Q 
   */
  FieldDataPair(const MdvxField *I, const MdvxField *Q);

  /**
   * Constructor.  Weighted average of two inputs
   * @param[in] inp0   One input
   * @param[in] w0   Weight to give inp0
   * @param[in] inp1  Other input
   * @param[in] w1   Weight to give inp1
   */
  FieldDataPair(const FieldDataPair &inp0, double w0,
		const FieldDataPair &inp1, double w1);

  /**
   * copy constructor
   * @param[in] p
   */
  FieldDataPair(const FieldDataPair &p);

  /**
   * operator=
   * @param[in] p
   * @return reference to local object
   */
  FieldDataPair & operator=(const FieldDataPair &p);


  /**
   * Copy input into local.  Where bad or missing, set IQ to 0,0
   * @param[in] f
   */
  void copyIQFilterBadOrMissing(const FieldDataPair &f);

  /**
   * For all points where r >= rMin set data to input 
   * at all other points, don't change local data
   * @param[in] rMin
   * @param[in] inp
   */
  void copyLargeR(int rMin, const VectorIQ &inp);

  /**
   * Free and set empty
   */
  inline void free(void) { _free();}

  /**
   * Smooth close to radar
   * @param[in] rMin  Min radius index
   * @param[in] rMax  Max radius index
   * @param[in] difPrevScan  Difference IQ
   */
  void smoothClose(int rMin, int rMax, const FieldDataPair &difPrevScan);

  /**
   * Smooth far from radar
   * @param[in] rMax  Max radius index for not far
   * @param[in] difPrevScan  Difference IQ
   */
  void smoothFar(int rMax, const FieldDataPair &difPrevScan);

  /**
   * @return pointer to I
   */
  inline const MdvxField *getI(void) const {return _I;}

  /**
   * @return pointer to Q
   */
  inline const MdvxField *getQ(void) const {return _Q;}

  /**
   * Add local IQ to output Mdv object
   * @param[in,out]  mdvFile
   */
  void addToOutput(DsMdvx &mdvFile) const;

  /**
   * Create a fieldwithData from "I", giving it the input name and units
   * Set to missing everywhere
   * @param[in] name
   * @param[in] units
   */
  FieldWithData createFromI(const std::string &name,
			    const std::string &units) const;

  /**
   * Create a fieldwithData from "I", giving it the input name and units
   * Set to input value everywhere
   * @param[in] name
   * @param[in] units
   * @param[in] value
   */
  FieldWithData createFromI(const std::string &name, const std::string &units,
			    double value) const;

  /**
   * @return number of gridpoints in a scan
   */
  inline int scanSize(void) const {return _scanSize;}

  /**
   * @return number of azmimuths
   */
  inline int numAzim(void) const {return _numAzimuth;}

  /**
   * @return number of gates
   */
  inline int numGates(void) const {return _numGates;}

  /**
   * @return gate spacing from field header
   */
  double gateSpacing(void) const;

  /**
   * @return azimuthal spacing from field header
   */
  double azimuthalSpacing(void) const;

  /**
   * @return data missing value for I
   */
  double missingValueI(void) const;

  /**
   * @return data missing value for Q
   */
  double missingValueQ(void) const;

  /**
   * @return true if I or Q is missing or bad  at a point
   * @param[in] index
   */
  bool missingIorQ(int index) const;

  /**
   * @return true if input gate spacing is not equal to local
   * @param[in] gate_spacing
   */
  bool wrongGateSpacing(double gate_spacing) const;

  /**
   * set args to grid dimensions
   * @param[out] ngates
   * @param[out] nazim
   */
  inline void getDimensions(int &ngates, int &nazim) const
  {
    ngates = _numGates;
    nazim = _numAzimuth;
  }

  /**
   * @return true if input value is bad or missing I
   * @param[in] b
   */
  inline bool isBadOrMissingI(fl32 b) const
  {
    return b == _IBad || b == _IMissing;
  }

  /**
   * @return true if input value is bad or missing Q
   * @param[in] b
   */
  inline bool isBadOrMissingQ(fl32 b) const
  {
    return b == _QBad || b == _QMissing;
  }

protected:
private:

  MdvxField *_I;   /**< Field containing the I component  */
  MdvxField *_Q;   /**< Field containing the Q component  */
  int _scanSize;   /**< Number of points in the scan */
  int _numAzimuth; /**< Number of azimuths */
  int _numGates;   /**< Number of gates */
  double _IBad;    /**< Bad value, I */
  double _QBad;    /**< Bad value, Q */
  double _IMissing;/**< Missing value, I */
  double _QMissing;/**< Missing value, Q */
  fl32 *_idata;    /**< pointer to data within _I */
  fl32 *_qdata;    /**< pointer to data within _Q */

  MdvxField *_createI(const std::string &name, const std::string &units) const;
  MdvxField *_createQ(const std::string &name, const std::string &units) const;
  MdvxField *_createI(const std::string &name, const std::string &units,
		      double value) const;
  MdvxField *_createQ(const std::string &name, const std::string &units,
		      double value) const;
  void _free(void);
};


#endif
