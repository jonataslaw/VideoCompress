#include "include/video_compress/video_compress_plugin.h"

#include <atomic>
#include <fcntl.h>
#include <flutter_linux/flutter_linux.h>
#include <gtk/gtk.h>
#include <sstream>
#include <sys/utsname.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <spawn.h>
#include <unistd.h>
#include <vector>

#include "video_compress_plugin_private.h"

#define VIDEO_COMPRESS_PLUGIN(obj) \
	(G_TYPE_CHECK_INSTANCE_CAST((obj), video_compress_plugin_get_type(), VideoCompressPlugin))

struct _VideoCompressPlugin {
	GObject parent_instance;
};

G_DEFINE_TYPE(VideoCompressPlugin, video_compress_plugin, g_object_get_type())

extern char **environ;

/** Set to -1 when no compression is ongoing.
 *  Set to 0 when compression is ongoing, but ffmpeg isn't live.
 *  Set to the pid of ffmpeg otherwise.
 */
static std::atomic<pid_t> ffmpeg_pid = -1;

static std::string get_output_dir(void) {
	std::stringstream ss;
	ss << "/tmp/video_compress." << getpid();
	return ss.str();
}

static std::string new_output_filename(void) {
	std::stringstream ss;
	ss << "/tmp/video_compress." << getpid() << "/XXXXXX.mp4";
	return ss.str();
}

static FlMethodResponse *get_method_args(FlMethodCall *method_call, ...) {
	FlValue *method_args = fl_method_call_get_args(method_call);
	va_list args;
	va_start(args, method_call);

	GString *error = g_string_new(nullptr);
	for (;;) {
		const char *key = va_arg(args, const char *);
		if (key == nullptr) {
			break;
		}
		printf("KEY: %s\n", key);

		FlValue *value = fl_value_lookup_string(method_args, key);
		if (value == nullptr) {
			// Skip over to the next key.
			FlValueType value_type = va_arg(args, FlValueType);
			if (value_type == FL_VALUE_TYPE_BOOL) {
				va_arg(args, bool *);
			} else if (value_type == FL_VALUE_TYPE_INT) {
				va_arg(args, int *);
			} else if (value_type == FL_VALUE_TYPE_INT) {
				va_arg(args, const char **);
			} else {
				abort();
			}
			continue;
		}

		FlValueType value_type = va_arg(args, FlValueType);
		if (value_type == FL_VALUE_TYPE_BOOL) {
			FlValueType actual = fl_value_get_type(value);
			bool *out = va_arg(args, bool *);
			if (actual == FL_VALUE_TYPE_BOOL) {
				*out = fl_value_get_bool(value);
			} else if (actual != FL_VALUE_TYPE_NULL) {
				g_string_printf(error, "expected '%s' to be bool", key);
				break;
			}
		} else if (value_type == FL_VALUE_TYPE_INT) {
			FlValueType actual = fl_value_get_type(value);
			int64_t *out = va_arg(args, int64_t *);
			if (actual == FL_VALUE_TYPE_INT) {
				*out = fl_value_get_int(value);
			} else if (actual != FL_VALUE_TYPE_NULL) {
				g_string_printf(error, "expected '%s' to be int, got %d", key, fl_value_get_type(value));
				break;
			}
		} else if (value_type == FL_VALUE_TYPE_STRING) {
			FlValueType actual = fl_value_get_type(value);
			const char **out = va_arg(args, const char **);
			if (actual == FL_VALUE_TYPE_STRING) {
				*out = fl_value_get_string(value);
			} else if (actual != FL_VALUE_TYPE_NULL) {
				g_string_printf(error, "expected '%s' to be string", key);
				break;
			}
		} else {
			abort();
		}
	}

	va_end(args);

	FlMethodResponse *response = nullptr;

	if (error->len != 0) {
		response = FL_METHOD_RESPONSE(fl_method_error_response_new("invalid arguments", error->str, nullptr));
	}

	g_string_free(error, true);
	return response;
}

