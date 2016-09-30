#include <unordered_map>
#include <string>

#include "grammar.hpp"

namespace Grammar
{
using StatementConstructor = std::function<std::shared_ptr<Statement>(std::vector<std::shared_ptr<Symbol>>)>;

std::vector<std::shared_ptr<Symbol>> fromTokens(std::vector<SymbolicToken>);

std::unordered_map<std::string, StatementConstructor> construction_map = {
        {"expression.grm", 
            [](std::vector<std::shared_ptr<Symbol>> tokens)
            {
                Expression e;
                e.base = tokens[0];
                if (tokens.size() > 1)
                {
                    if ((tokens.size() - 1) % 2 != 0)
                        throw std::exception();

                    for (int i = 1; i < tokens.size(); i += 2)
                    {
                        e.extensions.push_back(std::make_tuple(tokens[i], tokens[i + 1]));
                    }
                }

                return std::make_shared<Expression>(e);
            }
        },
        {"assignment.grm",
            [](std::vector<std::shared_ptr<Symbol>> tokens)
            {
                return std::make_shared<Assignment>(Assignment(tokens));
            }
        }
    };
}
