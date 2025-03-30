#include <array>
#include <atomic>
#include <chrono>
#include <deque>
#include <functional>
#include <memory>
#include <mutex>
#include <optional>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <queue>
#include <random>

#include <eccrypto.h>
#include <osrng.h>
#include <aes.h>
#include <gcm.h>
#include <sha.h>
#include <filters.h>
#include <hex.h>
#include <ecp.h>
#include <oids.h>

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#else
#include <arpa/inet.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#endif

namespace VoltaNet {
    constexpr size_t MAX_PACKET_SIZE = 1200;
    constexpr size_t MAX_BUFFER_SIZE = 1500;
    constexpr size_t HEADER_SIZE = 32;
    constexpr size_t SEQUENCE_WINDOW = 10000;
    constexpr size_t MAX_RECENT_SEQUENCES = 1000;
    constexpr size_t JITTER_HISTORY_SIZE = 100;
    constexpr size_t MAX_CONNECTIONS = 1000;
    constexpr size_t MAX_PACKETS_PER_IP_PER_SEC = 100;
    constexpr std::chrono::minutes KEY_ROTATION_INTERVAL{60};

    using byte = CryptoPP::byte;

    struct BufferConfig {
        size_t maxPendingSize = 1000;
        size_t maxReceivedSize = 1000;
        size_t maxJitterSize = 100;
        size_t maxPacketsPerUpdate = 100;
    };

    using LogCallback = std::function<void(std::string)>;

    inline bool isSequenceGreater(uint64_t a, uint64_t b) {
        constexpr uint64_t HALF_MAX = std::numeric_limits<uint64_t>::max() / 2;
        return ((a > b) && (a - b <= HALF_MAX)) || ((a < b) && (b - a > HALF_MAX));
    }

    // Custom hash functor for std::array<byte, 12>
    struct NonceHash {
        std::size_t operator()(const std::array<byte, 12>& nonce) const {
            CryptoPP::SHA256 hash;
            std::array<byte, CryptoPP::SHA256::DIGESTSIZE> digest;
            hash.Update(nonce.data(), nonce.size());
            hash.Final(digest.data());
            std::size_t result = 0;
            std::memcpy(&result, digest.data(), sizeof(result));
            return result;
        }
    };

    struct Packet {
        std::array<uint8_t, MAX_PACKET_SIZE> data{};
        size_t dataSize = 0;
        bool isReliable = false;
        uint64_t sequence = 0;
        sockaddr_storage address{};
        std::array<byte, 12> nonce{};
        uint64_t timestamp = 0;

        Packet();
        void setData(const uint8_t* src, size_t size);
        Packet clone() const;
        bool operator<(const Packet& other) const;
    };

    class Connection {
    public:
        Connection(sockaddr_storage addr, BufferConfig config, std::chrono::milliseconds timeout);
        
        bool isVerified() const;
        void verify();
        sockaddr_storage getAddress() const;
        uint64_t getSessionToken() const;
        uint64_t nextSequence();
        std::chrono::steady_clock::time_point getHandshakeTimestamp() const;
        std::chrono::steady_clock::time_point getLastActivity() const;
        bool isTimedOut(std::chrono::steady_clock::time_point now) const;
        bool needsKeyRotation(std::chrono::steady_clock::time_point now) const;
        
        bool queueReliablePacket(std::unique_ptr<Packet> packet);
        void queueReceivedPacket(uint64_t sequence, std::unique_ptr<Packet> packet);
        void queueUnreliablePacket(std::unique_ptr<Packet> packet, std::chrono::steady_clock::time_point senderTimestamp);
        
        std::optional<std::unique_ptr<Packet>> getNextReliablePacket();
        std::optional<std::unique_ptr<Packet>> getNextUnreliablePacket(std::chrono::milliseconds minJitterDelay);
        
        void ackReceived(uint64_t sequence);
        std::unordered_map<uint64_t, std::pair<std::unique_ptr<Packet>, std::chrono::steady_clock::time_point>> getPending() const;
        void modifyPending(std::function<void(std::unordered_map<uint64_t, std::pair<std::unique_ptr<Packet>, std::chrono::steady_clock::time_point>>&)> callback);
        bool isAcked(uint64_t sequence) const;
        bool isValidSequence(uint64_t sequence);

        void setSessionKey(const std::vector<byte>& key, CryptoPP::AutoSeededRandomPool& rng);
        std::vector<byte> encryptPacket(const Packet& packet, CryptoPP::AutoSeededRandomPool& rng);
        std::optional<Packet> decryptPacket(const std::vector<byte>& ciphertext, const std::array<byte, 12>& nonce, LogCallback& log);

    private:
        mutable std::mutex mutex_;
        sockaddr_storage address_;
        std::atomic<uint64_t> sessionToken_;
        std::chrono::steady_clock::time_point handshakeTimestamp_;
        std::chrono::steady_clock::time_point lastActivityTime_;
        std::chrono::steady_clock::time_point keyRotationTime_;
        std::chrono::milliseconds timeout_;
        std::atomic<bool> verified_;
        std::atomic<uint64_t> nextSequence_;
        std::atomic<uint64_t> nextExpectedSequence_;
        std::atomic<uint64_t> nonceCounter_;
        BufferConfig config_;
        
        std::unordered_map<uint64_t, std::pair<std::unique_ptr<Packet>, std::chrono::steady_clock::time_point>> pending_;
        std::deque<std::pair<uint64_t, std::unique_ptr<Packet>>> receivedQueue_;
        std::queue<std::pair<std::unique_ptr<Packet>, std::chrono::steady_clock::time_point>> jitterBuffer_;
        std::unordered_set<uint64_t> acked_;
        std::unordered_set<uint64_t> recentSequences_;
        std::unordered_set<std::array<byte, 12>, NonceHash> usedNonces_; // Updated to use custom hash
        
        double jitterMean_, jitterVariance_;
        uint64_t packetCount_;
        std::deque<std::chrono::milliseconds> jitterHistory_;
        std::chrono::steady_clock::time_point lastPacketTime_;

        std::vector<byte> sessionKey_;
        CryptoPP::GCM<CryptoPP::AES>::Encryption enc_;
        CryptoPP::GCM<CryptoPP::AES>::Decryption dec_;
        CryptoPP::HMAC<CryptoPP::SHA256> hmac_;

        void updateJitter(std::chrono::steady_clock::time_point timestamp);
        std::chrono::milliseconds calculateAdaptiveJitterDelay() const;
        bool isNonceUsed(const std::array<byte, 12>& nonce) const;
    };

