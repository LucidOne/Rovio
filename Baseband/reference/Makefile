###############################################################################
#
# Makefile that can supports build, distribution, re-create Makefile and
# .mcp (CodeWarrior) from "project.cfg".
#
###############################################################################


#Stock varible for config file, do not edit it!
PROJECT_CONFIG := project.cfg
PROJECT_DIR := $(notdir $(shell pwd))
PROJECT_PATH := $(shell pwd)




###############################################################################
#Configuration of this Makefile.
###############################################################################

PROJECT_NAME :=\
	project_name


LINK_SOURCE :=


DIST_BASE_DIR :=\
	..


DIST_FILE :=\
	.\
	./reference/$(THIS_MAKEFILE)


NO_DIST_FILE :=\
	.*\\.o\
	.*_Data\
	.*/Thumbs.db


SUB_PROJECT :=











###############################################################################
#Make environment.
###############################################################################
LIBS := $(LIBS) -lm -lc -lgcc -lc


.SUFFIXES : .x .o .c .s

BUILD_ROOT := /usr/local/arm_semihost/
LIB := ${BUILD_ROOT}arm-elf/lib
LIB1 := ${BUILD_ROOT}lib/gcc-lib/arm-elf/3.0/
LIB2 := ${BUILD_ROOT}lib/gcc-lib/arm-elf/3.3.2/

INCLUDE := ${BUILD_ROOT}arm-elf/include


PATH := ${BUILD_ROOT}bin:${PATH}
CFLAGS := -I${INCLUDE} -I. -fshort-wchar -mcpu=arm7tdmi -O2 -Wall -DPLATFORM_GCC_W90N740
CC := arm-elf-gcc
CPP := arm-elf-g++
AR := arm-elf-ar
AS := arm-elf-as
LD := arm-elf-ld -zo
OC  = arm-elf-objcopy

WEC_LDFLAGS := -L. -L${LIB} -L${LIB1} -L${LIB2} -T../../Samples/armelf.lds





###############################################################################
#Runtime data and escaped strings in shell
###############################################################################

THIS_MAKEFILE := $(word 1, $(MAKEFILE_LIST))

#If project name not found, use the directory name instead.
ifndef PROJECT_NAME
REAL_PROJECT_NAME := $(notdir $(shell pwd))
else
REAL_PROJECT_NAME := $(PROJECT_NAME)
endif


#Objects
LINK_OBJECT := $(patsubst %.c,%.o,\
				$(patsubst %.cpp,%.o,\
				$(patsubst %.c++,%.o,\
				$(patsubst %.cc,%.o,\
				$(LINK_SOURCE)))))


#Escape function for shell
sh_escape_parenthesis_l=(
sh_escape_parenthesis_r=)
sh_escape = $(subst\
			|,\|,$(subst\
			$(sh_escape_parenthesis_l),\$(sh_escape_parenthesis_l),$(subst\
			$(sh_escape_parenthesis_r),\$(sh_escape_parenthesis_r),$(subst\
			\#,\\\#,$(subst\
			>,\>,$(subst\
			<,\<,$(subst\
			\\,\,$(subst\
			\b\,b,$(subst\
			\a\,a,$(subst\
			b',"'",$(subst\
			a",'"',$(subst\
			',b',$(subst\
			",a",$(subst\
			b,\b\,$(subst\
			a,\a\,$(subst\
			\,\\,$(1)))))))))))))))))


