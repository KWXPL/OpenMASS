#include "gnss_info.h"
#include "gnss_proto.h"

#include <bit>
#include <cassert>
#include <cstdint>
#include <sstream>
#include <string>
#include <unordered_map>

namespace {

std::uint64_t readVarint(const std::string& bytes, std::size_t& position) {
    std::uint64_t value = 0;
    unsigned shift = 0;
    while (position < bytes.size()) {
        const auto byte = static_cast<std::uint8_t>(bytes[position++]);
        value |= static_cast<std::uint64_t>(byte & 0x7fU) << shift;
        if ((byte & 0x80U) == 0) {
            return value;
        }
        shift += 7;
    }
    assert(false && "truncated varint");
    return 0;
}

double readDouble(const std::string& bytes, std::size_t& position) {
    assert(position + 8 <= bytes.size());
    std::uint64_t bits = 0;
    for (unsigned shift = 0; shift < 64; shift += 8) {
        bits |= static_cast<std::uint64_t>(static_cast<std::uint8_t>(bytes[position++])) << shift;
    }
    return std::bit_cast<double>(bits);
}

} // namespace

int main() {
    const std::string response = "\r\n+CGNSINF: "
                                 "1,1,20260721123456.000,54.352000,18.646600,12.300,4.500,182.000,"
                                 "2,R1,0.800,1.100,0.900,R2,10,7,5,R3,42,1.200,2.300\r\nOK\r\n";

    const auto info = parseGnssInfo(response);
    assert(info);
    assert(info->state == GnssState::fixed);
    assert(info->utc == "20260721123456.000");
    assert(info->latitude == 54.352);
    assert(info->longitude == 18.6466);
    assert(info->mslAltitude == 12.3);
    assert(info->speedOverGround == 4.5);
    assert(info->courseOverGround == 182.0);
    assert(info->fixMode == 2);
    assert(info->reserved1 == "R1");
    assert(info->hdop == 0.8);
    assert(info->pdop == 1.1);
    assert(info->vdop == 0.9);
    assert(info->reserved2 == "R2");
    assert(info->gpsSatellitesInView == 10);
    assert(info->gnssSatellitesUsed == 7);
    assert(info->glonassSatellitesInView == 5);
    assert(info->reserved3 == "R3");
    assert(info->cn0Max == 42);
    assert(info->horizontalPositionAccuracy == 1.2);
    assert(info->verticalPositionAccuracy == 2.3);

    const std::string message = serializeGnssProto(*info);
    std::unordered_map<std::uint32_t, std::string> strings;
    std::unordered_map<std::uint32_t, std::uint64_t> integers;
    std::unordered_map<std::uint32_t, double> doubles;
    std::size_t position = 0;
    while (position < message.size()) {
        const std::uint64_t key = readVarint(message, position);
        const auto field = static_cast<std::uint32_t>(key >> 3U);
        const auto wireType = static_cast<unsigned>(key & 7U);
        if (wireType == 0) {
            integers[field] = readVarint(message, position);
        } else if (wireType == 1) {
            doubles[field] = readDouble(message, position);
        } else {
            assert(wireType == 2);
            const auto size = static_cast<std::size_t>(readVarint(message, position));
            strings[field] = message.substr(position, size);
            position += size;
        }
    }

    assert(integers.at(1) == 2);
    assert(integers.at(2) == 1);
    assert(integers.at(3) == 1);
    assert(strings.at(4) == info->utc);
    assert(doubles.at(5) == *info->latitude);
    assert(doubles.at(6) == *info->longitude);
    assert(doubles.at(7) == *info->mslAltitude);
    assert(doubles.at(8) == *info->speedOverGround);
    assert(doubles.at(9) == *info->courseOverGround);
    assert(integers.at(10) == *info->fixMode);
    assert(strings.at(11) == info->reserved1);
    assert(doubles.at(12) == *info->hdop);
    assert(doubles.at(13) == *info->pdop);
    assert(doubles.at(14) == *info->vdop);
    assert(strings.at(15) == info->reserved2);
    assert(integers.at(16) == *info->gpsSatellitesInView);
    assert(integers.at(17) == *info->gnssSatellitesUsed);
    assert(integers.at(18) == *info->glonassSatellitesInView);
    assert(strings.at(19) == info->reserved3);
    assert(integers.at(20) == *info->cn0Max);
    assert(doubles.at(21) == *info->horizontalPositionAccuracy);
    assert(doubles.at(22) == *info->verticalPositionAccuracy);

    std::ostringstream framed;
    writeDelimitedGnssProto(framed, *info);
    const std::string stream = framed.str();
    position = 0;
    assert(readVarint(stream, position) == message.size());
    assert(stream.substr(position) == message);

    const auto waiting = parseGnssInfo("\r\n+CGNSINF: 1,0,,,,,,,,,,,,,4,0,2,,,,\r\nOK\r\n");
    assert(waiting);
    assert(waiting->state == GnssState::waitingForFix);
    assert(!waiting->latitude);
    assert(waiting->gpsSatellitesInView == 4);
    assert(waiting->gnssSatellitesUsed == 0);
    assert(waiting->glonassSatellitesInView == 2);

    const auto disabled =
        parseGnssInfo("\r\n+CGNSINF: 0,0,20260721123457.000,54.0,,,,,,,,,,,,,,,,,\r\n");
    assert(disabled);
    assert(disabled->state == GnssState::disabled);
    assert(disabled->utc == "20260721123457.000");
    assert(disabled->latitude == 54.0);
}
