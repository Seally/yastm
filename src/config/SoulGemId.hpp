#ifndef SOULGEMID_HPP
#define SOULGEMID_HPP

#include <string>
#include <unordered_set>
#include <cstdint>

#include <toml++/toml.h>

#include <boost/container_hash/hash.hpp>

class SoulGemId {
	const std::uint32_t _formId;
	const std::string _pluginName;

public:
    explicit SoulGemId(const std::uint32_t formId, const std::string& pluginName);

    static SoulGemId constructFromToml(toml::array& array);

    template<typename iterator>
    static bool areAllUnique(iterator begin, iterator end);

    std::uint32_t formId() const { return _formId; }
    const std::string& pluginName() const { return _pluginName; }
};

inline bool operator==(const SoulGemId& a, const SoulGemId& b) {
    return a.formId() == b.formId() && a.pluginName() == b.pluginName();
}

// Inject hash specialization into std namespace.
namespace std {
    template<> struct hash<SoulGemId>
    {
        std::size_t operator()(const SoulGemId& soulGemId) const noexcept {
            std::size_t seed = 0;

            boost::hash_combine(seed, soulGemId.formId());
            boost::hash_combine(seed, soulGemId.pluginName());

            return seed;
        }
    };
}

template<typename iterator>
bool SoulGemId::areAllUnique(iterator begin, iterator end) {
    static_assert(std::is_same_v<typename std::iterator_traits<iterator>::value_type, SoulGemId>);

    struct Hash {
        std::size_t operator()(const SoulGemId* const a) const noexcept {
            return std::hash<SoulGemId>{}(*a);
        }
    };

    auto equals = [](const SoulGemId* const a, const SoulGemId* const b) {
        return *a == *b;
    };

    // Use an unordered set to disambiguate between SoulGemIds. We override the
    // pointers
    std::unordered_set<const SoulGemId*, Hash, decltype(equals)> uniques;

    std::size_t count = 0;

    for (auto it = begin; it != end; ++it) {
        uniques.insert(&(*it));
        ++count;
    }

    return uniques.size() == count;
}

#endif // SOULGEMID_HPP
