package main

// SmcAPI is an interface that represents the access methods to the Lottery SMC API
type SmcAPI interface {
	// /prize_fund
	GetPrizeFund() (PrizeFundResp, error)
	// /participants
	GetParticipants() (ParticipantsResp, error)
	// /lucky_nums
	GetLuckyNums() (LuckyNumsResp, error)
	// /prizes
	GetPrizes() (PrizesResp, error)
	// /is_winner?addr=<addr>
	GetIsWinner(addr string) (IsWinnerResp, error)
}

// PrizeFundResp is an /prize_fund endpoint response
type PrizeFundResp struct {
	PrizeFund int64 `json:"prize_fund"`
}

// ParticipantResp is an element of array in /participants endpoint response
type ParticipantResp struct {
	Addr struct {
		WC   int8   `json:"wc"`
		Addr string `json:"addr"`
	} `json:"addr"`
	Nums struct {
		N1 uint8 `json:"n1"`
		N2 uint8 `json:"n2"`
		N3 uint8 `json:"n3"`
	} `json:"nums"`
}

// ParticipantsResp is an /participants endpoint response
type ParticipantsResp struct {
	Participants []ParticipantResp `json:"participants"`
}

// LuckyNumsResp is an /lucky_nums endpoint response
type LuckyNumsResp struct {
	LuckyNums []uint8 `json:"lucky_nums"`
}

// PrizesResp is an /prizes endpoint response
type PrizesResp struct {
	Prizes struct {
		P1 int64 `json:"p1"`
		P2 int64 `json:"p2"`
		P3 int64 `json:"p3"`
	} `json:"prizes"`
}

// IsWinnerResp is an /is_winner endpoint response
type IsWinnerResp struct {
	Prize int64 `json:"prize"`
}
