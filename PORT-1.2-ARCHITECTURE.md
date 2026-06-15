# FicsIt-Networks — Architektur-Überblick (für den 1.2/UE5.6-Port)

> Erstellt 2026-06-15 als Vorarbeit zum Port. Quelle: Code-Review des Clones unter `C:\Dev\Repository\FicsIt-Networks`.

## Modul-Landkarte (14 Module)

| Modul | Typ | Verantwortung |
|---|---|---|
| **TracyLib** | External | ThirdParty header-only Tracy-Profiler-Wrapper. |
| **Tracy** | Runtime | UBT-Modul um TracyLib (Cpp17). |
| **Eris** | Runtime | Lua 5.4 + Eris-Persistenz (kompletter Lua-Interpreter in `ThirdParty/eris/src`). RTTI+Exceptions, `LUA_API=ERIS_API`. |
| **FicsItLogLibrary** | Runtime | In-Game-Logging (`FILLogContainer/Entry/Scope`) — `/log`-Quelle. |
| **FicsItReflection** | Runtime | **Herzstück** — reflektiert Game-/Mod-API für Skripte. Höchste Game-API-Kopplung. |
| **FicsItFileSystem** | Runtime | Virtuelles FS (`CodersFileSystem`), Devices, FileWatcher. Nur `Core`-Dep → niedriges Risiko. |
| **FicsItNetworksMisc** | Runtime | Querschnitt: Futures, Utils, **CustomVersion (Save-Migration)**, Media, Module-Panel. |
| **FicsItNetworksCircuit** | Runtime | Netzwerk-/Kabel-/Adapter-System, Circuit-Graph, Signals, Wireless. |
| **FicsItNetworksComputer** | Runtime | Computer-Case, Kernel, Processor, Memory/Drive, PCI (GPU/Screen/NetworkCard/Internet). |
| **FicsItNetworksLua** | Runtime | Lua-Runtime + Processor, Lua-API-Module, Reflection→Lua-Bridge, Eris-Persistenz. |
| **FicsItNetworksMicrocontroller** | Runtime | Kompakter Microcontroller-Computer. |
| **FicsItNetworks** | Runtime | **Top-Level** — Buildables (Screen, CodeableSplitter, Poles), Reflection-UI (Slate), Lua-Editor. |
| **FicsItNetworksDocumentation** | Runtime | Commandlets zur Doku-Generierung. |
| **FicsItNetworksEd** | Editor | Localization-Gather-Commandlet. |

## Build-/Abhängigkeitsreihenfolge (Blätter zuerst)
`TracyLib → Tracy / Eris / FicsItLogLibrary → FicsItReflection → FicsItFileSystem / Misc → Circuit → Computer → Lua → Microcontroller → FicsItNetworks → {Ed, Documentation}`

**Kernaussage:** `FicsItReflection` zuerst lauffähig machen — fast alles hängt daran.

## Kernsysteme (Kurz)
- **Reflection:** Eigenes Laufzeit-Typsystem (`UFIRClass/Struct/Function/Signal/Property`) parallel zu UE-UClass. Zwei Sources: `FIRSourceUObject` (generisch zur Laufzeit) + `FIRSourceStatic` (handkuratierte Game-API in `Private/Reflection/Source/Static/FIRSourceStatic_*.cpp`, Makro-Registrierung via `FIRSourceStaticMacros.h`). `AFIRHookSubsystem`/`UFIRHook` fangen Game-Methoden ab → `UFIRSignal`-Events.
- **Computer/Kernel:** `AFINComputerCase` → `UFINKernelSystem` (IFGSaveInterface). Hält Processor (`UFINLuaProcessor`), NetworkController, Log, FS-Root, PCI-Devices. `FutureQueue` entkoppelt Async vom Game-Tick.
- **Lua/Eris:** Lua 5.4 eingebettet. **Eris serialisiert den kompletten Lua-State in den Game-Save** (`eris_persist`/`eris_unpersist`, `FINLuaRuntime.cpp`) — heikelstes Feature.
- **Network/Circuit:** `UFINNetworkConnectionComponent` + `AFINNetworkCable` → `UFINNetworkCircuit`-Graph. Signals + Wireless. RCO-Klassen für MP-Replikation.
- **FileSystem:** `CodersFileSystem`-Namespace, plattformspezifischer FileWatcher.

## Port-Risikozonen (nach Priorität)
1. **Static-Reflection (A, höchstes Risiko):** `FIRSourceStatic_*.cpp` referenzieren direkt Game-Klassen/Properties/private Member. **Railroad** (`FIRSourceStatic_Railroad.cpp`) am fragilsten — `FIRRailroadHelper` greift auf private `FFGRailroadSignalBlock::mOccupiedBy/mPendingReservations/mApprovedReservations`, `AFGBuildableTrainPlatform::mPlatformConnection0`.
2. **Game-Hooks (B, hoch):** `FIRSourceStaticHooks.h` — `SUBSCRIBE_METHOD` auf konkrete Game-Methoden (Railroad Track/Station, PipeHyper, FactoryConnection). Vtable/Signatur-Brüche.
3. **AccessTransformers + SML_Patch (hoch):** `AccessTransformers.ini` friend/accessor auf Game-internals; `SML_Patch.patch` patcht 3 FactoryGame-Header direkt (`FGBuildable.h`, `FGRailroadSignalBlock.h`, `FGBuildableRailroadSwitchControl.h`) — Patch gegen 1.2 neu erstellen.
4. **Serialisierung (D, hoch, nur Laufzeit sichtbar):** Eris-Persistenz, `EFINCustomVersion`-Migration, IFGSaveInterface-Implementierer.
5. **Rendering/Slate (E, mittel):** GPU-T2 eigene `SLeafWidget`/`OnPaint`, Lua-Code-Editor.
6. **ThirdParty (F, mittel):** Eris/Lua C-Tree (MSVC-Bump), Tracy (Cpp17).
7. **Editor/Build-Pfade (G, niedrig-mittel):** `FicsItNetworksEd.Build.cs` hartkodierter PCH-Pfad; `.uplugin` SML/GameVersion.

## Empfohlene Port-Reihenfolge
1. `SML_Patch.patch` + `AccessTransformers.ini` gegen 1.2 neu.
2. `FicsItReflection` (Static-Sources + Hooks, Railroad zuerst).
3. `Computer/Lua` (Kernel-Save, Eris, GPU-Slate).
4. Top-Module (Folgefehler).

Niedrigstes Risiko: `FicsItFileSystem`, `FicsItLogLibrary`, `TracyLib`, `Misc`.
