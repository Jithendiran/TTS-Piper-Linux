// gcc alsa.c -lasound -o alsa.o
#include <alsa/asoundlib.h>

const static char *device = "default";                     // playback device
const static unsigned int latency = 500000;                // 0.5sec
const static unsigned int playback_time_in_millisec = 750; // 750ms

const static unsigned short channel =  1;//2;//1;                   // mono
const static unsigned short sample_size = 2;//4;                // 2 for S16
const static unsigned int sample_rate = 22050;//44100;//22050;             // rate

const static unsigned int frame_size = sample_size * channel;

int main(void)
{
    int err;
    unsigned int i;
    snd_pcm_t *handle;
    snd_pcm_sframes_t frames;

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
    int readfd = open("/media/ssd/Project/TTS-Piper-Linux/poc/poc.wav", O_RDONLY);
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
    printf("Size of buffer = %d\n", playback_buff);
    while ((readval = read(readfd, buff, sizeof(buff))) > 0)
    {
        long int frame_to_write = readval / frame_size;
        frames = snd_pcm_writei(handle, buff, frame_to_write);
        fprintf(stderr, "Written : %ld\n", frames);
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

    /* pass the remaining samples, otherwise they're dropped in close */
    err = snd_pcm_drain(handle);
    if (err < 0)
        fprintf(stderr, "snd_pcm_drain failed: %s\n", snd_strerror(err));
    snd_pcm_close(handle);
    return 0;
}

/*
Todo
play buffer of 0.75 sec
pause/play
stop
*/

/*
echo 'Welcome to the world of speech synthesis!' | /opt/calibre/bin/piper/piper --model /home/jidesh/.cache/calibre/piper-voices/en_US-hfc_male-medium.onnx --config /home/jidesh/.cache/calibre/piper-voices/en_US-hfc_male-medium.onnx.json --output_file poc.wav
*/