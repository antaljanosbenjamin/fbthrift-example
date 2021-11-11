
namespace cpp2 example.storage

enum TemporalDataType {
    DATE = 0,
    LOCAL_TIME = 1,
    LOCAL_DATE_TIME = 2,
    DURATION = 3,
  }

struct TemporalData {
  1: TemporalDataType type,
  2: i64 microseconds,
}

union PropertyValue {
    1: Null null_v;
    2: bool bool_v;
    3: i64 int_v;
    4: double double_v;
    5: binary string_v;
    6: List list_v;
    7: Map map_v;
    8: TemporalData temporal_v;
}

struct Null {
}

struct List {
    1: list<PropertyValue> values,
}

struct Map {
    1: map<binary, PropertyValue> kv_pairs,
}

struct PropertyRequest {
  1: binary name,
  2: optional i64 count,
}

service StorageService {
  list<PropertyValue> GetPropertyStream(1: PropertyRequest req)
  PropertyValue GetProperty(1: PropertyRequest req)
}
