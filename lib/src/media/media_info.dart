import 'dart:io';

class MediaInfo {
  String? path;
  String? title;
  String? author;
  int? width;
  int? height;

  /// [Android] API level 17
  int? orientation;

  /// bytes
  int? filesize; // filesize
  /// microsecond
  double? duration;
  bool? isCancel;
  File? file;

  MediaInfo({
    required this.path,
    this.title,
    this.author,
    this.width,
    this.height,
    this.orientation,
    this.filesize,
    this.duration,
    this.isCancel,
    this.file,
  });

  MediaInfo.fromJson(Map<String, dynamic> json) {
    path = json['path'];
    title = json['title'];
    author = json['author'];
    width = json['width'];
    height = json['height'];
    orientation = json['orientation'];
    filesize = json['filesize'];
    duration = double.tryParse('${json['duration']}');
    isCancel = json['isCancel'];
    file = File(path!);
  }

  Map<String, dynamic> toJson() {
    final Map<String, dynamic> data = Map<String, dynamic>();
    data['path'] = this.path;
    data['title'] = this.title;
    data['author'] = this.author;
    data['width'] = this.width;
    data['height'] = this.height;
    if (this.orientation != null) {
      data['orientation'] = this.orientation;
    }
    data['filesize'] = this.filesize;
    data['duration'] = this.duration;
    if (this.isCancel != null) {
      data['isCancel'] = this.isCancel;
    }
    data['file'] = File(path!).toString();
    return data;
  }
}
