
<div align="center">
     <img width="500" alt="incplot_logo" src="https://media.githubusercontent.com/media/InCom-0/incplot/0925a45f1aaf126df925ab2ccb6986a916d6aea9/images/logo/incplot_logo_t.png" />
     <hr>
     <a href="https://github.com/InCom-0/incplot/releases/"><img alt="Version" src="https://img.shields.io/github/v/release/InCom-0/incplot"></a>
     <a href="https://github.com/InCom-0/incplot/releases/latest/download/incplot_windows_msvc_x86_64.zip"><img alt="Windows" src="https://custom-icon-badges.demolab.com/badge/Windows-0078D6?logo=windows11&logoColor=white"></a>
     <a href="https://github.com/InCom-0/incplot/releases/latest/download/incplot_linux_x86_64.tar.gz"><img alt="Linux" src="https://img.shields.io/badge/Linux-FCC624?logo=linux&logoColor=black"></a>
     <a href="https://github.com/InCom-0/incplot/releases/latest/download/incplot_macos_arm64.tar.gz"><img alt="Linux" src="https://img.shields.io/badge/macOS-000000?logo=apple&logoColor=F0F0F0"></a>
     <a href="https://github.com/InCom-0/incplot/blob/main/LICENSE.txt"><img alt="MIT License" src="https://img.shields.io/github/license/InCom-0/incplot"></a>
     <br>
     <a href="https://github.com/InCom-0/incplot/actions/workflows/build.yml"><img alt="Build Status" src="https://github.com/InCom-0/incplot/actions/workflows/build.yml/badge.svg?branch=main"></a>
     <a href="https://en.cppreference.com/index.html"><img alt="CPP" src="https://img.shields.io/badge/C++-%2300599C.svg?logo=c%2B%2B&logoColor=white"></a>
     <a href="https://cmake.org/"><img alt="CMake" src="https://img.shields.io/badge/CMake-%23003765?style=flat&logo=cmake&labelColor=%23003765"></a>
     <br><br>
</div>
<div align="center">
Incplot is a command like tool for drawing great looking plots in the terminal using unicode characters.<br>
Automatically infers what to display and how based on the data piped in.<br><br>
'As quick and as easy as possible' visualization of small to moderate sized data and information.
</div>

<br><img width="400" alt="scatter" src="https://media.githubusercontent.com/media/InCom-0/incplot/8708dcc3d8eacdf9d079dd3b0b13d8ccc8be8c48/images/readme/scatter.png" /><img width="400" alt="multiline" src="https://media.githubusercontent.com/media/InCom-0/incplot/0925a45f1aaf126df925ab2ccb6986a916d6aea9/images/readme/multiline.png" /><img width="400" alt="barV" src="https://media.githubusercontent.com/media/InCom-0/incplot/0925a45f1aaf126df925ab2ccb6986a916d6aea9/images/readme/barV.png" />



## Installation ##
To quickly install incplot:
```bash
# Linux and macOS
brew install --cask InCom-0/incom/incplot
# Windows (with Scoop)
scoop bucket add incoms https://github.com/InCom-0/ScoopBucket_InCom
scoop install incoms/incplot
```

Otherwise there are precompiled [releases](https://github.com/InCom-0/incplot/releases/) in the repository itself.

## Features ##

* Great looking plots 'out of the box'
* Zero configuration required, super simple to use
* No command line arguments necessary most of the time due to automatic inferrence
* Mainstream platform support (Windows, Linux, MacOS)
* User can optionally specify some or all arguments in any (sensible) combination
* Accepts piped in data in [JSON](https://en.wikipedia.org/wiki/JSON), [JSON Lines](https://jsonlines.org/), [NDJSON](https://github.com/ndjson), [CSV](https://en.wikipedia.org/wiki/Comma-separated_values), [TSV](https://en.wikipedia.org/wiki/Tab-separated_values)
* Blazingly fast
* Full true color support
* Automatic but configurable filtering of extreme values
* Automatic sizing
* Automatic value labels and scaling with [metric prefixes](https://en.wikipedia.org/wiki/Metric_prefix)
* Automatic string label sizing (ie. shortening where necessary)
* Automatic tickmarks
* Automatic legend
* Reasonable error messages (especially when the user asks for something that is impossible/unsupported)

## Showcase examples ##

* 

## Hints, tips and tricks ##
### Autoguessing ###

Incplot tries to be smart and makes an educated guess as to what it is the user wants displayed. However, it may not always succeed in guessing or guessing right. For this reason it is usually best to pipe in only the data (ie. the data columns) that the user wants displayed. It is especially not advisable to pipe in entire 'databases' worth of data of dozens of columns. If that is what you are starting with it is generally better to use some dataframes library or the tools/features provided by your shell to do the heavy data wrangling upfront before piping the data into incplot.  
  
### Plot types ###
Scatter plots tend to work better in smaller plot sizes (the dots are more 'densely' packed then which is typically visually nicer)  
  
(Multi)line plots tend to work better in larger plot sizes (fullscreen even)

### Extreme values ###
The common case of having extreme values in the data that would normally prevent drawing 'nice' and useful plots can be taken care of with the optional [-e VAR] command line parameter which instruct incplot to filter out all observations (ie. rows) with any value above and below VAR standard deviations from mean. Defaults to [-e 6]. [-e 0] turns this feature off.

## Features under consideration ##

* Automatic discovery of terminal colors (usually dubbed 'color theme') ... this would enable matching colors with your existing setup 'out of the box'. However, there are a lot of problemss with it both in technically making it work (would require the tool to somehow query the terminal app or the system which is a cross platform nightmare) and in using colors that are not distinct red, green and blue

## Non-Features ##

* Will not work on terminal emulators without proper unicode support (those are arguably ancient)
* No data wrangling or analysis of any kind
* Doesn't offer 'all the plot types in existence', but rather just several of the most useful types
* Accepts flat numbers, strings and booleans only (ie. doesn't parse 'complex' data types such as date/time formats from string back into their numeric representation)
* Doesn't understand or work with 'units' (ie. 10km will be treated as string not value 10 with unit 'kilometers')
* No export of the rendered plot into other formats (such as pictures)
* Not guaranteed to work well on terminal emulators without (true) color support
* Doesn't (and cannot) make data 'look good' if the data is fundamentally unsuitable for plotting
* Doesn't (and cannot) show 100% visual accuracy due to inherent limitations of terminal interfaces

## Technical information ##

Most of technical details of how the tool works under the hood are described in [incplot-lib](https://github.com/InCom-0/incplot-lib) which is the underlying library implementing all the core features except those that are directly related to running in the terminal. All external dependencies (mostly lightweight header only libraries related to parsing) are included in [incplot-lib](https://github.com/InCom-0/incplot-lib)

## License
This code is free to use under the terms of the [MIT license](https://github.com/InCom-0/incplot/blob/main/LICENSE.txt).

## Acknowledgement
Main inspirations drawn from: [YouPlot](https://github.com/red-data-tools/YouPlot) and [UnicodePlots.jl](https://github.com/JuliaPlots/UnicodePlots.jl).