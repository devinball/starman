#pragma once

using Number = float; // used to define basic precision
const float MAX_FLOATING_VALUE = 4000;// 4 km if measured in meters, yes that is pathetic, which is why big number exists

// also implement all sorts of operator overiding, so you can do BigNumber + float, BigNumber * double, BigNumber / int, etc...
// with 4k as floating max, this should be able the handle ~7.8*10^9 light years, which should be plenty
// using a normal long would only be ~1.8 light years, which also likely would be fine
struct BigNumber {
    long long multiplier;
    Number floating;

    BigNumber operator=(const BigNumber &n) {
        return BigNumber{};
    }

    BigNumber operator+(const BigNumber &n) {
        return BigNumber{};
    }

    BigNumber operator-(const BigNumber &n) {
        return BigNumber{};
    }

    BigNumber operator*(const BigNumber &n) {
        return BigNumber{};
    }

    BigNumber operator/(const BigNumber &n) {
        return BigNumber{};
    }

    BigNumber operator+=(const BigNumber &n) {
        return BigNumber{};
    }

    BigNumber operator-=(const BigNumber &n) {
        return BigNumber{};
    }

    BigNumber operator*=(const BigNumber &n) {
        return BigNumber{};
    }

    BigNumber operator/=(const BigNumber &n) {
        return BigNumber{};
    }

    bool operator==(const BigNumber &n) {
        return (multiplier == n.multiplier) && (floating == n.floating);
    }
};
