//-----------------------------------------------------------------------------
// Filename: DtlsHandshakeTest.h
//
// Description: Test console that executes the client and server ends of a DTLS
// handshake on two separate threads.
//
// Author(s):
// Aaron Clauson (aaron@sipsorcery.com)
//
// History:
// 29 Jan 2020  Aaron Clauson	  Created, Dublin, Ireland.
//
// License: 
// Creative Commons Zero v1.0 Universal, see included LICENSE file.
//
// OpenSSL License:
// This application includes software developed by the OpenSSL Project and 
// cryptographic software written by Eric Young (eay@cryptsoft.com)
// See https://github.com/openssl/openssl/blob/master/LICENSE for conditions.
//-----------------------------------------------------------------------------

#include <openssl/bio.h>
#include <openssl/dtls1.h>
#include <openssl/err.h>
#include <openssl/ssl.h>
#include <winsock2.h>
#include <ws2tcpip.h>

#include <iostream>
#include <thread>

#pragma comment(lib, "Ws2_32.lib")

#define SERVER_PORT 9000
#define CLIENT_PORT 9001
#define CERTIFICATE_PATH "localhost.pem"
#define CERTIFICATE_KEY_PATH "localhost_key.pem"
#define ERROR_BUFFER_SIZE 2048
#define HANDSHAKE_TIMEOUT_SECONDS 15
#define DTLS_COOKIE "dummy"

#define SSL_WHERE_INFO(ssl, w, flag, msg) {                \
    if(w & flag) {                                         \
      printf("%20.20s", msg);                              \
      printf(" - %30.30s ", SSL_state_string_long(ssl));   \
      printf(" - %5.10s ", SSL_state_string(ssl));         \
      printf("\n");                                        \
	    }                                                    \
    } 

enum class AddressFamily
{
  IPv4,
  IPv6
};

// Forward function definitions.
void RunServer(AddressFamily addrFamily);
void RunClient(AddressFamily addrFamily);

void info_callback(const SSL* ssl, int where, int ret)
{
  if (ret == 0) {
    printf("-- krx_ssl_info_callback: error occurred.\n");
    return;
  }

  SSL_WHERE_INFO(ssl, where, SSL_CB_LOOP, "LOOP");
  SSL_WHERE_INFO(ssl, where, SSL_CB_HANDSHAKE_START, "HANDSHAKE START");
  SSL_WHERE_INFO(ssl, where, SSL_CB_HANDSHAKE_DONE, "HANDSHAKE DONE");
}

int verify_cookie(SSL* ssl, const unsigned char* cookie, unsigned int cookie_len)
{
  // Accept any cookie.
  return 1;
}

int generate_cookie(SSL* ssl, unsigned char* cookie, unsigned int* cookie_len)
{
  int cookieLength = sizeof(DTLS_COOKIE);
  *cookie_len = cookieLength;
  memcpy(cookie, (unsigned char*)DTLS_COOKIE, cookieLength);
  return 1;
}

int main()
{
  std::cout << "DTLS Test Console:" << std::endl;

  // Initialise Windows sockets.
  WSADATA w = { 0 };
  int error = WSAStartup(0x0202, &w);

  if (error || w.wVersion != 0x0202) {
    std::cerr << "Could not initialise Winsock2." << std::endl;
    return -1;
  }

  // Initialise OpenSSL.
  SSL_library_init();
  SSL_load_error_strings();
  ERR_load_BIO_strings();
  OpenSSL_add_all_algorithms();

  //AddressFamily addrFamily = AddressFamily::IPv6;
  AddressFamily addrFamily = AddressFamily::IPv4;

  // Start DTLS server thread.
  std::thread svrThd(RunServer, addrFamily);

  Sleep(2000);

  // Start DTLS client thread.
  std::thread cliThd(RunClient, addrFamily);

  std::cout << "Press any key to exit..." << std::endl ;
  auto o = getchar();

  cliThd.join();
  svrThd.join();
}

/**
 * Attempts to bind a UDP server socket and hand it off to OpenSSL to
 * complete the server end of a DTLS handshake.
 */
