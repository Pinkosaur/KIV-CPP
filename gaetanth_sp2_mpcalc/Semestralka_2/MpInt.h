#pragma once
#include <vector>
#include <stdexcept>
#include <algorithm>
#include <concepts>
#include <cstdint>
#include <string>
#include <sstream>
#include <iomanip>
#include <limits>
#include <type_traits>
#include <future>
#include <thread>
#include <map>

template <std::size_t MaxBytes>
class MpInt;

// Vyjimka pro preteceni
template <std::size_t MaxBytes>
class MpIntOverflowException final : public std::exception {  // pridano final
    std::string message;
    MpInt<MaxBytes> overflowed_result;
public:
    explicit MpIntOverflowException(const std::string& msg, const MpInt<MaxBytes>& result)
        : message(msg + ": " + result.to_string()), overflowed_result(result) {
    }
    const char* what() const noexcept override { return message.c_str(); }
    const MpInt<MaxBytes>& get_overflowed_result() const { return overflowed_result; }
};

template <std::size_t MaxBytes>
class MpInt final {  // pridano final
private:
    std::vector<uint32_t> chunks;
    bool is_negative = false;

    // Pomocna metoda: Zjisti zda je presnost neomezena
    static constexpr bool is_unlimited = MaxBytes == std::numeric_limits<std::size_t>::max();

    // Pomocna metoda: Vypocet maximalniho poctu chunku
    static constexpr std::size_t MaxChunks =
        is_unlimited ? std::numeric_limits<std::size_t>::max() : (MaxBytes / sizeof(uint32_t));

    // Pomocna metoda: Kontrola platne velikosti
    void ensure_valid_size() const {
        if constexpr (!is_unlimited) {
            if (chunks.size() > MaxChunks) {
                throw MpIntOverflowException<MaxBytes>("Preteceni", *this);
            }
        }
    }

    // Pomocna metoda: Odstraneni uvodnich nul
    void remove_leading_zeros() {
        while (chunks.size() > 1 && chunks.back() == 0) {
            chunks.pop_back();
        }
        if (chunks.size() == 1 && chunks[0] == 0) {
            is_negative = false; // Nula je vzdy nezaporna
        }
    }

    static int compare_abs(const MpInt& lhs, const MpInt& rhs) {
        if (lhs.chunks.size() != rhs.chunks.size()) {
            return lhs.chunks.size() < rhs.chunks.size() ? -1 : 1;
        }

        // Hromadne porovnani pomoci memcmp
        const std::size_t num_chunks = lhs.chunks.size();
        const uint32_t* lhs_data = lhs.chunks.data();
        const uint32_t* rhs_data = rhs.chunks.data();

        for (std::size_t i = num_chunks; i-- > 0;) {
            if (lhs_data[i] != rhs_data[i]) {
                return lhs_data[i] < rhs_data[i] ? -1 : 1;
            }
        }

        return 0;
    }

    static std::map<uint32_t, MpInt<MaxBytes>> initializePrecomputed() {
        static std::map<uint32_t, MpInt<MaxBytes>> precomputed;
        if (MaxBytes == std::numeric_limits<std::size_t>::max()) {
            precomputed = {
                {50, MpInt<MaxBytes>::from_string("30414093201713378043612608166064768844377641568960512000000000000")},
                {75, MpInt<MaxBytes>::from_string("24809140811395398091946477116594033660926243886570122837795894512655842677572867409443815424000000000000000000")},
                {100, MpInt<MaxBytes>::from_string("93326215443944152681699238856266700490715968264381621468592963895217599993229915608941463976156518286253697920827223758251185210916864000000000000000000000000")},
                {200, MpInt<MaxBytes>::from_string("788657867364790503552363213932185062295135977687173263294742533244359449963403342920304284011984623904177212138919638830257642790242637105061926624952829931113462857270763317237396988943922445621451664240254033291864131227428294853277524242407573903240321257405579568660226031904170324062351700858796178922222789623703897374720000000000000000000000000000000000000000000000000")},
            };
        }
        else {
            precomputed = {
                {10, MpInt<MaxBytes>(3628800)},
                {50, MpInt<MaxBytes>::from_string("30414093201713378043612608166064768844377641568960512000000000000")},
            };
        }
        return precomputed;
    }

