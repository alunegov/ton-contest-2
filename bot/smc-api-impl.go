package main

import (
	"encoding/json"
	"fmt"
	"net/http"
	"net/url"
)

// SmcAPIImpl is an implementation of SmcAPI interface to access the Lottery SMC API
type SmcAPIImpl struct {
	client   *http.Client
	endpoint string
}

// GetPrizeFund returns the prize fund of current round
func (thiz SmcAPIImpl) GetPrizeFund() (PrizeFundResp, error) {
	resp, err := thiz.client.Get(thiz.endpoint + "/prize_fund")
	if err != nil {
		return PrizeFundResp{}, err
	}
	defer closeRespBody(resp)

	if err = checkResp(resp); err != nil {
		return PrizeFundResp{}, err
	}

	var result PrizeFundResp

	err = json.NewDecoder(resp.Body).Decode(&result)
	if err != nil {
		return PrizeFundResp{}, err
	}

	return result, nil
}

// GetParticipants returns the participants of current round
func (thiz SmcAPIImpl) GetParticipants() (ParticipantsResp, error) {
	resp, err := thiz.client.Get(thiz.endpoint + "/participants")
	if err != nil {
		return ParticipantsResp{}, err
	}
	defer closeRespBody(resp)

	if err = checkResp(resp); err != nil {
		return ParticipantsResp{}, err
	}

	var result ParticipantsResp

	err = json.NewDecoder(resp.Body).Decode(&result)
	if err != nil {
		return ParticipantsResp{}, err
	}

	return result, nil
}

// GetLuckyNums returns the winning nums of previous round
func (thiz SmcAPIImpl) GetLuckyNums() (LuckyNumsResp, error) {
	resp, err := thiz.client.Get(thiz.endpoint + "/lucky_nums")
	if err != nil {
		return LuckyNumsResp{}, err
	}
	defer closeRespBody(resp)

	if err = checkResp(resp); err != nil {
		return LuckyNumsResp{}, err
	}

	var result LuckyNumsResp

	err = json.NewDecoder(resp.Body).Decode(&result)
	if err != nil {
		return LuckyNumsResp{}, err
	}

	return result, nil
}

// GetPrizes returns the prizes of previous round
func (thiz SmcAPIImpl) GetPrizes() (PrizesResp, error) {
	resp, err := thiz.client.Get(thiz.endpoint + "/prizes")
	if err != nil {
		return PrizesResp{}, err
	}
	defer closeRespBody(resp)

	if err = checkResp(resp); err != nil {
		return PrizesResp{}, err
	}

	var result PrizesResp

	err = json.NewDecoder(resp.Body).Decode(&result)
	if err != nil {
		return PrizesResp{}, err
	}

	return result, nil
}

// GetIsWinner returns the prize for specific participant address
func (thiz SmcAPIImpl) GetIsWinner(addr string) (IsWinnerResp, error) {
	resp, err := thiz.client.Get(thiz.endpoint + "/is_winner?addr=" + url.QueryEscape(addr))
	if err != nil {
		return IsWinnerResp{}, err
	}
	defer closeRespBody(resp)

	if err = checkResp(resp); err != nil {
		return IsWinnerResp{}, err
	}

	var result IsWinnerResp

	err = json.NewDecoder(resp.Body).Decode(&result)
	if err != nil {
		return IsWinnerResp{}, err
	}

	return result, nil
}

func closeRespBody(resp *http.Response) {
	_ = resp.Body.Close()
}

const (
	contentTypeHeader   = "Content-Type"
	expectedContentType = "text/json; charset=utf-8"
)

func checkResp(resp *http.Response) error {
	if resp.StatusCode != http.StatusOK {
		return fmt.Errorf(`got resp with status "%s"`, resp.Status)
	}

	if contentType := resp.Header.Get(contentTypeHeader); contentType != expectedContentType {
		return fmt.Errorf(`expected "%s" as "%s" but got "%s"`, expectedContentType, contentTypeHeader, contentType)
	}

	return nil
}
