@echo on

ffmpeg -r 10 -f image2 -i "%%d.png" -vcodec png -crf 25  -pix_fmt yuv420p test-video-full.mp4