    static MpInt<MaxBytes> productRange(uint32_t low, uint32_t high) {
        if (low == high) return MpInt<MaxBytes>(low);
        if (high - low == 1) return MpInt<MaxBytes>(low) * MpInt<MaxBytes>(high);
        if (high - low < 10) { // Primocary vypocet pro male rozsahy
            MpInt<MaxBytes> result(low);
            for (uint32_t i = low + 1; i <= high; ++i) {
                result = result * MpInt<MaxBytes>(i);
            }
            return result;
        }

        const uint32_t mid = (low + high) / 2;

        // Pouziti vicevlaken pro velke rozsahy
        if (high - low > 20) {
            std::future<MpInt<MaxBytes>> left_part = std::async(std::launch::async, productRange, low, mid);
            const MpInt<MaxBytes> right_part = productRange(mid + 1, high);
            return left_part.get() * right_part;
        }
        else {
            // Sekvencni vypocet pro male rozsahy
            return productRange(low, mid) * productRange(mid + 1, high);
        }
    }

public:
    static constexpr std::size_t Unlimited = std::numeric_limits<std::size_t>::max();

    // Zakladni konstruktor
    MpInt() : chunks(1, 0), is_negative(false) {}

    // Konstruktor z celociselne hodnoty
    explicit MpInt(int64_t value) {
        is_negative = value < 0;
        uint64_t abs_value = is_negative ? -value : value;
        while (abs_value > 0) {
            chunks.push_back(static_cast<uint32_t>(abs_value & 0xFFFFFFFF));
            abs_value >>= 32;
        }
        if (chunks.empty()) {
            chunks.push_back(0);
        }
    }

    // Konstruktor z iteratoru
    template <std::input_iterator InputIt>
    MpInt(InputIt first, InputIt last) : chunks(first, last), is_negative(false) {
        if (!is_unlimited && chunks.size() > MaxChunks) {
            throw MpIntOverflowException<MaxBytes>("Prekrocen maximalni pocet chunku", *this);
        }
        remove_leading_zeros();
    }

    // Kopirovaci konstruktor
    MpInt(const MpInt& other) = default;

    // Presunovaci konstruktor
    MpInt(MpInt&& other) noexcept = default;

    // Kopirovaci operator prirazeni
    MpInt& operator=(const MpInt& other) = default;

    // Presunovaci operator prirazeni
    MpInt& operator=(MpInt&& other) noexcept = default;

    // Operator scitani
    template <std::size_t OtherMaxBytes>
    MpInt<(MaxBytes > OtherMaxBytes ? MaxBytes : OtherMaxBytes)> operator+(const MpInt<OtherMaxBytes>& other) const {
        if (is_negative == other.is_negative) {
            MpInt result;
            result.is_negative = is_negative;
            const std::size_t max_size = std::max(chunks.size(), other.chunks.size());
            result.chunks.resize(max_size, 0);

            uint64_t carry = 0;
            for (std::size_t i = 0; i < max_size || carry > 0; ++i) {
                const uint64_t lhs_chunk = i < chunks.size() ? chunks[i] : 0;
                const uint64_t rhs_chunk = i < other.chunks.size() ? other.chunks[i] : 0;
                const uint64_t sum = lhs_chunk + rhs_chunk + carry;

                if (i >= result.chunks.size()) {
                    result.chunks.push_back(static_cast<uint32_t>(sum));
                }
                else {
                    result.chunks[i] = static_cast<uint32_t>(sum);
                }
                carry = sum >> 32;
            }

            result.ensure_valid_size();
            return result;
        }
        else {
            return *this - (-other);
        }
    }

    // Operator scitani s int
    MpInt operator+(int value) const {
        return *this + MpInt(value);
    }

    // Operator odcitani
    template <std::size_t OtherMaxBytes>
    MpInt<(MaxBytes > OtherMaxBytes ? MaxBytes : OtherMaxBytes)> operator-(const MpInt<OtherMaxBytes>& other) const {
        if (is_negative != other.is_negative) {
            return *this + (-other);
        }

        if (compare_abs(*this, other) < 0) {
            return -(other - *this);
        }

        MpInt result;
        result.is_negative = is_negative;
        result.chunks.resize(chunks.size(), 0);

        int64_t borrow = 0;
        for (std::size_t i = 0; i < chunks.size(); ++i) {
            const int64_t lhs_chunk = chunks[i];
            const int64_t rhs_chunk = i < other.chunks.size() ? other.chunks[i] : 0;
            int64_t diff = lhs_chunk - rhs_chunk - borrow;

            if (diff < 0) {
                diff += (1LL << 32);
                borrow = 1;
            }
            else {
                borrow = 0;
            }
            result.chunks[i] = static_cast<uint32_t>(diff);
        }

        result.remove_leading_zeros();
        result.ensure_valid_size();
        return result;
    }

