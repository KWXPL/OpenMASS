#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <string_view>

enum class GnssState { disabled, waitingForFix, fixed };

struct GnssInfo {
    GnssState state{GnssState::disabled};
    std::string utc;
    std::optional<double> latitude;
    std::optional<double> longitude;
    std::optional<double> mslAltitude;
    std::optional<double> speedOverGround;
    std::optional<double> courseOverGround;
    std::optional<std::uint32_t> fixMode;
    std::string reserved1;
    std::optional<double> hdop;
    std::optional<double> pdop;
    std::optional<double> vdop;
    std::string reserved2;
    std::optional<std::uint32_t> gpsSatellitesInView;
    std::optional<std::uint32_t> gnssSatellitesUsed;
    std::optional<std::uint32_t> glonassSatellitesInView;
    std::string reserved3;
    std::optional<std::uint32_t> cn0Max;
    std::optional<double> horizontalPositionAccuracy;
    std::optional<double> verticalPositionAccuracy;
};

std::optional<GnssInfo> parseGnssInfo(std::string_view response);
