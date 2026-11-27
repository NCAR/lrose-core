/**
 * @file VolumeTemplate.hh 
 * @brief A simple derived class with empty virtual methods (return NULL)
 *
 * @class VolumeTemplate
 * @brief A simple derived class with empty virtual methods (return NULL)
 */

#ifndef VOLUME_TEMPLATE_H
#define VOLUME_TEMPLATE_H

#include "VolumeBase.hh"

//------------------------------------------------------------------
class VolumeTemplate : public VolumeBase
{
public:

  /**
   * empty
   */
  VolumeTemplate(void);

  /**
   * Destructor
   */
  virtual ~VolumeTemplate(void);

  /**
   * Set template (base class) to input
   * @param[in] b
   */
  void setV(const VolumeBase &b);

  /**
   * virtual method
   * @return NULL
   */
  virtual MathUserData *volumeInit(void);

  /**
   * virtual method
   * @return NULL
   */
  virtual MathUserData *volumeFinish(void);

  /**
   * virtual method
   * @return NULL
   */
  virtual RadxPersistentClutter *algPtr(void) const;

protected:
private:

};

#endif
