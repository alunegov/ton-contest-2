# lottery-back

Provides RESTfull server which converts get-methods of lottery SMC to API endpoints. Based on restinio and tonlib. Server runs at 8080 port. Config file for TON are *ton-lite-client-test1.config.json*.

## API

- /prize_fund
- /participants
- /lucky_nums
- /prizes
- /is_winner?addr=\<addr>

Swagger [specification](lottery-back_oas2.yml).

## Build

```sh
vcpkg install restinio zlib openssl
git submodule update --init --recursive -- _ext/ton  // in parent dir
cmake -B build -D CMAKE_TOOLCHAIN_FILE=[vcpkg root]/scripts/buildsystems/vcpkg.cmake .
cmake --build build --target lottery-back --config Release
```

> For ***vcpkg*** it may be usefull to set ***VCPKG_DEFAULT_TRIPLET*** environment variable.

## Run

```sh
lottery-cli -C <ton_config_file> -a <smc_addr>
```

Service [file](_misc/lottery.service) for systemd.
