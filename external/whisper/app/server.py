from fastapi import FastAPI, UploadFile, File, HTTPException, Form
from fastapi.responses import JSONResponse
from faster_whisper import WhisperModel
import torch 
import tempfile
import os

# curl --request POST \
#   --url https://api.openai.com/v1/audio/transcriptions \
#   --header "Authorization: Bearer $OPENAI_API_KEY" \
#   --header 'Content-Type: multipart/form-data' \
#   --form file=@/path/to/file/audio.mp3 \
#   --form model=gpt-4o-transcribe

app = FastAPI(title="Whisper API", version="1.0.0")
device = "cuda" if torch.cuda.is_available() else "cpu"
compute_type = "float16" if torch.cuda.is_available() else "int8"

# For TensorRT: Install nvidia-tensorrt and set backend="tensorrt" in WhisperModel
# Available models: tiny, base, small, medium, large-v2, large-v3
try:
    model = WhisperModel("small", device, compute_type=compute_type)  
except Exception as e:
    raise  

@app.post("/transcribe")
async def transcribe_audio(
    file: UploadFile = File(...),
    language: str = Form(None)  # Optional: "id", "en", etc.
):
    if not file.filename.lower().endswith(('.mp3', '.wav', '.m4a', '.flac', '.mp4', '.ogg')):
        raise HTTPException(400, "Unsupported file type. Supported: .mp3, .wav, .m4a, .flac, .mp4, .ogg")

    # Save temp file
    with tempfile.NamedTemporaryFile(delete=False, suffix=os.path.splitext(file.filename)[1]) as tmp:
        tmp.write(await file.read())
        tmp_path = tmp.name

    try:
        # Transcribe using faster-whisper (TensorRT optimized)
        segments, info = model.transcribe(
            tmp_path,
            language=language,  # None for auto-detection
            beam_size=5,
            vad_filter=True,  # Voice activity detection
            vad_parameters=dict(threshold=0.5, min_speech_duration_ms=250)
        )

        # Combine all segments into full text
        full_text = " ".join([segment.text for segment in segments])

        return JSONResponse({
            "language": info.language,
            "text": full_text.strip()
        })
    except Exception as e:
        raise HTTPException(500, f"Transcription failed: {str(e)}")
    finally:
        os.unlink(tmp_path)