import 'dart:io';

import 'package:flutter/services.dart';
import 'package:flutter_driver/driver_extension.dart';
import 'package:path_provider/path_provider.dart';
import 'package:video_compress_example/main.dart' as app;

void main() {
  enableFlutterDriverExtension();
  _mockImagePicker();

  app.main();
}

void _mockImagePicker() {
  final channel = MethodChannel('plugins.flutter.io/image_picker');
  channel.setMockMethodCallHandler((_) async {
    final data = await rootBundle.load(
      'assets/samples/sample.mp4',
    );
    final bytes = data.buffer.asUint8List();
    final dir = await getTemporaryDirectory();
    final file = await File('${dir.path}/sample.mp4').writeAsBytes(bytes);
    return file.path;
  });
}
