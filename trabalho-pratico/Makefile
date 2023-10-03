# CONFIGURATION VARIABLES

CC              := gcc
CFLAGS          := -Wall -Wextra -Werror -pedantic
STANDARDS       := -std=c99 -D_POSIX_C_SOURCE=200809L
LIBS            := -lm -lpthread

DEBUG_CFLAGS    := -O0 -ggdb3
RELEASE_CFLAGS  := -O2
PROFILE_CFLAGS  := -O2 -ggdb3

# Note: none of these directories can be the root of the project
# Also, these may need to be synced with the ones in .gitignore
BUILDDIR        := build
MAIN_EXENAME    := programa-principal
TEST_EXENAME    := programa-testes
DEPDIR          := deps
DOCSDIR         := docs
OBJDIR          := obj

# Default installation directory (if PREFIX is not set)
PREFIX          ?= $(HOME)/.local

# END OF CONFIGURATION

SOURCES = $(shell find "src" -name '*.c' -type f)
MAIN_SOURCES = $(filter-out test.c, $(SOURCES))
TEST_SOURCES = $(filter-out main.c, $(SOURCES))

OBJECTS = $(patsubst src/%.c, $(OBJDIR)/%.o, $(SOURCES))
MAIN_OBJECTS = $(filter-out $(OBJDIR)/test.o, $(OBJECTS))
TEST_OBJECTS = $(filter-out $(OBJDIR)/main.o, $(OBJECTS))

