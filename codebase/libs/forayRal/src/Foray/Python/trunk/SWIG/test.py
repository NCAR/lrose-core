#!/usr/bin/env python

from foray import ForayVersion

if __name__=="__main__":

    print "Foray Version is ", ForayVersion.get_svn_revision_string()


