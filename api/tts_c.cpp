#include "Piper.hpp"
#include "Audio.hpp"
#include "TTSController.hpp"
#include "tts_c.h"

extern "C"
{
    void *tts_create_piper(char *app_path, char *model_path, char *tts_args[], int len)
    {
        return new Piper(app_path, model_path, (const char **)tts_args, len);
    }

    void *tts_create_audio(char *app_path, char *args[], int len)
    {
        return new Audio(app_path, (const char **)args, len);
    }

    void *tts_create(void *pip_obj, void *aud_obj)
    {
        return new TTSController(static_cast<Piper *>(pip_obj), static_cast<Audio *>(aud_obj));
    }

    void tts_destroy(void *tts_ctrl)
    {
        delete static_cast<TTSController *>(tts_ctrl);
    }

    int tts_start(void *tts_ctrl) {
        return  static_cast<TTSController *>(tts_ctrl)->start();
    }

    ssize_t tts_write(void *tts_ctrl, const char *text)
    {
        return static_cast<TTSController *>(tts_ctrl)->write(text);
    }

    void tts_streamAudio(void *tts_ctrl)
    {
        static_cast<TTSController *>(tts_ctrl)->streamAudio();
    }

    int tts_is_completed(void *tts_ctrl)
    {
        return static_cast<TTSController *>(tts_ctrl)->is_completed();
    }

    void tts_interrupt(void *tts_ctrl)
    {
        static_cast<TTSController *>(tts_ctrl)->interrupt();
    }

    void tts_pause(void *tts_ctrl)
    {
        static_cast<TTSController *>(tts_ctrl)->pause();
    }

    void tts_resume(void *tts_ctrl)
    {
        static_cast<TTSController *>(tts_ctrl)->resume();
    }

    void tts_stop(void *tts_ctrl)
    {
        static_cast<TTSController *>(tts_ctrl)->stop();
    }
}