/**
 * @file Sweep.hh
 * @brief Sweep data
 * @class Sweep
 * @brief Sweep data
 */

#ifndef SWEEP_HH
#define SWEEP_HH

#include "Parms.hh"
#include <FiltAlgVirtVol/VirtVolSweep.hh>
#include <vector>

class Volume;

//------------------------------------------------------------------
class Sweep : public VirtVolSweep
{
public:

  /**
   * Empty constructor
   */
  Sweep(void);

  /**
   * Constructor
   * @param[in] volume  The full volume
   * @param[in] index  Index into vertical levels 0,1,...
   * @param[in] vlevel  The actual vertical level angle degrees
   */
  Sweep(const Volume &volume, int index, double vlevel);

  /**
   * Destructor
   */
  virtual ~Sweep(void);

  #define FILTALG_DERIVED
  #include <rapmath/MathDataVirtualMethods.hh>
  #undef FILTALG_DERIVED
  
protected:
private:

  /**
   * Name of the special function using templates to estimate mesocyclones
   */
  static const std::string _mesoTemplateStr;

  /**
   * App params
   */
  Parms _parms;

  bool _needToSynch(const std::string &userKey) const;
  bool _processMesoTemplate(std::vector<ProcessingNode *> &args);
};

#endif
