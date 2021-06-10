#include <iostream>
#include <fstream>
#include <sstream>
#include <iterator>
#include <regex>
#include <string>
#include <algorithm>


#include "SoloScriptTranslator.hh"
 
using namespace std;

SoloScriptTranslator::SoloScriptTranslator() {}
SoloScriptTranslator::~SoloScriptTranslator() {}

void SoloScriptTranslator::format_it(string &command) {
	std::transform(command.begin(), command.end(), command.begin(), ::toupper);
	replace(command.begin(), command.end(), '-', '_');
}


/*
void SoloScriptTranslator::construct_new_field(string &field) {

    string next;
	reference_as_assignment(field, next);
	field = next;
}
*/

void SoloScriptTranslator::reference_as_assignment(string &field) {
	string next_assignment;

    // is this a raw field?
    if (field_map_raw.find(field) != field_map_raw.end()) {
        next_assignment = field;
        int x = field_map_raw[field];
        x += 1;
        next_assignment.append("_");
        next_assignment.append(to_string(x));
        field_map_raw[field] = x;
    } else if (field_map_derived.find(field) != field_map_derived.end()) {
        next_assignment = field;
        int x = field_map_derived[field];
        x += 1;
        next_assignment.append("_");
        next_assignment.append(to_string(x));
        field_map_derived[field] = x;
    } else { 
    	// this is a new field, add it to the map of derived fields
        field_map_derived[field] = 1;
        next_assignment = field;
    }
    cout << "reference_as_assignment: field " << field << " next_assignment = " << next_assignment << endl;
    field = next_assignment;
}

void SoloScriptTranslator::reference_as_source(string &field) {
    string next_source;

	// is this a raw field? 
    if (field_map_raw.find(field) != field_map_raw.end()) {
        next_source = field;
        int x = field_map_raw[field];
        if (x > 1) {
           next_source.append("_");
           next_source.append(to_string(x));
        } else {
        	//next_source.append("_V");
        }
        //next_source.append("_");
        //next_source.append(to_string(x));
        //field_map_raw[field] = x;
    } else if (field_map_derived.find(field) != field_map_derived.end()) {  
        // is this a derived field? 
        next_source = field;
        int x = field_map_derived[field];
        if (x > 1) {
           next_source.append("_");
           next_source.append(to_string(x));
        } else {
           ; // do nothing to the name
        } 

    } else {    
    	// have not seen this field before, it must be a raw field
    	// because derived fields can only be added as assignment      
    	field_map_raw[field] = 1;
        next_source = field;
        //next_source.append("_V");
    }
    cout << "reference_as_source: field " << field << " next_source = " << next_source << endl;
    field = next_source;
}

/*
// only distinction is here  and only for the first time the field is referenced
// if the field is a raw data field, then append _V for pass by reference
// otherwise, the field is derived, then do not append _V
void SoloScriptTranslator::format_field_by_reference(string &field) {
	std::transform(field.begin(), field.end(), field.begin(), ::toupper);
	//field.append("_V");
    string last;
    if (field_map.find(field) != field_map.end()) {
    	cout << "format_field_by_reference: " << field << " found in map" << endl;
        last = field;
        int x = field_map[field];
        if (x > 1) {
           last.append("_");
           last.append(to_string(x));
        } 
    } else {
    	cout << "format_field_by_reference: " << field << " NOT found in map" << endl;
        last = field;
        last.append("_V");
        field_map[field] = 1;
    }
	field = last;	
}
*/

bool SoloScriptTranslator::process_solo_comment(string line, std::iostream& javascript) {
	bool recognized = false;
    // ! solo comment
    if (line.length() < 1) {
    	javascript << endl;
    	recognized = true;
    } else if (line[0] == '!') {
	    string the_rest = line.substr(1, line.length() - 1);
	    cout << "// " << the_rest << endl;
	    javascript << "// " << the_rest << endl;
	    recognized = true;   
	} else {
	    std::cout << "regex_match returned false\n";
	} 
	return recognized;
}

