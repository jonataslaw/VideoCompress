import 'package:flutter_driver/flutter_driver.dart';
import 'package:test/test.dart';

void main() {
  final useSampleVideoBtnFinder = find.byValueKey('use_sample_video');
  final clearCacheBtnFinder = find.byValueKey('clear_cache');
  final statusFinder = find.byValueKey('status');
  final outputFileSizeFinder = find.byValueKey('output_file_size');

  FlutterDriver driver;

  setUpAll(() async {
    driver = await FlutterDriver.connect();
  });

  tearDownAll(() async {
    if (driver != null) {
      driver.close();
    }
  });

  test('compress video and clear cache', () async {
    expect(await driver.getText(statusFinder), 'init');

    // Initialize compression
    await driver.tap(useSampleVideoBtnFinder);

    // Wait for compression to finish
    await driver.waitFor(find.text('compressed'));

    // Output file should not be empty
    final outputFileSize = await driver.getText(outputFileSizeFinder);
    expect(int.parse(outputFileSize), greaterThan(0));

    // Clear the cache
    await driver.tap(clearCacheBtnFinder);

    // After clear cache, output file should be deleted
    await driver.waitFor(find.text('cache cleared'));
    expect(await driver.getText(outputFileSizeFinder), "-1");
  });
}
