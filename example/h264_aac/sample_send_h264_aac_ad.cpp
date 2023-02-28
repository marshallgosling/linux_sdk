//  Agora RTC/MEDIA SDK
//
//  Created by Jay Zhang in 2020-04.
//  Copyright (c) 2020 Agora.io. All rights reserved.
//

//video
//   ******************         --------         *************         ------------       *********        **********         ************       ------------        ***********       --------
//  {SDK::capture video}  ==>  |raw data|  ==>  {SDK::encode B}  ==>  |encoded data| ==> {SDK::send}  ==> {AGORA::VOS}  ==>  {SDK::receive} ==> |encoded data|  ==> {SDK::decode} ==> |raw data|
//   ******************         --------         *************         ------------       *********        **********         ************       ------------        ***********       --------
//                                                                                  sample send h264(this sample)                              sample receive h264                 

//This sample will show how to use the SDK to send the encoded_Video to the Agora_channel
//As a user,you should papera the encoded_Video and create the Agora_service,Agora_connection, Agora_EncodedImage_sender and Agora_local_video_track
//You should parse the encoded_Video and send to sdk one frame by one frame.(by use the AGORA_API: Sender->sendEncodedVideoImage())
//The class HelperH264FileParser is a common_helper class to parse the h264 video 
//And all Agora necessary Classes builded in main() function
//Last, the sendVideoThread call Sender->sendEncodedVideoImage() to send the encoded videoFrame

//The sdk also provide lots of call_back functions to help user get the network states , local states and  peer states
//The callback functions can be found in **observer , you should register the observer first.

// the SDKapi call flows:
// service = createAndInitAgoraService()
//     connection = service->createRtcConnection()
//         connection->registerObserver()
//             connection->connect()
//         factory = service->createMediaNodeFactory()
//             VideoSender = factory->createVideoEncodedImageSender();
//                 VideoTrack = service->createCustomVideoTrack()
//      connection->getLocalUser()->publishAudio(VideoTrack);
//                 Sender->sendEncodedVideoImage()  



//audio
//   ******************         --------         *************         ------------       *********        **********         ************       ------------        ***********       --------
//  {SDK::capture audio}  ==>  |raw data|  ==>  {SDK::encode B}  ==>  |encoded data| ==> {SDK::send}  ==> {AGORA::VOS}  ==>  {SDK::receive} ==> |encoded data|  ==> {SDK::decode} ==> |raw data|
//   ******************         --------         *************         ------------       *********        **********         ************       ------------        ***********       --------
//                        sample send pcm(this sample)                sample send opus                                                                                            sample receive pcm

//This sample will show how to use the SDK to send the raw AudioData(pcm) to the Agora_channel
//As a user,you should papera the audio and create the Agora_service,Agora_connection, Agora_audio_pcm_sender and Agora_local_audio_track
//You should parse the audio and send to sdk one frame by one frame.(by use the AGORA_API: Sender->sendAudioPcmData())
//And all Agora necessary Classes builded in main() function
//Last, the sendAudioThread call Sender->sendAudioPcmData() to send the audioFrame

//The sdk also provide lots of call_back functions to help user get the network states , local states and  peer states
//The callback functions can be found in **observer , you should register the observer first.


// the SDKapi call flows:
// service = createAndInitAgoraService()
//     connection = service->createRtcConnection()
//         connection->registerObserver()
//         connection->connect()
//         factory = service->createMediaNodeFactory()
//             AudioSender = factory->createAudioPcmDataSender();
//                 AudioTrack = service->createCustomAudioTrack();
//      connection->getLocalUser()->publishAudio(audioTrack);
//                 Sender->sendAudioPcmData()  

//The destruct order of all the class can be find in the main function end.

// Wish you have a great experience with Agora_SDK!


#include <csignal>
#include <cstring>
#include <sstream>
#include <string>
#include <thread>