bool SoloScriptTranslator::process_from_to(string line, std::iostream& javascript) {
	bool recognized = false;
		        // from x to y 
	    const std::regex pieces_regex("(remove-ring) in ([_[:alnum:]]+) from[\\s]+([-\\.[:digit:]]+) to ([-\\.[:digit:]]+) km\\.[\\s]*"); // ("copy[:space:]+([A-Z]+)[:space:]+to[:space:]+([A-Z]+)");
	    std::smatch pieces_match;
	        string mytest2 = "remove-ring in VG from 34. to -70.";
	        if (std::regex_match(line, pieces_match, pieces_regex)) {
	            //std::cout << line << '\n';
	            for (size_t i = 0; i < pieces_match.size(); ++i) {
	                std::ssub_match sub_match = pieces_match[i];
	                std::string piece = sub_match.str();
	                std::cout << "  submatch " << i << ": " << piece << '\n';
	            }   	      

	            string command = pieces_match[1];
	            format_it(command);
	            string field = pieces_match[2];
	            string new_field = field;  // make a copy of the destination field.
	            reference_as_source(field);	            
	            reference_as_assignment(new_field);
	            string from = pieces_match[3];
	            string to = pieces_match[4];
	            cout << new_field << " = " << command << " ( " << field << "," << from << "," << to << " )" << endl;
	            javascript << new_field << " = " << command << " ( " << field << "," << from << "," << to << " )" << endl;
	            recognized = true;
	        } else {
	        	std::cout << "regex_match returned false\n";
	        } 
	return recognized;
}

bool SoloScriptTranslator::process_action_in(string line, std::iostream& javascript) {
	bool recognized = false;
		        // from x to y 
	    const std::regex pieces_regex("(remove-only-surface|assert-bad-flags|copy-bad-flags) (in|from|of) ([_[:alnum:]]+)[\\s]*"); // ("copy[:space:]+([A-Z]+)[:space:]+to[:space:]+([A-Z]+)");
	    std::smatch pieces_match;
	        string mytest2 = "remove-ring in VG from 34. to -70.";
	        if (std::regex_match(line, pieces_match, pieces_regex)) {
	            //std::cout << line << '\n';
	            for (size_t i = 0; i < pieces_match.size(); ++i) {
	                std::ssub_match sub_match = pieces_match[i];
	                std::string piece = sub_match.str();
	                std::cout << "  submatch " << i << ": " << piece << '\n';
	            }   
	            string command = pieces_match[1];
	            format_it(command);
	            string field = pieces_match[3];
	            string new_field = field;  // make a copy of the destination field.
	            reference_as_source(field);	            
	            reference_as_assignment(new_field);
	            cout << new_field << " = " << command << " ( " << field << " )" << endl;
	            javascript << new_field << " = " << command << " ( " << field << " )" << endl;	            
	            recognized = true;
	        } else {
	        	std::cout << "regex_match returned false\n";
	        } 
	return recognized;
}

bool SoloScriptTranslator::process_when_above(string line, std::iostream& javascript) {
	bool recognized = false;
		        // from x to y 
	    // const std::regex pieces_regex("(and-bad-flags|set-bad-flags|or-bad-flags) when ([_[:alnum:]]+) (above|below) ([-\\.[:digit:]]+)[\\s]"); // ("copy[:space:]+([A-Z]+)[:space:]+to[:space:]+([A-Z]+)");

	    const std::regex pieces_regex("((and|set|or|xor)-bad-flags) when ([_[:alnum:]]+) (above|below) ([-\\.[:digit:]]+)[\\s]*"); // ("copy[:space:]+([A-Z]+)[:space:]+to[:space:]+([A-Z]+)");
	    std::smatch pieces_match;

	        if (std::regex_match(line, pieces_match, pieces_regex)) {
	            //std::cout << line << '\n';
	            for (size_t i = 0; i < pieces_match.size(); ++i) {
	                std::ssub_match sub_match = pieces_match[i];
	                std::string piece = sub_match.str();
	                std::cout << "  submatch " << i << ": " << piece << '\n';
	            }   
	            string command = pieces_match[1];
	            format_it(command);
	            string field = pieces_match[3];
	            string new_field = field;  // make a copy of the destination field.
	            reference_as_source(field);
	            reference_as_assignment(new_field);	            
	            string above_below = pieces_match[4];
	            format_it(above_below);
	            string value = pieces_match[5];
	            cout << new_field << " = " << command << "_" << above_below << " ( " << field << "," << value << " )" << endl;
	            javascript << new_field << " = " << command << "_" << above_below << " ( " << field << "," << value << " )" << endl;
	            recognized = true;
	        } else {
	        	std::cout << "regex_match returned false\n";
	        } 
	return recognized;
}

