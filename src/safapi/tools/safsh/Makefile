## Copyright(C) 1999-2005 The Regents of the University of California.
##     This work  was produced, in  part, at the  University of California, Lawrence Livermore National
##     Laboratory    (UC LLNL)  under    contract number   W-7405-ENG-48 (Contract    48)   between the
##     U.S. Department of Energy (DOE) and The Regents of the University of California (University) for
##     the  operation of UC LLNL.  Copyright  is reserved to  the University for purposes of controlled
##     dissemination, commercialization  through formal licensing, or other  disposition under terms of
##     Contract 48; DOE policies, regulations and orders; and U.S. statutes.  The rights of the Federal
##     Government  are reserved under  Contract 48 subject  to the restrictions agreed  upon by DOE and
##     University.
## 
## Copyright(C) 1999-2005 Sandia Corporation.  
##     Under terms of Contract DE-AC04-94AL85000, there is a non-exclusive license for use of this work
##     on behalf of the U.S. Government.  Export  of this program may require a license from the United
##     States Government.
## 
## Disclaimer:
##     This document was  prepared as an account of  work sponsored by an agency  of  the United States
##     Government. Neither the United States  Government nor the United States Department of Energy nor
##     the  University  of  California  nor  Sandia  Corporation nor any  of their employees  makes any
##     warranty, expressed  or  implied, or  assumes   any  legal liability  or responsibility  for the
##     accuracy,  completeness,  or  usefulness  of  any  information, apparatus,  product,  or process
##     disclosed,  or  represents that its  use would   not infringe  privately owned rights. Reference
##     herein  to any  specific commercial  product,  process,  or  service by  trade  name, trademark,
##     manufacturer,  or  otherwise,  does  not   necessarily  constitute  or  imply  its  endorsement,
##     recommendation, or favoring by the  United States Government   or the University of  California.
##     The views and opinions of authors expressed herein do not necessarily state  or reflect those of
##     the  United  States Government or  the   University of California   and shall  not be  used  for
##     advertising or product endorsement purposes.
## 
## 
## Active Developers:
##     Peter K. Espen              SNL
##     Eric A. Illescas            SNL
##     Jake S. Jones               SNL
##     Robb P. Matzke              LLNL
##     Greg Sjaardema              SNL
## 
## Inactive Developers:
##     William J. Arrighi          LLNL
##     Ray T. Hitt                 SNL
##     Mark C. Miller              LLNL
##     Matthew O'Brien             LLNL
##     James F. Reus               LLNL
##     Larry A. Schoof             SNL
## 
## Acknowledgements:
##     Marty L. Barnaby            SNL - Red parallel perf. study/tuning
##     David M. Butler             LPS - Data model design/implementation Spec.
##     Albert K. Cheng             NCSA - Parallel HDF5 support
##     Nancy Collins               IBM - Alpha/Beta user
##     Linnea M. Cook              LLNL - Management advocate
##     Michael J. Folk             NCSA - Management advocate 
##     Richard M. Hedges           LLNL - Blue-Pacific parallel perf. study/tuning 
##     Wilbur R. Johnson           SNL - Early developer
##     Quincey Koziol              NCSA - Serial HDF5 Support 
##     Celeste M. Matarazzo        LLNL - Management advocate
##     Tyce T. McLarty             LLNL - parallel perf. study/tuning
##     Tom H. Robey                SNL - Early developer
##     Reinhard W. Stotzer         SNL - Early developer
##     Judy Sturtevant             SNL - Red parallel perf. study/tuning 
##     Robert K. Yates             LLNL - Blue-Pacific parallel perf. study/tuning
## 

top_srcdir=../../..
top_builddir=../../..
srcdir=.
##------------------------------------------------------------ -*- makefile -*-
## The following section of this makefile comes from the
## `./config/commence' file which was generated with config.status
## from `./config/commence.in'.
##-----------------------------------------------------------------------------

## Copyright(C) 1999-2003 The Regents of the University of California.
##     This work  was produced, in  part, at the  University of California, Lawrence Livermore National
##     Laboratory    (UC LLNL)  under    contract number   W-7405-ENG-48 (Contract    48)   between the
##     U.S. Department of Energy (DOE) and The Regents of the University of California (University) for
##     the  operation of UC LLNL.  Copyright  is reserved to  the University for purposes of controlled
##     dissemination, commercialization  through formal licensing, or other  disposition under terms of
##     Contract 48; DOE policies, regulations and orders; and U.S. statutes.  The rights of the Federal
##     Government  are reserved under  Contract 48 subject  to the restrictions agreed  upon by DOE and
##     University.
## 
## Copyright(C) 1999-2003 Sandia Corporation.
##     Under terms of Contract DE-AC04-94AL85000, there is a non-exclusive license for use of this work
##     on behalf of the U.S. Government.  Export  of this program may require a license from the United
##     States Government.
## 
## Disclaimer:
##     This document was  prepared as an account of  work sponsored by an agency  of  the United States
##     Government. Neither the United States  Government nor the United States Department of Energy nor 
##     the  University  of  California  nor  Sandia  Corporation nor any  of their employees  makes any 
##     warranty, expressed  or  implied, or  assumes   any  legal liability  or responsibility  for the 
##     accuracy,  completeness,  or  usefulness  of  any  information, apparatus,  product,  or process 
##     disclosed,  or  represents that its  use would   not infringe  privately owned rights. Reference 
##     herein  to any  specific commercial  product,  process,  or  service by  trade  name, trademark, 
##     manufacturer,  or  otherwise,  does  not   necessarily  constitute  or  imply  its  endorsement, 
##     recommendation, or favoring by the  United States Government   or the University of  California.  
##     The views and opinions of authors expressed herein do not necessarily state  or reflect those of
##     the  United  States Government or  the   University of California   and shall  not be  used  for
##     advertising or product endorsement purposes.
## 
## Active Developers:
##     Peter K. Espen              SNL
##     Christi Forsythe            SNL
##     Jake S. Jones               SNL
##     Robb P. Matzke              LLNL
##     Mark C. Miller              LLNL
##     Greg Sjaardema              SNL
## 
## Inactive Developers:
##     William J. Arrighi          LLNL
##     Ray T. Hitt                 SNL
##     Matthew J. O'Brien          LLNL
##     James F. Reus               LLNL
##     Larry A. Schoof             SNL
## 
## Acknowledgements:
##     Marty L. Barnaby            SNL - parallel perf. study/tuning
##     David M. Butler             LPS - Data model design/implementation Spec.
##     Albert K. Cheng             NCSA - Parallel HDF5 support
##     Nancy Collins               IBM - Alpha/Beta user
##     Linnea M. Cook              LLNL - Management advocate
##     Michael J. Folk             NCSA - Management advocate 
##     Richard M. Hedges           LLNL - parallel perf. study/tuning 
##     Shirley R. Jennings         LLNL - Occasional release support
##     Wilbur W. Johnson           SNL - Early Developer
##     Quincey A. Koziol           NCSA - Serial HDF5 Support 
##     Celeste M. Matarazzo        LLNL - Management advocate
##     Tyce T. McLarty             LLNL - parallel perf. study/tuning
##     Tom H. Robey                SNL - Early developer
##     Reinhard W. Stotzer         SNL - Early developer
##     Judy Sturtevant             SNL - parallel perf. study/tuning 
##     Robert K. Yates             LLNL - Pacific parallel perf. study/tuning
## 

