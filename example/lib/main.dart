import 'package:flutter/material.dart';
import 'package:image_picker/image_picker.dart';
import 'package:video_compress/video_compress.dart';
import 'package:file_selector/file_selector.dart';
import 'dart:io';

void main() {
  runApp(MyApp());
}

class MyApp extends StatelessWidget {
  @override
  Widget build(BuildContext context) {
    return MaterialApp(
      title: 'Flutter Demo',
      theme: ThemeData(
        primarySwatch: Colors.blue,
      ),
      home: MyHomePage(title: 'Flutter Demo Home Page'),
    );
  }
}

class MyHomePage extends StatefulWidget {
  MyHomePage({Key key, this.title}) : super(key: key);

  final String title;

  @override
  _MyHomePageState createState() => _MyHomePageState();
}

class _MyHomePageState extends State<MyHomePage> {
  String _counter = "video";

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      appBar: AppBar(
        title: Text(widget.title),
      ),
      body: Center(
        child: Column(
          mainAxisAlignment: MainAxisAlignment.center,
          children: <Widget>[
            Text(
              'You have pushed the button this many times:',
            ),
            Text(
              '$_counter',
              style: Theme.of(context).textTheme.headline4,
            ),
            InkWell(
                child: Icon(
                  Icons.cancel,
                  size: 55,
                ),
                onTap: () {
                  VideoCompress.cancelCompression();
                })
          ],
        ),
      ),
      floatingActionButton: FloatingActionButton(
        onPressed: () async {
          var file;
          if (Platform.isMacOS) {
            final typeGroup = XTypeGroup(label: 'videos', extensions: ['mov', 'mp4']);
            file = await openFile(acceptedTypeGroups: [typeGroup]);
          } else {
            final picker = ImagePicker();
            PickedFile pickedFile = await picker.getVideo(source: ImageSource.gallery);
            file = File(pickedFile.path);
          }
          if (file == null) {
            return;
          }
          await VideoCompress.setLogLevel(0);
          final info = await VideoCompress.compressVideo(
            file.path,
            quality: VideoQuality.MediumQuality,
            deleteOrigin: false,
            includeAudio: true,
          );
          print(info.path);
          if (info != null) {
            setState(() {
              _counter = info.path;
            });
          }
        },
        tooltip: 'Increment',
        child: Icon(Icons.add),
      ),
    );
  }
}
