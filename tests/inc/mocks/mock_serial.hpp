#pragma once

#include "serial.hpp"

#include "gmock/gmock.h"

class SerialMock : public serial
{
  public:
    MOCK_METHOD(void, setBaud, (speed_t), (override));
    MOCK_METHOD(size_t, read,
                (std::vector<uint8_t>&, ssize_t, uint32_t, debug_t),
                (override));
    MOCK_METHOD(size_t, read, (std::vector<uint8_t>&, ssize_t, debug_t),
                (override));
    MOCK_METHOD(size_t, write, (const std::vector<uint8_t>&, debug_t),
                (override));
    MOCK_METHOD(void, flushBuffer, (), (override));
};
