IDIR := src
SRCDIR := src
ODIR := obj
LDIR :=

BLACKTDIR = ../blackt
LIBSVGMDIR = ../libsvgm

AR=ar
CXX=g++
# Compile only
CDEFINES = 
#CLIBS = -lpng
CFLAGS = -std=gnu++11 -O2 -Wall -L${LIBSVGMDIR} -lsvgm -L${BLACKTDIR} -lblackt
CINCLUDES = -I${BLACKTDIR}/src -I${LIBSVGMDIR}/src
CXXFLAGS=$(CFLAGS) $(CDEFINES) $(CINCLUDES) -I$(IDIR)

LIBDEPS := $(LIBSVGMDIR)/libsvgm.a $(BLACKTDIR)/libblackt.a

TOOLSRCS := $(wildcard $(SRCDIR)/*.cpp)
TOOLSINDIR := $(patsubst %.cpp,%,$(TOOLSRCS))
TOOLS := $(notdir $(TOOLSINDIR))

all: blackt libsvgm $(TOOLS)

blackt:
	cd ${BLACKTDIR} && $(MAKE) && cd $(CURDIR)

libsvgm:
	cd ${LIBSVGMDIR} && $(MAKE) && cd $(CURDIR)
	
#$(SRCDIR)/%: blackt libsms
#	$(CXX) $@.cpp $(OBJ) -o $(notdir $@) $(CXXFLAGS)

it2snvgm: $(SRCDIR)/it2snvgm.cpp $(LIBDEPS)
	$(CXX) $(SRCDIR)/$@.cpp $(OBJ) -o $@ $(CXXFLAGS)

.PHONY: blackt libsvgm cleanme clean

cleanme:
	rm -f $(TOOLS)

clean: cleanme
#	rm -f $(LIB)
#	rm -rf $(ODIR)
	cd ${BLACKTDIR} && $(MAKE) clean && cd $(CURDIR)
	cd ${LIBSVGMDIR} && $(MAKE) clean && cd $(CURDIR)