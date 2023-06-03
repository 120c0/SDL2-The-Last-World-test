TARGET := Executable

INCLUDE_DIR := include
SOURCE_DIR := src
BIN_DIR := bin
FILE_EXTENSION := cpp

SOURCE_FILES := \
	$(SOURCE_DIR)/main.$(FILE_EXTENSION)
OBJECT_FILES := $(SOURCE_FILES:%.$(FILE_EXTENSION)=%.o)

CFLAGS := -Wall -Wextra -pedantic -I$(INCLUDE_DIR)
LDLINKS := -lSDL2 -lSDL2_image
COMPILER := g++
REMOVE := rm

%.o: %.$(FILE_EXTENSION)
	$(COMPILER) -c -o $@ $(CFLAGS) $<
$(TARGET): $(OBJECT_FILES)
	$(COMPILER) -o $@ $^ $(LDLINKS) $(CFLAGS)

all: $(OBJECT_FILES) $(TARGET)

.PHONY: clean
clean:
	$(REMOVE) $(OBJECT_FILES) $(TARGET)
