#---------------------------------------------------------------------
#
# Makefile for script files
#
#---------------------------------------------------------------------

#	Default INSTALL_DIR, should be overridden on command line

INSTALL_DIR=$(RAP_BIN_DIR)

#	Script files to be installed

SCRIPTS = \
	convert_image.csh \
	make_anim.csh

#	Targets

all: install

install_shared:
	$(MAKE) RAP_BIN_DIR=$(RAP_SHARED_BIN_DIR) install

install: create_subdir
	if test $(INSTALL_DIR) != . ;\
	then \
		cp $(SCRIPTS) $(INSTALL_DIR) ;\
	fi

create_subdir:
	if test -d $(INSTALL_DIR) ;\
	then \
		cd . ;\
	else \
		mkdir $(INSTALL_DIR) ;\
	fi

install_bin: install
install_shared_bin: install_shared

# unused targets - needed to prevent make failures

clean:
debug:
profile:
opt:
lint:
depend:
clean_bin:
clean_tdrp:
clean_depend:
clean_all:
dbgopt:
staticopt:


