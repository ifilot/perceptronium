#!/bin/sh

ffmpeg -framerate 120 -i ../movie/out/frame_%05d.png -c:v libx265 -preset slow -crf 18 -pix_fmt yuv420p -r 120 ../movie/ch4.mp4