## How to search directories for target dependencies
VPATH=. $(srcdir) $(top_builddir)/include $(top_srcdir)/include

## Things that Make needs
.SUFFIXES:
.SUFFIXES: .c .o .lo .java .class


## Programs
SHELL=/bin/sh
AUTOCONF=/Users/miller86/tmp/jekyll_stuff/homebrew/bin/autoconf
CC=gcc
CFLAGS=-g -O2 
CXX=
CXXFLAGS=
CPPFLAGS=-I. -I${srcdir} -I${top_builddir}/include -I${top_srcdir}/safapi/lib -I$(top_builddir)/sslib/lib -I$(top_srcdir)/sslib/lib  
LDFLAGS=
LIBS=-lz -lm 
AR=
RANLIB=ranlib
RM=rm -f
CP=cp
INSTALL=$(top_srcdir)/config/install-sh
INSTALL_PROGRAM=$(top_srcdir)/config/install-sh -c -m 755
INSTALL_DATA=$(top_srcdir)/config/install-sh -c -m 644
RUNSERIAL=
RUNPARALLEL=
RUNTEST=SAF_ASSERT_DISABLE=none SAF_PRECOND_DISABLE=none SAF_POSTCOND_DISABLE=none $(RUNPARALLEL)
BANNER=$(top_srcdir)/config/banner
JAVA=false
JAVAC=false
JAVAH=false
JAVASDK=false
JNI_INCLUDE=false
JAVA_INCLUDE=false
BROWSER_SHARED_FLAG=

PYTHON=false
PYTHONDIR=false
PYTHON_INCLUDE=false
SAFSH_SHARED_FLAG=

## mkdoc (all mkdoc flags come from the various META files now)
mkdoc=$(top_srcdir)/../tools/mkdoc
mkdoc_flags=
mkdoc_lib_meta=-c $(top_srcdir)/../tools/Formats/library/META
mkdoc_ex_meta=-c $(top_srcdir)/../tools/Formats/example/META
mkdoc_sslib_meta=-c $(top_srcdir)/sslib/docs/mkdoc/SSlib-API.mkdoc
mkdoc_phaseI_meta=-c $(top_srcdir)/sslib/docs/mkdoc/Phase-I.mkdoc

## Installation points
prefix=/usr/local
exec_prefix=${prefix}
bindir=${exec_prefix}/bin
libdir=${exec_prefix}/lib
includedir=${prefix}/include
datadir=${prefix}/share
sysconfdir=${prefix}/etc
docsdir=/usr/local/docs
PUB_LIB=$(LIB)
TEST_FILE_PATH=.

## Shared libraries
LT=$(top_builddir)/libtool
LT_COMPILE=$(LT) --mode=compile $(CC)
LT_LINK_LIB=$(LT) --mode=link $(CC) -rpath $(libdir)
LT_LINK_EXE=LDFLAGS= $(LT) --mode=link $(CC) -rpath $(bindir)
LT_RUN=LDFLAGS= $(LT) --mode=execute
LT_INSTALL_PROG=$(LT) --mode=install $(INSTALL_PROGRAM)
LT_INSTALL_LIB=$(LT) --mode=install $(INSTALL_DATA)
LT_INSTALL_DATA=$(LT) --mode=install $(INSTALL_DATA)
LT_UNINSTALL=$(LT) --mode=uninstall $(RM)

## Optional variables. We must declare them here because Irix pmake
## complains if it sees a reference to a variable which has never been
## defined. The main makefile is free to redefine these to something else.
LIB=
LIB_SRC=
LIB_OBJ=
PUB_HDR=
PRIV_HDR=
LIB_DEPS=
PROGS=
TESTS=
CHECKS=
CHECK_ARGS=
TEST_SCRIPTS=
DATA=
CONFIG=
TOOLS=
TOOLS_SCRIPTS=
SUBDIRS=
SSLIB_HTML_DOCS=
EXP_HTML_DOCS=
SDIFF_HTML_DOCS=
ENSIGHT_HTML_DOCS=
SAF_HTML_DOCS=
SSLIB_DOCUMENTS=
SAF_DOCUMENTS=

## The default is to build the tools and tests, which also results in
## building the library.
all: tests
.PHONY: all

## The following rules insure that the Makefile is up-to-date by rerunning
## various autoconf components (although not all versions of make assume
## that the makefile is implicitly a target).  We use time stamp files to
## keep track of the most recent update of config.h.in and config.h
## because autoheader and config.status don't update the modification time
## if the contents don't change.
##
## Graphically, the dependencies are:
##
##		         configure.in
##                           |  |
##         +-----------------+  +------------+
##         |                                 |
##      stamp1                           configure
##   (config.h.in)                           |
##       |  |                                |
##       |  +---------------+    +-----------+
##       |                  |    |
##       |               config.status
##       |                     |
##       |  +------------------+
##       |  |
##      stamp2
##     (config.h)                      Makefile.in et al
##         |                                 |
##         +------------------+  +-----------+
##                            |  |
##                          Makefile
##
## A side effect of updating stamp1 is to generate config.h.in and a
## side effect of updating stamp2 is to generate config.h.  When using
## a version of make that doesn't treat the makefile as the initial target
## the user may want to occassionally type `make Makefile' in any source
## directory.
##
STAMP1=$(top_builddir)/config/stamp1
STAMP2=$(top_builddir)/config/stamp2

