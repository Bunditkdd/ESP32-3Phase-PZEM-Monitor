#define DEBUG 0

#if DEBUG
  #define debugPrint(...)   Serial.print(__VA_ARGS__)
  #define debugPrintln(...) Serial.println(__VA_ARGS__)
  #define debugPrintf(...)  Serial.printf(__VA_ARGS__)
#else
  #define debugPrint(...)
  #define debugPrintln(...)
  #define debugPrintf(...)
#endif