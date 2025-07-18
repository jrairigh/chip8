# Chip8 Emulator
My Chip8 emulator implementation.
<img width="1200" height="600" alt="screenshot000" src="https://github.com/user-attachments/assets/3fe63eea-33f4-441b-85a7-9487e2a445a8" />

## How to build
Open a powershell and run .\build.ps1 -Config Release

## How to Run
Open a powershell and run .\chip8.ps1

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
Thanks to inspiration from https://www.youtube.com/watch?v=jWpbHC6DtnU&t=2s and technical information provided in http://devernay.free.fr/hacks/chip8/C8TECH10.HTM#0.1