    class Socket {
    public:
#ifdef _WIN32
        static constexpr int INVALID_SOCKET_VAL = INVALID_SOCKET;
#else
        static constexpr int INVALID_SOCKET_VAL = -1;
#endif
        Socket(uint16_t port, bool ipv6, LogCallback log);
        ~Socket();
        Socket(Socket&& other) noexcept;
        Socket& operator=(Socket&& other) noexcept;
        
        int getHandle() const;
        bool sendRaw(const sockaddr_storage& dest, const uint8_t* data, size_t size);
        std::optional<std::pair<std::vector<uint8_t>, sockaddr_storage>> receiveRaw();

    private:
        int sock_;
        LogCallback log_;
        mutable std::mutex mutex_;
        void closeSocket() noexcept;
    };

    class ConnectionManager {
    public:
        enum PacketType : uint8_t { HANDSHAKE_INIT = 1, HANDSHAKE_RESPONSE = 2, DATA = 3, KEY_ROTATION = 4 };

        ConnectionManager(LogCallback log, BufferConfig config);
        ~ConnectionManager();

        int addSocket(uint16_t port, bool ipv6);
        uint64_t initiateHandshake(int socketIdx, std::string_view ip, uint16_t port, BufferConfig config,
                                 bool ipv6, std::chrono::milliseconds timeout);
        bool send(uint64_t connId, const std::vector<uint8_t>& data, bool reliable);
        std::optional<std::unique_ptr<Packet>> receive(uint64_t connId, bool reliable,
                                                    std::chrono::milliseconds minJitterDelay);
        void update(std::chrono::milliseconds timeout);

    private:
        LogCallback log_;
        BufferConfig config_;
        std::vector<Socket> sockets_;
        std::unordered_map<uint64_t, std::unique_ptr<Connection>> connections_;
        std::unordered_map<uint64_t, int> socketAssignments_;
        std::unordered_map<uint64_t, std::pair<CryptoPP::ECDSA<CryptoPP::ECP, CryptoPP::SHA256>::PrivateKey,
                                             CryptoPP::ECDSA<CryptoPP::ECP, CryptoPP::SHA256>::PublicKey>> handshakeKeys_;
        std::vector<std::pair<std::string, std::pair<std::chrono::steady_clock::time_point, size_t>>> rateLimit_;
        std::mt19937_64 randomEngine_;
        mutable std::mutex mutex_;
        CryptoPP::AutoSeededRandomPool rng_;

        uint64_t generateSecureRandom();
        uint64_t makeConnectionId(const sockaddr_storage& addr, uint64_t sessionToken) const;
        std::string addrToString(const sockaddr_storage& addr) const;
        bool isRateLimited(const sockaddr_storage& addr, std::chrono::steady_clock::time_point now);
        void sendAck(int socketIdx, const sockaddr_storage& dest, uint64_t sequence, uint64_t sessionToken);
        void receivePackets(int socketIdx, std::chrono::steady_clock::time_point now);
        void initiateKeyRotation(uint64_t connId);
    };

    uint64_t secureHash(const uint8_t* data, size_t size, uint64_t seed);
}

namespace VoltaNet {
    Packet::Packet() { std::memset(&address, 0, sizeof(address)); }

    void Packet::setData(const uint8_t* src, size_t size) {
        if (size > MAX_PACKET_SIZE) throw std::runtime_error("Packet data exceeds MAX_PACKET_SIZE: " + std::to_string(size));
        if (src == nullptr && size > 0) throw std::invalid_argument("Null source buffer with non-zero size");
        data.fill(0);
        if (size > 0) std::copy_n(src, size, data.begin());
        dataSize = size;
    }

    Packet Packet::clone() const {
        Packet newPacket;
        newPacket.dataSize = dataSize;
        newPacket.isReliable = isReliable;
        newPacket.sequence = sequence;
        newPacket.address = address;
        newPacket.nonce = nonce;
        newPacket.timestamp = timestamp;
        if (dataSize > 0) std::copy_n(data.begin(), std::min(dataSize, MAX_PACKET_SIZE), newPacket.data.begin());
        return newPacket;
    }

    bool Packet::operator<(const Packet& other) const {
        return isSequenceGreater(other.sequence, sequence) || (sequence == other.sequence && dataSize < other.dataSize);
    }

    Connection::Connection(sockaddr_storage addr, BufferConfig config, std::chrono::milliseconds timeout)
        : address_(addr), sessionToken_(secureHash(reinterpret_cast<const uint8_t*>(&addr), sizeof(addr), 0)),
          handshakeTimestamp_(std::chrono::steady_clock::now()), lastActivityTime_(handshakeTimestamp_),
          keyRotationTime_(handshakeTimestamp_), timeout_(timeout), verified_(false), nextSequence_(0),
          nextExpectedSequence_(0), nonceCounter_(0), config_(config), jitterMean_(0.0), jitterVariance_(0.0),
          packetCount_(0) {}

    bool Connection::isVerified() const { return verified_.load(std::memory_order_acquire); }

    void Connection::verify() {
        std::lock_guard lock(mutex_);
        verified_.store(true, std::memory_order_release);
        lastActivityTime_ = std::chrono::steady_clock::now();
    }

    sockaddr_storage Connection::getAddress() const {
        std::lock_guard lock(mutex_);
        return address_;
    }

    uint64_t Connection::getSessionToken() const { return sessionToken_.load(std::memory_order_acquire); }

    uint64_t Connection::nextSequence() { return nextSequence_.fetch_add(1, std::memory_order_seq_cst); }

    std::chrono::steady_clock::time_point Connection::getHandshakeTimestamp() const {
        std::lock_guard lock(mutex_);
        return handshakeTimestamp_;
    }

    std::chrono::steady_clock::time_point Connection::getLastActivity() const {
        std::lock_guard lock(mutex_);
        return lastActivityTime_;
    }

    bool Connection::isTimedOut(std::chrono::steady_clock::time_point now) const {
        std::lock_guard lock(mutex_);
        return std::chrono::duration_cast<std::chrono::milliseconds>(now - lastActivityTime_) > timeout_;
    }

    bool Connection::needsKeyRotation(std::chrono::steady_clock::time_point now) const {
        std::lock_guard lock(mutex_);
        return std::chrono::duration_cast<std::chrono::minutes>(now - keyRotationTime_) > KEY_ROTATION_INTERVAL;
    }

