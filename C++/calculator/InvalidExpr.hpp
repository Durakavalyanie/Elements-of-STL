#pragma once

#include <exception>

struct InvalidExpr : std::exception {
  [[nodiscard]] const char* what() const noexcept override {
    return "Invalid expression!";
  }
};