package main

import (
	"context"
	"io"
	"net"
	"net/http"
	"os"
	"os/signal"
	"syscall"
	"time"

	"github.com/myfantasy/3dprint-tool/printer_v0"

	log "github.com/sirupsen/logrus"
)

func generateList() {
	rl := make([]*printer_v0.Response, 0, 100)
	for i := 0; i < 100; i++ {
		rl = append(rl, &printer_v0.Response{Id: i + 20, N: 1000 + i*10, Ts: 500, Te: 500})
	}

	s := printer_v0.ResponseListToStringSep(rl, "\n", ",")

	os.WriteFile("./slice_example/example.model.csv", []byte(s), 0644)
}

var model []*printer_v0.Response

func loadModel() {
	body, err := os.ReadFile("./slice_example/model.csv")
	if err != nil {
		log.Fatal(err)
	}

	rl, err := printer_v0.ResponseListFromStringSep(string(body), "\n", ",")
	if err != nil {
		log.Fatal(err)
	}
	model = rl
}

func main() {
	log.SetLevel(log.DebugLevel)
	//generateList()
	loadModel()

	listenPort := getENVValue("LISTEN_PORT", ":7644")

	handler := &handlerStruct{}
	ctx, cancelCtx := context.WithCancel(context.Background())
	server := &http.Server{
		Addr:    listenPort,
		Handler: handler,
		BaseContext: func(l net.Listener) context.Context {
			return ctx
		},
	}

	go func() { server.ListenAndServe() }()

	// Wait for an interrupt
	stopChan := make(chan os.Signal, 1)
	signal.Notify(stopChan, syscall.SIGTERM, syscall.SIGINT)
	<-stopChan

	// Cancel contexts
	cancelCtx()
	// Attempt a graceful shutdown
	ctx, cancel := context.WithTimeout(context.Background(), 5*time.Second)
	defer cancel()
	server.Shutdown(ctx)

}

type handlerStruct struct {
}

func (hs *handlerStruct) ServeHTTP(resp http.ResponseWriter, req *http.Request) {
	body, err := io.ReadAll(req.Body)
	if err != nil {
		log.Error(err)

		returnPrinterAnswer(resp, &printer_v0.Response{N: 100, Ts: 500, Te: 500})
		return
	}

	var rq printer_v0.Request
	err = rq.FromStringSep(string(body), "\n")
	if err != nil {
		log.WithField("body", string(body)).Error(err)

		returnPrinterAnswer(resp, &printer_v0.Response{N: 100, Ts: 500, Te: 500})
		return
	}
	log.Debug("rq: ", rq.ToStringSep(","))
	nextI := 0
	for i, v := range model {
		if v.Id == rq.LastId {
			nextI = i + 1
			break
		}
	}

	if nextI < len(model) {
		returnPrinterAnswer(resp, model[nextI])
		return
	}

	returnPrinterAnswer(resp, &printer_v0.Response{N: 100, Ts: 500, Te: 500})
}

func returnPrinterAnswer(resp http.ResponseWriter, a *printer_v0.Response) {
	_, err := resp.Write([]byte(a.ToStringSep("\n")))
	log.Debug("rs: ", a.ToStringSep(","))
	if err != nil {
		log.Error(err)
	}
}

func getENVValue(key string, defaultVal string) string {
	if value, exists := os.LookupEnv(key); exists {
		return value
	}

	return defaultVal
}
