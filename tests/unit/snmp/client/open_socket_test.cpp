/**
 * @file open_socket_test.cpp
 * @brief Тесты ошибок открытия UDP-сокета в SnmpClient.
 */
#include <gtest/gtest.h>

#include <cerrno>
#include <cstring>
#include <memory>
#include <sys/socket.h>
#include <unistd.h>

#include "snmp_client.h"

extern "C" {

int __real_socket(int domain, int type, int protocol);
int __real_setsockopt(int sockfd, int level, int optname, const void* optval, socklen_t optlen);
int __real_close(int fd);

}  // extern "C"

namespace {

const int FAKE_SOCKET_FD = 42;

struct SocketWrapState {
    bool failSocket{ false };
    bool failReceiveTimeout{ false };
    bool failSend{ false };
    bool returnInvalidResponse{ false };
    bool returnInvalidVersionTag{ false };
    bool returnUnsupportedVersion{ false };
    bool returnInvalidCommunityTag{ false };
    bool returnInvalidPduTag{ false };
    bool returnInvalidRequestIdTag{ false };
    bool returnMismatchedRequestId{ false };
    bool returnInvalidErrorStatusTag{ false };
    bool returnInvalidErrorIndexTag{ false };
    bool returnSnmpErrorStatus{ false };
    bool returnInvalidVarBindListTag{ false };
    bool returnInvalidVarBindTag{ false };
    bool returnInvalidOidTag{ false };
    bool returnVarBindWithoutValue{ false };
    bool returnInvalidIntegerValue{ false };
    bool returnInvalidStringValue{ false };
    bool returnInvalidNullValue{ false };
    bool returnUnsupportedValueType{ false };
    bool returnEmptyResponse{ false };
    bool returnValidResponse{ false };
    bool returnTimeout{ false };
    int responseVersion{ 1 };
    int sentVersion{ -1 };
    int closeCalls{ 0 };
};

SocketWrapState& wrapState() {
    static SocketWrapState state;
    return state;
}

void resetWrapState() { wrapState() = SocketWrapState{}; }

bool usesFakeSocket() {
    return wrapState().failReceiveTimeout || wrapState().failSend ||
           wrapState().returnInvalidResponse || wrapState().returnInvalidVersionTag ||
           wrapState().returnUnsupportedVersion || wrapState().returnInvalidCommunityTag ||
           wrapState().returnInvalidPduTag || wrapState().returnInvalidRequestIdTag ||
           wrapState().returnMismatchedRequestId || wrapState().returnInvalidErrorStatusTag ||
           wrapState().returnInvalidErrorIndexTag || wrapState().returnSnmpErrorStatus ||
           wrapState().returnInvalidVarBindListTag || wrapState().returnInvalidVarBindTag ||
           wrapState().returnInvalidOidTag || wrapState().returnVarBindWithoutValue ||
           wrapState().returnInvalidIntegerValue || wrapState().returnInvalidStringValue ||
           wrapState().returnInvalidNullValue || wrapState().returnUnsupportedValueType ||
           wrapState().returnEmptyResponse || wrapState().returnValidResponse ||
           wrapState().returnTimeout;
}

ssize_t copyResponse(void* buf, size_t len, const uint8_t* response, size_t responseSize) {
    if (len < responseSize) {
        errno = ENOBUFS;
        return -1;
    }

    std::memcpy(buf, response, responseSize);
    return static_cast<ssize_t>(responseSize);
}

}  // namespace

