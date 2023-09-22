#include <cstdio>
#include <cstdint>
#include <iostream>

//#include <sii_param_widgets.hh>

#include "SiiPalette.hh"
#include "PaletteManager.hh"

// Global variables

PaletteManager *PaletteManager::_instance = (PaletteManager *)0;


/*********************************************************************
 * Constructor
 */

PaletteManager::PaletteManager()
{
  // Create the default list

  _defaultPalettesList();
  updatePaletteNames();
  
  // Set the singleton instance pointer

  _instance = this;
}


/*********************************************************************
 * Destructor
 */

PaletteManager::~PaletteManager()
{
}


/*********************************************************************
 * Inst()
 */

PaletteManager *PaletteManager::getInstance()
{
  if (_instance == 0)
    new PaletteManager();
  
  return _instance;
}


/*********************************************************************
 * newPaletteForParam()
 */

void PaletteManager::newPaletteForParam(const std::string &palette_name,
					const std::string &param_name)
{
  // Look for the indicated palette in the palette list.  If the palette
  // exists, remove it from the list because we'll add it back in later.  If
  // it doesn't exist, create a new palette.

  SiiPalette *pal = seek(palette_name, true);
  
  if (pal == 0)
    pal = new SiiPalette(palette_name, "");

  // Get a pointer to the palette

  // Prepend the parameter to the usual parameters in this palette.  Remove 
  // the parameter from the usual parameters in all other palettes.

  pal->prependToUsualParameters(param_name);
  _removeUsualParam(pal, param_name);

  // Add the palette to the palette list

  _paletteStack.insert(_paletteStack.begin(), pal);
  updatePaletteNames();
}


/*********************************************************************
 * readPalette()
 */
/*
void PaletteManager::readPalette(char *buf_ptr, const bool gotta_swap)
{
  // Read the palette information

  SiiPalette palette;
  palette.read(buf_ptr, gotta_swap);
  
  // If we have no existing palette with this name, create a new one. Otherwise,
  // update the existing one with the incoming values.

  SiiPalette *pal = seek(palette.getPaletteName(), true);
  if (pal == 0)
  {
    // Allocate a new palette constructed from the one just read in
    pal = new SiiPalette(palette);
  }
  else
  {
    // Copy the palette just read into the existing palette
    *pal = palette; // copy the new palette information to the existing palette
  }
  
  // Put the new or modified palette at the beginning of the stack
  _paletteStack.insert(_paletteStack.begin(), pal);
  updatePaletteNames();
}
*/

/*********************************************************************
 * setPalette()
 */

SiiPalette *PaletteManager::setPalette(const std::string &name)
{
  SiiPalette *pal = _paletteForParam(name, true);

  if (pal == 0)
    pal = new SiiPalette(std::string("p_") + name,
			 string(name) + ",");
  else
    pal->prependToUsualParameters(name);

  _paletteStack.insert(_paletteStack.begin(), pal);
  updatePaletteNames();
   
  return pal;
}


/*********************************************************************
 * updatePaletteNames()
 */

void PaletteManager::updatePaletteNames()
{

  _paramPaletteNames.clear();
  
  // Create the names list from the palette

  for (size_t i = 0; i < _paletteStack.size(); ++i)
  {
    SiiPalette *pal = _paletteStack[i];
    _paramPaletteNames.push_back(pal->getPaletteName() + "  (" +
				 pal->getUsualParametersStr() + ")");
  }
}


/*********************************************************************
 * writePalettes()
 */

bool PaletteManager::writePalettes(FILE *stream) const
{
  // Save the palettes in reverse order so they will pop onto the
  // stack in the current order

  for (std::vector< SiiPalette* >::const_reverse_iterator pal_iter =
	 _paletteStack.rbegin();
       pal_iter != _paletteStack.rend(); ++pal_iter)
  {
    SiiPalette *pal = *pal_iter;
    if (!pal->write(stream))
      return false;
  }
  
  return true;
}


/**********************************************************************
 *              Private Member Functions                              *
 **********************************************************************/

/*********************************************************************
 * _defaultPalettesList()
 */

