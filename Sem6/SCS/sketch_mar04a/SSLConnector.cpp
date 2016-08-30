#include "SSLConnector.h"

// For wolf ssl recv / send functions.
EthernetClient *pClient;

void SSLConnector::init(byte mac[])
{
  setupInternet(mac);
  wolfssl_init();
  pClient = &client;
  has_proxy = false;
}

void SSLConnector::init(byte mac[], char proxy_server[], int proxy_port)
{
  this->init(mac);
  strcpy(this->proxy_server, proxy_server);
  this->proxy_port = proxy_port;
  has_proxy = true;
}

void SSLConnector::enableDebugMsgs()
{
  debug_msgs = true;
}

void SSLConnector::setupInternet(byte mac[])
{
  // enable the ethernet connector (using a Linux command):
  // https://communities.intel.com/thread/53373?start=0&tstart=0
  if (debug_msgs) {
    Serial.println("Starting the ethernet controller");
  }
  system("ifup eth0");
  if (debug_msgs) {
    Serial.println("Initializing");
  }
  delay(3000);


  // start the Ethernet connection:
  if (Ethernet.begin(mac) == 0) {
    Serial.println("Failed to configure Ethernet using DHCP");
  }
  // give the Ethernet shield a second to initialize:
  delay(1000);
  if (debug_msgs) {
    Serial.print("Connected to internet. IP: ");
    Serial.println(Ethernet.localIP());
  }
}

bool SSLConnector::connectHTTPS(char server[], int port)
{
  if (!has_proxy) {
    return client.connect(server, port);
  } else {
    if (!client.connect(proxy_server, proxy_port)) {
      return false;
    }
    client.print("CONNECT ");
    client.print(server);
    client.print(":");
    client.print(port);
    client.print("\r\n");
    // READING THE DATA FROM PROXY, OTHERWISE BAD THINGS (wolfSSL_connect: record layer version error) CAN HAPPEN!!!!!! 
    if (debug_msgs) {
      Serial.println("Proxy response");
    }
    while (client.available()) {
      char c = client.read();
      if (debug_msgs) {
        Serial.print(c);
      }
    }
    Serial.println("Proxy ok!");
    if (debug_msgs) {
      Serial.println("End of proxy response");
    }
  }
}

bool SSLConnector::connectHTTP(char server[], int port)
{
  if (!has_proxy) {
    return client.connect(server, port);
  } else {
    return client.connect(proxy_server, proxy_port);
  }
}

char* SSLConnector::httpRequest(char server[], int port, char request[])
{
  if (debug_msgs) {
    Serial.print("Connecting to ");
    Serial.println(server);
  }
  if (connectHTTP(server, port)) {
    if (debug_msgs) {
      Serial.println("connected");
    }
    client.println(request);
    
    int n = 0;
    while (n == 0) {
      while (client.available()) {
        char c = client.read();
        response_message[n] = c;
        n++;
      }
      delay(100);
    }
    response_message[n] = '\0';
    if (debug_msgs) {
      Serial.print("Number of characters received: ");
      Serial.println(n);
    }
    
    return response_message;
  } else {
    // if you didn't get a connection to the server:
    if (debug_msgs) {
      Serial.println("connection failed");
    }
    return NULL;
  }
}

void SSLConnector::closeConnection()
{
  if (debug_msgs) {
    Serial.println("disconnecting.");
  }
  client.stop();
}


/**
  WOLFSSL STUFF:
*/

int client_send (WOLFSSL *_ssl, char *buf, int sz, void *_ctx);
int client_recv (WOLFSSL *_ssl, char *buf, int sz, void *_ctx);

