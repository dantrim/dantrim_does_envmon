# dantrim_does_envmon
Simple wrapper example for the ITkPix Strips EnvMon board

## Installation
Follow these steps to checkout this repository and compile the code.
```
git clone --recursive <url>
cd dantrim_does_envmon/
mkdir build
cd build
cmake3 ..
make -j4
```
Note that the `--recursive` flag is required in order to pull the
`labRemote` dependencies.

## Example Code for EnvMon
The script [src/tools/envmon_setup.cpp](src/tools/envmon_setup.cpp) provides an example
configuration of the EnvMon board in which, in addition to the AD7998 ADC (part labelled `U1`
on the EnvMon board silk screen), there are two SHT85 sensors connected to
two independent QWIIC/Stemma connectors: channel 0 on the TCA9548 mux device with `ID = 000`
and channel 1 on the TCA9548 mux device with `ID = 001`.

The script initializes the devices and begins a measurement loop.
If everything compiled correctly (following the steps in [Installation](#installation))
then you should be able to do:
```
cd build/
./bin/envmon_setup
```
