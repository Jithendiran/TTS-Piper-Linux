#include "TTSController.hpp"

TTSController::TTSController(Itts *ttsEngine, IAudio *audioEngine)
    : tts(ttsEngine), audio(audioEngine) {}

bool TTSController::start()
{
    if (!tts->init())
    {
        cout << "TTS init issue" << endl;
        return false;
    }

    if (!audio->init())
    {
        cout << "Audio init issue" << endl;
        return false;
    }

    // Wait until TTS is ready
    int retries = 100;
    while (!tts->is_started() && retries-- > 0)
    {
        usleep(10000); // 10ms
    }
    if (!tts->is_started())
    {
        cout << "TTS start issue" << endl;
        return false;
    }

    if (!audio->is_started())
    {
        cout << "Audio start issue" << endl;
        return false;
    }
    return true;
}

bool TTSController::write(const char *text)
{
    if (is_interrupted())
    {
        return false;
    }
    return tts->write(text);
}

void TTSController::streamAudio()
{
    cout << "stream started........" << endl;

    while (!tts->can_read())
    {
        usleep(1000);
    }

    cout << "Piper ready" << endl;

    char buffer[20000];

    while (tts->can_read())
    {
        if (is_interrupted())
        {
            interrupt();
            break;
        }
        // improve read complete store in local use that for audio
        size_t bytes_read = tts->read(buffer, sizeof(buffer));

        cout << "Piper read : "<<bytes_read <<endl;
        size_t bytes_write = 0;
        while (bytes_read > bytes_write)
        {
            bytes_write = audio->write(buffer + bytes_write, bytes_read);
            cout << "Bytes write : " << bytes_write <<endl;
            if (bytes_read == bytes_write)
                break;
            if (bytes_read > bytes_write)
            {
                bytes_read -= bytes_write;
                if (bytes_read <= 0)
                    break;
            }
        }

        memset(buffer, 0, sizeof(buffer));

        if (!tts->can_read() && bytes_read == 0 && tts->is_completed())
        {
            cout << " IS complete" << endl;
            break;
        }
    }
    cout << "stream completed........" << endl;
}

bool TTSController::is_completed()
{
    if (audio->can_write_audio())
    {
        usleep(2000000); // for current audio to complete
        return true;
    }
    return false;
}

void TTSController::set_interrupt(bool is_interrupt)
{
    this->is_interrupt = is_interrupt;
}

bool TTSController::is_interrupted()
{
    return is_interrupt;
}

void TTSController::interrupt()
{
    tts->interrupt();
    audio->interrupt();
}

void TTSController::pause()
{
    tts->pause();
    audio->pause();
}

void TTSController::resume()
{
    tts->play();
    audio->play();
}

void TTSController::stop()
{
    tts->stop();
    audio->stop();
}
TTSController::~TTSController()
{
    delete tts;
    delete audio;
    is_interrupt = false;
}