extern "C" {

int __wrap_socket(int domain, int type, int protocol) {
    if (wrapState().failSocket) {
        errno = EMFILE;
        return -1;
    }
    if (wrapState().failReceiveTimeout) {
        return FAKE_SOCKET_FD;
    }
    if (usesFakeSocket()) {
        return FAKE_SOCKET_FD;
    }

    return __real_socket(domain, type, protocol);
}

int __wrap_setsockopt(int sockfd, int level, int optname, const void* optval, socklen_t optlen) {
    if (wrapState().failReceiveTimeout && level == SOL_SOCKET && optname == SO_RCVTIMEO) {
        errno = ENOPROTOOPT;
        return -1;
    }
    if (wrapState().returnEmptyResponse && sockfd == FAKE_SOCKET_FD) {
        return 0;
    }
    if (usesFakeSocket() && sockfd == FAKE_SOCKET_FD) {
        return 0;
    }

    return __real_setsockopt(sockfd, level, optname, optval, optlen);
}

int __wrap_close(int fd) {
    ++wrapState().closeCalls;
    if (usesFakeSocket() && fd == FAKE_SOCKET_FD) {
        return 0;
    }

    return __real_close(fd);
}

ssize_t __wrap_sendto(int sockfd,
                      const void* buf,
                      size_t len,
                      int flags,
                      const struct sockaddr* dest_addr,
                      socklen_t addrlen) {
    (void)flags;
    (void)dest_addr;
    (void)addrlen;

    if (sockfd == FAKE_SOCKET_FD && len >= 5) {
        const uint8_t* request = static_cast<const uint8_t*>(buf);
        if (request[2] == 0x02 && request[3] == 0x01) {
            wrapState().sentVersion = request[4];
        }
    }

    if (wrapState().failSend && sockfd == FAKE_SOCKET_FD) {
        errno = EIO;
        return -1;
    }
    if (wrapState().returnInvalidResponse && sockfd == FAKE_SOCKET_FD) {
        return static_cast<ssize_t>(len);
    }
    if (wrapState().returnInvalidVersionTag && sockfd == FAKE_SOCKET_FD) {
        return static_cast<ssize_t>(len);
    }
    if (wrapState().returnUnsupportedVersion && sockfd == FAKE_SOCKET_FD) {
        return static_cast<ssize_t>(len);
    }
    if (wrapState().returnInvalidCommunityTag && sockfd == FAKE_SOCKET_FD) {
        return static_cast<ssize_t>(len);
    }
    if (wrapState().returnInvalidPduTag && sockfd == FAKE_SOCKET_FD) {
        return static_cast<ssize_t>(len);
    }
    if (wrapState().returnInvalidRequestIdTag && sockfd == FAKE_SOCKET_FD) {
        return static_cast<ssize_t>(len);
    }
    if (wrapState().returnMismatchedRequestId && sockfd == FAKE_SOCKET_FD) {
        return static_cast<ssize_t>(len);
    }
    if (wrapState().returnInvalidErrorStatusTag && sockfd == FAKE_SOCKET_FD) {
        return static_cast<ssize_t>(len);
    }
    if (wrapState().returnInvalidErrorIndexTag && sockfd == FAKE_SOCKET_FD) {
        return static_cast<ssize_t>(len);
    }
    if (wrapState().returnSnmpErrorStatus && sockfd == FAKE_SOCKET_FD) {
        return static_cast<ssize_t>(len);
    }
    if (wrapState().returnInvalidVarBindListTag && sockfd == FAKE_SOCKET_FD) {
        return static_cast<ssize_t>(len);
    }
    if (wrapState().returnInvalidVarBindTag && sockfd == FAKE_SOCKET_FD) {
        return static_cast<ssize_t>(len);
    }
    if (wrapState().returnInvalidOidTag && sockfd == FAKE_SOCKET_FD) {
        return static_cast<ssize_t>(len);
    }
    if (wrapState().returnVarBindWithoutValue && sockfd == FAKE_SOCKET_FD) {
        return static_cast<ssize_t>(len);
    }
    if (wrapState().returnInvalidIntegerValue && sockfd == FAKE_SOCKET_FD) {
        return static_cast<ssize_t>(len);
    }
    if (wrapState().returnInvalidStringValue && sockfd == FAKE_SOCKET_FD) {
        return static_cast<ssize_t>(len);
    }
    if (wrapState().returnInvalidNullValue && sockfd == FAKE_SOCKET_FD) {
        return static_cast<ssize_t>(len);
    }
    if (wrapState().returnUnsupportedValueType && sockfd == FAKE_SOCKET_FD) {
        return static_cast<ssize_t>(len);
    }
    if (wrapState().returnEmptyResponse && sockfd == FAKE_SOCKET_FD) {
        return static_cast<ssize_t>(len);
    }
    if ((wrapState().returnValidResponse || wrapState().returnTimeout) &&
        sockfd == FAKE_SOCKET_FD) {
        return static_cast<ssize_t>(len);
    }

    errno = EBADF;
    return -1;
}

ssize_t __wrap_recvfrom(int sockfd,
                        void* buf,
                        size_t len,
                        int flags,
                        struct sockaddr* src_addr,
                        socklen_t* addrlen) {
    (void)flags;
    (void)src_addr;
    (void)addrlen;

    if (wrapState().returnTimeout && sockfd == FAKE_SOCKET_FD) {
        errno = EAGAIN;
        return -1;
    }

    if (wrapState().returnValidResponse && sockfd == FAKE_SOCKET_FD) {
        // clang-format off
        uint8_t response[] = {
            0x30, 0x21,                  // Message SEQUENCE, len = 33
                0x02, 0x01, 0x01,        // version = v2c
                0x04, 0x06, 'p','u','b','l','i','c',
                0xA2, 0x14,              // GetResponse-PDU, len = 20
                    0x02, 0x01, 0x01,    // request-id = 1
                    0x02, 0x01, 0x00,    // error-status = 0
                    0x02, 0x01, 0x00,    // error-index = 0
                    0x30, 0x09,          // VarBindList, len = 9
                        0x30, 0x07,      // VarBind, len = 7
                            0x06, 0x02, 0x2B, 0x06, // OID = 1.3.6
                            0x02, 0x01, 0x05        // INTEGER = 5
        };
        // clang-format on

        response[4] = static_cast<uint8_t>(wrapState().responseVersion);
        return copyResponse(buf, len, response, sizeof(response));
    }

    if (wrapState().returnInvalidResponse && sockfd == FAKE_SOCKET_FD) {
        const uint8_t response[] = {
            0x31, 0x00  // WRONG top-level tag
        };

        return copyResponse(buf, len, response, sizeof(response));
    }

    if (wrapState().returnInvalidVersionTag && sockfd == FAKE_SOCKET_FD) {
        const uint8_t response[] = {
            0x30,
            0x02,  // Message SEQUENCE, len = 2
            0x05,
            0x00  // WRONG version tag: NULL instead of INTEGER
        };

        return copyResponse(buf, len, response, sizeof(response));
    }

    if (wrapState().returnUnsupportedVersion && sockfd == FAKE_SOCKET_FD) {
        const uint8_t response[] = {
            0x30,
            0x03,  // Message SEQUENCE, len = 3
            0x02,
            0x01,
            0x02  // version = 2 (unsupported)
        };

        return copyResponse(buf, len, response, sizeof(response));
    }

    if (wrapState().returnInvalidCommunityTag && sockfd == FAKE_SOCKET_FD) {
        const uint8_t response[] = {
            0x30,
            0x05,  // Message SEQUENCE, len = 5
            0x02,
            0x01,
            0x01,  // version = v2c
            0x05,
            0x00  // WRONG community tag: NULL instead of OCTET STRING
        };

        return copyResponse(buf, len, response, sizeof(response));
    }

    if (wrapState().returnInvalidPduTag && sockfd == FAKE_SOCKET_FD) {
        // clang-format off
        const uint8_t response[] = {
            0x30, 0x0D,              // Message SEQUENCE, len = 13
                0x02, 0x01, 0x01,    // version = v2c
                0x04, 0x06, 'p','u','b','l','i','c',
                0xA0, 0x00           // WRONG PDU tag: GetRequest instead of GetResponse
        };
        // clang-format on

        return copyResponse(buf, len, response, sizeof(response));
    }

    if (wrapState().returnInvalidRequestIdTag && sockfd == FAKE_SOCKET_FD) {
        // clang-format off
        const uint8_t response[] = {
            0x30, 0x15,              // Message SEQUENCE, len = 21
                0x02, 0x01, 0x01,    // version = v2c
                0x04, 0x06, 'p','u','b','l','i','c',
                0xA2, 0x08,          // GetResponse-PDU, len = 8
                    0x05, 0x00,      // WRONG request-id tag: NULL instead of INTEGER
                    0x02, 0x01, 0x00,
                    0x02, 0x01, 0x00
        };
        // clang-format on

        return copyResponse(buf, len, response, sizeof(response));
    }

    if (wrapState().returnMismatchedRequestId && sockfd == FAKE_SOCKET_FD) {
        // clang-format off
        const uint8_t response[] = {
            0x30, 0x18,              // Message SEQUENCE, len = 24
                0x02, 0x01, 0x01,    // version = v2c
                0x04, 0x06, 'p','u','b','l','i','c',
                0xA2, 0x0B,          // GetResponse-PDU, len = 11
                    0x02, 0x01, 0x02,// request-id = 2 (expected 1)
                    0x02, 0x01, 0x00,// error-status = 0
                    0x02, 0x01, 0x00,// error-index = 0
                    0x30, 0x00       // empty VarBindList
        };
        // clang-format on

        return copyResponse(buf, len, response, sizeof(response));
    }

    if (wrapState().returnInvalidErrorStatusTag && sockfd == FAKE_SOCKET_FD) {
        // clang-format off
        const uint8_t response[] = {
            0x30, 0x15,              // Message SEQUENCE, len = 21
                0x02, 0x01, 0x01,    // version = v2c
                0x04, 0x06, 'p','u','b','l','i','c',
                0xA2, 0x08,          // GetResponse-PDU, len = 8
                    0x02, 0x01, 0x01,// request-id = 1
                    0x05, 0x00,      // WRONG error-status tag: NULL instead of INTEGER
                    0x02, 0x01, 0x00 // error-index = 0
        };
        // clang-format on

        return copyResponse(buf, len, response, sizeof(response));
    }

    if (wrapState().returnInvalidErrorIndexTag && sockfd == FAKE_SOCKET_FD) {
        // clang-format off
        const uint8_t response[] = {
            0x30, 0x15,              // Message SEQUENCE, len = 21
                0x02, 0x01, 0x01,    // version = v2c
                0x04, 0x06, 'p','u','b','l','i','c',
                0xA2, 0x08,          // GetResponse-PDU, len = 8
                    0x02, 0x01, 0x01,// request-id = 1
                    0x02, 0x01, 0x00,// error-status = 0
                    0x05, 0x00       // WRONG error-index tag: NULL instead of INTEGER
        };
        // clang-format on

        return copyResponse(buf, len, response, sizeof(response));
    }

    if (wrapState().returnSnmpErrorStatus && sockfd == FAKE_SOCKET_FD) {
        // clang-format off
        const uint8_t response[] = {
            0x30, 0x18,              // Message SEQUENCE, len = 24
                0x02, 0x01, 0x01,    // version = v2c
                0x04, 0x06, 'p','u','b','l','i','c',
                0xA2, 0x0B,          // GetResponse-PDU, len = 11
                    0x02, 0x01, 0x01,// request-id = 1
                    0x02, 0x01, 0x05,// error-status = 5
                    0x02, 0x01, 0x02,// error-index = 2
                    0x30, 0x00       // empty VarBindList
        };
        // clang-format on

        return copyResponse(buf, len, response, sizeof(response));
    }

    if (wrapState().returnInvalidVarBindListTag && sockfd == FAKE_SOCKET_FD) {
        // clang-format off
        const uint8_t response[] = {
            0x30, 0x18,              // Message SEQUENCE, len = 24
                0x02, 0x01, 0x01,    // version = v2c
                0x04, 0x06, 'p','u','b','l','i','c',
                0xA2, 0x0B,          // GetResponse-PDU, len = 11
                    0x02, 0x01, 0x01,// request-id = 1
                    0x02, 0x01, 0x00,// error-status = 0
                    0x02, 0x01, 0x00,// error-index = 0
                    0x05, 0x00       // WRONG VarBindList tag: NULL instead of SEQUENCE
        };
        // clang-format on

        return copyResponse(buf, len, response, sizeof(response));
    }

    if (wrapState().returnInvalidVarBindTag && sockfd == FAKE_SOCKET_FD) {
        // clang-format off
        const uint8_t response[] = {
            0x30, 0x1A,              // Message SEQUENCE, len = 26
                0x02, 0x01, 0x01,    // version = v2c
                0x04, 0x06, 'p','u','b','l','i','c',
                0xA2, 0x0D,          // GetResponse-PDU, len = 13
                    0x02, 0x01, 0x01,// request-id = 1
                    0x02, 0x01, 0x00,// error-status = 0
                    0x02, 0x01, 0x00,// error-index = 0
                    0x30, 0x02,      // VarBindList, len = 2
                        0x05, 0x00   // WRONG VarBind tag: NULL instead of SEQUENCE
        };
        // clang-format on

        return copyResponse(buf, len, response, sizeof(response));
    }

    if (wrapState().returnInvalidOidTag && sockfd == FAKE_SOCKET_FD) {
        // clang-format off
        const uint8_t response[] = {
            0x30, 0x1C,              // Message SEQUENCE, len = 28
                0x02, 0x01, 0x01,    // version = v2c
                0x04, 0x06, 'p','u','b','l','i','c',
                0xA2, 0x0F,          // GetResponse-PDU, len = 15
                    0x02, 0x01, 0x01,// request-id = 1
                    0x02, 0x01, 0x00,// error-status = 0
                    0x02, 0x01, 0x00,// error-index = 0
                    0x30, 0x04,      // VarBindList, len = 4
                        0x30, 0x02,  // VarBind, len = 2
                            0x05, 0x00 // WRONG OID tag: NULL instead of OID
        };
        // clang-format on

        return copyResponse(buf, len, response, sizeof(response));
    }

    if (wrapState().returnVarBindWithoutValue && sockfd == FAKE_SOCKET_FD) {
        // clang-format off
        const uint8_t response[] = {
            0x30, 0x1E,              // Message SEQUENCE, len = 30
                0x02, 0x01, 0x01,    // version = v2c
                0x04, 0x06, 'p','u','b','l','i','c',
                0xA2, 0x11,          // GetResponse-PDU, len = 17
                    0x02, 0x01, 0x01,// request-id = 1
                    0x02, 0x01, 0x00,// error-status = 0
                    0x02, 0x01, 0x00,// error-index = 0
                    0x30, 0x06,      // VarBindList, len = 6
                        0x30, 0x04,  // VarBind, len = 4
                            0x06, 0x02, 0x2B, 0x06 // OID = 1.3.6, no value field
        };
        // clang-format on

        return copyResponse(buf, len, response, sizeof(response));
    }

    if (wrapState().returnInvalidIntegerValue && sockfd == FAKE_SOCKET_FD) {
        // clang-format off
        const uint8_t response[] = {
            0x30, 0x21,              // Message SEQUENCE, len = 33
                0x02, 0x01, 0x01,    // version = v2c
                0x04, 0x06, 'p','u','b','l','i','c',
                0xA2, 0x14,          // GetResponse-PDU, len = 20
                    0x02, 0x01, 0x01,// request-id = 1
                    0x02, 0x01, 0x00,// error-status = 0
                    0x02, 0x01, 0x00,// error-index = 0
                    0x30, 0x09,      // VarBindList, len = 9
                        0x30, 0x07,  // VarBind, len = 7
                            0x06, 0x02, 0x2B, 0x06, // OID = 1.3.6
                            0x02, 0x02, 0x01        // INTEGER length exceeds VarBind
        };
        // clang-format on

        return copyResponse(buf, len, response, sizeof(response));
    }

    if (wrapState().returnInvalidStringValue && sockfd == FAKE_SOCKET_FD) {
        // clang-format off
        const uint8_t response[] = {
            0x30, 0x21,              // Message SEQUENCE, len = 33
                0x02, 0x01, 0x01,    // version = v2c
                0x04, 0x06, 'p','u','b','l','i','c',
                0xA2, 0x14,          // GetResponse-PDU, len = 20
                    0x02, 0x01, 0x01,// request-id = 1
                    0x02, 0x01, 0x00,// error-status = 0
                    0x02, 0x01, 0x00,// error-index = 0
                    0x30, 0x09,      // VarBindList, len = 9
                        0x30, 0x07,  // VarBind, len = 7
                            0x06, 0x02, 0x2B, 0x06, // OID = 1.3.6
                            0x04, 0x02, 'A'         // OCTET STRING length exceeds VarBind
        };
        // clang-format on

        return copyResponse(buf, len, response, sizeof(response));
    }

    if (wrapState().returnInvalidNullValue && sockfd == FAKE_SOCKET_FD) {
        // clang-format off
        const uint8_t response[] = {
            0x30, 0x20,              // Message SEQUENCE, len = 32
                0x02, 0x01, 0x01,    // version = v2c
                0x04, 0x06, 'p','u','b','l','i','c',
                0xA2, 0x13,          // GetResponse-PDU, len = 19
                    0x02, 0x01, 0x01,// request-id = 1
                    0x02, 0x01, 0x00,// error-status = 0
                    0x02, 0x01, 0x00,// error-index = 0
                    0x30, 0x08,      // VarBindList, len = 8
                        0x30, 0x06,  // VarBind, len = 6
                            0x06, 0x02, 0x2B, 0x06, // OID = 1.3.6
                            0x05, 0x01              // NULL length exceeds VarBind
        };
        // clang-format on

        return copyResponse(buf, len, response, sizeof(response));
    }

    if (wrapState().returnUnsupportedValueType && sockfd == FAKE_SOCKET_FD) {
        // clang-format off
        const uint8_t response[] = {
            0x30, 0x20,              // Message SEQUENCE, len = 32
                0x02, 0x01, 0x01,    // version = v2c
                0x04, 0x06, 'p','u','b','l','i','c',
                0xA2, 0x13,          // GetResponse-PDU, len = 19
                    0x02, 0x01, 0x01,// request-id = 1
                    0x02, 0x01, 0x00,// error-status = 0
                    0x02, 0x01, 0x00,// error-index = 0
                    0x30, 0x08,      // VarBindList, len = 8
                        0x30, 0x06,  // VarBind, len = 6
                            0x06, 0x02, 0x2B, 0x06, // OID = 1.3.6
                            0x01, 0x00              // BOOLEAN is unsupported
        };
        // clang-format on

        return copyResponse(buf, len, response, sizeof(response));
    }

    if (!wrapState().returnEmptyResponse || sockfd != FAKE_SOCKET_FD) {
        errno = EBADF;
        return -1;
    }

    // clang-format off
    const uint8_t response[] = {
        0x30, 0x18,                  // Message SEQUENCE, len = 24
            0x02, 0x01, 0x01,        // version = v2c
            0x04, 0x06, 'p','u','b','l','i','c',
            0xA2, 0x0B,              // GetResponse-PDU, len = 11
                0x02, 0x01, 0x01,    // request-id = 1
                0x02, 0x01, 0x00,    // error-status = 0
                0x02, 0x01, 0x00,    // error-index = 0
                0x30, 0x00           // empty VarBindList
    };
    // clang-format on

    return copyResponse(buf, len, response, sizeof(response));
}

}  // extern "C"

