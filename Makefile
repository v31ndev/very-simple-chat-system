CC = gcc
CFLAGS = -Iinclude -Wall
LDFLAGS =  -lenet -lws2_32 -lwinmm
SRCDIR = src
OBJDIR = build

SERVER_SRC = $(SRCDIR)/server.c
SENDER_SRC = $(SRCDIR)/client/sender.c
LISTENER_SRC = $(SRCDIR)/client/listener.c

EXECS = $(OBJDIR)/server $(OBJDIR)/client/sender $(OBJDIR)/client/listener

all: $(OBJDIR) $(EXECS)

$(OBJDIR)/server: $(OBJDIR)/server.o
	$(CC) $^ -o $@ $(LDFLAGS)

$(OBJDIR)/client/sender: $(OBJDIR)/client/sender.o
	$(CC) $^ -o $@ $(LDFLAGS)

$(OBJDIR)/client/listener: $(OBJDIR)/client/listener.o
	$(CC) $^ -o $@ $(LDFLAGS)

$(OBJDIR)/%.o: src/%.c | $(OBJDIR)
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJDIR):
	mkdir -p $(OBJDIR)/client

clean:
	rm -rf $(OBJDIR)

.PHONY: all clean