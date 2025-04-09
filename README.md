# FicsIt-Networks

FicsIt-Networks allows you to control, mointor, manage and automate each process of your factory by providing a network system and programmable computers, aswell as other I/O.
It is inspired by [OpenComputers](https://github.com/MightyPirates/OpenComputers).

**If you want to learn more:**
[please visit the Documentation](https://docs.ficsit.app/ficsit-networks/latest)
[or the SMR Mod page](https://ficsit.app/mod/8d8gk4imvFanRs)

<a href="https://discord.gg/3VfZ6Da"><img height="50px" src="https://gotpa.ws/img/join_discord.png" /></a>
<a href="patreon.com/ficsitnetworks"><img style="height: 36px; margin: 10px;" src="https://raw.githubusercontent.com/Panakotta00/FicsIt-Networks/refs/heads/development/Resources/patreon_banner.png" /></a>
<a href='https://ko-fi.com/X8X51D8D13' target='_blank'><img style='height:50px; margin: 10px;' src='https://storage.ko-fi.com/cdn/kofi6.png?v=6' border='0' alt='Buy Me a Coffee at ko-fi.com' /></a>

## Development Versions

If you want to test the mod and so help with development,
we would reccomend you to join the [FicsIt-Networks Discord Server](https://discord.gg/3VfZ6Da) and contact one of the mod developers for a more in depth help.
Testing means, it would not be a good idea to use the mod in your normal game state,
testing means testing the mod in its own world to make sure every features works as intended.

You can download the latest builds:

[ [Latest Stable Version](https://github.com/Panakotta00/FicsIt-Networks/releases/latest) ] 
[ [Development Versions](https://github.com/Panakotta00/FicsIt-Networks/actions?query=branch%3Adevelopment) ]

To install it, simply download the `FicsItNetworks-Windows.zip` artifact and extract the contents of it into the `<sf installation>/FactoryGame/Mods/FicsItNetworks` folder.
It has to be this exact path and folder name! Extract it in a way, so the `.uplugin`-File is located within this folder.

## Features

- **Networks**  
This is a basic network like the existing power network allowing you to connect buildings with a network connector to the network.
When placing a network cable you're also able to automatically add a network adapter to existing Satisfactory Machines to connect them to the network.
These networks then allow you to access function from machines and get automatically send signals from machines e.g. no need of polling.
- **Modular Computer**  
The modular computer is a building wich has  network connector and a power connector.
The basic concept is, that you have panele were you can place different modules like the cpu, ram-bars or a drive enabling different features for your computer.
There is currently one type of processor, the Lua Processor. It allows you to run complex Lua code which uses the network to interact with your factory.
- **Modular I/O Panels**  
The modular I/O Panel is basically a control panel were you can attach different I/O modules like buttons, levers, LEDs, displays and so on. To read and write data to and from a computer.
- **Screens**  
Monitors/Monitors and GPUs allow you to visualize data in multiple different forms. They also allow you to use montiors as user inputs like Keyboard, Mouse and Touch.
- **Extensive API**  
Most of the games mechanics are exposed to the script environment using the reflection system. The in-game reflection viewer allows you to easily discover exposed functionallity for every machine and most buildings. Even other mods that do not explicity support FicsIt-Networks have some API available to work with.
- **Lua Scripting**  
Lua is a first-class-citizen and is used to programm in-game computers. The reflection system and even more API is exposed to the Lua runtime giving you a lot of freedom on how to control your factory.

## Build from Scratch
1. Setup the Mod-Development Environment for Satisfactory Modding. More info can be found in the [SMR Docs](https://docs.ficsit.app/satisfactory-modding/latest/Development/BeginnersGuide/index.html).
2. Now you have to clone this repository **recursively** into the Mods folder of the [Satisfactory Modloader](https://github.com/satisfactorymodding/SatisfactoryModLoader) repository folder you cloned in the setup process.
3. Apply the `SML_Patch.patch` file using `git apply` to the Satisfactory Modloader Repository folder. This will add simple changes to the header files that are necessary for FicsIt-Networks to build properly.
4. Now you can build FicsIt-Networks like any other Satisfactory Mod using your build tool of choice or Alpakit.

FicsIt-Networks relies on a slightly custom version of Eris for Lua 5.4 to run Lua code and persistency. The changes only apply to C++ compiler support and persistency adjustments. The dependency will be built together with the normal mod, so you only have to ensure the submodule is cloned properly.

## Contributors
- [Panakotta00](https://panakotta00.dev)
- [Roze](https://github.com/RozeDoyanawa)
- [RosszEmber](https://www.deviantart.com/ronsemberg)
- Deantendo
- Coffeediction
- [Raysh](https://www.artstation.com/raysh)
- Esper
- [leonardfactory](https://github.com/rockfactory)
