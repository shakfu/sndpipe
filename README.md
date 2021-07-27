# sndpipe

Early-stage project containing experiments with Max/MSP externals and the nifty [Soundpipe DSP](https://github.com/PaulBatchelor/Soundpipe) library.

My initial goal was to learn MSP external programming by using a DSP library, however it's so quick to develop with Soundpipe DSP that there's now a secondary goal to produce a set of Max/MSP objects which correspond on a one-to-one basis with Soundpipe modules.

Please note that is a macOS only package and it includes cycling 74's [maxsdk](https://github.com/Cycling74/max-sdk) as a submodule in the `source` directory, so you will have to do the following in the project root after cloning the repo.

```bash
git submodule init
git submodule update
```

## Compilation for Use

This project uses [Xcodegen](https://github.com/yonaskolb/XcodeGen) to generate an xcode project of a max external from a yaml spec

For convenience, `libsoundpipe.a` and `libsndfile.a` are included. Please feel free to compile these separately.

After compilation, to install, just soft link or copy this folder to `$HOME/Documents/Max 8/Packages/sndpipe`

## Externals developed to date

- **sp_bigverb~**: A nice-sounding spaceout reverb.

- **sp_bitcrush~**: Digital signal degradation.

- **sp_brown~**: Brownian noise generator.

- **sp_osc**~: A simple clear-sonding delay.

- **sp_osc**~: A simple oscillator sin and triangle.

- **sp_pshift~**: A pitch-shifter effect.

- **sndpipe~**: A kitchensink of things.

## Externals under-development (NOT YET WORKING)

- **smoothdelay~**: Smooth variable delay line without varispeed pitch.

## Development

To speed up conversion of soundpipe modules to Max/MSP externals, this project uses code generation of the subproject elements.

To create a new subproject. Pick a module from Sound pipe. Let's use `bitcrush` in this example. We see from the module lua [description](https://github.com/PaulBatchelor/Soundpipe/blob/master/modules/data/bitcrush.lua) that it has two (optional) params:

- int bitdepth with default: 8, min: 1, and max: 16
- float srate with default: 1000.0

The other important files during development of the external are:

- The [module code](https://github.com/PaulBatchelor/Soundpipe/blob/master/modules/bitcrush.c).

- The module [test file](https://github.com/PaulBatchelor/Soundpipe/blob/master/test/t/t_bitcrush.c).

So let's generate the subproject skeleton:

```bash
cd sndpipe/source/dev
./scripts/gen.py bitcrush -p bitdepth int 8 1 16 -p srate float 1000.0

```

## Credits

Thanks to Paul Batchelor for sharing his very welcoming Soundpipe library!
