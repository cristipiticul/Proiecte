#ifndef SSL_CONNECTOR_H
#define SSL_CONNECTOR_H

#include "Arduino.h"
#include <Ethernet.h>
#include <wolfssl/ssl.h>
#include "types.h"

#define MAX_CONTENT_LENGTH 16192

class SSLConnector
{
  public:
    void init(byte mac[]);
    void init(byte mac[], char proxy_server[], int proxy_port);
    void closeConnection();
    char* httpRequest(char server[], int port, char request[]);
    void enableDebugMsgs();
    char* sslRequest(char server[], int port, char request[]);
  private:
    void setupInternet(byte mac[]);
    int getResponseCode();
    void readResponseUntilEmptyLine();
    bool getResponseMessage();
    int wolfssl_init();
    bool connectHTTPS(char* server, int port);
    bool connectHTTP(char* server, int port);
    
    EthernetClient client;
    
    // The HTTP response status line
    // It looks like: "HTTP/1.1 200 OK"
    // See more here: http://www.tcpipguide.com/free/t_HTTPResponseMessageFormat.htm
    char status_code[10];
    
    char response_message[MAX_CONTENT_LENGTH];
    
    const int OK_RESPONSE_CODE = 200;
    
    bool debug_msgs = false;
    
    WOLFSSL_CTX *ctx= NULL;
    WOLFSSL *ssl= NULL;
    WOLFSSL_METHOD *method= NULL;
    
    char proxy_server[30];
    int proxy_port;
    bool has_proxy;
  
};

#endif