MAKEFILE_PARTS=$(srcdir)/Makefile.in		\
	       $(top_srcdir)/config/commence.in	\
	       $(top_srcdir)/config/conclude.in	\
	       $(top_srcdir)/config/depend1.in	\
	       $(top_srcdir)/config/depend2.in	\
	       $(top_srcdir)/config/depend3.in	\
	       $(top_srcdir)/config/depend4.in	\
	       $(top_srcdir)/config/dependN.in

$(STAMP1): $(top_srcdir)/configure.in
	touch $(STAMP1)
	-cd $(top_srcdir); autoheader

$(STAMP2): $(STAMP1) $(top_builddir)/config.status
	touch $(STAMP2)
	-cd $(top_builddir); \
	   CONFIG_FILES= CONFIG_HEADERS=include/SAFconfig.h ./config.status

$(top_srcdir)/configure: $(top_srcdir)/configure.in
	-cd $(top_srcdir); $(AUTOCONF)

$(top_builddir)/config.status: $(top_srcdir)/configure $(STAMP1)
	-cd $(top_builddir); ./config.status --recheck

# The following target was renamed from `Makefile' to `Makefiles' because
# we decided that automatic building of Makefiles was a nuisance.
# [Robb Matzke, LLNL, 2000-08-17]
Makefiles: $(MAKEFILE_PARTS) $(STAMP2)
	-cd $(top_builddir); CONFIG_HEADERS= ./config.status


##-----------------------------------------------------------------------------
## The following section of this makefile comes from the middle of
## `Makefile.in' from this directory.  It was generated by running
## `config.status'.
##-----------------------------------------------------------------------------

# All tools depend on the other SAF libraries. Make sure they're up to date.
LIB_DEPS=$(top_builddir)/safapi/lib/libsafapi.la

#new, Makefile symbols instead of autoconf symbols
LIBSAFAPI_LA=../../lib/libsafapi.la
LIBS=-lz -lm 


# Only edit past this point if you want to 

safsh_srcdir=${top_srcdir}/safapi/tools/safsh

TOOLS=ss.so safsh safsh.py Tree.py

Z_LIB=/usr/local/lib

Z_LIB=/usr/local/lib

ss.o: ${safsh_srcdir}/ss.c ${safsh_srcdir}/ss.h
	${CC} ${SAFSH_SHARED_FLAG} ${CPPFLAGS} ${CFLAGS} -I${PYTHON_INCLUDE} \
	-c ${safsh_srcdir}/ss.c -o ss.o

ss.so: ss.o
	${CC} ${SAFSH_SHARED_FLAG} ${CPPFLAGS} ${CFLAGS} -I${PYTHON_INCLUDE} \
	-L../../lib/.libs -L../../../sslib/lib/.libs \
	${LDFLAGS} \
	ss.o -o ss.so  -lsafapi -lss ${LIBS} ${SAFAPI_LIBS}
	if test -f safsh ; then /bin/true ; else cp -f ${safsh_srcdir}/safsh . ; fi
	if test -x safsh ; then /bin/true ; else chmod +x safsh ; fi
	if test -f safsh.py ; then /bin/true ; else cp -f ${safsh_srcdir}/safsh.py . ; fi
	if test -f Tree.py ; then /bin/true ; else cp -f ${safsh_srcdir}/Tree.py . ; fi

safsh: ss.so
	if test -f safsh ; then /bin/true ; else cp -f ${safsh_srcdir}/safsh . ; fi
	if test -x safsh ; then /bin/true ; else chmod +x safsh ; fi
	touch safsh

safsh.py: ss.so
	if test -f safsh.py ; then /bin/true ; else cp -f ${safsh_srcdir}/safsh.py . ; fi
	touch safsh.py

Tree.py: ss.so
	if test -f Tree.py ; then /bin/true ; else cp -f ${safsh_srcdir}/Tree.py . ; fi
	touch Tree.py

CLEAN: ss.o ss.so

##------------------------------------------------------------ -*- makefile -*-
## The following section of this makefile comes from the
## `./config/conclude' file which was generated with config.status
## from `./config/conclude.in'.
##-----------------------------------------------------------------------------

## Copyright(C) 1999 The Regents of the University of California.
##     This work  was produced, in  part, at the  University of California, Lawrence Livermore National
##     Laboratory    (UC LLNL)  under    contract number   W-7405-ENG-48 (Contract    48)   between the
##     U.S. Department of Energy (DOE) and The Regents of the University of California (University) for
##     the  operation of UC LLNL.  Copyright  is reserved to  the University for purposes of controlled
##     dissemination, commercialization  through formal licensing, or other  disposition under terms of
##     Contract 48; DOE policies, regulations and orders; and U.S. statutes.  The rights of the Federal
##     Government  are reserved under  Contract 48 subject  to the restrictions agreed  upon by DOE and
##     University.
## 
## Copyright(C) 1999 Sandia Corporation.
##     Under terms of Contract DE-AC04-94AL85000, there is a non-exclusive license for use of this work
##     on behalf of the U.S. Government.  Export  of this program may require a license from the United
##     States Government.
## 
## Disclaimer:
##     This document was  prepared as an account of  work sponsored by an agency  of  the United States
##     Government. Neither the United States  Government nor the United States Department of Energy nor 
##     the  University  of  California  nor  Sandia  Corporation nor any  of their employees  makes any 
##     warranty, expressed  or  implied, or  assumes   any  legal liability  or responsibility  for the 
##     accuracy,  completeness,  or  usefulness  of  any  information, apparatus,  product,  or process 
##     disclosed,  or  represents that its  use would   not infringe  privately owned rights. Reference 
##     herein  to any  specific commercial  product,  process,  or  service by  trade  name, trademark, 
##     manufacturer,  or  otherwise,  does  not   necessarily  constitute  or  imply  its  endorsement, 
##     recommendation, or favoring by the  United States Government   or the University of  California.  
##     The views and opinions of authors expressed herein do not necessarily state  or reflect those of
##     the  United  States Government or  the   University of California   and shall  not be  used  for
##     advertising or product endorsement purposes.
## 
## Authors:
## 	William J. Arrighi	LLNL
## 	Peter K. Espen		SNL
## 	Ray T. Hitt 		SNL
## 	Robb P. Matzke 		LLNL
## 	Mark C. Miller 		LLNL
## 	James F. Reus 		LLNL
## 	Larry A. Schoof 	SNL
## 
## Acknowledgements:
## 	Marty L. Barnaby	SNL - Red parallel perf. study/tuning
## 	David M. Butler		LPS - Data model design/implementation Spec.
## 	Albert K. Cheng		NCSA - Parallel HDF5 support
## 	Nancy Collins		IBM - Alpha/Beta user
## 	Linnea M. Cook		LLNL - Management advocate
## 	Michael J. Folk		NCSA - Management advocate 
## 	Richard M. Hedges	LLNL - Blue-Pacific parallel perf. study/tuning 
## 	Quincey A. Koziol	NCSA - Serial HDF5 Support 
## 	Celeste M. Matarazzo	LLNL - Management advocate
## 	Tom H. Robey 		SNL - Early developer
## 	Greg D. Sjaardema	SNL - Alpha/Beta user
## 	Reinhard W. Stotzer	SNL - Early developer
## 	Judy Sturtevant		SNL - Red parallel perf. study/tuning 
## 	Robert K. Yates		LLNL - Blue-Pacific parallel perf. study/tuning
## 

