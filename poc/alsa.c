// gcc alsa.c -lasound -lpthread -o alsa.o && export LIBASOUND_DEBUG=1 && ./alsa.o
#include <alsa/asoundlib.h>
#include <pthread.h>

const static char *device = "default";                     // playback device
const static unsigned int latency = 500000;                // 0.5sec
const static unsigned int playback_time_in_millisec = 750; // 750ms

const static unsigned short channel = 1;       // mono
const static unsigned short sample_size = 2;   // 2 for S16
const static unsigned int sample_rate = 22050; // rate

const static unsigned int frame_size = sample_size * channel;

snd_pcm_t *handle;
char not_play = 0;

void print_pcm_state(snd_pcm_t *handle)
{
    switch (snd_pcm_state(handle))
    {
    case SND_PCM_STATE_OPEN:
        fprintf(stderr, "State: OPEN\n");
        break;
    case SND_PCM_STATE_SETUP:
        fprintf(stderr, "State: SETUP\n");
        break;
    case SND_PCM_STATE_PREPARED:
        fprintf(stderr, "State: PREPARED\n");
        break;
    case SND_PCM_STATE_RUNNING:
        fprintf(stderr, "State: RUNNING\n");
        break;
    case SND_PCM_STATE_XRUN:
        fprintf(stderr, "State: XRUN\n");
        break;
    case SND_PCM_STATE_DRAINING:
        fprintf(stderr, "State: DRAINING\n");
        break;
    case SND_PCM_STATE_PAUSED:
        fprintf(stderr, "State: PAUSED\n");
        break;
    case SND_PCM_STATE_SUSPENDED:
        fprintf(stderr, "State: SUSPENDED\n");
        break;
    default:
        fprintf(stderr, "State: UNKNOWN\n");
        break;
    }
}

void open_handle()
{
    int err;
    if ((err = snd_pcm_open(&handle, device, SND_PCM_STREAM_PLAYBACK, 0)) < 0)
    {
        printf("Playback open error: %s\n", snd_strerror(err));
        exit(EXIT_FAILURE);
    }
    if ((err = snd_pcm_set_params(handle,
                                  SND_PCM_FORMAT_S16_LE,
                                  SND_PCM_ACCESS_RW_INTERLEAVED,
                                  channel,
                                  sample_rate,
                                  1,
                                  latency)) < 0)
    {
        printf("Playback open error: %s\n", snd_strerror(err));
        exit(EXIT_FAILURE);
    }
}

void playback()
{
    int err;
    unsigned int i;

    snd_pcm_sframes_t frames, buffer_size, period_size;

    int readfd = open("/media/ssd/Project/TTS-Piper-Linux/poc/poc-l.wav", O_RDONLY);
    if (readfd < 0)
    {
        perror("wavread");
        exit(1);
    }
    // Skip WAV header (44 bytes)
    lseek(readfd, 44, SEEK_SET);

    size_t readval = 0;
    int playback_buff = (int)(sample_rate * (playback_time_in_millisec / 1000.0)) * frame_size;
    char buff[playback_buff];
    printf("Size of buffer = %d, in Frame: %d \n", playback_buff, playback_buff / frame_size);

    if ((err = snd_pcm_get_params(handle, &buffer_size, &period_size)) < 0)
    {
        fprintf(stderr, "snd_pcm_get_params error!\n");
        exit(EXIT_FAILURE);
    }
    printf("Ring frame size = %ld\n", buffer_size);
    printf("Period frame size = %ld\n", period_size);

    while (1)
    {
        if (!not_play)
        {

            if ((readval = read(readfd, buff, sizeof(buff))) > 0)
            {
                long int frame_to_write = readval / frame_size;
                frames = snd_pcm_writei(handle, buff, frame_to_write);
                fprintf(stderr, "To write: %ld, Written : %ld\n", frame_to_write, frames);
                fprintf(stderr, "written %ld frames (buffer has %ld frames available)\n",
                        frame_to_write, snd_pcm_avail(handle));
                if (frames == -EPIPE)
                {
                    fprintf(stderr, "Underrun!\n");
                    snd_pcm_prepare(handle);
                }

                if (frames < 0)
                    frames = snd_pcm_recover(handle, frames, 0);
                else if (frames < 0)
                {
                    fprintf(stderr, "Error writing to PCM device: %s\n", snd_strerror(frames));
                    break;
                }
                if (frames > 0 && frames < frame_to_write)
                    fprintf(stderr, "Short write (expected %ld, wrote %ld)\n", frame_to_write, frames);
            }
            else
                break;
        }
    }

    /* pass the remaining samples, otherwise they're dropped in close */
    err = snd_pcm_drain(handle);
    if (err < 0)
        fprintf(stderr, "snd_pcm_drain failed: %s\n", snd_strerror(err));

    close(readfd);
    snd_pcm_close(handle);
}