#Shell-escaped strings
ESC_THIS_MAKEFILE := $(call sh_escape,$(THIS_MAKEFILE))
ESC_PROJECT_NAME := $(call sh_escape,$(REAL_PROJECT_NAME))
ESC_LINK_SOURCE := $(call sh_escape,$(LINK_SOURCE))
ESC_LINK_OBJECT := $(call sh_escape,$(LINK_OBJECT))
ESC_DIST_BASE_DIR := $(call sh_escape,$(DIST_BASE_DIR))
ESC_DIST_FILE := $(call sh_escape,$(DIST_FILE))
ESC_NO_DIST_FILE := $(call sh_escape,$(NO_DIST_FILE))
ESC_SUB_PROJECT := $(call sh_escape,$(SUB_PROJECT))
ESC_PROJECT_CONFIG := $(call sh_escape,$(PROJECT_CONFIG))
ESC_PROJECT_DIR := $(call sh_escape,$(PROJECT_DIR))
ESC_PROJECT_PATH := $(call sh_escape,$(PROJECT_PATH))
LIBS := $(call sh_escape,$(LIBS))
CFLAGS := $(call sh_escape,$(CFLAGS))
CC := $(call sh_escape,$(CC))
CPP := $(call sh_escape,$(CPP))
AR := $(call sh_escape,$(AR))
AS := $(call sh_escape,$(AS))
LD := $(call sh_escape,$(LD))
OC := $(call sh_escape,$(OC))




###############################################################################
#Targets start from here.
###############################################################################

.PHONY: default
default: help



.PHONY: build
ifdef LINK_SOURCE
build: proj_all_Makefile $(LINK_OBJECT) $(REAL_PROJECT_NAME).bin
else
build: proj_all_Makefile $(LINK_OBJECT)
endif
	@\
	for i in $(ESC_SUB_PROJECT); do \
		make -C "$$i" build; \
	done; \


ifdef LINK_SOURCE
$(REAL_PROJECT_NAME).bin: $(LINK_OBJECT)
	@echo Make application.
	$(LD) -o $(ESC_PROJECT_NAME).elf $(WEC_LDFLAGS) $(ESC_LINK_OBJECT) $(LIBS)
	$(OC) -I elf32-littlearm -O binary $(ESC_PROJECT_NAME).elf $(ESC_PROJECT_NAME).bin
	rm -f $(ESC_PROJECT_NAME).elf
endif


.c.o:
	$(CC) $(CFLAGS) -c -o "$*.o" "$*.c"

.cc.o:
	$(CPP) $(CFLAGS) -c -o "$*.o" "$*.cc"

.cpp.o:
	$(CPP) -c -o "$*.o" "$*.cpp"

.cxx.o:
	$(CPP) -c -o "$*.o" "$*.c++"

.s.o:
	$(AS) -ahld -o "$*.o" "$*.s"



.PHONY: clean
clean: proj_all_Makefile
ifdef LINK_SOURCE
	rm -f $(ESC_LINK_OBJECT)
	rm -f $(ESC_PROJECT_NAME).x
	rm -f $(ESC_PROJECT_NAME).elf
	rm -f $(ESC_PROJECT_NAME).bin
	rm -f $(ESC_PROJECT_NAME).zip
endif
	@\
	for i in $(ESC_SUB_PROJECT); do \
		make -C "$$i" clean; \
	done; \



.PHONY: proj
proj: proj_all_Makefile $(REAL_PROJECT_NAME).mcp
	@for i in $(ESC_SUB_PROJECT); do \
		make -C "$$i" proj; \
	done; \


.PHONY: proj_all_Makefile
proj_all_Makefile: $(THIS_MAKEFILE)
	@for i in $(ESC_SUB_PROJECT); do \
		if [ ! -f "$$i/Makefile" ]; then \
			cp "./$(ESC_THIS_MAKEFILE)" "$$i/Makefile"; \
			make -C "$$i" force_Makefile; \
		fi; \
	done; \


