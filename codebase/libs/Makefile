#
# Recursive make - makes the subdirectory code
#

include $(RAP_MAKE_INC_DIR)/rap_make_macros

TARGETS = $(GENERAL_TARGETS) $(LIB_TARGETS) $(DOC_TARGETS) $(INSTALL_TARGETS)

SUB_DIRS = \
	Mcidas_2003 \
        Mdv \
	MotifApp \
	SpdbServer \
	advect \
        cidd \
	class_tools/src \
        colide \
	contour \
        dataport \
	devguide \
	didss \
	dsdata \
	dsserver \
        euclid \
	Fmq \
	grib \
	history_forecast \
	jni \
	mdv \
	perl5 \
	physics \
	prod_sel \
	rapformats \
	rapmath \
	rapplot \
        rdi \
	rsbutil \
	sim \
	spdb \
	spdbFormats \
	symprod \
	symprod98 \
        tdrp \
	tetwws \
        tiff \
        titan \
        toolsa \
	xmlformats


$(TARGETS): FORCE
	$(MAKE) $(SUB_DIRS) TARGET=$@

$(SUB_DIRS): FORCE
	cd $@; $(MAKE) $(TARGET)

FORCE:





