CXX         = clang++
CXXFLAGS    = -std=c++1z -stdlib=libc++ -Wall -pedantic -c
LDFLAGS     = -lc++ -lc++abi -lGLEW -lglfw -lGL
INCFLAGS    = -I./include

TARGET      = main
MODULES     = . \
						  annotation \
							common \
							visualization

SRC_DIR     = $(addprefix src/,$(MODULES))
INC_DIR     = $(addprefix include/,$(MODULES))
BUILD_DIR   = $(addprefix build/,$(MODULES))
RSRC_DIR    = ./resources

SOURCES	  	= $(foreach sdir,$(SRC_DIR),$(wildcard $(sdir)/*.cc))
INCLUDES    = $(foreach idir,$(INC_DIR),$(wildcard $(idir)/*.h))
OBJECTS 	  = $(patsubst src/%.cc,build/%.o,$(SOURCES))

first: $(TARGET)

$(TARGET): $(OBJECTS)
	export LD_LIBRARY_PATH=/home/bence/llvm-release/lib:/home/bence/libcxx-release/lib
	$(CXX) $(LDFLAGS) $^ -o $@

build/%.o: src/%.cc
	$(CXX) $(CXXFLAGS) $(INCFLAGS) $< -o $@

clean:
	rm -f ./$(TARGET)
	find ./build -type f -name '*.o' -delete

format: $(SOURCES) $(INCLUDES)
	clang-format -style=Google -i $(SOURCES)
	clang-format -style=Google -i $(INCLUDES)

run:
	./$(TARGET)