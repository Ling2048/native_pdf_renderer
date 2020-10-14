#include <flutter/standard_method_codec.h>

using flutter::EncodableValue, flutter::EncodableMap;

const EncodableValue *ValueOrNull(const EncodableMap &map, const char *key) {
  auto it = map.find(EncodableValue(key));
  if (it == map.end()) {
    return nullptr;
  }
  return &(it->second);
}