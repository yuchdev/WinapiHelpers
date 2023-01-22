#pragma once

typedef int(*sqlite3_callback)(void*, int, char**, char**);

struct sqlite3;

namespace helpers{

/// @briefVery lightweight header-only C++ RAII wrapper under SQlite3 ANSI C API
/// Class does not throw exceptions. It could be considered as not C++ way, 
/// however it increases safety in low-level application like drivers
class sqlite3_helper
{
public:

    /// @brief Empty-state sqlite3
    sqlite3_helper() = default;

    /// @brief Open or create sqlite3 database
    sqlite3_helper(const char* database_name);

    /// @brief Close database handle
    /// Important, if sqlite3_close() in close() method returned error, database remain opened
    /// This situation may occur if database is under backup right now.
    /// We neglect this situation, as we destroy the database object, we don't need it anymore
    ~sqlite3_helper();

    /// No copy
    sqlite3_helper(const sqlite3_helper&) = delete;

    /// No assignment
    sqlite3_helper& operator=(const sqlite3_helper&) = delete;

    /// @brief Move c-tor leaves rhs-object in empty state
    /// without closing the database handle
    sqlite3_helper(sqlite3_helper&& rhs);

    /// @brief Assignment operator leaves rhs-object in empty state
    /// without closing the database handle
    sqlite3_helper& operator=(sqlite3_helper&& rhs);


    /// @brief Open or create sqlite3 database
    /// @return: SQLite error code
    /// See https://www.sqlite.org/rescode.html for details
    int open(const char* database_name);

    /// @brief Close database handle
    /// If sqlite3_close() in close() method returned error, database remain opened
    /// It could be checked with get_last_error() and handle by the caller, 
    /// rather than look for solution inside the method. 
    /// Helper designed to be as lightweight as possible
    /// @return: SQLite error code
    /// See https://www.sqlite.org/rescode.html for details
    int close();

    /// @brief Execute SQL query
    /// @return: SQLite error code
    /// See https://www.sqlite.org/rescode.html for details
    int exec(const char* sql, sqlite3_callback callback = nullptr);

    /// @brief Is database in valid state
    operator bool() const;

    /// @brief Is database in valid state (handle is opened and last status is SQLITE_OK)
    bool is_valid() const;

    /// @brief Check if the current sqlite3 build is thread-safe
    static bool is_threadsafe();

    /// @brief Return last error code of any SQLite operation
    /// If the code is not SQLITE_OK, helper is not in valid state
    int get_last_error() const;

    /// @brief Return last error message based on error code 
    const char* get_last_error_message() const;

private:

    /// SQLite3 Handle
    sqlite3* db_ = nullptr;

    /// Last returned error code
    int current_return_code_ = 0;
};

} // namespace helpers