#include "IAgoraService.h"
#include "NGIAgoraRtcConnection.h"
#include "common/file_parser/helper_h264_parser.h"
#include "common/file_parser/helper_aac_parser.h"
#include "common/helper.h"
#include "common/log.h"
#include "common/opt_parser.h"
#include "common/sample_common.h"
#include "common/sample_connection_observer.h"
#include "common/sample_local_user_observer.h"

#include "NGIAgoraAudioTrack.h"
#include "NGIAgoraLocalUser.h"
#include "NGIAgoraMediaNodeFactory.h"
#include "NGIAgoraMediaNode.h"
#include "NGIAgoraVideoTrack.h"

#define DEFAULT_CONNECT_TIMEOUT_MS (3000)
#define DEFAULT_SAMPLE_RATE (44100)
#define DEFAULT_AUDIO_FRAME_DURATION (243)
#define DEFAULT_NUM_OF_CHANNELS (2)
#define DEFAULT_FRAME_RATE (30)
#define DEFAULT_AUDIO_FILE "media/audio.aac"
#define DEFAULT_VIDEO_FILE "media/video.h264"
#define DEFAULT_AD_AUDIO_FILE "media/ad_audio.aac"
#define DEFAULT_AD_VIDEO_FILE "media/ad_video.h264"

struct SampleOptions {
  std::string appId;
  std::string channelId;
  std::string userId;
  std::string audioFile = DEFAULT_AUDIO_FILE;
  std::string videoFile = DEFAULT_VIDEO_FILE;
  std::string audioAdFile = DEFAULT_AD_AUDIO_FILE;
  std::string videoAdFile = DEFAULT_AD_VIDEO_FILE;
  std::string localIP;
  std::string logPath;
  int timeInterval = 10000;
  int adMode = 1;
  struct {
    int sampleRate = DEFAULT_SAMPLE_RATE;
    int numOfChannels = DEFAULT_NUM_OF_CHANNELS;
    int frameDuration = DEFAULT_AUDIO_FRAME_DURATION;
  } audio;
  struct {
    int frameRate = DEFAULT_FRAME_RATE;
    bool showBandwidthEstimation = false;
  } video;
};

static int sendCount = 0;
static int sendVCount = 0;

static bool adMode = false;
static bool once = true;

static void sendOnePcmFrame(const SampleOptions& options,
                            agora::agora_refptr<agora::rtc::IAudioPcmDataSender> audioFrameSender) {
  static FILE* file = nullptr;
  const char* fileName = options.audioFile.c_str();

  // Calculate byte size for 10ms audio samples
  int sampleSize = sizeof(int16_t) * options.audio.numOfChannels;
  int samplesPer10ms = options.audio.sampleRate / 100;
  int sendBytes = sampleSize * samplesPer10ms;

  if (!file) {
    if (!(file = fopen(fileName, "rb"))) {
      AG_LOG(ERROR, "Failed to open audio file %s", fileName);
      return;
    }
    AG_LOG(INFO, "Open audio file %s successfully", fileName);
  }

  uint8_t frameBuf[sendBytes];

  if (fread(frameBuf, 1, sizeof(frameBuf), file) != sizeof(frameBuf)) {
    if (feof(file)) {
      fclose(file);
      file = nullptr;
      AG_LOG(INFO, "End of audio file");
    } else {
      AG_LOG(ERROR, "Error reading audio data: %s", std::strerror(errno));
    }
    return;
  }

  if (audioFrameSender->sendAudioPcmData(frameBuf, 0, samplesPer10ms,  agora::rtc::TWO_BYTES_PER_SAMPLE,
                                         options.audio.numOfChannels,
                                         options.audio.sampleRate) < 0) {
    AG_LOG(ERROR, "Failed to send audio frame!");
  }
}

