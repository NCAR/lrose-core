#ifndef PaletteManager_HH
#define PaletteManager_HH

#include <string>
#include <vector>

//#include "sii_utils.hh"
//#include "solo_window_structs.h"
//#include "soloii.h"
//#include "sp_basics.hh"

#include "SiiPalette.hh"

class PaletteManager
{

public:

  ////////////////////
  // Public methods //
  ////////////////////

  /**
   * @brief Destructor.
   */

  virtual ~PaletteManager();

  /**
   * @brief Retrieve the singleton instance of this class.
   *
   * @return Returns a pointer to the instance.
   */

  static PaletteManager *getInstance();
  

  ////////////////////
  // Public methods //
  ////////////////////

  void newPaletteForParam(const std::string &palette_name,
			  const std::string &param_name);
  
  void updatePaletteNames();
  
  /**
   * @brief Read a single palette from the given buffer and add that palette
   *        information to the stack.
   *
   * @param[in] buf_ptr    Pointer to the input buffer.
   * @param[in] gotta_swap Flag indicating whether we need to swap the data.
   */

  //void readPalette(char *buf_ptr, const bool gotta_swap);
  

  /**
   * @brief Write the palettes to the specified stream.
   *
   * @param[in,out] stream    The output stream.
   *
   * @return Returns true on success, false on failure.
   */

  bool writePalettes(FILE *stream) const;
  

  int getNumPaletteNames() const
  {
    return _paramPaletteNames.size();
  }
  
  std::string getPaletteName(const int palette_num) const
  {
    return _paramPaletteNames[palette_num];
  }
  
  std::string getParamPalettesListString() const
  {
    std::string return_string = "";
  
    std::vector< std::string >::const_iterator palette_name;
    for (palette_name = _paramPaletteNames.begin();
	 palette_name != _paramPaletteNames.end(); ++palette_name)
    {
      return_string += *palette_name;
      return_string += "\n";
    }
  
    return return_string;
  }

  std::vector< std::string > getPalettesList() const
  {
    return _paramPaletteNames;
  }


  /**
   * @brief Find the indicated palette in the stack.
   *
   * @param[in] palette_name    The name of the palette.
   * @param[in] remove          Flag indicating whether to remove the
   *                              palette from the stack upon return.
   *                              If set to true, the calling method is
   *                              responsible for deleting this pointer
   *                              when it is no longer needed.
   *
   * @return Returns a pointer to the indicated palette on success, 0 on
   *         failure.
   */

  SiiPalette *seek(const std::string &palette_name,
		   const bool remove = false)
  {
    for (std::vector< SiiPalette* >::iterator pal_iter = _paletteStack.begin();
	 pal_iter != _paletteStack.end(); ++pal_iter)
    {
      SiiPalette *pal = *pal_iter;
      if (pal->getPaletteName() == palette_name)
      {
	if (remove)
	  _paletteStack.erase(pal_iter);
	return pal;
      }
    }

    return 0;
  }

