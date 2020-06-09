package main

import (
	"fmt"
	"strings"

	tgbotapi "github.com/go-telegram-bot-api/telegram-bot-api"
)

// BotCommands is an controller to processing bot commands.
type BotCommands struct {
	api    SmcAPI
	logger Logger
	debug  bool
}

// Process processes incomming message.
// In all cases it returns outgoing message.
func (thiz BotCommands) Process(msg *tgbotapi.Message) tgbotapi.MessageConfig {
	s := strings.Fields(msg.Text)
	if len(s) == 0 {
		return tgbotapi.NewMessage(msg.Chat.ID, "Empty command")
	}

	switch s[0] {
	case "/prize_fund":
		return thiz.prizeFund(msg.Chat.ID)
	case "/participants":
		return thiz.participants(msg.Chat.ID)
	case "/lucky_nums":
		return thiz.luckyNums(msg.Chat.ID)
	case "/prizes":
		return thiz.prizes(msg.Chat.ID)
	case "/is_winner":
		if len(s) < 2 {
			return tgbotapi.NewMessage(msg.Chat.ID, `Wrong command pattern, "/is_winner <addr>" expected`)
		}
		return thiz.isWinner(msg.Chat.ID, s[1])
	default:
		return tgbotapi.NewMessage(msg.Chat.ID, "Unsupported command")
	}
}

func (thiz BotCommands) prizeFund(chatID int64) tgbotapi.MessageConfig {
	prizeFund, err := thiz.api.GetPrizeFund()

	var text string
	if err == nil {
		if thiz.debug {
			thiz.logger.Printf("[%d] %v", chatID, prizeFund)
		}
		text = fmt.Sprintf("The prize fund of the current round are GR$%f", ToGrams(prizeFund.PrizeFund))
	} else {
		thiz.logger.Printf("[%d] %v", chatID, err)
		text = "Error getting prizeFund"
	}

	return tgbotapi.NewMessage(chatID, text)
}

func (thiz BotCommands) participants(chatID int64) tgbotapi.MessageConfig {
	participants, err := thiz.api.GetParticipants()

	var text string
	if err == nil {
		if thiz.debug {
			thiz.logger.Printf("[%d] %v", chatID, participants)
		}
		text = fmt.Sprintf("The number of participants of the current round are %d", len(participants.Participants))
	} else {
		thiz.logger.Printf("[%d] %v", chatID, err)
		text = "Error getting participants"
	}

	return tgbotapi.NewMessage(chatID, text)
}

func (thiz BotCommands) luckyNums(chatID int64) tgbotapi.MessageConfig {
	luckyNums, err := thiz.api.GetLuckyNums()

	var text string
	if err == nil {
		if thiz.debug {
			thiz.logger.Printf("[%d] %v", chatID, luckyNums)
		}
		text = fmt.Sprintf(`The winning nums of the previous round are (in descending order):
  %v`, luckyNums.LuckyNums)
	} else {
		thiz.logger.Printf("[%d] %v", chatID, err)
		text = "Error getting luckyNums"
	}

	return tgbotapi.NewMessage(chatID, text)
}

func (thiz BotCommands) prizes(chatID int64) tgbotapi.MessageConfig {
	prizes, err := thiz.api.GetPrizes()

	var text string
	parseMode := ""
	if err == nil {
		if thiz.debug {
			thiz.logger.Printf("[%d] %v", chatID, prizes)
		}
		text = fmt.Sprintf(`The prizes of the previous round are:
  *I place* - GR$%f
  *II place* - GR$%f
  *III place* - GR$%f`, ToGrams(prizes.Prizes.P1), ToGrams(prizes.Prizes.P2), ToGrams(prizes.Prizes.P3))
		parseMode = "Markdown"
	} else {
		thiz.logger.Printf("[%d] %v", chatID, err)
		text = "Error getting prizes"
	}

	result := tgbotapi.NewMessage(chatID, text)
	result.ParseMode = parseMode

	return result
}

func (thiz BotCommands) isWinner(chatID int64, addr string) tgbotapi.MessageConfig {
	prize, err := thiz.api.GetIsWinner(addr)

	var text string
	if err == nil {
		if thiz.debug {
			thiz.logger.Printf("[%d addr=%s] %v", chatID, addr, prize)
		}
		text = fmt.Sprintf("The prize are GR$%f", ToGrams(prize.Prize))
	} else {
		thiz.logger.Printf("[%d addr=%s] %v", chatID, addr, err)
		text = "Error getting isWinner"
	}

	return tgbotapi.NewMessage(chatID, text)
}

// ToGrams converts nanograms to grams.
func ToGrams(nanoGrams int64) float64 {
	return float64(nanoGrams) / 1_000_000_000.0
}
