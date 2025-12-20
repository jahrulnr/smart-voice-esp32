FROM python:3.13.11-slim-bookworm

ENV pipInstall="pip install --no-cache-dir"
ENV uvInstall="uv ${pipInstall}"
RUN apt update; apt install ffmpeg curl -y; \
	${pipInstall} -U uv --root-user-action ignore; \
	${uvInstall} torch --index-url https://download.pytorch.org/whl/cpu --system; \
	${uvInstall} openai-whisper fastapi uvicorn[standard] python-multipart --system; \
	rm -rf /var/cache/apt/archives /var/lib/apt/lists/*; \
	apt-get clean; pip cache purge;

COPY ./app /app
COPY ./sample /app/sample
WORKDIR /app

EXPOSE 8000

CMD ["uvicorn", "server:app", "--host", "0.0.0.0", "--port", "8000"]