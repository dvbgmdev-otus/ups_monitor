/**
 * @file snmp_client.cpp
 * @ingroup snmp
 * @brief Реализация UDP SNMP-клиента для GET-запросов.
 */
#include "snmp_client.h"

#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <unistd.h>

#include <cstring>

#include "snmp_codec.h"

namespace snmp {

using namespace codec;

SnmpClient::SnmpClient(const std::string& host,
                       uint16_t port,
                       const std::string& community,
                       SnmpVersion version)
    : m_community(community), m_version(version) {
    std::memset(&m_addr, 0, sizeof(m_addr));
    m_addr.sin_family = AF_INET;
    m_addr.sin_port = htons(port);
    if (inet_pton(AF_INET, host.c_str(), &m_addr.sin_addr) != 1) {
        // invalid IPv4 address — можно залогировать или оставить как есть
    }
}

SnmpClient::~SnmpClient() noexcept { closeSocket(); }

bool SnmpClient::openSocket(ErrorMessage& err) {
    // Идемпотентность: сокет уже открыт
    if (m_sock >= 0) return true;

    // Создание UDP сокета
    m_sock = ::socket(AF_INET, SOCK_DGRAM, 0);
    if (m_sock < 0) {
        err = "Failed to create UDP socket";
        return false;
    }

    // Таймаут приёма
    struct timeval tv;  // раньше инициализировал через {},
                        // но Астра на такое ругается, сделал через memset
    std::memset(&tv, 0, sizeof(tv));
    tv.tv_sec = 2;
    tv.tv_usec = 0;

    if (::setsockopt(m_sock, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) < 0) {
        err = "Failed to set socket receive timeout";
        ::close(m_sock);
        m_sock = -1;
        return false;
    }

    return true;
}

void SnmpClient::closeSocket() {
    if (m_sock >= 0) {
        ::close(m_sock);
        m_sock = -1;
    }
}

bool SnmpClient::get(const Oid& oid, SnmpValue& out, ErrorMessage* err) {
    std::vector<SnmpValue> values;
    if (!get(std::vector<Oid>{ oid }, values, err)) return false;

    if (values.empty()) {
        if (err) *err = "SNMP response contains no values";
        return false;
    }

    out = values.front();
    return true;
}

bool SnmpClient::get(const std::vector<Oid>& oids,
                     std::vector<SnmpValue>& out,
                     ErrorMessage* err) {
    ErrorMessage localErr;

    if (!openSocket(localErr)) {
        if (err) *err = localErr;
        return false;
    }

    int requestId = m_requestId;

    SnmpGetRequest getRequest;
    getRequest.requestId = requestId;
    getRequest.version = m_version;
    getRequest.community = m_community;
    getRequest.oids = oids;

    std::vector<uint8_t> request;
    SnmpCodec::encodeGetRequest(getRequest, request);

    ssize_t sent = sendto(m_sock,
                          request.data(),
                          request.size(),
                          0,
                          reinterpret_cast<sockaddr*>(&m_addr),
                          sizeof(m_addr));

    if (sent < 0) {
        if (err) *err = "Failed to send SNMP request";
        return false;
    }

    ++m_requestId;

    uint8_t buffer[4096];
    ssize_t received = recvfrom(m_sock, buffer, sizeof(buffer), 0, nullptr, nullptr);

    if (received <= 0) {
        if (err) *err = "SNMP response timeout";
        return false;
    }

    if (!SnmpCodec::decodeGetResponse(buffer, received, requestId, m_version, out, localErr)) {
        if (err) *err = localErr;
        return false;
    }

    return true;
}

}  // namespace snmp
