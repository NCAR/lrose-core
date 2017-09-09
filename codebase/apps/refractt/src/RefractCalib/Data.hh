/**
 * @file Data.hh
 * @class Data.hh
 */

#ifndef DATA_H
#define DATA_H

#include <Refract/FieldDataPair.hh>
#include <Refract/FieldAlgo.hh>

class Data
{
public:

  /**
   * @brief Invalid data value.
   */

  static const float INVALID;

  /**
   * @brief Very large data value.
   */

  static const float VERY_LARGE;


  inline Data(void) : _fluctSnr(NULL), _pixelCount(NULL) {}
  inline ~Data(void) {}
  // {
  //   //_clear();
  // }
  bool gateSpacingChange(const FieldDataPair &I);
  double getGateSpacing(void) const;
  void allocate(const FieldDataPair &iq);
  void accumulateABP(const FieldDataPair &difPrevScan);
  void accumulateAB(const FieldDataPair &iq);
  void adjustmentsForSNR(const FieldAlgo &oldSNR,
			 const FieldWithData &SNR);
  void initialCalibration(int r_min);
  void sideLobeFiltering(double contamin_pow, int r_min, double side_lobe_pow,
			 const FieldAlgo &oldSnr);
  void calculateQuality(void);
  void setABToZero(void);
  void normalizeReference(void);
  void addCalibFields(DsMdvx &calib_file);
  void addDebug(DsMdvx &debug_file, bool first) const;
  void findReliableTargets(int r_min,  int r_max, const FieldDataPair &iq,
			   const FieldWithData &SNR,FieldDataPair &difPrevScan);
  // double *setPhaseTargetData(void) const;
  inline static double SQR(double value)
  {
    return value * value;
  }

  
  double *_fluctSnr;  /**< Array for recording fluctuations in the SNR values.*/
  int *_pixelCount;   /**< Array for storing the pixel count. */
  FieldAlgo _meanSnr;      /**< Mean Signal to noise field */
  FieldAlgo _sumP;         /**< P which is the normalization for AB*/
  FieldAlgo _calibStrength;/**< SNR-based strength of ref target. */
  FieldAlgo _calibPhaseEr; /**< Expected error of target phase. */
  FieldAlgo _calibQuality; /**< The quality of the target. */
  FieldAlgo _calibNcp;     /**< The calculated NCP value for the target.*/

  FieldDataPair _sumAB;        /**< AB summation */
  FieldDataPair _avIQ;         /**< Average IQ */
  FieldDataPair _difFromRefIQ; /**< Diff from reference IQ */
  FieldDataPair _calibAvIQ;    /**< Average of target for N=reference N value */

private:
  void _clear(void);
  

};
#endif