    // Naivni nasobeni pro male vstupy
    MpInt naiveMultiply(const MpInt& other) const {
        MpInt result;
        result.chunks.resize(chunks.size() + other.chunks.size(), 0);

        for (std::size_t i = 0; i < chunks.size(); ++i) {
            uint64_t carry = 0;
            for (std::size_t j = 0; j < other.chunks.size() || carry > 0; ++j) {
                const uint64_t product = result.chunks[i + j] +
                    static_cast<uint64_t>(chunks[i]) * (j < other.chunks.size() ? other.chunks[j] : 0) +
                    carry;
                result.chunks[i + j] = static_cast<uint32_t>(product);
                carry = product >> 32;
            }
        }

        result.remove_leading_zeros();
        result.ensure_valid_size();
        return result;
    }

    // Optimalizovane Karatsubovo nasobeni
    template <std::size_t OtherMaxBytes>
    MpInt karatsubaMultiply(const MpInt<OtherMaxBytes>& x, const MpInt<OtherMaxBytes>& y) const {
        const std::size_t n = std::max(x.chunks.size(), y.chunks.size());

        // Zakladni pripad: pouziti naivniho nasobeni pro mala cisla
        if (n <= 2) {
            return x.naiveMultiply(y);
        }

        const std::size_t half = (n + 1) / 2;

        MpInt x_low(x.chunks.begin(), x.chunks.begin() + std::min(half, x.chunks.size()));
        MpInt x_high(x.chunks.begin() + std::min(half, x.chunks.size()), x.chunks.end());
        MpInt y_low(y.chunks.begin(), y.chunks.begin() + std::min(half, y.chunks.size()));
        MpInt y_high(y.chunks.begin() + std::min(half, y.chunks.size()), y.chunks.end());

        // Rekurzivni vypocet tri produktu
        const MpInt z0 = karatsubaMultiply(x_low, y_low);
        const MpInt z2 = karatsubaMultiply(x_high, y_high);
        const MpInt z1 = karatsubaMultiply(x_low + x_high, y_low + y_high) - z2 - z0;

        // Kombinace vysledku
        MpInt result = z0 + (z1 << (half * 32)) + (z2 << (2 * half * 32));
        result.ensure_valid_size();
        return result;
    }

    // Hybridni logika nasobeni
    template <std::size_t OtherMaxBytes>
    MpInt hybridMultiply(const MpInt<OtherMaxBytes>& x, const MpInt<OtherMaxBytes>& y) const {
        // Prahove hodnoty pro prepinani algoritmu
        constexpr std::size_t NAIVE_THRESHOLD = 16;  // Naivni pro mala cisla
        constexpr std::size_t KARATSUBA_THRESHOLD = 128; // Karatsuba pro stredni cisla

        const std::size_t x_size = x.chunks.size();
        const std::size_t y_size = y.chunks.size();

        if (x_size < NAIVE_THRESHOLD || y_size < NAIVE_THRESHOLD) {
            return x.naiveMultiply(y);
        }

        if (x_size < KARATSUBA_THRESHOLD || y_size < KARATSUBA_THRESHOLD) {
            return karatsubaMultiply(x, y);
        }

        // Zaloha na Karatsubu pro velka cisla
        return karatsubaMultiply(x, y);
    }

    // Operator nasobeni
    template <std::size_t OtherMaxBytes>
    MpInt operator*(const MpInt<OtherMaxBytes>& other) const {
        MpInt<(MaxBytes > OtherMaxBytes ? MaxBytes : OtherMaxBytes)> result = hybridMultiply(*this, other);
        result.is_negative = (is_negative != other.is_negative);
        result.ensure_valid_size();
        return result;
    }

    // Operator deleni
    template <std::size_t OtherMaxBytes>
    MpInt operator/(const MpInt<OtherMaxBytes>& other) const {
        if (other == MpInt(0)) {
            throw std::invalid_argument("Deleni nulou.");
        }

        // Osetreni deleni 1 nebo -1
        if (other.chunks.size() == 1 && other.chunks[0] == 1) {
            MpInt<(MaxBytes > OtherMaxBytes ? MaxBytes : OtherMaxBytes)> result = *this;
            result.is_negative = (is_negative != other.is_negative);
            return result;
        }

        if (compare_abs(*this, other) < 0) {
            return MpInt(0);
        }

        MpInt quotient;
        MpInt remainder = *this;
        remainder.is_negative = false;

        MpInt divisor = other;
        divisor.is_negative = false;

        while (compare_abs(remainder, divisor) >= 0) {
            MpInt temp_divisor = divisor;
            MpInt temp_quotient(1);

            while (compare_abs(remainder, temp_divisor << 1) >= 0) {
                temp_divisor = temp_divisor << 1;
                temp_quotient = temp_quotient << 1;
            }

            remainder = remainder - temp_divisor;
            quotient = quotient + temp_quotient;
        }

        quotient.is_negative = (is_negative != other.is_negative);
        quotient.remove_leading_zeros();
        quotient.ensure_valid_size();
        return quotient;
    }