.PHONY: force_Makefile
$(THIS_MAKEFILE) force_Makefile: $(PROJECT_CONFIG)
	@\
	echo "Re-create $(ESC_THIS_MAKEFILE) from "$(ESC_PROJECT_CONFIG)"."; \
	if ! awk --version > /dev/null; then exit 1; fi; \
	s_tmp_makefile="/tmp/$(ESC_THIS_MAKEFILE).new"; \
	awk -v head="PROJECT_NAME=LINK_SOURCE=DIST_BASE_DIR=DIST_FILE=NO_DIST_FILE=SUB_PROJECT" \
		'{\
			split(head, ahead, /=/); \
			for (i in ahead) \
			{ \
				if ($$0 ~ "^[[:space:]]*"ahead[i]) \
				{ \
					while ($$0 ~ /\\$$/) \
					{ \
						getline; \
					} \
					getline; \
					if (!b_insert[i]) \
					{ \
						printf "%s :=", ahead[i]; \
						b_insert[i] = 1; \
						cfg_reg = "^[[:space:]]*"ahead[i]"(_GCC)?[[:space:]]*="; \
						while (getline cfg_line < "'$(ESC_PROJECT_CONFIG)'") \
						{ \
							if (cfg_line ~ cfg_reg) \
							{ \
								sub(cfg_reg, "", cfg_line); \
								gsub(/(^[[:space:]]*|[[:space:]]*$$)/, "", cfg_line); \
								gsub(/[[:space:]#\\]/, "\\\\&", cfg_line); \
								printf "\\\n	%s", cfg_line; \
							} \
						} \
						print ""; \
						close ("'$(ESC_PROJECT_CONFIG)'") \
					} \
				} \
			} \
			print; \
		}' "$(ESC_THIS_MAKEFILE)" > "$$s_tmp_makefile"; \
	cp "$$s_tmp_makefile" "$(ESC_THIS_MAKEFILE)"; \
	rm -f "$$s_tmp_makefile"; \
	{ make -f "$(ESC_THIS_MAKEFILE)" depend || :; } 2> /dev/null; \



$(REAL_PROJECT_NAME).mcp: $(PROJECT_CONFIG)
	@\
	echo "Re-create "$(ESC_PROJECT_NAME)".mcp since "$(ESC_PROJECT_CONFIG)" has already been changed."; \
	s_SrcBase=$(ESC_DIST_BASE_DIR); \
	while [ "$${s_SrcBase%%[/\\]}" != "$$s_SrcBase" ]; do \
		s_SrcBase="$${s_SrcBase%%[/\\]}"; \
	done; \
	if [ ! -f "$$s_SrcBase/reference/create_project.js" ]; then \
		echo "Can not found script for creating ADS project file!" 1>&2; \
	else \
		cscript "$$s_SrcBase/reference/create_project.js" -ads $(ESC_PROJECT_CONFIG); \
	fi; \



.PHONY: projclean
projclean:
	@for i in $(ESC_SUB_PROJECT); do \
		if [ ! -f "$$i/Makefile" ]; then \
			cp "$(ESC_THIS_MAKEFILE)" "$$i/Makefile"; \
			make -C "$$i" force_Makefile; \
		fi; \
		make -C "$$i" projclean; \
	done; \
	rm -rf $(ESC_PROJECT_NAME).mcp; \
	rm -rf $(ESC_PROJECT_NAME)_Data; \
	mv "$(ESC_THIS_MAKEFILE)" Makefile.bak; \



.PHONY: dist
dist:
	@\
	echo "Please try:"; \
	echo "    make dist.zip"; \
	echo "    make dist.tar"; \
	echo "    make dist.tar.gz"; \
	echo "    make dist.tar.bz2"; \
	echo "    make dist.7z"; \
	echo "    make dist.rar";



.PHONY: distclean
distclean:
	@\
	s_DistNamePattern="_[0-9yY][0-9yY][0-9yY][0-9yY]-[0-9mM][0-9mM]-[0-9dD][0-9dD]_[0-9hH][0-9hH].[0-9mM][0-9mM]"; \
	for i in zip tar tar.gz tar.bz2 7z rar; do \
		echo "Try to rm" $(ESC_PROJECT_NAME)$$s_DistNamePattern".$$i"; \
		rm -f $(ESC_PROJECT_NAME)$$s_DistNamePattern".$$i"; \
	done



