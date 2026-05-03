# Infusion Viewer

## About this project
- It started as a fun project some years ago, where I wrote some cheat engine script for darksouls 3 in order to present client sided visuals for all elemental infusions of weapons
- recently I wanted to build a proper mod for it, and started to work on it in elden ring
- this project was just created to play around hacking in windows programming and rendering overlays

## Details
- presents a mod in form of an injectable dll made for eldenring (without easy anticheat evasion) and darksouls 3
- Only made for singleplayer or seamless coop mod
- Weapons which are infused with elemental effects, will receive a corresponding visualization, by adding a representing vfx effects on the weapon itself
- You can adjust phantom colors on your self and other players in your lobby
- All these visuals are clientsided and should not affect other players, when playing in a online session
- Contains a directx overlay hook menu, in order to configure visualization
  - you can open the menu with "ALT+Num1", there you will find more information about features and configuration
 
## Installation
- the dll file can be injected after the process of the game was started (no suspend)
- you can either use:
  - an injection tool like Cheat Engine
  - [me3](https://github.com/garyttierney/me3) --> probably the best in order to have a safe and clean setup
  - or use my hastly written [dinput8.dll loader](https://github.com/Jackydima/JDModLoader)
  - any other method of Loading a dll file into the game ...

## Open things
- include more variants of visual weaponbuffs, configureable with the menu
- resolve presenting visual effects (ffx) without relying on SPEffects params by calling game functions
- add some other convinient stuff to the mod
- provide saving the configuration for the mod

## Warnings
- I will not take responsibility, when you will get banned while using it in online vanilla

## Credits
- [The Grand Archives](https://github.com/The-Grand-Archives) for providing some aob patterns and offset references and the speffect struct for ds3
- [libER](https://github.com/Dasaav-dsv/libER) for grapping the speffect structs for eldenring
- [ImGui examples](https://github.com/ocornut/imgui/tree/master/examples) for presenting practical examples for rendering a menu
- [MinHook](https://github.com/tsudakageyu/minhook) for hooking functions
- ?ServerName?, for sharing details about modding the game