bool SoloScriptTranslator::process_on_var_below(string line, std::iostream& javascript) {
	bool recognized = false;
		        // from x to y 
	    const std::regex pieces_regex("(threshold) ([_[:alnum:]]+) on ([_[:alnum:]]+) (above|below) ([-\\.[:digit:]]+)[\\s]*"); // ("copy[:space:]+([A-Z]+)[:space:]+to[:space:]+([A-Z]+)");
	    std::smatch pieces_match;

	        if (std::regex_match(line, pieces_match, pieces_regex)) {
	            //std::cout << line << '\n';
	            for (size_t i = 0; i < pieces_match.size(); ++i) {
	                std::ssub_match sub_match = pieces_match[i];
	                std::string piece = sub_match.str();
	                std::cout << "  submatch " << i << ": " << piece << '\n';
	            }   
	            string command = pieces_match[1];
	            string field1 = pieces_match[2];
	            string new_field = field1;  // make a copy of the destination field.

	            reference_as_source(field1);
	            string field2 = pieces_match[3];
	            reference_as_source(field2);
	            string above_below = pieces_match[4];
	            string value = pieces_match[5];
	            reference_as_assignment(new_field);
	            command.append("_");
	            command.append(above_below);
	            format_it(command);
	            cout << new_field << " = " << command << " ( " << field1 << "," << field2 << "," << value << " )" << endl;
	            javascript << new_field << " = " << command << " ( " << field1 << "," << field2 << "," << value << " )" << endl;
	            recognized = true;
	        } else {
	        	std::cout << "regex_match returned false\n";
	        } 
	return recognized;
}

bool SoloScriptTranslator::process_copy(string line, std::iostream& javascript) {
	bool recognized = false;
	    //  [\\s] is blank space
	    const std::regex pieces_regex("copy[\\s]+([_[:alnum:]]+) to ([_[:alnum:]]+)[\\s]*"); // ("copy[:space:]+([A-Z]+)[:space:]+to[:space:]+([A-Z]+)");
	    std::smatch pieces_match;
	        string mytest = "copy VG to VP1";
	        if (std::regex_match(line, pieces_match, pieces_regex)) {
	            //std::cout << line << '\n';
	            for (size_t i = 0; i < pieces_match.size(); ++i) {
	                std::ssub_match sub_match = pieces_match[i];
	                std::string piece = sub_match.str();
	                std::cout << "  submatch " << i << ": " << piece << '\n';
	            }   
	            string field1 = pieces_match[2];
	            string new_field = field1;  // make a copy of the destination field.
	            reference_as_assignment(new_field);
	            // -- 
	            cout << new_field << " = " << pieces_match[1] << endl;
	            javascript << new_field << " = " << pieces_match[1] << endl;
	            recognized = true;
	        } else {
	        	std::cout << "regex_match returned false\n";
	        }  
	return recognized; 
}

bool SoloScriptTranslator::process_assignment(string line, std::iostream& javascript) {
	bool recognized = false;
	//  [\\s] is blank space
	const std::regex pieces_regex("([-[:alpha:]]+) is ([-\\.[:digit:]]+)[\\s]*(|degrees|gates)[\\s]*");

	std::smatch pieces_match;

	        if (std::regex_match(line, pieces_match, pieces_regex)) {

	            for (size_t i = 0; i < pieces_match.size(); ++i) {
	                std::ssub_match sub_match = pieces_match[i];
	                std::string piece = sub_match.str();
	                std::cout << "  submatch " << i << ": " << piece << '\n';
	            }   
	            string command = pieces_match[1];
	            format_it(command);
	            string value = pieces_match[2];
	            cout << command << " = " << value << endl;
	            javascript << command << " = " << value << endl;
	            recognized = true;
	        } else {
	        	std::cout << "regex_match returned false\n";
	        }  
	return recognized; 
}

