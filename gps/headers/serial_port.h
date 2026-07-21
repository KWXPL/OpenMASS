#pragma once

#include <chrono>
#include <cstddef>
#include <string>
#include <string_view>

class SerialPort {
  public:
    explicit SerialPort(const std::string& device);
    ~SerialPort();

    SerialPort(const SerialPort&) = delete;
    SerialPort& operator=(const SerialPort&) = delete;

    void flushInput();
    void writeAll(std::string_view data);
    std::size_t readSome(char* buffer, std::size_t capacity, std::chrono::milliseconds timeout);

  private:
    int fd_{-1};
};
