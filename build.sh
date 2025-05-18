# Create and move to build directory
rm -rf build
mkdir -p build && cd build

# Compile all implementation files
g++ -fPIC -c ../concrete/Piper.cpp          -o Piper.o
g++ -fPIC -c ../concrete/Audio.cpp          -o Audio.o
g++ -fPIC -c ../controller/TTSController.cpp -o TTSController.o

# Create static library
# ar rcs libttspiper.a Piper.o Audio.o TTSController.o

# Create shared library
g++ -shared -o libttspiper.so Piper.o Audio.o TTSController.o


# Compile and link test file
g++ ../test.cpp  -L. -lttspiper -o test

export LD_LIBRARY_PATH=.:$LD_LIBRARY_PATH