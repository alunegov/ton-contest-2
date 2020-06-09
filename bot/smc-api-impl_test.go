package main

import (
	"fmt"
	"net/http"
	"net/http/httptest"
	"testing"

	"github.com/stretchr/testify/assert"
)

func TestGetPrizeFund(t *testing.T) {
	srv := httptest.NewServer(http.HandlerFunc(func(w http.ResponseWriter, r *http.Request) {
		assert.Equal(t, "/prize_fund", r.URL.String())
		w.Header().Add(contentTypeHeader, expectedContentType)
		fmt.Fprintln(w, `{"prize_fund":1900000000}`)
	}))
	defer srv.Close()

	sut := SmcAPIImpl{srv.Client(), srv.URL}

	prizeFund, err := sut.GetPrizeFund()

	assert.Nil(t, err)
	assert.Equal(t, int64(1_900_000_000), prizeFund.PrizeFund)
}

func TestGetPrizeFund_ServerFail(t *testing.T) {
	testCases := []struct {
		name    string
		handler http.HandlerFunc
	}{
		{
			"bad status",
			func(w http.ResponseWriter, r *http.Request) {
				w.WriteHeader(http.StatusNotFound)
			},
		},
		{
			"bad status and Content-Type with no payload",
			func(w http.ResponseWriter, r *http.Request) {
				w.WriteHeader(http.StatusNotFound)
				w.Header().Add(contentTypeHeader, expectedContentType)
			},
		},
		{
			"bad status and Content-Type with correct payload",
			func(w http.ResponseWriter, r *http.Request) {
				w.WriteHeader(http.StatusNotFound)
				fmt.Fprintln(w, `{"prize_fund":1900000000}`)
			},
		},
		{
			"bad Content-Type",
			func(w http.ResponseWriter, r *http.Request) {
				fmt.Fprintln(w, `{"prize_fund":1900000000}`)
			},
		},
		{
			"bad payload",
			func(w http.ResponseWriter, r *http.Request) {
				w.Header().Add(contentTypeHeader, expectedContentType)
				fmt.Fprintln(w, `{prize_fund:1900000000}`)
			},
		},
	}

	for _, tc := range testCases {
		t.Run(tc.name, func(t *testing.T) {
			srv := httptest.NewServer(tc.handler) //nolint:scopelint
			defer srv.Close()

			sut := SmcAPIImpl{srv.Client(), srv.URL}

			_, err := sut.GetPrizeFund()

			assert.NotNil(t, err)
		})
	}
}

func TestGetParticipants(t *testing.T) {
	srv := httptest.NewServer(http.HandlerFunc(func(w http.ResponseWriter, r *http.Request) {
		assert.Equal(t, "/participants", r.URL.String())
		w.Header().Add(contentTypeHeader, expectedContentType)
		fmt.Fprintln(w, `{"participants":[{},{}]}`)
	}))
	defer srv.Close()

	sut := SmcAPIImpl{srv.Client(), srv.URL}

	participants, err := sut.GetParticipants()

	assert.Nil(t, err)
	assert.Equal(t, 2, len(participants.Participants))
}

func TestGetLuckyNums(t *testing.T) {
	srv := httptest.NewServer(http.HandlerFunc(func(w http.ResponseWriter, r *http.Request) {
		assert.Equal(t, "/lucky_nums", r.URL.String())
		w.Header().Add(contentTypeHeader, expectedContentType)
		fmt.Fprintln(w, `{"lucky_nums":[5,3,1]}`)
	}))
	defer srv.Close()

	sut := SmcAPIImpl{srv.Client(), srv.URL}

	luckyNums, err := sut.GetLuckyNums()

	assert.Nil(t, err)
	assert.Equal(t, []uint8{5, 3, 1}, luckyNums.LuckyNums)
}

func TestGetPrizes(t *testing.T) {
	srv := httptest.NewServer(http.HandlerFunc(func(w http.ResponseWriter, r *http.Request) {
		assert.Equal(t, "/prizes", r.URL.String())
		w.Header().Add(contentTypeHeader, expectedContentType)
		fmt.Fprintln(w, `{"prizes":{"p1":1,"p2":2,"p3":3}}`)
	}))
	defer srv.Close()

	sut := SmcAPIImpl{srv.Client(), srv.URL}

	prizes, err := sut.GetPrizes()

	assert.Nil(t, err)
	assert.Equal(t, int64(1), prizes.Prizes.P1)
	assert.Equal(t, int64(2), prizes.Prizes.P2)
	assert.Equal(t, int64(3), prizes.Prizes.P3)
}

func TestGetIsWinner(t *testing.T) {
	srv := httptest.NewServer(http.HandlerFunc(func(w http.ResponseWriter, r *http.Request) {
		assert.Equal(t, "/is_winner?addr=wc%3Aaddr", r.URL.String())
		w.Header().Add(contentTypeHeader, expectedContentType)
		fmt.Fprintln(w, `{"prize":900000000}`)
	}))
	defer srv.Close()

	sut := SmcAPIImpl{srv.Client(), srv.URL}

	isWinner, err := sut.GetIsWinner("wc:addr")

	assert.Nil(t, err)
	assert.Equal(t, int64(900_000_000), isWinner.Prize)
}
