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



#ifdef USE_SQLITE

#include "DatabaseSqlite.h"
#include "Logger.h"

namespace DB {

DatabaseSqlite::DatabaseSqlite(const std::string& file) :
    Database()
{
    connected_ = false;

    std::fstream fin(file.c_str(), std::ios::in | std::ios::binary);
    if (fin.fail()) {
        LOG_ERROR << "Failed to initialize SQLite connection. File " << file <<
            " does not exist." << std::endl;
        return;
    }
    fin.close();

    if (sqlite3_open(file.c_str(), &handle_) != SQLITE_OK)
    {
        LOG_ERROR << "Failed to initialize SQLite connection." << std::endl;
        sqlite3_close(handle_);
    }
    else
        connected_ = true;
}

DatabaseSqlite::~DatabaseSqlite()
{
    sqlite3_close(handle_);
}

bool DatabaseSqlite::GetParam(DBParam param)
{
    switch (param)
    {
    case DBPARAM_MULTIINSERT:
        return false;
    default:
        return false;
    }
}

bool DatabaseSqlite::BeginTransaction()
{
    return ExecuteQuery("BEGIN");
}

bool DatabaseSqlite::Rollback()
{
    return ExecuteQuery("ROLLBACK");
}

bool DatabaseSqlite::Commit()
{
    return ExecuteQuery("COMMIT");
}

std::string DatabaseSqlite::Parse(const std::string& s)
{
    std::string query = "";

    query.reserve(s.size());
    bool inString = false;
    uint8_t ch;
    for (uint32_t a = 0; a < s.length(); a++)
    {
        ch = s[a];

        if (ch == '\'')
        {
            if (inString && s[a + 1] != '\'')
                inString = false;
            else
                inString = true;
        }

        if (ch == '`' && !inString)
            ch = '"';

        query += ch;
    }

    return query;
}

bool DatabaseSqlite::InternalQuery(const std::string &query)
{
    std::lock_guard<std::recursive_mutex> lockClass(lock_);

    if (!connected_)
        return false;

#ifdef DEBUG_SQL
    LOG_DEBUG << "SQLITE QUERY: " << query << std::endl;
#endif

    std::string buf = Parse(query);
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(handle_, buf.c_str(), (int)buf.length(), &stmt, NULL) != SQLITE_OK)
    {
        sqlite3_finalize(stmt);
        LOG_ERROR << "sqlite3_prepare_v2(): SQLITE ERROR: " << sqlite3_errmsg(handle_) << " (" << buf << ")" << std::endl;
        return false;
    }

    int ret = sqlite3_step(stmt);
    if (ret != SQLITE_OK && ret != SQLITE_DONE && ret != SQLITE_ROW)
    {
        sqlite3_finalize(stmt);
        LOG_ERROR << "sqlite3_step(): SQLITE ERROR: " << sqlite3_errmsg(handle_) << " (" << buf << ")" << std::endl;
        return false;
    }

    sqlite3_finalize(stmt);

    return true;
}

std::shared_ptr<DBResult> DatabaseSqlite::InternalSelectQuery(const std::string &query)
{
    std::lock_guard<std::recursive_mutex> lockClass(lock_);

    if (!connected_)
        return std::shared_ptr<DBResult>();

#ifdef __DEBUG_SQL__
    LOG_DEBUG << "SQLITE QUERY: " << query << std::endl;
#endif

    std::string buf = Parse(query);
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(handle_, buf.c_str(), (int)buf.length(), &stmt, NULL) != SQLITE_OK)
    {
        sqlite3_finalize(stmt);
        LOG_ERROR << "sqlite3_prepare_v2(): SQLITE ERROR: " << sqlite3_errmsg(handle_) << " (" << buf << ")" << std::endl;
        return std::shared_ptr<DBResult>();
    }

    std::shared_ptr<DBResult> results(new SqliteResult(stmt),
        std::bind(&Database::FreeResult, this, std::placeholders::_1));
    return VerifyResult(results);
}

uint64_t DatabaseSqlite::GetLastInsertId()
{
    return (uint64_t)sqlite3_last_insert_rowid(handle_);
}

