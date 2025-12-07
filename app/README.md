# App Source Directory

This is the main source directory for the ESP32 assistant project.

- `main.cpp`: Entry point that orchestrates all features. Initializes logger and microphone in setup(), creates FreeRTOS tasks (no Arduino loop() used).
- `config.h`: Global configuration for pins, constants, and settings.

Subfolders organize features and shared components for modularity and growth.