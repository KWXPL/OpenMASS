#include "at_command_client.h"
#include "gnss_info.h"
#include "gnss_proto.h"
#include "serial_port.h"

#include <chrono>
#include <csignal>
#include <exception>
#include <iostream>
#include <stdexcept>
#include <string>
#include <thread>

namespace {

volatile std::sig_atomic_t running = 1;

void stop(int) { running = 0; }

void transmitPosition(const std::string& response) {
    const auto info = parseGnssInfo(response);
    if (!info) {
        std::cerr << "No +CGNSINF response from the module.\n";
        return;
    }
    writeDelimitedGnssProto(std::cout, *info);
}

} // namespace

int main(int argc, char* argv[]) {
    const std::string device = argc > 1 ? argv[1] : "/dev/serial0";
    int intervalSeconds = 1;
    if (argc > 2) {
        try {
            intervalSeconds = std::stoi(argv[2]);
        } catch (const std::exception&) {
            std::cerr << "Usage: " << argv[0] << " [device] [interval-seconds]\n";
            return 2;
        }
        if (intervalSeconds < 1) {
            std::cerr << "The interval must be at least 1 second.\n";
            return 2;
        }
    }

    std::signal(SIGINT, stop);
    std::signal(SIGTERM, stop);

    try {
        SerialPort serial(device);
        AtCommandClient modem(serial);
        std::cerr << "Connected to " << device << " (115200 8N1).\n";

        if (!isOkResponse(modem.command("AT", std::chrono::seconds(2)))) {
            throw std::runtime_error(
                "The module does not respond to AT. Check the power supply, UART, and device.");
        }
        if (!isOkResponse(modem.command("AT+CGNSPWR=1", std::chrono::seconds(3)))) {
            throw std::runtime_error("Failed to enable GNSS with the AT+CGNSPWR=1 command.");
        }

        std::cerr << "GNSS enabled. Streaming length-delimited protobuf to stdout.\n";
        while (running != 0) {
            transmitPosition(modem.command("AT+CGNSINF", std::chrono::seconds(2)));
            for (int i = 0; i < intervalSeconds * 10 && running != 0; ++i) {
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
            }
        }
    } catch (const std::exception& error) {
        std::cerr << "Error: " << error.what() << '\n';
        return 1;
    }

    return 0;
}
