/* 2017-2020 WEBGEARS SERVICES LIMITED (c) All Rights Reserved. 
 * Proprietary and confidential.
 * The Software and any accompanying documentation are copyrighted and protected 
 * by copyright laws and international copyright treaties, as well as other 
 * intellectual property laws and treaties.
 * Unauthorized copying of this file, via any medium is strictly prohibited.
 */

#include <helpers/sqlite3_helper.h>
#include <sqlite3.h>

using namespace helpers;

sqlite3_helper::sqlite3_helper(const char* database_name) :
    current_return_code_(sqlite3_open(database_name, &db_))
{

}

sqlite3_helper::sqlite3_helper(sqlite3_helper&& rhs) :
    db_(rhs.db_),
    current_return_code_(rhs.current_return_code_)
{
    db_ = rhs.db_;
    current_return_code_ = rhs.current_return_code_;
}

sqlite3_helper& sqlite3_helper::operator=(sqlite3_helper&& rhs)
{
    db_ = rhs.db_;
    current_return_code_ = rhs.current_return_code_;
    rhs.db_ = nullptr;
    rhs.current_return_code_ = 0;
    return (*this);
}

sqlite3_helper::~sqlite3_helper()
{
    close();
}

int sqlite3_helper::open(const char* database_name)
{
    current_return_code_ = sqlite3_open(database_name, &db_);
    return current_return_code_;
}

int sqlite3_helper::close()
{
    current_return_code_ = sqlite3_close(db_);
    if (current_return_code_ == SQLITE_OK) {
        db_ = nullptr;
    }
    return current_return_code_;
}

int sqlite3_helper::exec(const char* sql, sqlite3_callback callback /*= nullptr*/)
{
    current_return_code_ = sqlite3_exec(db_, sql, callback, nullptr, nullptr);
    return current_return_code_;
}

sqlite3_helper::operator bool() const
{
    return is_valid();
}

bool sqlite3_helper::is_valid() const
{
    return (db_ != nullptr) && (current_return_code_ == SQLITE_OK);
}

bool sqlite3_helper::is_threadsafe()
{
    return sqlite3_threadsafe();
}

int helpers::sqlite3_helper::get_last_error() const
{

    return current_return_code_;
}

const char* helpers::sqlite3_helper::get_last_error_message() const
{
    return sqlite3_errstr(current_return_code_);
}
