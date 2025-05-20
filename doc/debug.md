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

## Attach the degugger if already running

### cli
In this example python program is running, that program uses the shared library

1. find the process id: 
```bash
jidesh@jidesh-MS-7E26:~/Project/TTS-Piper-Linux$ ps aux | grep python
root         822  0.0  0.1  41176 20476 ?        Ss   09:34   0:00 /usr/bin/python3 /usr/bin/networkd-dispatcher --run-startup-triggers
jidesh      3921  0.2  2.3 1219552028 360108 ?   Sl   09:37   0:07 /usr/share/code/code /home/jidesh/.vscode/extensions/ms-python.vscode-pylance-2025.5.1/dist/server.bundle.js --cancellationReceive=file:3af2b2912aad0ee45a068bba29538339a79d1619a6 --node-ipc --clientProcessId=3545
jidesh      8274  0.0  0.2  43808 31468 pts/5    S+   10:16   0:00 python3 test.py
jidesh      8289  0.0  0.2 191300 31760 pts/5    tl+  10:16   0:00 /usr/bin/python3 test.py
jidesh      8581  0.0  0.0   9216  2560 pts/6    S+   10:21   0:00 grep --color=auto python
jidesh@jidesh-MS-7E26:~/Project/TTS-Piper-Linux$ 
```

`jidesh      8289  0.0  0.2 191300 31760 pts/5    tl+  10:16   0:00 /usr/bin/python3 test.py` this is process i'm looking for, note the process number `8289`. select the process with `/usr/bin/python3 test.py`

don't select `jidesh      8274  0.0  0.2  43808 31468 pts/5    S+   10:16   0:00 python3 test.py`, it is not working

2. `sudo gdb -p 8289` execute this, required sudo permission

3. check if shared lib is loaded or not `info sharedlibrary`

4. Set break point and do continue


