TARGET := processor
LIBTARGET := libcsdec.a

SRC_DIR := src
INC_DIR := include

# capstone library name (without prefix 'lib' and suffix '.so')
LIBCAPSTONE := capstone

CXX := g++
CXXFLAGS := -std=c++17 -Wall
CXXFLAGS += -I$(INC_DIR)
CXXFLAGS += -l$(LIBCAPSTONE)

# When the value is 1, cache mode is enabled.
# This mode speeds up the decoding process by saving the disassemble
# and trace results in the software cache."
CACHE_MODE := 1

PRINT_EDGE_COV := 0

ifeq ($(CACHE_MODE), 1)
	CXXFLAGS += -DCACHE_MODE
endif

ifeq ($(PRINT_EDGE_COV), 1)
	CXXFLAGS += -DPRINT_EDGE_COV
endif

SRCS := $(SRC_DIR)/decoder.cpp \
	$(SRC_DIR)/deformatter.cpp \
	$(SRC_DIR)/disassembler.cpp \
	$(SRC_DIR)/utils.cpp \
	$(SRC_DIR)/bitmap.cpp \
	$(SRC_DIR)/common.cpp \
	$(SRC_DIR)/cache.cpp \
	$(SRC_DIR)/trace.cpp \
	$(SRC_DIR)/process.cpp \
	$(SRC_DIR)/libcsdec.cpp \
	$(SRC_DIR)/processor.cpp

OBJS := $(SRCS:.cpp=.o)

FIB_TEST := tests/fib
BRANCHES_TEST := tests/branches


all: CXXFLAGS += -O3
all: CXXFLAGS += -DNDEBUG # Disable calls to assert()
all: $(TARGET) $(LIBTARGET)

# FIXME: Enabling UBSAN and GLIBCXX_DEBUG is not compatible with proc-trace
debug: CXXFLAGS += -DDEBUG_BUILD
debug: CXXFLAGS += -g -fsanitize=undefined -D_GLIBCXX_DEBUG
debug: $(TARGET) $(LIBTARGET)

$(TARGET): $(OBJS)
	$(CXX) -o $@ $^ $(CXXFLAGS)

$(LIBTARGET): $(subst src/processor.o,,$(OBJS))
	$(AR) -rc $@ $^

test: fib-test branches-test

fib-test:
	make -C $(FIB_TEST) test

branches-test:
	make -C $(BRANCHES_TEST) test

clean:
	rm -rf $(OBJS) $(TARGET) $(LIBTARGET)

dist-clean: clean
	make -C $(FIB_TEST) clean
	make -C $(BRANCHES_TEST) clean

.PHONY: all debug test fib-test branches-test clean dist-clean