    void Connection::setSessionKey(const std::vector<byte>& key, CryptoPP::AutoSeededRandomPool& rng) {
        std::lock_guard lock(mutex_);
        sessionKey_ = key;
        enc_.SetKeyWithIV(sessionKey_.data(), sessionKey_.size(), nullptr, 0);
        dec_.SetKeyWithIV(sessionKey_.data(), sessionKey_.size(), nullptr, 0);
        hmac_.SetKey(sessionKey_.data(), sessionKey_.size());
        nonceCounter_.store(0, std::memory_order_release);
        keyRotationTime_ = std::chrono::steady_clock::now();
    }

    std::vector<byte> Connection::encryptPacket(const Packet& packet, CryptoPP::AutoSeededRandomPool& rng) {
        std::lock_guard lock(mutex_);
        if (sessionKey_.empty()) throw std::runtime_error("No session key set");

        std::array<byte, 12> nonce;
        uint64_t counter = nonceCounter_.fetch_add(1, std::memory_order_seq_cst);
        rng.GenerateBlock(nonce.data(), 4);
        std::memcpy(nonce.data() + 4, &counter, 8);

        enc_.SetKeyWithIV(sessionKey_.data(), sessionKey_.size(), nonce.data(), nonce.size());
        
        uint64_t timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch()).count();
        
        std::vector<byte> plaintext(sizeof(uint64_t) + packet.dataSize);
        std::memcpy(plaintext.data(), &timestamp, sizeof(uint64_t));
        std::copy_n(packet.data.begin(), packet.dataSize, plaintext.data() + sizeof(uint64_t));

        std::array<byte, CryptoPP::HMAC<CryptoPP::SHA256>::DIGESTSIZE> signature;
        hmac_.Update(plaintext.data(), plaintext.size());
        hmac_.Final(signature.data());

        std::string ciphertext;
        CryptoPP::ArraySource(plaintext.data(), plaintext.size(), true,
            new CryptoPP::AuthenticatedEncryptionFilter(enc_, new CryptoPP::StringSink(ciphertext)));

