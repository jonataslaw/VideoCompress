import 'dart:async';

import 'dart:ui';

class ObservableBuilder<T> {
  StreamController<T> _observable = StreamController();
  bool notSubscribed = true;

  void next(T value) {
    _observable.add(value);
  }

  Subscription subscribe(void Function(T event) onData,
      {Function? onError, void Function()? onDone, bool? cancelOnError}) {
    notSubscribed = false;
    _observable.stream.listen(onData,
        onError: onError, onDone: onDone, cancelOnError: cancelOnError);
    return Subscription(() {
      _observable.close();

      // Create a new instance to avoid errors
      _observable = StreamController();
    });
  }
}

class Subscription {
  final VoidCallback unsubscribe;
  const Subscription(this.unsubscribe);
}