int SSLConnector::wolfssl_init()
{
  char err[17];
  // Create our SSL context
  wolfSSL_Init();
  method = wolfTLSv1_2_client_method();
  ctx = wolfSSL_CTX_new(method);
  if ( ctx == NULL ) return 0;
  // Don't do certification verification
  wolfSSL_CTX_set_verify(ctx, SSL_VERIFY_NONE, 0);
  /*
  int errcode;
  if ((errcode = wolfSSL_CTX_use_certificate_chain_file(ctx, "/home/root/certs/client-cert.pem")) != SSL_SUCCESS) {
    Serial.print("Error code:");
    Serial.println(errcode, 10);
    Serial.println("can't load client cert file, check file and run from wolfSSL home dir");
    return 0;
  }

  if (errcode = wolfSSL_CTX_use_PrivateKey_file(ctx, "/home/root/certs/client-key.pem", SSL_FILETYPE_PEM)
                != SSL_SUCCESS) {
    Serial.print("Error code:");
    Serial.println(errcode, 10);
    Serial.println("can't load client private key file, check file and run "
                   "from wolfSSL home dir");
    return 0;
  }*/

  // Specify callbacks for reading to/writing from the socket (EthernetClient
  // object).

  wolfSSL_SetIORecv(ctx, client_recv);
  wolfSSL_SetIOSend(ctx, client_send);

  return 1;
}

int client_recv (WOLFSSL *_ssl, char *buf, int sz, void *_ctx)
{
  int i = 0;

  // Read a byte while one is available, and while our buffer isn't full.
  while ( pClient->available() > 0 && i < sz) {
    buf[i++] = pClient->read();
  }

  return i;
}

int client_send (WOLFSSL *_ssl, char *buf, int sz, void *_ctx)
{
  int n = pClient->write((byte *) buf, sz);
  return n;
}

char* SSLConnector::sslRequest(char server[], int port, char request[]) {
  char errstr[81];
  char buf[256];
  int err;
  int bwritten, bread, totread;
  if (debug_msgs) {
    Serial.print("Connecting to ");
    Serial.println(server);
  }
  if ( connectHTTPS(server, port) ) {
    if (debug_msgs) {
      Serial.println("connected");
    }

    ssl = wolfSSL_new(ctx);
    if ( ssl == NULL ) {
      err = wolfSSL_get_error(ssl, 0);
      wolfSSL_ERR_error_string_n(err, errstr, 80);
      Serial.print("wolfSSL_new error: ");
      Serial.println(errstr);
      goto fail;
    }

    if ( wolfSSL_connect(ssl) != SSL_SUCCESS ) {
      err = wolfSSL_get_error(ssl, 0);
      wolfSSL_ERR_error_string_n(err, errstr, 80);
      Serial.print("wolfSSL_connect error: ");
      Serial.println(errstr);
      goto fail;
    }

    bwritten = wolfSSL_write(ssl, (char *) request, strlen(request));

    if (debug_msgs) {
      Serial.print("Bytes written: ");
      Serial.print(bwritten);
      Serial.print(". Should be: ");
      Serial.println(strlen(request));
    }

    if ( bwritten > 0 ) {
      totread = 0;
      response_message[0] = '\0';
      while ( client.available() || wolfSSL_pending(ssl) ) {
        bread = wolfSSL_read(ssl, buf, sizeof(buf) - 1);

        if ( bread >= 0 ) {
          buf[bread] = '\0';
          strcpy(response_message + totread, buf);
        } else {
          Serial.println();
          Serial.print("Read error at char #");
          Serial.println(totread);
          Serial.println();
        }
        totread += bread;
      }

      if (debug_msgs) {
        Serial.print("Bytes read= ");
        Serial.println(totread);
      }
    } else {
      err = wolfSSL_get_error(ssl, 0);
      wolfSSL_ERR_error_string_n(err, errstr, 80);
      Serial.print("wolfSSL_write error: ");
      Serial.println(errstr);
    }

    if ( ssl != NULL ) wolfSSL_free(ssl);
fail:
    client.stop();
    if (debug_msgs) {
      Serial.println("Connection closed");
    }
    return response_message;
  }
}


