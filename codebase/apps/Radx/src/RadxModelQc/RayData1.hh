/**
 * @file RayData1.hh 
 * @brief Container for the processing of one ray. Has pointer to a ray,
 *        plus pointers to previous and next rays.
 * @class RayData1
 * @brief Container for the processing of one ray. Has pointer to a ray,
 *        plus pointers to previous and next rays.
 */

#ifndef RAY_DATA_1_H
#define RAY_DATA_1_H

#include <radar/RadxAppRayData.hh>

class Volume;

//------------------------------------------------------------------
class RayData1 : public RadxAppRayData
{
public:

  /**
   * Empty constructor
   */
  RayData1(void);

  /**
   * Set up for index'th ray in the Volume
   *
   * @param[in] r The volume object
   * @param[in] index  Ray index
   */
  RayData1(const Volume &r, int index);

  /**
   * Destructor
   */
  virtual ~RayData1(void);

  /**
   * Virtuals from MathData class for Radx
   */
  #include <radar/RadxAppMathDataVirtualMethods.hh>

  /**
   * @return true if previous ray is not set
   */
  inline bool prevMissing(void) const {return _ray0 == NULL;}

  /**
   * @return true if next ray is not set
   */
  inline bool nextMissing(void) const {return _ray1 == NULL;}

  /**
   * @return pointer to the center ray
   */
  inline const RadxRay *rPtr(void) const {return _ray;}

  /**
   * @return pointer to the previous ray
   */
  inline const RadxRay *r0Ptr(void) const {return _ray0;}
  /**
   * @return pointer to the next ray
   */
  inline const RadxRay *r1Ptr(void) const {return _ray1;}

protected:
private:

  static const std::string _azGradientStr;     /**< User unary */
  static const std::string _qscaleStr;         /**< User unary */
  static const std::string _oneMinusQscaleStr; /**< User unary */
  static const std::string _clutter2dQualStr;  /**< User unary */
  static const std::string _special0Str;       /**< User unary */
  static const std::string _special1Str;       /**< User unary */
  static const std::string _FIRStr;            /**< User unary */
  static const std::string _variance1dStr;     /**< User unary */

  RadxRay *_ray0;            /**< Pointer to previous ray or NULL */
  RadxRay *_ray1;            /**< Pointer to next ray or NULL */
  SpecialUserData _special;  /**< Special data, passed in from volume */

  /**
   * Names of user derived inputs to a filter
   */
  SpecialUserData _specialInp;

  bool _processAzGradient(std::vector<ProcessingNode *> &args);
  bool _processQscale(std::vector<ProcessingNode *> &args,
		      bool subtractFromOne) const;
  bool _processClutter2dQual(std::vector<ProcessingNode *> &args) const;
  bool _processSpecial0(std::vector<ProcessingNode *> &args);
  bool _processSpecial1(std::vector<ProcessingNode *> &args);
  bool _processFIR(std::vector<ProcessingNode *> &args) const;
  bool _processVariance1d(std::vector<ProcessingNode *> &args) const;
};

#endif
