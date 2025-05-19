#ifndef TTS_API
#define TTS_API
#include <sys/types.h>
#ifdef __cplusplus
extern "C"
{
#endif
    void *tts_create_piper(char *app_path, char *model_path, char *tts_args[], int len);
    void *tts_create_audio(char *app_path, char *args[], int len);
    void *tts_create(void *pip_obj, void *aud_obj);
    void tts_destroy(void *tts_ctrl);
    int tts_start(void *tts_ctrl);
    ssize_t tts_write(void *tts_ctrl, const char *text);
    void tts_streamAudio(void *tts_ctrl);
    int tts_is_completed(void *tts_ctrl);
    void tts_interrupt(void *tts_ctrl);
    void tts_pause(void *tts_ctrl);
    void tts_resume(void *tts_ctrl);
    void tts_stop(void *tts_ctrl);
#ifdef __cplusplus
}
#endif
#endif