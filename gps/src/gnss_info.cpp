#include "gnss_info.h"

#include <cctype>
#include <charconv>
#include <vector>

namespace {

std::vector<std::string_view> split(std::string_view value, char delimiter) {
    std::vector<std::string_view> fields;
    while (true) {
        const auto position = value.find(delimiter);
        fields.push_back(value.substr(0, position));
        if (position == std::string_view::npos) {
            return fields;
        }
        value.remove_prefix(position + 1);
    }
}

std::string_view trim(std::string_view value) {
    while (!value.empty() && std::isspace(static_cast<unsigned char>(value.front())) != 0) {
        value.remove_prefix(1);
    }
    while (!value.empty() && std::isspace(static_cast<unsigned char>(value.back())) != 0) {
        value.remove_suffix(1);
    }
    return value;
}

std::string_view fieldView(const std::vector<std::string_view>& fields, std::size_t index) {
    return index < fields.size() ? trim(fields[index]) : std::string_view{};
}

std::string field(const std::vector<std::string_view>& fields, std::size_t index) {
    return std::string(fieldView(fields, index));
}

template <typename T>
std::optional<T> numericField(const std::vector<std::string_view>& fields, std::size_t index) {
    const auto valueText = fieldView(fields, index);
    if (valueText.empty()) {
        return std::nullopt;
    }

    T value{};
    const auto [end, error] = std::from_chars(valueText.begin(), valueText.end(), value);
    if (error != std::errc{} || end != valueText.end()) {
        return std::nullopt;
    }
    return value;
}

} // namespace

std::optional<GnssInfo> parseGnssInfo(std::string_view response) {
    constexpr std::string_view prefix = "+CGNSINF: ";
    const auto start = response.find(prefix);
    if (start == std::string_view::npos) {
        return std::nullopt;
    }
    const auto dataStart = start + prefix.size();
    const auto end = response.find_first_of("\r\n", dataStart);
    const auto fields = split(response.substr(dataStart, end - dataStart), ',');
    if (fields.size() < 2) {
        return std::nullopt;
    }

    GnssInfo info;
    if (fields[0] != "1") {
        info.state = GnssState::disabled;
    } else {
        info.state = fields[1] == "1" ? GnssState::fixed : GnssState::waitingForFix;
    }

    info.utc = field(fields, 2);
    info.latitude = numericField<double>(fields, 3);
    info.longitude = numericField<double>(fields, 4);
    info.mslAltitude = numericField<double>(fields, 5);
    info.speedOverGround = numericField<double>(fields, 6);
    info.courseOverGround = numericField<double>(fields, 7);
    info.fixMode = numericField<std::uint32_t>(fields, 8);
    info.reserved1 = field(fields, 9);
    info.hdop = numericField<double>(fields, 10);
    info.pdop = numericField<double>(fields, 11);
    info.vdop = numericField<double>(fields, 12);
    info.reserved2 = field(fields, 13);
    info.gpsSatellitesInView = numericField<std::uint32_t>(fields, 14);
    info.gnssSatellitesUsed = numericField<std::uint32_t>(fields, 15);
    info.glonassSatellitesInView = numericField<std::uint32_t>(fields, 16);
    info.reserved3 = field(fields, 17);
    info.cn0Max = numericField<std::uint32_t>(fields, 18);
    info.horizontalPositionAccuracy = numericField<double>(fields, 19);
    info.verticalPositionAccuracy = numericField<double>(fields, 20);
    return info;
}
