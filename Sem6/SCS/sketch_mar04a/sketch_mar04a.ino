
#undef max
#undef min
#include <vector>
#define max(a,b) ((a)>(b)?(a):(b))
#define min(a,b) ((a)<(b)?(a):(b))
#include <jsmn.h>
#include <stdlib.h>
#include <cstdarg>
#include <time.h>

/* References:
Library tutorial: http://arduino.cc/en/Hacking/LibraryTutorial
JSON parser: http://zserge.com/jsmn.html
*/
/*
 Based on Web client
 by David A. Mellis
 modified 9 Apr 2012
 by Tom Igoe, based on work by Adrian McEwen

 */

#include <SPI.h>
#include <Ethernet.h>

#include <wolfssl/ssl.h>
#include "SSLConnector.h"
#include "ResponseParser.h"
#include "JsonParser.h"

byte mac[] = { 0x98, 0x4F, 0xEE, 0x00, 0x31, 0xAD };

char request_buffer[500];
char tmp_request_buffer[500];
char user_access_token[256];
char user_code[20];
char app_code[128];
bool last_post_is_story;

unsigned long start_arduino_time;
time_t start_real_time;
time_t gmt_offset;

char proxy_server[] = "cache.utcluj.ro";
int proxy_port = 3128;

SSLConnector connector;
ResponseParser parser;

#define ONE_HOUR_IN_SECONDS 3600
int led_pin = 13;

#define TIMEZONE_DB_API_KEY "GX2R40YCSP1N"
#define TIMEZONE_DB_ZONE "Europe/Bucharest"
// https://developers.facebook.com/docs/facebook-login/for-devices
// Response 1: {"code":"c31134a40d767887be04e9d2ef967fcd","user_code":"LFN3BBIB","verification_uri":"https:\/\/www.facebook.com\/device","expires_in":420,"interval":5}
// Response 2: {"access_token":"CAALYPuZBictwBAMo2ZBfZC8yswg295DyvmeHG0MRWvFvpEps0ZCzNqwdpUmt2N67zeaHr8nesyMWx7L3vlarZCTZC61hnqGshK4UoZC1ZCsZCZBo6He7DUT5FIA1iQBLGi1N8ZBCtgK9iCeQsZB9k9zMJeOAH1ZCzeg8sYyBUbIViI5hZBtPzLtG0SKYNXUUrVocLXAbIZD","expires_in":5171995}
//client.println("POST https://graph.facebook.com/oauth/device?type=device_code&client_id=800714773394140&scope=user_posts");
//client.println("POST https://graph.facebook.com/oauth/device?type=device_token&client_id=800714773394140&code=099b8a52f32b82c3b82210f620475a98");
//client.println("GET /v2.5/me?access_token=CAACEdEose0cBACD82LBZCEkJOYR0JdZC5VfjTDzoqvNGOqnKlJMsyTZAteatM7T0r01rofaKpZAIlr2ZAuKy7fZC0IsKg0XkpRvrO3XS6cXtUptgnXsVlnZA2Rx3D5vUnhu0GSPqvLd8JeZBmV6yzQYnco2uxKai6UPSapV6WTPZAjMbUofAxc961tqe0vAMNr9B2hydfimZCX4GZA4cwWRI5F7 HTTP/1.1");
// TODO: extend access token: https://developers.facebook.com/docs/facebook-login/access-tokens/expiration-and-extension
// Permissions: https://developers.facebook.com/docs/facebook-login/permissions


void modifiedAppend(char* &dst, char *src) {
  strcpy(dst, src);
  dst += strlen(dst);
}

/*
Creates a list of arguments in the "out" buffer from the "args" list in the following format:
  key1=value1&key2=value2&...&keyN=valueN

The key1, value1, key2, value2 ... (in this order) are provided in the "args" argument.
For GET requests, a question mark should be added to the start of the string. For that
purpose, call the function with "true" as the third argument.
Returns the position of the end of the string (where the '\0' character is).
*/
char* createArgs(char* out, va_list args, bool add_question_mark = false) {
  char* key = va_arg(args, char*);
  char* value;
  bool first = true;
  while (key != NULL) {
    value = va_arg(args, char*);
    if (first) {
      if (add_question_mark) {
        modifiedAppend(out, "?");
      }
      first = false;
    }
    else {
        modifiedAppend(out, "&");
    }
    modifiedAppend(out, key);
    modifiedAppend(out, "=");
    modifiedAppend(out, value);
    key = va_arg(args, char*);
  }
  *out = '\0';
  return out;
}

