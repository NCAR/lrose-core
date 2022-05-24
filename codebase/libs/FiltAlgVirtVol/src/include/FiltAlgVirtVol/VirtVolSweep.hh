/**
 * @file VirtVolSweep.hh
 * @brief VirtVolSweep   Any number of 2d grids and any number of
 *                       special data pointers.
 * @class VirtVolSweep
 * @brief VirtVolSweep   Any number of 2d grids and any number of
 *                       special data pointers.
 */

#ifndef VIRTVOL_SWEEP_HH
#define VIRTVOL_SWEEP_HH

#include <FiltAlgVirtVol/FiltAlgParms.hh>
#include <FiltAlgVirtVol/GriddedData.hh>
#include <rapmath/SpecialUserData.hh>
#include <rapmath/MathData.hh>
#include <Mdv/MdvxProj.hh>
#include <Mdv/MdvxRadar.hh>
#include <vector>

class VirtVolVolume;
class Algorithm;

//------------------------------------------------------------------
class VirtVolSweep : public MathData
{
public:

  /**
   * Empty constructor
   */
  VirtVolSweep(void);

  /**
   * Constructor
   * @param[in] volume  Object from which sweep is extracted
   * @param[in] index  Index into sweeps 0 = lowest
   * @param[in] vlevel Vertical level for the sweep
   */
  VirtVolSweep(const VirtVolVolume &volume, int index, double vlevel);

  /**
   * Destructor
   */
  virtual ~VirtVolSweep(void);

  /**
   * @return the function definitions for those functions supported by VirtVolSweep
   * 
   * Each such function is supported within any derived class
   */
  static std::vector<FunctionDef> virtVolUserUnaryOperators(void);

  /**
   * @return true if it is a full 360 degree sweep
   */
  bool isCircular(void) const;

  /**
   * Deal with all inputs and the one output by filling in local state
   * _inps and _outputSweep values.
   *
   * Can copy from _grid2d to _data
   *
   * @param[in] output  The name of the output
   * @param[in] inputs  The names of the inputs
   * @param[out] haveAll set to true if all inputs have been accounted for
   *                     can be false if some inputs are not gridded
   *
   * @return true if output was set, false otherwise.
   */
  bool
  synchGriddedInputsAndOutputs(const std::string &output,
			       const std::vector<std::string> &inputs,
			       bool &haveAll);
  /**
   * @return true if input name is one of the _inps 
   * @param[in] name
   */
  bool isSynchedInput(const std::string &name) const;

  /**
   * @return true for clockwise azimuthal increments
   */
  inline bool clockwise(void) const {return _clockwise;}

  #define FILTALG_BASE
  #include <rapmath/MathDataVirtualMethods.hh>
  #undef FILTALG_BASE

  /**
   * Process using information from the input processing nod.
   *
   * It is expected that the function specified will be one of those supported within
   * VirtVolSweep (see the static private strings)
   *
   * @param[in] p   The node with keyword, and args
   * @return true for success, false for failure
   */ 
  bool processVirtVolUserLoopFunction(ProcessingNode &p);

  /**
   * @return reference to the data vector
   */
  inline const std::vector<GriddedData> &newDataRef(void) const {return _data;}

  /**
   * @return pointer to named special data, or NULL for no match
   * @param[in] name
   */
  const MathUserData *specialDataPtrConst(const std::string &name) const;

    /**
   * @return pointer to named special data, or NULL for no match
   * @param[in] name
   */
  MathUserData *specialDataPtr(const std::string &name);


protected:
  MdvxProj _proj;   /**< Grid projection */
  bool _clockwise;  /**< True for clockwise azimuthal increments */
  double _vlevel;   /**< Vertical level */
  int _vlevelIndex; /**< Vertical level index */
  time_t _time;     /**< Time of volume */
  const FiltAlgParms *_parms;  /**< Parameters pointer (to derived class) */
  const SpecialUserData *_inpSpecial;  /**< Special data from Volume */
  const std::vector<GriddedData> *_grid2d; /**< Input grids from Volume */

  /**
   * data grids filled in by filtering, eventually returned to Volume
   */
  std::vector<GriddedData> _data;

  /**
   * Map from names of special data fields to pointer to special values
   * This gets filled in by the filtering
   */
  SpecialUserData _special;

  /**
   * Each filter modifies this to produce pointers to its inputs
   */
  std::vector<GriddedData *> _inps;

  /**
   * Each filter modifies this to produce a pointer to its output
   */
  GriddedData *_outputSweep;
  
  GriddedData *_refToData(const std::string &name, bool suppressWarn=false);
  GriddedData *_exampleData(const std::string &name);
  GriddedData *_match(const std::string &n);
  const GriddedData *_matchConst(const std::string &n) const;
  bool _loadGridValueValue(std::vector<ProcessingNode *> &args,
			   const GriddedData **field, double &v0,
			   double &v1) const;
  bool
  _loadGridandPairs(std::vector<ProcessingNode *> &args,
		    const GriddedData **field,
		    std::vector<std::pair<double,double> > &fuzzyPairs) const;
  bool
  _loadValueAndMultiFields(std::vector<ProcessingNode *> &args,
			   double &value,
			   std::vector<const GriddedData *> &fields) const;
  bool _loadMultiFields(std::vector<ProcessingNode *> &args,
			std::vector<const GriddedData *> &fields) const;
private:
  static const std::string _percentLessThanStr; /**< Function keyword */
  static const std::string _largePositiveNegativeStr; /**< Function keyword */
  static const std::string _smoothPolarStr; /**< Function keyword */
  static const std::string _smoothWithThreshPolarStr; /**< Function keyword */
  static const std::string _dilatePolarStr; /**< Function keyword */
  static const std::string _medianPolarStr; /**< Function keyword */
  static const std::string _percentOfAbsMaxStr; /**< Function keyword */
  static const std::string _azimuthalPolarShearStr; /**< Function keyword */
  static const std::string _clumpFiltStr; /**< Function keyword */
  static const std::string _virtVolFuzzyStr; /**< virtual volume fuzzy function keyword */
  static const std::string _expandInMaskStr;/**< Function keyword */

  bool _processPercentLessThan(std::vector<ProcessingNode *> &args);
  bool _processLargePositiveNegative(std::vector<ProcessingNode *> &args);
  bool _processSmoothPolar(std::vector<ProcessingNode *> &args);
  bool _processSmoothWithThreshPolar(std::vector<ProcessingNode *> &args);
  bool _processDilatePolar(std::vector<ProcessingNode *> &args);
  bool _processMedianPolar(std::vector<ProcessingNode *> &args);
  bool _processPercentOfAbsMax(std::vector<ProcessingNode *> &args);
  bool _processAzimuthalPolarShear(std::vector<ProcessingNode *> &args);
  bool _processClumpFilt(std::vector<ProcessingNode *> &args);
  bool _processFuzzy(std::vector<ProcessingNode *> &args);
  bool _processExpandInMask(std::vector<ProcessingNode *> &args);
};

#endif
