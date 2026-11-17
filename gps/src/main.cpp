#include "at_command_client.h"
#include "gnss_info.h"
#include "gnss_proto.h"
#include "serial_port.h"

#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

#include <chrono>
#include <csignal>
#include <exception>
#include <filesystem>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <thread>
#include <unistd.h>

namespace {

volatile std::sig_atomic_t running = 1;

void stop(int) { running = 0; }

std::shared_ptr<spdlog::logger> initLogging() {
    const auto executable = std::filesystem::read_symlink("/proc/self/exe");
    const auto logDirectory = executable.parent_path() / "MASSlogs";
    std::filesystem::create_directories(logDirectory);

    auto console = std::make_shared<spdlog::sinks::stderr_color_sink_mt>();
    auto file = std::make_shared<spdlog::sinks::basic_file_sink_mt>(
        (logDirectory / "openmass_gps_status.log").string(), false);
    auto logger = std::make_shared<spdlog::logger>(
        "openmass", spdlog::sinks_init_list{console, file});
    logger->flush_on(spdlog::level::info);
    spdlog::set_default_logger(std::move(logger));

    auto gpsLogger = spdlog::basic_logger_mt(
        "gps_data", (logDirectory / "openmass_gps_data.log").string());
    gpsLogger->flush_on(spdlog::level::info);
    return gpsLogger;
}

void transmitPosition(const std::string& response, bool writeToStdout,
                      spdlog::logger& gpsLogger) {
    const auto start = response.find("+CGNSINF:");
    if (start != std::string::npos) {
        const auto end = response.find_first_of("\r\n", start);
        gpsLogger.info("{}", response.substr(start, end - start));
    }
    const auto info = parseGnssInfo(response);
    if (!info) {
        spdlog::warn("No +CGNSINF response from the module.");
        return;
    }
    if (writeToStdout) {
        writeDelimitedGnssProto(std::cout, *info);
    }
}

} // namespace

int main(int argc, char* argv[]) {
    std::shared_ptr<spdlog::logger> gpsLogger;
    try {
        gpsLogger = initLogging();
    } catch (const std::exception& error) {
        std::cerr << "Failed to initialize logging: " << error.what() << '\n';
        return 1;
    }

    const std::string device = argc > 1 ? argv[1] : "/dev/serial0";
    int intervalSeconds = 1;
    if (argc > 2) {
        try {
            intervalSeconds = std::stoi(argv[2]);
        } catch (const std::exception&) {
            spdlog::error("Usage: {} [device] [interval-seconds]", argv[0]);
            return 2;
        }
        if (intervalSeconds < 1) {
            spdlog::error("The interval must be at least 1 second.");
            return 2;
        }
    }

    std::signal(SIGINT, stop);
    std::signal(SIGTERM, stop);

    try {
        SerialPort serial(device);
        AtCommandClient modem(serial);
        spdlog::info("Connected to {} (115200 8N1).", device);

        if (!isOkResponse(modem.command("AT", std::chrono::seconds(2)))) {
            throw std::runtime_error(
                "The module does not respond to AT. Check the power supply, UART, and device.");
        }
        if (!isOkResponse(modem.command("AT+CGNSPWR=1", std::chrono::seconds(3)))) {
            throw std::runtime_error("Failed to enable GNSS with the AT+CGNSPWR=1 command.");
        }

        const bool writeToStdout = ::isatty(STDOUT_FILENO) == 0;
        if (writeToStdout) {
            spdlog::info("GNSS enabled. Streaming length-delimited protobuf to stdout.");
        } else {
            spdlog::info("GNSS enabled. Binary output disabled because stdout is a terminal.");
        }
        while (running != 0) {
            transmitPosition(modem.command("AT+CGNSINF", std::chrono::seconds(2)), writeToStdout,
                             *gpsLogger);
            for (int i = 0; i < intervalSeconds * 10 && running != 0; ++i) {
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
            }
        }
    } catch (const std::exception& error) {
        spdlog::error("Error: {}", error.what());
        return 1;
    }

    return 0;
}
