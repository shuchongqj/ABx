#pragma once

#ifdef USE_PGSQL

#include "Database.h"
#include <libpq-fe.h>

namespace DB {

class DatabasePgsql final : public Database
{
protected:
    PGconn* handle_;
    bool InternalQuery(const std::string& query) final;
    std::shared_ptr<DBResult> InternalSelectQuery(const std::string& query) final;
    std::string Parse(const std::string& s);
public:
    DatabasePgsql();
    virtual ~DatabasePgsql();

    bool BeginTransaction() final;
    bool Rollback() final;
    bool Commit() final;

    bool GetParam(DBParam param) final;
    uint64_t GetLastInsertId() final;
    std::string EscapeString(const std::string& s) final;
    std::string EscapeBlob(const char* s, uint32_t length) final;
    void FreeResult(DBResult* res) final;
};

class PgsqlResult final : public DBResult
{
    friend class DatabasePgsql;
protected:
    PgsqlResult(PGresult* res);

    int32_t rows_, cursor_;
    PGresult* handle_;
public:
    virtual ~PgsqlResult();
    int32_t GetInt(const std::string& col) final;
    uint32_t GetUInt(const std::string& col) final;
    int64_t GetLong(const std::string& col) final;
    uint64_t GetULong(const std::string& col) final;
    time_t GetTime(const std::string& col) final;
    std::string GetString(const std::string& col) final;
    const char* GetStream(const std::string& col, unsigned long& size) final;
    bool IsNull(const std::string& col) final;

    bool Empty() const final { return cursor_ >= rows_; }
    std::shared_ptr<DBResult> Next() final;
};

}

#endif