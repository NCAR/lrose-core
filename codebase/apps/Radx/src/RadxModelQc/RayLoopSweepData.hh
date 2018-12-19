/**
 * @file RayLoopSweepData.hh 
 * @brief Container for one sweep, one field
 *
 * @class RayLoopSweepData
 * @brief Container for one sweep, one field
 */

#ifndef RAY_LOOP_SWEEP_DATA_H
#define RAY_LOOP_SWEEP_DATA_H
#include <vector>
#include <string>
#include <Radx/RayxData.hh>
#include <rapmath/MathLoopData.hh>

class CircularLookupHandler;

//------------------------------------------------------------------
class RayLoopSweepData : public MathLoopData
{
public:

  RayLoopSweepData(void);
  RayLoopSweepData(const std::string &name, 
		   int i0, int i1, const std::vector<RayxData> &rays);


  /**
   * Destructor
   */
  virtual ~RayLoopSweepData(void);


  #include <rapmath/MathLoopDataVirtualMethods.hh>

  inline std::string getName(void) const {return _name;}
  void modifyForOutput(const std::string &name, const std::string &units);
  int getNGates(void) const;
  int getNGatesMax(void) const;
  void retrieveRayData(int rayIndex, float *data, int nGatesPrimary) const;
  std::string getUnits(void) const;
  int getNpoints(int rayIndex) const;
  double getMissing(void) const;
  bool variance2d(const RayLoopSweepData &inp,
		  const CircularLookupHandler &lookup);
protected:
private:

  int _i0, _i1;
  string _name;
  std::vector<RayxData> _rays;
  int _numDataPerRay;
  int _numRay;

  /**
   * @return Two dimensional y index for one dimensional index
   * @param[in] ipt  One dimensional index
   */
  inline int _rayIndex(const int ipt) const 
  { 
    return ipt/_numDataPerRay;
  }

  /**
   * @return Two dimensional x index for one dimensional index
   * @param[in] ipt  One dimensional index
   */
  inline int _gateIndex(const int ipt) const
  {
    return ipt -  _rayIndex(ipt)*_numDataPerRay;
  }

  bool _update2dVarOnRay(int i, const RayLoopSweepData &inp,
			 const CircularLookupHandler &lookup,
			 bool circular);
  bool _update2dVarGate(int i, int r, const RayLoopSweepData &inp,
			const CircularLookupHandler &lookup,
			bool circular);
  bool _addLookupToData(int i, int r, int rj, int aj,
			const RayLoopSweepData &inp,
			const CircularLookupHandler &lookup, bool circular,
			vector<double> &data, double &count) const;
};

#endif