/*
Creates a GET request with parameters. The last argument must be NULL.
Example of a call:
  char buf[100];
  createGetRequest("graph.facebook.com", "/", buf, "a", "b", NULL);
Results in:
  GET /?a=b HTTP/1.1
  Host: graph.facebook.com
*/
void createGetRequest(char* host, char* url, char* out, char* tmp_buffer ...) {
  va_list args;
  va_start(args, tmp_buffer);
  
  createArgs(tmp_buffer, args, true);
  
  sprintf(out, "GET %s%s HTTP/1.1\r\nHost: %s\r\nConnection: close\r\n\r\n", url, tmp_buffer, host);
  
  va_end(args);
}

void createPostRequest(char* host, char* url, char* out, char* tmp_buffer, ...) {
  va_list args;
  va_start(args, tmp_buffer);

  createArgs(tmp_buffer, args, false);
  
  sprintf(out, "POST %s HTTP/1.1\r\nHost: %s\r\nFrom: cristipiticul@yahoo.com\r\nUser-Agent: Arduino/1.0\r\nContent-Type: application/x-www-form-urlencoded\r\nContent-Length: %d\r\n\r\n%s\r\n", url, host, strlen(tmp_buffer), tmp_buffer);

  va_end(args);
}

void printTime(time_t time) {
  const tm* splitted_time = localtime(&time);
  char time_pretty_format[30];
  strftime(time_pretty_format, 30, "%a, %d %b %y, %T", splitted_time);
  Serial.println(time_pretty_format);
} 
void getTime() {
  Serial.println("Reading the time from timezonedb");
  createGetRequest("api.timezonedb.com", "http://api.timezonedb.com/", request_buffer, tmp_request_buffer,
    "key", TIMEZONE_DB_API_KEY,
    "zone", TIMEZONE_DB_ZONE,
    "format", "json",
    NULL);
  char* time_zone_response = connector.httpRequest(proxy_server, proxy_port, request_buffer);
  connector.closeConnection();
  
  start_arduino_time = millis() / 1000;
  JsonObject* time_zone_json = parser.getResponseMessage(time_zone_response);
  //TODO: check if the time was successfully retreived.
  JsonString* timestamp_json_string = (JsonString*) time_zone_json->get("timestamp");
  start_real_time = atoi(timestamp_json_string->getValue());
  
  JsonString* gmt_offset_json_string = (JsonString*) time_zone_json->get("gmtOffset");
  gmt_offset = atoi(gmt_offset_json_string->getValue());
  
  delete time_zone_json;
}

void getCode() {
  Serial.println("Getting the code...");
  createPostRequest("graph.facebook.com", "/v2.6/device/login", request_buffer, tmp_request_buffer, 
    "access_token", "800714773394140|154a5388147fb279c63fcfe7b81b52fd",
    "scope", "public_profile,user_posts",
    NULL);
  char* response = connector.sslRequest("graph.facebook.com", 443, request_buffer);
  JsonObject* response_contents = parser.getResponseMessage(response);
  strcpy(user_code, ((JsonString*)(response_contents->get("user_code")))->getValue());
  strcpy(app_code, ((JsonString*)(response_contents->get("code")))->getValue());
  delete response_contents;
}