class SnmpClientOpenSocketTest : public ::testing::Test {
protected:
    void SetUp() override { resetWrapState(); }
    void TearDown() override { resetWrapState(); }
};

#if 1  // Часть 1 — Ошибки открытия сокета
// ============================================================
// Часть 1 — Ошибки открытия сокета
// ============================================================

// Тест 1.1: socket() завершился ошибкой
TEST_F(SnmpClientOpenSocketTest, OpenSocket_WhenSocketCreationFails_ReturnsError) {
    wrapState().failSocket = true;

    snmp::SnmpClient client("127.0.0.1");
    snmp::codec::SnmpValue value;
    snmp::ErrorMessage err;

    ASSERT_FALSE(client.get("1.3.6", value, &err));
    EXPECT_EQ(err, "Failed to create UDP socket");
    EXPECT_EQ(wrapState().closeCalls, 0);
}

// Тест 1.2: setsockopt(SO_RCVTIMEO) завершился ошибкой
TEST_F(SnmpClientOpenSocketTest, OpenSocket_WhenSetReceiveTimeoutFails_ClosesSocketAndReturnsError) {
    wrapState().failReceiveTimeout = true;

    snmp::SnmpClient client("127.0.0.1");
    snmp::codec::SnmpValue value;
    snmp::ErrorMessage err;

    ASSERT_FALSE(client.get("1.3.6", value, &err));
    EXPECT_EQ(err, "Failed to set socket receive timeout");
    EXPECT_EQ(wrapState().closeCalls, 1);
}

