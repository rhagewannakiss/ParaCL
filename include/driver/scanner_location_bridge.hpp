#ifndef SCANNER_LOCATION_BRIDGE_HPP
#define SCANNER_LOCATION_BRIDGE_HPP

#include <cstddef>

namespace yy {

class NumDriver;

NumDriver* scanner_active_driver();
void scanner_set_active_driver(NumDriver* driver);
void scanner_advance_columns(std::size_t count);
void scanner_advance_newlines(std::size_t count);

} // namespace yy

#endif // SCANNER_LOCATION_BRIDGE_HPP
