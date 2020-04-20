package com.example.video_compress

import android.content.Context
import android.graphics.Bitmap
import io.flutter.plugin.common.MethodChannel
import java.io.ByteArrayOutputStream
import java.io.File
import java.io.IOException

class ThumbnailUtility(channelName: String) {
    private val utility = Utility(channelName)

    fun getByteThumbnail(path: String, quality: Int, position: Long, result: MethodChannel.Result) {
        val bmp = utility.getBitmap(path, position, result)

        val stream = ByteArrayOutputStream()
        bmp.compress(Bitmap.CompressFormat.JPEG, quality, stream)
        val byteArray = stream.toByteArray()
        bmp.recycle()
        result.success(byteArray.toList().toByteArray())
    }

    fun getFileThumbnail(context: Context, path: String, quality: Int, position: Long,
                             result: MethodChannel.Result) {
        val bmp = utility.getBitmap(path, position, result)

        val dir = context.getExternalFilesDir("video_compress")

        if (dir != null && !dir.exists()) dir.mkdirs()

        val file = File(dir, path.substring(path.lastIndexOf('/'),
                path.lastIndexOf('.')) + ".jpg")
        utility.deleteFile(file)

        val stream = ByteArrayOutputStream()
        bmp.compress(Bitmap.CompressFormat.JPEG, quality, stream)
        val byteArray = stream.toByteArray()

        try {
            file.createNewFile()
            file.writeBytes(byteArray)
        } catch (e: IOException) {
            e.printStackTrace()
        }

        bmp.recycle()

        result.success(file.absolutePath)
    }
}