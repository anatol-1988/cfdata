#pragma once
#include "entities.hpp"
#include <experimental/optional>

using std::mutex;
using std::vector;
using std::lock_guard;
using std::find_if;
using std::experimental::optional;
using std::experimental::nullopt;
using entities::Page;

class Queue
{
public:
    auto push_back(Page const& page) -> void
    {
        lock_guard<mutex> lock{_mutex};
        _pages.push_back(page);
    }

    auto get(size_t index) -> optional<Page>
    {
        lock_guard<mutex> lock{_mutex};
        auto const it
            = find_if(cbegin(_pages), cend(_pages), [index](auto const& p) {
                  return p.pagination.currentPage == index;
              });

        if (it == cend(_pages))
            return nullopt;

        auto const page = *it;
        _pages.erase(it);
        return page;
    }

private:
    vector<Page> _pages;
    mutex _mutex;
};
