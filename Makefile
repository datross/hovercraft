
CC = gcc
# On met std=c99 pour s'assurer de l'existence du format %n dans sscanf().
# -Wall active -Wswitch, qui râle dès qu'on ne traite pas tous les cas
# possibles dans des switch() - Inacceptable pour des switchs de touches du clavier.
CFLAGS = -g -std=c99 -Wall -Wno-switch -O3 -D_GNU_SOURCE
LDFLAGS = -lm -lGL -lGLU -lSDL -lSDL_image

APP_BIN = hovercraft

SRC_PATH = src
OBJ_PATH = obj
INC_PATH = include
BIN_PATH = bin

SRC_FILES = $(shell find $(SRC_PATH) -type f -name '*.c')
OBJ_FILES = $(patsubst $(SRC_PATH)/%.c,$(OBJ_PATH)/%.o, $(SRC_FILES))


.PHONY: all clean mrproper re

all: $(BIN_PATH)/$(APP_BIN) data/screenshots

data: 
	@mkdir $@
data/screenshots: data
	@mkdir -p $@


$(BIN_PATH)/$(APP_BIN): $(OBJ_FILES)
	@mkdir -p $(BIN_PATH)
	$(CC) -o $(BIN_PATH)/$(APP_BIN) $(OBJ_FILES) $(LDFLAGS)

$(OBJ_PATH)/%.o: $(SRC_PATH)/%.c
	@mkdir -p "$(@D)"
	$(CC) $(CFLAGS) -I$(INC_PATH) -c $^ -o $@


clean:
	rm -f $(OBJ_FILES) $(BIN_PATH)/$(APP_BIN)
re: clean all
mrproper: re
