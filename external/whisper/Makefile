imagename = whisper
platform = amd

build-amd:
	docker build . -t ${imagename}:amd64 --platform amd64

build-arm:
	docker build . -t ${imagename}:arm64 --platform arm64

build: build-amd build-arm

save:
	docker save ${imagename} | gzip > ${imagename}.tar.gz

run: build-${platform}
	@make stop || true
	docker run --name ${imagename} --network mqttui_default \
	-d -p 8000:8000 --name whisper  \
	--restart unless-stopped \
	${imagename}:${platform}64

stop:
	@docker stop whisper; docker rm whisper

test:
	curl -X POST "http://localhost:8000/transcribe" \
		-H "accept: application/json" \
		-H "Content-Type: multipart/form-data" \
		-F "language=id" \
		-F "file=@sample/audio.mp3"
