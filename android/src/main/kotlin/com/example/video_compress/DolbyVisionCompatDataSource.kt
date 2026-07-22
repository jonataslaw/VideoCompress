package com.example.video_compress

import android.media.MediaFormat
import com.otaliastudios.transcoder.common.TrackType
import com.otaliastudios.transcoder.source.DataSource
import com.otaliastudios.transcoder.source.DataSourceWrapper

/**
 * Some devices have no registered decoder for "video/dolby-vision" (seen on Pixel devices
 * recording HDR video), which makes MediaCodec.createDecoderByType() throw NAME_NOT_FOUND
 * before transcoding can even start. Dolby Vision profile 8 content embeds a standard,
 * independently decodable HEVC base layer, so remapping the reported mime to video/hevc lets
 * a normal HEVC decoder read that base layer instead of failing outright.
 */
class DolbyVisionCompatDataSource(source: DataSource) : DataSourceWrapper(source) {
    override fun getTrackFormat(type: TrackType): MediaFormat? {
        val format = super.getTrackFormat(type)
        if (type == TrackType.VIDEO && format?.getString(MediaFormat.KEY_MIME) == "video/dolby-vision") {
            format.setString(MediaFormat.KEY_MIME, MediaFormat.MIMETYPE_VIDEO_HEVC)
        }
        return format
    }
}
