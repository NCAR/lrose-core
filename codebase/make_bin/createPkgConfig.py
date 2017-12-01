

import os
import sys
import glob
import re


def write_dictionary(info):
    for key, value in info.iteritems():
        print >> sys.stdout, "#" + key 
        for key2, value2 in value.iteritems():
            result = "   " + key + '_' + key2 + '='
            if (len(value2) > 0):
                result += formatted(value2)
            print >> sys.stdout, result

def write_variables(variable_defs):
    for key_arch, value_defs in variable_defs.iteritems():
        print >> sys.stdout, "# architecture-" + key_arch
        for key, value2 in value_defs.iteritems():
            result = key + "="
            if (len(value2) > 0):
                result += formatted(value2)                                     
                print >> sys.stdout, result
        print >> sys.stdout, " "


def write_the_script(variables,idict_libs, idict_apps):
# how to do this?
# write output to files that are "included" in the shell script framwork?
# write the entire file and keep the template as strings in python file?
    print >> sys.stdout, "#! /bin/sh"
    write_variables(variable_defs)  # write to separate files based on architecture
    write_dictionary(idict_libs)    # write to one output file
    write_dictionary(idict_apps)

# create a dictionary with keys of target architecture and
#                          values that are dictionaries with keys that are variable names
#                                                            values are lists of strings
def get_variables(variables_to_find, path):
    variable_defs = {}
    filelist = glob.glob(path + '/rap_make.*')
    for file in filelist:
        filename, file_extension = os.path.splitext(file)
        var_defs_for_architecture = {}
        for var in variables_to_find:
            valList = getValueListForKey(file, var) # "NETCDF4_LIBS")   
            var_defs_for_architecture[var] = valList
        variable_defs[file_extension.replace(".","")] = var_defs_for_architecture

    for key_arch, value_defs in variable_defs.iteritems():
        print >> sys.stdout, "architecture-" + key_arch
        for key, value2 in value_defs.iteritems():
            result = key + "="
            if (len(value2) > 0):
                result += formatted(value2)
#                formatted = '\"' + " ".join(value2) + '\"'
                print >> sys.stdout, result
        print >> sys.stdout, " "
    return variable_defs

def detect_variable(astring):
# ^\$[(]   matches $( at the beginning of the string
# (\w+)    captures any following alphanumeric characters and the underscore
    m2 = re.match("^\$[(](\w+)", astring)
    if m2:
        return m2.group(1)
 


def main(path, module_keyword):
    info = {} # info is a dictionary with module names as keys
    loc_includes = set()
    loc_libs = set()
    loc_ldflags = set()
    loc_cflags = set()
    current_module = ""
    variables_to_retrieve = set()
    makefileNames = ["makefile", "Makefile", "_makefile"]
    rootDir = path + '/.'
    for root, dirs, files in os.walk(rootDir):  # , topdown=False):
        # print >> sys.stdout, "root dir: " + root
        # don't need dirs at this time, because we will hit them the
        # next time around as root
	# for dir in dirs:
        #    print >> sys.stdout, "dir: " + dir
        #    dirPath = os.path.join(root, dir)

        # check if makefile exists, extract info if possible                    
                                            
        for name in makefileNames:
            makefileName = os.path.join(root, name)
            if (os.path.isfile(makefileName)):
                # print >>sys.stdout, "Extracting from " + makefileName

                # search for MODULE_NAME key in makefile
                valList = getValueListForKey(makefileName, module_keyword) # "MODULE_NAME")
                if (len(valList) > 0):
                    # print >> sys.stdout, "New Module ", valList[0]
                    # store the current loc sets with the current module
                    if (current_module != ""):
                        info[current_module] = {"includes":loc_includes, "libs":loc_libs, "ldflags":loc_ldflags, "cflags":loc_cflags}
                    current_module = valList[0]
                    loc_includes = set()
                    loc_libs = set()
                    loc_ldflags = set()
                    loc_cflags = set()

