
## 概述


**Agora SDK Samples** 位于本目录中的子文件夹中，是 **Agora RTC SDK** 的基本示例代码。其中包含了对各种音频、视频格式的媒体流的发送和接收。每个示例代码可以单独运行。
提供这一系列示例代码的目的，是供开发者体验和学习声网的**RTC SDK API**，并且在理解API的基础上，更轻松的集成将功能到自己的应用程序中。
强烈建议你仔细阅读**h264_pcm**文件夹中的两个sample文件，文件头中附有大量的注释帮助你快速理解本SDK的音视频收发逻辑


## Agora SDK Samples 编译 （For Linux）

```
$ ./build.sh
$ ./sync-data.sh

编译完成后，在out目录里会产生一系列 **sample_xxx** 可执行文件。
另外，执行sync-data.sh的作用是把实例代码中依赖的一些测试媒体文件同步到本地的media目录。
```


## Agora SDK Samples 编译 (For Android)

```
$ ./build.sh

编译完成后，在build_android/bin/${arch_abi}目录里会产生一系列 **sample_xxx** 可执行文件。
为了在Android设备上运行这些程序，需要将该目录(包括里面的libs目录)整体push到Android设备上某个有执行权限的目录，比如/data 目录，并且在运行sample前指定动态库的搜索路径
$ export LD_LIBRARY_PATH=./libs
```

## Agora SDK Samples 运行

#### 参数

**sample_send_h264_aac_ad** 示例程序用来展示 **H264** 和 **AAC** 推流的相关功能，支持的参数选项如下：

* **--token ：** 用于指定用户的appId或token。无默认值，必填
* **--channelId ：** 用于指定加入频道的名称。无默认值，必填
* **--userId ：** 用于指定用户ID。默认值为 **0**，代表SDK随机指定
* **--audioFile ：** 用于指定发送的 **AAC** 音频文件。参数为文件路径，默认值为 **media/audio.aac**
* **--videoFile ：** 用于指定发送的 **H264** 视频文件。参数为文件路径，默认值为 **media/video.h264**
* **--audioAdFile ：** 用于指定发送的 **AAC** 广告音频文件。参数为文件路径，默认值为 **media/ad_audio.aac**
* **--videoAdFile ：** 用于指定发送的 **H264** 广告视频文件。参数为文件路径，默认值为 **media/ad_video.h264**
* **--fps ：** 用于指定发送的视频的帧率。默认值为 **30**
* **--timeInterval ：** 指定插入广告的时间点，加入频道后的第 x 毫秒。 默认值 **10000**

## 准备 H264 和 AAC 文件 

#### FFMPEG usage
```
$ ffmpeg -i raw.mp4 -c:v h264 -bf 0 -g 25 -an -f m4v video.h264
$ ffmpeg -i raw.mp4 -c:v h264 -bf 0 -g 25 -s 360*640 -an -f m4v video.h264
$ ffmpeg -i raw.mp4 -ac 2 -ar 48000 -c:a aac audio.aac

```