## Library objects have names similar to library sources
LIB_OBJ=$(LIB_SRC:.c=.lo)

## Dependencies for dependencies ;-) Dependencies can only depend on things
## easily generated by this makefile.  We assume that all library objects
## depend on the header files since we're unable to generate the *.c files
## easily (they depend on all of HDF5 being compiled and/or installed).
$(LIB_OBJ) .depend: $(PUB_HDR) $(PRIV_HDR)

## The `libs' phony target builds library objects and the library, both of
## which are optional. If library objects exist but no library then we just
## enter the names of the objects into the `.lib_obj' file so the parent
## makefile can find them to build the library (see the $(LIB) target below).
## This makefile cannot create a library if it has only some of the library
## objects because shared libraries cannot generally be incrementally linked.
.PHONY: libs
libs:
	@for d in X $(SUBDIRS); do \
	    if test $$d != X; then \
		(:; cd $$d && $(MAKE) $@) || exit 1; \
	    fi; \
	done
	@$(MAKE) --no-print-directory libs_here

## When compiling library source files, create a `.lib_obj' file which lists
## all the library objects, but only if we have library objects and are not
## linking a library in this directory.
.PHONY: libs_here
libs_here:
	@if test -z "$(LIB)" -a -n "$(LIB_OBJ)"; then \
	    echo $(LIB_OBJ) >.lib_obj; \
	fi
	@$(MAKE) --no-print-directory .lib_stamp
	@if test -n "$(LIB)"; then $(MAKE) --no-print-directory $(LIB) || exit 1; fi

## The `.lib_stamp' file is touched if any library source files were
## recompiled, allowing the parent directory to determine if it should relink
## its library.  If this isn't the root of the build tree then we also touch
## the `.lib_stamp' in the parent directory.
.lib_stamp: $(LIB_OBJ)
	@if test -n "$(LIB_OBJ)"; then \
	    (set -x; touch .lib_stamp); \
	    test "." != $(top_builddir) && (set -x; touch ../.lib_stamp); \
	fi

## The library objects defined in this directory with $(LIB_OBJ) together with 
## library objects in any subdirectories are linked to create a static and/or
## shared library.  The `__no_library__' is present to prevent a syntax error
## when the $(LIB) macro is empty.  The `.lib_stamp' file was touched
## by subdirectories which updated library objects which are to be
## linked here (ignore the file if it doesn't exist).
$(LIB) __no_library__: .lib_stamp
	@LIB_OBJ=; \
	for d in X $(SUBDIRS); do \
	    if test -d $$d -a -f $$d/.lib_obj; then \
		for f in X `cat $$d/.lib_obj`; do \
		    if test $$f != X; then \
			LIB_OBJ="$$LIB_OBJ $$d/$$f"; \
		    fi; \
		done; \
	    fi; \
	done; \
	$(LT_LINK_LIB) -o $@ $(CFLAGS) $(LIB_OBJ) $$LIB_OBJ

## The LIB_DEPS are the libraries (*.la) on which the tools and test
## depend. It is empty by default. Usual values are things like
## $(top_builddir)/lib/sslib/libss.la.
$(LIB_DEPS) lib_dep_dummy:
	@dir=`echo $@ |sed 's/\/lib[a-z_A-Z0-9]*\.la//'`; \
	set -x; \
	cd $$dir && $(MAKE) libs

$(TOOLS) $(TESTS) tools_tests_dummy: $(LIB_DEPS)

## Build data from the bottom up, always building the library (if any) at
## any given level then the tools before any data.
.PHONY: data
data:
	@for d in X $(SUBDIRS); do \
	   if test $$d != X; then \
		(set -x; cd $$d && $(MAKE) $@) || exit 1; \
	   fi; \
	done
	@$(MAKE)  data_here

.PHONY: data_here
data_here: tools_here $(TOOLS) $(TOOLS_SCRIPTS) $(DATA)

## Some makes refuse to understand that a target might not have anything to do
## with a file (even though we use .PHONY), so we've added another target
## called `_tools' which should be used instead..PHONY: tools
.PHONY: tools
tools:
	@echo "Some makefiles don't understand that a target can be something"
	@echo "other than a file. Therefore, we've been forced to rename this"
	@echo "target as _tools instead."
	exit 1

## Build tools from the bottom up, always building the library (if any) at
## any given level before the tools.
.PHONY: _tools
_tools:
	@for d in X $(SUBDIRS); do \
	    if test $$d != X; then \
		(:; cd $$d && $(MAKE) $@) || exit 1; \
	    fi; \
	done
	@$(MAKE) --no-print-directory tools_here

.PHONY: tools_here _tools_here
tools_here _tools_here: libs_here $(TOOLS) $(TOOLS_SCRIPTS)

## Build test suite from the bottom up, always building the tools (and thereby
## the library) at any given level before the tests.
.PHONY: tests
tests:
	@for d in X $(SUBDIRS); do \
	    if test $$d != X; then \
		(:; cd $$d && $(MAKE) $@) || exit 1; \
	    fi; \
	done
	@$(MAKE) --no-print-directory tests_here

.PHONY: tests_here
tests_here: tools_here $(DATA) $(TESTS)

## Documentation in HTML, MIF, Texinfo, GNU Info, or plain text format
.PHONY: documentation documentation_here
documentation_here: html_here mif_here texi_here text_here info_here rest_here
documentation:
	@for d in X $(SUBDIRS); do \
	    if test $$d != X; then \
		(:; cd $$d && $(MAKE) $@) || exit 1; \
	    fi; \
	done
	@$(MAKE) --no-print-directory documentation_here

