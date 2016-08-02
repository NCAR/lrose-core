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
 ///////////////////////////////////////////////////////////////
 // USAirmet.cc
 //
 // SigAirMet2Spdb object -- utilities functions for US AIRMETS
 //
 // Deirdre Garvey, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
 //
 // Spring 2004
 //
 ///////////////////////////////////////////////////////////////

 #include <string>
 #include <vector>
 #include <cerrno>
 #include <toolsa/toolsa_macros.h>
 #include <toolsa/globals.h>
 #include <toolsa/pjg_flat.h>
 #include <toolsa/DateTime.hh>
 #include <Spdb/DsSpdb.hh>
 #include "SigAirMet2Spdb.hh"

////////////////////////////////////////////////////
// check if the type is a US AIRMET *HEADER* type
// the header types are different from the actual
// AIRMET types
//
// Return true if a valid HEADER type, false otherwise
//

bool SigAirMet2Spdb::_isValidUSAirmetHeaderType(const string &type)

{
  // Set defaults

  bool found=false;

   // Check against the valid types

   if (type == "TANGO" ||
       type == "SIERRA" ||
       type == "ZULU") {
     found=true;
   }

   return found;

}

////////////////////////////////////////////////////
// check if the type is a US AIRMET type, not a 
// header type
//
// Return true if a valid type, false otherwise
//

bool SigAirMet2Spdb::_isValidUSAirmetType(const string &type)

{
  // Set defaults

   bool found=false;

   // Check against the valid types

   if (type == "ICE" ||
       type == "IFR" ||
       type == "TURB" ||
       type == "MTN OBSCN") {
     found=true;
   }

   return found;

}

////////////////////////////////////////////////////
// check if the type is a US AIRMET type valid for the
// current header type
//
// Return true if a valid type for the header type,
// false otherwise
//

bool SigAirMet2Spdb::_isValidUSAirmetTypeForHeader(const string &header_type,
						   const string &type)
  
{
  // Set defaults

   bool found=false;

   // Do we even have a valid header and type?

   if (!_isValidUSAirmetHeaderType(header_type)) {
     return found;
   }

   if (!_isValidUSAirmetType(type)) {
     return found;
   }

   // Check the header against the valid types

   if ((header_type == "TANGO") && (type == "TURB")) {
     found=true;
   }

   if ((header_type == "SIERRA") &&
       (type == "IFR" || type == "MTN OBSCN")) {
     found=true;
   }

   if ((header_type == "ZULU") && (type == "ICE")) {
     found=true;
   }

  return found;

}

////////////////////////////////////////////////////
// check if the type is a US AIRMET valid type
//
// Return true if a valid type, false otherwise
// Return the type string in type
//

bool SigAirMet2Spdb::_findUSAirmetType(const string &tok, 
				       const string &nextTok,
				       bool &is_header,
				       string &header_type,
				       string &type)

{
  // Set defaults

   bool found=false;
   is_header=false;
   header_type="NULL";
   type="NULL";

  // Have to check for trailing ... stuff

   string useTok=tok;
   if ((tok.find(".", 0) != string::npos)) {
     _chopSpacer2End(".", tok, useTok);
   }

   string useNextTok=nextTok;
   if ((nextTok.find(".", 0) != string::npos)) {
     _chopSpacer2End(".", nextTok, useNextTok);
   }
 
   // First check against the valid header types

   found=_isValidUSAirmetHeaderType(useTok);
   
   if (found) {
     header_type=useTok;
     is_header=true;
   }

   // If not found, see if it is a non-header type

   if (!found) {

     found=_isValidUSAirmetType(useTok);
   
     if (found) {
       type=useTok;
     }  else {

       // Try combining the 2 tokens
       // This is because of MTN OBSCN

       type=useTok + " " + useNextTok;
       found=_isValidUSAirmetType(type);
       if (!found) {
	 type="NULL";
       }
     }
   }

  return found;
}


////////////////////////////////////////////////////
// check if a US AIRMET UPDATE (as opposed to original)
//
// Return true if an update, false otherwise
// Return the update number (if found) or 0 otherwise
//

