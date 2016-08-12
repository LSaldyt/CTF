#include "Language.hpp"

namespace Lex
{
    LanguageParser::LanguageParser(ParseFunction set_parser, std::string set_name, std::string set_type, int set_precedence)
    {
        parser     = set_parser;
        name       = set_name;
        type       = set_type;
        precedence = set_precedence;
    }

    Language::Language(const LanguageTermSets& set_term_sets, const LanguageParsers&  set_language_parsers) {
        language_term_sets = set_term_sets;

        seperators.insert(seperators.end(), whitespace.begin(), whitespace.end());

        language_parsers.insert(language_parsers.end(), set_language_parsers.begin(), set_language_parsers.end());

        for (auto term_set : language_term_sets)
        {
            for (auto term : std::get<0>(term_set))
            {
                language_parsers.push_back(LanguageParser(just(term), term, std::get<1>(term_set), 1));
                if(std::get<1>(term_set) != "keyword") //seperating by keywords would make identifiers containing keywords impossible
                {
                    seperators.push_back(std::make_tuple(term, true));
                }
            }
        }

        std::sort(language_parsers.begin(), language_parsers.end(), [](auto &left, auto &right) {
                    return left.precedence < right.precedence;
                    });
        for (auto p : language_parsers)
        {
            std::cout << p.name << " " << p.type << std::endl;
        }
    }

    std::tuple<Token, Terms> Language::identify(Terms terms) const
    {
        for (auto parser : language_parsers)
        {
            std::cout << "Attempting to identify if term is " << parser.name << std::endl;
            auto result = parser.parser(terms);
            std::cout << "Parsed: " << std::endl;
            for (auto p : result.parsed)
            {
                std::cout << p << std::endl;
            }
            if(result.result)
            {
                std::cout << "Term identified" << std::endl;
                return std::make_tuple(Token(result.parsed, parser.name, parser.type), result.remaining);
            }
        }

        std::cout << "Could not identify terms:" << std::endl;
        for (auto t : terms)
        {
            std::cout << "\"" << t << "\"" << std::endl;
        }
        return std::make_tuple(Token({}, "unidentified", "failure"), Terms());
    }
}