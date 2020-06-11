#pragma once
#include <iostream>
#include <vector>
#include <utility>

class Weight {
public:
    Weight() {}
    Weight(size_t len) : value(len) {}
    Weight(Weight&& f) : value(std::move(f.value)) {}
    Weight(const Weight& f) = default;

    Weight& operator =(const Weight& f) = default;
    float& operator[] (size_t i) { return value[i]; }
    const float& operator[] (size_t i) const { return value[i]; }
    size_t size() const { return value.size(); }

public:
    friend std::ostream& operator <<(std::ostream& out, const Weight& w) {
        auto& value = w.value;
        uint64_t size = value.size();
        out.write(reinterpret_cast<const char*>(&size), sizeof(uint64_t));
        out.write(reinterpret_cast<const char*>(value.data()), sizeof(float) * size);
        return out;
    }
    friend std::istream& operator >>(std::istream& in, Weight& w) {
        auto &value = w.value;
        uint64_t size = 0;
        in.read(reinterpret_cast<char*>(&size), sizeof(uint64_t));
        value.resize(size);
        in.read(reinterpret_cast<char*>(value.data()), sizeof(float) * size);
        return in; 
    }

protected:
    std::vector<float> value;
};