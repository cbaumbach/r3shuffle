# ==== COMPILE AND LINK TIME VARIABLES ===============================

CC = gcc
CFLAGS += -Wall -Wextra -O2
CPPFLAGS += -I include
CPPFLAGS += $(unity_includes)
CPPFLAGS += -D _GNU_SOURCE

# ==== MACROS ========================================================

SED := sed
TEE := tee
MV := mv -f
RM := rm -f
MKDIR := mkdir -p

# $(call exclude-files,list-of-files-to-exclude,list-of-files)
define exclude-files
$(strip \
  $(foreach f, $2, \
    $(if $(findstring $f, $1),, $f)))
endef

# $(call make-dependency-file,source-file,object-file,dependency-file)
define make-dependency-file
$(CC) $(CFLAGS) $(CPPFLAGS) $(TARGET_ARCH) $(if $(filter gcc,$(CC)),-MM,-M) $1 | \
$(SED) -e 's,$(notdir $2) *:,$2 $3:,' > $3.tmp && \
$(SED) -e 's/#.*//' -e 's/^[^:]*: *//' -e 's/ *$$//' -e '/^$$/d' \
       -e 's/\([^\\]\)$$/\1:/' $3.tmp >> $3.tmp && \
$(MV) $3.tmp $3
endef

make-dependency-file-command = \
  $(call make-dependency-file,$<,$@,$(subst .o,.d,$@))

# $(call subst-dir,files,directory)
subst-dir = $(strip $(foreach f, $1, $2/$(notdir $f)))


# ==== FILES =========================================================

primary_directory := src
test_directory := test
test_runner_directory := $(test_directory)/test_runners
unity_directory := $(test_directory)/unity

primary_sources := $(wildcard $(primary_directory)/*.c)
primary_objects := $(subst .c,.o,$(primary_sources))
primary_executables := r3shuffle

include path_to_unity
ifndef UNITY_HOME
  $(error In ./path_to_unity file, set 'UNITY_HOME = path/to/unity')
endif
unity_primary_directory := $(UNITY_HOME)/src
unity_fixture_directory := $(UNITY_HOME)/extras/fixture/src
unity_includes := -I $(unity_primary_directory) -I $(unity_fixture_directory)
unity_primary_sources := $(unity_primary_directory)/unity.c
unity_fixture_sources := $(unity_fixture_directory)/unity_fixture.c
unity_sources := $(unity_primary_sources) $(unity_fixture_sources)
unity_objects := $(subst .c,.o,$(call subst-dir,$(unity_sources),$(unity_directory)))
unity_dependencies := $(subst .o,.d,$(unity_objects))

test_sources := $(wildcard $(test_directory)/*.c)
test_sources += $(wildcard $(test_runner_directory)/*.c)
test_objects := $(subst .c,.o,$(test_sources))
test_executables := $(test_runner_directory)/all_tests
test_logfiles := $(addsuffix .log,$(test_executables))

objects := $(primary_objects) $(helper_objects) $(test_objects) $(unity_objects)
dependencies := $(subst .o,.d,$(objects))

primary_library := $(primary_directory)/libprimary.a
test_library := $(test_directory)/libtest.a
unity_library := $(unity_directory)/libunity.a
libraries := $(test_library) $(unity_library) $(primary_library)
executables := $(primary_executables) $(test_executables)


# ==== RULES =========================================================

# Empty the list of known suffix rules.  We'll roll our own.
.SUFFIXES:

.PHONY: all
all: $(primary_executables) test

r3shuffle: $(primary_directory)/main.o $(primary_library)
	$(LINK.o) $^ -o $@

$(primary_library): $(call exclude-files,$(primary_directory)/main.o,$(primary_objects))
	$(AR) $(ARFLAGS) $@ $? >/dev/null

$(test_library): $(call exclude-files,$(test_directory)/all_tests.o,$(test_objects))
	$(AR) $(ARFLAGS) $@ $? >/dev/null

$(unity_directory)/unity.o: $(unity_primary_directory)/unity.c
	@$(make-dependency-file-command)
	$(COMPILE.c) -o $@ $<

$(unity_directory)/unity_fixture.o: $(unity_fixture_directory)/unity_fixture.c
	@$(make-dependency-file-command)
	$(COMPILE.c) -o $@ $<

$(unity_library): $(unity_objects)
	$(AR) $(ARFLAGS) $@ $? >/dev/null

.PHONY: test
test: $(test_executables)

$(test_runner_directory)/all_tests: $(test_runner_directory)/all_tests.o $(libraries) \
        | $(test_directory)/tmp
	$(LINK.o) $^ -o $@
	-$@ | $(TEE) $@.log

$(test_directory)/tmp:
	$(MKDIR) $@

%.o: %.c
	@$(make-dependency-file-command)
	$(COMPILE.c) -o $@ $<

ifneq "$(MAKECMDGOALS)" "clean"
-include $(dependencies)
endif

.PHONY: clean
clean:
	$(RM) $(dependencies) $(objects) $(libraries) $(executables) $(test_logfiles)
	$(RM) -r $(test_directory)/tmp
