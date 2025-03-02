#pragma once

#include <sstream>
#include <string>

#include "AbstractToken.hpp"

template <typename T>
class OperandToken : public AbstractToken {
 public:
  OperandToken(const std::string& input);

  OperandToken(const T& value) : OperandToken(std::to_string(value)) {}

  const T& GetValue() { return value_; }

 private:
  T value_;
};

template <typename T>
OperandToken<T>::OperandToken(const std::string& input) : AbstractToken(input) {
  std::stringstream stream;
  stream << input;
  stream >> value_;
}