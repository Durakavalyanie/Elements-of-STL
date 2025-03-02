#pragma once

#include <string>

#include "AbstractToken.hpp"

class BracketToken : public AbstractToken {
 public:
  BracketToken(const std::string& input)
      : AbstractToken(input), isopening_(string_view_ == "(") {}

  bool IsOpening() const { return isopening_; }

 private:
  bool isopening_;
};