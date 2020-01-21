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
 * @file DataHandlerMdv.hh
 * @brief Handles data input/output for Mdv data stored in DsMdvx
 * @class DataHandlerMdv
 * @brief Handles data input/output for Mdv data stored in DsMdvx
 */

# ifndef    DATA_HANDLER_MDV_H
# define    DATA_HANDLER_MDV_H

#include "DataHandler.hh"
#include "DataGrid.hh"
#include <Mdv/DsMdvx.hh>
#include <map>

//----------------------------------------------------------------
class DataHandlerMdv : public DataHandler
{
public:

  /**
   * Default constructor
   *
   * @param[in] p  The parameters to put into state
   */
  DataHandlerMdv(const Params &p);
  
  /**
   * Copy constructor
   */
  DataHandlerMdv(const DataHandlerMdv &d);

  /**
   * @return a created copy of the derived class, down cast to base class
   */
  virtual DataHandler *clone(void) const;

  /**
   *  Destructor
   */
  virtual ~DataHandlerMdv(void);

  virtual bool init(void);
  virtual void reset(void);
  virtual void store(const Data &data);
  virtual void write(const time_t &t);

protected:

  virtual void _initIndexing(void);
  virtual Xyz _computeXyzMeters(void);
  virtual bool _increment(void);

private:  

  /**
   * Object used to read and write 
   */
  DsMdvx _mdv;

  /**
   * Field header from template
   */
  Mdvx::field_header_t _field_hdr;

  /**
   * vlevel header from template
   */
  Mdvx::vlevel_header_t _vlevel_hdr;

  /**
   * storage of data for all fields that get written
   */
  std::map<int, DataGrid> _data;

  int _ix;  /**< Current Index into data */
  int _iy;  /**< Current Index into data */
  int _iz;  /**< Current Index into data */

  MdvxField *_read(const time_t &t);
  MdvxField *_regrid(void);
  bool _setProjectionType(const Mdvx::field_header_t &f);
  void _addField(const MdvxField &f, const Params::Field_t t);
};

# endif 
