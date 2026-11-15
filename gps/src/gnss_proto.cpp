#include "gnss_proto.h"

#include "gnss_info.h"

#include <bit>
#include <cstdint>
#include <ostream>
#include <stdexcept>
#include <string_view>

namespace {

void appendVarint(std::string& output, std::uint64_t value) {
    while (value >= 0x80U) {
        output.push_back(static_cast<char>((value & 0x7fU) | 0x80U));
        value >>= 7U;
    }
    output.push_back(static_cast<char>(value));
}

void appendBool(std::string& output, std::uint32_t number, bool value) {
    appendVarint(output, static_cast<std::uint64_t>(number) << 3U);
    appendVarint(output, value ? 1U : 0U);
}

void appendUInt32(std::string& output, std::uint32_t number,
                  const std::optional<std::uint32_t>& value) {
    if (!value) {
        return;
    }
    appendVarint(output, static_cast<std::uint64_t>(number) << 3U);
    appendVarint(output, *value);
}

void appendDouble(std::string& output, std::uint32_t number, const std::optional<double>& value) {
    if (!value) {
        return;
    }
    appendVarint(output, (static_cast<std::uint64_t>(number) << 3U) | 1U);
    const auto bits = std::bit_cast<std::uint64_t>(*value);
    for (unsigned shift = 0; shift < 64; shift += 8) {
        output.push_back(static_cast<char>((bits >> shift) & 0xffU));
    }
}

void appendString(std::string& output, std::uint32_t number, std::string_view value) {
    if (value.empty()) {
        return;
    }
    appendVarint(output, (static_cast<std::uint64_t>(number) << 3U) | 2U);
    appendVarint(output, value.size());
    output.append(value);
}

} // namespace

std::string serializeGnssProto(const GnssInfo& info) {
    std::string output;
    appendVarint(output, 1U << 3U);
    appendVarint(output, 2U);
    appendBool(output, 2, info.state != GnssState::disabled);
    appendBool(output, 3, info.state == GnssState::fixed);
    appendString(output, 4, info.utc);
    appendDouble(output, 5, info.latitude);
    appendDouble(output, 6, info.longitude);
    appendDouble(output, 7, info.mslAltitude);
    appendDouble(output, 8, info.speedOverGround);
    appendDouble(output, 9, info.courseOverGround);
    appendUInt32(output, 10, info.fixMode);
    appendString(output, 11, info.reserved1);
    appendDouble(output, 12, info.hdop);
    appendDouble(output, 13, info.pdop);
    appendDouble(output, 14, info.vdop);
    appendString(output, 15, info.reserved2);
    appendUInt32(output, 16, info.gpsSatellitesInView);
    appendUInt32(output, 17, info.gnssSatellitesUsed);
    appendUInt32(output, 18, info.glonassSatellitesInView);
    appendString(output, 19, info.reserved3);
    appendUInt32(output, 20, info.cn0Max);
    appendDouble(output, 21, info.horizontalPositionAccuracy);
    appendDouble(output, 22, info.verticalPositionAccuracy);
    return output;
}

void writeDelimitedGnssProto(std::ostream& output, const GnssInfo& info) {
    const std::string message = serializeGnssProto(info);
    std::string length;
    appendVarint(length, message.size());
    output.write(length.data(), static_cast<std::streamsize>(length.size()));
    output.write(message.data(), static_cast<std::streamsize>(message.size()));
    output.flush();
    if (!output) {
        throw std::runtime_error("Failed to write the protobuf GNSS message.");
    }
}