bool SoloScriptTranslator::process_action_no_args(string line, std::iostream& javascript) {
	bool recognized = false;
		        // from x to y 
	    const std::regex pieces_regex("(clear-bad-flags)[\\s]*");

	    std::smatch pieces_match;

	        if (std::regex_match(line, pieces_match, pieces_regex)) {
	            //std::cout << line << '\n';
	            for (size_t i = 0; i < pieces_match.size(); ++i) {
	                std::ssub_match sub_match = pieces_match[i];
	                std::string piece = sub_match.str();
	                std::cout << "  submatch " << i << ": " << piece << '\n';
	            }   
	            string command = pieces_match[1];
	            format_it(command);
	            cout << command << " ( )" << endl;
	            javascript << command << " ( )" << endl;
	            recognized = true;
	        } else {
	        	std::cout << "regex_match returned false\n";
	        } 
	return recognized;
}

bool SoloScriptTranslator::process_action_extra_args(string line, std::iostream& javascript) {
		bool recognized = false;
		        // from x to y 
	    const std::regex pieces_regex("(despeckle) ([_[:alnum:]]+)[\\s]*");
	    std::smatch pieces_match;

	        if (std::regex_match(line, pieces_match, pieces_regex)) {
	            //std::cout << line << '\n';
	            for (size_t i = 0; i < pieces_match.size(); ++i) {
	                std::ssub_match sub_match = pieces_match[i];
	                std::string piece = sub_match.str();
	                std::cout << "  submatch " << i << ": " << piece << '\n';
	            }   
	            string command = pieces_match[1];
	            string field1 = pieces_match[2];
	            string new_field = field1;  // make a copy of the destination field.

	            reference_as_source(field1);
	            reference_as_assignment(new_field);	          
	            string extra_arg = "A_SPECKLE";
	            format_it(command);
	            cout << new_field << " = " << command << " ( " << field1 << "," << extra_arg << " )" << endl;
	            javascript << new_field << " = " << command << " ( " << field1 << "," << extra_arg << " )" << endl;
	            recognized = true;
	        } else {
	        	std::cout << "regex_match returned false\n";
	        } 
	return recognized;
}

bool SoloScriptTranslator::process_bool(string line, std::iostream& javascript, unfold_alg *BB_use) {
		bool recognized = false;
		        //
	    const std::regex pieces_regex("BB-use-(local-wind|ac-wind|first-good-gate)[\\s]*");
	    std::smatch pieces_match;

	        if (std::regex_match(line, pieces_match, pieces_regex)) {
	            //std::cout << line << '\n';
	            for (size_t i = 0; i < pieces_match.size(); ++i) {
	                std::ssub_match sub_match = pieces_match[i];
	                std::string piece = sub_match.str();
	                std::cout << "  submatch " << i << ": " << piece << '\n';
	            }   
	            string command = pieces_match[1];
                if (command.find("local-wind") != std::string::npos) {
                	*BB_use = LOCAL_WIND;
                	recognized = true;
                } else if (command.find("ac-wind") != std::string::npos) {
                	*BB_use = AC_WIND;
                	recognized = true;
                } else if (command.find("first-good-gate") != std::string::npos){
                	*BB_use = FIRST_GOOD_GATE;
                	recognized = true;
                } else {
                	recognized = false;
                }
                
	        } else {
	        	std::cout << "regex_match returned false\n";
	        } 
	return recognized;
}

