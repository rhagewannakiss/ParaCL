#include "Visitors/detail/ScopeGuard.hpp"

namespace ast::detail {

ScopeGuard::ScopeGuard(VarTable& table, const SourceRange& location)
  : table_(table)
  , location_(location)
{
    table_.enter_scope();
}

ScopeGuard::~ScopeGuard() noexcept
{
    table_.leave_scope(location_);
}

} // namespace ast::detail