    // Operator modulo
    template <std::size_t OtherMaxBytes>
    MpInt operator%(const MpInt<OtherMaxBytes>& other) const {
        if (other == MpInt(0)) {
            throw std::invalid_argument("Modulo nulou.");
        }

        if (other.chunks.size() == 1 && other.chunks[0] == 1) {
            return MpInt(0);
        }

        if (compare_abs(*this, other) < 0) {
            return *this;
        }

        MpInt remainder = *this;
        remainder.is_negative = false;

        MpInt divisor = other;
        divisor.is_negative = false;

        while (compare_abs(remainder, divisor) >= 0) {
            MpInt temp_divisor = divisor;

            while (compare_abs(remainder, temp_divisor << 1) >= 0) {
                temp_divisor = temp_divisor << 1;
            }

            remainder = remainder - temp_divisor;
        }

        remainder.is_negative = is_negative;
        remainder.remove_leading_zeros();
        remainder.ensure_valid_size();
        return remainder;
    }



    // Deleni 32-bitovym celym cislem
    void divide_by_uint32(uint32_t divisor, MpInt& quotient, uint32_t& remainder) const {
        quotient.chunks.clear();
        quotient.is_negative = false; // Vysledek je nezaporny

        uint64_t current_remainder = 0;
        std::vector<uint32_t> quotient_chunks;

        // Zpracovani chunku od nejvyznamnejsiho po nejmene vyznamny
        for (auto it = chunks.rbegin(); it != chunks.rend(); ++it) {
            current_remainder = (current_remainder << 32) | *it;
            uint32_t q = static_cast<uint32_t>(current_remainder / divisor);
            current_remainder %= divisor;
            quotient_chunks.push_back(q);
        }

        // Chunky podilu jsou ulozeny v opacnem poradi (od MSB k LSB)
        std::reverse(quotient_chunks.begin(), quotient_chunks.end());
        quotient.chunks = quotient_chunks;
        quotient.remove_leading_zeros();
        remainder = static_cast<uint32_t>(current_remainder);
    }

    // Operator -
    MpInt operator-() const {
        MpInt result = *this;
        if (result != MpInt(0)) {
            result.is_negative = !is_negative;
        }
        return result;
    }

    // Operator += 
    MpInt operator+=(const MpInt& other) {
        *this = *this + other;
        return *this;
    }

    // Operator += pro int 
    MpInt operator+=(const int x) {
        *this = *this + MpInt(x);
        return *this;
    }

    // Operator -= 
    MpInt operator-=(const MpInt& other) {
        *this = *this - other;
        return *this;
    }

    // Operator -= pro int 
    MpInt operator-=(const int x) {
        *this = *this - MpInt(x);
        return *this;
    }

    // Operator *=
    MpInt operator*=(const MpInt& other) {
        *this = *this * other;
        return *this;
    }

    // Operator *= pro int
    MpInt operator*=(const int x) {
        *this = *this * MpInt(x);
        return *this;
    }

    // Operator /=
    MpInt operator/=(const MpInt& other) {
        *this = *this / other;
        return *this;
    }

    // Operator /= pro int
    MpInt operator/=(const int x) {
        *this = *this / MpInt(x);
        return *this;
    }

    // operator inkrementace
    MpInt operator++() {
        *this += 1;
        return *this;
    }

    // operator dekrementace
    MpInt operator--() {
        *this -= 1;
        return *this;
    }

    // Operator rovnosti
    bool operator==(const MpInt& other) const {
        return is_negative == other.is_negative && chunks == other.chunks;
    }

    // Operator nerovnosti 
    bool operator!=(const MpInt& other) const {
        return !(*this == other);
    }

    // Operator mensi nez 
    bool operator<(const MpInt& other) const {
        if (is_negative != other.is_negative) {
            return is_negative;
        }
        int abs_comparison = compare_abs(*this, other);
        return is_negative ? abs_comparison > 0 : abs_comparison < 0;
    }

    // Operator mensi nebo rovno
    bool operator<=(const MpInt& other) const {
        return *this < other || *this == other;
    }

    // Operator vetsi nez
    bool operator>(const MpInt& other) const {
        return !(*this <= other);
    }

    // Operator vetsi nebo rovno 
    bool operator>=(const MpInt& other) const {
        return !(*this < other);
    }

