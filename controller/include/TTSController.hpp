#ifndef CONTROLLER_HPP
#define CONTROLLER_HPP
#include <thread>
#include "core.hpp"
class TTSController{
    Itts *tts;
    IAudio *audio;
    bool is_interrupt = false;

    public:
    TTSController(Itts *ttsEngine, IAudio *audioEngine);
    bool start();
    bool write(const char *text);
    void streamAudio();
    bool is_completed();
    void playAudio();
    void set_interrupt(bool is_interrupt);
    bool is_interrupted();
    void interrupt();
    void pause();
    void resume();
    void stop();
    ~TTSController();
};
#endif