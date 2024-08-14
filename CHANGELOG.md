## 3.1.3

- Update build.gradle (@dharambudh1)
- Bump up transcoder to 0.10.5 (@Ayman-Barghout)
- Add AGP 8 support (@Zazo032)
- Add missing import in 3.1.2 (@jamesdixon)
- Bump to 3.1.2 (@jonataslaw)
- Fix issue where video name containing spaces wasn't properly decoded (@unknown-undefined)

## 3.1.2

- Fix "Failed to stop the muxer" and "java.lang.IllegalStateException" (@VoronovAlexander)
- Fix files with spaces (@unknown-undefined)

## 3.1.1

- Fix issue on iOS with files containing whitespaces (@kaiquegazola)
- Fix cancel compress on IOS (@posawatji)
- Update Android compression library (@crtl)
- Update flutter 3 (@pranavo72bex)
- Fix multiple compression on android (@neelansh-creatorstack )
- Fix multiple compression on iOS (zhuyangyang-lingoace)

## 3.1.0

- Bug fix on getMediaInfo (@trustmefelix)
- Improve getFileThumbnail() (@FelixMoMo/@trustmefelix)
- invalidate updateProgress timer after the completion of video export (@jinthislife)
- Added Support for resolution presets (@yanivshaked)

## 3.0.0

- Added MacOS support (thank's @efraespada)
- Null-safety support (thank's @rlazom feat @leynier)

## 2.1.1

- Fix Subscription import
- Fix Error on android 10 with no includeAudio option

## 2.1.0

- Added cancel compression to android
- Fix compress progress
- Added audio remove/include to android
- Upgrade to android v2
- Fix subscription progress receive same listener

## 2.0.0

- refactor code
  Breaking changes, call VideoCompress.method directly, without having to instantiate it.

## 1.0.0

- release new version

## 0.1.3

- added progress listen

## 0.1.2

- Removed unecessary intent when process is done

## 0.1.1

- Change default value to HD

## 0.1.0

- initial release