bool SigAirMet2Spdb::_getUSAirmetUpdate(const vector <string> toks,
					const size_t useStartIdx,
					string &updateNum)

{
  // Set defaults

  bool isUpdate=false;
  updateNum="0";

  bool foundUpdateNum=false;
  for (size_t ii=useStartIdx; ii<toks.size()-1; ii++) {
     string tok=toks[ii];
     string nextTok=toks[ii+1];

     if (tok == "UPDT") {
       isUpdate=true;
       if (_allDigits(nextTok)) {
	 updateNum=nextTok;
	 foundUpdateNum=true;
       }
       break;
     }
  }

  if (isUpdate && !foundUpdateNum) {
    isUpdate=false;
    if (_params.debug || _params.print_decode_problems_to_stderr) {
      cerr << "Found update token but no update number, set to not_an_update" << endl;
    }
  }
  return isUpdate;
}

////////////////////////////////////////////////////
// find the string of states after the type so can use as qualifier
//
// Return true if found, false otherwise
// Return the qualifier string
//

bool SigAirMet2Spdb::_findUSAirmetQualifier(const vector <string> toks,
					    const size_t useStartIdx,
					    const string type,
					    string &qualifier)

{
  // Set defaults

   bool found=false;
   qualifier="NULL";

   // Want the stuff after the type and before the word FROM
   
   bool foundType=false;
   bool foundFrom=false;
   size_t typeIdx = 0;
   size_t fromIdx = 0;
   for (size_t ii=useStartIdx; ii<toks.size()-1; ii++) {
     string tok=toks[ii];
     string nextTok=toks[ii+1];

     // Have to check for trailing ... stuff for the type

     string useTok=tok;
     if ((tok.find(".", 0) != string::npos)) {
       _chopSpacer2End(".", tok, useTok);
     }

     if (useTok == type) {
       foundType=true;
       typeIdx=ii;
     } else {

       // Try combining the 2 tokens
       // This is because of MTN OBSCN

       string useNextTok=nextTok;
       if ((nextTok.find(".", 0) != string::npos)) {
	 _chopSpacer2End(".", nextTok, useNextTok);
       }

       string tryType=useTok + " " + useNextTok;
       if (tryType == type) {
	 foundType=true;
	 typeIdx=ii+1;
       }
     }

     if (useTok == "FROM") {
       foundFrom=true;
       fromIdx=ii;
       break;
     }
   }

   // Bail if we did not find the type or FROM

   if (!foundType || !foundFrom) {
     return found;
   }

   // Set the qualifier to be the remaining string
   
   for (size_t ii=typeIdx; ii<fromIdx; ii++) {
     string tok=toks[ii];

     if (ii == typeIdx) {

       // Strip the leading type... if it exists
       string useTok=tok;
       while (size_t pos=useTok.find(".", 0) != string::npos) {
	 useTok=useTok.erase(0, pos);
       }
       qualifier=useTok;
     } else {

       // Strip off any trailing ...
       string useTok=tok;
       if ((tok.find(".", 0) != string::npos)) {
	 _chopSpacer2End(".", tok, useTok);
       }
       qualifier=qualifier + " " + useTok;
     }

     // Set the used[] array

     _used[ii]=true;
   }

   found=true;

   
   if (_params.debug >= Params::DEBUG_VERBOSE) {
     cerr << "Qualifier: " << qualifier << endl;
   }

   return found;
}

////////////////////////////////////////////////////
// Get the sub AIRMET message number to use in the ID
//
// Return true if can set, false otherwise
//

bool SigAirMet2Spdb::_getUSAirmetSubMessageId(string &id)

