/**
 * @file Sweep.hh
 * @brief Sweep data
 * @class Sweep
 * @brief Sweep data
 */

#ifndef SWEEP_HH
#define SWEEP_HH

#include "Parms.hh"
#include "OldDataHandler.hh"
#include <FiltAlgVirtVol/VirtVolSweep.hh>

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
   * @param[in] volume  The entire volume 
   * @param[in] index   Index to a sweep within the volume
   * @param[in] vlevel  The vertical level value for this sweep
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

  static const std::string _lineDetStr;        /**< User Unary Operator */
  static const std::string _lineDirStr;        /**< User Unary Operator */
  static const std::string _shearDetStr;       /**< User Unary Operator */
  static const std::string _shearDirStr;       /**< User Unary Operator */
  static const std::string _ellipStr;          /**< User Unary Operator */
  static const std::string _ellipOrientStr;    /**< User Unary Operator */
  static const std::string _ellipConfStr;      /**< User Unary Operator */
  static const std::string _enhanceStr;        /**< User Unary Operator */
  static const std::string _enhanceDirStr;     /**< User Unary Operator */
  static const std::string _regionStr;         /**< User Unary Operator */
  static const std::string _regCombStr;        /**< User Unary Operator */
  static const std::string _historyStr;        /**< User Unary Operator */
  static const std::string _maxAgeMinutesStr;  /**< User Unary Operator */

  /**
   * Parameters
   */
  Parms _parms;

  /**
   * One per old field
   */
  OldSweepData _oldData;
  
  bool _processLineDir(std::vector<ProcessingNode *> &args);
  bool _processLineDet(std::vector<ProcessingNode *> &args);
  bool _processShearDir(std::vector<ProcessingNode *> &args);
  bool _processShearDet(std::vector<ProcessingNode *> &args);
  bool _processEllipOrient(std::vector<ProcessingNode *> &args);
  bool _processEllip(std::vector<ProcessingNode *> &args);
  bool _processEllipConf(std::vector<ProcessingNode *> &args);
  bool _processEnhance(std::vector<ProcessingNode *> &args);
  bool _processEnhanceDirection(std::vector<ProcessingNode *> &args);
  bool _processRegion(std::vector<ProcessingNode *> &args);
  bool _processRegComb(std::vector<ProcessingNode *> &args);
  bool _processHistory(std::vector<ProcessingNode *> &args);
  bool _processMaxAgeMinutes(std::vector<ProcessingNode *> &args);
};

#endif