void RunServer(AddressFamily addrFamily)
{
  SOCKET svrSock = 0;
  sockaddr* svrAddr = nullptr;
  int svrAddrSize = 0;
  sockaddr_in svrAddr4 = { 0 };
  sockaddr_in6 svrAddr6 = { 0 };
  SSL_CTX* ctx = nullptr;
  SSL* ssl = nullptr;
  BIO* bio = nullptr;
  int res = 0;
  char buf[ERROR_BUFFER_SIZE];
  BIO_ADDR* clientAddr = BIO_ADDR_new();
  struct timeval timeout;
  OSSL_HANDSHAKE_STATE handshakeState = OSSL_HANDSHAKE_STATE::TLS_ST_BEFORE;

  std::cout << "RunServer..." << std::endl;

  // Dump any openssl errors.
  ERR_print_errors_fp(stderr);

  // Bind to a UDP socket that will listen for client handshakes.
  svrSock = socket((addrFamily == AddressFamily::IPv6) ? AF_INET6 : AF_INET, 
    SOCK_DGRAM, IPPROTO_UDP);
  if (svrSock == INVALID_SOCKET || svrSock < 0) {
    std::cerr << "Server socket initialisation failed." << std::endl;
    goto cleanup;
  }

  if (addrFamily == AddressFamily::IPv6) {
    svrAddr6.sin6_family = AF_INET6;
    svrAddr6.sin6_addr = in6addr_loopback;
    svrAddr6.sin6_port = htons(SERVER_PORT);

    svrAddr = (sockaddr*)&svrAddr6;
    svrAddrSize = sizeof(svrAddr6);
  }
  else {
    svrAddr4.sin_family = AF_INET;
    svrAddr4.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    svrAddr4.sin_port = htons(SERVER_PORT);

    svrAddr = (sockaddr*)&svrAddr4;
    svrAddrSize = sizeof(svrAddr4);
  }

  res = bind(svrSock, svrAddr, svrAddrSize);
  if (res == SOCKET_ERROR) {
    wprintf(L"Server socket bind failed with error: %ld\n", WSAGetLastError());
    goto cleanup;
  }

  // Create a new DTLS context.
  ctx = SSL_CTX_new(DTLS_server_method());
  if (!ctx) {
    printf("Error: cannot create SSL_CTX.\n");
    ERR_print_errors_fp(stderr);
    goto cleanup;
  }

  // Set our supported ciphers.
  //res = SSL_CTX_set_cipher_list(ctx, "ALL:!ADH:!LOW:!EXP:!MD5:@STRENGTH");
  res = SSL_CTX_set_cipher_list(ctx, "ALL:NULL:eNULL:aNULL");
  if (res != 1) {
    printf("Error: cannot set the cipher list.\n");
    ERR_print_errors_fp(stderr);
    goto cleanup;
  }

  // Load certificate.
  res = SSL_CTX_use_certificate_file(ctx, CERTIFICATE_PATH, SSL_FILETYPE_PEM);
  if (res != 1) {
    printf("Error: cannot load certificate file.\n");
    ERR_print_errors_fp(stderr);
    goto cleanup;
  }

  /* load private key */
  res = SSL_CTX_use_PrivateKey_file(ctx, CERTIFICATE_KEY_PATH, SSL_FILETYPE_PEM);
  if (res != 1) {
    printf("Error: cannot load private key file.\n");
    ERR_print_errors_fp(stderr);
    goto cleanup;
  }

  // Check if the private key is valid.
  res = SSL_CTX_check_private_key(ctx);
  if (res != 1) {
    printf("Error: checking the private key failed. \n");
    ERR_print_errors_fp(stderr);
    goto cleanup;
  }

  // The client doesn't have to send it's certificate.
  SSL_CTX_set_verify(ctx, SSL_VERIFY_PEER, nullptr);
  SSL_CTX_set_cookie_generate_cb(ctx, generate_cookie);
  SSL_CTX_set_cookie_verify_cb(ctx, verify_cookie);

  // Create SSL.
  ssl = SSL_new(ctx);
  if (!ssl) {
    printf("Error: cannot create new SSL.\n");
    goto cleanup;
  }

  // Create Basic I/O.
  bio = BIO_new_dgram(svrSock, BIO_NOCLOSE);
  if (!bio) {
    printf("Error: cannot create new BIO.\n");
    goto cleanup;
  }

  /* Set and activate timeouts */
  timeout.tv_sec = HANDSHAKE_TIMEOUT_SECONDS;
  timeout.tv_usec = 0;
  BIO_ctrl(bio, BIO_CTRL_DGRAM_SET_RECV_TIMEOUT, 0, &timeout);

  SSL_set_bio(ssl, bio, bio);
  SSL_set_info_callback(ssl, info_callback);
  SSL_set_options(ssl, SSL_OP_COOKIE_EXCHANGE);

  // Act as the server end of the DTLS connection.
  SSL_set_accept_state(ssl);

  // Checks that the cookie has been set in client hello.
  while (DTLSv1_listen(ssl, clientAddr) <= 0);

  printf("New DTLS client connection.\n");

  // Finish handshake.
  do { res = SSL_accept(ssl); } while (res == 0);
  if (res < 0) {
    perror("SSL_accept");
    printf("%s\n", ERR_error_string(ERR_get_error(), buf));
    goto cleanup;
  }

  handshakeState = SSL_get_state(ssl);
  printf("Server handshake state %d, is complete %d.\n", handshakeState, handshakeState == TLS_ST_OK);

  //printf("Server SSL state %d.\n", ssl->);

cleanup:

  // Dump any openssl errors.
  ERR_print_errors_fp(stderr);

  BIO_ADDR_free(clientAddr);

  if (ssl != nullptr) {
    SSL_shutdown(ssl);
    SSL_free(ssl);
  }

  if (ctx != nullptr) {
    SSL_CTX_free(ctx);
  }

  closesocket(svrSock);

  std::cout << "RunServer finished." << std::endl;
}

