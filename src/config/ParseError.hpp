#pragma once

#include <exception>
#include <string>
#include <string_view>
#include <unordered_map>
#include <variant>

class ParseError : public std::runtime_error {
public:
    explicit ParseError(const std::string& message);
};

using KeyType = std::variant<std::size_t, std::string>;

class EntryError : public ParseError {
public:
    const KeyType key;

    explicit EntryError(KeyType key, const std::string& message);
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
        const std::string& message);
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

    explicit EntryRange(Value expected);
    explicit EntryRange(Value min, Value max);
    explicit EntryRange(std::initializer_list<Value> values);
};

class EntryValueOutOfRangeError : public EntryError {
public:
    const EntryRange expectedRange;

    explicit EntryValueOutOfRangeError(
        KeyType key,
        const EntryRange& expectedRange,
        const std::string& message);
};

class ArrayEntryError : public EntryError {
public:
    enum class Type {
        InvalidSize,
        DuplicateEntries,
    };

    const Type type;

protected:
    explicit ArrayEntryError(
        KeyType key,
        Type type,
        const std::string& message);
};

class ArrayDuplicateEntriesError : public ArrayEntryError {
public:
    explicit ArrayDuplicateEntriesError(
        KeyType key,
        const std::string& message);
};

class ArrayInvalidSizeError : public ArrayEntryError {
public:
    const EntryRange expectedRange;

    explicit ArrayInvalidSizeError(
        KeyType key,
        const EntryRange& expectedRange,
        const std::string& message);
};