```gdb
Attaching to process 8289
[New LWP 8304]
[Thread debugging using libthread_db enabled]
Using host libthread_db library "/lib/x86_64-linux-gnu/libthread_db.so.1".
0x000074e53d71b63d in __GI___select (nfds=0, readfds=0x0, writefds=0x0, exceptfds=0x0, timeout=0x7ffe005ea5a0) at ../sysdeps/unix/sysv/linux/select.c:69
69	../sysdeps/unix/sysv/linux/select.c: No such file or directory.
(gdb) b Piper
Piper.cpp                                                      Piper::interrupt()
Piper.hpp                                                      Piper::is_completed()
Piper::Piper(char const*, char const*, char const**, int)      Piper::is_started()
Piper::Piper(char const*, char const*, char const**, int)@plt  Piper::read(char*, long)
Piper::can_read()                                              Piper::write(char const*)
Piper::check_for_word(char*, char*) const                      Piper::~Piper()
Piper::check_for_word(char*, char*) const@plt                  Piper::~Piper()@plt
Piper::init()                                                  
(gdb) b Piper::can_read() 
Breakpoint 1 at 0x74e53cdcd372: file Piper.cpp, line 134.
(gdb) b Piper::write(char const*) 
Breakpoint 2 at 0x74e53cdcd520: file Piper.cpp, line 157.
(gdb)  info sharedlibrary
From                To                  Syms Read   Shared Object Library
0x000074e53d98c3a0  0x000074e53da078c8  Yes         /lib/x86_64-linux-gnu/libm.so.6
0x000074e53d951290  0x000074e53d96fbb7  Yes (*)     /lib/x86_64-linux-gnu/libexpat.so.1
0x000074e53d933280  0x000074e53d943c14  Yes (*)     /lib/x86_64-linux-gnu/libz.so.1
0x000074e53d628700  0x000074e53d7ba93d  Yes         /lib/x86_64-linux-gnu/libc.so.6
0x000074e53da7f090  0x000074e53daa8315  Yes         /lib64/ld-linux-x86-64.so.2
0x000074e53da697c0  0x000074e53da6f6c5  Yes (*)     /usr/lib/python3.10/lib-dynload/_json.cpython-310-x86_64-linux-gnu.so
0x000074e53d82b0a0  0x000074e53d82b1d7  Yes (*)     /usr/lib/python3.10/lib-dynload/_contextvars.cpython-310-x86_64-linux-gnu.so
0x000074e53cb93850  0x000074e53cb9d252  Yes (*)     /usr/lib/python3.10/lib-dynload/_ssl.cpython-310-x86_64-linux-gnu.so
0x000074e53cafcd70  0x000074e53cb53f7e  Yes (*)     /lib/x86_64-linux-gnu/libssl.so.3
0x000074e53c6b4000  0x000074e53c910e22  Yes (*)     /lib/x86_64-linux-gnu/libcrypto.so.3
0x000074e53d5975a0  0x000074e53d59920b  Yes (*)     /usr/lib/python3.10/lib-dynload/_bz2.cpython-310-x86_64-linux-gnu.so
0x000074e53caca280  0x000074e53cad6563  Yes (*)     /lib/x86_64-linux-gnu/libbz2.so.1.0
0x000074e53d58c920  0x000074e53d58f891  Yes (*)     /usr/lib/python3.10/lib-dynload/_lzma.cpython-310-x86_64-linux-gnu.so
0x000074e53caa03c0  0x000074e53caba0de  Yes (*)     /lib/x86_64-linux-gnu/liblzma.so.5
0x000074e53d585090  0x000074e53d585931  Yes         /home/jidesh/.local/lib/python3.10/site-packages/markupsafe/_speedups.cpython-310-x86_64-linux-gnu.so
0x000074e53d580040  0x000074e53d580105  Yes         /lib/x86_64-linux-gnu/libpthread.so.0
0x000074e53ca90cc0  0x000074e53ca95fdb  Yes (*)     /usr/lib/python3.10/lib-dynload/_hashlib.cpython-310-x86_64-linux-gnu.so
0x000074e53d57b140  0x000074e53d57b42d  Yes (*)     /usr/lib/python3.10/lib-dynload/_opcode.cpython-310-x86_64-linux-gnu.so
0x000074e53cddc7a0  0x000074e53cdef0c3  Yes (*)     /usr/lib/python3.10/lib-dynload/_decimal.cpython-310-x86_64-linux-gnu.so
0x000074e53ca622c0  0x000074e53ca823e7  Yes (*)     /lib/x86_64-linux-gnu/libmpdec.so.3
0x000074e53d5760e0  0x000074e53d576229  Yes (*)     /usr/lib/python3.10/lib-dynload/_uuid.cpython-310-x86_64-linux-gnu.so
0x000074e53ca56540  0x000074e53ca59cbe  Yes (*)     /lib/x86_64-linux-gnu/libuuid.so.1
0x000074e53be55500  0x000074e53be63ba9  Yes (*)     /usr/lib/python3.10/lib-dynload/_ctypes.cpython-310-x86_64-linux-gnu.so
0x000074e53ca49460  0x000074e53ca4f392  Yes (*)     /lib/x86_64-linux-gnu/libffi.so.8
0x000074e53cdcbc60  0x000074e53cdcec22  Yes         /home/jidesh/Project/TTS-Piper-Linux/build/libttspiper.so
0x000074e53bca2420  0x000074e53bdaafc2  Yes (*)     /lib/x86_64-linux-gnu/libstdc++.so.6
0x000074e53bbe3660  0x000074e53bbf9805  Yes (*)     /lib/x86_64-linux-gnu/libgcc_s.so.1
0x000074e53cdc14a0  0x000074e53cdc1ed9  Yes (*)     /usr/lib/python3.10/lib-dynload/termios.cpython-310-x86_64-linux-gnu.so
0x000074e53be476e0  0x000074e53be49ad6  Yes (*)     /usr/lib/python3.10/lib-dynload/mmap.cpython-310-x86_64-linux-gnu.so
0x000074e53cdb94e0  0x000074e53cdba65c  Yes (*)     /usr/lib/python3.10/lib-dynload/_multiprocessing.cpython-310-x86_64-linux-gnu.so
(*): Shared library is missing debugging information.
(gdb) info functions Pip
Piper                                                              Piper::is_completed()
Piper::Piper(char const*, char const*, char const**, int)          Piper::is_started()
Piper::Piper(char const*, char const*, char const**, int)@got.plt  Piper::read(char*, long)
Piper::Piper(char const*, char const*, char const**, int)@plt      Piper::write(char const*)
Piper::can_read()                                                  Piper::~Piper()
Piper::check_for_word(char*, char*) const                          Piper::~Piper()@got.plt
Piper::check_for_word(char*, char*) const@got.plt                  Piper::~Piper()@plt
Piper::check_for_word(char*, char*) const@plt                      pipe2@got[plt]
Piper::init()                                                      pipe@got[plt]
Piper::interrupt()                                                 
(gdb) info functions Piper
Piper                                                              Piper::interrupt()
Piper::Piper(char const*, char const*, char const**, int)          Piper::is_completed()
Piper::Piper(char const*, char const*, char const**, int)@got.plt  Piper::is_started()
Piper::Piper(char const*, char const*, char const**, int)@plt      Piper::read(char*, long)
Piper::can_read()                                                  Piper::write(char const*)
Piper::check_for_word(char*, char*) const                          Piper::~Piper()
Piper::check_for_word(char*, char*) const@got.plt                  Piper::~Piper()@got.plt
Piper::check_for_word(char*, char*) const@plt                      Piper::~Piper()@plt
Piper::init()                                                      
(gdb) info functions Piper::can_read() 
All functions matching regular expression "Piper::can_read()":

File Piper.cpp:
132:	bool Piper::can_read();
(gdb) c
Continuing.
[New Thread 0x74e53b2de640 (LWP 8413)]
[Switching to Thread 0x74e53b2de640 (LWP 8413)]

Thread 3 "python3" hit Breakpoint 2, Piper::write (this=0x580babef9660, text_data=0x74e53bb764e0 "{'text': 'Hi hello i am super man.'}\n") at Piper.cpp:157
157	    int len = strlen(text_data);
(gdb) 

```

### vscode
Debug with launch.json and select `Attach to Process (GDB)`