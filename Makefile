CC         := g++
LD         := g++

EXEC       := landstalker_editor
LIBS       := 
SRCDIR     := src
BUILDDIR   := build
BINDIR     := bin
INC_DIRS   := 
INCS       := $(SRCDIR) $(addsuffix /include,$(SRCDIR)) $(addprefix $(SRCDIR)/,$(INC_DIRS))

SRC       := $(foreach sdir,$(SRCDIR),$(wildcard $(sdir)/*.cpp))
OBJ       := $(patsubst $(SRCDIR)/%.cpp,$(BUILDDIR)/%.o,$(SRC))
INCLUDES  := $(addprefix -I,$(INCS))
EXECS     := $(addprefix $(BINDIR)/,$(EXEC))
EXEC_SDIR := $(addprefix $(SRCDIR)/,$(EXEC))
EXEC_ODIR := $(addprefix $(BUILDDIR)/,$(EXEC))

vpath %.cpp $(SRCDIR) $(EXEC_SDIR)

define make-obj
$(BUILDDIR)/%.o: $(SRCDIR)/%.cpp
	$(CC) $(CFLAGS) $(CXXFLAGS) $(INCLUDES) -c $$< -o $$@
endef

define make-exec
$(BINDIR)/$1: $(OBJ) $(patsubst $(SRCDIR)/%.cpp,$(BUILDDIR)/%.o,$(wildcard $(SRCDIR)/*.cpp))
	$(LD) $(LDFLAGS) $$^ $(LIBS) -o $(BINDIR)/$1
endef

.PHONY: all checkdirs clean clean-all

all: checkdirs $(EXECS)

checkdirs: $(BUILD_DIRS) $(BINDIR)

$(BUILD_DIRS):
	@mkdir -p $@/src

$(BINDIR):
	@mkdir -p $@

clean:
	@rm -rf $(BUILDDIR)

clean-all: clean
	@rm -rf $(BINDIR)

$(foreach exec,$(EXEC),$(eval $(call make-obj,$(exec))))
$(foreach exec,$(EXEC),$(eval $(call make-exec,$(exec))))

