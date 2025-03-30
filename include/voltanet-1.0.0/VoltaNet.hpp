#ifndef VOLTANET_HPP
#define VOLTANET_HPP

#include <vector>
#include <array>
#include <optional>
#include <string>
#include <stdexcept>
#include <memory>
#include <chrono>
#include <unordered_map>
#include <unordered_set>
#include <queue>
#include <random>
#include <mutex>
#include <cstdint>
#include <cstring>
#include <algorithm>
#include <string_view>
#include <cmath>
#include <functional>
#include <atomic>
#include <limits>

#include "sha.h"
#include "filters.h"
#include "hex.h"

#ifdef _WIN32
    #include <winsock2.h>
    #include <ws2tcpip.h>
    #pragma comment(lib, "Ws2_32.lib")
#else
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
    #include <unistd.h>
    #include <fcntl.h>
#endif

namespace VoltaNet {
    static_assert(sizeof(uint64_t) == 8, "uint64_t must be 8 bytes");
    static_assert(sizeof(uint32_t) == 4, "uint32_t must be 4 bytes");

    static constexpr size_t MAX_PACKET_SIZE = 1400;
    static constexpr size_t HEADER_SIZE = 24;
    static constexpr size_t MAX_BUFFER_SIZE = MAX_PACKET_SIZE + HEADER_SIZE;
    static constexpr std::chrono::milliseconds DEFAULT_CONNECTION_TIMEOUT{5000};
    static constexpr uint8_t HANDSHAKE_INIT = 0x01;
    static constexpr uint8_t HANDSHAKE_RESPONSE = 0x02;
    static constexpr uint64_t HALF_MAX = 0x7FFFFFFFFFFFFFFFULL;
    static constexpr uint64_t SEQUENCE_WINDOW = 10000;
    static constexpr size_t MAX_RECENT_SEQUENCES = 1000;
    static constexpr size_t MAX_PACKETS_PER_IP_PER_SEC = 100;

    using LogCallback = std::function<void(std::string_view)>;

    inline bool isSequenceGreater(uint64_t a, uint64_t b, uint64_t window = HALF_MAX / 2) {
        const uint64_t diff = a - b;
        if (diff == 0) return false;
        return (a > b && diff <= window) || (a < b && diff > (HALF_MAX - window));
    }

    struct Packet {
        std::array<uint8_t, MAX_PACKET_SIZE> data{};
        size_t dataSize{0};
        sockaddr_storage address{};
        bool isReliable{false};
        uint64_t sequence{0};

        Packet();
        void setData(const uint8_t* src, size_t size);
        Packet clone() const;
        bool operator<(const Packet& other) const;

        Packet(const Packet&) = delete;
        Packet& operator=(const Packet&) = delete;
        Packet(Packet&&) noexcept = default;
        Packet& operator=(Packet&&) noexcept = default;
    };

    struct BufferConfig {
        size_t maxPendingSize = 100;
        size_t maxReceivedSize = 1000;
        size_t maxJitterSize = 500;
        size_t maxPacketsPerUpdate = 1000;

        BufferConfig() = default;
        BufferConfig(size_t pending, size_t received, size_t jitter, size_t packets = 1000)
            : maxPendingSize(pending), maxReceivedSize(received), maxJitterSize(jitter), maxPacketsPerUpdate(packets) {}
    };

    class Connection {
    public:
        Connection(sockaddr_storage addr, uint64_t sessionToken, BufferConfig config = BufferConfig(),
                  std::chrono::milliseconds timeout = DEFAULT_CONNECTION_TIMEOUT);
        
        bool isVerified() const;
        void verify();
        sockaddr_storage getAddress() const;
        uint64_t getSessionToken() const;
        uint64_t nextSequence();
        std::chrono::steady_clock::time_point getHandshakeTimestamp() const;
        std::chrono::steady_clock::time_point getLastActivity() const;
        bool isTimedOut(std::chrono::steady_clock::time_point now) const;
        bool queueReliablePacket(std::unique_ptr<Packet> packet);
        void queueReceivedPacket(uint64_t sequence, std::unique_ptr<Packet> packet);
        void queueUnreliablePacket(std::unique_ptr<Packet> packet, std::chrono::steady_clock::time_point senderTimestamp);
        std::optional<std::unique_ptr<Packet>> getNextReliablePacket();
        std::optional<std::unique_ptr<Packet>> getNextUnreliablePacket(std::chrono::milliseconds minJitterDelay = std::chrono::milliseconds(10));
        void ackReceived(uint64_t sequence);
        std::unordered_map<uint64_t, std::pair<std::unique_ptr<Packet>, std::chrono::steady_clock::time_point>> getPending() const;
        void modifyPending(std::function<void(std::unordered_map<uint64_t, 
                              std::pair<std::unique_ptr<Packet>, std::chrono::steady_clock::time_point>>&)> callback);
        bool isAcked(uint64_t sequence) const;

