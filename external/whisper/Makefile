imagename = whisper

build:
	docker build . -t ${imagename}

run: build
	@make stop || true
	docker run --name ${imagename} --network mqttui_default \
	-d -p 8000:8000 --name whisper  \
	--restart unless-stopped \
	${imagename}

stop:
	@docker stop whisper; docker rm whisper

test:
	curl -X POST "http://localhost:8000/transcribe" \
		-H "accept: application/json" \
		-H "Content-Type: multipart/form-data" \
		-F "language=id" \
		-F "file=@sample/audio.mp3"