.PHONY: rest rest_here
rest_here:
rest:
	@for d in X $(SUBDIRS); do \
	    if test $$d != X; then \
		(:; cd $$d && $(MAKE) $@) || exit 1; \
	    fi; \
	done
	@$(MAKE) --no-print-directory rest_here

.PHONY: html html_here
html_here:
html:
	@for d in X $(SUBDIRS); do \
	    if test $$d != X; then \
		(:; cd $$d && $(MAKE) $@) || exit 1; \
	    fi; \
	done
	@$(MAKE) --no-print-directory html_here

.PHONY: mif mif_here
mif_here:
mif:
	@for d in X $(SUBDIRS); do \
	    if test $$d != X; then \
		(:; cd $$d && $(MAKE) $@) || exit 1; \
	    fi; \
	done
	@$(MAKE) --no-print-directory mif_here

.PHONY: texi texi_here
texi_here:
texi:
	@for d in X $(SUBDIRS); do \
	    if test $$d != X; then \
		(:; cd $$d && $(MAKE) $@) || exit 1; \
	    fi; \
	done
	@$(MAKE) --no-print-directory texi_here

.PHONY: text text_here
text_here:
text:
	@for d in X $(SUBDIRS); do \
	    if test $$d != X; then \
		(:; cd $$d && $(MAKE) $@) || exit 1; \
	    fi; \
	done
	@$(MAKE) --no-print-directory text_here

.PHONY: info info_here
info_here:
info:
	@for d in X $(SUBDIRS); do \
	    if test $$d != X; then \
		(:; cd $$d && $(MAKE) $@) || exit 1; \
	    fi; \
	done
	@$(MAKE) --no-print-directory info_here

## Running the test suite should be from the bottom up.
.PHONY: check
check:
	@for d in X $(SUBDIRS); do \
	    if test $$d != X; then \
		(:; cd $$d && $(MAKE) $@) || exit 1; \
	    fi; \
	done
	@$(MAKE) --no-print-directory check_here

.PHONY: check_here
check_here: $(CHECKS)
	@for test in X $(TEST_SCRIPTS); do \
	    if test $$test != X; then \
		$(BANNER) $$test $(CHECK_ARGS); \
	        RUNSERIAL="$(RUNSERIAL)" RUNPARALLEL="$(RUNPARALLEL)" \
	            srcdir=$(srcdir) \
		    /bin/sh $$test $(CHECK_ARGS) || \
		    exit 1; \
	        echo ""; \
	    fi; \
	done

.PHONY: recheck
recheck:
	@for d in X $(SUBDIRS); do \
	    if test $$d != X; then \
	        (:; cd $$d && $(MAKE) $@) || exit 1; \
	    fi; \
	done
	rm -f $(CHECKS)
	$(MAKE) check;


## Make installation directories directories if they don't exist.
$(libdir):
	mkdir $@ && chmod 755 $@

$(includedir):
	mkdir $@ && chmod 755 $@

$(includedir)/private:
	mkdir $@ && chmod 755 $@

$(bindir):
	mkdir $@ && chmod 755 $@

$(datadir):
	mkdir $@ && chmod 755 $@

$(sysconfdir):
	mkdir $@ && chmod 755 $@

$(docsdir):
	mkdir $@ && chmod 755 $@

$(docsdir)/sslib_refman.html:
	mkdir $@ && chmod 755 $@

$(docsdir)/refman.html:
	mkdir $@ && chmod 755 $@

$(docsdir)/examples.html:
	mkdir $@ && chmod 755 $@

$(docsdir)/safdiff.html:
	mkdir $@ && chmod 755 $@

$(docsdir)/ensight_reader.html:
	mkdir $@ && chmod 755 $@

$(docsdir)/sslib_refman.rest:
	mkdir $@ && chmod 755 $@

$(docsdir)/refman.rest:
	mkdir $@ && chmod 755 $@

$(docsdir)/examples.rest:
	mkdir $@ && chmod 755 $@

$(docsdir)/safdiff.rest:
	mkdir $@ && chmod 755 $@


## Install the library, the public header files, and tools for this
## directory and all children from the bottom up.
.PHONY: install
install: data $(PUB_HDR) $(libdir) $(includedir) $(includedir)/private $(bindir) $(datadir) $(sysconfdir)
	@for d in X $(SUBDIRS); do \
	    if test $$d != X; then \
		(:; cd $$d && $(MAKE) $@) || exit 1; \
	    fi; \
	done
	@for f in X $(PUB_LIB); do \
	    if test $$f != X; then \
	        ($(LT_INSTALL_LIB) $$f $(libdir)/. || exit 1); \
	        libbase=`basename $$f .la`; \
	        if test -f $$libbase.settings; then \
	            (set -x; $(INSTALL_DATA) $$libbase.settings $(libdir)/. || exit 1);  \
	        fi; \
	    fi; \
	done
	@for f in X $(PUB_HDR); do \
	    if test $$f != X; then \
	        if test -f $$f; then \
		    (set -x; $(INSTALL_DATA) $$f $(includedir)/. || exit 1); \
	        else \
	            (set -x; $(INSTALL_DATA) $(srcdir)/$$f $(includedir)/. || exit 1); \
	        fi; \
	    fi; \
	done
	@# The source code distinguishes between public and private methods, datatypes, etc.
	@# by placing public items in public header files that are installed, and private
	@# items in private header files that are not installed.  This is a scoping
	@# mechanism.  However, certain clients have had the need to augment the API and
	@# required access to some private objects. Therefore, we will also install the
	@# private headers in a `private' subdirectory, replacing the scoping mechanism with
	@# a scoping policy.  The policy is that normal clients should include the public
	@# header(s) and will get definitions for the API, while other clients may include
	@# the private header(s) and will get definitions for both public and private items.
	@#
	@# Problems to watch out for when using these private files are:
	@#    1. The client must exercise care that a SAF private header file name doesn't
	@#	conflict with the name of a non-SAF header file (public SAF header files
	@#       have names of the form SAF*.h).
	@#    2. The client must use the `-I' or similar compiler switch so that both the private
	@#	public SAF include directories are searched.
	@#    3. The client must be aware that private header files are more closely tied to
	@#	the SAF library than the public files. A client that uses private SAF header files
	@#	will more likely need to be recompiled than a client that uses only public SAF
	@#	header files.
	@#
	@# This fixes HYPer03305.
	@for f in X $(PRIV_HDR); do \
	    if test $$f != X; then \
		if test -f $$f; then \
		    (set -x; $(INSTALL_DATA) $$f $(includedir)/private/. || exit 1); \
		else \
		    (set -x; $(INSTALL_DATA) $(srcdir)/$$f $(includedir)/private/. || exit 1); \
		fi; \
	    fi; \
	done
	@for f in X $(TOOLS) $(TOOLS_SCRIPTS); do \
	    if test $$f != X; then \
	       ($(LT_INSTALL_PROG) $$f $(bindir)/. || exit 1); \
	    fi; \
	done
	@for f in X $(DATA); do \
	    if test $$f != X; then \
	       ($(LT_INSTALL_DATA) $$f $(datadir)/. || exit 1); \
	    fi; \
	done
	@for f in X $(CONFIG); do \
	    if test $$f != X; then \
	       ($(LT_INSTALL_DATA) $$f $(sysconfdir)/. || exit 1); \
	    fi; \
	done
	@if test $(top_builddir) = "."; then \
	    $(LT) --mode=finish $(libdir); \
	fi


