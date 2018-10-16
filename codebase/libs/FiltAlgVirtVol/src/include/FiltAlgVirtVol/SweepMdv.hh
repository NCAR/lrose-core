/**
 * @file SweepMdv.hh
 * @brief SweepMdv data.  Any number of 2d grids and any number of special data
 *                        pointers.
 * @class SweepMdv
 * @brief SweepMdv data.  Any number of 2d grids and any number of special data
 *                        pointers.
 */

#ifndef SWEEP_MDV_HH
#define SWEEP_MDV_HH

#include <FiltAlgVirtVol/FiltAlgParms.hh>
#include <FiltAlgVirtVol/GriddedData.hh>
#include <rapmath/SpecialUserData.hh>
#include <rapmath/MathData.hh>
#include <Mdv/MdvxProj.hh>
#include <Mdv/MdvxRadar.hh>
#include <vector>

class VolumeMdv;
class Algorithm;

//------------------------------------------------------------------
class SweepMdv : public MathData
{
public:

  /**
   * Empty constructor
   */
  SweepMdv(void);

  /**
   * Constructor
   * @param[in] volume  Object from which sweep is extracted
   * @param[in] index  Index into sweeps 0 = lowest
   * @param[in] vlevel Vertical level for the sweep
   */
  SweepMdv(const VolumeMdv &volume, int index, double vlevel);

  /**
   * Destructor
   */
  virtual ~SweepMdv(void);

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

  #define FILTALG_BASE
  #include <rapmath/MathDataVirtualMethods.hh>
  #undef FILTALG_BASE

  /**
   * @return reference to the data vector
   */
  const std::vector<GriddedData> &newDataRef(void) const {return _data;}

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
  MdvxProj _proj;   /**< Grid projectsion */
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
};

#endif