.PHONY: dist.zip
dist.zip:
	@make -f "$(ESC_THIS_MAKEFILE)" assist_dist R_EXT=zip R_TOOL=zip "R_TOOL_PARAM=-r -9"



.PHONY: dist.tar
dist.tar:
	@make -f "$(ESC_THIS_MAKEFILE)" assist_dist R_EXT=tar R_TOOL=tar "R_TOOL_PARAM=cvf"



.PHONY: dist.tar.gz
dist.tar.gz:
	@make -f "$(ESC_THIS_MAKEFILE)" assist_dist R_EXT=tar.gz R_TOOL=tar "R_TOOL_PARAM=cvzf"



.PHONY: dist.tar.bz2
dist.tar.bz2:
	@make -f "$(ESC_THIS_MAKEFILE)" assist_dist R_EXT=tar.bz2 R_TOOL=tar "R_TOOL_PARAM=cvjf"



.PHONY: dist.7z
dist.7z:
	@export rtool="$$(cygpath "$$(regtool get '\HKLM\SOFTWARE\7-Zip\Path')")/7z"; \
	make -f "$(ESC_THIS_MAKEFILE)" assist_dist R_EXT=7z "R_TOOL=$$rtool" "R_TOOL_PARAM=a -t7z -r -mx=9 -ms=on -mmt=on"



.PHONY: dist.rar
dist.rar:
	@export rtool="$$(regtool get '\HKLM\SOFTWARE\Classes\WinRAR\shell\open\command\')"; \
	rtool="$${rtool% *}"; \
	rtool="$${rtool#\"}"; \
	rtool="$${rtool%\\*.exe\"}"; \
	rtool="$${rtool}\\rar"; \
	rtool="$$(cygpath "$$rtool")"; \
	make -f "$(ESC_THIS_MAKEFILE)" assist_dist R_EXT=rar "R_TOOL=$$rtool" "R_TOOL_PARAM=a -m5 -r -s"



