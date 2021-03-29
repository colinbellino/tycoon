CC=gcc
BUILD_DIR=build
OBJ_DIR=$(BUILD_DIR)/objs
DYN_DIR=$(BUILD_DIR)/dyn
SRC_DIR=src
GAME_ENTRY_FILE=$(SRC_DIR)/game.cpp
CC_OPT=-Wall
CC_DEV_OPT=-MMD -MP
GCC_DEBUG_OPT=-g
SRC=$(filter-out $(GAME_ENTRY_FILE), $(filter-out $(SRC_DIR)/main%,$(wildcard $(SRC_DIR)/*.cpp)))
EXE_NAME=tycoon

OBJS=$(patsubst $(SRC_DIR)/%,$(OBJ_DIR)/%,$(SRC:.cpp=.o))
DEPS=$(patsubst $(SRC_DIR)/%,$(OBJ_DIR)/%,$(SRC:.cpp=.d))

ifeq ($(OS),Windows_NT)
	ifeq ($(PROCESSOR_ARCHITECTURE),AMD64)
		MAIN=$(SRC_DIR)/main-win.cpp
		OBJS+=$(OBJ_DIR)/main-win.o
		GAME_LIB=$(patsubst $(SRC_DIR)/%,$(DYN_DIR)/%,$(GAME_ENTRY_FILE:.cpp=.dll))
		CFLAGS += -I ./include/win64/**
		LDFLAGS += -L ./lib/win64/**
		LDLIBS += -lmingw32
		SHARED_LIB=-shared
	endif
else
	ifeq ($(shell uname -s),Darwin)
		MAIN=$(SRC_DIR)/main-mac.cpp
		OBJS+=$(OBJ_DIR)/main-mac.o
		GAME_LIB=$(patsubst $(SRC_DIR)/%,$(DYN_DIR)/%,$(GAME_ENTRY_FILE:.cpp=.so))
		DEPS += $(GAME_LIB:.so=.d)
		CFLAGS += -I ./include/mac/**
		LDFLAGS += -L ./lib/mac/**
		LDLIBS += -lSDL2 -lSDL2_image
		SHARED_LIB=-dynamiclib
	endif
endif

GAME_ENTRY_OBJ=$(patsubst $(SRC_DIR)/%,$(OBJ_DIR)/%,$(GAME_ENTRY_FILE:.cpp=.o))

default: build

gamelib: $(GAME_LIB)

$(GAME_LIB): $(GAME_ENTRY_OBJ) $(OBJS)
	@echo Relinking game shared object
	@$(CC) $(LDFLAGS) $(LDLIBS) $(SHARED_LIB) $^ -o $(GAME_LIB)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp | ensure_dirs
	@echo Building $@
	@$(CC) $(CC_OPT) -c $(CC_DEV_OPT) $(CFLAGS) $< -o $@ $(DEV_VARS)

dev: DEV_VARS=-D HOT_RELOAD
dev: $(OBJS) | $(GAME_LIB)
	@echo Building $(BUILD_DIR)/$(EXE_NAME)
	@$(CC) $(LDFLAGS) $^ $(LDLIBS) -o $(BUILD_DIR)/$(EXE_NAME)

build: $(GAME_ENTRY_OBJ) $(OBJS)
	@echo Building $(BUILD_DIR)/$(EXE_NAME)
	@$(CC) $(LDFLAGS) $^ $(LDLIBS) -o $(BUILD_DIR)/$(EXE_NAME)

run:
	@echo Running $(BUILD_DIR)/$(EXE_NAME)
	@$(BUILD_DIR)/$(EXE_NAME)


.PHONY: clean

-include $(DEPS)

ensure_dirs:
	@mkdir -p $(BUILD_DIR) $(OBJ_DIR) $(DYN_DIR)

clean:
	rm -rf $(BUILD_DIR)