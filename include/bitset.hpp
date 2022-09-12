#include <stdint.h>

template<uint8_t N>
class Bitset {
public:
  constexpr uint8_t size() {
    return N;
  }

  bool operator[](uint8_t pos) const {
    return test(pos);
  }

  bool set(uint8_t bit) {
    uint8_t mask;
    uint8_t& dataCell = addressHelper(bit, mask);
    bool wasSet = dataCell & mask;
    dataCell |= mask;
    return wasSet;
  }

  bool reset(uint8_t bit) {
    uint8_t mask;
    uint8_t& dataCell = addressHelper(bit, mask);
    const uint8_t oldValue = dataCell;
    dataCell &= ~mask;
    return oldValue != dataCell;
  }

  bool test(uint8_t bit) const {
    uint8_t mask;
    uint8_t cellIndex;
    addressHelper(bit, mask, cellIndex);
    return (data[cellIndex] & mask) > 0;
  }

  uint8_t count() const {
    uint8_t value = 0;
    for (uint8_t i = 0; i < array_size(data); i++) {
      uint8_t mask = 1;
      while (mask > 0) {
        if (data[i] & mask) {
          value++;
        }
        mask <<= 1;
      }
    }
    return value;
  }
private:
  static constexpr uint8_t bitsPerData = sizeof(uint8_t) * 8;

  static constexpr uint8_t sizeForBits(uint8_t bits) {
    return bits / Bitset::bitsPerData + (bits % Bitset::bitsPerData > 0 ? 1 : 0); 
  }

  void addressHelper(uint8_t bit, uint8_t& mask, uint8_t& byte) const {
    byte = bit / Bitset::bitsPerData;
    bit %= Bitset::bitsPerData;
    mask = 1 << bit;
  }

  uint8_t& addressHelper(uint8_t bit, uint8_t& mask) {
    uint8_t byte;
    addressHelper(bit, mask, byte);
    return data[byte];
  }

  uint8_t data[Bitset::sizeForBits(N)] = {0};
};