  SiiPalette *dupOpal(struct solo_palette *opal);
 /* 
  void broadcastGridColor(const SiiPalette *copy_palette)
  {
    // Update the color in each of the palettes

    for (size_t i = 0; i < _paletteStack.size(); ++i)
    {
      SiiPalette *pal = _paletteStack[i];
      if (pal == copy_palette)
	continue;
      pal->setGridColor(copy_palette->getGridColor());
    }

    // Signal that the window information has changed

    for (int jj = 0; jj < MAX_FRAMES; jj++)
      solo_return_wwptr(jj)->parameter.changed = 1;
  }
  */
/* 
  void broadcastBoundaryColor(const SiiPalette *copy_palette)
  {
    // Update the color in each of the palettes

    for (size_t i = 0; i < _paletteStack.size(); ++i)
    {
      SiiPalette *pal = _paletteStack[i];
      if (pal == copy_palette)
	continue;
      pal->setBoundaryColor(copy_palette->getBoundaryColor());
      pal->setGridColor(copy_palette->getGridColor());
    }

    // Signal that the window information has changed

    for (int jj = 0; jj < MAX_FRAMES; jj++)
      solo_return_wwptr(jj)->parameter.changed = 1;
  }
  
  void broadcastBackgroundColor(const SiiPalette *copy_palette)
  {
    // Update the color in each of the palettes

    for (size_t i = 0; i < _paletteStack.size(); ++i)
    {
      SiiPalette *pal = _paletteStack[i];
      if (pal == copy_palette)
	continue;
      pal->setBackgroundColor(copy_palette->getBackgroundColor());
      pal->setGridColor(copy_palette->getGridColor());
    }

    // Signal that the window information has changed

    for (int jj = 0; jj < MAX_FRAMES; jj++)
      solo_return_wwptr(jj)->parameter.changed = 1;
  }
  
  void broadcastExceededColor(const SiiPalette *copy_palette)
  {
    // Update the color in each of the palettes

    for (size_t i = 0; i < _paletteStack.size(); ++i)
    {
      SiiPalette *pal = _paletteStack[i];
      if (pal == copy_palette)
	continue;
      pal->setExceededColor(copy_palette->getExceededColor());
      pal->setGridColor(copy_palette->getGridColor());
    }

    // Signal that the window information has changed

    for (int jj = 0; jj < MAX_FRAMES; jj++)
      solo_return_wwptr(jj)->parameter.changed = 1;
  }
  
  void broadcastMissingDataColor(const SiiPalette *copy_palette)
  {
    // Update the color in each of the palettes

    for (size_t i = 0; i < _paletteStack.size(); ++i)
    {
      SiiPalette *pal = _paletteStack[i];
      if (pal == copy_palette)
	continue;
      pal->setMissingDataColor(copy_palette->getMissingDataColor());
      pal->setGridColor(copy_palette->getGridColor());
    }

    // Signal that the window information has changed

    for (int jj = 0; jj < MAX_FRAMES; jj++)
      solo_return_wwptr(jj)->parameter.changed = 1;
  }
  
  void broadcastEmphasisColor(const SiiPalette *copy_palette)
  {
    // Update the color in each of the palettes

    for (size_t i = 0; i < _paletteStack.size(); ++i)
    {
      SiiPalette *pal = _paletteStack[i];
      if (pal == copy_palette)
	continue;
      pal->setEmphasisColor(copy_palette->getEmphasisColor());
      pal->setGridColor(copy_palette->getGridColor());
    }

    // Signal that the window information has changed

    for (int jj = 0; jj < MAX_FRAMES; jj++)
      solo_return_wwptr(jj)->parameter.changed = 1;
  }
  
  void broadcastAnnotationColor(const SiiPalette *copy_palette)
  {
    // Update the color in each of the palettes

    for (size_t i = 0; i < _paletteStack.size(); ++i)
    {
      SiiPalette *pal = _paletteStack[i];
      if (pal == copy_palette)
	continue;
      pal->setAnnotationColor(copy_palette->getAnnotationColor());
    }

    // Signal that the window information has changed

    for (int jj = 0; jj < MAX_FRAMES; jj++)
      solo_return_wwptr(jj)->parameter.changed = 1;
  }
 */ 
  SiiPalette *setPalette(const std::string &name);
  

  /**
   * @brief Find the first palette in our stack that includes the given
   *        parameter in it's list of parameters.
   *
   * @param[in] param_name   The parameter name.
   * @param[in] remove          Flag indicating whether to remove the
   *                              palette from the stack upon return.
   *                              If set to true, the calling method is
   *                              responsible for deleting this pointer
   *                              when it is no longer needed.
   *
   * @return Returns a pointer to the palette on success, 0 on failure.
   */

  SiiPalette *_paletteForParam(const std::string &param_name,
                               const bool remove = false)
  {
    // Loop through the palettes, looking for one with the indicated parameter

    for (std::vector< SiiPalette* >::iterator pal_iter = _paletteStack.begin();
         pal_iter != _paletteStack.end(); ++pal_iter)
    {
      // Get a pointer to the palette object

      SiiPalette *pal = *pal_iter;

      // If the parameter is in the palette, return this item

      if (pal->isParameterIncluded(param_name))
      {
        if (remove)
          _paletteStack.erase(pal_iter);
        return pal;
      }

    }

    return 0;
  }


protected:

  ///////////////////////
  // Protected members //
  ///////////////////////

  /**
   * @brief Singleton instance pointer.
   */

  static PaletteManager *_instance;
  
  /**
   * @brief The palette stack.
   */

  std::vector< SiiPalette* > _paletteStack;
  
  /**
   * @brief The list of palette names.
   */

  std::vector< std::string > _paramPaletteNames;
  

  ///////////////////////
  // Protected methods //
  ///////////////////////

  /**
   * @brief Constructor.  This is private because this is a singleton object.
   */

  PaletteManager();

  void _defaultPalettesList();
  
  /**
   * @brief Find the first palette in our stack that includes the given
   *        parameter in it's list of parameters.
   *
   * @param[in] param_name   The parameter name.
   * @param[in] remove          Flag indicating whether to remove the
   *                              palette from the stack upon return.
   *                              If set to true, the calling method is
   *                              responsible for deleting this pointer
   *                              when it is no longer needed.
   *
   * @return Returns a pointer to the palette on success, 0 on failure.
   */
/* made this public ...
  SiiPalette *_paletteForParam(const std::string &param_name,
			       const bool remove = false)
  {
    // Loop through the palettes, looking for one with the indicated parameter

    for (std::vector< SiiPalette* >::iterator pal_iter = _paletteStack.begin();
	 pal_iter != _paletteStack.end(); ++pal_iter)
    {
      // Get a pointer to the palette object

      SiiPalette *pal = *pal_iter;

      // If the parameter is in the palette, return this item

      if (pal->isParameterIncluded(param_name))
      {
	if (remove)
	  _paletteStack.erase(pal_iter);
	return pal;
      }
      
    }

    return 0;
  }
*/

  void _removeUsualParam(SiiPalette *palx, const std::string &name);
  
};

#endif
