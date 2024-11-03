#pragma once
#include <vector>
#include <optional>
#include <memory>

template <class Key, class Value>
class keyValueLookup
{
    std::vector<std::pair<Key, Value>> data;

    template <class U>
    std::optional<size_t> find(const U& key) const
        requires std::equality_comparable_with<Key, U>
    {
        for (size_t i = 0; i < data.size(); ++i)
        {
            if (data[i].first == key)
            {
                return i;
            }
        }
        return std::nullopt;
    }

public:
    template <class U>
    Value& operator[](const U& key)
        requires std::equality_comparable_with<Key, U>
    {
        auto idx = find(key);
        return idx.has_value() ? data[idx.value()].second : data.emplace_back(key, Value{}).second;
    }

    template <class U>
    Value* tryGet(const U& key)
        requires std::equality_comparable_with<Key, U>
    {
        auto idx = find(key);
        return idx.has_value() ? &data[idx.value()].second : nullptr;
    }

    template <class U>
    const Value* tryGet(const U& key) const
        requires std::equality_comparable_with<Key, U>
    {
        auto idx = find(key);
        return idx.has_value() ? &data[idx.value()].second : nullptr;
    }

    template <class U>
	const Value& getOr(const U& key, const Value& defaultValue) const
		requires std::equality_comparable_with<Key, U>
	{
		auto idx = find(key);
		return idx.has_value() ? data[idx.value()].second : defaultValue;
	}

    bool contains(const Key& key) const
    {
        return find(key).has_value();
    }

    template <class U>
    void erase(const U& key)
        requires std::equality_comparable_with<Key, U>
    {
        auto idx = find(key);

        if (idx.has_value())
        {
            data.erase(data.begin() + idx.value());
        }
    }

    size_t size() const { return data.size(); }

    auto begin() { return data.begin(); }
    auto end() { return data.end(); }
    auto begin() const { return data.begin(); }
    auto end() const { return data.end(); }
};