{
  // Set defaults

  id="NULL";
  if (_usAirmetHdrInfo.sub_msg_counter == 1) {
    id="a";
  } else if (_usAirmetHdrInfo.sub_msg_counter == 2) {
    id="b";
  } else if (_usAirmetHdrInfo.sub_msg_counter == 3) {
    id="c";
  } else if (_usAirmetHdrInfo.sub_msg_counter == 4) {
    id="d";
  } else if (_usAirmetHdrInfo.sub_msg_counter == 5) {
    id="e";
  } else if (_usAirmetHdrInfo.sub_msg_counter == 6) {
    id="f";
  } else if (_usAirmetHdrInfo.sub_msg_counter == 7) {
    id="g";
  } else if (_usAirmetHdrInfo.sub_msg_counter == 8) {
    id="h";
  } else if (_usAirmetHdrInfo.sub_msg_counter == 9) {
    id="i";
  } else if (_usAirmetHdrInfo.sub_msg_counter == 10) {
    id="j";
  }

  if (id == "NULL") {
    return false;
  } else {
    return true;
  }
}


////////////////////////////////////////////////////
// assemble the US AIRMET ID
//
// return the assembled id
//

void SigAirMet2Spdb::_assembleUSAirmetId(const string &updateNum,
					 const string &subMsgId,
					 const string &msgNum,
					 string &id)
{

  // The ID is:      update number (from header if set)
  //                 header message number
  //                 sub message number

  id=updateNum + "_" + _usAirmetHdrInfo.msg_num + "_" + subMsgId;
}

////////////////////////////////////////////////////
// get the different portions of the US AIRMET ID
//
// Return true if able to get, false otherwise
//

bool SigAirMet2Spdb::_disassembleUSAirmetId(const string &id,
					    string &updateNum,
					    string &subMsgId,
					    string &msgNum)

{

  // The ID is:      update number (from header if set)
  //                 header message number
  //                 sub message number

  string tmp=id;
  vector <string> toks;
  _tokenize(tmp, "_", toks);

  if (toks.size() < 3) {
     return false;
   }
   
  updateNum=toks[0];
  msgNum=toks[1];
  subMsgId=toks[2];

  return true;
}

////////////////////////////////////////////////
// find the US AIRMET ID to cancel.
// If an update, find the previous AIRMET so we can cancel it.
//
// Sets _doCancel, _cancelId and _cancelGroup

void SigAirMet2Spdb::_USAirmetCheckCancel()
  
