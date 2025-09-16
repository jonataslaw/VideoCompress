#include <flutter_linux/flutter_linux.h>

// This file exposes some plugin internals for unit testing. See
// https://github.com/flutter/flutter/issues/88724 for current limitations
// in the unit-testable API.

FlMethodResponse *get_platform_version(void);
FlMethodResponse *cancel_compression(void);
FlMethodResponse *compress_video(
	const gchar *path,
	int64_t quality,
	bool delete_origin,
	int64_t start_time,
	int64_t duration,
	bool include_audio,
	int64_t frame_rate
);
FlMethodResponse *delete_all_cache(void);
FlMethodResponse *set_log_level(void);
