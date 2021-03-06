// Code generated by mockery v1.1.2. DO NOT EDIT.

package main

import mock "github.com/stretchr/testify/mock"

// MockSmcAPI is an autogenerated mock type for the SmcAPI type
type MockSmcAPI struct {
	mock.Mock
}

// GetIsWinner provides a mock function with given fields: addr
func (_m *MockSmcAPI) GetIsWinner(addr string) (IsWinnerResp, error) {
	ret := _m.Called(addr)

	var r0 IsWinnerResp
	if rf, ok := ret.Get(0).(func(string) IsWinnerResp); ok {
		r0 = rf(addr)
	} else {
		r0 = ret.Get(0).(IsWinnerResp)
	}

	var r1 error
	if rf, ok := ret.Get(1).(func(string) error); ok {
		r1 = rf(addr)
	} else {
		r1 = ret.Error(1)
	}

	return r0, r1
}

// GetLuckyNums provides a mock function with given fields:
func (_m *MockSmcAPI) GetLuckyNums() (LuckyNumsResp, error) {
	ret := _m.Called()

	var r0 LuckyNumsResp
	if rf, ok := ret.Get(0).(func() LuckyNumsResp); ok {
		r0 = rf()
	} else {
		r0 = ret.Get(0).(LuckyNumsResp)
	}

	var r1 error
	if rf, ok := ret.Get(1).(func() error); ok {
		r1 = rf()
	} else {
		r1 = ret.Error(1)
	}

	return r0, r1
}

// GetParticipants provides a mock function with given fields:
func (_m *MockSmcAPI) GetParticipants() (ParticipantsResp, error) {
	ret := _m.Called()

	var r0 ParticipantsResp
	if rf, ok := ret.Get(0).(func() ParticipantsResp); ok {
		r0 = rf()
	} else {
		r0 = ret.Get(0).(ParticipantsResp)
	}

	var r1 error
	if rf, ok := ret.Get(1).(func() error); ok {
		r1 = rf()
	} else {
		r1 = ret.Error(1)
	}

	return r0, r1
}

// GetPrizeFund provides a mock function with given fields:
func (_m *MockSmcAPI) GetPrizeFund() (PrizeFundResp, error) {
	ret := _m.Called()

	var r0 PrizeFundResp
	if rf, ok := ret.Get(0).(func() PrizeFundResp); ok {
		r0 = rf()
	} else {
		r0 = ret.Get(0).(PrizeFundResp)
	}

	var r1 error
	if rf, ok := ret.Get(1).(func() error); ok {
		r1 = rf()
	} else {
		r1 = ret.Error(1)
	}

	return r0, r1
}

// GetPrizes provides a mock function with given fields:
func (_m *MockSmcAPI) GetPrizes() (PrizesResp, error) {
	ret := _m.Called()

	var r0 PrizesResp
	if rf, ok := ret.Get(0).(func() PrizesResp); ok {
		r0 = rf()
	} else {
		r0 = ret.Get(0).(PrizesResp)
	}

	var r1 error
	if rf, ok := ret.Get(1).(func() error); ok {
		r1 = rf()
	} else {
		r1 = ret.Error(1)
	}

	return r0, r1
}