        std::vector<byte> result(nonce.begin(), nonce.end());
        result.insert(result.end(), signature.begin(), signature.end());
        result.insert(result.end(), ciphertext.begin(), ciphertext.end());
        return result;
    }

    std::optional<Packet> Connection::decryptPacket(const std::vector<byte>& ciphertext, 
                                                  const std::array<byte, 12>& nonce, 
                                                  LogCallback& log) {
        std::lock_guard lock(mutex_);
        if (sessionKey_.empty() || isNonceUsed(nonce)) return std::nullopt;

        dec_.SetKeyWithIV(sessionKey_.data(), sessionKey_.size(), nonce.data(), nonce.size());
        
        size_t signatureSize = CryptoPP::HMAC<CryptoPP::SHA256>::DIGESTSIZE;
        if (ciphertext.size() < signatureSize) return std::nullopt;

        std::vector<byte> signature(ciphertext.begin(), ciphertext.begin() + signatureSize);
        std::string plaintext;
        
        try {
            CryptoPP::ArraySource(ciphertext.data() + signatureSize, 
                                ciphertext.size() - signatureSize, true,
                new CryptoPP::AuthenticatedDecryptionFilter(dec_, 
                    new CryptoPP::StringSink(plaintext)));
        } catch (const CryptoPP::Exception& e) {
            if (log) log("Decryption failed: " + std::string(e.what()));
            return std::nullopt;
        }

        if (plaintext.size() < sizeof(uint64_t)) return std::nullopt;

        std::array<byte, CryptoPP::HMAC<CryptoPP::SHA256>::DIGESTSIZE> computedSignature;
        hmac_.Update(reinterpret_cast<const byte*>(plaintext.data()), plaintext.size());
        hmac_.Final(computedSignature.data());
        
        if (!std::equal(signature.begin(), signature.end(), computedSignature.begin())) {
            if (log) log("HMAC signature verification failed");
            return std::nullopt;
        }

        uint64_t timestamp;
        std::memcpy(&timestamp, plaintext.data(), sizeof(uint64_t));
        uint64_t nowMs = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch()).count();
        
        if (std::abs(static_cast<int64_t>(nowMs - timestamp)) > 5000) {
            if (log) log("Timestamp outside valid window");
            return std::nullopt;
        }

        Packet packet;
        packet.setData(reinterpret_cast<const uint8_t*>(plaintext.data() + sizeof(uint64_t)),
                      plaintext.size() - sizeof(uint64_t));
        packet.nonce = nonce;
        packet.timestamp = timestamp;
        
        usedNonces_.insert(nonce);
        if (usedNonces_.size() > MAX_RECENT_SEQUENCES) {
            usedNonces_.erase(usedNonces_.begin());
        }

        return packet;
    }

    bool Connection::queueReliablePacket(std::unique_ptr<Packet> packet) {
        if (!packet) return false;
        std::lock_guard lock(mutex_);
        if (pending_.size() >= config_.maxPendingSize) {
            if (pending_.empty()) return false;
            auto oldest = pending_.begin();
            if (oldest == pending_.end() || acked_.count(oldest->first)) return false;
            pending_.erase(oldest);
        }
        if (packet->dataSize > MAX_PACKET_SIZE) return false;
        pending_.emplace(packet->sequence, std::make_pair(std::move(packet), std::chrono::steady_clock::now()));
        lastActivityTime_ = std::chrono::steady_clock::now();
        return true;
    }

    void Connection::queueReceivedPacket(uint64_t sequence, std::unique_ptr<Packet> packet) {
        if (!packet || !isValidSequence(sequence)) return;
        std::lock_guard lock(mutex_);
        if (receivedQueue_.size() >= config_.maxReceivedSize) receivedQueue_.pop_front();
        receivedQueue_.emplace_back(sequence, std::move(packet));
        std::sort(receivedQueue_.begin(), receivedQueue_.end(),
                  [](const auto& a, const auto& b) { return isSequenceGreater(b.first, a.first); });
        lastActivityTime_ = std::chrono::steady_clock::now();
    }

    void Connection::queueUnreliablePacket(std::unique_ptr<Packet> packet, std::chrono::steady_clock::time_point senderTimestamp) {
        if (!packet) return;
        std::lock_guard lock(mutex_);
        if (jitterBuffer_.size() >= config_.maxJitterSize) jitterBuffer_.pop();
        auto timestamp = senderTimestamp != std::chrono::steady_clock::time_point{} ? senderTimestamp : std::chrono::steady_clock::now();
        updateJitter(timestamp);
        jitterBuffer_.push({std::move(packet), timestamp});
        lastActivityTime_ = std::chrono::steady_clock::now();
    }

    std::optional<std::unique_ptr<Packet>> Connection::getNextReliablePacket() {
        std::lock_guard lock(mutex_);
        std::optional<std::unique_ptr<Packet>> result;
        while (!receivedQueue_.empty()) {
            auto& [seq, packet] = receivedQueue_.front();
            uint64_t expected = nextExpectedSequence_.load(std::memory_order_acquire);
            if (seq == expected) {
                result = std::move(packet);
                receivedQueue_.pop_front();
                nextExpectedSequence_.store(expected + 1, std::memory_order_release);
                lastActivityTime_ = std::chrono::steady_clock::now();
                break;
            } else if (isSequenceGreater(expected, seq)) {
                receivedQueue_.pop_front();
            } else {
                break;
            }
        }
        return result;
    }

    std::optional<std::unique_ptr<Packet>> Connection::getNextUnreliablePacket(std::chrono::milliseconds minJitterDelay) {
        std::lock_guard lock(mutex_);
        if (jitterBuffer_.empty()) return std::nullopt;
        auto now = std::chrono::steady_clock::now();
        auto& [packet, timestamp] = jitterBuffer_.front();
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - timestamp);
        auto adaptiveDelay = calculateAdaptiveJitterDelay();
        if (elapsed >= std::max(minJitterDelay, adaptiveDelay)) {
            std::unique_ptr<Packet> result = std::move(packet);
            jitterBuffer_.pop();
            lastActivityTime_ = now;
            return result;
        }
        return std::nullopt;
    }

    void Connection::ackReceived(uint64_t sequence) {
        std::lock_guard lock(mutex_);
        acked_.insert(sequence);
        lastActivityTime_ = std::chrono::steady_clock::now();
    }

    std::unordered_map<uint64_t, std::pair<std::unique_ptr<Packet>, std::chrono::steady_clock::time_point>> Connection::getPending() const {
        std::lock_guard lock(mutex_);
        std::unordered_map<uint64_t, std::pair<std::unique_ptr<Packet>, std::chrono::steady_clock::time_point>> copy;
        for (const auto& [seq, pair] : pending_) {
            copy.emplace(seq, std::make_pair(std::make_unique<Packet>(pair.first->clone()), pair.second));
        }
        return copy;
    }

    void Connection::modifyPending(std::function<void(std::unordered_map<uint64_t, std::pair<std::unique_ptr<Packet>, std::chrono::steady_clock::time_point>>&)> callback) {
        std::lock_guard lock(mutex_);
        callback(pending_);
        lastActivityTime_ = std::chrono::steady_clock::now();
    }

    bool Connection::isAcked(uint64_t sequence) const {
        std::lock_guard lock(mutex_);
        return acked_.count(sequence) > 0;
    }

    bool Connection::isValidSequence(uint64_t sequence) {
        std::lock_guard lock(mutex_);
        uint64_t expected = nextExpectedSequence_.load(std::memory_order_acquire);
        if (isSequenceGreater(sequence, expected) && (sequence - expected) > SEQUENCE_WINDOW) return false;
        if (recentSequences_.count(sequence)) return false;
        recentSequences_.insert(sequence);
        if (recentSequences_.size() > MAX_RECENT_SEQUENCES) recentSequences_.erase(recentSequences_.begin());
        return true;
    }

    bool Connection::isNonceUsed(const std::array<byte, 12>& nonce) const {
        std::lock_guard lock(mutex_);
        return usedNonces_.count(nonce) > 0;
    }

    void Connection::updateJitter(std::chrono::steady_clock::time_point timestamp) {
        auto now = std::chrono::steady_clock::now();
        if (packetCount_ > 0) {
            auto delta = std::chrono::duration_cast<std::chrono::milliseconds>(timestamp - lastPacketTime_);
            double deltaMs = std::clamp(static_cast<double>(delta.count()), 0.0, 1000.0);
            if (jitterHistory_.size() >= JITTER_HISTORY_SIZE) jitterHistory_.pop_front();
            jitterHistory_.push_back(std::chrono::milliseconds(static_cast<int64_t>(deltaMs)));
            if (jitterHistory_.size() == 1) {
                jitterMean_ = deltaMs;
                jitterVariance_ = 0.0;
            } else {
                double oldMean = jitterMean_;
                jitterMean_ = oldMean + (deltaMs - oldMean) / jitterHistory_.size();
                jitterVariance_ = ((jitterHistory_.size() - 1) * jitterVariance_ + (deltaMs - oldMean) * (deltaMs - jitterMean_)) / std::max(1.0, static_cast<double>(jitterHistory_.size() - 1));
            }
        }
        lastPacketTime_ = timestamp;
        packetCount_ = std::min(packetCount_ + 1, static_cast<uint64_t>(JITTER_HISTORY_SIZE * 2));
    }

    std::chrono::milliseconds Connection::calculateAdaptiveJitterDelay() const {
        if (jitterHistory_.empty() || packetCount_ < 2) return std::chrono::milliseconds(50);
        double variance = jitterVariance_;
        double stdDev = std::sqrt(std::max(0.0, variance));
        int64_t delayMs = static_cast<int64_t>(jitterMean_ + 2.0 * stdDev);
        return std::chrono::milliseconds(std::clamp(delayMs, 10LL, 500LL));
    }

    Socket::Socket(uint16_t port, bool ipv6, LogCallback log) : sock_(INVALID_SOCKET_VAL), log_(std::move(log)) {
#ifdef _WIN32
        WSADATA wsaData;
        if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) throw std::runtime_error("WSAStartup failed");
        sock_ = socket(ipv6 ? AF_INET6 : AF_INET, SOCK_DGRAM, IPPROTO_UDP);
        if (sock_ == INVALID_SOCKET) {
            WSACleanup();
            throw std::runtime_error("Socket creation failed: " + std::string(strerror(WSAGetLastError())));
        }
        u_long mode = 1;
        if (ioctlsocket(sock_, FIONBIO, &mode) != 0) {
            closesocket(sock_);
            WSACleanup();
            sock_ = INVALID_SOCKET_VAL;
            throw std::runtime_error("Failed to set non-blocking mode: " + std::string(strerror(WSAGetLastError())));
        }
#else
        sock_ = socket(ipv6 ? AF_INET6 : AF_INET, SOCK_DGRAM, IPPROTO_UDP);
        if (sock_ == INVALID_SOCKET_VAL) throw std::runtime_error("Socket creation failed: " + std::string(strerror(errno)));
        if (fcntl(sock_, F_SETFL, O_NONBLOCK) < 0) {
            close(sock_);
            sock_ = INVALID_SOCKET_VAL;
            throw std::runtime_error("Failed to set non-blocking mode: " + std::string(strerror(errno)));
        }