/**
 * Attempts to bind a UDP client socket and hand it off to OpenSSL to
 * complete the client end of a DTLS handshake.
 */
void RunClient(AddressFamily addrFamily)
{
  SOCKET cliSock = 0;
  sockaddr* cliAddr = nullptr;
  int cliAddrSize = 0;
  sockaddr_in cliAddr4 = { 0 };
  sockaddr_in6 cliAddr6 = { 0 };
  sockaddr* svrAddr = nullptr;
  int svrAddrSize = 0;
  sockaddr_in svrAddr4 = { 0 };
  sockaddr_in6 svrAddr6 = { 0 };
  int res = 0;
  SSL_CTX* ctx = nullptr;
  SSL* ssl = nullptr;
  BIO* bio = nullptr;
  char buf[ERROR_BUFFER_SIZE];
  struct timeval timeout;
  OSSL_HANDSHAKE_STATE handshakeState = OSSL_HANDSHAKE_STATE::TLS_ST_BEFORE;

  std::cout << "RunClient..." << std::endl;

  // Dump any openssl errors.
  ERR_print_errors_fp(stderr);

  cliSock = socket((addrFamily == AddressFamily::IPv6) ? AF_INET6 : AF_INET, 
    SOCK_DGRAM, IPPROTO_UDP);
  if (cliSock == INVALID_SOCKET || cliSock < 0) {
    std::cerr << "Client socket initialisation failed." << std::endl;
    goto cleanup;
  }

  if (addrFamily == AddressFamily::IPv6) {
    cliAddr6.sin6_family = AF_INET6;
    cliAddr6.sin6_addr = in6addr_loopback;
    cliAddr6.sin6_port = htons(CLIENT_PORT);

    cliAddr = (sockaddr*)&cliAddr6;
    cliAddrSize = sizeof(cliAddr6);

    svrAddr6.sin6_family = AF_INET6;
    svrAddr6.sin6_addr = in6addr_loopback;
    svrAddr6.sin6_port = htons(SERVER_PORT);

    svrAddr = (sockaddr*)&svrAddr6;
    svrAddrSize = sizeof(svrAddr6);
  }
  else {
    cliAddr4.sin_family = AF_INET;
    cliAddr4.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    cliAddr4.sin_port = htons(CLIENT_PORT);

    cliAddr = (sockaddr*)&cliAddr4;
    cliAddrSize = sizeof(cliAddr4);

    svrAddr4.sin_family = AF_INET;
    svrAddr4.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    svrAddr4.sin_port = htons(SERVER_PORT);

    svrAddr = (sockaddr*)&svrAddr4;
    svrAddrSize = sizeof(svrAddr4);
  }

  res = bind(cliSock, cliAddr, cliAddrSize);
  if (res == SOCKET_ERROR) {
    wprintf(L"Client socket bind failed with error: %ld\n", WSAGetLastError());
    closesocket(cliSock);
    goto cleanup;
  }

  // Even though it's UDP we call connect to set the destination socket.
  res = connect(cliSock, svrAddr, svrAddrSize);
  if (res == SOCKET_ERROR) {
    wprintf(L"Client socket connect failed with error: %ld\n", WSAGetLastError());
    closesocket(cliSock);
    goto cleanup;
  }

  // Create a new DTLS context.
  ctx = SSL_CTX_new(DTLS_client_method());
  if (!ctx) {
    printf("Error: cannot create SSL_CTX.\n");
    ERR_print_errors_fp(stderr);
    goto cleanup;
  }

  // Set our supported ciphers.
  //res = SSL_CTX_set_cipher_list(ctx, "ALL:!ADH:!LOW:!EXP:!MD5:@STRENGTH");
  res = SSL_CTX_set_cipher_list(ctx, "ALL:NULL:eNULL:aNULL");
  if (res != 1) {
    printf("Error: cannot set the cipher list.\n");
    ERR_print_errors_fp(stderr);
    goto cleanup;
  }

  SSL_CTX_set_ecdh_auto(ctx, 1);                        // Needed for FireFox DTLS negotiation.
  SSL_CTX_set_verify(ctx, SSL_VERIFY_NONE, nullptr);    // The client doesn't have to send it's certificate.

  // Create SSL.
  ssl = SSL_new(ctx);
  if (!ssl) {
    printf("Error: cannot create new SSL.\n");
    goto cleanup;
  }

  // Create Basic I/O.
  bio = BIO_new_dgram(cliSock, BIO_NOCLOSE);
  if (!bio) {
    printf("Error: cannot create new BIO.\n");
    goto cleanup;
  }

  SSL_set_bio(ssl, bio, bio);
  SSL_set_info_callback(ssl, info_callback);

  SSL_set_connect_state(ssl);

  /* Set and activate timeouts */
  timeout.tv_sec = HANDSHAKE_TIMEOUT_SECONDS;
  timeout.tv_usec = 0;
  BIO_ctrl(bio, BIO_CTRL_DGRAM_SET_SEND_TIMEOUT, 0, &timeout);

  if (BIO_ctrl(bio, BIO_CTRL_DGRAM_SET_CONNECTED, 0, &svrAddr) <= 0) {
    printf("Error: BIO_CTL to set BIO_CTRL_DGRAM_SET_CONNECTED failed.\n");
  }

  // Exits on error (<0) or success (==1).
  do { res = SSL_connect(ssl); } while (res == 0);
  if (res < 0) {
    perror("SSL_connect");
    printf("%s\n", ERR_error_string(ERR_get_error(), buf));
    goto cleanup;
  }

  handshakeState = SSL_get_state(ssl);
  printf("Client handshake state %d, is complete %d.\n", handshakeState, handshakeState == TLS_ST_OK);

cleanup:

  // Dump any openssl errors.
  ERR_print_errors_fp(stderr);

  if (ssl != nullptr) {
    SSL_shutdown(ssl);
    SSL_free(ssl);
  }

  if (ctx != nullptr) {
    SSL_CTX_free(ctx);
  }

  closesocket(cliSock);

  std::cout << "RunClient finished." << std::endl;
}