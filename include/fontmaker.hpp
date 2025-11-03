#pragma once

#include <sfntly/font.h>
#include <sfntly/font_factory.h>
#include <vector>


namespace incom::terminal_plot {
class FontMaker {
    sfntly::Font              primaryFont;
    std::vector<sfntly::Font> supplementaryFonts;

    
};
} // namespace incom::terminal_plot