// Called when a method call is received from Flutter.
static void video_compress_plugin_handle_method_call(
	VideoCompressPlugin *self,
	FlMethodCall *method_call
) {
	g_autoptr(FlMethodResponse) response = nullptr;

	const char *method = fl_method_call_get_name(method_call);

	g_warning("CALLINGM ETHOD: %s", method);

	if (strcmp(method, "getPlatformVersion") == 0) {
		response = get_platform_version();
	} else if (strcmp(method, "cancelCompression") == 0) {
		response = cancel_compression();
	} else if (strcmp(method, "compressVideo") == 0) {
		const char *path = nullptr;
		int64_t quality = 0;
		bool delete_origin = false;
		int64_t start_time_s = -1;
		int64_t duration_s = -1;
		bool include_audio = true;
		int64_t frame_rate = 30;
		response = get_method_args(method_call,
			"path", FL_VALUE_TYPE_STRING, &path,
			"quality", FL_VALUE_TYPE_INT, &quality,
			"delete_origin", FL_VALUE_TYPE_BOOL, &delete_origin,
			"start_time", FL_VALUE_TYPE_INT, &start_time_s,
			"duration", FL_VALUE_TYPE_INT, &duration_s,
			"include_audio", FL_VALUE_TYPE_BOOL, &include_audio,
			"frame_rate", FL_VALUE_TYPE_INT, &frame_rate,
			nullptr
		);
		if (response == nullptr) {
			response = compress_video(
				path,
				quality,
				delete_origin,
				start_time_s,
				duration_s,
				include_audio,
				frame_rate
			);
		}
	} else if (strcmp(method, "deleteAllCache") == 0) {
		response = delete_all_cache();
	} else if (strcmp(method, "setLogLevel") == 0) {
		response = set_log_level();
	} else {
		response = FL_METHOD_RESPONSE(fl_method_not_implemented_response_new());
	}

	g_autoptr(GError) error = nullptr;
	if (!fl_method_call_respond(method_call, response, &error)) {
		g_warning("Failed to send response: %s", error->message);
	}
}

FlMethodResponse *get_platform_version(void) {
	struct utsname uname_data = {};
	uname(&uname_data);
	g_autofree gchar *version = g_strdup_printf("Linux %s", uname_data.version);
	g_autoptr(FlValue) result = fl_value_new_string(version);
	return FL_METHOD_RESPONSE(fl_method_success_response_new(result));
}

FlMethodResponse *cancel_compression(void) {
	pid_t pid = 0;
	// Try to stop the compression before ffmpeg is spawned.
	bool stopped = ffmpeg_pid.compare_exchange_strong(
		pid,
		-1,
		std::memory_order_seq_cst,
		std::memory_order_seq_cst
	);
	if (stopped || pid == -1) {
		// We stopped the compression, or no compression was ongoing.
		return FL_METHOD_RESPONSE(fl_method_success_response_new(nullptr));
	}

	if (kill(pid, SIGTERM) == 0) {
		// we managed to kill ffmpeg :D (ffmpeg didn't die before
		// compress_video had the time to reset ffmpeg_pid)
		int status = 0;
		TEMP_FAILURE_RETRY(waitpid(pid, &status, 0));
	}

	// Reset ffmpeg_pid... unless another call to cancel_compression has
	// already done it, and then a compress_video call started which thus
	// reinitialized ffmpeg_pid with another pid. In any case the
	// cancellation is a success.
	ffmpeg_pid.compare_exchange_strong(
		pid,
		-1,
		std::memory_order_seq_cst,
		std::memory_order_seq_cst
	);

	return FL_METHOD_RESPONSE(fl_method_success_response_new(nullptr));
}