.PHONY: assist_dist
assist_dist:
	@echo Build distribution: $(ESC_PROJECT_NAME)".yyyy-mm-dd_hh.mm.$(R_EXT)".
	@s_RTool=`which ${R_TOOL} 2>/dev/null`; \
	if [ "$${s_RTool}" = "" ]; \
		then if [ ! -x "${R_TOOL}" ]; then echo "${R_TOOL} not found!" 1>&2; exit 1; fi; \
	fi; \
	if ! awk --version > /dev/null; then exit 11; fi; \
	s_SrcDir="$$(pwd)"; \
	:; \
	echo ; \
	echo "Make a temporary copy, waiting..."; \
	s_DistDir="/tmp/"$(ESC_PROJECT_NAME); \
	while [ "$${s_DistDir%%[/\\]}" != "$$s_DistDir" ]; do \
		s_DistDir="$${s_DistDir%%[/\\]}"; \
	done; \
	if [ "$$s_DistDir" = "/tmp" ]; then echo "ERROR dir: $$s_DistDir" 1>&2; exit 2; fi; \
	echo "$$s_DistDir"; \
	rm -rf "$$s_DistDir"; \
	if ! mkdir "$$s_DistDir"; then echo "ERROR: can not make temporary dir: $$s_DistDir" 1>&2; exit 3; fi; \
	s_SrcBase=$(ESC_DIST_BASE_DIR); \
	while [ "$${s_SrcBase%%[/\\]}" != "$$s_SrcBase" ]; do \
		s_SrcBase="$${s_SrcBase%%[/\\]}"; \
	done; \
	echo "$$s_SrcBase"; \
	s_Makefile_path=""; \
	for i in $(ESC_DIST_FILE); do \
		while [ "$${i%%[/\\]}" != "$$i" ]; do \
			i="$${i%%[/\\]}"; \
		done; \
		s_from="$$s_SrcBase/$$i"; \
		s_to="$$s_DistDir/$$i"; \
		s_topath="$${s_to%[/\\]*}"; \
		if [ -d "$$s_from" ]; then \
			if ! mkdir -p "$$s_to"; then exit 4; fi; \
			cp -p -r -f "$$s_from/"* "$$s_to"; \
		else \
			if ! mkdir -p "$$s_topath"; then exit 5; fi; \
			cp -p -r -f "$$s_from" "$$s_to"; \
		fi; \
		if [ -d "$$s_from" ] && [ "$$(cygpath -a "$$s_from")" = $(ESC_PROJECT_PATH) ]; then \
			s_Makefile_path="$$i"; \
		elif [ -f "$$s_from" ] && [ "$$(cygpath -a "$${s_from%[/\\]*}")" = $(ESC_PROJECT_PATH) ]; then \
			s_Makefile_path="$${i%[/\\]*}"; \
		fi; \
	done; \
	if [ "$$s_Makefile_path" = "" ]; then s_Makefile_path="."; fi; \
	echo "Verify Makefile path: $$s_DistDir/$$s_Makefile_path/$(ESC_THIS_MAKEFILE)"; \
	if [ ! -f "$$s_DistDir/$$s_Makefile_path/$(ESC_THIS_MAKEFILE)" ]; then \
		echo "ERROR: $(ESC_THIS_MAKEFILE) not found after make a copy." 1>&2; \
		exit 6; \
	fi; \
	:; \
	echo ; \
	echo "Enter temporary directory."; \
	if ! cd "$$s_DistDir"; then exit 7; fi; \
	echo "Synchronize project files (Makefile, .mcp, etc)."; \
	make -C "$$s_Makefile_path" -f "$(ESC_THIS_MAKEFILE)" proj; \
	:; \
	echo ; \
	echo "Clean compiling results for project and sub-projects."; \
	make -C "$$s_Makefile_path" -f "$(ESC_THIS_MAKEFILE)" clean; \
	:; \
	echo ; \
	echo "Clean old distributions for project and sub-projects."; \
	make -C "$$s_Makefile_path" -f "$(ESC_THIS_MAKEFILE)" distclean; \
	for i in $(ESC_SUB_PROJECT); do \
		make -C "$$i" distclean; \
	done; \
	:; \
	echo ; \
	echo "Delete unreleased files ..."; \
	s_dir_check="$$(pwd)"; if [ "$${s_dir_check##/tmp}" = "$$s_dir_check" ]; then \
		echo "ERROR: not in temporary directory!" 1>&2; \
		echo "Current dir: $$(pwd)" 1>&2; \
		exit 8; \
	fi; \
	for i in $(ESC_NO_DIST_FILE); do \
		find -regex "$$i" | awk '{gsub(/[[:space:]'\'\"']/, "\\\\&"); print; }' | xargs rm -rf ; \
	done; \
	:; \
	echo ; \
	echo "Detect last modification time ..."; \
	s_ProjectTime="$$( find -type f -and -not -regex "./$(ESC_THIS_MAKEFILE)" -and -not -regex '.*\.mcp' \
		| awk '{gsub(/[[:space:]'\'\"']/, "\\\\&"); print; }' | xargs ls -b -d -lt --full-time \
		| head -n 1 2>/dev/null \
		| awk '{gsub(/\\./, ""); print;}' | awk '{$$1=$$2=$$3=$$4=$$5=$$NF=""; print;}' \
		| date -f - "+%F_%H.%M" )"; \
	if [ "$$s_ProjectTime" = "" ]; then \
		s_ProjectTime="yyyy-mm-dd_hh.mm"; \
	fi; \
	if [ "$$s_ProjectTime" = "" ]; then \
		s_ProjectTime="yyyy-mm-dd_hh.mm"; \
	fi; \
	echo "$$s_ProjectTime"; \
	s_ProjectName_new=$(ESC_PROJECT_NAME)"_$$s_ProjectTime"; \
	s_DistDir_new="/tmp/$$s_ProjectName_new"; \
	:; \
	echo ; \
	echo "Leave temporary directory and enter source dir."; \
	if ! cd "$$s_SrcDir"; then exit 9; fi; \
	:; \
	echo ; \
	echo "Rename the temporary copy by appending the modification time."; \
	echo "$$s_DistDir -> $$s_DistDir_new"; \
	if [ "$$s_DistDir_new" = "/tmp/" ]; then echo "ERROR dir: $$s_DistDir_new" 1>&2; exit 10; fi; \
	rm -rf "$$s_DistDir_new"; \
	{ rename $(ESC_PROJECT_NAME) "$$s_ProjectName_new" "$$s_DistDir" || :; } 2>&1; \
	if [ -d "$$s_DistDir" ]; then \
		echo "Rename failed! Try to use mv instread."; \
		mv "$$s_DistDir" "$$s_DistDir_new"; \
	fi; \
	if [ ! -d "$$s_DistDir_new" ]; then \
		echo "ERROR: temporary with time stamp not found!" 1>&2; \
		exit 11; \
	fi; \
	:; \
	echo ; \
	echo "Compress now ..."; \
	if ! cd /tmp; then exit 12; fi; \
	echo "${R_TOOL}" ${R_TOOL_PARAM} "$$s_ProjectName_new.${R_EXT}" "$$s_ProjectName_new"; \
	if ! "${R_TOOL}" ${R_TOOL_PARAM} "$$s_ProjectName_new.${R_EXT}" "$$s_ProjectName_new"; then \
		echo "Compress failed." 1>&2; \
		exit 13; \
	fi; \
	rm -rf "$$s_DistDir_new"; \
	if ! cd "$$s_SrcDir"; then exit 14; fi; \
	mv "$$s_DistDir_new.${R_EXT}" . ; \
	echo "The distribution has been built: $$s_DistDir_new.${R_EXT}"; \



