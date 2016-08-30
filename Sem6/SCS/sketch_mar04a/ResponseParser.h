#ifndef _RESPONSE_PARSER_H_
#define _RESPONSE_PARSER_H_

#include "Arduino.h"
#include "types.h"
#include "JsonParser.h"

class ResponseParser {
public:
  ResponseParser();
  void enableDebugMsgs();
  int getResponseCode(char* response);
  char* readResponseUntilEmptyLine(char* response);
  JsonObject* getResponseMessage(char* response);
private:
  bool debug_msgs;
  JsonParser jsonParser;
  bool found_chunked_header;
  char* parseChunks(char* content);
};

#endif
