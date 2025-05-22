#include <stdio.h>
#include "tts_c.h"

int main() {
     const char *tts_args[] = {
        "--config",
        "/home/jidesh/.cache/calibre/piper-voices/en_US-hfc_male-medium.onnx.json",
        "--output-raw", "--json-input", "--sentence-silence 0.1", "--length_scale 1.2",
        NULL};
    char *text[] = {
        "{\"text\": \"Hi hello i am super man.\"}\n", 
        "{\"text\": \"Hi hello you  man.\"}\n", 
        "{\"text\": \"A week ago a friend invited a couple of other couples over for dinner. Eventually, the food (but not the wine) was cleared off the table for what turned out to be some fierce Scrabbling. Heeding the strategy of going for the shorter, more valuable word over the longer cheaper word, our final play was Bon, which–as luck would have it!–happens to be a Japanese Buddhist festival, and not, as I had originally asserted while laying the tiles on the board, one half of a chocolate-covered cherry treat. Anyway, the strategy worked. My team only lost by 53 points instead of 58.\"}\n",
        // "{\"text\": \"Hi hello you  man.\"}\n", 
        NULL};
    
    void* piper = tts_create_piper(
        "/opt/calibre/bin/piper/piper",
        "/home/jidesh/.cache/calibre/piper-voices/en_US-hfc_male-medium.onnx",
        tts_args,
        7,
        0
    );
    const char *alsa_args[] = {"-f", "S16_LE", "-r", "22050", "-q", NULL};
    void* audio = tts_create_audio("aplay", alsa_args, 5, 0);

    void* tts_ctrl = tts_create(piper, audio);

    tts_start(tts_ctrl);
    char **add = text;
    while(*add) {
        printf("playing :  %s\n",*add);

        tts_write(tts_ctrl, *add);
        tts_streamAudio(tts_ctrl);
        while (!tts_is_completed(tts_ctrl))
        {
        }
        printf("completed\n");
        *add++;
    }
    tts_stop(tts_ctrl);

    tts_destroy(tts_ctrl);
}

// gcc test_api.c -lttspiper -L../build -o test_api.out