## Install the html documentation

.PHONY: install-docs
install-docs: $(docsdir) $(docsdir)/sslib_refman.html $(docsdir)/refman.html $(docsdir)/examples.html $(docsdir)/safdiff.html $(docsdir)/ensight_reader.html $(SSLIB_HTML_DOCS) $(SAF_HTML_DOCS) $(EXP_HTML_DOCS) $(SDIFF_HTML_DOCS) $(ENSIGHT_HTML_DOCS) $(SSLIB_DOCUMENTS) $(SAF_DOCUMENTS)
	@for d in X $(SUBDIRS); do \
	    if test $$d != X; then \
		(:; cd $$d && $(MAKE) $@) || exit 1; \
	    fi; \
	done
	@for i in X $(SSLIB_DOCUMENTS); do \
	    if test $$i != X; then \
		($(LT_INSTALL_DATA) $$i $(docsdir) || exit 1); \
	    fi; \
	done
	@for i in X $(SAF_DOCUMENTS); do \
	    if test $$i != X; then \
		($(LT_INSTALL_DATA) $$i $(docsdir) || exit 1); \
	    fi; \
	done
	@for i in X $(WEB_DOCUMENTS); do \
	    if test $$i != X; then \
		($(LT_INSTALL_DATA) $$i $(docsdir) || exit 1); \
	    fi; \
	done
	@for i in X $(SSLIB_HTML_DOCS); do \
	    if test $$i != X; then \
		($(LT_INSTALL_DATA) $$i $(docsdir)/sslib_refman.html || exit 1); \
	    fi; \
	done
	@for i in X $(SAF_HTML_DOCS); do \
	    if test $$i != X; then \
		($(LT_INSTALL_DATA) $$i $(docsdir)/refman.html || exit 1); \
	    fi; \
	done
	@for i in X $(EXP_HTML_DOCS); do \
	    if test $$i != X; then \
		($(LT_INSTALL_DATA) $$i $(docsdir)/examples.html || exit 1); \
	    fi; \
	done
	@for i in X $(SDIFF_HTML_DOCS); do \
	    if test $$i != X; then \
		($(LT_INSTALL_DATA) $$i $(docsdir)/safdiff.html || exit 1); \
	    fi; \
	done
	@for i in X $(ENSIGHT_HTML_DOCS); do \
	    if test $$i != X; then \
		($(LT_INSTALL_DATA) $$i $(docsdir)/ensight_reader.html || exit 1); \
	    fi; \
	done


## Removes those things that `make install' (would have) installed,
## from the top down.
.PHONY: uninstall
uninstall:
	@for f in X $(LIB); do \
	    if test $$f != X; then \
	        $(LT_UNINSTALL) $(libdir)/$$f; \
	    fi; \
	done
	@for f in X $(PUB_HDR); do \
	    if test $$f != X; then \
		:; $(RM) $(includedir)/`basename $$f`; \
	    fi; \
	done
	@for f in X $(TOOLS); do \
	    if test $$f != X; then \
	        $(LT_UNINSTALL) $(bindir)/$$f; \
	    fi; \
	done
	@for f in X $(TOOLS_SCRIPTS); do \
	    if test $$f != X; then \
		:; $(RM) $(bindir)/$$f; \
	    fi; \
	done
	@for f in X $(DATA); do \
	    if test $$f != X; then \
		:; $(RM) $(datadir)/$$f; \
	    fi; \
	done
	@for f in X $(CONFIG); do \
	    if test $$f != X; then \
		:; $(RM) $(sysconfdir)/$$f; \
	    fi; \
	done
	@for d in X $(SUBDIRS); do \
	    if test $$d != X; then \
		(:; cd $$d && $(MAKE) $@) || exit 1; \
	    fi; \
	done

## uninstall the documentation
.PHONY: uninstall-docs
uninstall-docs:
	@for d in X $(SUBDIRS); do \
	    if test $$d != X; then \
		(:; cd $$d && $(MAKE) $@) || exit 1; \
	    fi; \
	done
	@for f in X $(SAF_DOCUMENTS); do \
	    if test $$f != X; then \
		$(LT_UNINSTALL) $(docsdir)/$$f; \
	    fi; \
	done
	@for f in X $(SSLIB_DOCUMENTS); do \
	    if test $$f != X; then \
		$(LT_UNINSTALL) $(docsdir)/$$f; \
	    fi; \
	done
	@for f in X $(SSLIB_HTML_DOCS); do \
	    if test $$f != X; then \
		$(LT_UNINSTALL) $(docsdir)/$$f; \
	    fi; \
	done
	@for f in X $(SAF_HTML_DOCS); do \
	    if test $$f != X; then \
		$(LT_UNINSTALL) $(docsdir)/$$f; \
	    fi; \
	done
	@for f in X $(EXP_HTML_DOCS); do \
	    if test $$f != X; then \
		$(LT_UNINSTALL) $(docsdir)/$$f; \
	    fi; \
	done
	@for f in X $(SDIFF_HTML_DOCS); do \
	    if test $$f != X; then \
		$(LT_UNINSTALL) $(docsdir)/$$f; \
	    fi; \
	done
	@for f in X $(ENSIGHT_HTML_DOCS); do \
	    if test $$f != X; then \
		$(LT_UNINSTALL) $(docsdir)/$$f; \
	    fi; \
	done