{
  _doCancel = false;
  
  string id = _decoded.getId();
  string qual = _decoded.getQualifier();
  _mergeIdAndQualifier(id, qual);

  string funcName="_findUSAirmetUpdate2Cancel";
  
  // Disassemble the ID
  
  string updateNumStr, subMsgId, msgNum;
  if (!_disassembleUSAirmetId(id, updateNumStr, subMsgId, msgNum)) {
    return;
  }
  
  // If an update, find the previous update ID number
  
  string prevUpdateNum;
  if (updateNumStr != "0") {
    
    // Convert the update number to an integer
    
    const char *updateNum = updateNumStr.c_str();
    int updateNumInt;
    if (sscanf(updateNum, "%d", &updateNumInt) != 1) {
      return;
    }
    if (updateNumInt <= 0) {
      return;
      cerr << "ERROR: " << funcName
           << ": Update number is: " << updateNumInt << endl;
    }     

    // Decrement the number
    
    updateNumInt = updateNumInt - 1;
    
    // Convert to a string
    
    char arr[4];
    sprintf(arr, "%d", updateNumInt);
    prevUpdateNum=*arr;
  }

  // At a minimum we need to find a match with the type,
  // previous update number.
  // The message number will not match.

  string wxType=_decoded.getWx();
  string partialId=wxType + prevUpdateNum;
  string qualifier=_decoded.getQualifier();
  string source=_decoded.getSource();
  
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << funcName << ": Want to find match for partial id: "
         << partialId << ", and qualifier: " << qualifier << endl;
  }
  
  // grab the list of currently valid AIRMETs to see if we can find the one
  // to cancel
  
  DsSpdb in;
  int success;
  
  success=in.getValid(_params.cancel_input_url, _decoded.getIssueTime());
  if (success == -1) {
    if (_params.debug) {
      cerr << "ERROR - SigAirMet2Spdb::" << funcName << endl;
      cerr << "  Cannot retrieve AIRMET to cancel"
           << _decoded.getSource() << " " << id << endl;
      cerr << "  " << in.getErrStr() << endl;
    }
    return;
  }

  // Skip any already cancelled. Skip any with
  // weather type that does not match current type.

  const vector<Spdb::chunk_t> &chunks = in.getChunks();
  vector<SigAirMet *> candidates;
  for (size_t ii = 0; ii < chunks.size(); ii++) {
    SigAirMet *sa = new SigAirMet();
    if ((sa->disassemble(chunks[ii].data, chunks[ii].len) == 0) &&
	(!sa->getCancelFlag()) &&
	(sa->getWx() == wxType)) {
      candidates.push_back(sa);
      if (_params.debug >= Params::DEBUG_VERBOSE) {
	cerr << funcName << ": Found a valid airmet with type: "
             << wxType << endl;
      }
    } else {
      delete sa;
    }
  }

  // now look for airmets that match the previous update number and
  // the source and qualifier. Ignore the subMsgNum and msgNum parts
  // of the ID, these were needed to ensure uniqueness, not for cancel
  // matches

  bool found=false;
  size_t n_found=0;
  
  for (int ii = (int) candidates.size() - 1; ii >= 0; ii--){
    
    SigAirMet *sa = candidates[ii];
    string saType, saUpdateNum, saSubMsgNum, saMsgNum;
    string saId=sa->getId();
    string saSource=sa->getSource();
    string saQualifier=sa->getQualifier();
    
    // Look for a match
    
    bool foundSourceAndUpdate=false;
    bool foundQualifier=false;
    if (_disassembleUSAirmetId(saId, saUpdateNum, saSubMsgNum, saMsgNum)) {
      if ((prevUpdateNum == saUpdateNum) && (source == saSource)) {
	foundSourceAndUpdate=true;
	if (_params.debug >= Params::DEBUG_VERBOSE) {
	  cerr << funcName << ": Found a match for prevUpdateNum and source"
               << saId << ", source: " << source << endl;
	}
      }
      if (qualifier == saQualifier) {
	foundQualifier=true;
	if (_params.debug >= Params::DEBUG_VERBOSE) {
	  cerr << funcName << ": Found a match: for qualifier:"
               << qualifier << endl;
	}
      }
      if (foundQualifier && foundSourceAndUpdate) {
        found=true;
        _cancelId = saId;
        n_found++;
        if (_params.debug >= Params::DEBUG_VERBOSE) {
          cerr << funcName
               << ": Found a match for source, qual, prev update num: "
               << source << ", " << qualifier << ", " << saId << endl;
        }
      }
    }
  } // ii

  // free up memory
  
  for (int ii = 0; ii < (int) candidates.size(); ii++){
    delete candidates[ii];
  }

  // Error prints
  
  if (n_found > 1) {
    cerr << "ERROR: " << funcName
         << ": Found more than one match in search: " << n_found << endl;
    found=false;
  }

  if (!found) {
    cerr << "WARNING: " << funcName
         << ": Could not find AIRMET to cancel" << endl;
    return;
  }

  _doCancel = true;
  _cancelGroup = AIRMET_GROUP;

}


////////////////////////////////////////////////////
// If a US AIRMET have to set the qualifier and ID differently.
//
// Sets the qualifier to the location.
// Sets the ID to the type and a number.
// Sets the Weather to the type
//
// Return true if a US AIRMET, false otherwise
//

void SigAirMet2Spdb::_setUSAirmetID(const vector <string> toks,
				    const size_t useStartIdx)

