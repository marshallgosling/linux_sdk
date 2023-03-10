#include <string>

#include "AgoraBase.h"

struct HelperAudioFrame {
  agora::rtc::EncodedAudioFrameInfo audioFrameInfo;
  uint8_t* buffer;
  int bufferLen;
};

class HelperAacFileParser {
 public:
  HelperAacFileParser(const char* filepath);
  ~HelperAacFileParser();
  bool initialize();
  std::unique_ptr<HelperAudioFrame> getAudioFrame(int frameSizeDuration);
  bool getEndOfFileTag();
  void setOnce(bool once);
  void setFileParseRestart();

 private:
  std::string file_path_;
  int data_offset_;
  int data_size_;
  uint8_t* data_buffer_;
  bool end_of_file_;
  bool only_once_;
};
