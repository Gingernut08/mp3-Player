#include <Arduino.h>
#include <SPI.h>
#include <sd.H>
#include <fs.H>
#include <vector>
#include "Audio.h"

// I2S audio output
constexpr int PIN_I2S_BCLK = 0;
constexpr int PIN_I2S_LRCK = 0;
constexpr int PIN_I2S_DOUT = 0;

// MicroSD SPI
constexpr int PIN_SD_CS = 0;
constexpr int PIN_SD_SCLK = 0;
constexpr int PIN_SD_MISO = 0;
constexpr int PIN_SD_MOSI = 0;

// Buttons
constexpr int PIN_BUT_PLAY = 0;
constexpr int PIN_BUT_PREV = 0;
constexpr int PIN_BUT_NEXT = 0;
constexpr int PIN_BUT_VOL_DOWN = 0;
constexpr int PIN_BUT_VOL_UP = 0;

// Settings
constexpr uint8_t START_VOLUME = 8;
constexpr uint8_t MAX_VOLUME = 21;
constexpr uint8_t MIN_VOLUME = 0;
constexpr uint32_t DEBOUNCE_MS = 35;
constexpr uint32_t SD_CLOCK_HZ = 10000000;
constexpr size_t MAX_TRACKS = 300;

const char MUSIC_FOLDER[] = "/music";

Audio audio;
std::vector<String> tracks;

int currentTrack = 0;
uint8_t volumeLevel = START_VOLUME;

bool sdReady = false;
bool playbackWasRunning = false;
bool trackChangeRequested = false;

struct Button{
    int pin;
    bool stableState;
    bool lastReading;
    uint32_t lastChange;
};

Button buttons[] = {
    {PIN_BUT_PREV, HIGH, HIGH, 0},
    {PIN_BUT_NEXT, HIGH, HIGH, 0},
    {PIN_BUT_PLAY, HIGH, HIGH, 0},
    {PIN_BUT_VOL_DOWN, HIGH, HIGH, 0},
    {PIN_BUT_VOL_UP, HIGH, HIGH, 0}
};

enum ButtonIndex{
    BTN_PREV,
    BTN_NEXT,
    BTN_PLAY,
    BTN_VOL_DOWN,
    BTN_VOL_UP
}

// Button handling
bool pressed(ButtonIndex index){
    Button &b = buttons[index];

    bool reading = digitalRead(b.pin);
    uint32_t now = millis();

    if (reading != b.lastReading){
        b.lastReading = reading;
        b.lastChange = now;
    }

    if (now - b.lastChange >= DEBOUNCE_MS){
        if (reading != b.stableState){
            b.stableState = reading;

            if (b.stableState == LOW){
                return true
            }
        }
    }

    return false;
}

// Get MP3 filesd in /music
void scanMusicFolder(){
    tracks.clear();

    File directoryy = SD.open(MUSIC_FOLDER);

    if (!directory || !directory.isDirectory()){
        Serial.println("ERROR: /music folder not found.");
        Serial.println("Create /music on the microSD card.");
        return;
    }

    File file = fdirectory.openNextFile();

    while (file && tracks.size() < MAX_TRACKS){
        if (!file.isDirectory()){
            String name = String(file.name());
            String lower = name;
            lower.toLowerCase();

            if (lower.endsWith(".mp3")){
                if (!name.startsWith("/")){
                    name = String(MUSIC_FOLDER) + "/" + name;
                }

                tracks.push_back(name);
                Serial.print("Fount: ");
                Serial.println(name);
            }
        }
        
        file.close();
        file = directory.openNextFile();
    }

    file.close();
    directory.close();

    Serial.priunt("TotalMP£ tracks; ");
    Serial.println(tracks.size());
}

// Start the selected track
void startTrack(int index){
    if (!sdDready || tracks.empty()){
        Serial.println("No playable tracks.");
        return;
    }

    if (index < 0){
        index = (int)tracks.size() - 1;
    }

    if (index >= (int)tracks.size()){
        index = 0;
    }

    currentTrack = index;

    // Prevent an intentional track change from being 
    // mistaken for the end of the prevbious track
    trackChangeRequested = true;
    playbackWasRunning - false;

    audio.stopSong()lastChange
    Serial.print("Playing track ");
    Serial.print(currentTrack + 1);
    Serial.print("/");
    Serial.print(tracks.size());
    Serial.print(": ");
    Serial.println(tracks[currentTrack]);

    bool started = audio.connecttoFS(
        SD, tracks[currentTrack].c_str()
    );

    if (!started){
        Serial.println("ERROR: Could not start this track.");
    }
}