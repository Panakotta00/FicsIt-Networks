FicsIt-Networks [![Build Status](https://jenkins.massivebytes.net/job/FicsIt-Networks/job/master/badge/icon)](https://jenkins.massivebytes.net/job/FicsIt-Networks/job/master)
===============
FicsIt-Networks is a mod for Satisfactory, written in C++/BP using Unreal Engine and the Satisfactory Modloader, which allows you to control, monitor, manage and automate each process of your factory by providing a network system and programmable computers, aswell as other I/O.
It is inspired by [OpenComputers](https://github.com/MightyPirates/OpenComputers).

**If you want to learn more:**
[please visit the Documentation](https://docs.ficsit.app/ficsit-networks/latest)
[or the SMR Mod page](https://ficsit.app/mod/8d8gk4imvFanRs)

Discord
=======
You can join our [discord server](https://discord.gg/3VfZ6Da) where you can get help for using the mod, where you can share your scripts or just simply discuss the mod.

Work In Progress
================
This project is in no finished state!
We are constantly working on features to make the mod even greater.

Want to test the Mod?
=====================
If you want to test the mod and so help with development,
we would reccomend you to join the [FicsIt-Networks Discord Server](https://discord.gg/3VfZ6Da) and contact one of the mod developers for a more in depth help.
Testing means, it would not be a good idea to use the mod in your normal game state,
testing means testing the mod in its own world to make sure every features works as intended.

You can download the latest builds:

[ [Latest Stable Version](https://jenkins.massivebytes.net/job/FicsIt-Networks/job/master/) ] 
[ [Development Version](https://jenkins.massivebytes.net/job/FicsIt-Networks/job/development/) ]

To install it, simply download the .zip artifact and extract the contents of it into the `<sf installation>/FactoryGame/Mods/FicsItNetworks` folder.
It has to be this exact path and folder name! Extract it in a way, so the `.uplugin`-File is located within this folder.

Streams
=======
The biggest part of making this mod gets streamed at [Panakotta00's Twitch Channel](https://twitch.tv/panakotta00).

You-Tube
========
- [Release Trailer](https://www.youtube.com/watch?v=EErI0OiWttw)
- [Mod Showcase by RandomGamer](https://www.youtube.com/watch?v=EtybEOkgJ4o)
- [Tutorial Series](https://www.youtube.com/playlist?list=PLKTdAeAt_BilFGjKoIG9GObwjqmxdSoeE)
- [Lua Tutorial Series](https://www.youtube.com/playlist?list=PLKTdAeAt_BimxLkH05GSNBZpydxc553hE)

Features
========
- Networks
  This is a basic network like the existing power network allowing you to connect buildings with a network connector to the network.
  When placing a network cable you're also able to automatically add a network adapter to existing Satisfactory Machines to connect them to the network.
  These networks then allow you to access function from machines and get automatically send signals from machines e.g. no need of polling.
- Modular Computer
  The modular computer is a building wich has  network connector and a power connector.
  The basic concept is, that you have panele were you can place different modules like the cpu, ram-bars or a drive enabling different features for your computer.
  There is currently one type of processor, the Lua Processor. It allows you to run complex Lua code which uses the network to interact with your factory.
- Modular I/O Panel
  The modular I/O Panel is basically a control panel were you can attach different I/O modules like buttons, levers, LEDs, displays and so on. To read and write data to and from a computer.
- File Systems
  You can read and write files to and from a virtual file system in f.e. a hard drive which you can connect to a computer by the drive holder computer module.
- Speakers
  With the Speaker-Pole you can play custom sound files ingame on command. So you can alert the player automatically when f.e. the Iron stock runs low.
- Codeable Splitters/Mergers
  There are also Splitters and Mergers able to connect to the network allowing you to fully customize their behaviour by controlling each step from a computer via. f.e. a lua code.
- Power Controller
  The Power Controller Pole is basically a smart power switch allowing you to connect and disconnect two different power networks.
- Network Adapter
  The network adapter allows you to connect any satisfactory machine to the network and allow some basic and some more complex interactions like reading inventories.
- Monitors & GPUs
  Monitors and GPUs allow you to visualize data in multiple different forms. They also allow you to use montiors as user inputs like Keyboard, Mouse and Touch.
- Vehicle Scanner
  Allows to interact with vehicles that pass over it. It also looks nice ;-)
- Reflection System
  The Reflection System allows to check data-types, functions, properties, signals and more. You can look up all sort of information and it helps you
  for a more dynamic way to interact with the machines. This system also provides a abstraction layer so that further language implementations and such can get implemented more
  easily and quicker. It also provides a dependency-less system so other mods can provide functionality that can be used by ficsit-networks.
  It also provides the reflection viewer which is UI Widget allowing you to browse and explore the reflection data, like an ingame documentation.
- Internet Card
  The Internet Card allows you to connect to the internet so you can do f.e. http requests and download
  custom user made scripts from f.e. gist or pastebin. But you can use it in anyway, like talking to an REST API.
- Wireless Networks
  You can broaden your horizons with Wireless Access Points which connect to already built Radar Towers and allow wireless communications over long distances.

Dependencies
============
For using the mod there are no dependencies except the [Satisfactory Modloader](https://github.com/satisfactorymodding/SatisfactoryModLoader) for the game version and mod version you want to use.

As general project build dependency we have Eris for Lua 5.3 to run Lua code and persistency.

Roadmap
=======
You can find the detailed progress in the [project board](https://github.com/CoderDE/FicsIt-Networks/projects/1).

- add new sensors (like player sensors)
- microcontrollers
- computer power consumption
- adding support for a visual scripting language
- port the mod to official mod kit (when released)
(the list is dynamic and gets updated based on new ideas)

Contributors
============
- [Panakotta00](https://panakotta00.massivebytes.net)
- [RosszEmber](https://www.deviantart.com/ronsemberg)
- Deantendo
- Coffeediction
- [Roze](https://github.com/RozeDoyanawa)
- [Raysh](https://www.artstation.com/raysh)
- Esper
- [leonardfactory](https://github.com/rockfactory)
