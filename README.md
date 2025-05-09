<hr>
<div align="center">
Incplot is a command like tool for drawing great looking plots in the terminal using unicode characters.

Automatically infers what to display and how based on the data piped in.

'As quick and as easy as possible' visualization of small to moderate sized data and information.

</div>

## Features ##

* Great looking plots 'out of the box'
* Zero configuration required, super simple to use
* No command line arguments necessary most of the time due to automatic inferrence
* Cross platform on sensible mainstream (Windows, Linux, MacOS)
* User can optionally specify some or all arguments in any (sensible) combination
* Accepts piped in data in [JSON](https://en.wikipedia.org/wiki/JSON), [JSON Lines](https://jsonlines.org/), [NDJSON](https://github.com/ndjson), [CSV](https://en.wikipedia.org/wiki/Comma-separated_values), [TSV](https://en.wikipedia.org/wiki/Tab-separated_values)
* Blazingly fast
* Full true color support internally, reasonably modern terminal emulators have support for it
* Automatic sizing
* Automatic value labels and scaling with [metric prefixes](https://en.wikipedia.org/wiki/Metric_prefix)
* Automatic string label sizing (ie. shortening where necessary)
* Automatic tickmarks
* Automatic legend
* Reasonable error messages (especially when the user asks for something that is impossible/unsupported)

## Non-Features ##

* No data wrangling or analysis of any kind
* Doesn't offer 'all the plot types in existence', but rather just several of the most useful types
* Accepts flat numbers, strings and booleans only (ie. doesn't parse 'complex' data types such as date/time formats from string back into their numeric representation)
* Doesn't understand or work with 'units' (ie. 10km will be treated as string not value 10 with unit 'kilometers')
* No export of the rendered plot into other formats (such as pictures)
* Doesn't (and cannot) make data 'look good' if the data is fundamentally unsuitable for plotting
* Doesn't (and cannot) show 100% visual accuracy due to inherent limitations of terminal interfaces
* Not guaranteed to work well on ancient terminal emulators without (true) color support

## Features under consideration ##

* Automatic discovery of terminal colors (usually dubbed 'color theme') ... this would enable matching colors with your existing setup 'out of the box', would be super nice

## Disclaimer ##

## General information ##

*

## Technical information ##

Most of technical details of how the tool works under the hood are described in [incplot-lib](https://github.com/InCom-0/incplot-lib) which is the underlying library implementing all the core features except those that are directly related to running in the terminal

## External libraries used ##

* [nlohmann-json](https://github.com/nlohmann/json)
