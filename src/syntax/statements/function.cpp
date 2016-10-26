#include "function.hpp"

namespace syntax
{
    Function::Function() 
    {
    }
    Function::Function(vector<vector<shared_ptr<Symbol>>> symbol_groups)
    {
        print(symbol_groups.size());
        identifier = symbol_groups[0][0]->name();
        for (auto argname : symbol_groups[1])
        {
            argnames.push_back(argname->name());
        }
        body = symbol_groups[2];
        return_expression = symbol_groups[3][0];
    }

    string Function::representation()
    {
        string args = "";
        for (int i =0; i < argnames.size(); i++)
        { 
            args += (argnames[i]);
            if (i+1 != argnames.size()) //If not on last iteration
            {
                args += ", ";
            }
        }

        string body_string = "";
        for (auto statement : body)
        {
            body_string += statement->representation();
        }

        return "Function " + identifier + ":\n\tArguments: (" + args + ")\n\tBody: " + body_string + "\n\tReturns: " + return_expression->representation() + ")";
    }
    string Function::name()
    {
        return "*" + identifier;
    }
    string Function::source(unordered_set<string>& names)
    {
        /*
        template<class T, class U>
        auto add(T t, U u) -> decltype(t + u) // the return type is the type of operator+(T, U)
        {
            return t + u;
        }
        */

        //string templateline = "template <";
        string arglist = "";
        for (int i =0; i < argnames.size(); i++)
        { 
            arglist += ("auto " + argnames[i]);
            if (i+1 != argnames.size()) //If not on last iteration
            {
                arglist += ", ";
            }
        }
        string body_source = "";
        for (auto statement : generate(body))
        {
            body_source += statement;
        }

        string return_source = return_expression->source(names);
        print("RETURN SOURCE");
        print(return_source);

        return "auto " + identifier + "(" + arglist + ") -> decltype(" + return_source + ")\n{\n" + body_source + "return " + return_source + ";\n}";
    }
}