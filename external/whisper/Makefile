imagename = whisper
platform = arm

build-amd:
	docker build . -t ${imagename}:amd64 --platform amd64

build-arm:
	docker build . -t ${imagename}:arm64 --platform arm64

build: build-amd build-arm

save:
	docker save ${imagename} | gzip > ${imagename}.tar.gz

run: build-${platform}
	docker run --name ${imagename} --network mqttui_default \
	-d -p 8000:8000 --name whisper \
	--restart unless-stopped \
	--cpu-shares="512" --memory="512m" \
	${imagename}:${platform}64

test:
	curl -X POST "http://localhost:8000/transcribe" \
		-H "accept: application/json" \
		-H "Content-Type: multipart/form-data" \
		-F "file=@sample/audio.mp3" \
		-F "language=id"
