# TTS-PIPER-LINUX System Design

This system is designed for Linux-based machines.  
This is a 2 thread process.

    Main thread monitors user activity; when entering the play state, it creates a detachable thread to handle TTS processing and audio playback.
    Detachable thread will feed input to the TTS sub process. It will then get the raw audio and play it on the audio device.

No more than 2 threads and 1 subprocess for TTS will be created.
APIs are exposed via a C/C++ shared library (.so), consumable by other apps

## Life Cycle

**Start**  

API: `bool start();`  
    return true if success, else false  
    This is a blocking operation

    Init Piper (TTS engine)  
        Piper is a child process. Accepts json as input  
    Init Alsa  (Audio system)  
        Init hardware & software parameter  

    Take the configs from json file

    wait for Piper and Alsa to start

    set the status as 0


> [!TIP]  
> User application will give text data. Piper processes the data and generates raw audio. ALSA takes the raw audio and converts it to sound

**Status**

API: `char getStatus();`  
    return -1, 0, 1  
    This is a blocking operation

    0 -> system idle.  Accept input.
    1 -> System is busy. Don't accept input. 
    -1 -> System broken. Don't accept input. Throws system broken error

*Status Transitions*

    - On successful play: status = 1 (busy)
    - On pause: status remains 1, but system is paused
    - On resume: status remains 1, system resumes playback
    - On Completion: status = 0 (idle)
    - On interrupt: status = 0 (idle) after draining/reset
    - On stop or unrecoverable error: status = -1 (broken)

**Play**

API: `bool play(string json)`    
    input json in the form of string  `{ "text": "..." }`  
    return true if success  
    This is a blocking operation  

    If status == 1 and user pushes data, the system throws a busy error.
    If status == -1 and user pushes data, the system throws a broken error.

    set status as 1

**Pause**

API: `bool pause()`  
    return true if paused, else false  
    This is a blocking operation  

    If status == 1 and not paused, it pauses the flow
    If status == 1 and paused, the system ignores the request.
    If status == 0, the system ignores the request.
    status == -1, System throws broken error 

**Resume**  

API: `bool resume()`  
    return true if resumed, else false  
    This is a blocking operation

    If status == 1 and paused, it resumes the flow
    If status == 1 and not paused, the system ignores the request.
    If status == 0, the system ignores the request.
    If status == -1, System throws broken error 

**Completion**

    After completion. Set status as 0

**Interrupt**  

API: `void interrupt()`  
    This is a blocking operation

    If status == 1, It will drain the input for TTS and audio. Immediately stop and discard
    If status == -1, the system throws a broken error.
    status == 0, System ignore 

**Stop**

API: `bool stop()`  
    return true if success else false  
    This is a blocking operation

    Kill the TTS sub process and stop the ALSA
    set the status as -1
    Resource cleanup



## TODO
* Interface class design

* Error recovery (what if Piper dies or ALSA crashed?)


## Interface design

```
enum Status {
    OK,
    Error
};
```

```
interface IReadable {
    size_t read(int fd, size_t size);
};

interface IWritable {
    size_t write(int fd, size_t size);
};
```

Piper, Alsa and TTS controler will implements this
```
interface IProcessControl {
    Status init();
    Status stop();
};
```

```
interface ICompletable {
    bool is_completed();
}
interface IMediaControl {
    Status pause();
    Status resume();
    Status interrupt();
};

```

TTS controler interface
```
enum TTSState{
    Error,
    Idle,
    Busy
}

interface ITTSController extends IProcessControl, IMediaControl {
    Status play();
    TTSState get_state();
};
```

```
interface ITTS extends IProcessControl, IMediaControl, ICompletable, IReadable, IWritable {

}

interface IAudio extends IProcessControl, IMediaControl, ICompletable, IWritable {

}

class TTSController implements  ITTSController {
    TTS 
    Audio
}

```