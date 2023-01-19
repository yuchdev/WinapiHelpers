/* 2017-2020 WEBGEARS SERVICES LIMITED (c) All Rights Reserved. 
 * Proprietary and confidential.
 * The Software and any accompanying documentation are copyrighted and protected 
 * by copyright laws and international copyright treaties, as well as other 
 * intellectual property laws and treaties.
 * Unauthorized copying of this file, via any medium is strictly prohibited.
 */

#pragma once
#include <map>
#include <functional>
#include <helpers/is_callable.h>

namespace helpers {


/// @brief Policy class that passed to HandlerMap
/// Implement behavior when no handler associated with the key
/// Return default value or do nothing if the function is void()
/// See class specialization below
template <typename RetType>
class DefaultValuePolicy
{
public:

    /// @brief Return default value
    static RetType null_handler()
    {
        return RetType();
    }
};

/// @brief Do nothing if the function is void()
template <>
class DefaultValuePolicy<void>
{
public:

    static void null_handler()
    {
        return;
    }
};


/// @brief Policy class that passed to HandlerMap
/// Implement default behavior when the key is not found
/// It may lead to serious problems, so we throw here
template <typename RetType>
struct ThrowPolicy
{
    [[noreturn]]
    static RetType no_handler()
    {
        throw std::runtime_error("Inappropriate handler value");
    }
};


/// @brief Perform universal mapping Key type to executable Handler
/// @typename Key: any param that satisfies std::map key prerequisites (int, enum etc)
/// @typename Handler: any callable object (should have the same signature for one Map; Could be null)
/// @typename NoKeyPolicy: action when Key is not found
/// @typename NullHandlePolicy: action when Handle is NULL
template <typename Key, typename Handler, 
    template <typename RetType> class NoKeyPolicy = ThrowPolicy,
    template <typename RetType> class NullHandlePolicy = DefaultValuePolicy>
class HandlerMap
{
public:

    /// @brief Empty handlers map
    HandlerMap()
    {
        static_assert(helpers::IsCallable<Handler>::value, 
            "Second HandlerMap<> template param should be callable");
    }

    /// @brief Initialize handlers map with {}-notation
    HandlerMap(std::map<Key, Handler> other) : handler_map_(std::move(other)) {}

    /// @brief Initialize handlers map std::move() operation
    HandlerMap(std::map<Key, Handler>&& other) : handler_map_(std::move(other)) {}

    /// @brief Add new handler
    void insert(const Key key, Handler handler)
    {
        handler_map_[key] = handler;
    }

    /// @brief Add new handler in a functional way
    /// It could be chained like my_map(0, handler0)(1, handler1)...
    HandlerMap& operator()(const Key key, Handler handler)
    {
        handler_map_[key] = handler;
        return *this;
    }

    /// @brief Handler invoke point. Lookup handler by the key, pass params pack,
    /// return handler value. Perform Policy actions on non-existent Key and nullptr Handle
    template <typename... Args>
    auto call(const Key& key, Args&&... args) -> std::result_of_t<Handler(Args...)>
    {
        /// Check whether we have a return type in our handler
        using ResultType = std::result_of_t <Handler(Args...)>;

        // If handler does not exist just leave
        auto it = handler_map_.find(key);
        if (it == handler_map_.end()) {
            return NoKeyPolicy<ResultType>::no_handler();
        }

        // call handler if not 0
        auto callback = handler_map_.at(key);
        if (nullptr == callback) {
            return NullHandlePolicy<ResultType>::null_handler();
        }
        return callback(std::forward<Args>(args)...);
    }

    /// @brief Sometimes we have to deal with fixed number of handlers
    /// This method provide us simple runtime-check
    void throw_if_unexpected(size_t s)
    {
        if (handler_map_.size() != s) {
            throw std::runtime_error("Size of dynamic handler map is not same as expected");
        }
    }

    /// @brief Handlers count
    size_t size() const 
    {
        return handler_map_.size();
    }

private:

    /// Map of callable objects
    std::map<Key, Handler> handler_map_;
};

} // namespace helpers 
