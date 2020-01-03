FicsIt-Networks
===============
FicsIt-Networks is a mod for Satisfactory written in >= SML 1.1 which allows you to control, mointor, manage and automate each process of your factory by providing a network system and programmable computers and other I/O.

**If you want to learn more:**
[please visit the Documentation](https://docs.ficsit.app/ficsit-networks/0.0.1)

Work In Progress
================
This project is in no finished state!
We are constantly working on features to get the mod to release soon.
This doesn't mean you can't build the mod your self to test it though.

Features
========
- Networks
  This is a basic network like the existing power network allowing you to connect buildings with a network connector to the network.
  When placing a network cable you're also able to automatically add a network adapter to existing Satisfactory Machines to connect them to the network.
- Modular Computer
  The modular computer is a building wich has  network connector and a power connector.
  The basic concept is, that you have panele were you can place different modules like the cpu, ram-bars or a drive enabling different features for your computer.
  There is currently one type of computer, the Lua Processor. It allows you to run complex Lua code which uses the network to interact with your factory.
- Modular I/O Panel
  The modular I/O Panel is basically a control panel were you can attach different I/O modules like buttons, levers, LEDs, displays and so on. to read and write data to and from a computer.
- Factory Hook
  A so called Factory Hook allows you to listen to a conveyor connection so you get basically push signals when the connector transfers a item. You can attach such a hook in code when you get the factory connector structure from a network adapter.
- File Systems
  You can read and write files to and from a virtual file system in f.e. a hard drive wich you can connect to a computer by the drive holder computer module.
- Speakers
  With the Speaker-Pole you can play custom sound files ingame on command. So you can alert the player automatically when f.e. the Iron stock runs low.
- Codeable Splitters/Mergers
  There are also Splitters and Mergers able to connect to the network allowing you to fully customize their behaviour by controlling each step from a computer via. f.e. a lua code.
- Power Controller
  The Power Controller Pole is basically a smart power switch allowing you to connect and disconnect two different power networks.
- Network Adapter
  The network adapter allows you to connect any satisfactory machine to the network and allow some basic and some more complex interactions like reading inventories.

Dependencies
============
For using the mod there are no dependencies except [Satisfactory Modloader v1.1](https://github.com/satisfactorymodding/SatisfactoryModLoader)

For cooking the unreal content we use a copy of Alpakit provided in the [Satisfactory Modding Unreal Project](https://github.com/satisfactorymodding/SatisfactoryUnrealProject)

As general project build dependency we have Lua 5.3 to run Lua code.

Roadmap
=======
You can find the detailed progress in the [project board](https://github.com/CoderDE/FicsIt-Networks/projects/1).

Until first release:
- Transportable File System (items holding file systems like hard drives)
- Documentation
- fixing some bugs
- make component representation invalid when component gets disconnected
- add signals and function to check if power circuite fused and when component network updates
- reimplement module system
- add more functions to make component lookup/search easier
- add component nick system
- add new sensors (like truck or player sensors)
- add new I/O modules for the modular control panels (like Text-Display)
- add modular computer modules (f.e. CPUs, memory sticks)
- add eeprom (and eeprom holder for computer)

After release:
- tune and make improvements to general concepts and performance
- adding support for a visual scripting language
- port the mod to official mod kit (when released)

Contributors
============
- Panakotta00
- RosszEmber