#endif

#if 1  // Часть 2 — GET нескольких OID
// ============================================================
// Часть 2 — GET нескольких OID
// ============================================================

// Тест 2.1: sendto() завершился ошибкой
TEST_F(SnmpClientOpenSocketTest, GetManyOids_WhenSendFails_ReturnsError) {
    wrapState().failSend = true;

    snmp::SnmpClient client("127.0.0.1");
    std::vector<snmp::codec::SnmpValue> values;
    snmp::ErrorMessage err;

    ASSERT_FALSE(client.get(std::vector<snmp::Oid>{ "1.3.6" }, values, &err));
    EXPECT_EQ(err, "Failed to send SNMP request");
}

// Тест 2.2: ответ получен, но не декодируется как SNMP GetResponse
TEST_F(SnmpClientOpenSocketTest, GetManyOids_WhenResponseDecodeFails_ReturnsDecodeError) {
    wrapState().returnInvalidResponse = true;

    snmp::SnmpClient client("127.0.0.1");
    std::vector<snmp::codec::SnmpValue> values;
    snmp::ErrorMessage err;

    ASSERT_FALSE(client.get(std::vector<snmp::Oid>{ "1.3.6" }, values, &err));
    EXPECT_EQ(err, "Invalid tag: expected 0x30, got 0x31");
}

