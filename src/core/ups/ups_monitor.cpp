/**
 * @file ups_monitor.cpp
 * @ingroup ups
 * @brief Реализация монитора UPS и фонового SNMP-опроса.
 */
#include "ups_monitor.h"

#include <chrono>
#include <stdexcept>

#include "fs_utils.h"
#include "snmp_client.h"
#include "ups_model_detector.h"

UpsMonitor::~UpsMonitor() {
    m_running.store(false);
    if (m_thread.joinable()) {
        m_thread.join();
    }
}

void UpsMonitor::init(const std::string& configPath) {
    // Защита от повторного init
    if (m_running.load()) {
        throw std::logic_error("UpsMonitor::init() called more than once");
    }

    // --- 1. читаем конфиг с IP ---
    const std::string ip{ "127.0.0.1" };

    // --- 2. создаём SNMP client ---
    m_snmp = createSnmpClient(ip);

    // --- 3. определяем модель UPS ---
    const std::string upsModelSpecFile{ utils::resolvePath("../config/ups_model_spec.ini") };
    std::string modelName;
    ups::ErrorMessage err;

    if (!ups::UpsModelDetector::detect(*m_snmp, upsModelSpecFile, modelName, err)) {
        throw std::runtime_error("UPS model detection failed: " + err);
    }

    // --- 4. загружаем спецификацию модели ---
    // UpsModelDetector вроде бы уже все проверил и следующая проверка не нужна
    // но оставим на всякий случай
    if (!m_modelSpec.load(upsModelSpecFile, modelName)) {
        // LCOV_EXCL_START
        throw std::runtime_error("UPS model spec load failed: " + m_modelSpec.lastError());
        // LCOV_EXCL_STOP
    }

    // --- 5. фиксируем успешную инициализацию ---
    m_running.store(true);
    m_thread = std::thread(&UpsMonitor::pollLoop, this);
}

bool UpsMonitor::tryConsumeState(ups::UpsState& state) {
    return m_stateBuffer.tryConsumeState(state);
}

std::unique_ptr<snmp::ISnmpClient> UpsMonitor::createSnmpClient(const std::string& ip) {
    return std::unique_ptr<snmp::ISnmpClient>(new snmp::SnmpClient(ip));
}

void UpsMonitor::pollLoop() {
    const auto updatePeriod = std::chrono::milliseconds(1000);

    while (m_running.load()) {
        auto t0 = std::chrono::steady_clock::now();

        // 1. Один реальный опрос UPS
        ups::UpsState state = ups::UpsStatePoller::poll(m_modelSpec, *m_snmp);

        // 2. Сохраняем состояние
        m_stateBuffer.storeState(state);

        // 3. sleep до следующего опроса, учитывая время выполнения текущего цикла
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::steady_clock::now() - t0);

        if (elapsed < updatePeriod) {
            std::this_thread::sleep_for(updatePeriod - elapsed);
        }
    }
}
