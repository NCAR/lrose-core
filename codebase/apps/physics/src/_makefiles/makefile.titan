#
# Recursive make - makes the subdirectory code
#

include $(RAP_MAKE_INC_DIR)/rap_make_macros

TARGETS = $(GENERAL_TARGETS) $(INSTALL_TARGETS)

SUB_DIRS = \
        AverageFields \
        CalcHumidity \
        ModelDerive \
        NegBuoyancy \
        Pressure2Height \
        ShearForLeadingEdge \
        ThetaEAdvect \
        TrendField \
        VerticalAverage \
        VerticalShear \

$(TARGETS): FORCE
	$(MAKE) $(SUB_DIRS) TARGET=$@

$(SUB_DIRS): FORCE
	cd $@; $(MAKE) $(TARGET)

FORCE:
