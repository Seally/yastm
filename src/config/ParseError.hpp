#pragma once

#include <exception>
#include <string>
#include <string_view>
#include <unordered_map>
#include <variant>

class ParseError : public std::runtime_error {
public:
    explicit ParseError(const std::string& message)
        : std::runtime_error(message)
    {}

    explicit ParseError(const char* message)
        : std::runtime_error(message)
    {}
};

using KeyType = std::variant<std::size_t, std::string>;

class EntryError : public ParseError {
public:
    const KeyType key;

    explicit EntryError(KeyType key, const std::string& message)
        : ParseError(message)
        , key(key)
    {}

    explicit EntryError(KeyType key, const char* message)
        : ParseError(message)
        , key(key)
    {}
};

enum class ValueType {
    Integer,
    String,
    Array,
    Table,
};

class InvalidEntryValueTypeError : public EntryError {
public:
    const KeyType key;
    const ValueType expectedType;

    explicit InvalidEntryValueTypeError(
        KeyType key,
        ValueType expectedType,
        const std::string& message)
        : EntryError(key, message)
        , expectedType(expectedType)
    {}

    explicit InvalidEntryValueTypeError(
        KeyType key,
        ValueType expectedType,
        const char* message)
        : EntryError(key, message)
        , expectedType(expectedType)
    {}
};

class EntryRange {
    using Value = std::variant<std::string, int>;

public:
    enum class Type {
        Enumerated,
        Between,
        Equals,
    };

    const Type type;
    const std::vector<Value> values;

    explicit EntryRange(Value expected)
        : type(EntryRange::Type::Equals)
        , values{expected}
    {}
    explicit EntryRange(Value min, Value max)
        : type(EntryRange::Type::Between)
        , values{min, max}
    {}
    explicit EntryRange(std::initializer_list<Value> values)
        : type(EntryRange::Type::Enumerated)
        , values(values)
    {}
};

class EntryValueOutOfRangeError : public EntryError {
public:
    const EntryRange expectedRange;

    explicit EntryValueOutOfRangeError(
        KeyType key,
        const EntryRange& expectedRange,
        const std::string& message)
        : EntryError(key, message)
        , expectedRange(expectedRange)
    {}

    explicit EntryValueOutOfRangeError(
        KeyType key,
        const EntryRange& expectedRange,
        const char* message)
        : EntryError(key, message)
        , expectedRange(expectedRange)
    {}
};

class ArrayEntryError : public EntryError {
public:
    enum class Type {
        InvalidSize,
        DuplicateEntries,
    };

    const Type type;

protected:
    explicit ArrayEntryError(KeyType key, Type type, const std::string& message)
        : EntryError(key, message)
        , type(type)
    {}
};

class ArrayDuplicateEntriesError : public ArrayEntryError {
public:
    explicit ArrayDuplicateEntriesError(KeyType key, const std::string& message)
        : ArrayEntryError(key, ArrayEntryError::Type::DuplicateEntries, message)
    {}

    explicit ArrayDuplicateEntriesError(KeyType key, const char* message)
        : ArrayEntryError(key, ArrayEntryError::Type::DuplicateEntries, message)
    {}
};

class ArrayInvalidSizeError : public ArrayEntryError {
public:
    const EntryRange expectedRange;

    explicit ArrayInvalidSizeError(
        KeyType key,
        const EntryRange& expectedRange,
        const std::string& message)
        : ArrayEntryError(key, ArrayEntryError::Type::InvalidSize, message)
        , expectedRange(expectedRange)
    {}

    explicit ArrayInvalidSizeError(
        KeyType key,
        const EntryRange& expectedRange,
        const char* message)
        : ArrayEntryError(key, ArrayEntryError::Type::InvalidSize, message)
        , expectedRange(expectedRange)
    {}
};
