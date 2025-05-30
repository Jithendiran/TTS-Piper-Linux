30 May 25  
Issue:   

    when piper_read buffer size is between 10 - 20 k audio played well. if buffer size is equal to system buffer size 65536 large audio interruptted in between. when i added log statements it played fine

Observation :  
    This behaviour is because piper is filling the pipe very fast, but aplay is delay in taking the inputs, so if too low or high of buffer cause buffer over flow check [learning](../doc/learning.md#sound-alsa)
