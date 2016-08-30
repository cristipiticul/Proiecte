#include "JsonParser.h"

using namespace std;

JsonElement::JsonElement(jsmntype_t type) {
  this->type = type;
}

jsmntype_t JsonElement::getType() {
  return type;
}

JsonObject::JsonObject(): JsonElement(JSMN_OBJECT) {
}

void JsonObject::set(char* field_name, JsonElement* value) {
  fields.push_back(make_pair(field_name, value));
}
JsonElement* JsonObject::get(char* field_name) {
  for (int i = 0; i < fields.size(); i++) {
    if (strcmp(fields[i].first, field_name) == 0) {
      return fields[i].second;
    }
  }
  Serial.print("Error! Field \"");
  Serial.print(field_name);
  Serial.println("\" not found!");
  return NULL;
}
JsonObject::~JsonObject() {
  for (int i = fields.size() - 1; i >= 0; i--) {
    char* field_name = fields[i].first;
    JsonElement* element = fields[i].second; 
    fields.pop_back();
    delete[] field_name;
    delete element;
  }
}

JsonString::JsonString(char* value): JsonElement(JSMN_STRING) {
  val = new char[strlen(value) + 1];
  strcpy(val, value);
}
char* JsonString::getValue() {
  return val;
}
JsonString::~JsonString() {
  delete[] val;
}


JsonArray::JsonArray(): JsonElement(JSMN_ARRAY) {
}
JsonElement* JsonArray::get(int i) {
  return contents[i];
}
void JsonArray::add(JsonElement* element) {
  contents.push_back(element);
}
JsonArray::~JsonArray() {
  for (int i = contents.size() - 1; i >= 0; i--) {
    JsonElement* element = contents[i];
    contents.pop_back();
    delete element;
  }
}


JsonObject* JsonParser::parse(char* input) {
  jsmn_init(&p);
  number_of_tokens = jsmn_parse(&p, input, strlen(input), t, sizeof(t) / sizeof(t[0]));
  if (number_of_tokens < 0) {
    Serial.print("Error! Failed to parse JSON \"");
    Serial.print(input);
    Serial.println("\"!");
    switch (number_of_tokens) {
    case JSMN_ERROR_NOMEM:
        Serial.println("Not enough tokens were provided. Check if the maximum number of tokens is high enough.");
	break;
    case JSMN_ERROR_INVAL:
	Serial.println("Invalid character inside JSON string");
        break;
    case JSMN_ERROR_PART:
	Serial.println("The string is not a full JSON packet, more bytes expected");
        break;
    default:
        Serial.println("Unknown error");
    }
    Serial.println();
    goto fail_parse;
  }
  
  /* Assume the top-level element is an object */
  if (number_of_tokens < 1 || t[0].type != JSMN_OBJECT) {
    Serial.println("Object expected");
    Serial.println("Got: ");
    Serial.println(input);
    goto fail_parse;
  }
  
  token_index = 0;
  return parseObject(input);
fail_parse:
  return NULL;
}

char* parseStringToken(char* input, jsmntok_t* token) {
  char* start = input + token->start;
  size_t length = token->end - token->start;
  char* result = new char[length + 1];
  strncpy(result, start, length);
  result[length] = '\0';
  return result;
}

JsonElement* JsonParser::parseElement(char* input) {
  jsmntok_t &current_token = t[token_index];
  switch (current_token.type) {
    case JSMN_OBJECT:
      return parseObject(input);
    case JSMN_ARRAY:
      return parseArray(input);
    case JSMN_STRING:
      return parseString(input);
    case JSMN_PRIMITIVE: // treat primitives just like strings
      return parseString(input);
  }
  return NULL;
}

JsonString* JsonParser::parseString(char* input) {
  JsonString* string = new JsonString(parseStringToken(input, &t[token_index]));
  return string;
}

JsonArray* JsonParser::parseArray(char* input) {
  JsonArray* array = new JsonArray();
  jsmntok_t &current_token = t[token_index];
  token_index++;
  for (int i = 0; i < current_token.size && token_index < number_of_tokens; token_index++, i++) {
    JsonElement* elem = parseElement(input);
    array->add(elem);
  }
  token_index--;
  return array;
}

