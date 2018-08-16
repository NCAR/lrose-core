/**
 * @file RayData1.hh 
 * @brief Container for the processing of one ray
 * @class RayData1
 * @brief Container for the processing of one ray
 */

#ifndef RAY_DATA_1_H
#define RAY_DATA_1_H
#include "RayLoopData.hh"
#include <rapmath/VolumeData.hh>
#include <rapmath/MathData.hh>
#include <Radx/RadxVol.hh>
#include <Radx/RayxData.hh>
#include <vector>
#include <map>
#include <string>

class MathUserData;
class RayData;

//------------------------------------------------------------------
class RayData1 : public MathData
{
public:

  /**
   * Set up for index'th ray in the RadxVol
   *
   * @param[in] The volume object
   * @param[in] index  Ray index
   */
  RayData1(const RayData &r, int index);

  /**
   * Destructor
   */
  virtual ~RayData1(void);

  #include <rapmath/MathDataVirtualMethods.hh>
  
protected:
private:


  RadxRay *_ray;        /**< Pointer to Center ray (main one ) */
  RadxRay *_ray0;       /**< Pointer to previous ray or NULL */
  RadxRay *_ray1;       /**< Pointer to next ray or NULL */

  /**
   * The names of special data fields, coming from RayData creator
   */
  std::vector<std::string> _specialName;

  /**
   * Pointers to special data to go with each such field
   */
  std::vector<MathUserData *> _specialValue;

  /**
   * Derived data as filters are processed gets put here
   */
  std::vector<RayLoopData> _data;

  /**
   * Each filter modifies to produce pointers to its particular inputs
   */
  std::vector<RayLoopData *> _inps;

  /**
   * Each filter modifies to produce pointer to its particular output
   */
  RayLoopData *_outputRay;

  /**
   * Names of user derived inputs to a filter
   */
  std::vector<std::string> _specialInpNames;

  /**
   * Pointers to user derived inputs to a filter
   */
  std::vector<MathUserData *> _specialInps;
  
  RayLoopData *_refToData(const std::string &name, bool suppressWarn=false);
  RayLoopData *_exampleData(const std::string &name);
  void _updateRay(void);
  bool _needToSynch(const std::string &userKey) const;
  RayLoopData *_match(const std::string &n);
  const RayLoopData *_matchConst(const std::string &n) const;
  bool _processAzGradient(std::vector<ProcessingNode *> &args) const;
  bool _processQscale(std::vector<ProcessingNode *> &args,
		     bool subtractFromOne) const;
  bool _processClutter2dQual(std::vector<ProcessingNode *> &args) const;
  bool _processSpecial0(std::vector<ProcessingNode *> &args) const;
  bool _processSpecial1(std::vector<ProcessingNode *> &args) const;
  bool _processFIR(std::vector<ProcessingNode *> &args) const;
  bool _processVariance1d(std::vector<ProcessingNode *> &args) const;

  /**
   * Can write to _data
   */
  bool _isInput(const std::string &name);
  void _setLocalInput(const std::string &input);
};

#endif