## Removes temporary files without removing the final target files.  That is,
## remove things like object files but not libraries, executables, or data files.
.PHONY: mostlyclean
mostlyclean:
	@backwards= ; \
	for d in X $(SUBDIRS); do \
	    backwards="$$d $$backwards"; \
	done; \
	for d in $$backwards; do \
	    if test $$d != X; then \
		(:; cd $$d && $(MAKE) $@) || exit 1; \
	    fi; \
	done
	-$(RM) $(MOSTLYCLEAN)
	-$(RM) $(LIB_OBJ)  $(LIB_OBJ:.lo=.o)
	-$(RM) $(TEST_OBJ) $(TEST_OBJ:.lo=.o)
	-$(RM) $(TOOL_OBJ) $(TOOL_OBJ:.lo=.o)

## Like mostlyclean except also removes data files.
.PHONY: dataclean
dataclean:
	@backwards= ; \
	for d in X $(SUBDIRS); do \
	    backwards="$$d $$backwards"; \
	done; \
	for d in $$backwards; do \
	    if test $$d != X; then \
		(:; cd $$d && $(MAKE) $@) || exit 1; \
	    fi; \
	done
	-$(RM) $(MOSTLYCLEAN) $(DATACLEAN) $(DATA)
	-$(RM) $(LIB_OBJ)  $(LIB_OBJ:.lo=.o)
	-$(RM) $(TEST_OBJ) $(TEST_OBJ:.lo=.o)
	-$(RM) $(TOOL_OBJ) $(TOOL_OBJ:.lo=.o)

## Like `mostlyclean' except it also removes the final targets: things like
## libraries and executables.  This target doesn't remove any file that
## is part of the SAF distribution.
.PHONY: clean
clean:
	@backwards= ; \
	for d in X $(SUBDIRS); do \
	    backwards="$$d $$backwards"; \
	done; \
	for d in $$backwards; do \
	    if test $$d != X; then \
		(:; cd $$d && $(MAKE) $@) || exit 1; \
	    fi; \
	done
	-$(RM) $(MOSTLYCLEAN) $(DATACLEAN) $(DATA)
	-$(RM) -rf $(CLEAN)
	-$(RM) $(LIB_OBJ)  $(LIB_OBJ:.lo=.o)  $(LIB_OBJ:.lo=.d)
	-$(RM) $(TEST_OBJ) $(TEST_OBJ:.lo=.o) $(TEST_OBJ:.lo=.d) $(TEST_OBJ:.lo=.bb) $(TEST_OBJ:.lo=.bbg)
	-$(RM) $(TOOL_OBJ) $(TOOL_OBJ:.lo=.o) $(TOOL_OBJ:.lo=.d)
	-$(RM) $(LIB) $(TESTS) $(TOOLS) $(CHECKS)
	-$(RM) .lib_obj .lib_stamp core *.core core.*
	-$(RM) -r .libs 

## Like `clean' except it also removes files that were created by running
## configure.  If you've unpacked the source and built SAF without creating
## any other files, then `make distclean' will leave only the files that were
## in the distribution.
.PHONY: distclean
distclean:
	@backwards= ; \
	for d in X $(SUBDIRS); do \
	    backwards="$$d $$backwards" ;\
	done; \
	for d in $$backwards; do \
	    if test $$d != X; then \
		(:; cd $$d && $(MAKE) $@) || exit 1; \
	    fi; \
	done
	-$(RM) $(MOSTLYCLEAN) $(DATACLEAN) $(DATA) $(CLEAN) $(DISTCLEAN)
	-$(RM) $(LIB_OBJ)  $(LIB_OBJ:.lo=.o)  $(LIB_OBJ:.lo=.d)
	-$(RM) $(TEST_OBJ) $(TEST_OBJ:.lo=.o) $(TEST_OBJ:.lo=.d)
	-$(RM) $(TOOL_OBJ) $(TOOL_OBJ:.lo=.o) $(TOOL_OBJ:.lo=.d)
	-$(RM) $(LIB) $(TESTS) $(TOOLS) $(CHECKS)
	-$(RM) $(SSLIB_HTML_DOCS) $(SAF_HTML_DOCS) $(EXP_HTML_DOCS)
	-$(RM) $(SDIFF_HTML_DOCS) $(ENSIGHT_HTML_DOCS)
	-$(RM) $(SSLIB_MISC_DOCS) $(SAF_MISC_DOCS)
	-$(RM) $(SAF_DOCUMENTS) $(SSLIB_DOCUMENTS)
	-$(RM) .lib_obj .lib_stamp core *.core core.* TAGS *~ *.bak
	-$(RM) -r .libs
	@if test -f $(srcdir)/Makefile.in; then \
	    (set -x; $(RM) Makefile); \
	fi

## Like `distclean' except it deletes all files that can be regenerated from
## the makefile, including those generated from autoheader and autoconf.
.PHONY: maintainer-clean
maintainer-clean:
	@backwards= ; \
	for d in X $(SUBDIRS); do \
	    backwards="$$d $$backwards"; \
	done; \
	for d in $$backwards; do \
	    if test $$d != X; then \
		(:; cd $$d && $(MAKE) $@) || exit 1; \
	    fi; \
	done
	-$(RM) $(MOSTLYCLEAN) $(DATACLEAN) $(DATA) $(CLEAN) $(DISTCLEAN)
	-$(RM) $(LIB_OBJ)  $(LIB_OBJ:.lo=.o)  $(LIB_OBJ:.lo=.d)
	-$(RM) $(TEST_OBJ) $(TEST_OBJ:.lo=.o) $(TEST_OBJ:.lo=.d)
	-$(RM) $(TOOL_OBJ) $(TOOL_OBJ:.lo=.o) $(TOOL_OBJ:.lo=.d)
	-$(RM) $(LIB) $(TESTS) $(TOOLS) $(CHECKS)
	-$(RM) .lib_obj .lib_stamp core *.core core.* TAGS *~ *.bak
	-$(RM) -r .libs
	-$(RM) Dependencies $(MAINTAINER_CLEAN)
	@if test -f $(srcdir)/Makefile.in; then \
	    (set -x; $(RM) Makefile); \
	fi

## Implicit rules
.c.o:
	$(CC) $(CFLAGS) $(CPPFLAGS) -c $<

.c.lo:
	@$(LT_COMPILE) $(CFLAGS) $(CPPFLAGS) -c $<