// Тест 2.3: recvfrom() завершился по тайм-ауту
TEST_F(SnmpClientOpenSocketTest, GetManyOids_WhenReceiveTimesOut_ReturnsError) {
    wrapState().returnTimeout = true;

    snmp::SnmpClient client("127.0.0.1");
    std::vector<snmp::codec::SnmpValue> values;
    snmp::ErrorMessage err;

    ASSERT_FALSE(client.get(std::vector<snmp::Oid>{ "1.3.6" }, values, &err));
    EXPECT_EQ(err, "SNMP response timeout");
}

#endif

#if 1  // Часть 3 — Ошибки формата версии SNMP
// ============================================================
// Часть 3 — Ошибки формата версии SNMP
// ============================================================

// Тест 3.1: поле version имеет неверный BER-тег
TEST_F(SnmpClientOpenSocketTest, DecodeGetResponse_WhenVersionTagIsInvalid_ReturnsError) {
    wrapState().returnInvalidVersionTag = true;

    snmp::SnmpClient client("127.0.0.1");
    std::vector<snmp::codec::SnmpValue> values;
    snmp::ErrorMessage err;

    ASSERT_FALSE(client.get(std::vector<snmp::Oid>{ "1.3.6" }, values, &err));
    EXPECT_EQ(err, "Invalid tag: expected 0x02, got 0x05");
}

