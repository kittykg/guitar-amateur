# Guitar Amateur 

Inspired by Guitar Hero

## Getting Started

### Prerequisites

If running on Mac: OSX >= 10.7 and gcc >= 4.8

SDL2, SDL\_mixer, SDL\_image and SDL\_ttf.

A desired song stored in a folder that is runable on either Guitar Hero or Clone 
Hero, including its .ogg files and notes.chart file.

### Building

To compile the game run:
```
make
```

If the .ogg files for the song are not already combined into a single song.ogg
file, run:

```
./mix.sh path/of/song/folder
```

FFmpeg is required to mix the .ogg files.

## Running

To run the game:
```
./guitar_amateur path/of/song/folder [difficulty]
```

If no difficulty is given, the first difficulty in the notes.chart file will be
chosen.

Possible difficulties include: easy, medium, hard and expert (case insensitive).
However, the notes.chart file may not contain all difficulty levels.

## Authors

* **Bianca Casapu**
* **George Ordish**
* **Kexin Gu**
* **Matthew Baugh** - leader

## Acknowledgments

* Font: I Hate Comic Sans - Utopia 
