# Dubug the shared library

Build TTS-Piper-Linux `make debug`, This will build entire files with debug option

>[!TIP]  
>for vscode debug, place break points in c++ TTS files before starting debugger 

## Debug from test_api.c file

1. pwd `...TTS-Piper-Linux/test`, build `make debug`
### cli
2. $ LD_LIBRARY_PATH=../build gdb --args ./build/test_api.out
```gdb
Type "apropos word" to search for commands related to "word"...
--Type <RET> for more, q to quit, c to continue without paging--c
Reading symbols from ./build/test_api.out...
(gdb) b main 
Breakpoint 1 at 0x12b8: file test_api.c, line 4.
(gdb) start
Temporary breakpoint 2 at 0x12b8: file test_api.c, line 4.
Starting program: /home/jidesh/Project/TTS-Piper-Linux/test/build/test_api.out 
[Thread debugging using libthread_db enabled]
Using host libthread_db library "/lib/x86_64-linux-gnu/libthread_db.so.1".

Breakpoint 1, main () at test_api.c:4
4       int main() {
(gdb) b Piper::init()
```
### vscode
2. open `test_api.c` now choose debug with launch.json and select `C Debug test_api (GDB)`

## Debug from test.cpp file

1. pwd `...TTS-Piper-Linux/test`, build `make debug`
### cli
2. $ LD_LIBRARY_PATH=../build gdb --args ./build/test.out
```gdb
Type "apropos word" to search for commands related to "word"...
Reading symbols from ./build/test.out...
(gdb) b main
Breakpoint 1 at 0x133b: file test.cpp, line 16.
(gdb) start
Temporary breakpoint 2 at 0x133b: file test.cpp, line 16.
Starting program: /home/jidesh/Project/TTS-Piper-Linux/test/build/test.out 
[Thread debugging using libthread_db enabled]
Using host libthread_db library "/lib/x86_64-linux-gnu/libthread_db.so.1".

Breakpoint 1, main () at test.cpp:16
16      {
(gdb) b Piper::in
Piper::init()       Piper::interrupt()  
(gdb) b Piper::init() 
Breakpoint 3 at 0x7ffff7fb50c8: file Piper.cpp, line 58.
(gdb) c
Continuing.
```
### vscode
2. open `test.cpp` now choose debug with launch.json and select `Cpp Debug test(GDB)`

## Debug from test.py

### cli
$ gdb --args python3 test.py 
```gdb
Type "apropos word" to search for commands related to "word"...
Reading symbols from python3...
(No debugging symbols found in python3)
(gdb) set stop-on-solib-events 1
(gdb) run
Starting program: /usr/bin/python3 test/test.py
Stopped due to shared library event (no libraries added or removed)
(gdb) c
Continuing.
[Thread debugging using libthread_db enabled]
Using host libthread_db library "/lib/x86_64-linux-gnu/libthread_db.so.1".
Stopped due to shared library event:
  Inferior loaded /lib/x86_64-linux-gnu/libm.so.6
    /lib/x86_64-linux-gnu/libexpat.so.1
    /lib/x86_64-linux-gnu/libz.so.1
    /lib/x86_64-linux-gnu/libc.so.6
(gdb) c
Continuing.
Stopped due to shared library event:
  Inferior loaded /usr/lib/python3.10/lib-dynload/_ctypes.cpython-310-x86_64-linux-gnu.so
(gdb) c
Continuing.
```
do continue till `libttspiper.so` is loaded
```gdb
Stopped due to shared library event:
  Inferior loaded /home/jidesh/Project/TTS-Piper-Linux/build/libttspiper.so
(gdb) b Piper::init() 
Breakpoint 1 at 0x7ffff7b930c8: file Piper.cpp, line 58.
(gdb) c
Continuing.
Stopped due to shared library event:
  Inferior loaded /lib/x86_64-linux-gnu/libstdc++.so.6
    /lib/x86_64-linux-gnu/libgcc_s.so.1
(gdb) c
Continuing.

Breakpoint 1, Piper::init (this=0x555555b45900) at Piper.cpp:58
58      {
(gdb) 
```

### vscode
Open `test.py` now choose debug with launch.json and select `Python with libttspiper.so (GDB)`