bool hasLoggedIn() {
  bool logged_in = false;
  Serial.println("Checking if you have logged in...");
  logged_in = false;
  createPostRequest("graph.facebook.com", "/v2.6/device/login_status", request_buffer, tmp_request_buffer,
    "access_token", "800714773394140|154a5388147fb279c63fcfe7b81b52fd",
    "code", app_code,
    NULL);
    
  char* login_check_response = connector.sslRequest("graph.facebook.com", 443, request_buffer);
  int login_check_response_code = parser.getResponseCode(login_check_response);
  if (login_check_response_code == 200) {
    JsonObject* login_check_response_object = parser.getResponseMessage(login_check_response);
    JsonString* user_access_token_json_string = ((JsonString*)(login_check_response_object->get("access_token")));
    if (user_access_token_json_string != NULL) {
      strcpy(user_access_token, user_access_token_json_string->getValue());
      logged_in = true;
    }
    delete login_check_response_object;
  }
  else {
    Serial.println("Not yet...");
  }
  connector.closeConnection();
  return logged_in;
}

// Creates a timestamp from a date-time format like this: 2016-04-23T08:38:05+0000
time_t parseTime(char* time_str)
{
  tm time_splitted;
  time_t result = 0;
  if (strptime(time_str, "%Y-%m-%dT%H:%M:%S", &time_splitted) != NULL) {
    result = mktime(&time_splitted);
  } else {
    Serial.print("Error parsing time string \"");
    Serial.print(time_str);
    Serial.println("\"!");
  }
  return result;
}
time_t getLastPostTime() {
  Serial.println("Retreiving the news feed...");
  createGetRequest("graph.facebook.com", "/v2.6/me/feed", request_buffer, tmp_request_buffer,
    "access_token", user_access_token,
    NULL);
  
  char* facebook_posts_string = connector.sslRequest("graph.facebook.com", 443, request_buffer);
  connector.closeConnection();
  int facebook_posts_response_code = parser.getResponseCode(facebook_posts_string);
  if (facebook_posts_response_code == 200) {
    JsonObject* posts = parser.getResponseMessage(facebook_posts_string);
    Serial.println("The last post:");
    JsonObject* last_post = (JsonObject*) (((JsonArray*) posts->get("data"))->get(0));
    if (last_post->get("story") != NULL) {
      last_post_is_story = true;
    } else {
      last_post_is_story = false;
    }
    time_t result = parseTime(((JsonString*) last_post->get("created_time"))->getValue());
    delete posts;
    return result + gmt_offset; // Facebook returns time in GMT 00, and we have the offset from timezonedb
  }
  else {
    Serial.println("An error occured while reading the facebook posts!");
    Serial.print("Error code: ");
    Serial.println(facebook_posts_response_code, 10);
    return (time_t) 0;
  }
}

void setup() {
  Serial.begin(9600);
  pinMode(led_pin, OUTPUT);

  //connector.enableDebugMsgs();
  //parser.enableDebugMsgs();
  //connector.init(mac);
  connector.init(mac, proxy_server, proxy_port);
  
  getTime();
  Serial.print("Current time: ");
  printTime(start_real_time);

  //connector.enableDebugMsgs();  
  getCode();
  Serial.print("Please enter the following code at https://www.facebook.com/device : ");
  Serial.println(user_code);
  
  bool logged_in;
  do {
    logged_in = hasLoggedIn();
  }
  while (!logged_in);
  Serial.println("The login was successful!");
//  Serial.println("Access token:");
//  Serial.println(user_access_token);
}

void loop() {
  time_t last_post_time = getLastPostTime();
  connector.closeConnection();
  
  time_t current_time = start_real_time + millis() / 1000 - start_arduino_time;
  Serial.print("Last post time: ");
  printTime(last_post_time);
  Serial.print("Current time:   ");
  printTime(current_time);
  bool has_message;
  if ((long int) current_time - (long int) last_post_time <= ONE_HOUR_IN_SECONDS) {
    has_message = true;
  } else {
    has_message = false;
  }
  if (has_message) {
    Serial.println("New message!");
    if (last_post_is_story) {
      Serial.println("And it is a cool story.");
    }
  }
  for (int i = 0; i < 25; i++) {
    if (has_message) {
      digitalWrite(led_pin, HIGH);
    } else {
      digitalWrite(led_pin, LOW);
    }
    delay(100);
    if (has_message && last_post_is_story) {
      digitalWrite(led_pin, HIGH);
    } else {
      digitalWrite(led_pin, LOW);
    }
    delay(100);
  }
}


