#ifndef _JSON_PARSER_H_
#define _JSON_PARSER_H_
#include "jsmn.h"
#include "Arduino.h"
#undef max
#undef min
#include <vector>
#define max(a,b) ((a)>(b)?(a):(b))
#define min(a,b) ((a)<(b)?(a):(b))

#define MAX_NUMBER_OF_TOKENS 1024

void testJson();

using namespace std;

class JsonElement
{
public:
  JsonElement(jsmntype_t type);
  jsmntype_t getType();
private:
  jsmntype_t type;
};

class JsonObject: public JsonElement
{
public:
  JsonObject();
  void set(char* field_name, JsonElement* value);
  JsonElement* get(char* field_name);
  ~JsonObject();
private:
  vector<pair<char*, JsonElement*> > fields;
};

class JsonString: public JsonElement
{
public:
  JsonString(char* value);
  char* getValue();
  ~JsonString();
private:
  char* val;
};

class JsonArray: public JsonElement
{
public:
  JsonArray();
  JsonElement* get(int i);
  void add(JsonElement* element);
  ~JsonArray();
private:
  vector<JsonElement*> contents;
};


class JsonParser
{
public:
  JsonObject* parse(char* input);
private:
  JsonObject* parseObject(char* input);
  JsonElement* parseElement(char* input);
  JsonString* parseString(char* input);
  JsonArray* parseArray(char* input);
  
  jsmn_parser p;
  jsmntok_t t[MAX_NUMBER_OF_TOKENS];
  int token_index;
  int number_of_tokens;
};

#endif
