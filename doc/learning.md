# Sound (ALSA)

```
Aplay info
--------------
stream       : PLAYBACK
access       : RW_INTERLEAVED
format       : S16_LE
subformat    : STD
channels     : 1
rate         : 22050
exact rate   : 22050 (22050/1)
msbits       : 16
buffer_size  : 11025
period_size  : 2756
period_time  : 125000
tstamp_mode  : NONE
tstamp_type  : GETTIMEOFDAY
period_step  : 1
avail_min    : 2756
period_event : 0
start_threshold  : 11025
stop_threshold   : 11025
silence_threshold: 0
silence_size : 0
boundary     : 6206523236469964800
```

**Frame** is a set of audio samples, one for each channel, at a single point in time (It can be minutes or seconds or millisecond or ...)
* For mono audio (1 channel), 1 frame = 1 sample.
* For stereo audio (2 channels), 1 frame = 2 samples (one for left, one for right).

**Format** refers to the audio sample format, which defines how each audio sample is represented in memory.

```
format : S16_LE

S = Signed (samples are signed integers)
16 = 16 bits per sample (2 bytes)
LE = Little Endian (byte order)

```

**Rate** refers to the audio frame rate, which is the number of frames per second per channel.  frame rate means how often an analog signal is measured per sec during recoring. 1 frame is NOT equal to 1 second  , frame rate (e.g., 22050 Hz) tells you how many frames are in 1 second

eg:  
    rate = 22050 hz means per second 22050 frames are captured. When playing back digital audio, your system reads the frames at the same rate they were recorded  
    * Every second, the microphone captures 22050 tiny snapshots of the sound wave.  
    * These snapshots are stored as digital values (based on bit depth, e.g., 16-bit or 24-bit).  
    * When played back, your speakers reconstruct the waveform using those 22050 points for every second of audio.  

One digital value is called sample. More samples are collected to frames (frame is terminology for ALSA) depending on count of converters used at one specific time  
    * when only one converter is used - mono  
    * when only two converter is used - stereo  

**Exact rate** The actual frame rate that the hardware or driver is using. Sometimes, the hardware cannot match the requested rate exactly, so it uses the closest possible value

Example
-------

-> If audio is mono, 16-bit, 22050 Hz:  
    - 1 sample = 2 bytes (since 16 bits = 2 bytes, 1 channel)  
    - 1 frame = 1 sample = 2 bytes  
    - For 1 sec: 22050 frames × 2 bytes = 44,100 bytes  
    - For 1 sec 22050 frames will be captured, each frame size has 2 bytes of size
    - 1 frame play back time is  1 / 22050 ~= 0.00004535 seconds per frame

-> If audio is stereo, 16-bit, 22050 Hz:  
    - 1 sample = 2 bytes (per channel)  
    - 1 frame = 2 samples (left + right) = 4 bytes  
    - For 1 sec: 22050 frames × 4 bytes = 88,200 bytes  
    - For 1 sec 22050 frames will be captured, each frame size has 4 bytes of size
    - 1 frame play back time is  1 / 22050 ~= 0.00004535 seconds per frame

| Channel | Sample Rate | Sample Size | Frame Size | Bytes/sec |
| ------- | ----------- | ----------- | ---------- | --------- |
| Mono    | 22050       | 2 bytes     | 2 bytes    | 44,100    |
| Stereo  | 22050       | 2 bytes     | 4 bytes    | 88,200    |


------------------------
**buffer_size** The total size of the audio buffer (in frames). This is the maximum amount of audio data that can be queued for playback at once.  
**period_size** Audio is processed in chunks of this size

>[!TIP]  
> once buffer is filled buffer_size/period_size cycle is required to empty the buffer  
> Eg: buffer_size = 11025 frames, period_size = 2756 => 11025 / 2756 ≈ 4 periods to empty the buffer

**period_time** The duration (in microseconds) of one period (period_size), calculated as period_size / sample_rate * 1,000,000 = 2756/22050 = 0.124988662 * 1,000,000 = 124988.662 microseconds or about 125 ms  

**avail_min** when the buffer down to avail_min size, ALSA notifies the application to write more data.  
 eg: now buffer is full 11025  
 period         buffer left          Notes  
 1              11025 - 2756 = 8269  
 2              8269  - 2756 = 5513  
 3              5513  - 2756 = 2757  alomost ask for data  
 4              2757  - 2756 = 1     now buffer available size is below avail_min, now ALSA ask for more data  

**start_threshold** The number of frames that must be in the buffer before playback actually starts. Playback will wait until this threshold is reached.
```
In your case, start_threshold equals the full buffer size, so playback will only start when the buffer is completely full (11025 frames).  
After playback starts, each period (period_size = 2756 frames) is played, reducing the buffer.  
After the first period, 8269 frames remain. Playback continues; it does not wait to refill the buffer to start_threshold.  
ALSA will notify your application to write more data when the available space reaches avail_min (i.e., when only 2756 frames are left).
```

**stop_threshold** If the buffer drops below this, playback stops

**silence_size** The number of frames that will be filled with silence if the application does not provide enough data.

Audio contains **sample**, sample is the smallest unit of audio. sample are collected to form the frame

How much frames needs for 1 sec audio?  

    **rate** is answer  
    For 1 sec 22050 frame is needed

Buffer size of aplay is 11025 frames  

    how much memory do we need to allocate for aplay buffer?  
    buffer_size (frames) * channel * (msbits / 8) bytes
    for mono:  
        11025 * 1 * 2 = 22050 bytes
    for stero:  
        11025 * 2 * 2 = 44100 bytes  

Genaral
---------------------------
* ALSA uses the ring buffer to store outgoing (playback) and incoming (capture, record) samples.
* There are two pointers being maintained to allow a precise communication between application and device pointing to current processed sample by hardware and last processed sample by application.
* In modern audio chip the stream of samples is divided to small chunks. Device acknowledges to application when the transfer of a chunk is complete.


Time measurement
----------------------------
Time Units (from largest to smallest):

seconds (s)
milliseconds (ms) = 1/1000 second = 0.001 s
microseconds (µs) = 1/1000000 second = 0.000001 s
nanoseconds (ns) = 1/1000000000 second = 0.000000001 s

Frame Timing (at 22050 Hz)
-------------------------
1 frame playback time = 1/22050 seconds
- In seconds: 0.000045351 seconds
- In milliseconds: 0.045351 ms
- In microseconds: 45.351 µs

This means each frame (whether mono or stereo) is played for approximately:
- 45.351 microseconds
- 0.045351 milliseconds
- 0.000045351 seconds

if need buffer for playback of 750ms = (1/22050 = 0.045351) × 750 = 34.01325 is 34 frames approx, 34 * 2 (size of sample) = 68 bytes

Buffer Size Calculation Example
-----------------------------
If we need buffer for playback of 750ms:
- Frames needed = sample_rate × (time_ms/1000)
- Frames = 22050 × (750/1000) = 16538 frames

For mono (16-bit):
- Buffer size = 16538 frames × 2 bytes = 33076 bytes

For stereo (16-bit):
- Buffer size = 16538 frames × 4 bytes = 66152 bytes