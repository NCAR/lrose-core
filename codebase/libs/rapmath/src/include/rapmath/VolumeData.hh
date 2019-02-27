/**
 * @file VolumeData.hh 
 * @brief Container for data specific to your app
 * @class VolumeData
 * @brief Container for data specific to your app
 *
 * The data for an entire 'volume', or whatever the triggering data content is
 * 
 * A pure virtual base class
 */

#ifndef VOLUME_DATA_H
#define VOLUME_DATA_H

class MathData;
class ProcessingNode;
class MathUserData;
class UnaryNode;
class FunctionDef;

#include <vector>
#include <string>

class VolumeData
{
public:
  /**
   * Empty constructor
   */
  inline VolumeData(void) {}

  /**
   * Destructor
   */
  inline virtual ~VolumeData(void) {}

#define VOLUME_DATA_BASE
  #include <rapmath/VolumeDataVirtualMethods.hh>
#undef VOLUME_DATA_BASE

protected:
private:

};

#endif
