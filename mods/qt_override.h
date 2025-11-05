// clang_qt_override.h — included before any Qt headers when parsing with libclang

#ifndef CLANG_QT_OVERRIDE_H
#define CLANG_QT_OVERRIDE_H

#undef Q_OBJECT
#define Q_OBJECT

#undef Q_PROPERTY
#define Q_PROPERTY(...)

#undef Q_CLASSINFO
#define Q_CLASSINFO(...)

#undef Q_ENUM
#define Q_ENUM(...)

#undef Q_FLAGS
#define Q_FLAGS(...)

#undef signals
#define signals \
 public         \
  __attribute__((annotate("qt_signal")))

#undef slots
#define slots \
 public       \
  __attribute__((annotate("qt_slot")))

#undef Q_INVOKABLE
#define Q_INVOKABLE __attribute__((annotate("qt_invokable")))

#undef Q_SIGNAL
#define Q_SIGNAL __attribute__((annotate("qt_signal")))

#undef Q_SLOT
#define Q_SLOT __attribute__((annotate("qt_slot")))

#undef emit
#define emit

#endif  // CLANG_QT_OVERRIDE_H
