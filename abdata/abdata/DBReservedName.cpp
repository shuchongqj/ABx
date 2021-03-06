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


#include "DBReservedName.h"
#include "StorageProvider.h"

namespace DB {

// Player names are case insensitive. The DB needs a proper index for that:
// CREATE INDEX reserved_names_name_ci_index ON reserved_names USING btree (lower(name))

bool DBReservedName::Create(AB::Entities::ReservedName& rn)
{
    if (Utils::Uuid::IsEmpty(rn.uuid))
    {
        LOG_ERROR << "UUID is empty" << std::endl;
        return false;
    }

    Database* db = GetSubsystem<Database>();
    std::ostringstream query;
    query << "INSERT INTO `reserved_names` (`uuid`, `name`, `is_reserved`, `reserved_for_account_uuid`, `expires`";
    query << ") VALUES (";

    query << db->EscapeString(rn.uuid) << ", ";
    query << db->EscapeString(rn.name) << ", ";
    query << (rn.isReserved ? 1 : 0) << ", ";
    query << db->EscapeString(rn.reservedForAccountUuid) << ", ";
    query << rn.expires;

    query << ")";

    DBTransaction transaction(db);
    if (!transaction.Begin())
        return false;

    if (!db->ExecuteQuery(query.str()))
        return false;

    // End transaction
    if (!transaction.Commit())
        return false;

    return true;
}

bool DBReservedName::Load(AB::Entities::ReservedName& n)
{
    Database* db = GetSubsystem<Database>();

    std::ostringstream query;
    query << "SELECT * FROM `reserved_names` WHERE ";
    if (!Utils::Uuid::IsEmpty(n.uuid))
        query << "`uuid` = " << db->EscapeString(n.uuid);
    else if (!n.name.empty())
        query << "LOWER(`name`) = LOWER(" << db->EscapeString(n.name) << ")";
    else
    {
        LOG_ERROR << "UUID and name are empty" << std::endl;
        return false;
    }

    std::shared_ptr<DB::DBResult> result = db->StoreQuery(query.str());
    if (!result)
        return false;

    n.uuid = result->GetString("uuid");
    n.name = result->GetString("name");
    n.isReserved = result->GetInt("is_reserved") != 0;
    n.reservedForAccountUuid = result->GetString("reserved_for_account_uuid");
    n.expires = result->GetLong("expires");

    return true;
}

bool DBReservedName::Save(const AB::Entities::ReservedName& rn)
{
    if (Utils::Uuid::IsEmpty(rn.uuid))
    {
        LOG_ERROR << "UUID is empty" << std::endl;
        return false;
    }

    Database* db = GetSubsystem<Database>();
    std::ostringstream query;

    query << "UPDATE `reserved_names` SET ";

    // Only these may be changed
    query << " `is_reserved` = " << (rn.isReserved ? 1 : 0) << ", ";
    query << " `expires` = " << rn.expires;

    query << " WHERE `uuid` = " << db->EscapeString(rn.uuid);

    DBTransaction transaction(db);
    if (!transaction.Begin())
        return false;

    if (!db->ExecuteQuery(query.str()))
        return false;

    // End transaction
    return transaction.Commit();
}

bool DBReservedName::Delete(const AB::Entities::ReservedName& rn)
{
    if (Utils::Uuid::IsEmpty(rn.uuid))
    {
        LOG_ERROR << "UUID is empty" << std::endl;
        return false;
    }

    Database* db = GetSubsystem<Database>();
    std::ostringstream query;
    query << "DELETE FROM `reserved_names` WHERE `uuid` = " << db->EscapeString(rn.uuid);
    DBTransaction transaction(db);
    if (!transaction.Begin())
        return false;

    if (!db->ExecuteQuery(query.str()))
        return false;

    // End transaction
    return transaction.Commit();
}

bool DBReservedName::Exists(const AB::Entities::ReservedName& n)
{
    Database* db = GetSubsystem<Database>();

    std::ostringstream query;
    query << "SELECT COUNT(*) AS `count` FROM `reserved_names` WHERE ";
    if (!Utils::Uuid::IsEmpty(n.uuid))
        query << "`uuid` = " << db->EscapeString(n.uuid);
    else if (!n.name.empty())
        query << "LOWER(`name`) = LOWER(" << db->EscapeString(n.name) << ")";
    else
    {
        LOG_ERROR << "UUID and name are empty" << std::endl;
        return false;
    }
    if (n.isReserved)
        query << " AND `is_reserved` = 1";

    std::shared_ptr<DB::DBResult> result = db->StoreQuery(query.str());
    if (!result)
        return false;
    return result->GetUInt("count") != 0;
}

void DBReservedName::DeleteExpired(StorageProvider* sp)
{
    // When expires == 0 it does not expire, otherwise it's the time stamp
    Database* db = GetSubsystem<Database>();
    std::ostringstream query;
    query << "SELECT `uuid` FROM `reserved_names` WHERE ";
    query << "(`expires` <> 0 AND `expires` < " << Utils::Tick() << ")";

    std::shared_ptr<DB::DBResult> result = db->StoreQuery(query.str());
    if (!result)
        // No matches
        return;

    std::vector<std::string> rns;
    for (result = db->StoreQuery(query.str()); result; result = result->Next())
    {
        rns.push_back(result->GetString("uuid"));
    }

    // First invalidate them
    for (const std::string& g : rns)
    {
        AB::Entities::ReservedName rn;
        rn.uuid = g;
        sp->EntityInvalidate(rn);
    }

    // Then delete from DB
    query.str("");
    query << "DELETE FROM `reserved_names` WHERE ";
    query << "(`expires` <> 0 AND `expires` < " << Utils::Tick() << ")";

    DBTransaction transaction(db);
    if (!transaction.Begin())
        return;

    if (!db->ExecuteQuery(query.str()))
        return;

    // End transaction
    if (!transaction.Commit())
        return;
}

}