{

  // initialize

  _isUSAirmet = false;
  _isUSAirmetHeader = false;
  _usAirmetHdrInfo.msg_num = "";
  _usAirmetHdrInfo.header_type = "";
  _usAirmetHdrInfo.is_update = false;
  _usAirmetHdrInfo.update_num = "";
  _usAirmetHdrInfo.start_time = 0;
  _usAirmetHdrInfo.end_time = 0;
  _usAirmetHdrInfo.sub_msg_counter = 0;

  // Is this an AIRMET

  if (_decoded.getGroup() != AIRMET_GROUP) {
    return;
  }

  // Need to get several tokens beyond the start token number

  if (useStartIdx+2 >= toks.size()) {
    return;
  }

  size_t tokN= useStartIdx + 1;
  string tokStr1 = toks[tokN];

  tokN= useStartIdx + 2;
  string tokStr2 = toks[tokN];

  // Is this a US AIRMET type?

  string type, header_type;
  bool is_header;
  bool isUSAirmet =
    _findUSAirmetType(tokStr1, tokStr2, is_header, header_type, type);

  if (!isUSAirmet) {
    return;
  }

  // Set the global flag for US AIRMET header
  // Update the header struct so can re-use info for later AIRMETs if needed

  if (is_header) {
    _isUSAirmetHeader=true;
    _usAirmetHdrInfo.header_type=header_type;
    _usAirmetHdrInfo.start_time=0;
    _usAirmetHdrInfo.end_time=0;
    _usAirmetHdrInfo.is_update=false;
    _usAirmetHdrInfo.msg_num="UKNOWN";
    _usAirmetHdrInfo.sub_msg_counter=0;
  } else {
    _isUSAirmetHeader=false;
    _usAirmetHdrInfo.sub_msg_counter++;
  }

  if (_params.debug >= Params::DEBUG_VERBOSE ||
      _params.print_decode_problems_to_stderr) {
    cerr << "   is a US AIRMET, ";
    if (is_header) {
      cerr << "is a header, type: " << header_type;
    } else {
      cerr << "is a non_header, type: " << type;
    }
    cerr << endl;
  }

  // Is this an update or not?

  string updateNum;
  bool is_update=_getUSAirmetUpdate(toks, useStartIdx, updateNum);

  if ((is_update) && (is_header)) {
    _usAirmetHdrInfo.is_update=true;
    _usAirmetHdrInfo.update_num=updateNum;
  }

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "   this msg is an update: " << is_update << ", updateNum: " << updateNum << endl;
    if (!is_header && _usAirmetHdrInfo.is_update) {
      cerr << "   header is update num: " << _usAirmetHdrInfo.update_num << ", so will use that" << endl;
    }
  }

  // If a header, can we get the very first token (expected to be the message number)
  // This is useful for distinguishing messages from one another

  if (is_header) {
   string msg_num=_msgToks[0];
    _usAirmetHdrInfo.msg_num=msg_num;
  } 

  // If a header, set the ID and return

  if (is_header) {
    string idStr=header_type + updateNum;
    _decoded.setId(idStr);
    if (_params.debug >= Params::DEBUG_VERBOSE) {
      cerr << "ID: " << idStr << endl;
    }
    _isUSAirmet = true;
  }

  // Set the Wx type to the SIGMET type

  if (!is_header) {
    _decoded.setWx(type);
    if (_params.print_decode_problems_to_stderr) {
      cerr << "Weather: " << type << endl;
    }
  }
  
  // Get the sub message id number

  string subMsgId;
  if (!_getUSAirmetSubMessageId(subMsgId)) {
    subMsgId="z";
  }

  // Not a header, we still have to find and set the ID

  string idStr;
  string useUpdateNum=updateNum;
  if (!is_update) {
    useUpdateNum=_usAirmetHdrInfo.update_num;
  }

  if (useUpdateNum.size() <= 0) {
    useUpdateNum="0";
  }

  _assembleUSAirmetId(useUpdateNum, subMsgId, _usAirmetHdrInfo.msg_num, idStr);
  _decoded.setId(idStr);

  // Set the qualifier to the string after the type (e.g., the states)

  string qualifier;
  if (_findUSAirmetQualifier(toks, useStartIdx, type, qualifier)) {
    _decoded.setQualifier(qualifier);
    string id = _decoded.getId();
    if (_mergeIdAndQualifier(id, qualifier) == 0) {
      _decoded.setId(id);
    }
  }

  _isUSAirmet = true;
  return;
  
}
