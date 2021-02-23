CXX        = g++
LD         = g++

CXXFLAGS   = `wx-config --cxxflags` -std=c++17
CPPFLAGS   = `wx-config --cppflags`

EXEC       := landstalker_editor
LIBS       := `wx-config --libs core,base,aui` -lstdc++fs
SRCDIR     := src
BUILDDIR   := build
BINDIR     := bin
INC_DIRS   := 
INCS       := $(SRCDIR) $(addsuffix /include,$(SRCDIR)) $(addprefix $(SRCDIR)/,$(INC_DIRS))

SRC       := $(foreach sdir,$(SRCDIR),$(wildcard $(sdir)/*.cpp))
OBJ       := $(patsubst $(SRCDIR)/%.cpp,$(BUILDDIR)/%.o,$(SRC))
INCLUDES  := $(addprefix -I,$(INCS))

vpath %.cpp $(SRCDIR) $(EXEC_SDIR)

DEBUG=no
ifeq ($(DEBUG),yes)
    CXXFLAGS += -g
endif

.PHONY: all checkdirs clean clean-all

all: checkdirs $(EXEC)

checkdirs: $(BUILDDIR)

$(BUILDDIR):
	@mkdir -p $@

clean:
	@rm -rf $(BUILDDIR)

clean-all: clean
	@rm -rf $(EXEC)

$(BUILDDIR)/%.o: $(SRCDIR)/%.cpp
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) $(INCLUDES) -c $< -o $@

$(EXEC): $(OBJ) $(patsubst $(SRCDIR)/%.cpp,$(BUILDDIR)/%.o,$(wildcard $(SRCDIR)/*.cpp))
	$(LD) $^ -o $@ $(LIBS)

