/*
 *  This extra small demo sends a random samples to your speakers.
 */
// gcc alsa.c -lasound -o alsa.o
#include <alsa/asoundlib.h>

static char *device = "default";  // playback device
unsigned int sample_rate = 22050; // rate
unsigned short channel = 1;       // mono
unsigned int latency = 500000;    // 0.5sec

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
    size_t readval = 0;
    
    char buff[sample_rate];
    while ((readval = read(readfd, buff, sizeof(buff))) > 0)
    {   
        frames = snd_pcm_writei(handle, buff, readval/2);
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
        if (frames > 0 && frames < readval/2)
            fprintf(stderr,"Short write (expected %li, wrote %li)\n", readval/2, frames);
    }

    /* pass the remaining samples, otherwise they're dropped in close */
    err = snd_pcm_drain(handle);
    if (err < 0)
        fprintf(stderr,"snd_pcm_drain failed: %s\n", snd_strerror(err));
    snd_pcm_close(handle);
    return 0;
}

/*
echo 'Welcome to the world of speech synthesis!' | /opt/calibre/bin/piper/piper --model /home/jidesh/.cache/calibre/piper-voices/en_US-hfc_male-medium.onnx --config /home/jidesh/.cache/calibre/piper-voices/en_US-hfc_male-medium.onnx.json --output_file poc.wav
*/