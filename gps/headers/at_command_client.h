#pragma once

#include <chrono>
#include <string>
#include <string_view>

class SerialPort;

class AtCommandClient {
  public:
    explicit AtCommandClient(SerialPort& serial);
    std::string command(std::string_view command, std::chrono::milliseconds timeout);

  private:
    SerialPort& serial_;
};

bool isOkResponse(std::string_view response);
