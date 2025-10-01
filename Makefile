CC=gcc
FLAGS= -Wall # enable all  warnings

MYNAME=finch

FLAGS += -g # this flag enables debugging symbols to be included

FLAGS += -std=c99
# use asan to check for address/memory access issues (turn off for release)
# FLAGS += -fsanitize=address -fno-omit-frame-pointer

LFLAGS+= -lm  -lpng # include math library, libm

-include Makefile.sdlflags

OUTDIR = build
DEPDIR = build/deps

TEST_PROG=$(MYNAME)_test
VISUAL_TEST_PROG=visual_test
VISUAL_INTEGRATION_TEST_PROG=visual_integration_test
SCREENSHOT_GEN=screenshot_generator
MAIN_PROG=$(MYNAME)
LIB=lib$(MYNAME).a

all: test build

build: $(MAIN_PROG) $(TEST_PROG) $(VISUAL_TEST_PROG) $(VISUAL_INTEGRATION_TEST_PROG)

test: tests
tests: $(TEST_PROG)
	./$(TEST_PROG)

run_visual_test: $(VISUAL_TEST_PROG)
	./$(VISUAL_TEST_PROG)

run_visual_integration_test: $(VISUAL_INTEGRATION_TEST_PROG)
	./$(VISUAL_INTEGRATION_TEST_PROG)

generate_reference_images: $(VISUAL_INTEGRATION_TEST_PROG)
	@echo "Generating reference images..."
	@mkdir -p test_references
	./$(VISUAL_INTEGRATION_TEST_PROG)
	@cp visual_test_*.png test_references/
	@echo "✓ Reference images saved to test_references/"

COMMON_SRCS = finch.c blit.c font.c
LIB_SRCS :=  $(COMMON_SRCS) sound.c sdl2main.c
TEST_SRCS := finch_test.c
VISUAL_TEST_SRCS := visual_test.c
VISUAL_INTEGRATION_TEST_SRCS := visual_integration_test.c
SCREENSHOT_GEN_SRCS := screenshot_generator.c
MAIN_SRCS := finch_main.c

TEST_OBJS := $(addprefix $(OUTDIR)/,$(TEST_SRCS:.c=.o))
VISUAL_TEST_OBJS := $(addprefix $(OUTDIR)/,$(VISUAL_TEST_SRCS:.c=.o))
VISUAL_INTEGRATION_TEST_OBJS := $(addprefix $(OUTDIR)/,$(VISUAL_INTEGRATION_TEST_SRCS:.c=.o))
SCREENSHOT_GEN_OBJS := $(addprefix $(OUTDIR)/,$(SCREENSHOT_GEN_SRCS:.c=.o))
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
	rm -f *.o $(OBJS) $(TEST_PROG) $(VISUAL_TEST_PROG) $(VISUAL_INTEGRATION_TEST_PROG) $(SCREENSHOT_GEN) $(MAIN_PROG) $(LIB)
	rm -f .depend gmon.out core visual_test_output.png visual_test_*.png
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

$(VISUAL_TEST_PROG): $(VISUAL_TEST_OBJS) $(LIB)
	$(CC) $(FLAGS) $(VISUAL_TEST_OBJS) $(LIB) -o $(VISUAL_TEST_PROG) $(LFLAGS)

$(VISUAL_INTEGRATION_TEST_PROG): $(VISUAL_INTEGRATION_TEST_OBJS) $(LIB)
	$(CC) $(FLAGS) $(VISUAL_INTEGRATION_TEST_OBJS) $(LIB) -o $(VISUAL_INTEGRATION_TEST_PROG) $(LFLAGS)

$(SCREENSHOT_GEN): $(SCREENSHOT_GEN_OBJS) $(LIB)
	$(CC) $(FLAGS) $(SCREENSHOT_GEN_OBJS) $(LIB) -o $(SCREENSHOT_GEN) $(LFLAGS)

.PHONY: docs
docs: $(SCREENSHOT_GEN)
	@echo "Generating documentation screenshots..."
	@mkdir -p docs/images
	./$(SCREENSHOT_GEN)
	@echo "✓ Documentation ready in docs/"

.PHONY: preview preview-docs
preview: preview-docs
preview-docs:
	@echo "Starting documentation preview server..."
	@echo "Visit: http://localhost:8000"
	@cd docs && python3 -m http.server 8000

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
