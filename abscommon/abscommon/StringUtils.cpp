/**
 * Copyright 2017-2020 Stefan Ascher
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


#include "StringUtils.h"
#include <sstream>
#include <locale>
#include <codecvt>

namespace Utils {

#ifdef AB_WINDOWS
static const std::locale locale_utf8("");
#else
static const std::locale locale_utf8("en_US.UTF-8");
#endif

bool StringEquals(const std::string& l, const std::string& r)
{
    return l.size() == r.size()
        && std::equal(l.cbegin(), l.cend(), r.cbegin(),
            [](std::string::value_type l1, std::string::value_type r1)
    {
        return toupper(l1) == toupper(r1);
    });
}

bool StringEquals(const std::wstring& l, const std::wstring& r)
{
    return l.size() == r.size()
        && std::equal(l.cbegin(), l.cend(), r.cbegin(),
            [](std::wstring::value_type l1, std::wstring::value_type r1)
    {
        return std::tolower<wchar_t>(l1, locale_utf8) == std::tolower<wchar_t>(r1, locale_utf8);
    });
}

std::wstring Utf8ToWString(const std::string& utf8)
{
    // FIXME: Deprecated in C++17
    std::wstring_convert<std::codecvt_utf8<wchar_t>> convert;
    return convert.from_bytes(utf8);
}

std::string WStringToUtf8(const std::wstring& wstr)
{
    // FIXME: Deprecated in C++17
    std::wstring_convert<std::codecvt_utf8<wchar_t>> convert;
    return convert.to_bytes(wstr);
}

bool SameName(const std::string& l, const std::string& r)
{
    const std::wstring wl = Utf8ToWString(l);
    const std::wstring wr = Utf8ToWString(r);
    return StringEquals(wl, wr);
}

std::string Utf8ToLower(const std::string& str)
{
    std::wstring result = Utf8ToWString(str);
    std::transform(result.begin(), result.end(), result.begin(),
        [](wchar_t c) { return std::tolower<wchar_t>(c, locale_utf8); });
    return WStringToUtf8(result);
}

bool SameFilename(const std::string& l, const std::string& r)
{
#if defined(AB_WINDOWS)
    return StringEquals(l, r);
#else
    return l.compare(r) == 0;
#endif
}

std::string ChangeFileExt(const std::string& fn, const std::string& ext)
{
    size_t pos = fn.find_last_of('.');
    if (pos != std::string::npos)
        return fn.substr(0, pos) + ext;
    return fn + ext;
}

std::string GetFileExt(const std::string& fn)
{
    size_t pos = fn.find_last_of('.');
    if (pos != std::string::npos)
        return fn.substr(pos);
    return "";
}

std::string ExtractFileDir(const std::string& fn)
{
    size_t pos = fn.find_last_of("\\/");
    return fn.substr(0, pos);
}

std::string ExtractFileName(const std::string& fn)
{
    size_t pos = fn.find_last_of("\\/");
    if (pos != std::string::npos)
        return fn.substr(pos + 1);
    return fn;
}

std::string ConcatPath(const std::string& path, const std::string& name, const std::string& ext /* = "" */)
{
    std::string result = path;
    if (result.back() != '/' && result.back() != '\\' && name.front() != '/' && name.front() != '\\')
        result += '/';
    result += name;
    if (!ext.empty())
        result += ext;
    return result;
}

bool IsNumber(const std::string& s)
{
    std::string::const_iterator it = s.begin();
    while (it != s.end() && std::isdigit(*it)) ++it;
    return !s.empty() && it == s.end();
}

std::string ConvertIPToString(uint32_t ip, bool mask /* = false */)
{
    unsigned char bytes[4];
    bytes[0] = ip & 0xFF;
    bytes[1] = (ip >> 8) & 0xFF;
    bytes[2] = (ip >> 16) & 0xFF;
    bytes[3] = (ip >> 24) & 0xFF;

    char buffer[INET_ADDRSTRLEN];
    if (!mask)
#ifdef _MSC_VER
        sprintf_s(buffer, INET_ADDRSTRLEN, "%u.%u.%u.%u", bytes[3], bytes[2], bytes[1], bytes[0]);
#else
        sprintf(buffer, "%u.%u.%u.%u", bytes[3], bytes[2], bytes[1], bytes[0]);
#endif
    else
#ifdef _MSC_VER
        sprintf_s(buffer, INET_ADDRSTRLEN, "%u.%u.x.x", bytes[3], bytes[2]);
#else
        sprintf(buffer, "%u.%u.x.x", bytes[3], bytes[2]);
#endif
    return buffer;
}

uint32_t ConvertStringToIP(const std::string& ip)
{
    unsigned int byte3;
    unsigned int byte2;
    unsigned int byte1;
    unsigned int byte0;

#ifdef _MSC_VER
    if (sscanf_s(ip.c_str(), "%u.%u.%u.%u", &byte3, &byte2, &byte1, &byte0) == 4)
#else
    if (sscanf(ip.c_str(), "%u.%u.%u.%u", &byte3, &byte2, &byte1, &byte0) == 4)
#endif
    {
        if ((byte3 < 256)
            && (byte2 < 256)
            && (byte1 < 256)
            && (byte0 < 256))
        {
            return (byte3 << 24)
                + (byte2 << 16)
                + (byte1 << 8)
                + byte0;
        }
    }

    return 0;
}

static float RoundOff(float n)
{
    const float d = n * 100.0f;
    const int i = static_cast<int>(d + 0.5f);
    return static_cast<float>(i) / 100.0f;
}

std::string ConvertSize(size_t size)
{
    static const char* SIZES[] = { "B", "KB", "MB", "GB" };
    size_t div = 0;
    size_t rem = 0;

    while (size >= 1024 && div < (sizeof SIZES / sizeof *SIZES))
    {
        rem = (size % 1024);
        div++;
        size /= 1024;
    }

    float size_d = static_cast<float>(size) + static_cast<float>(rem) / 1024.0f;
    std::ostringstream convert;
    convert << RoundOff(size_d);
    std::string result = convert.str() + " " + SIZES[div];
    return result;
}

std::string UrlEncode(const std::string& str)
{
    std::string new_str = "";
    char c;
    int ic;
    const char* chars = str.c_str();
    char bufHex[10];
    size_t len = strlen(chars);

    for (size_t i = 0; i<len; i++)
    {
        c = chars[i];
        ic = c;
        // uncomment this if you want to encode spaces with +
        if (c == ' ')
            new_str += '+';
        else if (isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~')
            new_str += c;
        else
        {
            sprintf(bufHex, "%X", c);
            if (ic < 16)
                new_str += "%0";
            else
                new_str += "%";
            new_str += bufHex;
        }
    }
    return new_str;
}

std::string UrlDecode(const std::string& str)
{
    std::string ret;
    char ch;
    size_t i, len = str.length();
    unsigned ii;

    for (i = 0; i < len; i++)
    {
        if (str[i] != '%')
        {
            if (str[i] == '+')
                ret += ' ';
            else
                ret += str[i];
        }
        else
        {
            sscanf(str.substr(i + 1, 2).c_str(), "%x", &ii);
            ch = static_cast<char>(ii);
            ret += ch;
            i += 2;
        }
    }
    return ret;
}

std::string EscapeArguments(const std::string& value)
{
    const size_t spacepos = value.find(' ');
    if (spacepos != std::string::npos)
        return "\"" + value + "\"";
    const size_t tabpos = value.find('\t');
    if (tabpos != std::string::npos)
        return "\"" + value + "\"";

    return value;
}

}