FlMethodResponse *compress_video(
	const char *path,
	int64_t quality,
	bool delete_origin,
	int64_t start_time_s,
	int64_t duration_s,
	bool include_audio,
	int64_t frame_rate
) {
	g_warning("path=%s quality=%ld delete_origin=%d %ld %ld %d %ld", path, quality, delete_origin, start_time_s, duration_s, include_audio, frame_rate);

	// We can only start a compression job if ffmpeg_pid is -1.
	pid_t pid = -1;
	bool can_start = ffmpeg_pid.compare_exchange_strong(
		pid,
		0,
		std::memory_order_seq_cst,
		std::memory_order_seq_cst
	);
	if (!can_start) {
		return FL_METHOD_RESPONSE(fl_method_error_response_new("already compressing", nullptr, nullptr));
	}

	{
		// Put this in a scope so that "dir" gets freed early.
		std::string dir = get_output_dir();
		if (mkdir(dir.c_str(), 0700) < 0 && errno != EEXIST) {
			pid_t zero = 0;
			ffmpeg_pid.compare_exchange_strong(
				zero,
				-1,
				std::memory_order_seq_cst,
				std::memory_order_seq_cst
			);
			return FL_METHOD_RESPONSE(fl_method_error_response_new("failed to create output file", strerror(errno), nullptr));
		}
	}

	std::string output_filename = new_output_filename();
	int output_fd = mkstemps(output_filename.data(), 4); // 4 == len(".mp4")
	if (output_fd < 0) {
		pid_t zero = 0;
		ffmpeg_pid.compare_exchange_strong(
			zero,
			-1,
			std::memory_order_seq_cst,
			std::memory_order_seq_cst
		);
		return FL_METHOD_RESPONSE(fl_method_error_response_new("failed to create output file", nullptr, nullptr));
	}

	// Don't bother unlinking it, ffmpeg will overwrite it.
	close(output_fd);

	std::vector<const char *> argv({
		"ffmpeg",
		"-i", path,
		"-y", // overwrite the file created by mkstemps
		"-c:v", "libx264", // most popular AVC encoder
		"-preset", "veryfast", // it's already slow enough
	});

	if (include_audio) {
		// use the built-in AAC encoder
		argv.push_back("-c:a");
		argv.push_back("aac");

		// lowest bitrate in the "recommended range"
		// https://trac.ffmpeg.org/wiki/Encode/HighQualityAudio#Recommended
		argv.push_back("-b:a");
		argv.push_back("128k");
	} else {
		argv.push_back("-an");
	}

	std::stringstream ss;
	std::string fps_filter;
	std::string start_time_str;
	std::string duration_str;

	argv.push_back("-vf");
	ss << "fps=min(source_fps\\," << frame_rate << ")";
	fps_filter = ss.str();
	argv.push_back(fps_filter.c_str());

	if (start_time_s > 0) {
		argv.push_back("-ss");
		ss.str("");
		ss.clear();
		ss << start_time_s;
		start_time_str = ss.str();
		argv.push_back(start_time_str.c_str());
	}

	if (duration_s > 0) {
		argv.push_back("-t");
		ss.str("");
		ss.clear();
		ss << start_time_s;
		duration_str = ss.str();
		argv.push_back(duration_str.c_str());
	}

	// Notes on scaling:
	// see https://ffmpeg.org/ffmpeg-filters.html#scale-1 for details.
	//
	// "iw" is the input width, "ow" is the output width, "a" is the input
	// aspect ratio (iw/ih).
	//
	// For when we want to scale down to 480p, 720p or 1080p but we don't
	// care about the other dimension (as long as aspect ratio is kept), we
	// set the width to "min(iw,max(480,480*a))": the "min" is so that we
	// don't upscale, and the "max" is so that if the video is vertical
	// (iw<ih, i.e. a<1) we set the width to 480. If the video is horizontal
	// (iw>ih, i.e. a>1), we set the width to 480*a, so the height is 480
	// (i.e. 480*a/a).
	//
	// For when we want to scale down to LxH with constraints on the two
	// dimensions, we separate the cases where "a" is smaller or larger than
	// the desired aspect ratio. If it's smaller, it means that the height
	// has to be constrained to TODO
	//
	// Also, "trunc" is used to ensure the height is even. libx264 doesn't
	// work with odd sizes.
	switch (quality) {
	case 1:
		// Low quality setting
		// 480p, constant rate factor of 30
		argv.push_back("-crf");
		argv.push_back("30");
		argv.push_back("-vf");
		argv.push_back("scale=w=min(iw\\,max(480\\,480*a)):h=trunc(ow/a/2)*2");
		break;
	case 3:
		// High quality setting
		// 1080p, constant rate factor of 22
		argv.push_back("-crf");
		argv.push_back("22");
		argv.push_back("-vf");
		argv.push_back("scale=w=min(iw\\,max(1080\\,1080*a)):h=trunc(ow/a/2)*2");
		break;
	case 4:
		// 640x480 quality setting
		argv.push_back("-vf");
		argv.push_back("scale=w=if(lt(a\\,640/480)\\,min(iw\\,min(480\\,640*a))\\,min(iw\\,min(640\\,480*a))):h=trunc(ow/a/2)*2");
		break;
	case 5:
		// 960x540 quality setting
		argv.push_back("-vf");
		argv.push_back("scale=w=if(lt(a\\,960/540)\\,min(iw\\,min(540\\,960*a))\\,min(iw\\,min(960\\,540*a))):h=trunc(ow/a/2)*2");
		break;
	case 6:
		// 1280x720 quality setting
		argv.push_back("-vf");
		argv.push_back("scale=w=if(lt(a\\,1280/720)\\,min(iw\\,min(720\\,1280*a))\\,min(iw\\,min(1280\\,720*a))):h=trunc(ow/a/2)*2");
		break;
	case 7:
		// 1920x1080 quality setting
		argv.push_back("-vf");
		argv.push_back("scale=w=if(lt(a\\,1920/1080)\\,min(iw\\,min(1080\\,1920*a))\\,min(iw\\,min(1920\\,1080*a))):h=trunc(ow/a/2)*2");
		break;
	default:
		// Medium quality setting
		// 720p, constant rate factor of 26
		argv.push_back("-crf");
		argv.push_back("26");
		argv.push_back("-vf");
		argv.push_back("scale=w=min(iw\\,max(720\\,720*a)):h=trunc(ow/a/2)*2");
		break;
        }

	argv.push_back(output_filename.c_str());
	argv.push_back(nullptr);

	posix_spawn_file_actions_t actions;
	posix_spawn_file_actions_init(&actions);
	// TODO log file name
	posix_spawn_file_actions_addopen(&actions, 3, "/tmp/last_ffmpeg.log", O_CREAT | O_TRUNC | O_WRONLY, 0600);
	posix_spawn_file_actions_adddup2(&actions, 3, 2);
	posix_spawn_file_actions_adddup2(&actions, 3, 1);

	int r = posix_spawnp(
		&pid,
		"ffmpeg",
		&actions,
		nullptr,
		const_cast<char **>(&argv[0]),
		environ
	);

	posix_spawn_file_actions_destroy(&actions);

	if (r != 0) {
		pid_t zero = 0;
		ffmpeg_pid.compare_exchange_strong(
			zero,
			-1,
			std::memory_order_seq_cst,
			std::memory_order_seq_cst
		);
		return FL_METHOD_RESPONSE(fl_method_error_response_new("failed to spawn ffmpeg", nullptr, nullptr));
	}

	pid_t zero = 0;
	bool can_continue = ffmpeg_pid.compare_exchange_strong(
		zero,
		pid,
		std::memory_order_seq_cst,
		std::memory_order_seq_cst
	);
	if (!can_continue) {
		// We have been cancelled.
		if (kill(pid, SIGTERM) == 0) {
			int status = 0;
			TEMP_FAILURE_RETRY(waitpid(pid, &status, 0));
		}
		// TODO return isCancel=true
		return FL_METHOD_RESPONSE(fl_method_success_response_new(nullptr));
	}

	int status = 0;
	TEMP_FAILURE_RETRY(waitpid(pid, &status, 0));

	ffmpeg_pid.compare_exchange_strong(
		pid,
		-1,
		std::memory_order_seq_cst,
		std::memory_order_seq_cst
	);

	if (status == 127) {
		// Special status returned by posix_spawn when the execve call fails.
		return FL_METHOD_RESPONSE(fl_method_error_response_new("failed to spawn ffmpeg", nullptr, nullptr));
	} else if (!WIFEXITED(status)) {
		return FL_METHOD_RESPONSE(fl_method_error_response_new("ffmpeg error", nullptr, nullptr));
	}

	if (delete_origin) {
		if (unlink(path) < 0 && errno != ENOENT) {
			return FL_METHOD_RESPONSE(fl_method_error_response_new("failed to delete origin", strerror(errno), nullptr));
		}
	}

	ss.str("");
	ss.clear();
	ss <<
		"{"
		"\"path\":\"" << output_filename << "\","
		"\"title\":\"\","
		"\"author\":\"\","
		"\"width\":0,"
		"\"height\":0,"
		"\"duration\":0,"
		"\"filesize\":0"
		"}";

	std::string media_info = ss.str();
	FlValue *media_info_v = fl_value_new_string(media_info.c_str());

	return FL_METHOD_RESPONSE(fl_method_success_response_new(media_info_v));
}

