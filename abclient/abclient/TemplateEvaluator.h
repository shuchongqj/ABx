/**
 * Copyright 2020 Stefan Ascher
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
 * of the Software, and to permit persons to whom the Software is furnished to do
 * so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#pragma once

#include <string>
#include <vector>
#include <string_view>

class Actor;
class TinyExpr;

class TemplateEvaluator
{
private:
    Actor& actor_;
public:
    TemplateEvaluator(Actor& actor);
    std::string Evaluate(const std::string& source);
};

class TemplateParser
{
public:
    enum class TokenType
    {
        Invalid,
        String,
        Expression
    };
    struct Token
    {
        TokenType type;
        size_t start;
        size_t end;
        std::string value;
    };
private:
    size_t index_{ 0 };
    std::string_view source_;
    bool Eof() const { return index_ >= source_.size(); }
    char Next() { return source_.at(index_++); }
    char Peek(size_t offset = 1) const
    {
        if (source_.size() > index_ + offset)
            return source_.at(index_ + offset);
        return 0;
    }
    Token GetNextToken();
public:
    std::vector<Token> Parse(std::string_view source);
};
