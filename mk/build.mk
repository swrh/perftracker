# build.mk
#
# Copyright (c) 2005-2010 Fernando Silveira <fsilveira@gmail.com>
# All rights reserved.
# 
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
# 1. Redistributions of source code must retain the above copyright
#    notice, this list of conditions and the following disclaimer.
# 2. Redistributions in binary form must reproduce the above copyright
#    notice, this list of conditions and the following disclaimer in the
#    documentation and/or other materials provided with the distribution.
# 3. All advertising materials mentioning features or use of this software
#    must display the following acknowledgement:
#      This product includes software developed by Fernando Silveira.
# 4. The name of the author may not be used to endorse or promote products
#    derived from this software without specific prior written permission.
# 
# THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
# IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
# WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
# DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE FOR
# ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
# DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
# OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
# HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
# STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
# ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
# POSSIBILITY OF SUCH DAMAGE.

ifndef build.mk
build.mk=		y

ifndef DOTMKDIR
	DOTMKDIR=		$(dir $(lastword $(MAKEFILE_LIST)))

	dotmk_tmp:=		$(shell [ ! -d $(DOTMKDIR) ] || echo $(DOTMKDIR))
	ifeq ($(dotmk_tmp),)
		dotmk_tmp:=	$(error Couldn't determine DOTMKDIR variable automatically)
	endif

	ifeq ($(findstring else-if,$(.FEATURES)),)
		dotmk_tmp:=	$(error You need a newer version of GNU make to use `dotmk')
	endif

endif

ifneq ($(wildcard $(DOTMKDIR)/build-local.mk),)
	include $(DOTMKDIR)/build-local.mk
endif


ifdef DEBUG
	ifeq ($(DEBUG),)
		dotmk_DEBUG=n
	else ifeq ($(DEBUG),n)
		dotmk_DEBUG=n
	else ifeq ($(DEBUG),no)
		dotmk_DEBUG=n
	else ifeq ($(DEBUG),N)
		dotmk_DEBUG=n
	else ifeq ($(DEBUG),NO)
		dotmk_DEBUG=n
	else
		dotmk_DEBUG=y
	endif
else
	dotmk_DEBUG=n
endif

# C compiler.
CFLAGS+=		-Wall -Wunused
ifeq ($(dotmk_DEBUG),y)
CFLAGS+=		-O0 -ggdb3 -DDEBUG
else
CFLAGS+=		-O2 -DNDEBUG
endif

# C++ compiler.
CXXFLAGS+=		-Wall -Wunused
ifeq ($(dotmk_DEBUG),y)
CXXFLAGS+=		-O0 -ggdb3 -DDEBUG
else
CXXFLAGS+=		-O2 -DNDEBUG
endif

# Linker.
#ifeq ($(dotmk_DEBUG),y)
#LDFLAGS+=		
#else
#LDFLAGS+=		
#endif

# Archiver.
ARFLAGS=		rcs

# Commands.
AR=			$(CROSS_COMPILE)ar
AWK=			awk
CC=			$(CROSS_COMPILE)gcc
CTAGS=			ctags
CXX=			$(CROSS_COMPILE)g++
INSTALL=		install
MKDEP=			mkdep
RM=			rm -f

# Directories.
PREFIX=			/usr/local

LINK.o=			$(CC)

# Empty goals.
.PHONY: no not empty null
no not empty null:


# global variables

ifdef LIB
LIBS+=				$(LIB)
ifdef SRCS
$(notdir $(LIB))_SRCS+=		$(SRCS)
endif
ifdef OBJS
$(notdir $(LIB))_OBJS+=		$(OBJS)
endif
endif

ifdef PROG
PROGS+=				$(PROG)
ifdef SRCS
$(notdir $(PROG))_SRCS+=	$(SRCS)
endif
ifdef OBJS
$(notdir $(PROG))_OBJS+=	$(OBJS)
endif
endif

ifdef DISABLE_TARGET
DISABLE_TARGETS+=		$(DISABLE_TARGET)
endif


# binaries

define BIN_template
$(notdir $(1))_LINKER=		$(CC)

ifneq ($(OBJDIR),)
$(notdir $(1))_OBJPREFIX=	$(OBJDIR)/
endif

MKDEPARGS+=			$$($(notdir $(1))_CXXFLAGS)

define BIN_SRC_template

# Read ".depend" file to append dependencies to each object target.
ifneq ($(wildcard .depend),)
	# Using $(MAKE) to read file dependencies is VEEEEEEEEEEERY slow. Please use $(AWK).
	#$$(1)_depend=	$$$$(shell OBJ="$$(notdir $$$$(patsubst %.cpp,%.o,$$$$(1)))"; echo -e ".PHONY: $$$$$$$${OBJ}\\n$$$$$$$${OBJ}:\\n\\t@echo $$$$$$$$^\\n" | make -f - -f .depend)
	# Faster, but less compatible.
	$$(1)_depend=	$$$$(shell exec $(AWK) -v OBJ=$$(notdir $$(patsubst %.cpp,%.o,$$(1))) '{ if (/^[^ \t]/) obj = 0; if ($$$$$$$$1 == OBJ":") { obj = 1; $$$$$$$$1 = ""; } else if (!obj) next; if (/\\$$$$$$$$/) sub(/\\$$$$$$$$/, " "); else sub(/$$$$$$$$/, "\n"); printf("%s", $$$$$$$$0); }' .depend)
endif

ifneq ($$(1),$$(patsubst %.cpp,%.o,$$(1)))

$(notdir $(1))_LINKER=		$(CXX)

MKDEPARGS+=		$$($$(1)_CXXFLAGS)

ifndef $$(1)_CXXFLAGS
$$(1)_CXXFLAGS+=	$$($(notdir $(1))_CXXFLAGS) $$(CXXFLAGS)
endif

# avoid defining a target more than one time
ifneq ($$$$(_$$(1)),x)
$$($(notdir $(1))_OBJPREFIX)$$(patsubst %.cpp,%.o,$$(1)): $$(1) $$$$($$(1)_DEPS) $$$$($$(1)_depend)
	@[ -d $$$$(dir $$$$@) ] || { echo 'mkdir -pv $$$$(dir $$$$@)'; mkdir -pv $$$$(dir $$$$@); }
	$(CXX) $$$$($$(1)_CXXFLAGS) $$($(notdir $(1))_INCDIRS:%=-I%) $$(INCDIRS:%=-I%) -c -o $$$$@ $$$$<
endif

$(notdir $(1))_OBJS+=		$$($(notdir $(1))_OBJPREFIX)$$(patsubst %.cpp,%.o,$$(1))
$(notdir $(1))_CLEANFILES+=	$$($(notdir $(1))_OBJPREFIX)$$(patsubst %.cpp,%.o,$$(1))

CXX_SRCS+=		$$(1)

else

ifndef $$(1)_CFLAGS
$$(1)_CFLAGS+=		$$($(notdir $(1))_CFLAGS) $$(CFLAGS)
endif

# avoid defining a target more than one time
ifneq ($$$$(_$$(1)),x)
$$($(notdir $(1))_OBJPREFIX)$$(patsubst %.c,%.o,$$(1)): $$(1) $$$$($$(1)_DEPS) $$$$($$(1)_depend)
	@[ -d $$$$(dir $$$$@) ] || { echo 'mkdir -pv $$$$(dir $$$$@)'; mkdir -pv $$$$(dir $$$$@); }
	$(CC) $$$$($$(1)_CFLAGS) $$($(notdir $(1))_INCDIRS:%=-I%) $$(INCDIRS:%=-I%) -c -o $$$$@ $$$$<
endif

$(notdir $(1))_OBJS+=		$$($(notdir $(1))_OBJPREFIX)$$(patsubst %.c,%.o,$$(1))
$(notdir $(1))_CLEANFILES+=	$$($(notdir $(1))_OBJPREFIX)$$(patsubst %.c,%.o,$$(1))

C_SRCS+=		$$(1)

endif

_$$(1)=			x

endef

ifndef $(1)_SRCS
ifneq ($(wildcard $(1).cpp),)
$(1)_SRCS=		$(1).cpp
else ifneq ($(wildcard $(1).c),)
$(1)_SRCS=		$(1).c
else ifneq ($(wildcard main.cpp),)
$(1)_SRCS=		main.cpp
else ifneq ($(wildcard main.c),)
$(1)_SRCS=		main.c
endif
endif

$$(foreach src,$$($(notdir $(1))_SRCS),$$(eval $$(call BIN_SRC_template,$$(src))))

MKDEPARGS+=		$$($(notdir $(1))_INCDIRS:%=-I%)
CTAGSARGS+=		$$($(notdir $(1))_INCDIRS)

endef

$(foreach bin,$(LIBS) $(PROGS),$(eval $(call BIN_template,$(bin))))

MKDEPARGS+=		$(INCDIRS:%=-I%)
CTAGSARGS+=		$(INCDIRS)

# libraries

define LIB_template
ifeq ($$(dir $(1)),./)
$(notdir $(1))_LIBFILE:=		lib$$(notdir $(1)).so
else
$(notdir $(1))_LIBFILE:=		$$(dir $(1))lib$$(notdir $(1)).so
endif

ifeq ($$($(notdir $(1))_INSTDIR),)
$(notdir $(1))_INSTDIR=		$(if $(INSTDIR),$(INSTDIR),$(PREFIX)/lib)
endif

ifeq ($$(subst -rpath,,$$($(notdir $(1))_LDFLAGS) $$(LDFLAGS)),$$($(notdir $(1))_LDFLAGS) $$(LDFLAGS))
$(notdir $(1))_LDFLAGS+=		-Wl,-rpath,$$($(notdir $(1))_INSTDIR)
endif

DEFAULT_TARGETS+=	$$($(notdir $(1))_LIBFILE)
$$($(notdir $(1))_LIBFILE): $$($(notdir $(1))_OBJS)
	@[ -d $$(dir $$@) ] || { echo 'mkdir -pv $$(dir $$@)'; mkdir -pv $$(dir $$@); }
	$$($(notdir $(1))_LINKER) $$^ $$($(notdir $(1))_LIBDIRS:%=-L%) $$(foreach lib,$$($(notdir $(1))_DEPLIBS),$$(if $$(wildcard $$(lib)),$$(lib),-l$$(lib))) -shared $$($(notdir $(1))_LDFLAGS) $$(LIBDIRS:%=-L%) $$(foreach lib,$$(DEPLIBS),$$(if $$(wildcard $$(lib)),$$(lib),-l$$(lib))) $$(LDFLAGS) $$(LDLIBS) -o $$@

$(1): $$($(notdir $(1))_LIBFILE)

CLEAN_TARGETS+=		$(notdir $(1))_clean
$(notdir $(1))_clean:
	$(RM) $$($(notdir $(1))_LIBFILE) $$($(notdir $(1))_CLEANFILES)

INSTALL_TARGETS+=	$(notdir $(1))_install
$(notdir $(1))_install: $$($(notdir $(1))_LIBFILE)
	$(INSTALL) -c -D $$($(notdir $(1))_LIBFILE) $$($(notdir $(1))_INSTDIR)/$$(notdir $$($(notdir $(1))_LIBFILE))

UNINSTALL_TARGETS+=	$(notdir $(1))_uninstall
$(notdir $(1))_uninstall:
	$(RM) $$($(notdir $(1))_INSTDIR)/$$(notdir $$($(notdir $(1))_LIBFILE))
endef

$(foreach lib,$(LIBS),$(eval $(call LIB_template,$(lib))))


# programs

define PROG_template
ifeq ($$($(notdir $(1))_INSTDIR),)
$(notdir $(1))_INSTDIR=		$(if $(INSTDIR),$(INSTDIR),$(PREFIX)/bin)
endif

DEFAULT_TARGETS+=	$(1)
$(1): $$($(notdir $(1))_OBJS)
	@[ -d $$(dir $$@) ] || { echo 'mkdir -pv $$(dir $$@)'; mkdir -pv $$(dir $$@); }
	$$($(notdir $(1))_LINKER) $$^ $$($(notdir $(1))_LIBDIRS:%=-L%) $$(foreach lib,$$($(notdir $(1))_DEPLIBS),$$(if $$(wildcard $$(lib)),$$(lib),-l$$(lib))) $$($(notdir $(1))_LDFLAGS) $$(LIBDIRS:%=-L%) $$(foreach lib,$$(DEPLIBS),$$(if $$(wildcard $$(lib)),$$(lib),-l$$(lib))) $$(LDFLAGS) $$(LDLIBS) -o $$@

CLEAN_TARGETS+=	$(notdir $(1))_clean
$(notdir $(1))_clean:
	$(RM) $(1) $$($(notdir $(1))_CLEANFILES)

INSTALL_TARGETS+=	$(notdir $(1))_install
$(notdir $(1))_install: $(1)
	$(INSTALL) -c -D $(1) $$($(notdir $(1))_INSTDIR)/$$(notdir $(1))

UNINSTALL_TARGETS+=	$(notdir $(1))_uninstall
$(notdir $(1))_uninstall:
	$(RM) $$($(notdir $(1))_INSTDIR)/$$(notdir $(1))
endef

$(foreach prog,$(PROGS),$(eval $(call PROG_template,$(prog))))


# rules

ifneq ($(CXX_SRCS),)
MKDEPARGS+=		$(CXXFLAGS)
else ifneq ($(C_SRCS),)
MKDEPARGS+=		$(CFLAGS)
endif
MKDEPARGS+=		$(CXX_SRCS) $(C_SRCS)

.PHONY: dep
dep:
	$(MKDEP) $(MKDEPARGS)

DISTCLEAN_TARGETS+=	$(CLEAN_TARGETS)

DISTCLEAN_TARGETS+=	dep_distclean
.PHONY: dep_distclean
dep_distclean:
	$(RM) .depend

ifdef DISTCLEANFILES
DISTCLEAN_TARGETS+=	files_distclean
.PHONY: files_distclean
files_distclean:
	$(RM) -r $(DISTCLEANFILES)
endif

.PHONY: tags
tags:
	$(CTAGS) -R $(CTAGSARGS) $(C_SRCS) $(CXX_SRCS)

.PHONY: all install uninstall clean distclean
all: $(filter-out $(DISABLE_TARGETS),$(DEFAULT_TARGETS))
ifeq ($(filter install,$(DISABLE_TARGETS)),)
install: $(filter-out $(DISABLE_TARGETS),$(INSTALL_TARGETS))
endif
ifeq ($(filter uninstall,$(DISABLE_TARGETS)),)
uninstall: $(filter-out $(DISABLE_TARGETS),$(UNINSTALL_TARGETS))
endif
ifeq ($(filter clean,$(DISABLE_TARGETS)),)
clean: $(filter-out $(DISABLE_TARGETS),$(CLEAN_TARGETS))
endif
ifeq ($(filter distclean,$(DISABLE_TARGETS)),)
distclean: $(filter-out $(DISABLE_TARGETS),$(DISTCLEAN_TARGETS))
endif

.DEFAULT_GOAL:=		all

endif # ndef build.mk
