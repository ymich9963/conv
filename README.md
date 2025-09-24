# Convolution tool (conv)
![badge](https://badgen.net/badge/Coverage/100/blue) 
<a href="https://scan.coverity.com/projects/ymich9963-conv">
  <img alt="Coverity Scan Build Status"
       src="https://scan.coverity.com/projects/32163/badge.svg"/>
</a>

Convolve two inputs that are either audio files or CSV files/strings.

## Features
- Supprts three input types,
    - Audio files.
    - CSV files or strings.
- Normalise output to have a listenable audio file of the convolution result.
- Timer to benchmark different implementations.
- Output data as an audio file, CSV file, or to terminal.

## Installing
Currently an automatic installation exists only for Windows, and binaries are built only for Windows. For other Operating Systems you need to build from source.

### Windows
To install automatically, use the install script located in `install/` by executing the command below in a PowerShell terminal with Admin rights,

```
irm "https://raw.githubusercontent.com/ymich9963/conv/refs/heads/main/install/conv-windows-install.ps1" | iex
```

The script downloads the executable, moves it to `C:\Program Files\conv\`, and adds that path to the system environment variable. If you do not want the automated script feel free to download the executable or build from source. In case your organisation doesn't allow you to install from the script due to protected paths, download the script and change the `$CONV_install_path` variable to a location that suits you.

### macOS & Linux
Please the Building section. Use `make` to build from source.

## Usage
The `--help` option which provides a list of the available commands is listed below, followed by example uses.

```
Convolution tool (conv) help page.

Basic usage 'conv <Input audio file or CSV file or CSV string> <Input audio file or CSV file or CSV string> [options]. For list of options see below.

                --info                          = Output to stdout some info about the input file.
        -i,     --input <File/String>   = Accepts audio files and CSV files or strings. Make sure to separate string with commas, e.g. 1,0,0,1. Use the options below if you want to specify but CONV implements auto-detection.
        -o,     --output <File Name>            = Path or name of the output file.
        -f,     --output-format <Format>        = Format of the output file. Select between: 'audio', 'stdout', 'stdo ut-csv', 'columns', and 'csv'.
        -p,     --precision <Number>            = Decimal number to define how many decimal places to output.
        --norm, --normalise                     = Normalise the data. Only works wit --pow.
                --timer                         = Start a timer to see how long the calculation takes.
        -q,     --quiet                         = Silence all status messages to stdout. Overwrites '--info'.
```

### Example Uses
Convolve two different tracks,
```
conv is-this-it.wav smells-like-teen-spirit.wav
```
Or strings,
```
conv 1,2,3,4 1,2,3,4
```
Can also mix and match,
```
conv 1,2,3,4 smells-like-teen-spirit.wav
```

## Building
Simply use the `make` command to build the executable.

## Tests and Coverage
Running the tests or coverage can be done by running,

```
make test
```
```
make coverage
```
Testing suite used is [Unity](https://github.com/ThrowTheSwitch/Unity) and LLVM-COV for coverage.

## Resources
- [Wikipedia](https://en.wikipedia.org/wiki/Convolution)
- [Simple Convolution in C](https://lloydrochester.com/post/c/convolution/)
