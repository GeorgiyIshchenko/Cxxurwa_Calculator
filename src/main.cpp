#include <cassert>
#include <cmath>
#include <functional>
#include <iostream>
#include <memory>
#include <regex>
#include <unordered_map>

#include "statement.h"

class ConstOp : public Statement {
 public:
  ConstOp(int v) : Statement(0, 1, true), val(v) {};

  std::vector<int> apply(std::vector<int> in) const override {
    in.push_back(val);
    return in;
  }

 private:
  int val;
};

using funcIO = std::vector<int64_t>;

class Operation : public Statement {
 public:
  Operation(int a, int r, bool p, std::function<funcIO(funcIO)> func)
      : Statement(a, r, p), func(func) {};

  std::vector<int> apply(std::vector<int> in) const override {
    funcIO args;
    for (int i = 0; i < arguments; ++i) {
      int idx = in.size() - arguments + i;
      args.push_back(in[idx]);
    }
    for (int i = 0; i < arguments; ++i) {
      in.pop_back();
    }
    funcIO result = func(args);
    for (int i = 0; i < results; ++i) {
      in.push_back(result[i]);
    }
    return in;
  }

 private:
  std::function<funcIO(funcIO)> func;
};

// BinOps
Operation plus(2, 1, true,
               [](funcIO args) { return funcIO{args[0] + args[1]}; });
Operation minus(2, 1, true,
                [](funcIO args) { return funcIO{args[0] - args[1]}; });
Operation mult(2, 1, true,
               [](funcIO args) { return funcIO{args[0] * args[1]}; });
Operation divide(2, 1, true,
                 [](funcIO args) { return funcIO{args[0] / args[1]}; });
Operation percent(2, 1, true,
                  [](funcIO args) { return funcIO{args[0] % args[1]}; });
// Others
Operation myabs(1, 1, true, [](funcIO args) {
  return funcIO{args[0] >= 0 ? args[0] : -args[0]};
});
Operation input(0, 1, false, [](funcIO args) {
  (void)args;
  int buf;
  std::cin >> buf;
  return funcIO{buf};
});
Operation dublicate(1, 2, true,
                    [](funcIO args) { return funcIO{args[0], args[0]}; });

class Combine : public Statement {
 public:
  Combine(std::vector<std::shared_ptr<Statement>> ops = {})
      : Statement(0, 0, true), ops(ops) {};

  std::vector<int> apply(std::vector<int> in) const override {
    for (auto&& c : ops) {
      in = c->apply(in);
    }
    return in;
  }

  void append(std::shared_ptr<Statement> op) {
    pure &= op->is_pure();
    if (results < op->get_arguments_count()) {
      arguments += op->get_arguments_count() - results;
      results = op->get_results_count();
    } else {
      results += op->get_results_count() - op->get_arguments_count();
    }
    ops.push_back(op);
  }

  std::vector<std::shared_ptr<Statement>> ops;

};

std::regex constantRegex("^(\\+|\\-)?([0-9]+)$");
std::unordered_map<std::string, std::shared_ptr<Operation>> symbolMatch{
    {"+", std::make_shared<Operation>(plus)},
    {"-", std::make_shared<Operation>(minus)},
    {"*", std::make_shared<Operation>(mult)},
    {"/", std::make_shared<Operation>(divide)},
    {"%", std::make_shared<Operation>(percent)},
    {"abs", std::make_shared<Operation>(myabs)},
    {"input", std::make_shared<Operation>(input)},
    {"dup", std::make_shared<Operation>(dublicate)}};

std::shared_ptr<Statement> compile(std::string_view str) {
  auto result = std::make_shared<Combine>();
  for (int i = 0; i < str.size(); ++i) {
    std::string buf;
    while (i < str.size() && str[i] != ' ') {
      buf += str[i];
      ++i;
    }
    if (buf.empty()) continue;
    std::smatch numMatch;
    if (std::regex_match(buf, numMatch, constantRegex)) {
      int val = std::stoi(numMatch[2].str());
      int sign = numMatch[1] == "" || numMatch[1] == "+" ? 1 : -1;
      result->append(std::make_shared<ConstOp>(sign * val));
    } else {
      if (symbolMatch.count(buf)) {
        result->append(symbolMatch[buf]);
      }
    }
  }
  return result;
};

std::shared_ptr<Statement> operator|(std::shared_ptr<Statement> lhs,
                                     std::shared_ptr<Statement> rhs) {
  auto lhsCombine = dynamic_cast<Combine*>(lhs.get());
  auto rhsCombine = dynamic_cast<Combine*>(rhs.get());
  Combine result{*lhsCombine};
  for (auto&& rhsOp: rhsCombine->ops){
    result.ops.push_back(rhsOp);
  }
  return std::make_shared<Combine>(result);
};

#ifdef MYTEST

// TODO: optimize func
std::shared_ptr<Statement> optimize(std::shared_ptr<Statement> stmt) {
  return NULL;
}

void printvec(std::vector<int> vec) {
  for (auto&& i : vec) {
    std::cout << i << " ";
  };
  std::cout << std::endl;
}

int main(int argc, char** argv) {
  std::cout << "Hello, world!" << std::endl;

  auto plus = compile("+");
  auto minus = compile("-");
  auto odt = compile("-123");
  auto inc = compile("1 +");

  assert(plus->is_pure() && plus->get_arguments_count() == 2 &&
         plus->get_results_count() == 1);
  assert(inc->is_pure() && inc->get_arguments_count() == 1 &&
         inc->get_results_count() == 1);

  assert(plus->apply({2, 2}) == std::vector{4});
  assert(minus->apply({1, 2, 3}) == std::vector({1, -1}));

  auto plus_4 = inc | inc | inc | inc;

  assert(plus_4->is_pure() && plus_4->get_arguments_count() == 1 &&
         plus_4->get_results_count() == 1);
  assert(plus_4->apply({0}) == std::vector{4});
  assert(inc->apply({0}) == std::vector{1});

  auto dup = compile("dup");
  assert(dup->is_pure() && dup->get_arguments_count() == 1 &&
         dup->get_results_count() == 2);

  auto sqr = dup | compile("*");
  auto ten = compile("6") | plus_4;
  auto ten_apply = ten->apply({});
  auto apply_res = (ten | sqr)->apply({});
  assert(apply_res == std::vector{100});

  auto complicated_zero =
      compile(" 1    4  3 4   5  6 + -      - 3    / % -    ");
  assert(complicated_zero->is_pure() &&
         complicated_zero->get_arguments_count() == 0 &&
         complicated_zero->get_results_count() == 1);
  assert(complicated_zero->apply({}) == std::vector{0});

  for (int i = 0; i < 1000; ++i) {
    auto i_str = std::to_string(i);
    auto plus_i = "+" + i_str;
    auto minus_i = "-" + i_str;

    int res1 = compile(i_str)->apply({})[0];
    int res2 = compile(plus_i)->apply({})[0];
    int res3 = compile(minus_i)->apply({})[0];

    assert(res1 == i);
    assert(res2 == i);
    assert(res3 == -i);
  }

  auto nop = compile("");
  assert(nop->is_pure() && nop->get_arguments_count() == 0 &&
         nop->get_results_count() == 0);

  auto heh = compile(" 999 -9 - ");

  printvec(heh->apply({}));
}
#endif