FlMethodResponse *delete_all_cache(void) {
	FlMethodResponse *response = cancel_compression();
	if (response != nullptr) {
		// Failed to cancel, don't bother deleting the directory.
		return response;
	}

	std::string output_dir = get_output_dir();
	DIR *d = opendir(output_dir.c_str());
	if (d == nullptr) {
		if (errno == ENOENT) {
			// Don't return an error if delete_all_cache is called
			// before any compression job has started.
			return FL_METHOD_RESPONSE(fl_method_success_response_new(nullptr));
		}
		return FL_METHOD_RESPONSE(fl_method_error_response_new("failed to remove directory", nullptr, nullptr));
	}

	for (;;) {
		struct dirent *entry = readdir(d);
		if (entry == nullptr) {
			break;
		}
		// If this fails, the rmdir call later will probably fail, so
		// don't bother handling errors here.
		unlink(entry->d_name);
	}

	closedir(d);

	if (rmdir(output_dir.c_str()) < 0 && errno != ENOENT) {
		return FL_METHOD_RESPONSE(fl_method_error_response_new("failed to remove directory", nullptr, nullptr));
	}

	return FL_METHOD_RESPONSE(fl_method_success_response_new(nullptr));
}

FlMethodResponse *set_log_level(void) {
	// TODO
	return FL_METHOD_RESPONSE(fl_method_success_response_new(nullptr));
}

