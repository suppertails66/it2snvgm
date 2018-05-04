IDIR := src
SRCDIR := src
ODIR := obj
LDIR :=

BLACKTDIR = ../blackt

AR=ar
CXX=g++
# Compile only
CDEFINES = 
CFLAGS = -std=gnu++11 -O2 -Wall
CINCLUDES = -I${BLACKTDIR}/src
CXXFLAGS=$(CFLAGS) $(CDEFINES) $(CINCLUDES) -I$(IDIR)

SRC := $(wildcard $(SRCDIR)/*/*.cpp)
OBJ := $(patsubst $(SRCDIR)/%,$(ODIR)/%,$(patsubst %.cpp,%.o,$(SRC)))
DEP := $(patsubst %.o,%.d,$(OBJ))
LIB := libsvgm.a

all: blackt $(LIB)

-include $(DEP)

$(ODIR)/%.o: $(SRCDIR)/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) -c -MMD -MP -MF $(@:.o=.d) -o $@ $< $(CXXFLAGS)

$(LIB): $(OBJ)
	$(AR) rcs $@ $^

blackt:
	cd ../blackt && $(MAKE) && cd $(CURDIR)

.PHONY: blackt clean

clean:
	rm -f $(LIB)
	rm -rf $(ODIR)
	cd ../blackt && $(MAKE) clean && cd $(CURDIR)
