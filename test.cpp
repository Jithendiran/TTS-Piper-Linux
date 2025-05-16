#include <cstring>
#include <spawn.h>
#include <unistd.h>
#include <errno.h>
#include <cstdio>
#include <signal.h>
#include <iostream>
#include <fcntl.h>
#include <sys/wait.h>

#include "concrete/Piper.hpp"
#include "concrete/Audio.hpp"
#include "controller/TTSController.hpp"

int main()
{
    const char *tts_args[] = {
        "--config",
        "/home/jidesh/.cache/calibre/piper-voices/en_US-hfc_male-medium.onnx.json",
        "--output-raw", "--json-input", "--sentence-silence 0.1", "--length_scale 1.2",
        NULL};
    char *text[] = {
        "{\"text\": \"Hi hello i am super man.\"}\n", 
        "{\"text\": \"Hi hello you  man.\"}\n", 
        // "{\"text\": \"A week ago a friend invited a couple of other couples over for dinner. Eventually, the food (but not the wine) was cleared off the table for what turned out to be some fierce Scrabbling. Heeding the strategy of going for the shorter, more valuable word over the longer cheaper word, our final play was Bon, which–as luck would have it!–happens to be a Japanese Buddhist festival, and not, as I had originally asserted while laying the tiles on the board, one half of a chocolate-covered cherry treat. Anyway, the strategy worked. My team only lost by 53 points instead of 58.\"}\n",
        // "{\"text\": \"Hi hello you  man.\"}\n", 
        NULL};
    Piper *tts = new Piper(
        "/opt/calibre/bin/piper/piper",
        "/home/jidesh/.cache/calibre/piper-voices/en_US-hfc_male-medium.onnx",
        tts_args,
        7);
    const char *alsa_args[] = {"-f", "S16_LE", "-r", "22050", "-q", NULL};
    Audio *audio = new Audio("aplay", alsa_args, 5);

    TTSController controller(tts, audio);
    controller.start();
    char **add = text;
    while(*add) {
        cout<< "playing :  "<<*add;

        controller.write(*add);
        controller.streamAudio();
        while (!controller.is_completed())
        {
        }
        cout << "completed \n";
        // signal done
        *add++;
    }
    controller.stop();
    
    delete tts;
    delete audio;
    return 0;
}