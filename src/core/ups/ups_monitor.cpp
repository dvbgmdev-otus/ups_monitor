/**
 * @file ups_monitor.cpp
 * @ingroup ups
 * @brief Реализация монитора UPS и фонового SNMP-опроса.
 */
#include "ups_monitor.h"

#include <chrono>
#include <exception>

#include "fs_utils.h"
#include "snmp_client.h"
#include "ups_model_detector.h"

namespace {

constexpr std::chrono::seconds POLL_PERIOD{ 1 };

}  // namespace

UpsMonitor::~UpsMonitor() { stop(); }

void UpsMonitor::stop() {
    m_running.store(false);
    m_waitCondition.notify_all();
    if (m_thread.joinable()) {
        m_thread.join();
    }
}

bool UpsMonitor::init(const std::string& ip, uint16_t port, ups::ErrorMessage& error) {
    error.clear();

    // Защита от повторного init
    if (m_initialized) {
        error = "UPS monitor is already initialized";
        return false;
    }

    // --- 1. создаём SNMP client ---
    try {
        m_snmp = createSnmpClient(ip, port);
    } catch (const std::exception& exception) {
        error = "SNMP client creation failed: ";
        error += exception.what();
        return false;
    }

    // --- 2. определяем модель UPS ---
    const std::string upsModelSpecFile{ utils::resolvePath("../config/ups_model_spec.ini") };
    ups::IniSectionName model;
    if (!ups::UpsModelDetector::detect(*m_snmp, upsModelSpecFile, model, error)) {
        return false;
    }

    // --- 3. загружаем спецификацию модели ---
    // UpsModelDetector вроде бы уже все проверил и следующая проверка не нужна
    // но оставим на всякий случай
    if (!m_modelSpec.load(upsModelSpecFile, model)) {
        // LCOV_EXCL_START
        error = m_modelSpec.lastError();
        return false;
        // LCOV_EXCL_STOP
    }

    // --- 4. фиксируем успешную инициализацию ---
    m_running.store(true);
    try {
        m_thread = std::thread(&UpsMonitor::pollLoop, this);
        // LCOV_EXCL_START
    } catch (const std::exception& exception) {
        m_running.store(false);
        error = "UPS polling thread start failed for model ";
        error += m_modelSpec.modelName();
        error += ": ";
        error += exception.what();
        return false;
    }
    // LCOV_EXCL_STOP
    m_initialized = true;
    return true;
}

bool UpsMonitor::tryConsumeState(ups::UpsState& state) {
    return m_stateBuffer.tryConsumeState(state);
}

const std::string& UpsMonitor::modelName() const { return m_modelSpec.modelName(); }

std::unique_ptr<snmp::ISnmpClient> UpsMonitor::createSnmpClient(const std::string& ip,
                                                                uint16_t port) {
    return std::unique_ptr<snmp::ISnmpClient>(new snmp::SnmpClient(ip, port));
}

void UpsMonitor::pollLoop() {
    while (m_running.load()) {
        auto t0 = std::chrono::steady_clock::now();

        // 1. Один реальный опрос UPS
        ups::UpsState state = ups::UpsStatePoller::poll(m_modelSpec, *m_snmp);

        // 2. Сохраняем состояние
        m_stateBuffer.storeState(state);

        // 3. Ожидание следующего опроса с учётом времени выполнения текущего цикла
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::steady_clock::now() - t0);

        if (elapsed < POLL_PERIOD) {
            std::unique_lock<std::mutex> lock(m_waitMutex);
            m_waitCondition.wait_for(
                lock, POLL_PERIOD - elapsed, [this] { return !m_running.load(); });
        }
    }
}
