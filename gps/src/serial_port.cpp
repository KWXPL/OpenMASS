#include "serial_port.h"

#include <cerrno>
#include <cstring>
#include <fcntl.h>
#include <poll.h>
#include <stdexcept>
#include <termios.h>
#include <unistd.h>

namespace {

std::runtime_error systemError(const std::string& message) {
    return std::runtime_error(message + ": " + std::strerror(errno));
}

} // namespace

SerialPort::SerialPort(const std::string& device) {
    fd_ = ::open(device.c_str(), O_RDWR | O_NOCTTY | O_CLOEXEC);
    if (fd_ < 0) {
        throw systemError("Cannot open " + device);
    }

    termios tty{};
    if (::tcgetattr(fd_, &tty) != 0) {
        const auto error = systemError("Cannot read port settings");
        ::close(fd_);
        fd_ = -1;
        throw error;
    }

    ::cfmakeraw(&tty);
    ::cfsetispeed(&tty, B115200);
    ::cfsetospeed(&tty, B115200);
    tty.c_cflag |= CLOCAL | CREAD;
    tty.c_cflag &= ~CSTOPB;
    tty.c_cflag &= ~CRTSCTS;
    tty.c_cflag &= ~PARENB;
    tty.c_cflag = (tty.c_cflag & ~CSIZE) | CS8;
    tty.c_cc[VMIN] = 0;
    tty.c_cc[VTIME] = 0;

    if (::tcsetattr(fd_, TCSANOW, &tty) != 0) {
        const auto error = systemError("Cannot configure port");
        ::close(fd_);
        fd_ = -1;
        throw error;
    }
    if (::tcflush(fd_, TCIOFLUSH) != 0) {
        const auto error = systemError("Cannot flush port buffer");
        ::close(fd_);
        fd_ = -1;
        throw error;
    }
}

SerialPort::~SerialPort() {
    if (fd_ >= 0) {
        ::close(fd_);
    }
}

void SerialPort::flushInput() {
    if (::tcflush(fd_, TCIFLUSH) != 0) {
        throw systemError("Cannot flush UART input buffer");
    }
}

void SerialPort::writeAll(std::string_view data) {
    std::size_t written = 0;
    while (written < data.size()) {
        const ssize_t result = ::write(fd_, data.data() + written, data.size() - written);
        if (result < 0) {
            if (errno == EINTR) {
                continue;
            }
            throw systemError("Failed to write to UART");
        }
        if (result == 0) {
            throw std::runtime_error("No data was written to UART");
        }
        written += static_cast<std::size_t>(result);
    }
}

std::size_t SerialPort::readSome(char* buffer, std::size_t capacity,
                                 std::chrono::milliseconds timeout) {
    if (capacity == 0) {
        return 0;
    }
    const auto deadline = std::chrono::steady_clock::now() + timeout;
    while (true) {
        const auto now = std::chrono::steady_clock::now();
        if (now >= deadline) {
            return 0;
        }
        const auto remaining =
            std::chrono::duration_cast<std::chrono::milliseconds>(deadline - now);
        pollfd descriptor{fd_, POLLIN, 0};
        const int result = ::poll(&descriptor, 1, static_cast<int>(remaining.count()));
        if (result < 0) {
            if (errno == EINTR) {
                continue;
            }
            throw systemError("Failed while waiting for UART data");
        }
        if (result == 0) {
            return 0;
        }
        if ((descriptor.revents & (POLLERR | POLLHUP | POLLNVAL)) != 0) {
            throw std::runtime_error("UART port reported a connection error");
        }
        const ssize_t count = ::read(fd_, buffer, capacity);
        if (count < 0) {
            if (errno == EINTR) {
                continue;
            }
            throw systemError("Failed to read from UART");
        }
        return static_cast<std::size_t>(count);
    }
}
