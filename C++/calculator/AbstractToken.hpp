#pragma once
#include <string>

class AbstractToken {
 public:
  virtual ~AbstractToken() = default;

 protected:
  AbstractToken(const std::string& input) { string_view_ = input; };

  const std::string& GetStringToken() const { return string_view_; }

  std::string string_view_;
};