<div align="center">
Incplot is a command like tool for drawing great looking plots in the terminal using unicode characters.

Automatically infers what to display and how based on the data piped in.

'As quick and as easy as possible' visualization of small to moderate sized data and information.

</div>

## Features ##

* Great looking plots 'out of the box'
* Zero configuration required, super simple to use
* No command line arguments necessary most of the time due to automatic inferrence
* User can optionally specify some or all arguments in any (sensible) combination
* Accepts piped in data in [JSON Lines](https://jsonlines.org/) or [NDJSON](https://github.com/ndjson) flat format
* Very fast
* Full true color support (eg. useful for scatter plots)
* Automatic sizing
* Automatic value labels and scaling with [metric prefixes](https://en.wikipedia.org/wiki/Metric_prefix)
* Automatic tickmarks
* Automatic legend
* Reasonable error messages (especially when the user asks for something that is impossible/unsupported)

## Non-Features ##

* No data wrangling or analysis of any kind
* Not intended for plotting huge datasets
* Doesn't offer 'all the plot types in existence', but rather just several of the most useful types
* Ignores data passed in as JSON objects or JSON arrays. Accepts flat numbers, strings and booleans only
* No export of the rendered plot into other formats (such as pictures)
* Doesn't (and cannot) make data 'look good' if the data is fundamentally unsuitable for plotting
* Doesn't (and cannot) show 100% visual accuracy due to inherent limitations of terminal interfaces

## Features under consideration ##

* Automatic discovery of terminal colors (usually dubbed 'color theme') ... this would enable matching colors with your existing setup 'out of the box', would be super nice
* Piping data in using other formats

## Disclaimer ##

## General information ##

*

## Technical information ##

## External libraries used ##

* [nlohmann-json](https://github.com/nlohmann/json)
