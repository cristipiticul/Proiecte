#include "ResponseParser.h"

const int OK_RESPONSE_CODE = 200;

ResponseParser::ResponseParser()  {
  debug_msgs = false;
}

/**
This function reads the status line and returns
the status code.
*/
int ResponseParser::getResponseCode(char* response) {
  if (debug_msgs) {
    Serial.println("getResponseCode");
  }
  int code = -1;

  int i = 0;
  State state = HTTP_VERSION;
  char status_code[4];
  while (*response != '\0' && state != DONE) {
    char c = *response;
    response++;
    if (c == '\n') {
      state = DONE;
    } else if (c == ' ') {
      if (state == CODE) {
        status_code[i] = 0;
        code = atoi(status_code);
      }
      state = next_state(state);
    } else if (state == CODE) {
      status_code[i] = c;
      i++;
    }
  }

  if (debug_msgs) {
    Serial.print("Response code: ");
    Serial.println(code, 10);
    Serial.println();
  }
  return code;
}
/**
Returns a pointer to the start of the content of the response.
The content is located just after the first blank line.
*/
char* ResponseParser::readResponseUntilEmptyLine(char* response)
{
  found_chunked_header = false; // WE NEED TO TAKE THE "Transfer-Encoding: chunked" HEADER INTO CONSIDERATION!
  if (debug_msgs) {
    Serial.println("Reading the stuff nobody cares about: the data between status code and message.");
    Serial.println("Oh, wait. There might be a \"Transfer-Encoding: chunked\" which fucks everything up...");
  }
  bool hasChars = false;
  bool done = false;
  int line_start = 0;
  int line_end;
  int line_contents_end;
  const char chunked_header[] = "Transfer-Encoding: chunked";
  const int chunked_header_length = strlen(chunked_header);
  // Parse the response line by line and search for the chunked header
  while (response[line_start] != '\0' && !done) {
    for (line_end = line_start; response[line_end] != '\n'; line_end++)
      if (debug_msgs) {
        Serial.print(response[line_end]);
      }
    if (debug_msgs) {
      Serial.println(); // the return at the end of the current line
    }
    if (response[line_end - 1] == '\r') {
      line_contents_end = line_end - 2;
    } else {
      line_contents_end = line_end - 1;
    }
    if (strncmp(response + line_start, chunked_header, chunked_header_length) == 0) {
      found_chunked_header = true;
      if (debug_msgs) {
        Serial.println("Found the chunked header!!!");
      }
    }
    if (line_contents_end < line_start) {
      done = true;
    }
    line_start = line_end + 1;
  }
  if (debug_msgs) {
    Serial.println("End of stuff nobody cares about");
    Serial.println();
  }
  return response + line_start;
}

int base16_digit(char chr)
{
  if (chr >= '0' && chr <= '9') {
    return chr - '0';
  }
  if (chr >= 'A' && chr <= 'F') {
    return chr - 'A' + 10;
  } 
  if (chr >= 'a' && chr <= 'f') {
    return chr - 'a' + 10;
  }
  Serial.println("Wrong argument given to base16_digit");
}

int atoi_base16(char* str)
{
  int result = 0;
  for (int i = 0; str[i] != '\0'; i++) {
    result = (result << 4) + base16_digit(str[i]);
  }
  return result;
}

char* ResponseParser::parseChunks(char* content)
{
  char* result = new char[strlen(content)];
  char* result_p = result;
  int content_length;
  char content_length_str[5];
  if (debug_msgs) {
    Serial.println("Joining the chunks of content:");
    Serial.println(content);
    Serial.println("End of content");
  }
  
  do {
    if (debug_msgs) {
      Serial.println("Start of chunk");
    }
    // Skip all the characters until the first chunk.
    // The chunk starts with the chunk size (which is a number)
    int content_length_str_len = 0;
    while (!((*content >= '0' && *content <= '9') || (*content >= 'A' && *content <= 'F') || (*content >= 'a' && *content <= 'f'))) {
      content++;
    }
    while ((*content >= '0' && *content <= '9') || (*content >= 'A' && *content <= 'F') || (*content >= 'a' && *content <= 'f')) {
      content_length_str[content_length_str_len] = *content;
      content_length_str_len++; 
      content++;
    }
    if (*content == '\r') {
      content++;
    }
    if (*content == '\n') {
      content++;
    }
    content_length_str[content_length_str_len] = '\0';
    content_length = atoi_base16(content_length_str);
    if (debug_msgs) {
      Serial.print("content chunk length:");
      Serial.println(content_length, 10);
    }
    
    strncpy(result_p, content, content_length);
    result_p += content_length;
    content += content_length;
    if (*content == '\r') {
      content++;
    }
    if (*content == '\n') {
      content++;
    }
    if (debug_msgs) {
      Serial.println("End of chunk");
    }
  }
  while (content_length > 0);
  *result_p = '\0';
  
  return result;
}

JsonObject* ResponseParser::getResponseMessage(char* response)
{
  JsonObject* result;
  char* contents = readResponseUntilEmptyLine(response);
  if (found_chunked_header) {
    char* parsed_chunks = parseChunks(contents);
    result = jsonParser.parse(parsed_chunks);
    delete[] parsed_chunks;
  } else {
    result = jsonParser.parse(contents);
  }
  return result;
  
}

void ResponseParser::enableDebugMsgs()
{
  debug_msgs = true;
}


