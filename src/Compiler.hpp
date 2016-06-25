#pragma once
#include "Utilities/IO/IO.hpp"
#include "Utilities/Parse/Parse.hpp"
#include "Utilities/Syntax/Symbols.hpp"
#include "Utilities/Syntax/Token.hpp"
#include "Utilities/Lex/Lex.hpp"
#include "Utilities/Lex/Seperate.hpp"
#include "Utilities/Language.hpp"
#include <iostream>

namespace Compiler
{
    using namespace Parse;
    using namespace Lex;
    using namespace Syntax;

    std::vector<Tokens> tokenPass(std::vector<std::string>, const Language&);
    std::vector<SymbolicTokens> symbolicPass(std::vector<Tokens> tokens);
    SymbolicTokens join(std::vector<SymbolicTokens>);

    const auto makeTypeParserFromStrings = [](std::vector<std::string> strings)
    {
        using namespace Parse;
        using namespace Syntax;
        return typeParser<SymbolicToken>(inOrder(justFrom(strings)));
    };

    const auto makeTypeParser = [](std::vector<ParseFunction> functions)
    {
        using namespace Parse;
        using namespace Syntax;
        return typeParser<SymbolicToken>(inOrder(functions));
    };
}
