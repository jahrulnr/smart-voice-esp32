.PHONY: build run clean docker-build docker-run test

# Go parameters
GOCMD=go
GOBUILD=$(GOCMD) build
GOCLEAN=$(GOCMD) clean
GOTEST=$(GOCMD) test
GOGET=$(GOCMD) get
GOMOD=$(GOCMD) mod

# Main package
MAIN_PACKAGE=./cmd
BINARY_NAME=main
BINARY_UNIX=$(BINARY_NAME)_unix

# Build the project
build:
	$(GOBUILD) -o $(BINARY_NAME) -v $(MAIN_PACKAGE)

# Clean build files
clean:
	$(GOCLEAN)
	rm -f $(BINARY_NAME)
	rm -f $(BINARY_UNIX)

# Run tests
test:
	$(GOTEST) -v ./...

# Download dependencies
deps:
	$(GOMOD) download
	$(GOMOD) tidy

# Build for Linux
build-linux:
	CGO_ENABLED=0 GOOS=linux GOARCH=arm64 $(GOBUILD) -o $(BINARY_UNIX) -v $(MAIN_PACKAGE)

# Docker commands
docker-build:
	docker build -t golang-mqtt-consumer .

run: docker-build
	@make stop
	docker run --restart unless-stopped --network mqttui_default -d \
		--name golang_consumer \
		-e MQTT_BROKER=tcp://mosquitto:1883 \
		-e AUDIO_TOPIC=pioassistant/audio \
		-e STT_TOPIC=pioassistant/stt \
		-e WHISPER_URL=http://whisper:8000/transcribe \
		-e SAVE_AUDIO=true \
		-e AUDIO_SAVE_DIR=/app/audio \
		-e ENABLE_NOISE_REDUCTION=true \
		-v ./audio:/app/audio \
		golang-mqtt-consumer

stop:
	docker stop golang_consumer; docker rm golang_consumer

# Development
dev: clean deps build run