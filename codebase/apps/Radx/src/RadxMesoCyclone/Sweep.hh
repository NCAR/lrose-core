/**
 * @file Sweep.hh
 * @brief Sweep data
 * @class Sweep
 * @brief Sweep data
 */

#ifndef SWEEP_HH
#define SWEEP_HH

#include "Parms.hh"
#include "TemplateLookupMgr.hh"
#include <FiltAlgVirtVol/VirtVolSweep.hh>
#include <rapmath/SpecialUserData.hh>
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
   */
  Sweep(const Volume &volume, int index, double vlevel);

  /**
   * Destructor
   */
  virtual ~Sweep(void);

  // bool isCircular(void) const;

  #define FILTALG_DERIVED
  #include <rapmath/MathDataVirtualMethods.hh>
  #undef FILTALG_DERIVED
  
protected:
private:

  static const std::string _mesoTemplateStr;
  static const std::string _nyquistTestStr;
  static const std::string _azShearStr;
  static const std::string _expandStr;
  static const std::string _clumpFiltStr;
  static const std::string _clumpExtentStr;
  static const std::string _clumpLocStr;
  static const std::string _interestInMaskStr;

  Parms _parms;

  /**
   * Pointer from volume object
   */
  const std::vector<TemplateLookupMgr> *_templates;

  bool _needToSynch(const std::string &userKey) const;
  bool _processMesoTemplate(std::vector<ProcessingNode *> &args);
  bool _processNyquistTest(std::vector<ProcessingNode *> &args);
  bool _processAzShear(std::vector<ProcessingNode *> &args);
  bool _processClumpFilt(std::vector<ProcessingNode *> &args);
  bool _processExpand(std::vector<ProcessingNode *> &args);
  bool _processClumpExtent(std::vector<ProcessingNode *> &args,
			   bool doExtend);
  bool _processDist2Missing(std::vector<ProcessingNode *> &args);
  bool _processInterestInMask(std::vector<ProcessingNode *> &args);
  void _azShear(const Grid2d &input, int r, int a, int aplus,
		int aminus, Grid2d &g) const;
};

#endif
