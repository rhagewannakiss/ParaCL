#include "Visitors/detail/ScopeGuard.hpp"

#include <exception>

namespace ast::detail {

ScopeGuard::ScopeGuard(VarTable& table, SourceRange location)
  : table_(table)
  , location_(location)
{
    table_.enter_scope();
}

ScopeGuard::~ScopeGuard() noexcept
{
    try {
        table_.leave_scope(location_);
    } catch (...) {
        std::terminate();
    }
}

} // namespace ast::detail
