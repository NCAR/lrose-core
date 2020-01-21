// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
// © University Corporation for Atmospheric Research (UCAR) 2009-2010. 
// All rights reserved.  The Government's right to use this data and/or 
// software (the "Work") is restricted, per the terms of Cooperative 
// Agreement (ATM (AGS)-0753581 10/1/08) between UCAR and the National 
// Science Foundation, to a "nonexclusive, nontransferable, irrevocable, 
// royalty-free license to exercise or have exercised for or on behalf of 
// the U.S. throughout the world all the exclusive rights provided by 
// copyrights.  Such license, however, does not include the right to sell 
// copies or phonorecords of the copyrighted works to the public."   The 
// Work is provided "AS IS" and without warranty of any kind.  UCAR 
// EXPRESSLY DISCLAIMS ALL OTHER WARRANTIES, INCLUDING, BUT NOT LIMITED TO, 
// ANY IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR 
// PURPOSE.  
//  
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
#include <toolsa/copyright.h>
/**
 * @file DataHandlerCfRadial.hh
 * @brief Handles data input/output for CfRadial data stored in RadxVol
 * @class DataHandlerCfRadial
 * @brief Handles data input/output for CfRadial data stored in RadxVol
 */

# ifndef    DATA_HANDLER_CFRADIAL_H
# define    DATA_HANDLER_CFRADIAL_H

#include "DataHandler.hh"
#ifndef NO_NETCDF
#include <Radx/RadxVol.hh>
#endif

//----------------------------------------------------------------
class DataHandlerCfRadial : public DataHandler
{
public:

  /**
   * Default constructor
   *
   * @param[in] p  The parameters to put into state
   */
  DataHandlerCfRadial(const Params &p);
  
  /**
   * Copy constructor
  */
  DataHandlerCfRadial(const DataHandlerCfRadial &d);

  /**
   * @return a created copy of the derived class, down cast to base class
   */
  virtual DataHandler *clone(void) const;

  /**
   *  Destructor
   */
  virtual ~DataHandlerCfRadial(void);

  virtual bool init(void);
  virtual void reset(void);
  virtual void store(const Data &data);
  virtual void write(const time_t &t);

protected:

  virtual void _initIndexing(void);
  virtual Xyz _computeXyzMeters(void);
  virtual bool _increment(void);

private:  

#ifndef NO_NETCDF
  RadxVol _vol;            /**< Volume of data */
  vector<RadxRay *> _rays; /**< The rays within that volume */
#endif

  int _iray;  /**< index into rays */
  int _j;     /**< index into gates within a ray */

  bool _setRays(void);
};

# endif 
