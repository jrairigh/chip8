# Chip8 Emulator
My Chip8 emulator implementation.  

<img width="953" height="482" alt="image" src="https://github.com/user-attachments/assets/49cd7078-f9e5-49c1-a34a-f7570d9c91aa" />

## How to build
Open a powershell and run .\build.ps1.
### Parameters
- `$Config` either Debug or Release
- `$Clean` removes the build directory
- `$Log` either debug, info, warn, or error
- `$Generator` generator passed to cmake -G

## How to run
Open a powershell and run .\chip8.ps1
### Parameters
- `$Config` either Debug or Release

## How to run tests
Open a powershell and run .\run-tests.ps1
### Parameters
- `$Config` either Debug or Release

## Game Controls
All games use one or more of these keys to play the game.  
```
|---|---|---|---|  
| 1 | 2 | 3 | 4 |  
|---|---|---|---|  
| Q | W | E | R |  
|---|---|---|---|  
| A | S | D | F |  
|---|---|---|---|  
| Z | X | C | V |  
|---|---|---|---|
```

## Keyboard Commands
- F1 toggles debug window (only works in game)
- F2 returns to the game menu
- F10 steps through code (only works with debug window is open)
- +/- keys update speed (instructions per cycle) by factors of 10
- Esc exits the application

## Acknowledgements
Thanks to @ankushChatterjee for inspiration from https://www.youtube.com/watch?v=jWpbHC6DtnU&t=2s and technical details provided in http://devernay.free.fr/hacks/chip8/C8TECH10.HTM#0.1  
Also, Thanks to @kripod for ROMS I used to test emulator (https://github.com/kripod/chip8-roms)
