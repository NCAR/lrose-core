// Standalone listener for the antenna controller's ASCII UDP telemetry.

#include <arpa/inet.h>
#include <cerrno>
#include <csignal>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <limits>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

namespace {

const char *const kDefaultBindAddress = "0.0.0.0";
const int kDefaultPort = 5001;
const std::size_t kMaxUdpDatagramBytes = 65536;

volatile std::sig_atomic_t gStopRequested = 0;

void handleSignal(int)
{
  // Only set a signal-safe flag here. The signal interrupts recvfrom(), and
  // normal control flow then owns socket shutdown and stream cleanup.
  gStopRequested = 1;
}

void printUsage(const char *programName)
{
  std::cerr
      << "Usage: " << programName << " [-a bind_address] [-p port] [-s]\n"
      << "\n"
      << "Listen for ASCII antenna telemetry and write each UDP datagram to "
         "stdout.\n"
      << "\n"
      << "Options:\n"
      << "  -a address  Local IPv4 address to bind (default: "
      << kDefaultBindAddress << ")\n"
      << "  -p port     UDP port to bind (default: " << kDefaultPort << ")\n"
      << "  -s          Prefix each datagram with its sender address and port\n"
      << "  -h          Show this help\n";
}

bool parsePort(const char *text, int &port)
{
  char *end = NULL;
  errno = 0;
  const long value = std::strtol(text, &end, 10);
  if (errno != 0 || end == text || *end != '\0' || value < 1 ||
      value > std::numeric_limits<unsigned short>::max()) {
    return false;
  }
  port = static_cast<int>(value);
  return true;
}

bool installSignalHandlers()
{
  struct sigaction action;
  std::memset(&action, 0, sizeof(action));
  action.sa_handler = handleSignal;
  sigemptyset(&action.sa_mask);

  // Deliberately omit SA_RESTART so a termination signal wakes recvfrom().
  return sigaction(SIGINT, &action, NULL) == 0 &&
         sigaction(SIGTERM, &action, NULL) == 0;
}

}  // namespace

int main(int argc, char **argv)
{
  const char *bindAddress = kDefaultBindAddress;
  int port = kDefaultPort;
  bool showSender = false;

  int option = 0;
  while ((option = getopt(argc, argv, "a:p:sh")) != -1) {
    switch (option) {
      case 'a':
        bindAddress = optarg;
        break;
      case 'p':
        if (!parsePort(optarg, port)) {
          std::cerr << "Invalid UDP port: " << optarg << "\n";
          printUsage(argv[0]);
          return EXIT_FAILURE;
        }
        break;
      case 's':
        showSender = true;
        break;
      case 'h':
        printUsage(argv[0]);
        return EXIT_SUCCESS;
      default:
        printUsage(argv[0]);
        return EXIT_FAILURE;
    }
  }

  if (optind != argc) {
    std::cerr << "Unexpected argument: " << argv[optind] << "\n";
    printUsage(argv[0]);
    return EXIT_FAILURE;
  }

  struct sockaddr_in localAddress;
  std::memset(&localAddress, 0, sizeof(localAddress));
  localAddress.sin_family = AF_INET;
  localAddress.sin_port = htons(static_cast<unsigned short>(port));
  if (inet_pton(AF_INET, bindAddress, &localAddress.sin_addr) != 1) {
    std::cerr << "Invalid IPv4 bind address: " << bindAddress << "\n";
    return EXIT_FAILURE;
  }

  const int socketFd = socket(AF_INET, SOCK_DGRAM, 0);
  if (socketFd < 0) {
    std::cerr << "Cannot create UDP socket: " << std::strerror(errno) << "\n";
    return EXIT_FAILURE;
  }

  const int reuseAddress = 1;
  if (setsockopt(socketFd, SOL_SOCKET, SO_REUSEADDR, &reuseAddress,
                 sizeof(reuseAddress)) != 0) {
    std::cerr << "Cannot set SO_REUSEADDR: " << std::strerror(errno) << "\n";
    close(socketFd);
    return EXIT_FAILURE;
  }

  if (bind(socketFd, reinterpret_cast<struct sockaddr *>(&localAddress),
           sizeof(localAddress)) != 0) {
    std::cerr << "Cannot bind UDP socket to " << bindAddress << ":" << port
              << ": " << std::strerror(errno) << "\n";
    close(socketFd);
    return EXIT_FAILURE;
  }

  if (!installSignalHandlers()) {
    std::cerr << "Cannot install signal handlers: " << std::strerror(errno)
              << "\n";
    close(socketFd);
    return EXIT_FAILURE;
  }

  std::cerr << "Listening for ASCII UDP telemetry on " << bindAddress << ":"
            << port << " (Ctrl-C to stop)\n";

  char buffer[kMaxUdpDatagramBytes];
  while (!gStopRequested) {
    struct sockaddr_in senderAddress;
    socklen_t senderLength = sizeof(senderAddress);
    const ssize_t received =
        recvfrom(socketFd, buffer, sizeof(buffer), 0,
                 reinterpret_cast<struct sockaddr *>(&senderAddress),
                 &senderLength);

    if (received < 0) {
      if (errno == EINTR) {
        continue;
      }
      std::cerr << "UDP receive failed: " << std::strerror(errno) << "\n";
      close(socketFd);
      return EXIT_FAILURE;
    }

    if (showSender) {
      char senderText[INET_ADDRSTRLEN];
      const char *converted = inet_ntop(AF_INET, &senderAddress.sin_addr,
                                        senderText, sizeof(senderText));
      std::cout << "[" << (converted == NULL ? "unknown" : converted) << ":"
                << ntohs(senderAddress.sin_port) << "] ";
    }

    std::cout.write(buffer, received);
    if (received == 0 || buffer[received - 1] != '\n') {
      std::cout.put('\n');
    }
    if (!std::cout) {
      std::cerr << "Cannot write telemetry to stdout\n";
      close(socketFd);
      return EXIT_FAILURE;
    }
  }

  close(socketFd);
  std::cerr << "Listener stopped\n";
  return EXIT_SUCCESS;
}
