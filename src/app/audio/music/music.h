#pragma once

#include "../note.h"

// Music note structure for melodies
struct MusicNote {
    float frequency;  // Note frequency in Hz (0 for rest)
    int duration;     // Duration in milliseconds
};

// Sample melodies - simple acoustic-style tunes
// Each melody is an array of MusicNote, terminated by {0, 0}

// Twinkle Twinkle Little Star
const MusicNote TWINKLE_TWINKLE[] = {
    {NOTE_C4, 400}, {NOTE_C4, 400}, {NOTE_G4, 400}, {NOTE_G4, 400},
    {NOTE_A4, 400}, {NOTE_A4, 400}, {NOTE_G4, 800},
    {NOTE_F4, 400}, {NOTE_F4, 400}, {NOTE_E4, 400}, {NOTE_E4, 400},
    {NOTE_D4, 400}, {NOTE_D4, 400}, {NOTE_C4, 800},
    {NOTE_G4, 400}, {NOTE_G4, 400}, {NOTE_F4, 400}, {NOTE_F4, 400},
    {NOTE_E4, 400}, {NOTE_E4, 400}, {NOTE_D4, 800},
    {NOTE_G4, 400}, {NOTE_G4, 400}, {NOTE_F4, 400}, {NOTE_F4, 400},
    {NOTE_E4, 400}, {NOTE_E4, 400}, {NOTE_D4, 800},
    {NOTE_C4, 400}, {NOTE_C4, 400}, {NOTE_G4, 400}, {NOTE_G4, 400},
    {NOTE_A4, 400}, {NOTE_A4, 400}, {NOTE_G4, 800},
    {NOTE_F4, 400}, {NOTE_F4, 400}, {NOTE_E4, 400}, {NOTE_E4, 400},
    {NOTE_D4, 400}, {NOTE_D4, 400}, {NOTE_C4, 800},
    {0, 0}  // End marker
};

// Simple ascending scale
const MusicNote ASCENDING_SCALE[] = {
    {NOTE_C4, 300}, {NOTE_D4, 300}, {NOTE_E4, 300}, {NOTE_F4, 300},
    {NOTE_G4, 300}, {NOTE_A4, 300}, {NOTE_B4, 300}, {NOTE_C5, 600},
    {0, 0}
};

// Descending scale
const MusicNote DESCENDING_SCALE[] = {
    {NOTE_C5, 300}, {NOTE_B4, 300}, {NOTE_A4, 300}, {NOTE_G4, 300},
    {NOTE_F4, 300}, {NOTE_E4, 300}, {NOTE_D4, 300}, {NOTE_C4, 600},
    {0, 0}
};

// Happy birthday tune (simplified)
const MusicNote HAPPY_BIRTHDAY[] = {
    {NOTE_C4, 300}, {NOTE_C4, 150}, {NOTE_D4, 450}, {NOTE_C4, 450}, {NOTE_F4, 450}, {NOTE_E4, 900},
    {NOTE_C4, 300}, {NOTE_C4, 150}, {NOTE_D4, 450}, {NOTE_C4, 450}, {NOTE_G4, 450}, {NOTE_F4, 900},
    {NOTE_C4, 300}, {NOTE_C4, 150}, {NOTE_C5, 450}, {NOTE_A4, 450}, {NOTE_F4, 450}, {NOTE_E4, 450}, {NOTE_D4, 450},
    {NOTE_AS4, 300}, {NOTE_AS4, 150}, {NOTE_A4, 450}, {NOTE_F4, 450}, {NOTE_G4, 450}, {NOTE_F4, 900},
    {0, 0}
};

// Simple chord progression (arpeggiated)
const MusicNote SIMPLE_CHORD_PROGRESSION[] = {
    {NOTE_C4, 200}, {NOTE_E4, 200}, {NOTE_G4, 200}, {NOTE_C5, 400},  // C major
    {NOTE_A3, 200}, {NOTE_C4, 200}, {NOTE_E4, 200}, {NOTE_A4, 400},  // A minor
    {NOTE_F3, 200}, {NOTE_A3, 200}, {NOTE_C4, 200}, {NOTE_F4, 400},  // F major
    {NOTE_G3, 200}, {NOTE_B3, 200}, {NOTE_D4, 200}, {NOTE_G4, 400},  // G major
    {0, 0}
};

// Notification sound - ascending then descending
const MusicNote NOTIFICATION_SOUND[] = {
    {NOTE_C4, 150}, {NOTE_E4, 150}, {NOTE_G4, 150}, {NOTE_C5, 300},
    {NOTE_G4, 150}, {NOTE_E4, 150}, {NOTE_C4, 300},
    {0, 0}
};

// Error sound - descending notes
const MusicNote ERROR_SOUND[] = {
    {NOTE_G4, 200}, {NOTE_F4, 200}, {NOTE_E4, 200}, {NOTE_D4, 200}, {NOTE_C4, 400},
    {0, 0}
};

// Success sound - ascending notes
const MusicNote SUCCESS_SOUND[] = {
    {NOTE_C4, 150}, {NOTE_D4, 150}, {NOTE_E4, 150}, {NOTE_G4, 150}, {NOTE_C5, 400},
    {0, 0}
};