# *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
# ** Copyright UCAR (c) 1992 - 2010 
# ** University Corporation for Atmospheric Research(UCAR) 
# ** National Center for Atmospheric Research(NCAR) 
# ** Research Applications Laboratory(RAL) 
# ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA 
# ** 2010/10/7 23:12:37 
# *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
###########################################################################
#
# Makefile for the MathParse library
#
#
###########################################################################

include $(RAP_MAKE_INC_DIR)/rap_make_macros

TARGET_FILE = ../librapmath.a

LOC_INCLUDES = -I../include
LOC_CFLAGS =
LOC_INC_DIR = ../include 
MY_CPPCHECKFLAGS = #-AddStyleFAA -exclude-rules="4.14,4.09,4.12,4.16"

HDRS = \

CPPC_SRCS = \
AssignmentNode.cc \
BinaryArgs.cc \
BinaryNode.cc \
Find.cc \
FindSimple.cc \
FloatUserData.cc \
LeafContent.cc \
LeafNode.cc \
LogicalArgs.cc \
LogicalNode.cc \
MathData.cc \
MathDataSimple.cc \
MathParser.cc \
ProcessingNode.cc \
SpecialUserData.cc \
StatusUserData.cc \
UnaryNode.cc

#
# general targets
#

include $(RAP_MAKE_INC_DIR)/rap_make_lib_module_targets

#
# local targets
#

depend: depend_generic

# DO NOT DELETE THIS LINE -- make depend depends on it.
