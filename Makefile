# Compiler and flags
CXX = g++
CXXFLAGS = -std=c++17 -O2
LDFLAGS = -lglfw -lvulkan -ldl -lpthread -lX11 -lXxf86vm -lXrandr -lXi
DEPFLAGS = -MMD -MP

INCLUDE_DIRS = ./src ./lib
INCLUDES = $(addprefix -I,$(INCLUDE_DIRS))

# Directories
SRCDIR = ./src
OBJDIR = ./obj
DEPDIR = $(OBJDIR)/dep

# Automatically find source files and generate corresponding object and dependency file names
SOURCES := $(shell find $(SRCDIR) -type f -name '*.cpp')
OBJECTS := $(patsubst $(SRCDIR)/%.cpp,$(OBJDIR)/%.o,$(SOURCES))
DEPS := $(patsubst $(SRCDIR)/%.cpp,$(DEPDIR)/%.d,$(SOURCES))

# Name of the executable
NAME = WMTest

# Default target
all: $(NAME)

# Link object files to create the executable
$(NAME): $(OBJECTS)
	$(CXX) $^ -o $@ $(LDFLAGS)

# Compile source files into object files
$(OBJDIR)/%.o: $(SRCDIR)/%.cpp | $(DEPDIR)
	@mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) $(INCLUDES) $(DEPFLAGS) -c $< -o $@

# Create dependency files
$(DEPDIR):
	mkdir -p $@

# Include dependency files
-include $(DEPS)

# Phony targets
.PHONY: clean test

# Clean up generated files
clean:
	rm -rf $(NAME) $(OBJDIR)

# Test the executable
test: $(NAME)
	./$(NAME)