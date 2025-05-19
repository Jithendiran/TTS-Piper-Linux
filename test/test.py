import ctypes
import os

# Load the shared library
lib = ctypes.CDLL(os.path.abspath("build/libttspiper.so"))

# Define argument and return types for the functions you use
lib.tts_create_piper.restype = ctypes.c_void_p
lib.tts_create_piper.argtypes = [ctypes.c_char_p, ctypes.c_char_p, ctypes.POINTER(ctypes.c_char_p), ctypes.c_int]

lib.tts_create_audio.restype = ctypes.c_void_p
lib.tts_create_audio.argtypes = [ctypes.c_char_p, ctypes.POINTER(ctypes.c_char_p), ctypes.c_int]

lib.tts_create.restype = ctypes.c_void_p
lib.tts_create.argtypes = [ctypes.c_void_p, ctypes.c_void_p]

lib.tts_start.restype = ctypes.c_int
lib.tts_start.argtypes = [ctypes.c_void_p]

lib.tts_write.restype = ctypes.c_ssize_t
lib.tts_write.argtypes = [ctypes.c_void_p, ctypes.c_char_p]

lib.tts_streamAudio.restype = None
lib.tts_streamAudio.argtypes = [ctypes.c_void_p]

lib.tts_is_completed.restype = ctypes.c_int
lib.tts_is_completed.argtypes = [ctypes.c_void_p]

lib.tts_stop.restype = None
lib.tts_stop.argtypes = [ctypes.c_void_p]

lib.tts_destroy.restype = None
lib.tts_destroy.argtypes = [ctypes.c_void_p]

# Prepare arguments
tts_args = [
    b"--config",
    b"/home/jidesh/.cache/calibre/piper-voices/en_US-hfc_male-medium.onnx.json",
    b"--output-raw", b"--json-input", b"--sentence-silence 0.1", b"--length_scale 1.2",
    None
]
tts_args_array = (ctypes.c_char_p * len(tts_args))(*tts_args)

alsa_args = [b"-f", b"S16_LE", b"-r", b"22050", b"-q", None]
alsa_args_array = (ctypes.c_char_p * len(alsa_args))(*alsa_args)

# Create objects
piper = lib.tts_create_piper(
    b"/opt/calibre/bin/piper/piper",
    b"/home/jidesh/.cache/calibre/piper-voices/en_US-hfc_male-medium.onnx",
    tts_args_array,
    len(tts_args)
)
audio = lib.tts_create_audio(b"aplay", alsa_args_array, len(alsa_args))
tts_ctrl = lib.tts_create(piper, audio)

# Example text list
texts = [
    b'{"text": "Hi hello i am super man."}\n',
    b'{"text": "Hi hello you  man."}\n',
    b'{"text": "A week ago a friend invited a couple of other couples over for dinner..."}\n'
]

lib.tts_start(tts_ctrl)
for text in texts:
    print("playing:", text)
    lib.tts_write(tts_ctrl, text)
    lib.tts_streamAudio(tts_ctrl)
    while not lib.tts_is_completed(tts_ctrl):
        pass
    print("completed")
lib.tts_stop(tts_ctrl)
lib.tts_destroy(tts_ctrl)

#gdb --args python3 test.py 