static void video_compress_plugin_dispose(GObject* object) {
	G_OBJECT_CLASS(video_compress_plugin_parent_class)->dispose(object);
}

static void video_compress_plugin_class_init(VideoCompressPluginClass* klass) {
	G_OBJECT_CLASS(klass)->dispose = video_compress_plugin_dispose;
}

static void video_compress_plugin_init(VideoCompressPlugin* self) {}

static void method_call_cb(
	FlMethodChannel* channel,
	FlMethodCall* method_call,
	gpointer user_data
) {
	VideoCompressPlugin* plugin = VIDEO_COMPRESS_PLUGIN(user_data);
	video_compress_plugin_handle_method_call(plugin, method_call);
}

void video_compress_plugin_register_with_registrar(FlPluginRegistrar* registrar) {
	VideoCompressPlugin* plugin = VIDEO_COMPRESS_PLUGIN(
		g_object_new(video_compress_plugin_get_type(), nullptr));

	g_autoptr(FlStandardMethodCodec) codec = fl_standard_method_codec_new();
	g_autoptr(FlMethodChannel) channel = fl_method_channel_new(
		fl_plugin_registrar_get_messenger(registrar),
		"video_compress",
		FL_METHOD_CODEC(codec)
	);
	fl_method_channel_set_method_call_handler(
		channel,
		method_call_cb,
		g_object_ref(plugin),
		g_object_unref
	);

	g_object_unref(plugin);
}
