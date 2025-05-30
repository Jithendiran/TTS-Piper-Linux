#include "TTSController.hpp"

volatile bool is_playing = false;
volatile bool is_stopped = false;

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
    if (is_stopped)
    {
        cout << "Process is not running restart the process" << endl;
        return false;
    }
    // only when it is not interrupted and not being played, it is allowed to write
    if (is_playing || is_interrupted())
    {
        cout << "Audio is playing or interrupted" << endl;
        return false;
    }
    return tts->write(text);
}

void TTSController::playAudio()
{
    if (is_stopped)
    {
        cout << "Process is not running restart the process" << endl;
        return;
    }

    // if currently audio is playing it won't allow till it completed
    if (is_playing)
        return;
    // Launch streamAudio in a new thread
    std::thread([this]()
                { this->streamAudio(); })
        .detach(); // Detach so it runs independently
}

void TTSController::streamAudio()
{
    if (is_stopped)
    {
        cout << "Process is not running restart the process" << endl;
        return;
    }

    // if currently audio is playing it won't allow till it completed
    if (is_playing)
        return;
    cout << "stream started........" << endl;

    int retries = 50; // 5 seconds max (50 * 100ms)
    while (!tts->can_read() && retries-- > 0)
    {
        usleep(100000); // 100ms
    }

    if (retries <= 0)
    {
        std::cerr << "[TTSController::streamAudio] Timeout: pipe not writable after 5 seconds\n";
        return;
    }

    is_playing = true;
    cout << "Piper ready" << endl;

    char buffer[65536];

    while (tts->can_read()) // have to change the logic
    {
        if (is_interrupted())
        {
            interrupt();
            break;
        }
        // improve read complete store in local use that for audio
        size_t bytes_read = tts->read(buffer, sizeof(buffer));

        const size_t period_size = 2756;
        size_t total_written = 0;

        while (total_written < bytes_read)
        {
            if (is_interrupted())
            {
                interrupt();
                break;
            } 
            size_t to_write = std::min(period_size, bytes_read - total_written);
            ssize_t written = audio->write(buffer + total_written, to_write);
            if (written > 0)
            {
                total_written += written;
            }
            else
            {
                usleep(1000); // avoid busy wait
            }
        }

        if (!tts->can_read() && total_written >= bytes_read && tts->is_completed())
        {
            cout << " IS complete" << endl;
            break;
        }
    }
    is_playing = false;
    cout << "stream completed........" << endl;
    set_interrupt(false);
}

bool TTSController::is_completed()
{
    if (!is_playing && audio->can_write_audio())
    {
        usleep(1050000); // for current audio to complete
        return audio->can_write_audio();
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
    cout << "Interrupt started" << endl;
    set_interrupt(true);
    tts->interrupt();
    cout << "TTS Interrupt completed" << endl;
    audio->interrupt();
    cout << "Audio Interrupt completed " << endl;
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
    if (is_playing)
        interrupt();
    is_stopped = true;
    tts->stop();
    audio->stop();
}
TTSController::~TTSController()
{
    is_playing = false;
    is_interrupt = false;
}