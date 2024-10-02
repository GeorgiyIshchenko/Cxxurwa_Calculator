#pragma once

#include <string_view>
#include <vector>
#include <memory>

class Statement {
public:
    virtual std::vector<int> apply(std::vector<int> in) const = 0;

    Statement(unsigned arguments, unsigned results, bool pure): arguments(arguments), results(results), pure(pure) {}

    virtual ~Statement() = default;

    bool is_pure() const {
        return pure;
    }

    unsigned get_arguments_count() const {
        return arguments;
    }

    unsigned get_results_count() const {
        return results;
    }
protected:
    unsigned arguments;
    unsigned results;
    bool pure;
};

std::shared_ptr<Statement> operator|(std::shared_ptr<Statement> lhs, std::shared_ptr<Statement> rhs);
std::shared_ptr<Statement> compile(std::string_view str);
std::shared_ptr<Statement> optimize(std::shared_ptr<Statement> stmt);
