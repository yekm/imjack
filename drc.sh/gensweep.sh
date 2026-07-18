#!/bin/bash

set -evx

#ffmpeg -y -hide_banner -loglevel info -channels 1 -f jack -i ffmpeg_wav -channels 2 -f wav -c copy mic1.wav
./glsweep 48000 0.8 10 24000 60 1 0.025 0.0025 sweep.pcm inverse.pcm
sox -t f32 -r 48000 -c 1 sweep.pcm -t wav -c 1 sweep.wav
sox sweep.wav -c 2 left.wav remix 1 0
sox sweep.wav -c 2 right.wav remix 0 1
sox -r 48k -n -c 2 -t wav sine400.wav synth 0.2 sine 440 pad 1 1 gain -1
sox sine400.wav sine400.wav left.wav right.wav sine400-sweep.wav
scp sine400-sweep.wav nc110.lan:/home/yekm || true
parallel 'sox {} -n spectrogram -x 3920 -Y 1080 -o {}.png' ::: s*.wav