static void sendOneH264Frame(
    int frameRate, std::unique_ptr<HelperH264Frame> h264Frame,
    agora::agora_refptr<agora::rtc::IVideoEncodedImageSender> videoH264FrameSender) {
  agora::rtc::EncodedVideoFrameInfo videoEncodedFrameInfo;
  videoEncodedFrameInfo.rotation = agora::rtc::VIDEO_ORIENTATION_0;
  videoEncodedFrameInfo.codecType = agora::rtc::VIDEO_CODEC_H264;
  // videoEncodedFrameInfo.width = 1280;
  // videoEncodedFrameInfo.height = 720;
  videoEncodedFrameInfo.framesPerSecond = frameRate;
  //videoEncodedFrameInfo.streamType = agora::rtc::VIDEO_STREAM_HIGH;
  videoEncodedFrameInfo.frameType =
      (h264Frame.get()->isKeyFrame ? agora::rtc::VIDEO_FRAME_TYPE::VIDEO_FRAME_TYPE_KEY_FRAME
                                   : agora::rtc::VIDEO_FRAME_TYPE::VIDEO_FRAME_TYPE_DELTA_FRAME);

  videoH264FrameSender->sendEncodedVideoImage(
      reinterpret_cast<uint8_t*>(h264Frame.get()->buffer.get()), h264Frame.get()->bufferLen,
      videoEncodedFrameInfo);
}


static void SampleSendAudioTask(
    const SampleOptions& options,
    agora::agora_refptr<agora::rtc::IAudioEncodedFrameSender> audioFrameSender, bool& exitFlag) {
  std::unique_ptr<HelperAacFileParser> audioFileParser(
      new HelperAacFileParser(options.audioFile.c_str()));
  audioFileParser->initialize();
  PacerInfo pacer = {0, 23, 0, std::chrono::steady_clock::now()};
  
  std::unique_ptr<HelperAacFileParser> audioAdFileParser(
      new HelperAacFileParser(options.audioAdFile.c_str()));
  audioAdFileParser->initialize();
  audioAdFileParser->setOnce(true);

  auto timestamp = now_ms_t();

  while (!exitFlag) {

    int timetag = now_ms_t() - timestamp;

    if(timetag > options.timeInterval && once && options.adMode == 1) {

      printf("start adMode\n");

      once = false;
      adMode = true;
    }

    auto audioFrame = adMode ? audioAdFileParser->getAudioFrame(options.audio.frameDuration) :
      audioFileParser->getAudioFrame(options.audio.frameDuration);

    bool isEnd = adMode ? audioAdFileParser->getEndOfFileTag() : false;

    if (isEnd) adMode = false;
 
    if (audioFrame) {
      audioFrameSender->sendEncodedAudioFrame(audioFrame.get()->buffer, audioFrame.get()->bufferLen,
                                              audioFrame.get()->audioFrameInfo);
    // we should send one aac frame during 21.33333ms
      if((sendCount++)%3 == 0) pacer.sendIntervalInMs = 22;
      else pacer.sendIntervalInMs = 21;
      waitBeforeNextSend(pacer);  // sleep for a while before sending next frame
    }
  };
}


static void SampleSendVideoH264Task(
    const SampleOptions& options,
    agora::agora_refptr<agora::rtc::IVideoEncodedImageSender> videoH264FrameSender,
    bool& exitFlag) {
  std::unique_ptr<HelperH264FileParser> h264FileParser(
      new HelperH264FileParser(options.videoFile.c_str()));
  h264FileParser->initialize();

  std::unique_ptr<HelperH264FileParser> adFileParser(
    new HelperH264FileParser(options.videoAdFile.c_str())
  );
  adFileParser->initialize();
  adFileParser->setOnce(true);

  // Calculate send interval based on frame rate. H264 frames are sent at this interval
  PacerInfo pacer = {0, 1000/options.video.frameRate, 0, std::chrono::steady_clock::now()};

  while (!exitFlag) {

    auto h264Frame = adMode ? adFileParser->getH264Frame() : h264FileParser->getH264Frame();
   
    bool isEnd = adMode ? adFileParser->getEndOfFileTag() : false;

    if (isEnd) {
      adMode = false;
      printf("end adMode\n");
    }

    if (h264Frame) {
      
      sendOneH264Frame(options.video.frameRate, std::move(h264Frame), videoH264FrameSender);
      // if((sendVCount++)%3 == 0) pacer.sendIntervalInMs = 22;
      // else pacer.sendIntervalInMs = 66;
      waitBeforeNextSend(pacer);  // sleep for a while before sending next frame
    }
  };
}

