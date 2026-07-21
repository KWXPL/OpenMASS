#pragma once

#include <iosfwd>
#include <string>

struct GnssInfo;

// Serializes a GnssInfo according to gps/proto/gnss.proto.
std::string serializeGnssProto(const GnssInfo& info);

// Writes one protobuf message using the standard varint length-delimited framing.
void writeDelimitedGnssProto(std::ostream& output, const GnssInfo& info);
