#! /usr/bin/env python

from sand   import Sand
from fault  import Fault

if __name__ == "__main__":

    print "At the beach."

    sand = Sand()

    print "Number of sand castles is ", sand.numberOfCastles()


    print "Names:"
    print sand.namesOfCastles()
    print

    fault = Fault()
    fault.add_msg("test message Line 1\n");
    fault.add_msg("test message Line 2\n");
    print fault.msg(),
    print

    try:
        print "Calling castleFault"
        sand.castleFault()

    except Fault,f:
        print "Fault caught, message is:"
        print f.msg()



        