.cc.o:
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -c $<

.cc.lo:
	@$(LT) --mode=compile $(CXX) $(CXXFLAGS) $(CPPFLAGS) -c $<

.java.class:
	$(JAVAC) $<

## Dependencies
.PHONY: dep depend
dep depend:
	@for d in X $(SUBDIRS); do \
	    if test $$d != X; then \
	        (:; cd $$d && $(MAKE) $@) || exit 1; \
	    fi; \
	done
	@$(MAKE) --no-print-directory dep_here

.PHONY: dep_here depend_here
dep_here depend_here: Dependencies
	cp Dependencies $(srcdir)/Dependencies

##                                                             -*- makefile -*-
## Copyright(C) 1999 The Regents of the University of California.
##     This work  was produced, in  part, at the  University of California, Lawrence Livermore National
##     Laboratory    (UC LLNL)  under    contract number   W-7405-ENG-48 (Contract    48)   between the
##     U.S. Department of Energy (DOE) and The Regents of the University of California (University) for
##     the  operation of UC LLNL.  Copyright  is reserved to  the University for purposes of controlled
##     dissemination, commercialization  through formal licensing, or other  disposition under terms of
##     Contract 48; DOE policies, regulations and orders; and U.S. statutes.  The rights of the Federal
##     Government  are reserved under  Contract 48 subject  to the restrictions agreed  upon by DOE and
##     University.
## 
## Copyright(C) 1999 Sandia Corporation.
##     Under terms of Contract DE-AC04-94AL85000, there is a non-exclusive license for use of this work
##     on behalf of the U.S. Government.  Export  of this program may require a license from the United
##     States Government.
## 
## Disclaimer:
##     This document was  prepared as an account of  work sponsored by an agency  of  the United States
##     Government. Neither the United States  Government nor the United States Department of Energy nor 
##     the  University  of  California  nor  Sandia  Corporation nor any  of their employees  makes any 
##     warranty, expressed  or  implied, or  assumes   any  legal liability  or responsibility  for the 
##     accuracy,  completeness,  or  usefulness  of  any  information, apparatus,  product,  or process 
##     disclosed,  or  represents that its  use would   not infringe  privately owned rights. Reference 
##     herein  to any  specific commercial  product,  process,  or  service by  trade  name, trademark, 
##     manufacturer,  or  otherwise,  does  not   necessarily  constitute  or  imply  its  endorsement, 
##     recommendation, or favoring by the  United States Government   or the University of  California.  
##     The views and opinions of authors expressed herein do not necessarily state  or reflect those of
##     the  United  States Government or  the   University of California   and shall  not be  used  for
##     advertising or product endorsement purposes.
## 
## Authors:
## 	William J. Arrighi	LLNL
## 	Peter K. Espen		SNL
## 	Ray T. Hitt 		SNL
## 	Robb P. Matzke 		LLNL
## 	Mark C. Miller 		LLNL
## 	James F. Reus 		LLNL
## 	Larry A. Schoof 	SNL
## 
## Acknowledgements:
## 	Marty L. Barnaby	SNL - Red parallel perf. study/tuning
## 	David M. Butler		LPS - Data model design/implementation Spec.
## 	Albert K. Cheng		NCSA - Parallel HDF5 support
## 	Nancy Collins		IBM - Alpha/Beta user
## 	Linnea M. Cook		LLNL - Management advocate
## 	Michael J. Folk		NCSA - Management advocate 
## 	Richard M. Hedges	LLNL - Blue-Pacific parallel perf. study/tuning 
## 	Quincey A. Koziol	NCSA - Serial HDF5 Support 
## 	Celeste M. Matarazzo	LLNL - Management advocate
## 	Tom H. Robey 		SNL - Early developer
## 	Greg D. Sjaardema	SNL - Alpha/Beta user
## 	Reinhard W. Stotzer	SNL - Early developer
## 	Judy Sturtevant		SNL - Red parallel perf. study/tuning 
## 	Robert K. Yates		LLNL - Blue-Pacific parallel perf. study/tuning
## 

## We keep a list of dependencies in *.d for each of the source files.
## The `*.d' files are generated on the fly in the build directory and
## used as the dependencies. The Dependencies files are generated at the same
## time but stored in the source directory. Although `*.d' includes all
## header file dependencies, `Dependencies' is pruned by removing the system
## include files and the paths for the remaining include files, making the
## dependencies suitable for distribution.
##
## We must be careful about building `Dependencies' because the source tree
## might not be writable.
##
depfiles=$(sort $(filter %.d, $(LIB_SRC:.c=.d) $(LIB_SRC:.cc=.d)              \
			      $(TEST_SRC:.c=.d) $(TEST_SRC:.cc=.d)            \
			      $(TOOL_SRC:.c=.d) $(TOOL_SRC:.cc=.d)))

%.d: %.c
	@echo Building dependencies for $<
	@$(CC) -M -MG $(CPPFLAGS) $< 2>/dev/null |			      \
	    sed 's%\($*\)\.o[ :]*%\1.lo: %g' |				      \
	    sed 's% $(srcdir)/% $$(srcdir)/%g' |			      \
	    sed 's% $(top_srcdir)/% $$(top_srcdir)/%g' |		      \
	    sed 's% $(top_builddir)/% $$(top_builddir)/%g' >$@
	@[ -s $@ ] || $(RM) $@

%.d: %.cc
	@echo Building dependencies for $<
	@$(CC) -M -MG $(CPPFLAGS) $< 2>/dev/null |			      \
	    sed 's%\($*\)\.o[ :]*%\1.lo: %g' |				      \
	    sed 's% $(srcdir)/% $$(srcdir)/%g' |			      \
	    sed 's% $(top_srcdir)/% $$(top_srcdir)/%g' |		      \
	    sed 's% $(top_builddir)/% $$(top_builddir)/%g' >$@
	@[ -s $@ ] || $(RM) $@

Dependencies: $(depfiles)
	@if test '$(srcdir)' != '.'; then				      \
	    echo Building $@;						      \
	    echo '## This file is machine generated on GNU systems.' >$@;     \
	    echo '## Only temporary changes may be made here' >>$@;	      \
	    echo >>$@;							      \
	    perl -p $(top_srcdir)/config/distdep /dev/null $^ >>$@;	      \
	else                                                                  \
	    echo 'Dependencies cannot be built when $$srcdir == $$builddir';  \
	fi

-include Dependencies