int main(void)
{
    int err;
    pthread_t thread1;
    snd_pcm_hw_params_t *hw = NULL;
    char is_pausable = 0;

    open_handle();

    // Allocate the hw_params structure
    if (snd_pcm_hw_params_malloc(&hw) < 0)
    {
        fprintf(stderr, "Cannot allocate hw params\n");
        exit(1);
    }

    snd_pcm_hw_params_current(handle, hw);
    (snd_pcm_hw_params_can_pause(hw) == 1) && (is_pausable = 1);

    printf("Is pausable : %d\n", snd_pcm_hw_params_can_pause(hw));

    // Creating a new thread.
    pthread_create(&thread1, NULL, (void *(*)(void *))playback, NULL);
    sleep(5);

    if (is_pausable)
    {
        if ((err = snd_pcm_pause(handle, 1)) < 0)
        {
            fprintf(stderr, "Pausable err : %s\n", snd_strerror(err));
            is_pausable = 0;
        }
        not_play = 1;
        fprintf(stderr, "Paused -----------------------------\n");
    }

    sleep(3);
    if (is_pausable)
    {
        if ((err = snd_pcm_pause(handle, 0)) < 0)
        {
            fprintf(stderr, "Resume err : %s\n", snd_strerror(err));
            print_pcm_state(handle);
            // Try to recover
            if (err == -EAGAIN)
            {
                // Wait briefly and try again
                usleep(100000); // 100ms
                err = snd_pcm_pause(handle, 0);
                print_pcm_state(handle);
            }

            if (err < 0)
            {
                // More aggressive recovery
                snd_pcm_prepare(handle);
                err = snd_pcm_pause(handle, 0);
                print_pcm_state(handle);
            }

            if (err < 0)
            {
                fprintf(stderr, "Cannot recover from resume error: %s\n",
                        snd_strerror(err));
                print_pcm_state(handle);
                is_pausable = 0;
            }
        }
        not_play = 0;
    }

    not_play = 1;
    if ((err = snd_pcm_drop(handle)) < 0)
    {
        fprintf(stderr, "snd_pcm_drop failed: %s\n", snd_strerror(err));
    }
    fprintf(stderr, " Dropped\n");
    // print_pcm_state(handle);
    if ((err = snd_pcm_prepare(handle)) < 0)
    {
        fprintf(stderr, "Cannot prepare PCM after drop: %s\n", snd_strerror(err));
        return 0;
    }
    not_play = 0;
    print_pcm_state(handle);
    // Wait for playback thread to finish
    if (pthread_join(thread1, NULL) != 0)
    {
        fprintf(stderr, "Failed to join thread\n");
        exit(1);
    }
    snd_pcm_hw_params_free(hw);
    return 0;
}

/*
Todo
pause/play
stop
*/

/*
echo 'Welcome to the world of speech synthesis!' | /opt/calibre/bin/piper/piper --model /home/jidesh/.cache/calibre/piper-voices/en_US-hfc_male-medium.onnx --config /home/jidesh/.cache/calibre/piper-voices/en_US-hfc_male-medium.onnx.json --output_file poc.wav

echo "Welcome to the world of speech synthesis. This is a longer test to demonstrate audio buffering and playback.
In this demonstration, we will explore how ALSA handles large audio streams through its ring buffer system.
The sound card receives data in smaller chunks while maintaining smooth playback.
This helps us understand the relationship between application buffers, ring buffers, and hardware buffers.
Thank you for listening to this technical demonstration of text-to-speech synthesis." | \
/opt/calibre/bin/piper/piper \
--model /home/jidesh/.cache/calibre/piper-voices/en_US-hfc_male-medium.onnx \
--config /home/jidesh/.cache/calibre/piper-voices/en_US-hfc_male-medium.onnx.json \
--output_file poc-l.wav

echo "Welcome to this comprehensive demonstration of speech synthesis and audio processing.

Let's explore how digital audio works in computer systems. When we record sound, we capture many tiny snapshots of the audio wave thousands of times per second. These snapshots, called samples, are stored as digital values.

In our current setup, we're using a sample rate of 22,050 Hz, which means we capture 22,050 samples every second. Each sample is stored using 16 bits, allowing us to represent 65,536 different volume levels.

The ALSA sound system uses a sophisticated buffering mechanism to ensure smooth playback. It maintains both a hardware buffer and a larger ring buffer, working together like a well-orchestrated system.

When playing back audio, the system reads these samples at the same rate they were recorded, reconstructing the original sound wave through your speakers. This process happens continuously, with data flowing through various buffers and being converted back into analog signals that your ears can hear.

Thank you for listening to this detailed explanation of digital audio processing and playback systems." | \
/opt/calibre/bin/piper/piper \
--model /home/jidesh/.cache/calibre/piper-voices/en_US-hfc_male-medium.onnx \
--config /home/jidesh/.cache/calibre/piper-voices/en_US-hfc_male-medium.onnx.json \
--output_file poc-xl.wav
*/