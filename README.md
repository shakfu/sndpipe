# sndpipe

Early-stage project containing experiments with Max/MSP externals and the nifty [Soundpipe DSP](https://github.com/PaulBatchelor/Soundpipe) library.

My initial goal was to learn MSP external programming by using a DSP library, however it's so quick to develop with Soundpipe DSP that there's now a secondary goal to produce a set of Max/MSP objects which correspond on a one-to-one basis with Soundpipe modules.

Please note that is a macOS only package and it includes cycling 74's [maxsdk](https://github.com/Cycling74/max-sdk) as a submodule in the `source` directory, so you will have to do the following in the project root after cloning the repo.

```bash
git submodule init
git submodule update
```

This project uses [Xcodegen](https://github.com/yonaskolb/XcodeGen) to generate an xcode project of a max or msp external from a yaml spec

For convenience, `libsoundpipe.a` and `libsndfile.a` are included. Please feel free to compile these separately.

After compilation, to install, just soft link or copy this folder to `$HOME/Documents/Max 8/Packages/sndpipe`

## Externals developed to date

- **sp_bigverb~**: a nice-sounding spaceout reverb.

- **sp_brown~**: The first external is a simple proof-of-concept just uses the Soundpipe brown (brownian motion) module.

- **sp_osc**~: A simple clear-sonding delay.

- **sp_osc**~: A simple oscillator sin and triangle.

- **sp_pshift~**: A pitch-shifter effect.

- **sndpipe~**: A kitchensink of things..

## Credits

Thanks to Paul Batchelor for sharing his very welcoming Soundpipe library!
