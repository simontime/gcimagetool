CC = gcc
CFLAGS = -O3 -s -Iinclude

OBJDIR = obj

TARGET = gcimagetool

SRC = $(wildcard source/*.c)
OBJ = $(patsubst source/%.c,$(OBJDIR)/%.o,$(SRC))

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) $^ -o $@ -lm

$(OBJDIR)/%.o: source/%.c | $(OBJDIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJDIR):
	mkdir -p $(OBJDIR)

clean:
	rm -f $(OBJ) $(TARGET)
	rm -rf $(OBJDIR)

.PHONY: clean
