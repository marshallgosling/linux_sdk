
## Compile Agora SDK Samples（For Linux）

## Installation

### Install hiredis

Since *redis-plus-plus* is based on *hiredis*, you should install *hiredis* first. The minimum version requirement for *hiredis* is **v0.12.1**. However, [the latest stable release](https://github.com/redis/hiredis/releases) of *hiredis* is always recommended.

**NOTE**: You must ensure that there's only 1 version of hiredis is installed. Otherwise, you might get some wired problems.

Normally, you can install *hiredis* with a C++ package manager, and that's the easiest way to do it, e.g. `sudo apt-get install libhiredis-dev`. However, if you want to install the latest code of hiredis, or a specified version (e.g. async support needs hiredis v1.0.0 or later), you can install it from source.

Note again: DO NOT INSTALL MULTIPLE VERSIONS OF HIREDIS.

```shell
git clone https://github.com/redis/hiredis.git

cd hiredis

make

make install
```

By default, *hiredis* is installed at */usr/local*. If you want to install *hiredis* at non-default location, use the following commands to specify the installation path.

```shell
make PREFIX=/non/default/path

make PREFIX=/non/default/path install
```

### Install redis-plus-plus

*redis-plus-plus* is built with [CMAKE](https://cmake.org).

```shell
git clone https://github.com/sewenew/redis-plus-plus.git

cd redis-plus-plus

mkdir build

cd build

cmake .. -DREDIS_PLUS_PLUS_CXX_STANDARD=11

make

make install

cd ..
```

### Build Samples

```
$ cd example
$ ./build.sh
$ export LD_LIBRARY_PATH=./agora_sdk

Upon successful compilation, there are a couple of **sample_xxx"" excutables in out folder
Also, download the media files by running sync-data.sh script. These audio/video files will be used together with the sample code.
```

## Prepare your h264 and aac file 

You can download the sample media files from Agora
Just run sync_data.sh. It will download the media files into ./media folder.

```
$ ./sync_data.sh
```

#### FFMPEG usage
```
$ ffmpeg -i raw.mp4 -c:v h264 -bf 0 -g 25 -an -f m4v video.h264
$ ffmpeg -i raw.mp4 -c:v h264 -bf 0 -g 25 -s 360*640 -an -f m4v video.h264
$ ffmpeg -i raw.mp4 -ac 2 -ar 48000 -c:a aac audio.aac

```


## Run Agora SDK Samples

**sample_send_h264_aac_ad** reads from an H264 file and an AAC file and sends the combined stream to Agora channel, and pause the main stream with an AD files(h264,aac)

#### Example usage:
```
To send aac stream to Agora channel named "demo_channel". Note that `xxxxxx` should be replaced with your own App ID or token

$ out/sample_send_h264_aac_ad --token XXXXXX --channelId demo_channel --audioFile ./media/audio.aac --videoFile ./media/video.h264 --audioAdFile ./media/ad_audio.aac --videoAdFile ./media/ad_video.h264 --fps 30 --userId 900001


```




**sample_send_media_redis** reads from an H264 file and an AAC file and sends the combined stream to Agora channel, and pause the main stream with an AD files(h264,aac) via redis queue

#### Usage:
```
Run the sample bin file first
$ out/sample_send_media_redis --audioFile ./media/audio.aac --videoFile ./media/video.h264 --audioAdFile ./media/ad_audio.aac --videoAdFile ./media/ad_video.h264 --fps 30 --token XXXXX --channelId demo_channel --userId 900001 --redisIp 127.0.0.1 --redisQueue sample_queue


Then open another shell window and run redis-cli command
Input queue 'sample_queue' a simple string 'switch'

$ redis-cli
  127.0.0.1:6379> rpush sample_queue switch

```