#                else:
#                    valList = getValueListForKey(makefileName, "TARGET_FILE")
#                    if (len(valList) > 0):
#                        print >> sys.stdout, "New Target file ", valList[0]
#                    # store the current loc sets with the current module                                  
#                        if (current_module != ""):
#                            info[current_module] = {"includes":loc_includes, "libs":loc_libs, "ldflags":loc_ldflags, "cflags":loc_cflags}
#                        current_module = valList[0]
#                        loc_includes = set()
#                        loc_libs = set()
#                        loc_ldflags = set()
#                        loc_cflags = set()


                valList = getValueListForKey(makefileName, "LOC_LIBS")
                # print >> sys.stdout, "valList " 
                for item in valList:
                    # print >> sys.stdout, item
                    potential_variable = detect_variable(item)
                    if (potential_variable):
                        # stuff it into the variable list to retrieve later
                        variables_to_retrieve.add(potential_variable)
                    loc_libs.add(item)

                valList = getValueListForKey(makefileName, "LOC_INCLUDES")                   
                # print >> sys.stdout, "valList "                         
                for item in valList:
                    # print >> sys.stdout, item   
                    potential_variable = detect_variable(item)
                    if (potential_variable):
                        # stuff it into the variable list to retrieve later 
                        variables_to_retrieve.add(potential_variable)
                    loc_includes.add(item)

                valList = getValueListForKey(makefileName, "LOC_LDFLAGS")                  
                # print >> sys.stdout, "valList "                                                     
                for item in valList:
                    # print >> sys.stdout, item
                    potential_variable = detect_variable(item)
                    if (potential_variable):
                        # stuff it into the variable list to retrieve later 
                        variables_to_retrieve.add(potential_variable)          
                    loc_ldflags.add(item)

                valList = getValueListForKey(makefileName, "LOC_CFLAGS")                
                # print >> sys.stdout, "valList "                                                       
                for item in valList:
                    # print >> sys.stdout, item                      
                    potential_variable = detect_variable(item)
                    if (potential_variable):
                        # stuff it into the variable list to retrieve later            
                        variables_to_retrieve.add(potential_variable)                                 
                    loc_cflags.add(item)


    # store the current loc sets with the current module                                  
    if (current_module != ""):
        info[current_module] = {"includes":loc_includes, "libs":loc_libs, "ldflags":loc_ldflags,"cflags":loc_cflags}
#    print >> sys.stdout, "at the end: "
#    print >> sys.stdout, "variables found: ", variables_to_retrieve
#    print >> sys.stdout, "loc_libs = "
#    print >> sys.stdout, loc_libs

#    print >> sys.stdout, "loc_includes = "
#    print >> sys.stdout, loc_includes

#    print >> sys.stdout, "loc_ldflags = "
#    print >> sys.stdout, loc_ldflags

#    print >> sys.stdout, "loc_cflags = "
#    print >> sys.stdout, loc_cflags

#    write_dictionary(info)

#    for key, value in info.iteritems():
#        print >> sys.stdout, key 
#        for key2, value2 in value.iteritems():
#            result = "   " + key + '_' + key2 + '='
#            if (len(value2) > 0):
#                result += formatted(value2)
#            print >> sys.stdout, result

    return variables_to_retrieve, info


def formatted(set_or_list):
    if (len(set_or_list) >= 0):
        result =  '\"' + " ".join(set_or_list) + '\"'
        result = result.replace("(", "{")
        result = result.replace(")", "}")
        return result
    else:
        return

# def prettyPrint(dictionary):

    

# get string value based on search key
# the string may span multiple lines
#
# Example of keys: SRCS, SUB_DIRS, MODULE_NAME, TARGET_FILE
#
# value is returned

def getValueListForKey(path, key):

    valueList = []

    try:
        fp = open(path, 'r')
    except IOError as e:
        print >>sys.stdout, "ERROR - ", thisScriptName
        print >>sys.stdout, "  Cannot open file:", path
        print >>sys.stdout, "  dir: ", options.dir
        return valueList

    lines = fp.readlines()
    fp.close()

    foundKey = False
    multiLine = ""
    for line in lines:
        if (foundKey == False):
            if (line[0] == '#'):
                continue
        if (line.find(key) >= 0):
            foundKey = True
            multiLine = multiLine + line
            if (line.find("\\") < 0):
                break;
        elif (foundKey == True):
            if (line[0] == '#'):
                break
            if (len(line) < 2):
                break
            multiLine = multiLine + line;
            if (line.find("\\") < 0):
                break;

    if (foundKey == False):
        return valueList

    multiLine = multiLine.replace(key, " ",1)
    multiLine = multiLine.replace("=", " ")
    multiLine = multiLine.replace("\t", " ")
    multiLine = multiLine.replace("\\", " ")
    multiLine = multiLine.replace("\r", " ")
    multiLine = multiLine.replace("\n", " ")

    toks = multiLine.split(' ')
    for tok in toks:
        if (len(tok) > 0):
            valueList.append(tok)

    return valueList



########################################################################
                                                                          
# Run - entry point

if __name__ == "__main__":
    # path = './make_include'
    # get_variables(["NETCDF4_LIBS", "NETCDF4_LDFLAGS", "NETCDF4_INC"], path)
    # apps_path = './apps'
    # module_keyword = 'TARGET_FILE'
    # idict = main(apps_path, module_keyword)
    libs_path = './libs'
    module_keyword = 'MODULE_NAME'
    variables_libs, idict_libs = main(libs_path, module_keyword)
    # path = './make_include'
    # get_variables(variables, path)

    apps_path = './apps'                                                                               
    module_keyword = 'TARGET_FILE'                                                                     
    variables_apps, idict_apps = main(apps_path, module_keyword)  

    path = './make_include'
    variables = variables_libs.union(variables_apps)
    variable_defs = get_variables(variables, path)

    write_the_script(variable_defs, idict_libs, idict_apps)
