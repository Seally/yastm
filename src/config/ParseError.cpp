#include "ParseError.hpp"

ParseError::ParseError(const std::string& message)
    : std::runtime_error{message}
{}

EntryError::EntryError(const KeyType key, const std::string& message)
    : ParseError{message}
    , key{key}
{}

InvalidEntryValueTypeError::InvalidEntryValueTypeError(
    const KeyType key,
    ValueType expectedType,
    const std::string& message)
    : EntryError{key, message}
    , expectedType{expectedType}
{}

EntryRange::EntryRange(Value expected)
    : type{EntryRange::Type::Equals}
    , values{expected}
{}

EntryRange::EntryRange(Value min, Value max)
    : type{EntryRange::Type::Between}
    , values{min, max}
{}

EntryRange::EntryRange(std::initializer_list<Value> values)
    : type{EntryRange::Type::Enumerated}
    , values{values}
{}

EntryValueOutOfRangeError::EntryValueOutOfRangeError(
    const KeyType key,
    const EntryRange& expectedRange,
    const std::string& message)
    : EntryError{key, message}
    , expectedRange{expectedRange}
{}

ArrayEntryError::ArrayEntryError(
    KeyType key,
    Type type,
    const std::string& message)
    : EntryError{key, message}
    , type{type}
{}

ArrayDuplicateEntriesError::ArrayDuplicateEntriesError(
    KeyType key,
    const std::string& message)
    : ArrayEntryError{key, ArrayEntryError::Type::DuplicateEntries, message}
{}

ArrayInvalidSizeError::ArrayInvalidSizeError(
    KeyType key,
    const EntryRange& expectedRange,
    const std::string& message)
    : ArrayEntryError{key, ArrayEntryError::Type::InvalidSize, message}
    , expectedRange{expectedRange}
{}
