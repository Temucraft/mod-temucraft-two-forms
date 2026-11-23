/*
 * Temucraft: a container whose single operations take a lock.
 *
 * Player and creature hooks run on several map threads at once (MapUpdate.Threads > 1). A plain unordered_map or
 * unordered_set shared by all players breaks when two threads add or remove at the same time. Locked<> wraps the
 * operations these modules use (operator[], find, count, insert, emplace, erase, clear, empty, size), each under its
 * own lock. A reference or iterator to one element stays good while other threads add elements (unordered containers
 * keep their nodes in place); each player's element is only removed by that player's own logout. Walking the whole
 * container (begin/end) is not locked: do that only where no map thread runs (world scripts, commands, startup).
 */
#ifndef TEMUCRAFT_LOCKED_H
#define TEMUCRAFT_LOCKED_H

#include <mutex>
#include <type_traits>
#include <unordered_map>
#include <unordered_set>
#include <utility>

namespace Temucraft
{
    template <class Base>
    class Locked : public Base
    {
    public:
        using Base::Base;
        using typename Base::key_type;
        using typename Base::iterator;
        using typename Base::const_iterator;
        using typename Base::size_type;

        auto& operator[](key_type const& key) { std::lock_guard<std::recursive_mutex> g(_lock); return Base::operator[](key); }
        iterator find(key_type const& key) { std::lock_guard<std::recursive_mutex> g(_lock); return Base::find(key); }
        const_iterator find(key_type const& key) const { std::lock_guard<std::recursive_mutex> g(_lock); return Base::find(key); }
        size_type count(key_type const& key) const { std::lock_guard<std::recursive_mutex> g(_lock); return Base::count(key); }
        bool contains(key_type const& key) const { std::lock_guard<std::recursive_mutex> g(_lock); return Base::find(key) != Base::end(); }
        auto insert(typename Base::value_type const& value) { std::lock_guard<std::recursive_mutex> g(_lock); return Base::insert(value); }
        template <class... Args> auto insert(Args&&... args) { std::lock_guard<std::recursive_mutex> g(_lock); return Base::insert(std::forward<Args>(args)...); }
        template <class... Args> auto emplace(Args&&... args) { std::lock_guard<std::recursive_mutex> g(_lock); return Base::emplace(std::forward<Args>(args)...); }
        template <class... Args> auto try_emplace(Args&&... args) { std::lock_guard<std::recursive_mutex> g(_lock); return Base::try_emplace(std::forward<Args>(args)...); }
        size_type erase(key_type const& key) { std::lock_guard<std::recursive_mutex> g(_lock); return Base::erase(key); }
        template <class It, class = std::enable_if_t<!std::is_convertible_v<It, key_type>>>
        auto erase(It it) { std::lock_guard<std::recursive_mutex> g(_lock); return Base::erase(it); }
        void clear() { std::lock_guard<std::recursive_mutex> g(_lock); Base::clear(); }
        bool empty() const { std::lock_guard<std::recursive_mutex> g(_lock); return Base::empty(); }
        size_type size() const { std::lock_guard<std::recursive_mutex> g(_lock); return Base::size(); }

    private:
        mutable std::recursive_mutex _lock;
    };

    template <class K, class V, class... Rest> using LockedMap = Locked<std::unordered_map<K, V, Rest...>>;
    template <class K, class... Rest> using LockedSet = Locked<std::unordered_set<K, Rest...>>;
}

#endif
