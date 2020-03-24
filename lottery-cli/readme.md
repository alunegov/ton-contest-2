# lottery-back

Based on restinio and tonlib.

## API

- /current
- /prev
- /is_winner/:addr

## Build

```sh
vcpkg install restinio zlib openssl
git submodule update --init --recursive -- _ext/ton (in parent dir)
cmake -B build -D CMAKE_TOOLCHAIN_FILE=[vcpkg root]\scripts\buildsystems\vcpkg.cmake .
cmake --build build --target lottery-cli --config Release
```

For ***vcpkg*** it may be usefull to set ***VCPKG_DEFAULT_TRIPLET*** environment variable.

## Run

Server runs on 8080 port. Config file for TON are *ton-lite-client-test1.config.json*.
