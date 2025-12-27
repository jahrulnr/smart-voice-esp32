run:
	cd mqttui; docker compose up -d 2>/dev/null || true
	cd whisper; make run 2>/dev/null || true
	cd app; make run 2>/dev/null || true