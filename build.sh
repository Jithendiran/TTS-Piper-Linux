# Create and move to build directory
rm -rf build
mkdir -p build && cd build

# Compile all implementation files
g++ -c ../concrete/Piper.cpp          -o Piper.o
g++ -c ../concrete/Audio.cpp          -o Audio.o
g++ -c ../controller/TTSController.cpp -o TTSController.o

# Create static library
ar rcs libttspiper.a Piper.o Audio.o TTSController.o

# Compile and link test file
g++ ../test.cpp  -L. -lttspiper -o test