    private:
        mutable std::mutex mutex_;
        sockaddr_storage address_;
        std::atomic<uint64_t> sessionToken_;
        std::atomic<bool> verified_;
        std::atomic<uint64_t> nextSequence_;
        std::atomic<uint64_t> nextExpectedSequence_;
        std::chrono::steady_clock::time_point handshakeTimestamp_;
        std::chrono::steady_clock::time_point lastActivityTime_;
        std::chrono::milliseconds timeout_;
        BufferConfig config_;
        std::unordered_map<uint64_t, std::pair<std::unique_ptr<Packet>, std::chrono::steady_clock::time_point>> pending_;
        std::unordered_set<uint64_t> acked_;
        std::deque<std::pair<uint64_t, std::unique_ptr<Packet>>> receivedQueue_;
        std::queue<std::pair<std::unique_ptr<Packet>, std::chrono::steady_clock::time_point>> jitterBuffer_;
        std::chrono::steady_clock::time_point lastPacketTime_;
        std::deque<std::chrono::milliseconds> jitterHistory_;
        static constexpr size_t JITTER_HISTORY_SIZE = 50;
        double jitterMean_{0.0};
        double jitterVariance_{0.0};
        uint64_t packetCount_{0};
        std::unordered_set<uint64_t> recentSequences_;

        bool isValidSequence(uint64_t sequence);
        void updateJitter(std::chrono::steady_clock::time_point timestamp);
        std::chrono::milliseconds calculateAdaptiveJitterDelay() const;
    };

    class Socket {
    public:
        Socket(uint16_t port, bool ipv6 = false, LogCallback log = nullptr);
        ~Socket();
        Socket(const Socket&) = delete;
        Socket& operator=(const Socket&) = delete;
        Socket(Socket&& other) noexcept;
        Socket& operator=(Socket&& other) noexcept;

        int getHandle() const;
        bool sendRaw(const sockaddr_storage& dest, const uint8_t* data, size_t size);
        std::optional<std::pair<std::vector<uint8_t>, sockaddr_storage>> receiveRaw();

    private:
#ifdef _WIN32
        SOCKET sock_;
        static constexpr SOCKET INVALID_SOCKET_VAL = INVALID_SOCKET;
#else
        int sock_;
        static constexpr int INVALID_SOCKET_VAL = -1;
#endif
        LogCallback log_;
        mutable std::mutex mutex_;

        void closeSocket() noexcept;
    };

    class ConnectionManager {
    public:
        ConnectionManager(LogCallback log = nullptr, BufferConfig config = BufferConfig());
        ~ConnectionManager();

        int addSocket(uint16_t port, bool ipv6 = false);
        uint64_t initiateHandshake(int socketIdx, std::string_view ip, uint16_t port, 
                                 BufferConfig config = BufferConfig(), bool ipv6 = false,
                                 std::chrono::milliseconds timeout = DEFAULT_CONNECTION_TIMEOUT);
        bool send(uint64_t connId, const std::vector<uint8_t>& data, bool reliable = false);
        std::optional<std::unique_ptr<Packet>> receive(uint64_t connId, bool reliable, 
                                                    std::chrono::milliseconds minJitterDelay = std::chrono::milliseconds(10));
        void update(std::chrono::milliseconds timeout = std::chrono::milliseconds(1000));

    private:
        std::vector<Socket> sockets_;
        std::unordered_map<uint64_t, std::unique_ptr<Connection>> connections_;
        std::unordered_map<uint64_t, int> socketAssignments_;
        std::unordered_map<uint64_t, uint64_t> challenges_;
        std::mt19937_64 randomEngine_;
        std::mutex mutex_;
        LogCallback log_;
        BufferConfig config_;
        std::vector<std::pair<std::string, std::pair<std::chrono::steady_clock::time_point, size_t>>> rateLimit_;

        uint64_t generateSecureRandom();
        uint64_t makeConnectionId(const sockaddr_storage& addr, uint64_t sessionToken) const;
        std::string addrToString(const sockaddr_storage& addr) const;
        bool isRateLimited(const sockaddr_storage& addr, std::chrono::steady_clock::time_point now);
        void sendAck(int socketIdx, const sockaddr_storage& dest, uint64_t sequence, uint64_t sessionToken);
        void receivePackets(int socketIdx, std::chrono::steady_clock::time_point now);
    };

    uint64_t secureHash(const uint8_t* data, size_t size, uint64_t seed = 0);
}

#endif