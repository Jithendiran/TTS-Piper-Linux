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

**Frame** is a set of audio samples, one for each channel, at a single point in time 
* For mono audio (1 channel), 1 frame = 1 sample.
* For stereo audio (2 channels), 1 frame = 2 samples (one for left, one for right).

**Format** refers to the audio sample format, which defines how each audio sample is represented in memory.

```
format : S16_LE

S = Signed (samples are signed integers)
16 = 16 bits per sample (2 bytes)
LE = Little Endian (byte order)

```

**Rate** refers to the audio sample rate, which is the number of samples per second per channel.  sample rate means how often an analog signal is measured per sec during recoring.

eg:  
    rate = 22050 hz means per second 22050 samples are captured. When playing back digital audio, your system reads the samples at the same rate they were recorded  
    * Every second, the microphone captures 22050 tiny snapshots of the sound wave.  
    * These snapshots are stored as digital values (based on bit depth, e.g., 16-bit or 24-bit).  
    * When played back, your speakers reconstruct the waveform using those 44,100 points for every second of audio.  

One digital value is called sample. More samples are collected to frames (frame is terminology for ALSA) depending on count of converters used at one specific time  
    * when only one converter is used - mono  
    * when only two converter is used - stereo  

**Exact rate** The actual sample rate that the hardware or driver is using. Sometimes, the hardware cannot match the requested rate exactly, so it uses the closest possible value

Example
-------

-> If audio is mono, 16-bit, 22050 Hz:  
    - 1 sample = 2 bytes (since 16 bits = 2 bytes, 1 channel)  
    - 1 frame = 1 sample = 2 bytes  
    - For 1 second: 22050 frames × 2 bytes = 44,100 bytes  

-> If audio is stereo, 16-bit, 22050 Hz:  
    - 1 sample = 2 bytes (per channel)  
    - 1 frame = 2 samples (left + right) = 4 bytes  
    - For 1 second: 22050 frames × 4 bytes = 88,200 bytes  

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

Audio contains **sample**, sample is the smallest unit of audio. 

How much samples needs for 1 sec audio?  

    **rate** is answer  
    For 1 sec 22050 sample is needed

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
