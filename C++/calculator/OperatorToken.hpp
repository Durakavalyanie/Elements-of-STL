#pragma once

#include <string>

#include "AbstractToken.hpp"
#include "OperandToken.hpp"

template <typename T, bool B>
class OperatorToken : public AbstractToken {};

template <typename T>
class OperatorToken<T, true> : public AbstractToken {
 public:
  OperatorToken(const std::string& input) : AbstractToken(input) {}

  virtual OperandToken<T>* Calculate(OperandToken<T>* lhs,
                                     OperandToken<T>* rhs) = 0;
};

template <typename T>
class OperatorToken<T, false> : public AbstractToken {
 public:
  OperatorToken(const std::string& input) : AbstractToken(input) {}

  virtual OperandToken<T>* Calculate(OperandToken<T>* operand) = 0;
};

template <typename T>
class OperatorMinusUn : public OperatorToken<T, false> {
 public:
  OperatorMinusUn(const std::string& input) : OperatorToken<T, false>(input) {}

  OperandToken<T>* Calculate(OperandToken<T>* operand) override;
};

template <typename T>
OperandToken<T>* OperatorMinusUn<T>::Calculate(OperandToken<T>* operand) {
  OperandToken<T>* new_operand = new OperandToken(-operand->GetValue());
  delete operand;
  return new_operand;
}

template <typename T>
class OperatorPlusBin : public OperatorToken<T, true> {
 public:
  OperatorPlusBin(const std::string& input) : OperatorToken<T, true>(input) {}

  OperandToken<T>* Calculate(OperandToken<T>* lhs,
                             OperandToken<T>* rhs) override;
};

template <typename T>
OperandToken<T>* OperatorPlusBin<T>::Calculate(OperandToken<T>* lhs,
                                               OperandToken<T>* rhs) {
  OperandToken<T>* operand =
      new OperandToken(lhs->GetValue() + rhs->GetValue());
  delete lhs;
  delete rhs;
  return operand;
}

template <typename T>
class OperatorMinusBin : public OperatorToken<T, true> {
 public:
  OperatorMinusBin(const std::string& input) : OperatorToken<T, true>(input) {}

  OperandToken<T>* Calculate(OperandToken<T>* lhs,
                             OperandToken<T>* rhs) override;
};

template <typename T>
OperandToken<T>* OperatorMinusBin<T>::Calculate(OperandToken<T>* lhs,
                                                OperandToken<T>* rhs) {
  OperandToken<T>* operand =
      new OperandToken(rhs->GetValue() - lhs->GetValue());
  delete lhs;
  delete rhs;
  return operand;
}

template <typename T>
class OperatorDivide : public OperatorToken<T, true> {
 public:
  OperatorDivide(const std::string& input) : OperatorToken<T, true>(input) {}

  OperandToken<T>* Calculate(OperandToken<T>* lhs,
                             OperandToken<T>* rhs) override;
};

template <typename T>
OperandToken<T>* OperatorDivide<T>::Calculate(OperandToken<T>* lhs,
                                              OperandToken<T>* rhs) {
  OperandToken<T>* operand =
      new OperandToken(rhs->GetValue() / lhs->GetValue());
  delete lhs;
  delete rhs;
  return operand;
}

template <typename T>
class OperatorMultiply : public OperatorToken<T, true> {
 public:
  OperatorMultiply(const std::string& input) : OperatorToken<T, true>(input) {}

  OperandToken<T>* Calculate(OperandToken<T>* lhs,
                             OperandToken<T>* rhs) override;
};

template <typename T>
OperandToken<T>* OperatorMultiply<T>::Calculate(OperandToken<T>* lhs,
                                                OperandToken<T>* rhs) {
  OperandToken<T>* operand =
      new OperandToken(lhs->GetValue() * rhs->GetValue());
  delete lhs;
  delete rhs;
  return operand;
}