// Тест 3.2: версия SNMP не поддерживается клиентом
TEST_F(SnmpClientOpenSocketTest, DecodeGetResponse_WhenVersionIsUnsupported_ReturnsError) {
    wrapState().returnUnsupportedVersion = true;

    snmp::SnmpClient client("127.0.0.1");
    std::vector<snmp::codec::SnmpValue> values;
    snmp::ErrorMessage err;

    ASSERT_FALSE(client.get(std::vector<snmp::Oid>{ "1.3.6" }, values, &err));
    EXPECT_EQ(err, "Unsupported SNMP version (expected v1 or v2c)");
}

#endif

#if 1  // Часть 4 — Ошибки декодирования community
// ============================================================
// Часть 4 — Ошибки декодирования community
// ============================================================

// Тест 4.1: поле community имеет неверный BER-тег
TEST_F(SnmpClientOpenSocketTest, DecodeGetResponse_WhenCommunityTagIsInvalid_ReturnsError) {
    wrapState().returnInvalidCommunityTag = true;

    snmp::SnmpClient client("127.0.0.1");
    std::vector<snmp::codec::SnmpValue> values;
    snmp::ErrorMessage err;

    ASSERT_FALSE(client.get(std::vector<snmp::Oid>{ "1.3.6" }, values, &err));
    EXPECT_EQ(err, "Invalid tag: expected 0x04, got 0x05");
}

#endif

#if 1  // Часть 5 — Ошибки декодирования GetResponse PDU
// ============================================================
// Часть 5 — Ошибки декодирования GetResponse PDU
// ============================================================

// Тест 5.1: PDU имеет неверный BER-тег
TEST_F(SnmpClientOpenSocketTest, DecodeGetResponse_WhenPduTagIsInvalid_ReturnsError) {
    wrapState().returnInvalidPduTag = true;

    snmp::SnmpClient client("127.0.0.1");
    std::vector<snmp::codec::SnmpValue> values;
    snmp::ErrorMessage err;

    ASSERT_FALSE(client.get(std::vector<snmp::Oid>{ "1.3.6" }, values, &err));
    EXPECT_EQ(err, "Invalid tag: expected 0xA2, got 0xA0");
}

#endif

#if 1  // Часть 6 — Ошибки декодирования request-id
// ============================================================
// Часть 6 — Ошибки декодирования request-id
// ============================================================

// Тест 6.1: request-id имеет неверный BER-тег
TEST_F(SnmpClientOpenSocketTest, DecodeGetResponse_WhenRequestIdTagIsInvalid_ReturnsError) {
    wrapState().returnInvalidRequestIdTag = true;

    snmp::SnmpClient client("127.0.0.1");
    std::vector<snmp::codec::SnmpValue> values;
    snmp::ErrorMessage err;

    ASSERT_FALSE(client.get(std::vector<snmp::Oid>{ "1.3.6" }, values, &err));
    EXPECT_EQ(err, "Invalid tag: expected 0x02, got 0x05");
}

// Тест 6.2: request-id ответа не совпадает с request-id запроса
TEST_F(SnmpClientOpenSocketTest, DecodeGetResponse_WhenRequestIdDiffers_ReturnsError) {
    wrapState().returnMismatchedRequestId = true;

    snmp::SnmpClient client("127.0.0.1");
    std::vector<snmp::codec::SnmpValue> values;
    snmp::ErrorMessage err;

    ASSERT_FALSE(client.get(std::vector<snmp::Oid>{ "1.3.6" }, values, &err));
    EXPECT_EQ(err, "SNMP response request-id mismatch");
}

#endif

#if 1  // Часть 7 — Ошибки декодирования error-status / error-index
// ============================================================
// Часть 7 — Ошибки декодирования error-status / error-index
// ============================================================