    // Operator faktorialu (!) s kontrolou nezápornosti
    MpInt operator!() const {
        if (is_negative) {
            throw std::invalid_argument("Faktorial neni definovan pro zaporna cisla.");
        }
        return MpInt::factorial(this->to_uint32());
    }

    // Operator bitoveho posunu doleva
    MpInt operator<<(uint32_t shift) const {
        MpInt result = *this;
        result.leftShift(shift);
        return result;
    }

    // Pomocna metoda pro bitovy posun doleva
    void leftShift(uint32_t shift) {
        if (shift == 0) return;

        uint32_t chunk_shift = shift / 32;
        uint32_t bit_shift = shift % 32;

        std::vector<uint32_t> new_chunks;
        new_chunks.reserve(chunks.size() + chunk_shift + 1);

        // Pridani uvodnich nul pro chunk_shift
        new_chunks.insert(new_chunks.end(), chunk_shift, 0);

        uint32_t carry = 0;
        for (uint32_t chunk : chunks) {
            new_chunks.push_back((chunk << bit_shift) | carry);
            carry = (bit_shift > 0) ? (chunk >> (32 - bit_shift)) : 0;
        }

        if (carry > 0) {
            new_chunks.push_back(carry);
        }

        chunks = std::move(new_chunks);
        ensure_valid_size();
    }

    // Prevod MpInt na retezec
    std::string to_string() const {
        if (*this == MpInt(0)) {
            return "0";
        }

        MpInt temp = *this;
        temp.is_negative = false;
        std::ostringstream oss;
        if (is_negative) {
            oss << "-";
        }

        const uint32_t divisor = 1000000000; // 10^9
        std::vector<std::string> digit_groups;

        while (temp != MpInt(0)) {
            MpInt quotient;
            uint32_t remainder;
            temp.divide_by_uint32(divisor, quotient, remainder);
            temp = quotient;

            // Konverze zbytku na 9mistny retezec (s doplnenim nul)
            char buffer[10]; // 9 cislic + ukoncovaci znak
            snprintf(buffer, sizeof(buffer), "%09u", remainder);
            digit_groups.push_back(buffer);
        }

        // Spojeni skupin a odstraneni uvodnich nul 
        bool first_group = true;
        for (auto it = digit_groups.rbegin(); it != digit_groups.rend(); ++it) {
            if (first_group) {
                size_t start = it->find_first_not_of('0');
                if (start != std::string::npos) {
                    oss << it->substr(start);
                }
                else {
                    oss << "0";
                }
                first_group = false;
            }
            else {
                oss << *it;
            }
        }

        return oss.str();
    }

    // Konstrukce MpInt z retezce
    static MpInt from_string(const std::string& str) {
        if (str.empty()) {
            throw std::invalid_argument("Prazdny retezec nelze prevest na MpInt.");
        }

        bool is_negative = (str[0] == '-');
        std::size_t start_idx = (is_negative || str[0] == '+') ? 1 : 0;

        MpInt result;
        for (std::size_t i = start_idx; i < str.size(); ++i) {
            if (!std::isdigit(str[i])) {
                throw std::invalid_argument("Neplatny znak v retezci: " + str);
            }
            result = result * MpInt(10) + MpInt(static_cast<int64_t>(str[i] - '0'));
        }

        result.is_negative = is_negative;
        result.ensure_valid_size();
        return result;
    }

    // Prevod MpInt na uint32_t 
    uint32_t to_uint32() const {
        if (!fits_in_uint32()) {
            throw std::overflow_error("Hodnota je prilis velka pro prevod na uint32_t");
        }
        return static_cast<uint32_t>(chunks[0]);
    }

    // Kontrola, zda lze hodnotu ulozit do uint32_t
    bool fits_in_uint32() const {
        if (chunks.size() > 1) {
            return false;
        }
        return chunks[0] <= std::numeric_limits<uint32_t>::max();
    }

    // Vypocet faktorialu s vyuzitim predpocitanych hodnot a rozsahu soucinu
    static MpInt<MaxBytes> factorial(uint32_t n) {
        static std::map<uint32_t, MpInt<MaxBytes>> precomputed = initializePrecomputed();

        if (n < 2) {
            return MpInt<MaxBytes>(1);
        }

        // Hledani nejvetsi predpocitane hodnoty <= n
        auto it = precomputed.upper_bound(n);
        if (it != precomputed.begin()) --it;

        uint32_t base = it->first;
        MpInt<MaxBytes> result = it->second;

        if (base < n) {
            result = result * productRange(base + 1, n);
        }

        return result;
    }
};