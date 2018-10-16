/**
 * @file Sweep.hh
 * @brief Sweep data
 * @class Sweep
 * @brief Sweep data
 */

#ifndef SWEEP_HH
#define SWEEP_HH

#include "RepohParms.hh"
#include "KernelOutputs.hh"
#include "AsciiOutputs.hh"
#include "VolumeMdvInfo.hh"
#include "GridFields.hh"
#include <FiltAlgVirtVol/SpecialUserData.hh>
#include <rapmath/MathData.hh>
#include <vector>
#include <map>

class Volume;
class CloudGap;
class ClumpRegions;
class GriddedData;

//------------------------------------------------------------------
class Sweep : public MathData
{
public:

  /**
   * Constructor

   */
  Sweep(const Volume &volume, int index, double vlevel);

  /**
   * Destructor
   */
  virtual ~Sweep(void);


  #include <rapmath/MathDataVirtualMethods.hh>

  const GridFields &derivedDataRef(void) const {return _derivedData;}

  /**
   * Clump index 0,1,2,...   maps to colors 1,2,3,...
   */
  inline static double indexToColor(const int index)
  {
    return (double)(index+1);
  }

  /**
   * colors 1,2,3,... map to clump index 0,1,2,...
   */
  inline static int colorToIndex(const double color)
  {
    return (int)(color-1.0);
  }

protected:
private:

  VolumeMdvInfo _mdvInfo;       /**< Input from volume */
  KernelOutputs _kernelOutputs; /**< Input from volume, shared by volume */
  AsciiOutputs _asciiOutputs;   /**< Input from volume, shared by volume */
  time_t _time;
  RepohParms _repohParms;

  double _vlevel;          
  int _vlevelIndex;

  /**
   * One per field, copied in from volume
   */
  GridFields _inputGrids;

  /**
   * Map from names of special data fields to pointer to special values
   * This gets filled in by the filtering
   */
  SpecialUserData _special;

  /**
   * Derived values, filled in by filters as the filters get executed
   */
  GridFields _derivedData;

  /**
   * Each filter modifies to produce pointer to its output. The output is
   * either gridded data, kernel outputs (SPDB), or ascii outputs
   */
  GriddedData *_outputSweep;
  KernelOutput *_outputKernels;
  AsciiOutput *_outputAscii;


  bool _stageInputs(const std::vector<std::string> &inputs);
  bool _stageOutput(const std::string &output);
  GriddedData *_refToData(const std::string &name, bool suppressWarn=false);
  GriddedData *_exampleData(const std::string &name);
  bool _needToSynch(const std::string &userKey) const;
  // bool _processTextureX(std::vector<ProcessingNode *> &args);
  // bool _processStdDevNoOverlap(std::vector<ProcessingNode *> &args);
  // bool _processMedianNoOverlap(std::vector<ProcessingNode *> &args);
  // bool _processSnrFromDbz(std::vector<ProcessingNode *> &args);
  bool _clumpsToGrid(std::vector<ProcessingNode *> &args);
  bool _processRemoveSmallClumps(std::vector<ProcessingNode *> &args);
  MathUserData *_processAssociateClumps(const std::string &reg,
					const std::string &clump0,
					const std::string &clump1);
  MathUserData *_createClumps(const std::string &dataName);
  MathUserData *_processBuildGaps(const std::string &clumps,
				  const std::string &depth);
  MathUserData *_removeSmallGaps(const std::string &gaps,
				 const std::string &mingap);
  MathUserData *_filterGapsInside(const std::string &gaps,
				  const std::string &pidClumps,
				  const std::string &associatedClumps,
				  const std::string &maxPenetration);

  bool _getEdge(std::vector<ProcessingNode *> &args);
  bool _getOutside(std::vector<ProcessingNode *> &args);

  bool _inverseMask(std::vector<ProcessingNode *> &args);
  MathUserData *_kernelBuild(const std::string &outsideMask,
			     const std::string &gaps,
			     const std::string &clumpReg,
			     const std::string &kgrids);
  MathUserData *_kernelFilter(const std::string &kernels);
  MathUserData *_kernelToGenPoly(const std::string &kernels);
  bool _centerPoints(std::vector<ProcessingNode *> &args);
  bool _attenuation(std::vector<ProcessingNode *> &args);
  bool _nptBetweenGood(std::vector<ProcessingNode *> &args);
  bool _averageAttenuation(std::vector<ProcessingNode *> &args);
  bool _totalAttenuation(std::vector<ProcessingNode *> &args);
  bool _sumZ(std::vector<ProcessingNode *> &args);
  bool _humidity(std::vector<ProcessingNode *> &args);
  bool _fir(std::vector<ProcessingNode *> &args);
  MathUserData *_processAsciiOutput(const std::string &kernels);
  bool _kernel_mask(const CloudGap &gap, 
  		    const ClumpRegions &regions,
  		    const bool is_far, Grid2d &mask) const;
  MathUserData *_organize(const std::string &pid,
			  const std::string &snoise,
			  const std::string &knoise,
			  const std::string &sdbz,
			  const std::string &kdbz,
			  const std::string &szdr,
			  const std::string &srhohv,
			  const std::string &kdbzAdjusted,
			  const std::string &dbzDiff);
};

#endif
