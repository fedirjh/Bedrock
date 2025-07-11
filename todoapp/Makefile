# TODO Application Makefile using libstuff framework

# Project paths
PROJECT_ROOT = ..
LIBSTUFF_DIR = $(PROJECT_ROOT)/libstuff
MBEDTLS_DIR = $(PROJECT_ROOT)/mbedtls

# Compiler settings
CXX = g++
CXXFLAGS = -g -std=c++20 -fPIC -DSQLITE_ENABLE_NORMALIZE -O2 -Wall -Wformat-security \
           -Wno-unqualified-std-cast-call -Wno-error=deprecated-declarations -Wno-error=unused-result
INCLUDE = -I$(PROJECT_ROOT) -I$(MBEDTLS_DIR)/include

# Library paths and dependencies
LIBPATHS = -L$(PROJECT_ROOT) -L$(MBEDTLS_DIR)/library
LIBRARIES = -lstuff -lmbedtls -lmbedx509 -lmbedcrypto -lpthread -lpcre2-8 -ldl -lz -lm

# Source files
SRCDIR = src
SOURCES = $(wildcard $(SRCDIR)/*.cpp)
OBJECTS = $(SOURCES:.cpp=.o)
TARGET = todoapp

# Build directory for intermediate files
BUILDDIR = .build
OBJ_FILES = $(SOURCES:$(SRCDIR)/%.cpp=$(BUILDDIR)/%.o)
DEP_FILES = $(OBJ_FILES:.o=.d)

# Default target
all: check-libstuff $(TARGET)

# Check if libstuff.a exists, if not provide helpful message
check-libstuff:
	@if [ ! -f "$(PROJECT_ROOT)/libstuff.a" ]; then \
		echo "Error: libstuff.a not found."; \
		echo "Please build libstuff first by running 'make libstuff.a' in the parent directory."; \
		echo ""; \
		echo "cd $(PROJECT_ROOT) && make libstuff.a"; \
		echo ""; \
		exit 1; \
	fi

# Main target
$(TARGET): $(OBJ_FILES)
	$(CXX) -o $@ $^ $(LIBPATHS) $(LIBRARIES)
	@echo "Successfully built $(TARGET)"
	@echo "Usage: ./$(TARGET) --port=8080 --db=todos.db"

# Compile source files
$(BUILDDIR)/%.o: $(SRCDIR)/%.cpp | $(BUILDDIR)
	$(CXX) $(CXXFLAGS) $(INCLUDE) -MMD -MP -c $< -o $@

# Create build directory
$(BUILDDIR):
	mkdir -p $(BUILDDIR)

# Clean build artifacts
clean:
	rm -rf $(BUILDDIR) $(TARGET) *.db

# Install (copy to parent directory)
install: $(TARGET)
	cp $(TARGET) $(PROJECT_ROOT)/

# Debug target with additional debugging symbols
debug: CXXFLAGS += -DDEBUG -g3
debug: $(TARGET)

# Run the application with default settings
run: $(TARGET)
	./$(TARGET) --port=8080 --db=todos.db

# Quick test of the application
test: $(TARGET)
	@echo "Starting TODO app in background..."
	./$(TARGET) --port=8081 --db=test_todos.db &
	@APP_PID=$$!; \
	sleep 2; \
	echo "Testing health endpoint..."; \
	curl -s http://localhost:8081/health || echo "Health check failed"; \
	echo ""; \
	echo "Creating a test TODO..."; \
	curl -s -X POST http://localhost:8081/todos \
		-H "Content-Type: application/json" \
		-d '{"title":"Test TODO","description":"Testing the API","priority":"high"}' || echo "Create failed"; \
	echo ""; \
	echo "Listing all TODOs..."; \
	curl -s http://localhost:8081/todos || echo "List failed"; \
	echo ""; \
	echo "Stopping test server..."; \
	kill $$APP_PID; \
	rm -f test_todos.db

# Help target
help:
	@echo "TODO Application Makefile"
	@echo ""
	@echo "Targets:"
	@echo "  all       - Build the TODO application (default)"
	@echo "  clean     - Remove build artifacts"
	@echo "  debug     - Build with debug symbols"
	@echo "  install   - Copy binary to parent directory"
	@echo "  run       - Run the application with default settings"
	@echo "  test      - Quick test of the application"
	@echo "  help      - Show this help message"
	@echo ""
	@echo "Prerequisites:"
	@echo "  - libstuff.a must be built first (run 'make libstuff.a' in parent directory)"
	@echo "  - mbedtls libraries must be available"
	@echo ""
	@echo "Usage examples:"
	@echo "  make"
	@echo "  make clean && make debug"
	@echo "  make run"
	@echo "  ./todoapp --port=8080 --db=todos.db"

# Include dependency files
-include $(DEP_FILES)

# Phony targets
.PHONY: all clean debug install run test help check-libstuff