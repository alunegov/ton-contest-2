package main

// Logger is an interface that represents the required methods to log data.
type Logger interface {
	Printf(format string, v ...interface{})
}
