#include "grammar.hpp"
#include <exception>

namespace Grammar
{

const unordered_map<string, StatementConstructor> Grammar::construction_map = {
        {"expression", 
            [](vector<shared_ptr<Symbol>> tokens)
            {
                Expression e;
                print(tokens.size());
                e.base = tokens[0];
                if (tokens.size() > 1)
                {
                    if ((tokens.size() - 1) % 2 != 0)
                    {
                        print("Cannot build expression extension from odd number of tokens");
                        throw exception();
                    }

                    for (int i = 1; i < tokens.size(); i += 2)
                    {
                        e.extensions.push_back(make_tuple(tokens[i], tokens[i + 1]));
                    }
                }

                return make_shared<Expression>(e);
            }
        },
        {"assignment",
            [](vector<shared_ptr<Symbol>> tokens)
            {
                return make_shared<Assignment>(Assignment(tokens));
            }
        },
        {"functioncall",
            [](vector<shared_ptr<Symbol>> tokens)
            {
                return make_shared<FunctionCall>(FunctionCall(tokens));
            }
        },
        {"value",
            [](vector<shared_ptr<Symbol>> tokens)
            { 
                return make_shared<Symbol>(Symbol());
            }
        },
        {"function",
            [](vector<shared_ptr<Symbol>> tokens)
            {
                return make_shared<Symbol>(Symbol());
            }
        }

   };

Grammar::Grammar::Grammar(vector<string> filenames, string directory)
{
    for (auto filename : filenames)
    {
        grammar_map[filename] = read(directory + filename);
    }
}

vector<shared_ptr<Symbol>> Grammar::constructFrom(SymbolicTokens& tokens)
{
    vector<shared_ptr<Symbol>> symbols;

    while (tokens.size() > 0)
    {
        auto result = identify(tokens);
        print("Identified tokens as: " + get<0>(result));
        for (auto sub_result : get<1>(result))
        {
            for (auto t : sub_result.consumed)
            {
                print(t.value->representation());
            }
        }
        auto constructed = construct(get<0>(result), get<1>(result)); 
        symbols.push_back(constructed);
    }

    return symbols;
}


SymbolicTokenParsers Grammar::Grammar::readGrammarPairs(vector<string>& terms)
{
    SymbolicTokenParsers parsers;

    if (terms.size() % 2 != 0)
    {
        print("Could not read type pairs:");
        for (auto t : terms)
        {
            print(t + " ");
        }
        print("\n");
        throw exception();
    }
    for (int i = 0; i < (terms.size() / 2); i++)
    {
        int x = i * 2;
        vector<string> pair(terms.begin() + x, terms.begin() + x + 2);
        parsers.push_back(readGrammarTerms(pair));
    }

    return parsers;
}

SymbolicTokenParser Grammar::Grammar::readGrammarTerms(vector<string>& terms)
{
    SymbolicTokenParser parser;

    if (terms.size() == 2)
    {
        auto first = terms[0];
        bool keep  = true;
        // If first of pair starts with !, discard its parse result
        if (first[0] == '!')
        {
            first = string(first.begin() + 1, first.end());
            keep = false;
        }
        // Allow linking to other grammar files
        if (first == "link")
        {
            parser = retrieveGrammar(terms[1]);
        }
        // Parse by type only
        else if (terms[1] == "wildcard")
        {
            parser = typeParser(first);
        }
        // Parse by a specific subtype (ex "keyword return")
        else
        {
            if (first == "keyword")
            {
                keywords.push_back(terms[1]);
            }
            parser = dualTypeParser(first, terms[1]);
        }

        // Take care of a "!" if it was found early - make the parser discard its result
        if (not keep)
        {
            parser = discard(parser);
        }
    }
    else if (terms.size() > 2)
    {
        const auto keyword = terms[0];
        terms = vector<string>(terms.begin() + 1, terms.end());

        // Repeatedly parse a parser!
        if (keyword == "many")
        {
            parser = manySeperated(readGrammarTerms(terms)); 
        }
        // Optionally parse a parser
        else if (keyword == "optional")
        {
            parser = optional<SymbolicToken>(readGrammarTerms(terms));
        }
        else if (keyword == "annotate")
        // Annotate a parser for 
        {
            vector<string> remaining(terms.begin() + 1, terms.end());
            parser = annotate<SymbolicToken>(readGrammarTerms(remaining), terms[0]);
        }
        // Run several parsers in order, failing if any of them fail
        else if (keyword == "inOrder")
        {
            parser = inOrder<SymbolicToken>(readGrammarPairs(terms));
        }
        // Choose from several parsers
        else if (keyword == "anyOf")
        {
            parser = anyOf<SymbolicToken>(readGrammarPairs(terms));
        }
        else
        {
            print("Expected keyword...");
            throw exception();
        }
    }
    else
    {
        print("Grammar file incorrectly formatted: ");
        for (auto t : terms)
        {
            print(t + " ");
        }
        print("\n");
        throw exception();
    }

    return parser;
}

tuple<SymbolicTokenParsers, vector<int>> Grammar::Grammar::read(string filename)
{
    SymbolicTokenParsers parsers;
    auto content = readFile(filename);
    auto construct_line = content.back();
    content = vector<string>(content.begin(), content.end() - 1);
    
    for (auto line : content)
    {
        auto terms = Lex::seperate(line, {make_tuple(" ", false)});
        parsers.push_back(readGrammarTerms(terms));
    }

    vector<int> construct_indices;
    auto construct_terms = Lex::seperate(construct_line, {make_tuple(" ", false)});
    for (auto t : construct_terms)
    {
        construct_indices.push_back(stoi(t));
    }

    return make_tuple(parsers, construct_indices);
}

SymbolicTokenParser Grammar::Grammar::retrieveGrammar(string filename)
{
    SymbolicTokenParser grammar_parser = [filename, this](SymbolicTokens tokens)
    {
        SymbolicTokenParser parser;

        auto search = grammar_map.find(filename);
        if (search != grammar_map.end())
        {
             parser = annotate(inOrder<SymbolicToken>(get<0>(search->second)), filename);
        }
        else
        {
            print(filename + " is not an element of the grammar map");
            throw exception();
        }

        Result<SymbolicToken> result = parser(tokens);
        result.annotation = filename;
        return result;
    };
    return grammar_parser;
}


tuple<string, vector<Result<SymbolicToken>>> 
Grammar::identify
(SymbolicTokens& tokens)
{
    SymbolicTokens tokens_copy(tokens);

    vector<string> keys;
    keys.reserve(grammar_map.size());
    for (auto kv : grammar_map)
    {
        keys.push_back(kv.first);
    }

    // Sort keys by the lengths of the parsers they refer to
    sort(keys.begin(), keys.end(),
                      [this] (auto a, auto b) 
                      {
                          auto a_len = get<0>(grammar_map[a]).size();
                          auto b_len = get<0>(grammar_map[b]).size();
                          return a_len > b_len; 
                      });

    for (auto key : keys)
    {
        print("Attempting to identify as: " + key);

        auto value   = grammar_map[key];
        auto parsers = get<0>(value);
        auto result  = evaluateGrammar(parsers, tokens_copy);

        if (get<0>(result))
        {
            tokens = tokens_copy; // Apply our changes once we know the tokens were positively identified
            return make_tuple(key, get<1>(result));
        }
        else
        {
            tokens_copy = tokens;
        }
    }

    print("Could not identify tokens");
    throw exception();
}

tuple<bool, vector<Result<SymbolicToken>>> 
Grammar::evaluateGrammar
(SymbolicTokenParsers parsers, SymbolicTokens& tokens)
{
    vector<Result<SymbolicToken>> results;

    int i = 0;
    for (auto parser : parsers)
    {
        print("Parsing parser ", i ," against:");
        for (auto t : tokens)
        {
            print(t.value->representation());
        }
        auto result = parser(tokens);
        if (result.result)
        {
            tokens = result.remaining;
            results.push_back(result);
        }
        else
        {
            print("Failed on parser ", i ,". Remaining ", tokens.size(), " tokens were: ");
            for (auto t : tokens)
            {
                print(t.value->representation());
            }
            return make_tuple(false, results);
        }
        i++;
    }

    return make_tuple(true, results);
};


vector<shared_ptr<Symbol>> fromTokens(vector<SymbolicToken> tokens)
{
    vector<shared_ptr<Symbol>> symbols;
    symbols.reserve(tokens.size());

    for (auto t : tokens)
    {
        symbols.push_back(t.value);
    }

    return symbols;
}

shared_ptr<Symbol> Grammar::build(string name, vector<shared_ptr<Symbol>> symbols)
{
    StatementConstructor constructor;
    auto it = Grammar::construction_map.find(name);
    if (it != Grammar::construction_map.end())
        constructor = it->second;
    else
    {
        print(name + " is not an element of the construction map");
        throw exception();
    }

    auto constructed = constructor(symbols);
    return constructed;
}

shared_ptr<Symbol> Grammar::construct(string name, vector<Result<SymbolicToken>> results)
{
    print("Constructing " + name);
    auto construction_indices = get<1>(grammar_map[name]);

    vector<shared_ptr<Symbol>> result_symbols;

    for (auto i : construction_indices)
    {
        auto result = results[i];
        result.consumed = clean(result.consumed); // Discard tokens that have been marked as unneeded

        if (result.annotation == "none")
        {
            for (auto t : result.consumed)
            {
                result_symbols.push_back(t.value);
            }
        }
        else if (result.consumed.size() > 0) 
        {
            auto grouped_tokens = reSeperate(result.consumed);
            for (auto group : grouped_tokens)
            {
                print("Building sub-symbol " + result.annotation);
                auto constructed = build(result.annotation, fromTokens(group));
                result_symbols.push_back(constructed);
            }
        }
    }

    auto constructed = build(name, result_symbols);
    return constructed; 
}

}
