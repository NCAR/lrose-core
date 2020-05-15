#
# Recursive make - makes the subdirectory code
#

include $(LROSE_CORE_DIR)/build/make_include/lrose_make_macros

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
