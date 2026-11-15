#include "at_command_client.h"

#include "serial_port.h"

AtCommandClient::AtCommandClient(SerialPort& serial) : serial_(serial) {}

std::string AtCommandClient::command(std::string_view command, std::chrono::milliseconds timeout) {
    serial_.flushInput();
    const std::string request = std::string(command) + "\r";
    serial_.writeAll(request);

    std::string response;
    const auto deadline = std::chrono::steady_clock::now() + timeout;
    while (std::chrono::steady_clock::now() < deadline) {
        const auto remaining = std::chrono::duration_cast<std::chrono::milliseconds>(
            deadline - std::chrono::steady_clock::now());
        char buffer[256];
        const std::size_t count = serial_.readSome(buffer, sizeof(buffer), remaining);
        if (count == 0) {
            break;
        }
        response.append(buffer, count);
        if (isOkResponse(response) || response.find("\r\nERROR\r\n") != std::string::npos) {
            break;
        }
    }
    return response;
}

bool isOkResponse(std::string_view response) {
    return response.find("\r\nOK\r\n") != std::string_view::npos;
}
