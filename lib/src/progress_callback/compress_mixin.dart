import 'package:flutter/foundation.dart';
import 'package:flutter/services.dart';

import 'subscription.dart';

class CompressMixin {
  final compressProgress$ = ObservableBuilder<double>();
  final _channel = const MethodChannel('video_compress');

  @protected
  void initProcessCallback() {
    _channel.setMethodCallHandler(_progressCallback);
  }

  MethodChannel get channel => _channel;

  Future<void> _progressCallback(MethodCall call) async {
    switch (call.method) {
      case 'updateProgress':
        final progress = double.tryParse(call.arguments.toString());
        if (progress != null) compressProgress$.next(progress);
        break;
    }
  }
}
