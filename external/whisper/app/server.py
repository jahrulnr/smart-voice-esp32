from fastapi import FastAPI, UploadFile, File, HTTPException, Form
from fastapi.responses import JSONResponse
import whisper
import tempfile
import os

# curl --request POST \
#   --url https://api.openai.com/v1/audio/transcriptions \
#   --header "Authorization: Bearer $OPENAI_API_KEY" \
#   --header 'Content-Type: multipart/form-data' \
#   --form file=@/path/to/file/audio.mp3 \
#   --form model=gpt-4o-transcribe

app = FastAPI(title="Whisper API", version="1.0.0")

# Load model once at startup
# models: tiny, base, small, medium, large, turbo
model = whisper.load_model("tiny")

@app.post("/transcribe")
async def transcribe_audio(
    file: UploadFile = File(...),
    language: str = Form(None)  # Optional: "id", "en", etc.
):
    if not file.filename.lower().endswith(('.mp3', '.wav', '.m4a', '.flac')):
        raise HTTPException(400, "Unsupported file type. Supported: .mp3, .wav, .m4a, .flac")

    # Save temp file
    with tempfile.NamedTemporaryFile(delete=False, suffix=os.path.splitext(file.filename)[1]) as tmp:
        tmp.write(await file.read())
        tmp_path = tmp.name

    try:
        # Load and process audio
        audio = whisper.load_audio(tmp_path)
        audio = whisper.pad_or_trim(audio)
        mel = whisper.log_mel_spectrogram(audio, n_mels=model.dims.n_mels).to(model.device)

        # Handle language
        if language:
            detected_lang = language
            options = whisper.DecodingOptions(language=language)
        else:
            _, probs = model.detect_language(mel)
            detected_lang = max(probs, key=probs.get)
            options = whisper.DecodingOptions()

        # Decode
        result = whisper.decode(model, mel, options)

        return JSONResponse({
            "language": detected_lang,
            "text": result.text
        })
    finally:
        os.unlink(tmp_path)