JsonObject* JsonParser::parseObject(char* input) {
  JsonObject* result = new JsonObject();
  jsmntok_t &current_token = t[token_index];
  token_index++;
  for (int i = 0; i < current_token.size && token_index < number_of_tokens; token_index++, i++) {
    if (t[token_index].type == JSMN_STRING) {
      char* field_name = parseStringToken(input, &t[token_index]);
      token_index++;
      JsonElement *field_value = parseElement(input);
      result->set(field_name, field_value);
    } else {
      Serial.println("Error: Field name is not a string!");
      goto fail_parse_object;
    }
  }
  token_index--;
  return result;
fail_parse_object:
  return NULL;
}


char *JSON_STRING =
  "{\"user\": {\"name\":\"johndoe\", \"maini\": {\"nr\": 2}}, \"admin\": false,\n  "
  "\"groups\": [\"users\", \"wheel\", \"audio\", \"video\"], \"uid\": 1000}";

static int jsoneq(const char *json, jsmntok_t *tok, const char *s) {
  if (tok->type == JSMN_STRING && (int) strlen(s) == tok->end - tok->start &&
      strncmp(json + tok->start, s, tok->end - tok->start) == 0) {
    return 0;
  }
  return -1;
}

void testJson() {
  JsonParser json_parser;
  char* str = JSON_STRING;
  JsonObject* object = json_parser.parse(str);
  
  Serial.print("User: ");
  Serial.println(((JsonString*)(((JsonObject*)(((JsonObject*)(object->get("user")))->get("maini")))->get("nr")))->getValue());
  Serial.print("uid: ");
  Serial.println(((JsonString*)(object->get("uid")))->getValue());
  Serial.print("Group nr. 1: ");
  Serial.println(((JsonString*)((JsonArray*)(object->get("groups")))->get(1))->getValue());
  
  delete object;
  return;

  int i;
  int r;
  jsmn_parser p;
  jsmntok_t t[1024]; /* We expect no more than 128 tokens */

  jsmn_init(&p);
  r = jsmn_parse(&p, JSON_STRING, strlen(JSON_STRING), t, sizeof(t) / sizeof(t[0]));
  if (r < 0) {
    Serial.println("Failed to parse JSON.");
    return;
  }

  /* Assume the top-level element is an object */
  if (r < 1 || t[0].type != JSMN_OBJECT) {
    Serial.println("Object expected");
    return;
  }

  Serial.println(r);
  char buf[30];
  /* Loop over all keys of the root object */
  for (i = 1; i < r; i++) {
    if (jsoneq(JSON_STRING, &t[i], "user") == 0) {
      /* We may use strndup() to fetch string value */
      strncpy(buf, JSON_STRING + t[i + 1].start, t[i + 1].end - t[i + 1].start);
      Serial.print("User: ");
      Serial.println(buf);
      Serial.print("Token index: ");
      Serial.println(i);
      Serial.print("Token start index: ");
      Serial.println(t[i + 1].start);
      Serial.print("Token end index: ");
      Serial.println(t[i + 1].end);
      Serial.print("Token size: ");
      Serial.println(t[i + 1].size);
      Serial.print("Next token type: ");
      Serial.println(t[i + 5].type);
      i++;
    } else if (jsoneq(JSON_STRING, &t[i], "admin") == 0) {
      /* We may additionally check if the value is either "true" or "false" */
      printf("- Admin: %.*s\n", t[i + 1].end - t[i + 1].start,
             JSON_STRING + t[i + 1].start);
      i++;
    } else if (jsoneq(JSON_STRING, &t[i], "uid") == 0) {
      /* We may want to do strtol() here to get numeric value */
      printf("- UID: %.*s\n", t[i + 1].end - t[i + 1].start,
             JSON_STRING + t[i + 1].start);
      i++;
    } else if (jsoneq(JSON_STRING, &t[i], "groups") == 0) {
      int j;
      printf("- Groups:\n");
      if (t[i + 1].type != JSMN_ARRAY) {
        continue; /* We expect groups to be an array of strings */
      }
      for (j = 0; j < t[i + 1].size; j++) {
        jsmntok_t *g = &t[i + j + 2];
        printf("  * %.*s\n", g->end - g->start, JSON_STRING + g->start);
      }
      i += t[i + 1].size + 1;
    } else {
      printf("Unexpected key: %.*s\n", t[i].end - t[i].start,
             JSON_STRING + t[i].start);
    }
  }
}
