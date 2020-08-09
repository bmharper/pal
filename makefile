# Remember to add -ggdb to CC and CCX, if you want debugging information
CC := clang -I src -I .
CXX := clang++ -std=c++11 -I src -I .
LINK := clang++ -lpthread -lcurl -lssl -lcrypto -lunwind
CXX_EXE_OUT := -o  
CC_OBJ_OUT := -o  
CXX_OBJ_OUT := -o  
TEST := test
OBJ := .o
EXE := 

# target directory
OUT := build

# With this, you can do "make print-VARIABLE" to dump the value of that variable
print-% : ; @echo $* = $($*)

rwildcard=$(foreach d,$(wildcard $(1:=/*)),$(call rwildcard,$d,$2) $(filter $(subst *,%,$2),$d))

PAL_CPP := $(call rwildcard,src,*.cpp)
UTFZ_CPP := third_party/utfz/utfz.cpp
TSF_CPP := third_party/tsf/tsf.cpp
MODP_C := third_party/modp/modp_b16.c
SPOOKY_C := third_party/spooky/spooky.c

TEST_CPP := $(PAL_CPP) $(call rwildcard,tests,*.cpp) $(UTFZ_CPP) $(TSF_CPP) $(SPOOKY_CPP)
TEST_C := $(SPOOKY_C) $(MODP_C)

TEST_OBJ = $(patsubst %.cpp, $(OUT)/%$(OBJ), $(TEST_CPP)) $(patsubst %.c, $(OUT)/%$(OBJ), $(TEST_C))

$(OUT)/%$(OBJ): %.cpp
	@mkdir -p $(@D)
	$(CXX) $(CXX_OBJ_OUT)$@ -c $<

$(OUT)/%$(OBJ): %.c
	@mkdir -p $(@D)
	$(CC) $(CC_OBJ_OUT)$@ -c $<

$(OUT)/test$(EXE): $(TEST_OBJ)
	$(LINK) $(CXX_EXE_OUT)$@ $(TEST_OBJ)
