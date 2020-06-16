# lottery-bot

Provides Telegram bot for lottery smc, with data fetched from [lottery-back](../lottery-back/). Based on [telegram-bot-api](https://github.com/go-telegram-bot-api/telegram-bot-api).

## dev

To host/test bot in Russia use Tor and exec as `HTTP_PROXY="socks5://127.0.0.1:9050/" ./bot`.

To update mocks use `mockery -all -inpkg`.

## Run

```sh
bot -bot_token=<token> -api_endpoint=<back addr>
```

Service [file](_misc/lottery-bot.service) for systemd.