bool SoloScriptTranslator::process_action_unfold(string line, std::iostream& javascript, unfold_alg BB_use) {
		bool recognized = false;
		        // BB-unfolding) of ([_[:alnum:]]+)[\\s]*");
	    const std::regex pieces_regex("(BB-unfolding) of ([_[:alnum:]]+)[\\s]*");
	    std::smatch pieces_match;

	        if (std::regex_match(line, pieces_match, pieces_regex)) {
	            //std::cout << line << '\n';
	            for (size_t i = 0; i < pieces_match.size(); ++i) {
	                std::ssub_match sub_match = pieces_match[i];
	                std::string piece = sub_match.str();
	                std::cout << "  submatch " << i << ": " << piece << '\n';
	            }   
	            string command = pieces_match[1];
	            string field1 = pieces_match[2];
	            string new_field = field1;  // make a copy of the destination field.

	            reference_as_source(field1);
	            reference_as_assignment(new_field);	            
	            string wind_args = "EW_WIND, NS_WIND";
	            // TODO: optional arg VERT_WIND for local wind
	            string extra_arg = "BB_MAX_POS_FOLDS, BB_MAX_NEG_FOLDS, BB_GATES_AVERAGED";
	            string fgg_arg = "FIRST_GOOD_GATE";
	            format_it(command);

	            switch(BB_use) {
	            	case AC_WIND:
	            		cout << new_field << " = " << command << " ( " << field1 << "," << extra_arg << " )" << endl;
	                    javascript << new_field <<" = " << command <<  "_AC_WIND" 
	                       << " ( " << field1 << "," << extra_arg << " )" << endl;
	                    recognized = true;
	            	break;
	            	case LOCAL_WIND:
	            		cout << new_field << " = " << command << " ( " << field1 << "," << extra_arg << " )" << endl;
	                    javascript << new_field << "  = " << command << "_LOCAL_WIND" 
	                       << " ( " << field1 << "," << extra_arg << ", " << wind_args << " )" << endl;
	                    recognized = true;
	            	break;
	            	case FIRST_GOOD_GATE:
	            		cout << new_field << " = " << command << " ( " << field1 << "," << extra_arg << " )" << endl;
	                    javascript << new_field << " = " << command << "_FIRST_GOOD_GATE" 
	                       << " ( " << field1 << "," << fgg_arg << ", " << extra_arg << " )" << endl;
	                    recognized = true;	            	
	            	break;
	            	default:
	            	    cout << "unrecognized unfolding algorithm" << endl; // not recognized error ...
	            }
	        } else {
	        	std::cout << "regex_match returned false\n";
	        } 
	return recognized;
}

//void translate(ifstream& solo_script, std::stringstream& javascript)
void SoloScriptTranslator::translate(ifstream& solo_script, std::iostream& javascript) {
   if (solo_script.is_open()) {
     std::string line;
     unfold_alg BB_use = UNKNOWN;
     while (getline(solo_script, line)) {

     	std::cout << "|" << line << "| \n";
     	bool recognized = false;

       if (!recognized) recognized = process_action_no_args(line, javascript);
       if (!recognized) recognized = process_action_extra_args(line, javascript);
       if (!recognized) recognized = process_copy(line, javascript);
       if (!recognized) recognized = process_from_to(line, javascript);
       if (!recognized) recognized = process_action_in(line, javascript);
       if (!recognized) recognized = process_when_above(line, javascript);
       if (!recognized) recognized = process_on_var_below(line, javascript);
       if (!recognized) recognized = process_assignment(line, javascript);
       if (!recognized) recognized = process_bool(line, javascript, &BB_use);
       if (!recognized) recognized = process_action_unfold(line, javascript, BB_use);
       if (!recognized) recognized = process_solo_comment(line, javascript);
       if (!recognized) {
       	  cout << "ERROR not recognized " << line << endl;
       	  javascript << "// ERROR, not recognized: " << line << endl;
       }
	   std::cout << std::endl;
	   //javascript << endl;
     }
    }
}

/*
int main()
{

   std::ifstream script_file;
   
   script_file.open("/Users/brenda/data/solo_scripts/sed_med_auto", std::ios::in);
   std::stringstream javascript;
   SoloScriptTranslator xlator;

   xlator.translate(script_file, javascript);

     script_file.close();

     cout << "\n\nHere is the translated script ...\n";
     cout << javascript.str() << endl;
   //}
   
}
*/
