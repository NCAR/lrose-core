/*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
 ** Copyright UCAR (c) 1992 - 1999
 ** University Corporation for Atmospheric Research(UCAR)
 ** National Center for Atmospheric Research(NCAR)
 ** Research Applications Program(RAP)
 ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
 ** All rights reserved. Licenced use only.
 ** Do not copy or distribute without authorization
 ** 1999/03/14 14:18:54
 *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*/
#include <copyright.h>

//   File: $RCSfile: Parms.hh,v $
//   Version: $Revision: 1.1 $  Dated: $Date: 2010/08/16 23:33:17 $

/**
 * @file Parms.hh
 * @brief all parms.
 * @class Parms
 * @brief all parms.
 * @note
 * @todo
 */

# ifndef    Parms_HH
# define    Parms_HH

#include <string>
#include <vector>
#include <FiltAlg/FiltAlgParms.hh>
#include "Params.hh"
using namespace std;

/*----------------------------------------------------------------*/
class Parms  : public FiltAlgParms
{
public:

  /**
   * default constructor
   */
  Parms();
  Parms(int argc, char **argv);
  
  /**
   *  destructor
   */
  virtual ~Parms();


  /**
   * Public members
   */
  Params _main;

  static Params::app_filter_t name_to_type(const string &name);


  virtual int 
  app_max_elem_for_filter(const FiltAlgParams::data_filter_t f) const;
  virtual void *app_parms(void) const;

protected:
private:  
};

# endif     /* Parms_HH */
