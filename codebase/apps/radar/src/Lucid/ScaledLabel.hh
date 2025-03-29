// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
// ** Copyright UCAR (c) 1990 - 2016                                         
// ** University Corporation for Atmospheric Research (UCAR)                 
// ** National Center for Atmospheric Research (NCAR)                        
// ** Boulder, Colorado, USA                                                 
// ** BSD licence applies - redistribution and use in source and binary      
// ** forms, with or without modification, are permitted provided that       
// ** the following conditions are met:                                      
// ** 1) If the software is modified to produce derivative works,            
// ** such modified software should be clearly marked, so as not             
// ** to confuse it with the version available from UCAR.                    
// ** 2) Redistributions of source code must retain the above copyright      
// ** notice, this list of conditions and the following disclaimer.          
// ** 3) Redistributions in binary form must reproduce the above copyright   
// ** notice, this list of conditions and the following disclaimer in the    
// ** documentation and/or other materials provided with the distribution.   
// ** 4) Neither the name of UCAR nor the names of its contributors,         
// ** if any, may be used to endorse or promote products derived from        
// ** this software without specific prior written permission.               
// ** DISCLAIMER: THIS SOFTWARE IS PROVIDED "AS IS" AND WITHOUT ANY EXPRESS  
// ** OR IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED      
// ** WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.    
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
// ScaledLabel.h: interface for the ScaledLabel class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(SCALEDLABEL_HH)
#define SCALEDLABEL_HH

#include <string>
#include <sstream>
#include <map>

// Create a label that is scaled in engineering units. At
// construction, ScaledLabel is instructed on the type of
// conversion it will perform. Subsequent calls to scale(double)
// will return a string, scaled accordingly.

class ScaledLabel  
{
public:
  enum ScalingType {
    /// Scale for distance, in engineering units, 
    /// with the appropriate units designation appended.
    DistanceEng
  };
  
  // Constructor. 
  /// The type of scaling to apply.
  ScaledLabel(ScalingType t);
  
  //Destructor
  virtual ~ScaledLabel();
  
  /// Return a string containing the scaled representation.
  /// Pass in the value to be scaled.
  std::string scale(double scale);
  
protected:
  /// The type of scaling we are performing.
  ScalingType m_scalingType;
  
  /// A stringstream used to format numbers
  std::ostringstream m_stringStr;

};

#endif // !defined(SCALEDLABEL_H)

