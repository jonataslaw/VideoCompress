import 'dart:io';
import 'package:file_selector/file_selector.dart';
import 'package:flutter/material.dart';
import 'package:image_picker/image_picker.dart';
import 'package:video_compress/video_compress.dart';

class VideoThumbnail extends StatefulWidget {
  @override
  _VideoThumbnailState createState() => _VideoThumbnailState();
}

class _VideoThumbnailState extends State<VideoThumbnail> {
  File? _thumbnailFile;

  @override
  Widget build(BuildContext context) {
    Future<Null> _getVideoThumbnail() async {
      var file;

      if (Platform.isMacOS) {
        final typeGroup =
            XTypeGroup(label: 'videos', extensions: ['mov', 'mp4']);
        file = await openFile(acceptedTypeGroups: [typeGroup]);
      } else {
        final picker = ImagePicker();
        var pickedFile = await picker.pickVideo(source: ImageSource.gallery);
        file = File(pickedFile!.path);
      }

      if (file != null) {
        _thumbnailFile = await VideoCompress.getFileThumbnail(file.path);
        setState(() {
          print(_thumbnailFile);
        });
      } else {
        return null;
      }
    }

    return Scaffold(
      appBar: AppBar(title: Text('File Thumbnail')),
      body: Center(
        child: Column(
          mainAxisAlignment: MainAxisAlignment.center,
          crossAxisAlignment: CrossAxisAlignment.center,
          children: [
            Container(
                child: ElevatedButton(
                    onPressed: _getVideoThumbnail,
                    child: Text('Get File Thumbnail'))),
            _buildThumbnail(),
          ],
        ),
      ),
    );
  }

  Widget _buildThumbnail() {
    if (_thumbnailFile != null) {
      return Container(
        padding: EdgeInsets.all(20.0),
        child: Image(image: FileImage(_thumbnailFile!)),
      );
    }
    return Container();
  }
}
