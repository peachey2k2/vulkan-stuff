# Compiler and flags
CXX = g++
CXXFLAGS = -std=c++20 -O2
WARNINGS = -Wall -Wextra -Wpedantic -Werror -Wno-missing-field-initializers
LDFLAGS = -lglfw -lvulkan -ldl -lpthread -lX11 -lXxf86vm -lXrandr -lXi
DEPFLAGS = -MMD -MP

INCLUDE_DIRS = ./src ./lib
INCLUDES = $(addprefix -I,$(INCLUDE_DIRS))

# Directories
SRCDIR = ./src
OBJDIR = ./obj

# Automatically find source files and generate corresponding object and dependency file names
SOURCES := $(shell find $(SRCDIR) -type f -name '*.cpp')
OBJECTS := $(patsubst $(SRCDIR)/%.cpp,$(OBJDIR)/%.o,$(SOURCES))
DEPS := $(OBJECTS:.o=.d)

# Name of the executable
NAME = WMTest

# Default target
all: $(NAME)

# Link object files to create the executable
$(NAME): $(OBJECTS)
	$(CXX) $^ -o $@ $(LDFLAGS)

# Compile source files into object files
$(OBJDIR)/%.o: $(SRCDIR)/%.cpp
	@mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) $(WARNINGS) $(INCLUDES) $(DEPFLAGS) -c $< -o $@

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