/**
 * @file snmp_ready_waiter.cpp
 * @brief Реализация ожидания готовности SNMP-агента.
 */
#include "snmp_ready_waiter.h"

#include <thread>

bool waitUntilSnmpReady(snmp::ISnmpClient& client,
                        const snmp::Oid& oid,
                        unsigned int attempts,
                        std::chrono::milliseconds retryInterval,
                        snmp::ErrorMessage* err) {
    if (attempts == 0) {
        if (err != nullptr) {
            *err = "SNMP readiness check requires at least one attempt";
        }
        return false;
    }

    snmp::ErrorMessage lastError;
    snmp::codec::SnmpValue value;

    for (unsigned int attempt = 0; attempt < attempts; ++attempt) {
        lastError.clear();
        if (client.get(oid, value, &lastError)) {
            if (err != nullptr) {
                err->clear();
            }
            return true;
        }

        if (attempt + 1 < attempts) {
            std::this_thread::sleep_for(retryInterval);
        }
    }

    if (err != nullptr) {
        *err = lastError;
    }
    return false;
}
