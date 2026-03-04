#include "driver/scanner_location_bridge.hpp"

#include "driver/driver.hpp"

namespace yy {

namespace {
NumDriver* g_active_driver = nullptr;
} // namespace

NumDriver* scanner_active_driver()
{
    return g_active_driver;
}

void scanner_set_active_driver(NumDriver* driver)
{
    g_active_driver = driver;
}

void scanner_advance_columns(std::size_t count)
{
    if (g_active_driver != nullptr) {
        g_active_driver->advance_columns(count);
    }
}

void scanner_advance_newlines(std::size_t count)
{
    if (g_active_driver != nullptr) {
        g_active_driver->advance_newlines(count);
    }
}

} // namespace yy