static bool exitFlag = false;
static void SignalHandler(int sigNo) { exitFlag = true; }

int main(int argc, char* argv[]) {
  SampleOptions options;
  opt_parser optParser;

  optParser.add_long_opt("token", &options.appId, "The token for authentication / must");
  optParser.add_long_opt("channelId", &options.channelId, "Channel Id / must");
  optParser.add_long_opt("userId", &options.userId, "User Id / default is 0");
  optParser.add_long_opt("audioFile", &options.audioFile,
                         "The audio file in raw PCM format to be sent");
  optParser.add_long_opt("videoFile", &options.videoFile,
                         "The video file in H264 format to be sent");
  optParser.add_long_opt("audioAdFile", &options.audioAdFile,
                         "The audio file in raw PCM format to be sent");
  optParser.add_long_opt("videoAdFile", &options.videoAdFile,
                         "The video file in H264 format to be sent");
  optParser.add_long_opt("sampleRate", &options.audio.sampleRate,
                         "Sample rate for the PCM file to be sent");
  optParser.add_long_opt("numOfChannels", &options.audio.numOfChannels,
                         "Number of channels for the PCM file to be sent");
  optParser.add_long_opt("fps", &options.video.frameRate,
                         "Target frame rate for sending the video stream");
  optParser.add_long_opt("bwe", &options.video.showBandwidthEstimation,
                         "show or hide bandwidth estimation info");
  optParser.add_long_opt("localIP", &options.localIP,
                         "Local IP");
  optParser.add_long_opt("logPath", &options.logPath, "Log Path");
  optParser.add_long_opt("timeInterval", &options.timeInterval, "timeInterval");
  optParser.add_long_opt("adMode", &options.adMode, "AD Mode");

  if ((argc <= 1) || !optParser.parse_opts(argc, argv)) {
    std::ostringstream strStream;
    optParser.print_usage(argv[0], strStream);
    std::cout << strStream.str() << std::endl;
    return -1;
  }

  if (options.appId.empty()) {
    AG_LOG(ERROR, "Must provide appId!");
    return -1;
  }

  if (options.channelId.empty()) {
    AG_LOG(ERROR, "Must provide channelId!");
    return -1;
  }

  std::signal(SIGQUIT, SignalHandler);
  std::signal(SIGABRT, SignalHandler);
  std::signal(SIGINT, SignalHandler);

  // Create Agora service
  auto service = createAndInitAgoraService(false, true, true);
  if (!service) {
    AG_LOG(ERROR, "Failed to creating Agora service!");
  }

  //service->setLogFile()

  // Create Agora connection
  agora::rtc::RtcConnectionConfiguration ccfg;
  ccfg.autoSubscribeAudio = false;
  ccfg.autoSubscribeVideo = false;
  ccfg.clientRoleType = agora::rtc::CLIENT_ROLE_BROADCASTER;
  agora::agora_refptr<agora::rtc::IRtcConnection> connection = service->createRtcConnection(ccfg);
  if (!connection) {
    AG_LOG(ERROR, "Failed to creating Agora connection!");
    return -1;
  }

  if (!options.localIP.empty()) {
    if (setLocalIP(connection, options.localIP)){
      AG_LOG(ERROR, "set local IP to %s error!", options.localIP.c_str());
      return -1;
    }
  }

  // Register connection observer to monitor connection event
  auto connObserver = std::make_shared<SampleConnectionObserver>();
  connection->registerObserver(connObserver.get());

  // Register network observer to monitor bandwidth estimation result
  if (options.video.showBandwidthEstimation) {
    connection->registerNetworkObserver(connObserver.get());
  }

  // Create local user observer to monitor intra frame request
  auto localUserObserver = std::make_shared<SampleLocalUserObserver>(connection->getLocalUser());

  // Connect to Agora channel
  if (connection->connect(options.appId.c_str(), options.channelId.c_str(),
                          options.userId.c_str())) {
    AG_LOG(ERROR, "Failed to connect to Agora channel!");
    return -1;
  }

  // Create media node factory
  agora::agora_refptr<agora::rtc::IMediaNodeFactory> factory = service->createMediaNodeFactory();
  if (!factory) {
    AG_LOG(ERROR, "Failed to create media node factory!");
  }

  // Create audio data sender
  agora::agora_refptr<agora::rtc::IAudioEncodedFrameSender> audioFrameSender =
      factory->createAudioEncodedFrameSender();
  if (!audioFrameSender) {
    AG_LOG(ERROR, "Failed to create audio encoded frame sender!");
    return -1;
  }

  // Create audio track
  agora::agora_refptr<agora::rtc::ILocalAudioTrack> customAudioTrack =
      service->createCustomAudioTrack(audioFrameSender, agora::base::MIX_ENABLED);
  if (!customAudioTrack) {
    AG_LOG(ERROR, "Failed to create audio track!");
    return -1;
  }

  // Create video frame sender
  agora::agora_refptr<agora::rtc::IVideoEncodedImageSender> videoFrameSender =
      factory->createVideoEncodedImageSender();
  if (!videoFrameSender) {
    AG_LOG(ERROR, "Failed to create video frame sender!");
    return -1;
  }

  agora::rtc::SenderOptions option;
  option.ccMode = agora::rtc::TCcMode::CC_ENABLED;
  // Create video track
  agora::agora_refptr<agora::rtc::ILocalVideoTrack> customVideoTrack =
      service->createCustomVideoTrack(videoFrameSender, option);
  if (!customVideoTrack) {
    AG_LOG(ERROR, "Failed to create video track!");
    return -1;
  }

  // Publish audio & video track
  connection->getLocalUser()->publishAudio(customAudioTrack);
  connection->getLocalUser()->publishVideo(customVideoTrack);

  // Wait until connected before sending media stream
  connObserver->waitUntilConnected(DEFAULT_CONNECT_TIMEOUT_MS);

  if (!options.localIP.empty()) {
    std::string ip;
    getLocalIP(connection, ip);
    AG_LOG(INFO, "Local IP:%s", ip.c_str());
  }

  // Start sending media data
  AG_LOG(INFO, "Start sending audio & video data ...");
  std::thread sendAudioThread(SampleSendAudioTask, options, audioFrameSender, std::ref(exitFlag));
  std::thread sendVideoThread(SampleSendVideoH264Task, options, videoFrameSender,
                              std::ref(exitFlag));

  sendAudioThread.join();
  sendVideoThread.join();

  // Unpublish audio & video track
  connection->getLocalUser()->unpublishAudio(customAudioTrack);
  connection->getLocalUser()->unpublishVideo(customVideoTrack);

  // Unregister connection observer
  connection->unregisterObserver(connObserver.get());

  // Unregister network observer
  connection->unregisterNetworkObserver(connObserver.get());

  // Disconnect from Agora channel
  if (connection->disconnect()) {
    AG_LOG(ERROR, "Failed to disconnect from Agora channel!");
    return -1;
  }
  AG_LOG(INFO, "Disconnected from Agora channel successfully");

  // Destroy Agora connection and related resources
  connObserver.reset();
  localUserObserver.reset();
  audioFrameSender = nullptr;
  videoFrameSender = nullptr;
  customAudioTrack = nullptr;
  customVideoTrack = nullptr;
  factory = nullptr;
  connection = nullptr;

  // Destroy Agora Service
  service->release();
  service = nullptr;

  return 0;
}
