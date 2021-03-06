/// Copyright 2017 Lucas Saldyt
#include "lexmap.hpp"

namespace lex
{
    LexMapLexer::LexMapLexer(Matcher<string> set_match_function, string set_name, string set_type, int set_precedence)
    {
        match      = set_match_function;
        name       = set_name;
        type       = set_type;
        precedence = set_precedence;
    }

    /**
     * Construct a LexMap from term sets, language lexers, and whitespace seperators
     * Term sets are sets of language lexers corresponding to a particular type of term, i.e. operators
     */
    LexMap::LexMap(const LexMapTermSets& set_term_sets, 
               const vector<LexMapLexer>&  set_language_lexers, 
               vector<Seperator> whitespace,
               vector<char>      set_string_delimiters,
               string            set_ml_comment_delimiter,
               string            set_comment_delimiter
            )
        : language_term_sets(set_term_sets),
        string_delimiters(set_string_delimiters),
        multiline_comment_delimiter(set_ml_comment_delimiter),
        comment_delimiter(set_comment_delimiter)
    {
        print("Creating lexmap for language");
        // Always seperate by whitespace
        concat(seperators, whitespace);
        if (whitespace.size() > 3)
        {
            newline = get<1>(whitespace[3]);
        }

        // Custom language lexers (like int, vector<string>, etc..)
        concat(language_lexers, set_language_lexers);

        vector<tuple<string, string, int>> term_lexers;

        // Add term sets (like operators) to a language's seperators
        for (auto term_set : language_term_sets)
        {
            for (auto term : get<0>(term_set))
            {
                term_lexers.push_back(make_tuple(term, get<1>(term_set), get<2>(term_set))); 
            }
        }

        std::sort(term_lexers.begin(), term_lexers.end(), 
            [](auto a, auto b){
                return get<0>(a).size() > get<0>(b).size(); 
            });

        for (auto term_lexer : term_lexers)
        {
            auto term     = get<0>(term_lexer);
            auto type     = get<1>(term_lexer);
            auto priority = get<2>(term_lexer);
            language_lexers.push_back(LexMapLexer(just(term), term, type, priority));
            if (type != "keyword") //seperating by keywords would make identifiers containing keywords impossible
            {
                seperators.push_back(make_tuple(term, true)); // Keep seperators from term sets (ie operators)
            }
        }

        // Enforce lexing precedence
        sortBy(language_lexers, [](auto &left, auto &right) {
                    return left.precedence < right.precedence;
                    });
        print("LexMap lexers (sorted by precedence): ");
        for (auto lexer : language_lexers)
        {
            print(lexer.name + " " + lexer.type + " (" + std::to_string(lexer.precedence) + ")");
        }
    }
    LexMap::LexMap(){}

    /**
     * Identify a vector of terms using the internal contents of a LexMap
     */
    tuple<Token, vector<string>> LexMap::identify(vector<string> terms) const
    {
        // Return result of first lexer to match against remaining terms
        for (auto lexer : language_lexers)
        {
            auto result = lexer.match(terms);
            if(result.result)
            {
                string text;
                for (auto term : result.consumed)
                {
                    text += term;
                }
                //print("vector<string> identified as " + lexer.name);
                string name = lexer.name == "*text*" ? text
                                                     : lexer.name;
                return make_tuple(Token(result.consumed, name, lexer.type), result.remaining);
            }
        }

        string message = "Could not identify terms: \n";
        for (auto t : terms)
        {
            message += "\"" + t + "\"\n";
        }
        throw named_exception(message);
    }
}
