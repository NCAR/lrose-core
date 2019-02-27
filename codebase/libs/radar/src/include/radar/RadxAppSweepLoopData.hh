/**
 * @file RadxAppSweepLoopData.hh 
 * @brief Container for one sweep, one field
 *
 * @class RadxAppSweepLoopData
 * @brief Container for one sweep, one field
 */

#ifndef RADX_APP_SWEEP_LOOP_DATA_H
#define RADX_APP_SWEEP_LOOP_DATA_H
#include <vector>
#include <string>
#include <Radx/RayxData.hh>
#include <rapmath/MathLoopData.hh>

class RadxAppCircularLookupHandler;

//------------------------------------------------------------------
class RadxAppSweepLoopData : public MathLoopData
{
public:

  /**
   * Empty constructor
   */
  RadxAppSweepLoopData(void);

  /**
   * Constructor
   * @param[in] name   Field name
   * @param[in] i0  Index to beginning of sweep in volume
   * @param[in] i1  Index to end of sweep in volume
   * @param[in] rays  Data, for entire volume? or for just the sweep? 
   *                  (not sure, need to check). A copy is Stored locally
   */
  RadxAppSweepLoopData(const std::string &name, int i0, int i1,
		   const std::vector<RayxData> &rays);


  /**
   * Destructor
   */
  virtual ~RadxAppSweepLoopData(void);


  #include <rapmath/MathLoopDataVirtualMethods.hh>

  /**
   * @return the field name
   */
  inline std::string getName(void) const {return _name;}

  /**
   * Change field name and units
   * @param[in] name  New name
   * @param[in] units  New units
   */
  void modifyForOutput(const std::string &name, const std::string &units);

  /**
   * @return number of gates in this sweep (max over all rays)
   */
  int getNGates(void) const;

  /**
   * @return units string 
   */
  std::string getUnits(void) const;

  /**
   * @return number of points (gates) in a ray
   * @param[in] rayIndex
   */
  int getNpoints(int rayIndex) const;

  /**
   * @return the missing data value
   */
  double getMissing(void) const;

  /**
   * Retrieve the data for a particular ray
   * @param[in] rayIndex  Index to the ray 
   * @param[in,out] data  An array to store data
   * @param[in] nGatesPrimary  Length of data array
   *
   * If data is larger than local ray, the data array is padded with missing
   */
  void retrieveRayData(int rayIndex, float *data, int nGatesPrimary) const;

  /**
   * Set 2d variance into local data using input data and a lookup table
   * @param[in] inp  Data to take variance of
   * @param[in] lookup  Lookup object
   */
  bool variance2d(const RadxAppSweepLoopData &inp,
		  const RadxAppCircularLookupHandler &lookup);
protected:
private:

  int _i0, _i1;  /**< Range of volume ray indices for this sweep */
  string _name;  /**< Data field name */
  std::vector<RayxData> _rays;  /**< The rays that make up the sweep */
  int _numDataPerRay;   /**< Maximum number of gates per ray */
  int _numRay;          /**< Total number of rays */

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

  bool _update2dVarOnRay(int i, const RadxAppSweepLoopData &inp,
			 const RadxAppCircularLookupHandler &lookup,
			 bool circular);
  bool _update2dVarGate(int i, int r, const RadxAppSweepLoopData &inp,
			const RadxAppCircularLookupHandler &lookup,
			bool circular);
  bool _addLookupToData(int i, int r, int rj, int aj,
			const RadxAppSweepLoopData &inp,
			const RadxAppCircularLookupHandler &lookup, bool circular,
			vector<double> &data, double &count) const;
};

#endif
