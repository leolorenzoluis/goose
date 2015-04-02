rm ./bin/unit-test

set -e
set -x
clear

export GTEST_DIR=./third_party/googletest/

# Build GoogleTest if not found.
if [ ! -f ./bin/libgtest.a ]
  then
  echo "Building GoogleTest library."
  mkdir -p ./bin/
  g++ \
    -isystem $GTEST_DIR/include \
    -I$GTEST_DIR \
    -pthread \
    -c $GTEST_DIR/src/gtest-all.cc

 ar -rv ./bin/libgtest.a gtest-all.o
fi

# TODO(chris): Move this to the Makefile.
echo "Building klib unit tests."
g++ \
    -I$GTEST_DIR/include \
    -I. \
    -std=c++11 \
    -pthread \
    -fno-stack-protector -Wall -Wextra \
    ./klib/strings.cpp \
    ./klib/argaccumulator.cpp \
    ./klib/argaccumulator_test.cpp \
    ./klib/type_printer.cpp \
    ./klib/type_printer_test.cpp \
    ./klib/print_test.cpp \
    ./klib/tests_main.cpp \
    ./klib/print.cpp \
    ./bin/libgtest.a \
    -o ./bin/unit-test

echo "Running."
./bin/unit-test

rm ./bin/unit-test

echo "Building kernel unit tests."
g++ \
    -I$GTEST_DIR/include \
    -I. \
    -std=c++11 \
    -pthread \
    -fno-stack-protector -Wall -Wextra \
    ./kernel/memory.cpp \
    ./kernel/memory_test.cpp \
    ./kernel/tests_main.cpp \
    ./bin/libgtest.a \
    -o ./bin/unit-test

echo "Running."
./bin/unit-test