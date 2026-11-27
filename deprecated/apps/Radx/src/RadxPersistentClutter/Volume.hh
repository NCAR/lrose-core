/**
 * @file Volume.hh 
 * @brief Data handler, derived from VolumeBase, contains an algorithm pointer
 *
 * @class Volume
 * @brief Data handler, derived from VolumeBase, contains an algorithm pointer
 */

#ifndef VOLUME_H
#define VOLUME_H

#include "VolumeBase.hh"

//------------------------------------------------------------------
class Volume : public VolumeBase
{
public:

  /**
   * Empty
   */
  Volume(void);

  /**
   * @param[in] parms
   * @param[in] argc
   * @param[in] argv
   */
  Volume(const Parms *parms, int argc, char **argv);

  /**
   * Destructor
   */
  virtual ~Volume(void);

  /**
   * virtual method to initialize a volume prior to ray specific 
   * algorithmic steps
   * @return pointer to the status, which is a downcast StatusUserData ptr
   */
  virtual MathUserData *volumeInit(void);

  /**
   * virtual method to finalize a volume after all the ray specific 
   * algorithmic steps
   * @return pointer to the status, which is a downcast StatusUserData ptr
   */
  virtual MathUserData *volumeFinish(void);

  /**
   * Pure virtual method to point to the algorithm object
   * @return pointer to the RadxPersistentClutter base class
   */
  virtual RadxPersistentClutter *algPtr(void) const;

protected:
private:

  RadxPersistentClutter *_p;  /**< Pointer to the base class algorithm object*/
};

#endif
