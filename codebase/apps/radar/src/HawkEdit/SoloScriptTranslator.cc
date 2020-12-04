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

bool SoloScriptTranslator::process_from_to(string line, std::iostream& javascript) {
	bool recognized = false;
		        // from x to y 
	    const std::regex pieces_regex("(remove-ring) in ([_[:alnum:]]+) from[\\s]+([-\\.[:digit:]]+) to ([-\\.[:digit:]]+) km\\.[\\s]"); // ("copy[:space:]+([A-Z]+)[:space:]+to[:space:]+([A-Z]+)");
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
	            string from = pieces_match[3];
	            string to = pieces_match[4];
	            cout << command << " ( " << field << "," << from << "," << to << " )" << endl;
	            javascript << command << " ( " << field << "," << from << "," << to << " )" << endl;
	            recognized = true;
	        } else {
	        	std::cout << "regex_match returned false\n";
	        } 
	return recognized;
}

bool SoloScriptTranslator::process_action_in(string line, std::iostream& javascript) {
	bool recognized = false;
		        // from x to y 
	    const std::regex pieces_regex("(remove-only-surface|assert-bad-flags|copy-bad-flags|BB-unfolding) (in|from|of) ([_[:alnum:]]+)[\\s]"); // ("copy[:space:]+([A-Z]+)[:space:]+to[:space:]+([A-Z]+)");
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
	            cout << command << " ( " << field << " )" << endl;
	            javascript << command << " ( " << field << " )" << endl;	            
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

	    const std::regex pieces_regex("((and|set|or|xor)-bad-flags) when ([_[:alnum:]]+) (above|below) ([-\\.[:digit:]]+)[\\s]"); // ("copy[:space:]+([A-Z]+)[:space:]+to[:space:]+([A-Z]+)");
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
	            string above_below = pieces_match[4];
	            format_it(above_below);
	            string value = pieces_match[5];
	            cout << command << "_" << above_below << " ( " << field << "," << value << " )" << endl;
	            javascript << command << "_" << above_below << " ( " << field << "," << value << " )" << endl;
	            recognized = true;
	        } else {
	        	std::cout << "regex_match returned false\n";
	        } 
	return recognized;
}

bool SoloScriptTranslator::process_on_var_below(string line, std::iostream& javascript) {
	bool recognized = false;
		        // from x to y 
	    const std::regex pieces_regex("(threshold) ([_[:alnum:]]+) on ([_[:alnum:]]+) below ([-\\.[:digit:]]+)[\\s]"); // ("copy[:space:]+([A-Z]+)[:space:]+to[:space:]+([A-Z]+)");
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
	            string field1 = pieces_match[2];
	            string field2 = pieces_match[3];
	            string value = pieces_match[4];
	            cout << command << " ( " << field1 << "," << field2 << "," << value << " )" << endl;
	            javascript << command << " ( " << field1 << "," << field2 << "," << value << " )" << endl;
	            recognized = true;
	        } else {
	        	std::cout << "regex_match returned false\n";
	        } 
	return recognized;
}

bool SoloScriptTranslator::process_copy(string line, std::iostream& javascript) {
	bool recognized = false;
	    //  [\\s] is blank space
	    const std::regex pieces_regex("copy[\\s]+([_[:alnum:]]+) to ([_[:alnum:]]+)[\\s]+"); // ("copy[:space:]+([A-Z]+)[:space:]+to[:space:]+([A-Z]+)");
	    std::smatch pieces_match;
	        string mytest = "copy VG to VP1";
	        if (std::regex_match(line, pieces_match, pieces_regex)) {
	            //std::cout << line << '\n';
	            for (size_t i = 0; i < pieces_match.size(); ++i) {
	                std::ssub_match sub_match = pieces_match[i];
	                std::string piece = sub_match.str();
	                std::cout << "  submatch " << i << ": " << piece << '\n';
	            }   
	            cout << pieces_match[2] << " = " << pieces_match[1] << endl;
	            javascript << pieces_match[2] << " = " << pieces_match[1] << endl;
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
	    const std::regex pieces_regex("(clear-bad-flags)[\\s]");

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

//void translate(ifstream& solo_script, std::stringstream& javascript)
void SoloScriptTranslator::translate(ifstream& solo_script, std::iostream& javascript) {
   if (solo_script.is_open()) {
     std::string line;
     while (getline(solo_script, line)) {

     	std::cout << "|" << line << "| \n";
     	bool recognized = false;

       if (!recognized) recognized = process_action_no_args(line, javascript);
       if (!recognized) recognized = process_copy(line, javascript);
       if (!recognized) recognized = process_from_to(line, javascript);
       if (!recognized) recognized = process_action_in(line, javascript);
       if (!recognized) recognized = process_when_above(line, javascript);
       if (!recognized) recognized = process_on_var_below(line, javascript);
       if (!recognized) recognized = process_assignment(line, javascript);
       if (!recognized) {
       	  cout << "ERROR not recognized " << line << endl;
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
