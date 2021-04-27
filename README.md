# sndpipe

Early-stage project to experiment in using the nifty [Soundpipe DSP](https://github.com/PaulBatchelor/Soundpipe) library in Max/MSP externals.

This is a macOS only package and it includes the macsdk as has as a submodule in the `source` directory, so you will have to do the following in the project root after cloning the repo.

```bash
git submodule init
git submodule update
```

Examples so far:

- **sp_brown~**: The first external is a simple proof-of-concept just uses the Soundpipe brown (brownian motion) module.

- **sp_osc**~: A simple oscillator sin and triangle.

- **sndpipe~~**: A jumble of things..

The project includes pre-compiled dependencies of `libsoundpipe.a` and `libsndfile.a` This is just  matter of convenience, feel free to compile these separately.

To compile, install, just soft link or copy this folder to `$HOME/Documents/Max 8/Packages/sndpipe`
