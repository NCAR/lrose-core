/*
 *  Foray Info application.
 *
 *  All this program does not is print out subversion info to 
 *  standard output.
 */

#include <iostream>
using namespace std;

#include <stdio.h>

#include <Fault.h>
#include <ForayVersion.h>
using namespace ForayUtility;

//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
int main(int argc, char *argv[]){

    cout << "svn revision number:  " << ForayVersion::get_svn_revision_string() << endl;
    cout << "svn last edit date :  " << ForayVersion::get_svn_date_string() << endl;
}