void PaletteManager::_defaultPalettesList()
{
  // NOTE: Do we need to reverse the order of these????

   SiiPalette * pal;

   pal = new SiiPalette("p_ahav", "AH,AV,", 0.0, 22.0, "carbone17");
   _paletteStack.push_back(pal);
   
   pal = new SiiPalette("p_chcv", "CH,CV,", 0.5, 0.1, "carbone17");
   _paletteStack.push_back(pal);
   
   pal = new SiiPalette("p_pdesc", "PD,WPD,HYDID,", 9.0, 1.0, "pd17");
   _paletteStack.push_back(pal);

   pal = new SiiPalette("p_raccum", "KAC,ZAC,DAC,HAC,NAC,GAC,",
			50.0, 10.0, "bluebrown10");
   pal->setNumColors(10);
   _paletteStack.push_back(pal);

   pal = new SiiPalette("p_rho", "RHOHV,RHO,RH,RX,", 0.5, 0.1, "carbone17");
   _paletteStack.push_back(pal);

   pal = new SiiPalette("p_kdp", "KDP,CKDP,NKDP,MKDP,DKD_DSD,",
			0.7, 0.12, "carbone17");
   _paletteStack.push_back(pal);

   pal = new SiiPalette("p_nt", "NT_DSD,", 2.0, 0.25, "carbone17");
   _paletteStack.push_back(pal);

   pal = new SiiPalette("p_res", "RES_DSD,", 5.0, 0.6, "carbone17");
   _paletteStack.push_back(pal);

   pal = new SiiPalette("p_d0", "D0_DSD,", 2.0, 0.25, "carbone17");
   _paletteStack.push_back(pal);

   pal = new SiiPalette("p_lam", "LAM_DSD,", 5.0, 0.6, "carbone17");
   _paletteStack.push_back(pal);

   pal = new SiiPalette("p_lwd", "LWC_DSD,", 0.8, 0.1, "carbone17");
   _paletteStack.push_back(pal);

   pal = new SiiPalette("p_mu", "MU_DSD,", 5.0, 0.6, "carbone17");
   _paletteStack.push_back(pal);

   pal = new SiiPalette("p_n0", "N0_DSD,", 4.0, 0.5, "carbone17");
   _paletteStack.push_back(pal);

   pal = new SiiPalette("p_phi", "PHIDP,PHI,PH,DP,NPHI,CPHI",
			70.0, 10.0, "carbone17");
   _paletteStack.push_back(pal);

   pal = new SiiPalette("p_zdr", "ZDR,ZD,DR,UZDR,", 4.0, 0.7, "carbone17");
   _paletteStack.push_back(pal);

   pal = new SiiPalette("p_ldr", "LDR,TLDR,ULDR,LVDR,LH,LV",
			-6.0, 4.0, "carbone17");
   _paletteStack.push_back(pal);

   pal = new SiiPalette("p_dBm", "DM,LM,XM,XL,DL,DX", -80.0, 5.0, "carbone17");
   _paletteStack.push_back(pal);

   pal = new SiiPalette("p_dBz", "DBZ,DZ,XZ,DB,Z,UDBZ,CDZ,DCZ,",
			15.0, 5.0, "carbone17");
   _paletteStack.push_back(pal);

   pal = new SiiPalette("p_spectral", "SR,SW,S1,S2", 8.0, 1.0, "carbone17");
   _paletteStack.push_back(pal);

   pal = new SiiPalette("p_ncp", "NCP,NC,", 0.5, 0.1, "carbone17");
   _paletteStack.push_back(pal);

   pal = new SiiPalette("p_vel", "VR,VF,VG,VH,VN,VE,VU,VT,V1,V2,VELOCITY,",
			0.0, 3.0, "carbone17");
   _paletteStack.push_back(pal);

   pal = new SiiPalette("p_rrate", "RR_DSD,RNX,RZD,RKD,", 0.0, 0.4, "rrate11");
   pal->setNumColors(11);
   _paletteStack.push_back(pal);

   pal = new SiiPalette("p_niq", "NIQ,", -60.0, 7.0, "carbone17");
   _paletteStack.push_back(pal);

   pal = new SiiPalette("p_aiq", "AIQ,", 0.0, 22.0, "carbone17");
   _paletteStack.push_back(pal);
}


/*********************************************************************
 * _removeUsualParam()
 */

void PaletteManager::_removeUsualParam(SiiPalette *palx,
				       const std::string &name)
{
  if (palx == 0)
    return;
  
  for (size_t i = 0; i < _paletteStack.size(); ++i)
  {
    SiiPalette *pal = _paletteStack[i];
    if (pal == palx)
      continue;

    pal->removeFromUsualParameters(name);
  }
}
