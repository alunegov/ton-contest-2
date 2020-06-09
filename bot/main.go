package main

import (
	"flag"
	"log"
	"net/http"
	"os"
	"time"

	tgbotapi "github.com/go-telegram-bot-api/telegram-bot-api"
)

func main() {
	botToken := flag.String("bot_token", "", "Bot token")
	apiEndpoint := flag.String("api_endpoint", "", "Lottery SMC API")
	isDebug := flag.Bool("debug", false, "Debug mode")

	flag.Parse()

	logger := log.New(os.Stderr, "", log.LstdFlags)

	// didn't override default http.Client read timeout as below we use timeout for GetUpdatesChan
	bot, err := tgbotapi.NewBotAPI(*botToken)
	if err != nil {
		logger.Fatal(err)
	}

	bot.Debug = *isDebug

	logger.Printf("Authorized on account %s", bot.Self.UserName)

	u := tgbotapi.NewUpdate(0)
	u.Timeout = 60

	updates, err := bot.GetUpdatesChan(u)
	if err != nil {
		logger.Fatal(err)
	}

	smcAPITransport := &http.Client{
		Timeout: 5 * time.Second,
	}
	smcAPI := SmcAPIImpl{smcAPITransport, *apiEndpoint}

	botCommands := BotCommands{smcAPI, logger, *isDebug}

	for update := range updates {
		if update.Message == nil { // ignore any non-Message Updates
			logger.Printf("skipping non-Message update %v", update)
			continue
		}

		logger.Printf("[%s, %d]-> %s", update.Message.From.UserName, update.Message.Chat.ID, update.Message.Text)

		msg := botCommands.Process(update.Message)

		logger.Printf("[%s, %d]<- %s", update.Message.From.UserName, msg.ChatID, msg.Text)

		if _, err = bot.Send(msg); err != nil {
			logger.Print(err)
		}
	}
}