#endif
        sockaddr_storage addr{};
        socklen_t addrLen = 0;
        if (ipv6) {
            sockaddr_in6* addr6 = reinterpret_cast<sockaddr_in6*>(&addr);
            addr6->sin6_family = AF_INET6;
            addr6->sin6_addr = in6addr_any;
            addr6->sin6_port = htons(port);
            addrLen = sizeof(sockaddr_in6);
        } else {
            sockaddr_in* addr4 = reinterpret_cast<sockaddr_in*>(&addr);
            addr4->sin_family = AF_INET;
            addr4->sin_addr.s_addr = INADDR_ANY;
            addr4->sin_port = htons(port);
            addrLen = sizeof(sockaddr_in);
        }
        if (bind(sock_, reinterpret_cast<sockaddr*>(&addr), addrLen) < 0) {
            std::string errorMsg = "Bind failed: " + std::string(strerror(errno));
#ifdef _WIN32
            closesocket(sock_);
            WSACleanup();
#else
            close(sock_);
#endif
            sock_ = INVALID_SOCKET_VAL;
            throw errorMsg;
        }
    }

    Socket::~Socket() { closeSocket(); }

    Socket::Socket(Socket&& other) noexcept : sock_(other.sock_), log_(std::move(other.log_)) {
        other.sock_ = INVALID_SOCKET_VAL;
    }

    Socket& Socket::operator=(Socket&& other) noexcept {
        if (this != &other) {
            closeSocket();
            sock_ = other.sock_;
            log_ = std::move(other.log_);
            other.sock_ = INVALID_SOCKET_VAL;
        }
        return *this;
    }

    int Socket::getHandle() const { return sock_; }

    bool Socket::sendRaw(const sockaddr_storage& dest, const uint8_t* data, size_t size) {
        std::lock_guard lock(mutex_);
        if (sock_ == INVALID_SOCKET_VAL) throw std::runtime_error("Cannot send: invalid socket");
        if (size > MAX_BUFFER_SIZE || !data || size > std::numeric_limits<int>::max()) throw std::invalid_argument("Invalid size or null data");
        socklen_t addrLen = (dest.ss_family == AF_INET) ? sizeof(sockaddr_in) : sizeof(sockaddr_in6);
        int sent = sendto(sock_, reinterpret_cast<const char*>(data), static_cast<int>(size), 0,
                          reinterpret_cast<const sockaddr*>(&dest), addrLen);
        if (sent < 0) throw std::runtime_error("sendRaw failed: " + std::string(strerror(errno)));
        if (static_cast<size_t>(sent) != size) throw std::runtime_error("Partial send: " + std::to_string(sent) + " of " + std::to_string(size));
        return true;
    }

    std::optional<std::pair<std::vector<uint8_t>, sockaddr_storage>> Socket::receiveRaw() {
        std::lock_guard lock(mutex_);
        if (sock_ == INVALID_SOCKET_VAL) throw std::runtime_error("Cannot receive: invalid socket");
        std::vector<uint8_t> buffer(MAX_BUFFER_SIZE);
        sockaddr_storage sender{};
        socklen_t addrLen = sizeof(sender);
        int received = recvfrom(sock_, reinterpret_cast<char*>(buffer.data()), static_cast<int>(MAX_BUFFER_SIZE), 0,
                                reinterpret_cast<sockaddr*>(&sender), &addrLen);
        if (received < 0 && errno != EAGAIN && errno != EWOULDBLOCK) throw std::runtime_error("receiveRaw failed: " + std::string(strerror(errno)));
        if (received < 0) return std::nullopt;
        buffer.resize(static_cast<size_t>(received));
        return std::make_pair(std::move(buffer), sender);
    }

    void Socket::closeSocket() noexcept {
        std::lock_guard lock(mutex_);
        if (sock_ != INVALID_SOCKET_VAL) {
#ifdef _WIN32
            closesocket(sock_);
            WSACleanup();
#else
            close(sock_);
#endif
            sock_ = INVALID_SOCKET_VAL;
        }
    }

    ConnectionManager::ConnectionManager(LogCallback log, BufferConfig config) : log_(std::move(log)), config_(config) {
#ifdef _WIN32
        WSADATA wsaData;
        if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) throw std::runtime_error("WSAStartup failed");
