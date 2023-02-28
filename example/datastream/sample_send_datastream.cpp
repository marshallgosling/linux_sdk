//  Agora RTC/MEDIA SDK
//
//  Created by Jay Zhang in 2020-04.
//  Copyright (c) 2020 Agora.io. All rights reserved.
//
#include <csignal>
#include <cstring>
#include <sstream>
#include <string>
#include <thread>
#include<fstream>

#include "IAgoraService.h"
#include "NGIAgoraRtcConnection.h"
#include "common/file_parser/helper_h264_parser.h"
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

struct SampleOptions {
  std::string appId;
  std::string channelId;
  std::string userId;
  std::string localIP;
  std::string message_file = "message.txt";
  int sleep_time = 500;
  bool reliable = false;
  bool ordered = false;
  bool sync = false;
};

static bool exitFlag = false;
static void SignalHandler(int sigNo) { exitFlag = true; }

int main(int argc, char* argv[]) {
  SampleOptions options;
  opt_parser optParser;

  optParser.add_long_opt("token", &options.appId, "The token for authentication / must");
  optParser.add_long_opt("channelId", &options.channelId, "Channel Id / must");
  optParser.add_long_opt("userId", &options.userId, "User Id / default is 0");
  optParser.add_long_opt("reliable", &options.reliable, "The data stream reliable / default is false");
  optParser.add_long_opt("ordered", &options.ordered, "The data stream reliable / default is false");
  optParser.add_long_opt("sync", &options.sync, "The data stream reliable / default is false");
  optParser.add_long_opt("localIP", &options.localIP,
                         "Local IP");
  optParser.add_long_opt("mfile", &options.message_file,
                         "message_file default message.txt");
  optParser.add_long_opt("sleeptime", &options.sleep_time,
                         "sleep_time default 500ms");

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

  // Create local user observer to monitor intra frame request
  auto localUserObserver = std::make_shared<SampleLocalUserObserver>(connection->getLocalUser());
  

  if(!options.ordered && options.reliable){
    AG_LOG(ERROR, "datastream is unordered but reliable !");
    return -1;
  }

  int streamId;
  connection->createDataStream(&streamId,options.reliable,options.ordered,options.sync);

  // Connect to Agora channel
  if (connection->connect(options.appId.c_str(), options.channelId.c_str(),
                          options.userId.c_str())) {
    AG_LOG(ERROR, "Failed to connect to Agora channel!");
    return -1;
  }
  int data_message_seq = 0;
   std::ifstream fp;
   fp.open(options.message_file.c_str(), std::ios::in);
  while(!exitFlag){
    //std::string message = std::to_string(data_message_seq++)+"Hello agora";
    while(!fp.eof()){
    std::string message;
    std::getline(fp, message);
    connection->sendStreamMessage(streamId,message.c_str(),message.length());
    usleep(options.sleep_time*1000);
    }
    fp.close();
    fp.open(options.message_file.c_str(), std::ios::in);
  }
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
  connection = nullptr;

  // Destroy Agora Service
  service->release();
  service = nullptr;

  return 0;
}