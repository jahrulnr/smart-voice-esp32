# Build stage
FROM golang:1.24-alpine AS builder

WORKDIR /app

# Copy go mod files
COPY go.mod go.sum ./

# Download dependencies
RUN go mod download

# Copy source code
COPY . .

# Build the application
RUN CGO_ENABLED=0 GOOS=linux go build -a -installsuffix cgo -o main ./cmd

# Final stage
FROM alpine:latest

# Install SOX for audio processing
RUN apk --no-cache add ca-certificates sox

WORKDIR /app

# Copy the binary from builder stage
COPY --from=builder /app/main .
COPY audio/*.prof /app/audio/

# Expose port if needed (though this is a consumer, not a server)
# EXPOSE 8080

# Run the binary
CMD ["./main"]