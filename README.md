
## Compile Agora SDK Samples（For Linux）

```
$ ./build.sh
$ ./sync_data.sh
$ export LD_LIBRARY_PATH=./agora_sdk

Upon successful compilation, there are a couple of **sample_xxx"" excutables in out folder
Also, download the media files by running sync-data.sh script. These audio/video files will be used together with the sample code.
```


## Run Agora SDK Samples

**sample_send_h264_aac_ad** reads from an H264 file and an AAC file and sends the combined stream to Agora channel, and pause the main stream with an AD files(h264,aac)

#### Example usage:
```
To send aac stream to Agora channel named "demo_channel". Note that `xxxxxx` should be replaced with your own App ID or token
$ out/sample_send_h264_aac_ad --token XXXXXX --channelId demo_channel --audioFile ./media/audio.aac --videoFile ./media/video.h264 --audioAdFile ./media/ad_audio.aac --videoAdFile ./media/ad_video.h264 --fps 30 --userId 900001
```


## Prepare your h264 and aac file 

#### FFMPEG usage
```
$ ffmpeg -i raw.mp4 -c:v h264 -bf 0 -g 25 -an -f m4v video.h264
$ ffmpeg -i raw.mp4 -c:v h264 -bf 0 -g 25 -s 360*640 -an -f m4v video.h264
$ ffmpeg -i raw.mp4 -ac 2 -ar 48000 -c:a aac audio.aac

```
