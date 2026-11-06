# Get Started with NuttX (Sony Spresense)

This guide builds Ocre for the Sony Spresense main board, with the SD card on the extension board and the Spresense
Wi-Fi add-on (GS2200M). Check the [NuttX platform](PlatformNuttX.md) documentation for the implementation details.

## Prerequisites

- The Spresense SDK and its toolchain, see the
  [Spresense SDK setup guide](https://developer.spresense.sony-semicon.com/development-guides/?page=sdk_set_up&lang=en#_setup_for_linux).
- This repository, cloned with `--recursive`.
- [WASI SDK](https://github.com/WebAssembly/wasi-sdk) in `/opt/wasi-sdk`, to build containers.

## Configure

`nuttx/spresense/setup.sh` creates the `ocre` application in a Spresense application root directory (`SPRESENSE_HOME`) and configures the SDK with `default device/sdcard feature/wifi feature/ocre`. The application directory (`~/ocre-spresense-apps/ocre` below) is a build directory: its `Makefile` and `Kconfig` include the ones in `nuttx/`, so objects and generated files are created there and the source tree stays clean.

```sh
source ~/spresenseenv/setup
./nuttx/spresense/setup.sh <path to spresense> ~/ocre-spresense-apps
```

The pins used by the Wi-Fi add-on depend on its hardware version (printed on the board as iS110B v1.0A, v1.0B or v1.0C), and v1.0A is the default. With the wrong version the board hangs at boot without any console output, while the GS2200M driver waits for the module. For v1.0C:

```sh
./nuttx/spresense/setup.sh <path to spresense> ~/ocre-spresense-apps +WIFI_BOARD_IS110B_HARDWARE_VERSION_10C=y
```

More configurations can be appended to the command line. The Ocre options are in `Spresense SDK > Ocre > Ocre Runtime` of `make menuconfig`.

## Build and flash

```sh
export SPRESENSE_HOME=~/ocre-spresense-apps
cd <path to spresense>/sdk
make
./tools/flash.sh -c /dev/ttyUSB0 nuttx.spk
```

Connect to the NSH console at 115200 bps, i.e. `minicom -D /dev/ttyUSB0 -b 115200`.

## Run the mini sample

With a FAT formatted SD card inserted:

```
nsh> ocre_mini
```

It prints the hello world banner and leaves `/mnt/sd0/ocre/images/sample.wasm`.

## Run containers

Build a container with WASI SDK, i.e. `ocre-sdk/generic/hello-world`:

```sh
/opt/wasi-sdk/bin/clang --target=wasm32-wasip1 -O2 ocre-sdk/generic/hello-world/main.c \
    -Wl,--import-memory -Wl,--export-memory -Wl,--strip-all -Wl,--allow-undefined -Wl,--max-memory=4194304 \
    -o hello-world.wasm
```

Copy it to `ocre/images/` on the SD card, then:

```
nsh> ocred
nsh> ocre images
nsh> ocre run hello-world.wasm
nsh> ocre ps
```

`ocred` runs one command at a time: `ocre run` without `-d` returns when the container exits, and other `ocre` commands wait until then.  
Use `ocre run -d` for long running containers.

Check the [Ocre CLI](OcreCli.md) documentation for all the commands.

## Pull images over Wi-Fi

Start the Wi-Fi add-on in station mode and connect to the access point, as described in the
[Spresense Wi-Fi add-on documentation](https://developer.spresense.sony-semicon.com/development-guides/?page=sdk_developer_guide&lang=en):

```
nsh> gs2200m <ssid> <passphrase> &
nsh> ifconfig
```

Serve the images from a host in the same network, i.e. with `python3 -m http.server 8000`, then:

```
nsh> ocre pull http://<host>:8000/hello-world.wasm
nsh> ocre run hello-world.wasm
```
