include path_to_unity

CC = gcc

CFLAGS += -Wall -Wextra -O2

INCLUDES = -I $(UNITY_HOME)/src \
           -I $(UNITY_HOME)/extras/fixture/src \
           -I include

CPPFLAGS += -D _GNU_SOURCE $(INCLUDES)

SOURCES = src/parse_command_line_args.c \
          src/parse_layout_file.c \
          src/parse_data_file.c \
          src/err_msg.c

TEST_SOURCES = test/test_parse_command_line_args.c \
               test/test_runners/test_parse_command_line_args_runner.c \
               test/test_parse_layout_file.c \
               test/test_runners/test_parse_layout_file_runner.c \
               test/test_parse_data_file.c \
               test/test_runners/test_parse_data_file_runner.c \
               test/test_runners/all_tests.c

LIBUNITY = build/libunity.a

LDFLAGS += $(LIBUNITY)

OBJECTS = $(subst .c,.o,$(SOURCES))

TEST_OBJECTS = $(subst .c,.o,$(TEST_SOURCES))

TESTS = test/test_runners/all_tests

.PHONY: all test clean
all: $(TESTS) test r3shuffle
test: test/test_runners/all_tests.out
test/tmp:
	mkdir -p $@
clean:
	rm -f r3shuffle $(TESTS) test/test_runners/all_tests.out $(OBJECTS) $(TEST_OBJECTS)
	rm -rf test/tmp

# ==== parse_command_line_args =======================================

src/parse_command_line_args.o: src/parse_command_line_args.c \
                               include/parse_command_line_args.h \
                               include/err_msg.h

test/test_parse_command_line_args.o: test/test_parse_command_line_args.c

test/test_runners/test_parse_command_line_args_runner.o: \
        test/test_runners/test_parse_command_line_args_runner.c

# ==== parse_layout_file =============================================

src/parse_layout_file.o: src/parse_layout_file.c \
        include/parse_layout_file.h \
        include/parse_command_line_args.h \
        include/err_msg.h

test/test_parse_layout_file.o: test/test_parse_layout_file.c

test/test_runners/test_parse_layout_file_runner.o: \
        test/test_runners/test_parse_layout_file_runner.c

# ==== test_parse_data_file ==========================================

src/parse_data_file.o: src/parse_data_file.c \
        include/parse_data_file.h \
        include/parse_layout_file.h \
        include/parse_command_line_args.h \
        include/err_msg.h

test/test_parse_data_file.o: test/test_parse_data_file.c

test/test_runners/test_parse_data_file_runner.o: \
        test/test_runners/test_parse_data_file_runner.c

# ==== err_msg =======================================================

src/err_msg.o: src/err_msg.c include/err_msg.h

# ==== all_tests =====================================================

test/test_runners/all_tests.out: test/test_runners/all_tests \
        | test/tmp
	test/test_runners/all_tests | tee $@

test/test_runners/all_tests: $(OBJECTS) $(TEST_OBJECTS) $(LIBUNITY)

test/test_runners/all_tests.o: test/test_runners/all_tests.c

# ==== unity library =================================================
$(LIBUNITY): build/unity.o build/unity_fixture.o
	$(AR) $(ARFLAGS) $@ $^

build/unity.o: $(UNITY_HOME)/src/unity.c
	$(COMPILE.c) -o $@ $<

build/unity_fixture.o: $(UNITY_HOME)/extras/fixture/src/unity_fixture.c
	$(COMPILE.c) -o $@ $<

# ==== r3shuffle =====================================================

r3shuffle: src/main.c $(OBJECTS)
	$(LINK.c) -o $@ $^
