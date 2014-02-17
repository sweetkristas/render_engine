#
# Main Makefile, intended for use on Linux/X11 and compatible platforms
# using GNU Make.
#
# It should guess the paths to the game dependencies on its own, except for
# Boost which is assumed to be installed to the default locations. If you have
# installed Boost to a non-standard location, you will need to override CXXFLAGS
# and LDFLAGS with any applicable -I and -L arguments.
#
# The main options are:
#
#   CCACHE           The ccache binary that should be used when USE_CCACHE is
#                     enabled (see below). Defaults to 'ccache'.
#   CXX              C++ compiler comand line.
#   CXXFLAGS         Additional C++ compiler options.
#   OPTIMIZE         If set to 'yes' (default), builds with compiler
#                     optimizations enabled (-O2). You may alternatively use
#                     CXXFLAGS to set your own optimization options.
#   LDFLAGS          Additional linker options.
#   USE_CCACHE       If set to 'yes' (default), builds using the CCACHE binary
#                     to run the compiler. If ccache is not installed (i.e.
#                     found in PATH), this option has no effect.
#

OPTIMIZE=yes
CCACHE?=ccache
USE_CCACHE?=$(shell which $(CCACHE) 2>&1 > /dev/null && echo yes)
ifneq ($(USE_CCACHE),yes)
CCACHE=
endif

ifeq ($(OPTIMIZE),yes)
BASE_CXXFLAGS += -O2
endif

ifeq ($(CXX), g++)
GCC_GTEQ_490 := $(shell expr `$(CXX) -dumpversion | sed -e 's/\.\([0-9][0-9]\)/\1/g' -e 's/\.\([0-9]\)/0\1/g' -e 's/^[0-9]\{3,4\}$$/&00/'` \>= 40900)
ifeq "$(GCC_GTEQ_490)" "1"
BASE_CXXFLAGS += -fdiagnostics-color=auto -fsanitize=undefined
endif
endif

SDL2_CONFIG?=sdl2-config
USE_SDL2?=$(shell which $(SDL2_CONFIG) 2>&1 > /dev/null && echo yes)

ifneq ($(USE_SDL2),yes)
$(error SDL2 not found, SDL-1.2 is no longer supported)
endif

USE_LUA?=$(shell pkg-config --exists lua5.2 && echo yes)

# Initial compiler options, used before CXXFLAGS and CPPFLAGS.
BASE_CXXFLAGS += -std=c++0x -g -rdynamic -fno-inline-functions \
	-fthreadsafe-statics -Wnon-virtual-dtor -Werror \
	-Wignored-qualifiers -Wformat -Wswitch -Wreturn-type \
	-DUSE_SHADERS -DUTILITY_IN_PROC -DUSE_ISOMAP \
	-Wno-narrowing -Wno-literal-suffix

# Compiler include options, used after CXXFLAGS and CPPFLAGS.
INC := -Isrc -Iinclude $(shell pkg-config --cflags x11 sdl2 glew SDL2_image SDL2_ttf libpng zlib)

ifdef STEAM_RUNTIME_ROOT
	INC += -I$(STEAM_RUNTIME_ROOT)/include
endif

# Linker library options.
LIBS := $(shell pkg-config --libs x11 gl ) \
	$(shell pkg-config --libs sdl2 glew SDL2_image libpng zlib) -lSDL2_ttf -lSDL2_mixer

# libvpx check
USE_LIBVPX?=$(shell pkg-config --exists vpx && echo yes)
ifeq ($(USE_LIBVPX),yes)
	BASE_CXXFLAGS += -DUSE_LIBVPX
	INC += $(shell pkg-config --cflags vpx)
	LIBS += $(shell pkg-config --libs vpx)
endif

BASE_CXXFLAGS += -DUSE_SVG
INC += $(shell pkg-config --cflags cairo)
LIBS += $(shell pkg-config --libs cairo freetype2)

include Makefile.common

src/%.o : src/%.cpp
	@echo "Building:" $<
	@$(CCACHE) $(CXX) $(BASE_CXXFLAGS) $(CXXFLAGS) $(CPPFLAGS) $(INC) -DIMPLEMENT_SAVE_PNG -c -o $@ $<
	@$(CXX) $(BASE_CXXFLAGS) $(CXXFLAGS) $(CPPFLAGS) $(INC) -DIMPLEMENT_SAVE_PNG -MM $< > $*.d
	@mv -f $*.d $*.d.tmp
	@sed -e 's|.*:|src/$*.o:|' < $*.d.tmp > src/$*.d
	@sed -e 's/.*://' -e 's/\\$$//' < $*.d.tmp | fmt -1 | \
		sed -e 's/^ *//' -e 's/$$/:/' >> src/$*.d
	@rm -f $*.d.tmp

render_engine: $(objects)
	@echo "Linking : render_engine"
	@$(CCACHE) $(CXX) \
		$(BASE_CXXFLAGS) $(LDFLAGS) $(CXXFLAGS) $(CPPFLAGS) $(INC) \
		$(objects) -o render_engine \
		$(LIBS) -lboost_regex -lboost_system -lboost_filesystem -lpthread -fthreadsafe-statics

# pull in dependency info for *existing* .o files
-include $(objects:.o=.d)

clean:
	rm -f src/*.o src/*.d *.o *.d render_engine
