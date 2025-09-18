#pragma once

#include <utility>

namespace incom::crossplatform::console {
std::pair<int, int> get_rowColCount();
bool                is_inTerminal();
bool                is_stdin_inTerminal();
bool                is_stdout_inTerminal();
void                set_cocp();

} // namespace incom::crossplatform::console
