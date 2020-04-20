# video_compress_example

```dart
import 'dart:io';
import 'dart:async';
import 'dart:typed_data';

import 'package:flutter/material.dart';
import 'package:video_compress/video_compress.dart';
import 'package:image_picker/image_picker.dart' show ImagePicker, ImageSource;

void main() => runApp(MyApp());

class MyApp extends StatefulWidget {
  @override
  _MyAppState createState() => _MyAppState();
}

class _MyAppState extends State<MyApp> {
  final _flutterVideoCompress = FlutterVideoCompress();
  Uint8List _image;
  File _imageFile;
  Subscription _subscription;

  @override
  void initState() {
    super.initState();
    _subscription =
        _flutterVideoCompress.compressProgress$.subscribe((progress) {
      debugPrint('progress: $progress');
    });
  }

  @override
  void dispose() {
    super.dispose();
    _subscription.unsubscribe();
  }

  Future<void> _videoPicker() async {
    if (mounted) {
      final file = await ImagePicker.pickVideo(source: ImageSource.camera);
      if (file?.path != null) {
        final thumbnail = await _flutterVideoCompress.getByteThumbnail(
          file.path,
          quality: 50,
          position: -1,
        );

        setState(() {
          _image = thumbnail;
        });

        final resultFile = await _flutterVideoCompress.getFileThumbnail(
          file.path,
          quality: 50,
          position: -1,
        );
        debugPrint(resultFile.path);

        assert(resultFile.existsSync());

        debugPrint('file Exists: ${resultFile.existsSync()}');

        final MediaInfo info = await _flutterVideoCompress.compressVideo(
          file.path,
          deleteOrigin: true,
          quality: VideoQuality.LowQuality,
        );
        debugPrint(info.toJson().toString());
      }
    }
  }

  Future<void> _cancelCompression() async {
    await _flutterVideoCompress.cancelCompression();
  }

  Future<void> _getMediaInfo() async {
    if (mounted) {
      final file = await ImagePicker.pickVideo(source: ImageSource.gallery);
      if (file?.path != null) {
        final info = await _flutterVideoCompress.getMediaInfo(file.path);
        debugPrint(info.toJson().toString());
      }
    }
  }

  Future<void> _convertVideoToGif() async {
    if (mounted) {
      final file = await ImagePicker.pickVideo(source: ImageSource.gallery);
      if (file?.path != null) {
        final info = await _flutterVideoCompress.convertVideoToGif(
          file.path,
          startTime: 0,
          duration: 5,
        );

        debugPrint(info.path);
        setState(() {
          _imageFile = info;
        });
      }
    }
  }

  List<Widget> _builColumnChildren() {
    // dart 2.3 before
    final _list = <Widget>[
      FlatButton(child: Text('take video'), onPressed: _videoPicker),
      FlatButton(child: Text('stop compress'), onPressed: _cancelCompression),
      FlatButton(child: Text('get media info'), onPressed: _getMediaInfo),
      FlatButton(
        child: Text('convert video to gif'),
        onPressed: _convertVideoToGif,
      ),
    ];
    if (_imageFile != null) {
      _list.add(Flexible(child: Image.file(_imageFile)));
    } else if (_image != null) {
      _list.add(Flexible(child: Image.memory(_image)));
    }
    return _list;

    // dart 2.3
    // final _list = [
    //   FlatButton(child: Text('take video'), onPressed: _videoPicker),
    //   FlatButton(child: Text('stop compress'), onPressed: _cancelCompression),
    //   FlatButton(child: Text('get media info'), onPressed: _getMediaInfo),
    //   FlatButton(
    //     child: Text('convert video to gif'),
    //     onPressed: _convertVideoToGif,
    //   ),
    //   if (_imageFile != null)
    //     Flexible(child: Image.file(_imageFile))
    //   else
    //     if (_image != null) Flexible(child: Image.memory(_image))
    // ];
    // return _list;
  }

  @override
  Widget build(BuildContext context) {
    return MaterialApp(
      home: Scaffold(
        appBar: AppBar(title: const Text('Plugin example app')),
        body: Column(
          crossAxisAlignment: CrossAxisAlignment.stretch,
          children: _builColumnChildren(),
        ),
      ),
    );
  }
}
```