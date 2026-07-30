/**
 * @file ups_state_buffer_test.cpp
 * @brief Unit-тесты потокобезопасного буфера состояния ИБП.
 *
 * Проверяется:
 *  - отсутствие нового состояния;
 *  - однократное потребление состояния;
 *  - замена непрочитанного состояния;
 *  - повторные циклы сохранения и потребления;
 *  - сохранность статуса и маски отклонений.
 */

#include "ups_state_buffer.h"

#include <gtest/gtest.h>

#include "ups_model_types.h"

class UpsStateBufferTest : public ::testing::Test {
protected:
    static ups::UpsState makeState(ups::UpsStatus status, ups::UpsDeviationFlags deviations) {
        ups::UpsState state;
        state.status = status;
        state.deviations = deviations;
        return state;
    }

    static void expectState(const ups::UpsState& state,
                            ups::UpsStatus expectedStatus,
                            ups::UpsDeviationFlags expectedDeviations) {
        EXPECT_EQ(state.status, expectedStatus);
        EXPECT_EQ(state.deviations, expectedDeviations);
    }

    ups::UpsStateBuffer m_buffer;
};

#if (1)  // Part 1 — Пустой буфер

// Test 1.1: Пустой буфер возвращает false и не изменяет выходное состояние
TEST_F(UpsStateBufferTest, TryConsumeState_EmptyBuffer_ReturnsFalseAndPreservesOutput) {
    ups::UpsState out = makeState(ups::UpsStatus::FAILURE, ups::UpsDeviationFlags::OUTPUT_FAILURE);
    EXPECT_FALSE(m_buffer.tryConsumeState(out));
    expectState(out, ups::UpsStatus::FAILURE, ups::UpsDeviationFlags::OUTPUT_FAILURE);
}

#endif

#if (1)  // Part 2 — Сохранение и однократное потребление

// Test 2.1: Сохранённое состояние извлекается без изменений
TEST_F(UpsStateBufferTest, TryConsumeState_StoredState_ReturnsStateWithoutChanges) {
    const ups::UpsDeviationFlags deviations =
        ups::UpsDeviationFlags::INPUT_ALERT | ups::UpsDeviationFlags::BYPASS_ALERT;
    m_buffer.storeState(makeState(ups::UpsStatus::WARNING, deviations));
    ups::UpsState out;
    ASSERT_TRUE(m_buffer.tryConsumeState(out));
    expectState(out, ups::UpsStatus::WARNING, deviations);
}

// Test 2.2: Повторное чтение возвращает false и не изменяет выходное состояние
TEST_F(UpsStateBufferTest, TryConsumeState_StateAlreadyConsumed_ReturnsFalseAndPreservesOutput) {
    m_buffer.storeState(makeState(ups::UpsStatus::OK, ups::UpsDeviationFlags::NONE));
    ups::UpsState firstOut;
    ASSERT_TRUE(m_buffer.tryConsumeState(firstOut));
    ups::UpsState secondOut =
        makeState(ups::UpsStatus::FAILURE, ups::UpsDeviationFlags::OUTPUT_FAILURE);
    EXPECT_FALSE(m_buffer.tryConsumeState(secondOut));
    expectState(secondOut, ups::UpsStatus::FAILURE, ups::UpsDeviationFlags::OUTPUT_FAILURE);
}

#endif

#if (1)  // Part 3 — Замена состояния

// Test 3.1: Новая запись до чтения заменяет предыдущее состояние
TEST_F(UpsStateBufferTest, TryConsumeState_TwoStatesStored_ReturnsLastStateOnly) {
    m_buffer.storeState(makeState(ups::UpsStatus::WARNING, ups::UpsDeviationFlags::BATTERY_ALERT));
    m_buffer.storeState(makeState(ups::UpsStatus::FAILURE, ups::UpsDeviationFlags::OUTPUT_FAILURE));
    ups::UpsState out;
    ASSERT_TRUE(m_buffer.tryConsumeState(out));
    expectState(out, ups::UpsStatus::FAILURE, ups::UpsDeviationFlags::OUTPUT_FAILURE);
    EXPECT_FALSE(m_buffer.tryConsumeState(out));
}

#endif

#if (1)  // Part 4 — Повторные циклы

// Test 4.1: Последовательные циклы сохранения и потребления работают независимо
TEST_F(UpsStateBufferTest, TryConsumeState_RepeatedStoreConsumeCycles_ReturnsEachNewState) {
    ups::UpsState out;
    m_buffer.storeState(makeState(ups::UpsStatus::OK, ups::UpsDeviationFlags::NONE));
    ASSERT_TRUE(m_buffer.tryConsumeState(out));
    expectState(out, ups::UpsStatus::OK, ups::UpsDeviationFlags::NONE);
    m_buffer.storeState(makeState(ups::UpsStatus::WARNING, ups::UpsDeviationFlags::CHARGE_ALERT));
    ASSERT_TRUE(m_buffer.tryConsumeState(out));
    expectState(out, ups::UpsStatus::WARNING, ups::UpsDeviationFlags::CHARGE_ALERT);
}

#endif

#if (1)  // Part 5 — Разные значения состояния

// Test 5.1: Буфер сохраняет все статусы и комбинированную маску отклонений
TEST_F(UpsStateBufferTest, TryConsumeState_DifferentValues_PreservesStatusesAndCombinedMask) {
    const ups::UpsStatus statuses[] = {
        ups::UpsStatus::OK,
        ups::UpsStatus::WARNING,
        ups::UpsStatus::FAILURE,
        ups::UpsStatus::NO_INFO,
    };
    const ups::UpsDeviationFlags deviations = ups::UpsDeviationFlags::BATTERY_ALERT |
                                              ups::UpsDeviationFlags::CHARGE_ALERT |
                                              ups::UpsDeviationFlags::OUTPUT_FAILURE;
    for (const ups::UpsStatus status : statuses) {
        m_buffer.storeState(makeState(status, deviations));
        ups::UpsState out;
        ASSERT_TRUE(m_buffer.tryConsumeState(out));
        expectState(out, status, deviations);
    }
}

#endif
