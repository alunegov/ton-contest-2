package main

import (
	"errors"
	"fmt"
	"testing"

	tgbotapi "github.com/go-telegram-bot-api/telegram-bot-api"
	"github.com/stretchr/testify/assert"
	mock "github.com/stretchr/testify/mock"
)

func TestProcess(t *testing.T) {
	testCases := []struct {
		name      string
		inText    string
		outText   string
		parseMode string
	}{
		{"std /prize_fund", "/prize_fund", "GR$65.", ""},
		{"std /participants", "/participants", "2", ""},
		{"std /lucky_nums", "/lucky_nums", "5 3 1", ""},
		{"std /prizes", "/prizes", "GR$3.", "Markdown"},
		{"std /is_winner", "/is_winner addr", "GR$5.", ""},

		{"/prize_fund with extra arg", "/prize_fund dummy", "GR$65.", ""},
		{"/is_winner without arg", "/is_winner", "/is_winner <addr>", ""},
		{"/is_winner with extra arg", "/is_winner addr dummy", "GR$5.", ""},

		{"empty cmd", "", "Empty command", ""},
		{"unsupported cmd", "/unsupp_cmd", "Unsupported command", ""},
		{"unsupported cmd with some arg", "/unsupp_cmd dummy", "Unsupported command", ""},

		//{"cmd with bot mention", "/prize_fund@Bot", "GR$65.", ""},
	}

	mockSmcAPI := &MockSmcAPI{}
	mockSmcAPI.On("GetPrizeFund").Return(PrizeFundResp{65_000_000_000}, nil)
	mockSmcAPI.On("GetParticipants").Return(ParticipantsResp{[]ParticipantResp{{}, {}}}, nil)
	mockSmcAPI.On("GetLuckyNums").Return(LuckyNumsResp{[]uint8{5, 3, 1}}, nil)
	prizesResp := PrizesResp{}
	prizesResp.Prizes.P1 = 1_000_000_000
	prizesResp.Prizes.P2 = 2_000_000_000
	prizesResp.Prizes.P3 = 3_000_000_000
	mockSmcAPI.On("GetPrizes").Return(prizesResp, nil)
	mockSmcAPI.On("GetIsWinner", mock.Anything).Return(IsWinnerResp{5_000_000_000}, nil)

	mockLogger := &MockLogger{}
	mockLogger.On("Printf", mock.Anything, mock.Anything, mock.Anything, mock.Anything)

	sut := BotCommands{mockSmcAPI, mockLogger, true}

	for _, tc := range testCases {
		//nolint:scopelint
		t.Run(tc.name, func(t *testing.T) {
			msg := sut.Process(&tgbotapi.Message{
				Chat: &tgbotapi.Chat{ID: 3},
				Text: tc.inText,
			})

			assert.Equal(t, int64(3), msg.ChatID)
			assert.Contains(t, msg.Text, tc.outText)
			assert.Equal(t, tc.parseMode, msg.ParseMode)
		})
	}
}

func TestProcess_APIFail(t *testing.T) {
	testCases := []struct {
		inText    string
		outText   string
		parseMode string
	}{
		{"/prize_fund", "prizeFund", ""},
		{"/participants", "participants", ""},
		{"/lucky_nums", "luckyNums", ""},
		{"/prizes", "prizes", ""},
		{"/is_winner addr", "isWinner", ""},
	}

	mockSmcAPI := &MockSmcAPI{}
	mockSmcAPI.On("GetPrizeFund").Return(PrizeFundResp{}, errors.New("GetPrizeFund err"))
	mockSmcAPI.On("GetParticipants").Return(ParticipantsResp{}, errors.New("GetParticipants err"))
	mockSmcAPI.On("GetLuckyNums").Return(LuckyNumsResp{}, errors.New("GetLuckyNums err"))
	mockSmcAPI.On("GetPrizes").Return(PrizesResp{}, errors.New("GetPrizes err"))
	mockSmcAPI.On("GetIsWinner", mock.Anything).Return(IsWinnerResp{}, errors.New("GetIsWinner err"))

	mockLogger := &MockLogger{}
	mockLogger.On("Printf", mock.Anything, mock.Anything, mock.Anything, mock.Anything)

	sut := BotCommands{mockSmcAPI, mockLogger, true}

	for _, tc := range testCases {
		//nolint:scopelint
		t.Run(tc.inText, func(t *testing.T) {
			msg := sut.Process(&tgbotapi.Message{
				Chat: &tgbotapi.Chat{ID: 3},
				Text: tc.inText,
			})

			assert.Equal(t, int64(3), msg.ChatID)
			assert.Contains(t, msg.Text, tc.outText)
			assert.Equal(t, tc.parseMode, msg.ParseMode)
		})
	}
}

func TestToGrams(t *testing.T) {
	testCases := []struct {
		in  int64
		exp float64
	}{
		{65_000_000_000, 65.0},
		{65, 0.000_000_065},
	}

	for _, tc := range testCases {
		//nolint:scopelint
		t.Run(fmt.Sprintf("%d to %f", tc.in, tc.exp), func(t *testing.T) {
			assert.Equal(t, tc.exp, ToGrams(tc.in))
		})
	}
}