#endif
        randomEngine_.seed(std::chrono::high_resolution_clock::now().time_since_epoch().count());
    }

    ConnectionManager::~ConnectionManager() {
#ifdef _WIN32
        WSACleanup();
#endif
    }

    int ConnectionManager::addSocket(uint16_t port, bool ipv6) {
        std::lock_guard lock(mutex_);
        sockets_.emplace_back(port, ipv6, log_);
        return static_cast<int>(sockets_.size() - 1);
    }

    uint64_t ConnectionManager::initiateHandshake(int socketIdx, std::string_view ip, uint16_t port, BufferConfig config,
                                                bool ipv6, std::chrono::milliseconds timeout) {
        std::lock_guard lock(mutex_);
        if (socketIdx < 0 || socketIdx >= static_cast<int>(sockets_.size())) throw std::out_of_range("Invalid socket index");
        if (ip.empty() || ip.find('\0') != std::string_view::npos) throw std::invalid_argument("Invalid IP string");
        if (port == 0) throw std::invalid_argument("Invalid port");

        sockaddr_storage dest{};
        socklen_t addrLen = 0;
        if (ipv6) {
            sockaddr_in6* addr6 = reinterpret_cast<sockaddr_in6*>(&dest);
            addr6->sin6_family = AF_INET6;
            addr6->sin6_port = htons(port);
            if (inet_pton(AF_INET6, ip.data(), &addr6->sin6_addr) <= 0) throw std::invalid_argument("Invalid IPv6 address");
            addrLen = sizeof(sockaddr_in6);
        } else {
            sockaddr_in* addr4 = reinterpret_cast<sockaddr_in*>(&dest);
            addr4->sin_family = AF_INET;
            addr4->sin_port = htons(port);
            if (inet_pton(AF_INET, ip.data(), &addr4->sin_addr) <= 0) throw std::invalid_argument("Invalid IPv4 address");
            addrLen = sizeof(sockaddr_in);
        }

        if (connections_.size() >= MAX_CONNECTIONS) throw std::runtime_error("Connection limit reached");

        uint64_t connId = generateSecureRandom();
        auto conn = std::make_unique<Connection>(dest, config, timeout);
        uint64_t sessionToken = conn->getSessionToken();

        CryptoPP::ECDSA<CryptoPP::ECP, CryptoPP::SHA256>::PrivateKey ephemeralPrivateKey;
        ephemeralPrivateKey.Initialize(rng_, CryptoPP::ASN1::secp256r1());
        CryptoPP::ECDSA<CryptoPP::ECP, CryptoPP::SHA256>::PublicKey ephemeralPublicKey;
        ephemeralPrivateKey.MakePublicKey(ephemeralPublicKey);

        std::string pubKeySerialized;
        ephemeralPublicKey.Save(CryptoPP::StringSink(pubKeySerialized).Ref());

        std::array<uint8_t, HEADER_SIZE + 256> buffer{};
        buffer[0] = HANDSHAKE_INIT;
        std::copy_n(reinterpret_cast<const uint8_t*>(&sessionToken), sizeof(sessionToken), buffer.begin() + 1);
        std::copy_n(pubKeySerialized.data(), pubKeySerialized.size(), buffer.begin() + HEADER_SIZE);

        connections_.emplace(connId, std::move(conn));
        socketAssignments_.emplace(connId, socketIdx);
        handshakeKeys_.emplace(connId, std::make_pair(std::move(ephemeralPrivateKey), std::move(ephemeralPublicKey)));

        sockets_[socketIdx].sendRaw(dest, buffer.data(), HEADER_SIZE + pubKeySerialized.size());
        return connId;
    }

    bool ConnectionManager::send(uint64_t connId, const std::vector<uint8_t>& data, bool reliable) {
        std::lock_guard lock(mutex_);
        auto it = connections_.find(connId);
        if (it == connections_.end()) throw std::runtime_error("Connection not found");
        if (!it->second->isVerified()) throw std::runtime_error("Connection not verified");
        if (data.size() > MAX_PACKET_SIZE) throw std::invalid_argument("Data size exceeds MAX_PACKET_SIZE");
        if (data.empty()) throw std::invalid_argument("Empty data not allowed");

        int socketIdx = socketAssignments_.at(connId);
        auto& conn = *it->second;
        sockaddr_storage dest = conn.getAddress();
        uint64_t sessionToken = conn.getSessionToken();

        auto packet = std::make_unique<Packet>();
        packet->address = dest;
        packet->isReliable = reliable;
        packet->sequence = reliable ? conn.nextSequence() : 0;
        packet->setData(data.data(), data.size());

        std::vector<byte> encrypted = conn.encryptPacket(*packet, rng_);
        std::array<uint8_t, MAX_BUFFER_SIZE> buffer{};
        buffer[0] = DATA;
        std::copy_n(reinterpret_cast<const uint8_t*>(&sessionToken), sizeof(sessionToken), buffer.begin() + 1);
        std::copy_n(encrypted.data(), 12, buffer.begin() + 9); // nonce
        
        size_t offset = HEADER_SIZE;
        if (reliable) {
            std::copy_n(reinterpret_cast<const uint8_t*>(&packet->sequence), sizeof(packet->sequence), buffer.begin() + HEADER_SIZE);
            offset += sizeof(packet->sequence);
        }
        std::copy_n(encrypted.data() + 12, encrypted.size() - 12, buffer.begin() + offset);

        if (reliable) conn.queueReliablePacket(std::move(packet));
        return sockets_[socketIdx].sendRaw(dest, buffer.data(), offset + encrypted.size() - 12);
    }

    std::optional<std::unique_ptr<Packet>> ConnectionManager::receive(uint64_t connId, bool reliable,
                                                                    std::chrono::milliseconds minJitterDelay) {
        std::lock_guard lock(mutex_);
        auto it = connections_.find(connId);
        if (it == connections_.end()) throw std::runtime_error("Connection not found");
        return reliable ? it->second->getNextReliablePacket() : it->second->getNextUnreliablePacket(minJitterDelay);
    }

    void ConnectionManager::update(std::chrono::milliseconds timeout) {
        std::lock_guard lock(mutex_);
        auto now = std::chrono::steady_clock::now();
        for (int i = 0; i < static_cast<int>(sockets_.size()); ++i) receivePackets(i, now);

        std::vector<uint64_t> toErase;
        toErase.reserve(connections_.size());
        for (const auto& [connId, connPtr] : connections_) {
            if (connPtr->isTimedOut(now)) {
                toErase.push_back(connId);
            } else if (connPtr->isVerified() && connPtr->needsKeyRotation(now)) {
                initiateKeyRotation(connId);
            }
        }
        for (auto connId : toErase) {
            socketAssignments_.erase(connId);
            connections_.erase(connId);
            handshakeKeys_.erase(connId);
        }

        for (auto& [connId, connPtr] : connections_) {
            auto& conn = *connPtr;
            int socketIdx = socketAssignments_.at(connId);
            auto pending = conn.getPending();
            for (auto& [seq, pair] : pending) {
                if (conn.isAcked(seq)) continue;
                auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - pair.second);
                if (elapsed > timeout) {
                    std::vector<byte> encrypted = conn.encryptPacket(*pair.first, rng_);
                    std::array<uint8_t, MAX_BUFFER_SIZE> buffer{};
                    uint64_t sessionToken = conn.getSessionToken();
                    buffer[0] = DATA;
                    std::copy_n(reinterpret_cast<const uint8_t*>(&sessionToken), sizeof(sessionToken), buffer.begin() + 1);
                    std::copy_n(encrypted.data(), 12, buffer.begin() + 9); // nonce
                    std::copy_n(reinterpret_cast<const uint8_t*>(&seq), sizeof(seq), buffer.begin() + HEADER_SIZE);
                    std::copy_n(encrypted.data() + 12, encrypted.size() - 12, buffer.begin() + HEADER_SIZE + sizeof(seq));
                    sockets_[socketIdx].sendRaw(pair.first->address, buffer.data(), HEADER_SIZE + sizeof(seq) + encrypted.size() - 12);
                    conn.modifyPending([&](auto& pendingMap) { pendingMap[seq].second = now; });
                }
            }
        }

        rateLimit_.erase(std::remove_if(rateLimit_.begin(), rateLimit_.end(),
            [now](const auto& entry) {
                return std::chrono::duration_cast<std::chrono::seconds>(now - entry.second.first) > std::chrono::seconds(1);
            }), rateLimit_.end());
    }

    uint64_t ConnectionManager::generateSecureRandom() {
        uint64_t result;
        rng_.GenerateBlock(reinterpret_cast<byte*>(&result), sizeof(result));
        return result;
    }

    uint64_t ConnectionManager::makeConnectionId(const sockaddr_storage& addr, uint64_t sessionToken) const {
        std::string addrStr = addrToString(addr);
        return secureHash(reinterpret_cast<const uint8_t*>(addrStr.data()), addrStr.size(), sessionToken);
    }

    std::string ConnectionManager::addrToString(const sockaddr_storage& addr) const {
        char buffer[INET6_ADDRSTRLEN];
        if (addr.ss_family == AF_INET) {
            const sockaddr_in* addr4 = reinterpret_cast<const sockaddr_in*>(&addr);
            inet_ntop(AF_INET, &addr4->sin_addr, buffer, sizeof(buffer));
            return std::string(buffer) + ":" + std::to_string(ntohs(addr4->sin_port));
        } else {
            const sockaddr_in6* addr6 = reinterpret_cast<const sockaddr_in6*>(&addr);
            inet_ntop(AF_INET6, &addr6->sin6_addr, buffer, sizeof(buffer));
            return std::string(buffer) + ":" + std::to_string(ntohs(addr6->sin6_port));
        }
    }

    bool ConnectionManager::isRateLimited(const sockaddr_storage& addr, std::chrono::steady_clock::time_point now) {
        std::string addrStr = addrToString(addr);
        auto it = std::find_if(rateLimit_.begin(), rateLimit_.end(),
            [&addrStr](const auto& entry) { return entry.first == addrStr; });
        if (it == rateLimit_.end()) {
            rateLimit_.emplace_back(addrStr, std::make_pair(now, 1));
            return false;
        }
        auto& [timestamp, count] = it->second;
        if (std::chrono::duration_cast<std::chrono::seconds>(now - timestamp) > std::chrono::seconds(1)) {
            it->second = std::make_pair(now, 1);
            return false;
        }
        if (count >= MAX_PACKETS_PER_IP_PER_SEC) return true;
        it->second.second++;
        return false;
    }

    void ConnectionManager::sendAck(int socketIdx, const sockaddr_storage& dest, uint64_t sequence, uint64_t sessionToken) {
        std::array<uint8_t, HEADER_SIZE + sizeof(sequence)> buffer{};
        buffer[0] = DATA;
        std::copy_n(reinterpret_cast<const uint8_t*>(&sessionToken), sizeof(sessionToken), buffer.begin() + 1);
        std::copy_n(reinterpret_cast<const uint8_t*>(&sequence), sizeof(sequence), buffer.begin() + HEADER_SIZE);
        sockets_[socketIdx].sendRaw(dest, buffer.data(), HEADER_SIZE + sizeof(sequence));
    }

    void ConnectionManager::initiateKeyRotation(uint64_t connId) {
        auto it = connections_.find(connId);
        if (it == connections_.end() || !it->second->isVerified()) return;

        int socketIdx = socketAssignments_.at(connId);
        auto& conn = *it->second;
        sockaddr_storage dest = conn.getAddress();
        uint64_t sessionToken = conn.getSessionToken();

        CryptoPP::ECDSA<CryptoPP::ECP, CryptoPP::SHA256>::PrivateKey ephemeralPrivateKey;
        ephemeralPrivateKey.Initialize(rng_, CryptoPP::ASN1::secp256r1());
        CryptoPP::ECDSA<CryptoPP::ECP, CryptoPP::SHA256>::PublicKey ephemeralPublicKey;
        ephemeralPrivateKey.MakePublicKey(ephemeralPublicKey);

        std::string pubKeySerialized;
        ephemeralPublicKey.Save(CryptoPP::StringSink(pubKeySerialized).Ref());

        std::array<uint8_t, HEADER_SIZE + 256> buffer{};
        buffer[0] = KEY_ROTATION;
        std::copy_n(reinterpret_cast<const uint8_t*>(&sessionToken), sizeof(sessionToken), buffer.begin() + 1);
        std::copy_n(pubKeySerialized.data(), pubKeySerialized.size(), buffer.begin() + HEADER_SIZE);

        handshakeKeys_[connId] = std::make_pair(std::move(ephemeralPrivateKey), std::move(ephemeralPublicKey));
        sockets_[socketIdx].sendRaw(dest, buffer.data(), HEADER_SIZE + pubKeySerialized.size());
    }

    void ConnectionManager::receivePackets(int socketIdx, std::chrono::steady_clock::time_point now) {
        auto& socket = sockets_[socketIdx];
        size_t maxPackets = std::min(config_.maxPacketsPerUpdate, static_cast<size_t>(1000));
        while (maxPackets--) {
            auto receivedOpt = socket.receiveRaw();
            if (!receivedOpt) break;
            auto& [buffer, sender] = *receivedOpt;

            if (isRateLimited(sender, now)) {
                if (log_) log_("Rate limit exceeded for " + addrToString(sender));
                continue;
            }

            if (buffer.size() < HEADER_SIZE) {
                if (log_) log_("Packet too small for header");
                continue;
            }

            uint8_t packetType = buffer[0];
            uint64_t receivedToken;
            std::copy_n(buffer.data() + 1, sizeof(receivedToken), reinterpret_cast<uint8_t*>(&receivedToken));
            uint64_t connId = makeConnectionId(sender, receivedToken);

            if (packetType == HANDSHAKE_INIT || packetType == KEY_ROTATION) {
                if (buffer.size() < HEADER_SIZE + 32) {
                    if (log_) log_("Invalid handshake/key rotation packet size");
                    continue;
                }
                std::string peerPubKeyStr(buffer.begin() + HEADER_SIZE, buffer.end());
                CryptoPP::ECDSA<CryptoPP::ECP, CryptoPP::SHA256>::PublicKey peerPubKey;
                peerPubKey.Load(CryptoPP::StringSource(peerPubKeyStr, true).Ref());

                CryptoPP::ECDH<CryptoPP::ECP>::Domain dh(CryptoPP::ASN1::secp256r1());
                CryptoPP::SecByteBlock sharedSecret(dh.AgreedValueLength());

                CryptoPP::ECDSA<CryptoPP::ECP, CryptoPP::SHA256>::PrivateKey ephemeralPrivateKey;
                ephemeralPrivateKey.Initialize(rng_, CryptoPP::ASN1::secp256r1());
                CryptoPP::ECDSA<CryptoPP::ECP, CryptoPP::SHA256>::PublicKey ephemeralPublicKey;
                ephemeralPrivateKey.MakePublicKey(ephemeralPublicKey);

                CryptoPP::SecByteBlock privKey(dh.PrivateKeyLength());
                ephemeralPrivateKey.Save(CryptoPP::ArraySink(privKey.data(), privKey.size()).Ref());

                CryptoPP::SecByteBlock pubKey(dh.PublicKeyLength());
                CryptoPP::ArraySink pubSink(pubKey.data(), pubKey.size());
                peerPubKey.Save(pubSink);

                if (!dh.Agree(sharedSecret, privKey, pubKey)) {
                    if (log_) log_("ECDH agreement failed during handshake/key rotation init");
                    continue;
                }

                if (packetType == HANDSHAKE_INIT) {
                    auto conn = std::make_unique<Connection>(sender, config_, std::chrono::milliseconds(5000));
                    conn->setSessionKey(std::vector<byte>(sharedSecret.begin(), sharedSecret.end()), rng_);
                    connId = makeConnectionId(sender, conn->getSessionToken());
                    connections_.emplace(connId, std::move(conn));
                    socketAssignments_.emplace(connId, socketIdx);
                } else {
                    auto connIt = connections_.find(connId);
                    if (connIt != connections_.end() && connIt->second->isVerified()) {
                        connIt->second->setSessionKey(std::vector<byte>(sharedSecret.begin(), sharedSecret.end()), rng_);
                    }
                }

                std::string myPubKeySerialized;
                ephemeralPublicKey.Save(CryptoPP::StringSink(myPubKeySerialized).Ref());

                std::array<uint8_t, HEADER_SIZE + 256> respBuffer{};
                respBuffer[0] = (packetType == HANDSHAKE_INIT) ? HANDSHAKE_RESPONSE : KEY_ROTATION;
                std::copy_n(reinterpret_cast<const uint8_t*>(&receivedToken), sizeof(receivedToken), respBuffer.begin() + 1);
                std::copy_n(myPubKeySerialized.data(), myPubKeySerialized.size(), respBuffer.begin() + HEADER_SIZE);
                socket.sendRaw(sender, respBuffer.data(), HEADER_SIZE + myPubKeySerialized.size());

                if (packetType == HANDSHAKE_INIT) {
                    handshakeKeys_.emplace(connId, std::make_pair(std::move(ephemeralPrivateKey), std::move(ephemeralPublicKey)));
                }
                continue;
            } else if (packetType == HANDSHAKE_RESPONSE || packetType == KEY_ROTATION) {
                if (buffer.size() < HEADER_SIZE + 32) {
                    if (log_) log_("Invalid handshake/key rotation response size");
                    continue;
                }
                auto connIt = connections_.find(connId);
                if (connIt == connections_.end()) continue;

                std::string peerPubKeyStr(buffer.begin() + HEADER_SIZE, buffer.end());
                CryptoPP::ECDSA<CryptoPP::ECP, CryptoPP::SHA256>::PublicKey peerPubKey;
                peerPubKey.Load(CryptoPP::StringSource(peerPubKeyStr, true).Ref());

                auto keyIt = handshakeKeys_.find(connId);
                if (keyIt == handshakeKeys_.end()) continue;

                CryptoPP::ECDH<CryptoPP::ECP>::Domain dh(CryptoPP::ASN1::secp256r1());
                CryptoPP::SecByteBlock sharedSecret(dh.AgreedValueLength());
                CryptoPP::SecByteBlock privKey(dh.PrivateKeyLength());
                keyIt->second.first.Save(CryptoPP::ArraySink(privKey.data(), privKey.size()).Ref());

                CryptoPP::SecByteBlock pubKey(dh.PublicKeyLength());
                CryptoPP::ArraySink pubSink(pubKey.data(), pubKey.size());
                peerPubKey.Save(pubSink);

                if (!dh.Agree(sharedSecret, privKey, pubKey)) {
                    if (log_) log_("ECDH agreement failed during handshake/key rotation response");
                    continue;
                }

                connIt->second->setSessionKey(std::vector<byte>(sharedSecret.begin(), sharedSecret.end()), rng_);
                if (packetType == HANDSHAKE_RESPONSE) {
                    connIt->second->verify();
                    if (log_) log_("Handshake completed for connId: " + std::to_string(connId));
                } else {
                    if (log_) log_("Key rotation completed for connId: " + std::to_string(connId));
                }
                handshakeKeys_.erase(connId);
                continue;
            }

            auto connIt = connections_.find(connId);
            if (connIt == connections_.end()) {
                if (log_) log_("No connection found for connId: " + std::to_string(connId));
                continue;
            }

            auto& conn = *connIt->second;
            std::array<byte, 12> nonce;
            std::copy_n(buffer.data() + 9, 12, nonce.begin());
            
            size_t offset = (buffer.size() > HEADER_SIZE + sizeof(uint64_t)) ? HEADER_SIZE + sizeof(uint64_t) : HEADER_SIZE;
            std::vector<byte> ciphertext(buffer.begin() + offset, buffer.end());
            auto decryptedOpt = conn.decryptPacket(ciphertext, nonce, log_);
            
            if (!decryptedOpt) {
                if (log_) log_("Decryption failed for connId: " + std::to_string(connId));
                continue;
            }

            auto packet = std::make_unique<Packet>(std::move(*decryptedOpt));
            packet->address = sender;

            if (buffer.size() > HEADER_SIZE + sizeof(uint64_t)) {
                uint64_t sequence;
                std::copy_n(buffer.data() + HEADER_SIZE, sizeof(sequence), reinterpret_cast<uint8_t*>(&sequence));
                if (buffer.size() == HEADER_SIZE + sizeof(sequence)) {
                    conn.ackReceived(sequence);
                    continue;
                }
                packet->isReliable = true;
                packet->sequence = sequence;
                if (!conn.isAcked(sequence)) {
                    conn.queueReceivedPacket(sequence, std::move(packet));
                    sendAck(socketIdx, sender, sequence, receivedToken);
                }
            } else {
                conn.queueUnreliablePacket(std::move(packet), now);
            }
        }
    }

    uint64_t secureHash(const uint8_t* data, size_t size, uint64_t seed) {
        CryptoPP::SHA256 hash;
        std::array<byte, CryptoPP::SHA256::DIGESTSIZE> digest;
        hash.Update(reinterpret_cast<const byte*>(&seed), sizeof(seed));
        hash.Update(data, size);
        hash.Final(digest.data());
        uint64_t result = 0;
        std::memcpy(&result, digest.data(), sizeof(result));
        return result;
    }
}