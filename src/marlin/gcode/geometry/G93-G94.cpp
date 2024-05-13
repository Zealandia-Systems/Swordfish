#include <swordfish/modules/motion/FeedRate.h>

using namespace swordfish::motion;

#include "../gcode.h"

void GcodeSuite::G93() {
	auto old_feedrate_type = parser.feedrate_type;

	parser.set_feed_rate_type(FeedRateType::InverseTime);

	if (parser.chain()) {
		process_parsed_command(true);

		parser.set_feed_rate_type(old_feedrate_type);
	}
}

void GcodeSuite::G94() {
	auto old_feedrate_type = parser.feedrate_type;

	parser.set_feed_rate_type(FeedRateType::MillimetersPerSecond);

	if (parser.chain()) {
		process_parsed_command(true);

		parser.set_feed_rate_type(old_feedrate_type);
	}
}