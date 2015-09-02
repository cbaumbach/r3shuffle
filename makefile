include path_to_unity

CFLAGS = -Wall -Wextra -g -D_GNU_SOURCE

UNITY_INCLUDES = -I$(UNITY_HOME)/src \
                 -I$(UNITY_HOME)/extras/fixture/src

UNITY_SOURCES = $(UNITY_HOME)/src/unity.c \
                $(UNITY_HOME)/extras/fixture/src/unity_fixture.c

SOURCES = src/parse_command_line_args.c \
          src/parse_layout_file.c \
          src/parse_data_file.c \
          src/err_msg.c \
          test/test_parse_command_line_args.c \
          test/test_runners/test_parse_command_line_args_runner.c \
          test/test_parse_layout_file.c \
          test/test_runners/test_parse_layout_file_runner.c \
          test/test_parse_data_file.c \
          test/test_runners/test_parse_data_file_runner.c \
          test/test_runners/all_tests.c

OBJECTS = $(subst .c,.o,$(SOURCES))

TESTS = test/test_runners/all_tests

.PHONY: all test clean
all: $(TESTS) test
test: test/test_runners/all_tests.out
test/tmp:
	mkdir -p $@
clean:
	rm -f $(TESTS) test/test_runners/all_tests.out $(OBJECTS)
	rm -rf test/tmp

# ==== parse_command_line_args =======================================

src/parse_command_line_args.o: src/parse_command_line_args.c \
                               src/parse_command_line_args.h \
                               src/err_msg.h
	gcc $(CFLAGS) -Isrc -c -o $@ $<

test/test_parse_command_line_args.o: test/test_parse_command_line_args.c
	gcc $(CFLAGS) $(UNITY_INCLUDES) -Isrc -c -o $@ $<

test/test_runners/test_parse_command_line_args_runner.o: \
        test/test_runners/test_parse_command_line_args_runner.c
	gcc $(CFLAGS) $(UNITY_INCLUDES) -Isrc -c -o $@ $<

# ==== parse_layout_file =============================================

src/parse_layout_file.o: src/parse_layout_file.c \
        src/parse_layout_file.h \
        src/err_msg.h
	gcc $(CFLAGS) -Isrc -c -o $@ $<

test/test_parse_layout_file.o: test/test_parse_layout_file.c
	gcc $(CFLAGS) $(UNITY_INCLUDES) -Isrc -c -o $@ $<

test/test_runners/test_parse_layout_file_runner.o: \
        test/test_runners/test_parse_layout_file_runner.c
	gcc $(CFLAGS) $(UNITY_INCLUDES) -Isrc -c -o $@ $<

# ==== test_parse_data_file ==========================================

src/parse_data_file.o: src/parse_data_file.c \
        src/parse_data_file.h \
        src/err_msg.h
	gcc $(CFLAGS) -Isrc -c -o $@ $<

test/test_parse_data_file.o: test/test_parse_data_file.c
	gcc $(CFLAGS) $(UNITY_INCLUDES) -Isrc -c -o $@ $<

test/test_runners/test_parse_data_file_runner.o: \
        test/test_runners/test_parse_data_file_runner.c
	gcc $(CFLAGS) $(UNITY_INCLUDES) -Isrc -c -o $@ $<

# ==== err_msg =======================================================

src/err_msg.o: src/err_msg.c src/err_msg.h
	gcc $(CFLAGS) -Isrc -c -o $@ $<

# ==== all_tests =====================================================

test/test_runners/all_tests.out: test/test_runners/all_tests \
        | test/tmp
	test/test_runners/all_tests | tee $@

test/test_runners/all_tests: $(OBJECTS) $(UNITY_SOURCES)
	gcc $(CFLAGS) $(UNITY_INCLUDES) -Isrc -o $@ $^

test/test_runners/all_tests.o: test/test_runners/all_tests.c
	gcc $(CFLAGS) $(UNITY_INCLUDES) -Isrc -c -o $@ $<
