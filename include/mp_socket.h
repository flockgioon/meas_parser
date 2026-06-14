#pragma once

#include <stdio.h>
#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>

typedef SOCKET mp_socket_t;
#define MP_INVALID_SOCKET INVALID_SOCKET
#define mp_socket_close closesocket
#define mp_sleep(s) Sleep((s) * 1000)

// MSVC
#ifdef _MSC_VER
#include <basetsd.h>
typedef SSIZE_T ssize_t;
#endif

static inline int mp_net_init(void) {
  WSADATA wsa;
  return WSAStartup(MAKEWORD(2, 2), &wsa);
}
static inline void mp_net_cleanup(void) { WSACleanup(); }
static inline void mp_print_socket_error(const char *msg) {
  fprintf(stderr, "%s: error %d\n", msg, WSAGetLastError());
}

#else
// Linux / macOS
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

typedef int mp_socket_t;
#define MP_INVALID_SOCKET (-1)
#define mp_socket_close close
#define mp_sleep(s) sleep((s))

static inline int mp_net_init(void) { return 0; }
static inline void mp_net_cleanup(void) {}
static inline void mp_print_socket_error(const char *msg) { perror(msg); }
#endif