.PHONY: allclean
allclean: clean distclean projclean



.PHONY: show_proj_name
show_proj_name:
	@echo $(ESC_PROJECT_NAME)



.PHONY: show_sub_proj_path
show_sub_proj_path:
	@for i in $(ESC_SUB_PROJECT); do echo "$$i"; done



.PHONY: show_dist_include
show_dist_include:
	@echo $(ESC_DIST_BASE_DIR)" :"; \
	for i in $(ESC_DIST_FILE); do echo "$$i"; done



.PHONY: show_dist_exclude
show_dist_exclude:
	@echo $(ESC_DIST_BASE_DIR)" :"; \
	for i in $(ESC_NO_DIST_FILE); do echo "$$i"; done



.PHONY: show_source
show_source:
	@for i in $(ESC_LINK_SOURCE); do echo "$$i"; done



.PHONY: help
help:
	@\
	echo ""; \
	echo "Targets that includes sub-projects:"; \
	echo "  make build              build this project"; \
	echo "  make clean              clean compiling result"; \
	echo "  make proj               re-create the project file: (Makefile, .mcp)"; \
	echo "  make projclean          clean the project file"; \
	echo ""; \
	echo "Targets that excludes sub-projects:"; \
	echo "  make dist               display all distribution format supported"; \
	echo "   -> make dist.<ext>     build distribution with the format marked by <ext>"; \
	echo "  make distclean          clean the dist result"; \
	echo "  make allclean           do clean, projclean & distclean"; \
	echo "  make show_proj_name     display name of this project"; \
	echo "  make show_sub_proj_path display sub-projects' path"; \
	echo "  make show_dist_include  display the included files in building distribution"; \
	echo "  make show_dist_exclude  display the excluded files in building distribution"; \
	echo "  make show_source        display source in this project"; \
	echo "  make help               display this help"; \



depend:
	@echo "Re-build dependencies."; \
	makedepend -f$(ESC_THIS_MAKEFILE) -w1 -Y -- $(subst -I,-O,$(CFLAGS)) -- $(ESC_LINK_SOURCE) 2>/dev/null; \
	rm -f $(ESC_THIS_MAKEFILE).bak