std::string DatabaseSqlite::EscapeString(const std::string &s)
{
    if (!s.size())
        return std::string("''");

    char* output = new char[s.length() * 2 + 3];
    sqlite3_snprintf((int)s.length() * 2 + 3, output, "%Q", s.c_str());
    std::string r(output);
    delete[] output;
    return r;
}

std::string DatabaseSqlite::EscapeBlob(const char* s, size_t length)
{
    std::stringstream r;
    r << "'" << base64::encode((const unsigned char*)s, length) << "'";
    return r.str();
}

void DatabaseSqlite::FreeResult(DBResult* res)
{
    delete (SqliteResult*)res;
}

SqliteResult::SqliteResult(sqlite3_stmt* res) :
    handle_(res),
    rowAvailable_(false)
{
    listNames_.clear();
    int32_t fields = sqlite3_column_count(handle_);
    for (int32_t i = 0; i < fields; i++)
    {
        listNames_[sqlite3_column_name(handle_, i)] = i;
    }
}

SqliteResult::~SqliteResult()
{
    sqlite3_finalize(handle_);
}

int32_t SqliteResult::GetInt(const std::string& col)
{
    ListNames::iterator it = listNames_.find(col);
    if (it != listNames_.end())
        return sqlite3_column_int(handle_, it->second);

    LOG_ERROR << "Error during GetInt(" << col << ")." << std::endl;
    return 0; // Failed
}

uint32_t SqliteResult::GetUInt(const std::string& col)
{
    ListNames::iterator it = listNames_.find(col);
    if (it != listNames_.end())
        return static_cast<uint32_t>(sqlite3_column_int(handle_, it->second));

    LOG_ERROR << "Error during GetUInt(" << col << ")." << std::endl;
    return 0; // Failed
}

int64_t SqliteResult::GetLong(const std::string& col)
{
    ListNames::iterator it = listNames_.find(col);
    if (it != listNames_.end())
        return sqlite3_column_int64(handle_, it->second);

    LOG_ERROR << "Error during GetLong(" << col << ")." << std::endl;
    return 0; // Failed
}

uint64_t SqliteResult::GetULong(const std::string& col)
{
    ListNames::iterator it = listNames_.find(col);
    if (it != listNames_.end())
        return static_cast<uint64_t>(sqlite3_column_int64(handle_, it->second));

    LOG_ERROR << "Error during GetULong(" << col << ")." << std::endl;
    return 0; // Failed
}

time_t SqliteResult::GetTime(const std::string& col)
{
    return static_cast<time_t>(GetLong(col));
}

std::string SqliteResult::GetString(const std::string& col)
{
    ListNames::iterator it = listNames_.find(col);
    if (it != listNames_.end())
    {
        return std::string((const char*)sqlite3_column_text(handle_, it->second));
    }

    LOG_ERROR << "Error during GetString(" << col << ")." << std::endl;
    return std::string(""); // Failed
}

std::string SqliteResult::GetStream(const std::string& col)
{
    ListNames::iterator it = listNames_.find(col);
    if (it != listNames_.end())
    {
        const char* value = (const char*)sqlite3_column_blob(handle_, it->second);
        int size = sqlite3_column_bytes(handle_, it->second);
        return base64::decode(value, size);
    }

    LOG_ERROR << "Error during GetStream(" << col << ")." << std::endl;
    return std::string(""); // Failed
}

bool SqliteResult::IsNull(const std::string& col)
{
    ListNames::iterator it = listNames_.find(col);
    if (it != listNames_.end())
        return sqlite3_column_type(handle_, it->second) == SQLITE_NULL;

    LOG_ERROR << "Error during IsNull(" << col << ")." << std::endl;
    return true; // Failed
}

std::shared_ptr<DBResult> SqliteResult::Next()
{
    // checks if after moving to next step we have a row result
    bool rowAvail = (sqlite3_step(handle_) == SQLITE_ROW);
    return rowAvail ? shared_from_this() : std::shared_ptr<DBResult>();
}

}

#endif
