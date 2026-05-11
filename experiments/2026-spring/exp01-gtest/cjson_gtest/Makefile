// **********************************
// ***  Write your Makefile here  ***
// **********************************

# ---------- GTEST ----------
GTEST_ROOT := /opt/homebrew/Cellar/googletest/1.17.0
GTEST_INC := $(GTEST_ROOT)/include
GTEST_LIB := $(GTEST_ROOT)/lib

# ---------- Toolchain ----------
CXX      := clang++
LLVM_PROFDATA := llvm-profdata
LLVM_COV := llvm-cov

# ---------- Paths ----------
CJSON_DIR := ./cJSON
CJSON_LIB := $(CJSON_DIR)/libcjson.a
CJSON_HDR := $(CJSON_DIR)/cJSON.h
CJSON_SRC := $(CJSON_DIR)/cJSON.c

# ---------- Target ----------
TARGET   := test_cjson
SRC      := test_cjson.cpp
OBJ      := $(SRC:.cpp=.o)

# ---------- Coverage files ----------
PROFRAW  := $(TARGET).profraw
PROFDATA := $(TARGET).profdata
TEXT_REPORT := coverage.txt
HTML_DIR := htmlcov

# ---------- Flags ----------
CXXFLAGS := -O0 -g -fprofile-instr-generate -fcoverage-mapping -I$(CJSON_DIR) -I$(GTEST_INC)
LDFLAGS  := -fprofile-instr-generate
LDLIBS   := $(CJSON_LIB) -L$(GTEST_LIB) -lgtest -lgtest_main -lpthread

.PHONY: all run report html clean

all: $(TARGET)

$(TARGET): $(OBJ) $(CJSON_LIB)
	$(CXX) $(CXXFLAGS) $(OBJ) $(LDLIBS) $(LDFLAGS) -o $@

%.o: %.cpp $(CJSON_HDR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

run: $(TARGET)
	LLVM_PROFILE_FILE="$(PROFRAW)" ./$(TARGET)

report: run
	$(LLVM_PROFDATA) merge -sparse $(PROFRAW) -o $(PROFDATA)
	$(LLVM_COV) report ./$(TARGET) \
		-instr-profile=$(PROFDATA) \
		$(CJSON_SRC) | tee $(TEXT_REPORT)
	$(LLVM_COV) show ./$(TARGET) \
		-instr-profile=$(PROFDATA) \
		--show-line-counts-or-regions \
		--show-branches=count \
		$(CJSON_SRC) >> $(TEXT_REPORT)

html:
	$(LLVM_PROFDATA) merge -sparse $(PROFRAW) -o $(PROFDATA)
	rm -rf $(HTML_DIR)
	mkdir -p $(HTML_DIR)
	$(LLVM_COV) show ./$(TARGET) \
		-instr-profile=$(PROFDATA) \
                -show-mcdc-summary \
		-format=html \
		-output-dir=$(HTML_DIR) \
		--show-line-counts-or-regions \
		--show-branches=count \
		$(CJSON_SRC)
	$(LLVM_COV) report ./$(TARGET) \
		-instr-profile=$(PROFDATA) \
                -show-mcdc-summary \
		$(CJSON_SRC) | tee $(TEXT_REPORT)

clean:
	rm -f $(TARGET) $(OBJ) *.profraw *.profdata $(TEXT_REPORT)
	rm -rf $(HTML_DIR)

