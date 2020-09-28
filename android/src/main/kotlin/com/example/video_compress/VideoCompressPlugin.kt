package com.example.video_compress

import android.content.Context
import android.content.Intent
import android.net.Uri
import android.util.Log
import com.otaliastudios.transcoder.Transcoder
import com.otaliastudios.transcoder.TranscoderListener
import com.otaliastudios.transcoder.strategy.DefaultVideoStrategies
import com.otaliastudios.transcoder.strategy.DefaultVideoStrategy
import com.otaliastudios.transcoder.strategy.TrackStrategy
import com.otaliastudios.transcoder.strategy.size.*
import io.flutter.embedding.engine.plugins.FlutterPlugin
import io.flutter.plugin.common.BinaryMessenger
import io.flutter.plugin.common.MethodCall
import io.flutter.plugin.common.MethodChannel
import io.flutter.plugin.common.MethodChannel.MethodCallHandler
import io.flutter.plugin.common.PluginRegistry.Registrar
import java.io.File
import java.text.SimpleDateFormat
import java.util.*


/**
 * VideoCompressPlugin
 */
class VideoCompressPlugin : MethodCallHandler, FlutterPlugin {

    private var _context: Context? = null
    private var _channel: MethodChannel? = null

    val channelName = "video_compress"
    override fun onMethodCall(call: MethodCall, result: MethodChannel.Result) {
        val context = _context;
        val channel = _channel;

        if (context == null || channel == null) {
            Log.w(TAG, "Calling VideoCompress plugin before initialization")
            return
        }

        when (call.method) {
            "getByteThumbnail" -> {
                val path = call.argument<String>("path")
                val quality = call.argument<Int>("quality")!!
                val position = call.argument<Int>("position")!! // to long
                ThumbnailUtility(channelName).getByteThumbnail(path!!, quality, position.toLong(), result)
            }
            "getFileThumbnail" -> {
                val path = call.argument<String>("path")
                val quality = call.argument<Int>("quality")!!
                val position = call.argument<Int>("position")!! // to long
                ThumbnailUtility("video_compress").getFileThumbnail(context, path!!, quality,
                        position.toLong(), result)
            }
            "getMediaInfo" -> {
                val path = call.argument<String>("path")
                result.success(Utility(channelName).getMediaInfoJson(context, path!!).toString())
            }
            "deleteAllCache" -> {
                result.success(Utility(channelName).deleteAllCache(context, result));
            }
            "cancelCompression" -> {
                result.success(false);
                //TODO: Made Transcoder.into Global to call Transcoder.cancel(true); here
            }
            "compressVideo" -> {
                val path = call.argument<String>("path")!!
                val quality = call.argument<Int>("quality")!!
                val deleteOrigin = call.argument<Boolean>("deleteOrigin")!!
                val startTime = call.argument<Int>("startTime")
                val duration = call.argument<Int>("duration")
                val includeAudio = call.argument<Boolean>("includeAudio")
                val frameRate = if (call.argument<Int>("frameRate")==null) 30 else call.argument<Int>("frameRate")

                val tempDir: String = context.getExternalFilesDir("video_compress")!!.absolutePath
                val out = SimpleDateFormat("yyyy-MM-dd hh-mm-ss").format(Date())
                val destPath: String = tempDir + File.separator + "VID_" + out + ".mp4"

                var strategy: TrackStrategy = DefaultVideoStrategy.atMost(340).build();

                when (quality) {

                    0 -> {
                      strategy = DefaultVideoStrategy.atMost(720).build()
                    }

                    1 -> {
                        strategy = DefaultVideoStrategy.atMost(360).build()
                    }
                    2 -> {
                        strategy = DefaultVideoStrategy.atMost(640).build()
                    }
                    3 -> {

                        assert(value = frameRate != null)
                        strategy = DefaultVideoStrategy.Builder()
                                .keyFrameInterval(3f)
                                .bitRate(1280 * 720 * 4.toLong())
                                .frameRate(frameRate!!) // will be capped to the input frameRate
                                .build()
                    }
                }


                Transcoder.into(destPath!!)
                        .addDataSource(context, Uri.parse(path))
                        .setVideoTrackStrategy(strategy)
                        .setListener(object : TranscoderListener {
                            override fun onTranscodeProgress(progress: Double) {
                                channel.invokeMethod("updateProgress", progress * 100.00)
                            }
                            override fun onTranscodeCompleted(successCode: Int) {
                                channel.invokeMethod("updateProgress", 100.00)
                                val json = Utility(channelName).getMediaInfoJson(context, destPath)
                                json.put("isCancel", false)
                                result.success(json.toString())
                                if (deleteOrigin) {
                                    File(path).delete()
                                }
                            }

                            override fun onTranscodeCanceled() {
                                result.success(null)
                            }

                            override fun onTranscodeFailed(exception: Throwable) {
                                result.success(null)
                            }
                        }).transcode()
            }
            else -> {
                result.notImplemented()
            }
        }
    }

    override fun onAttachedToEngine(binding: FlutterPlugin.FlutterPluginBinding) {
        init(binding.applicationContext, binding.binaryMessenger)
    }

    override fun onDetachedFromEngine(binding: FlutterPlugin.FlutterPluginBinding) {
        _channel?.setMethodCallHandler(null)
        _context = null
        _channel = null
    }

    private fun init(context: Context, messenger: BinaryMessenger) {
        val channel = MethodChannel(messenger, channelName)
        channel.setMethodCallHandler(this)
        _context = context
        _channel = channel
    }

    companion object {
        private const val TAG = "video_compress"

        @JvmStatic
        fun registerWith(registrar: Registrar) {
            val instance = VideoCompressPlugin()
            instance.init(registrar.context(), registrar.messenger())
        }
    }

}