HEADERS = $(shell find "include" -name '*.h' -type f)
THEMES  = $(wildcard theme/*)
DEPENDS = $(patsubst src/%.c, $(DEPDIR)/%.d, $(SOURCES))
REPORTS = $(patsubst reports/%.tex, %.pdf, $(shell find reports -name '*.tex' -type f))

ifeq ($(DEBUG), 1)
	CFLAGS += $(DEBUG_CFLAGS)
	BUILD_TYPE = DEBUG
else ifeq ($(PROFILE), 1)
	CFLAGS += $(PROFILE_CFLAGS)
	BUILD_TYPE = PROFILE
else
	CFLAGS += $(RELEASE_CFLAGS)
	BUILD_TYPE = RELEASE
endif
CFLAGS += $(STANDARDS)

# Only generate dependencies for tasks that require them
# THIS WILL NOT WORK IF YOU TRY TO MAKE AN INDIVIDUAL FILE
ifeq (, $(MAKECMDGOALS))
	INCLUDE_DEPENDS = Y
else ifneq (, $(filter default, $(MAKECMDGOALS)))
	INCLUDE_DEPENDS = Y
else ifneq (, $(filter all, $(MAKECMDGOALS)))
	INCLUDE_DEPENDS = Y
else ifneq (, $(filter install, $(MAKECMDGOALS)))
	INCLUDE_DEPENDS = Y
else
	INCLUDE_DEPENDS = N
endif

# Automatic testing system requires the default rule to also build the test :-(
default: $(BUILDDIR)/$(MAIN_EXENAME) $(BUILDDIR)/$(TEST_EXENAME)
report: $(REPORTS)
all: $(BUILDDIR)/$(MAIN_EXENAME) $(BUILDDIR)/$(TEST_EXENAME) $(DOCSDIR) $(REPORTS)

ifeq (Y, $(INCLUDE_DEPENDS))
include $(DEPENDS)
endif

# Welcome to our unorthodox Makefile! To get auto-dependency generation working, this code is
# unusual for a Makefile, but hey, it works!
#
# To compile a source file, a makefile rule is generated with $(CC) -MM, to account for header
# dependencies. The commands that actually compile the source are added to that rule file before
# its included.
$(DEPDIR)/%.d: src/%.c Makefile
	$(eval OBJ := $(patsubst src/%.c, $(OBJDIR)/%.o, $<))

	@# Commands to make the object's file directory, compile the source and re-generate the file
	@# dependencies to $@2 while compiling. The dependencies are modifed (format), to put them in
	@# a single line and to add the dependency file to the rule's outputs.
	@#
	@# An echo command is used to give feedback on GCC (instead of make's output), to remove the
	@# preprocessor options related to dependency generation.

	$(eval RULE_CMD_MKDIR = @mkdir -p $(shell dirname $(OBJ)))
	$(eval RULE_CMD_FEEDBACK = @echo $(CC) -c -o $(OBJ) $< $(CFLAGS) -Iinclude)
	$(eval RULE_CMD_GCC = @$(CC) -MMD -MT $(OBJ) -MF $@2 -MP -c -o $(OBJ) $< $(CFLAGS) -Iinclude)
	$(eval RULE_CMD_FORMAT = @sed -ze 's/ \\\\\\\\\\\\n//g ; s|$(OBJ):|$(OBJ) $@:|g' -i $@2)

	@# Create another temporary dependency file ($@3), to update the dependencies, but keeping
	@# the compiler commands form $@. Then, $@3 is copied into $@ and the temporary files are
	@# removed.
	$(eval RULE_CMD_ADD_COMMANDS = @head -n1 $@2 > $@3 ; \
	                                tail -n+2 $@ | grep '^\\s' >> $@3 ; \
	                                tail -n+2 $@2 >> $@3)
	$(eval RULE_CMD_COPY = @cp $@3 $@ ; rm $@2 $@3)

	@# Create the dependency file for the first time, adding all the commands.
	@mkdir -p $(shell dirname $@)
	$(CC) -MM $< -MT $(OBJ) -Iinclude > $@

	@printf "\t$(RULE_CMD_MKDIR)\n" >> $@
	@printf "\t$(RULE_CMD_FEEDBACK)\n" >> $@
	@printf "\t$(RULE_CMD_GCC)\n" >> $@
	@printf "\t$(RULE_CMD_FORMAT)\n" >> $@
	@printf "\t$(RULE_CMD_ADD_COMMANDS)\n" >> $@
	@printf "\t$(RULE_CMD_COPY)\n" >> $@

$(BUILDDIR)/$(MAIN_EXENAME) $(BUILDDIR)/$(MAIN_EXENAME)_type: $(MAIN_OBJECTS)
	@mkdir -p $(BUILDDIR)
	@echo $(BUILD_TYPE) > $@_type
	$(CC) -o $@ $^ $(LIBS)
	@ln -s $@ . 2> /dev/null ; true

$(BUILDDIR)/$(TEST_EXENAME) $(BUILDDIR)/$(TEST_EXENAME)_type: $(TEST_OBJECTS)
	@mkdir -p $(BUILDDIR)
	@echo $(BUILD_TYPE) > $@_type
	$(CC) -o $@ $^ $(LIBS)
	@ln -s $@ . 2> /dev/null ; true

define Doxyfile
	INPUT                  = include src ../README.md ../DEVELOPERS.md
	RECURSIVE              = YES
	EXTRACT_ALL            = YES
	FILE_PATTERNS          = *.h *.c

	PROJECT_NAME           = LI3
	USE_MDFILE_AS_MAINPAGE = ../README.md

	OUTPUT_DIRECTORY       = $(DOCSDIR)
	GENERATE_HTML          = YES
	GENERATE_LATEX         = NO

	HTML_EXTRA_FILES       = theme
	HTML_EXTRA_STYLESHEET  = theme/UMinho.css
	PROJECT_LOGO           = theme/UMinhoLogo.png
endef
export Doxyfile

$(DOCSDIR): $(SOURCES) $(HEADERS) $(THEMES)
	echo "$$Doxyfile" | doxygen -
	@touch $(DOCSDIR) # Update "last updated" time to now
	@cp ../Requirements.pdf $(DOCSDIR)/html

%.pdf: reports/%.tex
	$(eval TMP_LATEX = $(shell mktemp -d))

	@mkdir -p $(BUILDDIR)
	pdflatex -halt-on-error -output-directory $(TMP_LATEX) $<
	@cp $(TMP_LATEX)/$@ $(BUILDDIR)/$@
	@ln -s $(BUILDDIR)/$@ $@
	@rm -r $(TMP_LATEX)

.PHONY: clean
clean:
	@# Reports must be removed from the "clean" rule when they're made permanent
	rm -r $(BUILDDIR) $(DEPDIR) $(DOCSDIR) $(OBJDIR) $(REPORTS) $(MAIN_EXENAME) $(TEST_EXENAME) \
		2> /dev/null ; true

install: $(BUILDDIR)/$(MAIN_EXENAME)
	install -Dm 755 $(BUILDDIR)/$(MAIN_EXENAME) $(PREFIX)/bin

.PHONY: uninstall
uninstall:
	rm $(PREFIX)/bin/$(MAIN_EXENAME)