// Тест 7.1: error-status имеет неверный BER-тег
TEST_F(SnmpClientOpenSocketTest, DecodeGetResponse_WhenErrorStatusTagIsInvalid_ReturnsError) {
    wrapState().returnInvalidErrorStatusTag = true;

    snmp::SnmpClient client("127.0.0.1");
    std::vector<snmp::codec::SnmpValue> values;
    snmp::ErrorMessage err;

    ASSERT_FALSE(client.get(std::vector<snmp::Oid>{ "1.3.6" }, values, &err));
    EXPECT_EQ(err, "Invalid tag: expected 0x02, got 0x05");
}

// Тест 7.2: error-index имеет неверный BER-тег
TEST_F(SnmpClientOpenSocketTest, DecodeGetResponse_WhenErrorIndexTagIsInvalid_ReturnsError) {
    wrapState().returnInvalidErrorIndexTag = true;

    snmp::SnmpClient client("127.0.0.1");
    std::vector<snmp::codec::SnmpValue> values;
    snmp::ErrorMessage err;

    ASSERT_FALSE(client.get(std::vector<snmp::Oid>{ "1.3.6" }, values, &err));
    EXPECT_EQ(err, "Invalid tag: expected 0x02, got 0x05");
}

// Тест 7.3: SNMP-агент вернул ненулевой error-status
TEST_F(SnmpClientOpenSocketTest, DecodeGetResponse_WhenErrorStatusIsNonZero_ReturnsError) {
    wrapState().returnSnmpErrorStatus = true;

    snmp::SnmpClient client("127.0.0.1");
    std::vector<snmp::codec::SnmpValue> values;
    snmp::ErrorMessage err;

    ASSERT_FALSE(client.get(std::vector<snmp::Oid>{ "1.3.6" }, values, &err));
    EXPECT_EQ(err, "SNMP error-status = 5, error-index = 2");
}

#endif

#if 1  // Часть 8 — Ошибки декодирования VarBindList
// ============================================================
// Часть 8 — Ошибки декодирования VarBindList
// ============================================================

// Тест 8.1: VarBindList имеет неверный BER-тег
TEST_F(SnmpClientOpenSocketTest, DecodeGetResponse_WhenVarBindListTagIsInvalid_ReturnsError) {
    wrapState().returnInvalidVarBindListTag = true;

    snmp::SnmpClient client("127.0.0.1");
    std::vector<snmp::codec::SnmpValue> values;
    snmp::ErrorMessage err;

    ASSERT_FALSE(client.get(std::vector<snmp::Oid>{ "1.3.6" }, values, &err));
    EXPECT_EQ(err, "Invalid tag: expected 0x30, got 0x05");
}

#endif

#if 1  // Часть 9 — Ошибки декодирования VarBind
// ============================================================
// Часть 9 — Ошибки декодирования VarBind
// ============================================================

// Тест 9.1: VarBind имеет неверный BER-тег
TEST_F(SnmpClientOpenSocketTest, DecodeGetResponse_WhenVarBindTagIsInvalid_ReturnsError) {
    wrapState().returnInvalidVarBindTag = true;

    snmp::SnmpClient client("127.0.0.1");
    std::vector<snmp::codec::SnmpValue> values;
    snmp::ErrorMessage err;

    ASSERT_FALSE(client.get(std::vector<snmp::Oid>{ "1.3.6" }, values, &err));
    EXPECT_EQ(err, "Invalid tag: expected 0x30, got 0x05");
}

// Тест 9.2: OID внутри VarBind имеет неверный BER-тег
TEST_F(SnmpClientOpenSocketTest, DecodeGetResponse_WhenOidTagIsInvalid_ReturnsError) {
    wrapState().returnInvalidOidTag = true;

    snmp::SnmpClient client("127.0.0.1");
    std::vector<snmp::codec::SnmpValue> values;
    snmp::ErrorMessage err;

    ASSERT_FALSE(client.get(std::vector<snmp::Oid>{ "1.3.6" }, values, &err));
    EXPECT_EQ(err, "Invalid tag: expected 0x06, got 0x05");
}

// Тест 9.3: VarBind содержит OID, но не содержит поле value
TEST_F(SnmpClientOpenSocketTest, DecodeGetResponse_WhenVarBindHasNoValue_ReturnsError) {
    wrapState().returnVarBindWithoutValue = true;

    snmp::SnmpClient client("127.0.0.1");
    std::vector<snmp::codec::SnmpValue> values;
    snmp::ErrorMessage err;

    ASSERT_FALSE(client.get(std::vector<snmp::Oid>{ "1.3.6" }, values, &err));
    EXPECT_EQ(err, "VarBind missing value field");
}

#endif

#if 1  // Часть 10 — Ошибки декодирования значения VarBind
// ============================================================
// Часть 10 — Ошибки декодирования значения VarBind
// ============================================================

// Тест 10.1: INTEGER-значение имеет длину больше границы VarBind
TEST_F(SnmpClientOpenSocketTest, DecodeGetResponse_WhenIntegerValueLengthExceedsVarBind_ReturnsError) {
    wrapState().returnInvalidIntegerValue = true;

    snmp::SnmpClient client("127.0.0.1");
    std::vector<snmp::codec::SnmpValue> values;
    snmp::ErrorMessage err;

    ASSERT_FALSE(client.get(std::vector<snmp::Oid>{ "1.3.6" }, values, &err));
    EXPECT_EQ(err, "ASN.1 length exceeds buffer");
}

