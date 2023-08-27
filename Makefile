CC=gcc
FLAGS= -Wall # enable all  warnings

MYNAME=finch

FLAGS += -g # this flag enables debugging symbols to be included

FLAGS += -std=c99
# use asan to check for address/memory access issues (turn off for release)
# FLAGS += -fsanitize=address -fno-omit-frame-pointer

LFLAGS+= -lm  -lpng # include math library, libm

include Makefile.sdlflags

OUTDIR = build
DEPDIR = build/deps

TEST_PROG=$(MYNAME)_test
MAIN_PROG=$(MYNAME)
LIB=lib$(MYNAME).a

all: test $(MAIN_PROG)

COMMON_SRCS = finch.c blit.c
LIB_SRCS :=  $(COMMON_SRCS) sound.c sdl2main.c
TEST_SRCS := finch_test.c
MAIN_SRCS := finch_main.c

TEST_OBJS := $(addprefix $(OUTDIR)/,$(TEST_SRCS:.c=.o))
LIB_OBJS := $(addprefix $(OUTDIR)/,$(LIB_SRCS:.c=.o))
MAIN_OBJS := $(addprefix $(OUTDIR)/,$(MAIN_SRCS:.c=.o))

.PHONY: print
print:
	@echo PROG=$(MYNAME)
	@echo TEST_PROG=$(MYNAME)_test
	@echo FLAGS=$(FLAGS)
	@echo LFLAGS=$(LFLAGS)
	@echo COMMON_SRCS=$(COMMON_SRCS)
	@echo TEST_SRCS=$(TEST_SRCS)
	@echo TEST_OBJS=$(TEST_OBJS)
	@echo LIB_OBJS=$(TEST_OBJS)
	@echo MAIN_OBJS=$(TEST_OBJS)
	@echo OUTDIR=$(OUTDIR)
	@echo DEPDIR=$(DEPDIR)

clean:
	rm -f *.o $(OBJS) $(TEST_PROG) $(MAIN_PROG) $(LIB)
	rm -f .depend gmon.out core
	rm -Rf build/*.o
	rm -Rf build/deps/*
	rm -Rf build/installer/

$(LIB): $(LIB_OBJS)
	$(AR) r $@ $(LIB_OBJS)
	-@ ($(RANLIB) $@ || true) >/dev/null 2>&1

$(MAIN_PROG): $(MAIN_OBJS) $(LIB)
	$(CC) $(FLAGS) $(MAIN_OBJS) $(LIB) -o $(MAIN_PROG) $(SDL_LFLAGS)

$(TEST_PROG): $(TEST_OBJS) $(LIB)
	$(CC) $(FLAGS) $(TEST_OBJS) $(LIB) -o $(TEST_PROG) $(LFLAGS)

test: tests
tests: $(TEST_PROG)
	./$(TEST_PROG)

force:

$(OUTDIR)/%.o : %.c
	@echo CC ----- $*.c -----
	@test -d $(@D) || mkdir -p $(@D)
	@test -d $(DEPDIR) || mkdir -p $(DEPDIR)
	$(CC) $(FLAGS) -c $*.c -o $(OUTDIR)/$*.o
	@gcc -E -MM -MT $(OUTDIR)/$*.o $(FLAGS) $*.c > $(DEPDIR)/$(notdir $*.d)

DEPINC := $(shell ls $(DEPDIR)/*.d 2>/dev/null)

ifneq ($(DEPINC),)
include $(DEPINC)
endif
