import sys
import re


tableB = {}
tableD = {}
tableA = {}
tableF = {}

def decode(mnemonic):
    if mnemonic in tableA:
        return tableA[mnemonic][0]
    elif mnemonic in tableB:
        return tableB[mnemonic][0]
    else:
        return "NA"

def mystrip(l):
    return str.strip(l, "{}[]<>")

def formatThis(code):
    l = [code[0],code[1:3],code[3:]]
    return ";".join(l)

def onlyDashes(x):
    return not bool(re.match('^-+$', x))

def addToTable(key, dictionary, value):
    if key in dictionary:
        print "OH NO! key ", key, " is already in dictionary"
    else:
        dictionary[key] = value


def main():
    # print command line arguments
    for arg in sys.argv[1:]:
        print arg


    inputFile = sys.argv[1]
    if (len(sys.argv) > 2):
        if sys.argv[2] == 'decorate':
            decorate = True
        else:
            decorate = False

    # for line in open("/scr/sci/dixon/data/bufr/src/reflectivity/prepobs_prep.bufrtable",'r'):
    for line in open(inputFile):    #  "/scr/sci/dixon/data/bufr/src/radialwind/bufr_radar.table",'r'):
        if (line[0]  == "*"):
            print "skipping comment ", line
        else:
            tokens = line.split("|")
            # remove empty items in list and remove leading and trailing whitespace
            cleanLine = filter(onlyDashes, filter(None, map(str.strip, tokens)))

            # stick into a dictionary with the mnemonic as the key

            # for each line,
            size = len(cleanLine)
            if (size > 1):
                if (size == 2):
                    addToTable(cleanLine[0], tableD, cleanLine[1:])
                    # tableD[cleanLine[0]] = cleanLine[1:]
                elif (size == 3):
                    if (cleanLine[1].find("A") > -1):
                        addToTable(cleanLine[0], tableA, cleanLine[1:])
                        # tableA[cleanLine[0]] = cleanLine[1:]
                    else:
                        addToTable(cleanLine[0], tableB, cleanLine[1:])
                        # tableB[cleanLine[0]] = cleanLine[1:]
                else:
                    addToTable(cleanLine[0], tableF, cleanLine[1:])
                    # tableF[cleanLine[0]] = cleanLine[1:]
            else:
                print "not key, value pair: ", cleanLine




    # print the dictionary
    print "Table A"
    print "{:<8} {:<15}".format('Key','Value')
    for k, v in tableA.iteritems():
        print "{:<8} {:<15}".format(k, v)

    print "Table B"
    print "{:<8} {:<15}".format('Key','Value')
    for k, v in tableB.iteritems():
        if (k in tableF):
            fv = filter(None, map(str.strip,tableF[k]))
            nElements = len(fv)
            # -2 to ignore the ending -------
            scaleBaseU = [fv[-1]] + fv[0:nElements-1]
            something = ";".join(scaleBaseU)
        else:
            something = "MISSING"
        print k, ":"
        print "{:<8};{:<15};{:<15}".format(formatThis(v[0]), str.strip(v[1]), something)

    print "Table D"
    print "{:<8} {:<15}".format('Key','Value')
    for k, v in tableD.iteritems():
        if k in tableB:
            num = tableB[k][0]
        elif k in tableA:
            num = tableA[k][0]
        else:
            num = k
        # decode each element of list
        v1 = v[0].split()
        v2 = filter(None, map(mystrip, v1))
        v3 = map(decode, v2)
        print k, ":"
        print "{:<8}; {:<15}".format(formatThis(num), formatThis(v3[0]))  # map(formatThis,v3)
        for n in v3[1:]:
            print "{:<8}; {:<15}".format(" ;  ;   ", formatThis(n))
            #    print "{:<8} ; {:<15}".format(num, v3)
            #    print "{:<8} {:<15}".format(k, v)

    print "Table F"
    print "{:<8} {:<15}".format('Key','Value')
    for k, v in tableF.iteritems():
        print "{:<8} {:<15}".format(k, v)


if __name__ == "__main__":
    main()