// Тест 10.2: OCTET STRING-значение имеет длину больше границы VarBind
TEST_F(SnmpClientOpenSocketTest, DecodeGetResponse_WhenStringValueLengthExceedsVarBind_ReturnsError) {
    wrapState().returnInvalidStringValue = true;

    snmp::SnmpClient client("127.0.0.1");
    std::vector<snmp::codec::SnmpValue> values;
    snmp::ErrorMessage err;

    ASSERT_FALSE(client.get(std::vector<snmp::Oid>{ "1.3.6" }, values, &err));
    EXPECT_EQ(err, "ASN.1 length exceeds buffer");
}

// Тест 10.3: NULL-значение имеет ненулевую длину без данных внутри VarBind
TEST_F(SnmpClientOpenSocketTest, DecodeGetResponse_WhenNullValueLengthExceedsVarBind_ReturnsError) {
    wrapState().returnInvalidNullValue = true;

    snmp::SnmpClient client("127.0.0.1");
    std::vector<snmp::codec::SnmpValue> values;
    snmp::ErrorMessage err;

    ASSERT_FALSE(client.get(std::vector<snmp::Oid>{ "1.3.6" }, values, &err));
    EXPECT_EQ(err, "ASN.1 length exceeds buffer");
}

// Тест 10.4: тип значения VarBind не поддерживается клиентом
TEST_F(SnmpClientOpenSocketTest, DecodeGetResponse_WhenValueTypeIsUnsupported_ReturnsError) {
    wrapState().returnUnsupportedValueType = true;

    snmp::SnmpClient client("127.0.0.1");
    std::vector<snmp::codec::SnmpValue> values;
    snmp::ErrorMessage err;

    ASSERT_FALSE(client.get(std::vector<snmp::Oid>{ "1.3.6" }, values, &err));
    EXPECT_EQ(err, "Unsupported SNMP value type (tag = 0x1)");
}

#endif

#if 1  // Часть 11 — GET одного OID
// ============================================================
// Часть 11 — GET одного OID
// ============================================================

// Тест 11.1: ответ SNMP успешно декодирован, но не содержит значений
TEST_F(SnmpClientOpenSocketTest, GetSingleOid_WhenResponseContainsNoValues_ReturnsError) {
    wrapState().returnEmptyResponse = true;

    snmp::SnmpClient client("127.0.0.1");
    snmp::codec::SnmpValue value;
    snmp::ErrorMessage err;

    ASSERT_FALSE(client.get("1.3.6", value, &err));
    EXPECT_EQ(err, "SNMP response contains no values");
}

// Тест 11.2: успешный ответ возвращает значение и клиент удаляется через интерфейс
TEST_F(SnmpClientOpenSocketTest, GetSingleOid_WhenResponseIsValid_ReturnsValue) {
    wrapState().returnValidResponse = true;

    std::unique_ptr<snmp::ISnmpClient> client(new snmp::SnmpClient("127.0.0.1"));
    snmp::codec::SnmpValue value;
    snmp::ErrorMessage err;

    ASSERT_TRUE(client->get("1.3.6", value, &err)) << err;
    EXPECT_EQ(value.oid, "1.3.6");
    EXPECT_EQ(value.type, snmp::codec::SnmpValue::Type::Integer);
    EXPECT_EQ(value.intValue, 5);
}

#endif

#if 1  // Часть 12 — Выбор и проверка версии SNMP
// ============================================================
// Часть 12 — Выбор и проверка версии SNMP
// ============================================================

// Тест 12.1: по умолчанию клиент использует SNMP v2c
TEST_F(SnmpClientOpenSocketTest, GetSingleOid_ByDefault_UsesSnmpV2c) {
    wrapState().returnValidResponse = true;

    snmp::SnmpClient client("127.0.0.1");
    snmp::codec::SnmpValue value;
    snmp::ErrorMessage err;

    ASSERT_TRUE(client.get("1.3.6", value, &err)) << err;
    EXPECT_EQ(wrapState().sentVersion, 1);
}

// Тест 12.2: явно выбранная версия SNMP v1 используется в запросе и ответе
TEST_F(SnmpClientOpenSocketTest, GetSingleOid_WhenSnmpV1Selected_UsesSnmpV1) {
    wrapState().returnValidResponse = true;
    wrapState().responseVersion = 0;

    snmp::SnmpClient client(
        "127.0.0.1", 161, "public", snmp::codec::SnmpVersion::V_1);
    snmp::codec::SnmpValue value;
    snmp::ErrorMessage err;

    ASSERT_TRUE(client.get("1.3.6", value, &err)) << err;
    EXPECT_EQ(wrapState().sentVersion, 0);
    EXPECT_EQ(value.intValue, 5);
}

// Тест 12.3: ответ другой версии отклоняется
TEST_F(SnmpClientOpenSocketTest, GetSingleOid_WhenResponseVersionDiffers_ReturnsError) {
    wrapState().returnValidResponse = true;

    snmp::SnmpClient client(
        "127.0.0.1", 161, "public", snmp::codec::SnmpVersion::V_1);
    snmp::codec::SnmpValue value;
    snmp::ErrorMessage err;

    ASSERT_FALSE(client.get("1.3.6", value, &err));
    EXPECT_EQ(wrapState().sentVersion, 0);
    EXPECT_EQ(err, "SNMP response version mismatch");
}

#endif
