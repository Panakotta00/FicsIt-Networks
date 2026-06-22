# FIN Port → Satisfactory 1.2 / UE 5.6.1 — Worklog

Branch: `port/1.2-ue5.6` · Engine: `5.6.1-CSS` (CL 83) · SML: v3.12.0
Startdatum: 2026-06-15

Ziel: FicsIt-Networks (zuletzt für Spiel 1.1 / UE 5.3) lauffähig auf Satisfactory 1.2 / UE 5.6.1.

---

## Build-Setup (abgeschlossen)

- Visual Studio 2022 + Workloads *Desktop/Game development with C++*, MSVC v143 14.38, .NET 8 Runtime, .NET 4.8.1 SDK
- CSS Custom UE `5.6.1-CSS` (CL 83) → `C:\Program Files\Unreal Engine - CSS`, registriert
- Wwise 2023.1.14.8770 integriert, Soundbanks generiert
- Starter-Projekt `SatisfactoryModLoader` @ v3.12.0 → `C:\Dev\Repository\SatisfactoryModLoader`
- FIN via Junction `Mods\FicsItNetworks` → `C:\Dev\Repository\FicsIt-Networks`
- Build-Befehl:
  ```
  "C:\Program Files\Unreal Engine - CSS\Engine\Build\BatchFiles\Build.bat" FactoryEditor Win64 Development -project="C:\Dev\Repository\SatisfactoryModLoader\FactoryGame.uproject" -waitmutex -progress
  ```

## Pre-Wave Fixes (vor erstem echten C++-Compile)

| Issue | Fix |
|---|---|
| eris-Submodul leer | `git submodule update --init --recursive ThirdParty/eris` |
| AccessTransformers: `UFGSplinePathMovementComponent → FFIRVehicleHelper` unused (UHT `-WarningsAsErrors`) | Zeile in `Config/AccessTransformers.ini` auskommentiert |
| Duplikat-Basename `FileSystemRoot.cpp` (Non-Unity .obj-Kollision) | `git mv` → `FINKernelFSRoot.cpp` |

---

## Welle 1 — erster echter C++-Compile (Stand: gemessen, noch nicht gefixt)

Build kam bis `[403/428]` — Wwise/SML/Rest kompilieren sauber, **nur FIN-Module brechen**. **33 Fehler**, 4 Kategorien:

| Kat | Typ | Anzahl | Grundursache |
|---|---|---|---|
| A | `C1083` fehlende Includes — FIN-eigene Header | ~7 | UE 5.6: `bLegacyPublicIncludePaths` Default `false` → bare Sub-Ordner-Includes finden nicht mehr |
| B | `C3861` `SA_FIELD_NAME` nicht gefunden | 13 | `SA_FIELD_NAME` (Structured-Archive-Makro) nicht mehr transitiv via `CoreMinimal.h` → `Serialization/StructuredArchive.h` fehlt |
| C | `C1083` Engine-Header umgezogen (`SharedPointer.h`→`Templates/`, `AssetRegistryModule.h`→`AssetRegistry/`, `Paths.h`→`Misc/`, `MSVCPlatformCompilerPreSetup.h`) | ~6 | IWYU-Pfad-Änderungen 5.3→5.6 |
| D | `C2039` `AFGTargetPoint::Get/Set TargetSpeed/WaitTime` kein Member | 4 | **Echte Game-API-Änderung** in 1.2 |

Betroffene Dateien (Auszug): `FicsItReflection/Public/Reflection/FIRExecutionContext.h`, `.../Source/Static/FIRTargetPoint.h`, `FicsItNetworksComputer/.../FINComputerGPUT1.h`, `Tracy/Private/Tracy.cpp`, `Eris/Public/FINLua.h`, diverse `FicsItNetworks*/Public/*.h`.

### Angewandte Fixes (Welle 1)

**Vorab-Recherche (systemisch, vor jeder Änderung Projekt-Suche):**
- `SA_FIELD_NAME`: nicht 13, sondern **45 Usages in 13 Dateien** → Symptom-Fix wäre fatal gewesen.
- Makro in 5.6 ** entfernt** (Engine hat nur noch `SA_VALUE`/`SA_ATTRIBUTE` in `StructuredArchiveNameHelpers.h`); war reiner Passthrough `(x)`.
- Kat-A-Header (`FINSignalData.h`, `FicsItKernel.h`, `File.h`…) sind **FIN-eigen** → eine Grundursache: `bLegacyPublicIncludePaths` in 5.6 default `false`.

**A — `bLegacyPublicIncludePaths = true` in allen 13 FIN-Modulen** (TracyLib ausgenommen, External). Systemisch: stellt rekursive Public-Subordner-Includepfade wieder her → behebt alle FIN-eigenen bare-Includes (auch die noch nicht vom Compiler erreichten). FicsItNetworksEd hatte explizit `false` → auf `true` gedreht.
_Pragmatische Wahl: schneller, reversibler Port. Saubere Alternative wäre vollständige IWYU-Migration (hunderte Includes) — bewusst aufgeschoben._

**B — `SA_FIELD_NAME(TEXT("x"))` → `TEXT("x")`** an allen 45 Stellen (sed, Muster verifiziert uniform). `TryEnterField` nimmt `FArchiveFieldName` (implizit aus `const TCHAR*`).

**C — Engine/Game/SML-Header auf volle Pfade** (deren Module-Settings sind nicht änderbar):
| bare | → |
|---|---|
| `SharedPointer.h` | `Templates/SharedPointer.h` |
| `Paths.h` | `Misc/Paths.h` |
| `AssetRegistryModule.h` | `AssetRegistry/AssetRegistryModule.h` |
| `MSVCPlatformCompilerPreSetup.h` | `MSVC/MSVCPlatformCompilerPreSetup.h` |
| `ModSubsystem.h` (SML) | `Subsystem/ModSubsystem.h` |
| `FGAvailabilityDependency.h` | `AvailabilityDependencies/FGAvailabilityDependency.h` |
| `FGBuildable.h` (4 Dateien!) | `Buildables/FGBuildable.h` |
| `FGBuildableFactoryBuilding.h` | `Buildables/FGBuildableFactoryBuilding.h` |

Projekt-Suche fand **2 zusätzliche** `FGBuildable.h` (FINLuaProcessor.cpp, FicsItNetworksMicrocontroller.cpp), die der Compiler noch nicht erreicht hatte → mitgefixt.

**D — `AFGTargetPoint`:** Getter/Setter in 1.2 entfernt, Member jetzt public (`int32 mTargetSpeed`, `float mWaitTime`). `Get/SetTargetSpeed/WaitTime` → direkter Member-Zugriff; float→int32 explizit gecastet (`bWarningsAsErrors`). 2 Dateien (FIRTargetPoint.h, FIRSourceStatic_Vehicle.cpp).

**Systemische Erkenntnis Welle 1:** Die 33 Fehler hatten nur **4 Grundursachen** — davon 2 echte Engine-/IWYU-Pattern (legacy paths, SA_FIELD_NAME), 1 Header-Umzug-Klasse, 1 echte Game-API-Änderung. Erwartung für Welle 2: Game-Header-Bare-Includes (wie `FGBuildable.h`) sind eine **wiederkehrende Klasse** — werden wellenweise auftauchen, je tiefer der Compiler kommt.

_(Ergebnis Build 4 / Welle 2 folgt.)_

---

## ⚠️ FEATURE-IMPACT: Self-Driving-Vehicle-API (1.2 hat das System ersetzt)

**Befund (2026-06-15):** Der auskommentierte AccessTransformer `UFGSplinePathMovementComponent → FFIRVehicleHelper` war **kein toter Code**, sondern Teil eines **voll ausgelieferten Features**: FINs Skript-API zum Lesen/Schreiben der Route eines selbstfahrenden Radfahrzeugs.

Betroffene Lua-API (`FIRSourceStatic_Vehicle.cpp`, Klasse `AFGDrivingTargetList`): `getTargetList`, `getTarget(s)`, `addTarget`, `removeTarget`, `setTarget(s)`, `setCurrentTarget`. Der Friend-Zugriff erlaubte den Aufruf des privaten `GetSimulationMovement()->SetTarget(...)`.

**Warum „unused": Satisfactory 1.2 hat das gesamte Self-Driving-System ersetzt.**
- WEG in 1.2: `UFGSplinePathMovementComponent`, `GetSimulationMovement()`, `FGWheeledVehicleInfo` (Header existiert nicht mehr). Altes Modell = Route per Fahren aufzeichnen → `AFGTargetPoint`-Wegpunktliste.
- NEU in 1.2: buildbares Pfad-Netz — `UFGVehicleAutopilotComponent`, `UFGVehiclePathPreset`, `AFGVehiclePathSegment` / `FGVehiclePathNode` / `FGStandaloneVehiclePathNode`, `FGVehiclePathValidation`, Hologramme zum Pfad-Bauen.
- `AFGTargetPoint`/`FGTargetPointLinkedList`-Header existieren noch (Legacy/Kompat), werden vom Fahrzeug aber nicht mehr über `GetInfo()->GetSimulationMovement()` angesteuert.

**Konsequenz:** `FIRSourceStatic_Vehicle.cpp` (Vehicle + DrivingTargetList) wird in einer späteren Welle eine Fehler-Wand werfen. Mechanischer Port unmöglich — die Spiel-API ist weg. **Plan: in der Port-Phase stubben/deaktivieren (Build grün halten, klar markieren), NICHTS still verlieren.** Die echte Neuimplementierung gegen das neue 1.2-Pfad-System = **geplantes Feature** „Fahrzeug-Routen/Stations (neu in 1.2)" → nach Absprache. **TODO-Marker: `// FIN-1.2-PORT: vehicle route API disabled — reimplement on UFGVehicleAutopilotComponent/AFGVehiclePathSegment`.**

---

## Welle 2 — vollständig (Fehler-Verlauf: 204 → 114 → 17 → 18 → 0*)

Nach Welle 1 öffnete der Compiler die tieferen, semantischen Schichten. Verlauf der Wellen-2-Iterationen:

| Build | Fehler | Hauptaktion |
|---|---|---|
| 4 | 204 | erste echte C++-Schicht offen |
| 5 | 114 | **TString-Kollision** gefixt (−90) |
| 6 | 17 | **Ed-PCH** gefixt (−97 UE-Header-Kaskaden) |
| 7 | 18 | 17 Einzelfixe (GetIngredients falsch, neue Warnungen) |
| 8 | 0* | GetIngredients korrigiert, GetKeys, Warnings relaxt |

### Systemische Wurzeln (nicht Symptome) — die großen Hebel
1. **`TString`-Kollision** (−90): UE5.6 `template using TString` in `ContainersFwd.h` vs Luas `TString`. Fix: Lua-Typ → `LuaTString` (128×, im eris-Submodul).
2. **Ed-PCH** (−97): `FicsItNetworksEd` nutzte `FactoryGame.h` als `PrivatePCHHeaderFile` → bootstrappt Core in 1.2 nicht (FString/FName undef). Fix: Custom-PCH raus → Shared-PCH.

### Echte Game-API-Änderungen (1.1 → 1.2)
| FIN-Nutzung | 1.2 | Fix |
|---|---|---|
| `AFGTargetPoint` Get/Set Speed/Wait | Member public, `int32` | direkter Zugriff + Cast (Welle 1) |
| `UFGRecipe::GetIngredients(self)` | statisch braucht worldContext | `self->GetDefaultObject<UFGRecipe>()->GetIngredients()` |
| `UFGFactoryConnectionComponent::GetConnector()` | Typ-Enum weg (Conveyor/Pipe = eigene Klassen) | konstant `0` ⚠️ |
| `AFGCentralStorageSubsystem::CanUploadItemsToCentralStorage` | by-type-Check weg | `GetCentralStorageItemLimit()>0` ⚠️ |
| `TMap::GetKeys(TArray<T*>)` | Key ist `TObjectPtr<T>` | `TArray<TObjectPtr<T>>` |
| `FFGRailroadSignalBlock` private | Friend nicht angewandt | `friend class FIRRailroadHelper` in 1.2-Header (SDK-Patch) |
| Self-Driving-Vehicle-System | komplett ersetzt | **gestubbt** → Feature-Phase |

### Deprecations (C4996, sauber gefixt)
- `ForUseOnly…CreateText` → `FText::AsLocalizable_Advanced` (Arg-Reihenfolge!) — 2 Call-Sites, systemisch
- `TArray::Pop(bool)` → `Pop(EAllowShrinking::No)`
- **`AddReferencedObject(UObject*&)`** → `TObjectPtr` — **echte GC-Crash-Gefahr**, nicht nur Kosmetik (2×)

---

## ⚠️ PORT-SCHULDEN (vor Release abarbeiten)

1. **`bWarningsAsErrors = false` in 11 FIN-Modulen** (Welle 2, Build 8). Nötig, weil viele `C4702 unreachable code` / `C4996`-Warnungen in **UE-/generiertem Code** liegen (nicht editierbar). **TODO:** wieder auf `true`, alle Warnungen fixen. Überschneidet sich mit Feature „Code-Review-Probleme fixen".
2. **SDK-Header-Patch** `FGRailroadSignalBlock.h` (`friend class FIRRailroadHelper`) liegt im SatisfactoryModLoader-SDK, **nicht** im FIN-Repo → muss in `SML_Patch.patch` für 1.2 neu erfasst werden, sonst bei frischem SDK-Checkout weg.
3. **Semantische Annahmen verifizieren (in-game):** `GetConnector→0`, `CanUpload→Limit>0` — mit `// FIN-1.2-PORT` markiert.
4. **Vehicle-Reflection** komplett deaktiviert (`.cpp.disabled` als Referenz) → Feature-Phase.

### ✅ Build 9 — `Result: Succeeded`, 0 Fehler
Alle 8 FicsItNetworks-DLLs gelinkt (FactoryEditor Win64 Development). Compile-Port abgeschlossen.

**Letzte Welle-2-Wurzel (Build 8→9):** `C4702 unreachable code` (22×, eskaliert UE5.6 **unabhängig** von `bWarningsAsErrors`). Ursache: Subsystem-Accessoren mit `#if WITH_EDITOR \n return; \n #endif` → im Editor-Build ist der Rest tot, propagiert in `gen.cpp` + Engine-Templates. Fix: `#else` statt `#endif` in 5 Dateien → 0 Fehler.

### ⚠️ Status: NUR Compile — Laufzeit ungetestet
Der Editor-Build linkt. Aber **compile ≠ korrekt**. Noch NICHT verifiziert (braucht laufendes Spiel):
- Railroad-Reflection (Signal-Block-Friend, höchstes Risiko)
- Eris Save/Load des Lua-States
- Game-Hooks (`SUBSCRIBE_METHOD`)
- Die semantischen Annahmen (`GetConnector→0`, `CanUpload→Limit>0`)

**Nächste Phasen:** (1) Mod packen via Alpakit, (2) in-game testen, (3) Port-Schulden abarbeiten, (4) Vehicle-Feature.

---

## Welle 3 — Cook / Laufzeit (CDO-Konstruktion)

Alpakit cookt FIN für **Shipping** (`WITH_EDITOR=0`) — ein *anderer* Build als der Editor (die `#else`-Runtime-Zweige). Der Cook konstruiert alle **CDOs** → deckt **Laufzeit-Konstruktor-Bugs** auf, die der Compile nicht sehen kann.

Alpakit-CLI (verifiziert):
```
RunUAT.bat -ScriptsForProject="<uproj>" PackagePlugin -project="<uproj>" \
  -clientconfig=Shipping -serverconfig=Shipping -utf8output -DLCName=FicsItNetworks \
  -nocompileeditor -CopyToGameDirectory_Windows="<SteamGameDir>"
```
Gut: Shipping-Build von FIN **kompiliert** (alle 10 Module → `*-Win64-Shipping.dll`). Spiel ist auf 1.2 (build 23652534) + SML 3.12.0.

### Cook-Crash #1 — `AFINComputerCase::AFINComputerCase()` (FINComputerCase.cpp:63)
`AddReplicatedSubObject(Log)` im **Konstruktor** crasht in UE5.6 bei der CDO-Konstruktion (Cook). Fix: raus aus Konstruktor → in `BeginPlay` (`if (HasAuthority())`). Registrierung gehört an die Instanz, nicht ans CDO. Nur 1 Vorkommen in FIN (nicht systemisch).

### Offen / beobachten
- **Junction-Warnung:** Asset-Registry meldet Pfad-Mismatch, weil `Mods/FicsItNetworks` ein Junction auf `C:\Dev\Repository\FicsIt-Networks` ist (`GetFilenameOnDisk` liefert echten Pfad). Evtl. Problem fürs Content-Cooking → beobachten; ggf. Junction durch echtes Verzeichnis ersetzen.
- Cook meldete „30 errors" — größtenteils Crash-Kaskade; nach Crash-Fix neu bewerten.

### Cook-Crash #1 behoben → Cook erfolgreich (Content-`.pak` cookt)
Nach dem FINComputerCase-Fix: `BUILD SUCCESSFUL`, Content cookt, `.pak/.ucas/.utoc` erstellt.

### ⚠️ Packaging-Falle: Alpakit baute EGS statt Steam + deployte keine Binaries
- Alpakit-`PackagePlugin` (CLI) baute `FactoryGameEGS-*-Shipping.dll` — **falsch für ein Steam-Spiel** (braucht `FactoryGameSteam-*`). Und re-Cooks bauten die Shipping-DLLs gar nicht neu (Cache) → NonUFS-Manifest enthielt nur `.uplugin`+Resources, **keine DLLs**.
- **Lösung:** Shipping-Target **explizit** bauen: `Build.bat FactoryGameSteam Win64 Shipping -project=...` → erzeugt die 12 korrekten `FactoryGameSteam-*-Win64-Shipping.dll`. Dann manuell ins Spiel deployen (DLLs + `.modules` + `.pak` + `.uplugin`).
- **Workflow-Regel:** Nach jeder FIN-Änderung: (1) `FactoryEditor`-Rebuild (für Cook-Commandlet), (2) `FactoryGameSteam Shipping`-Rebuild (für Runtime-DLLs), (3) Cook (Content), (4) deploy.
- Mod-BuildId `43139311` = Engine-`CompatibleChangelist` → passt zum Spiel.
- **Deploy-Stash:** `C:\Dev\Repository\_FIN-1.2-deploy\` (kompletter Mod-Ordner, ein-Befehl-Redeploy, falls SMM den Spiel-Ordner putzt).

### ✅ Stand: vollständige ladbare 1.2-Mod im Spiel deployt — bereit für In-Game-Test (Stufe 1)

### Cook-Blocker #2 — BuildId-Mismatch ("module could not be found")
Native UE-Modulladung lehnte FIN ab: Mod-BuildId `43139311` (Engine `CompatibleChangelist`) ≠ Spiel-BuildId `493833` (Spiel-Changelist, `CompatibleChangelist=0`). **Versions-Schere:** Spiel auf Patch **1.2.3 / CL 493833**, Modding-Toolchain (css-83 Engine, SML 3.12, Starter `currentVersion.txt`) zielt auf **491125**.
**Fix (Port-Schuld #5):** Engine `Build.version` `CompatibleChangelist` → `493833` angeglichen (Backup `.fin-backup`), Shipping neu gebaut → Mod-BuildId `493833`. BuildId lebt im `.modules`-Manifest (kein DLL-Recompile nötig). **Bei künftigem Spiel-Patch erneut angleichen.**

### 🎉 FIN LÄDT IM SPIEL (Satisfactory 1.2.3 / CL 493833)
Hauptmenü erreicht, **FicsItNetworks geladen** (2 Mods: SML+FIN), **Settings-UI funktioniert** (Log Viewer, Parametric Blueprints). Compile→Cook→Package→Load-Pfad steht.
**Noch zu verifizieren (Laufzeit):** Welt laden ohne Crash, Computer platzieren + Lua, Eris Save/Load, Railroad, die semantischen Annahmen.

---

## Welle 4 — In-Game-Laufzeit (begonnen)

### ✅ Verifiziert
- Welt lädt ohne Crash. Alle FIN-Buildables im Baumenü, Icons + Beschreibungen rendern.
- Computer Case + CPU T1 + RAM T1 + Screen Driver platzierbar.
- Cook-Content sauber: 909 Assets korrekt gemountet (`/FactoryGame/Mods/FicsItNetworks/Content/...`), 103 Cook-Warnungen alle benign (Game-BP-Warnings + Redirectoren).

### 🔴 BUG #12 — Computer-Config-UI öffnet nicht (E)
**Symptom:** „Press E to configure Computer Case" erscheint, aber E **öffnet nichts** — und erzeugt **NULL Log-Einträge** (kein Widget, kein Interact, kein Fehler).
**Diagnose:** `AFINComputerCase` (C++) hat keinen Interakt-Widget-Code → reines `AFGBuildable`. Das Config-UI ist **Blueprint-getrieben** (`mInteractWidgetClass` im `Build_ComputerCase`-BP). Kein Widget + kein Log ⇒ die Widget-Referenz ist null/kaputt **oder** die Interaktions-Widget-Anbindung hat sich in 1.2 geändert (Game-Interaction-System).
**WURZEL GEFUNDEN (systemisch, alle FIN-Buildables mit Config-UI):** 1.2 hat die Interakt-Widget-Property in `AFGBuildable` umgestellt:
- WEG: `mInteractWidgetClass` (harte `TSubclassOf<UFGInteractWidget>`)
- NEU: `mInteractWidgetSoftClass` (`TSoftClassPtr<UFGInteractWidget>`, `FGBuildable.h:1047`); `GetInteractWidgetClass()` lädt daraus.
FINs Buildable-Blueprints (Build_ComputerCase, Build_NetworkRouter, …) setzen die **alte, in 1.2 nicht mehr existierende** Property → Wert geht beim Load verloren → Soft-Property leer → `GetInteractWidgetClass()` = null → kein Widget, kein Log. Verifiziert: Vanilla-Config (Storage Container) öffnet normal; mehrere FIN-Buildables brechen identisch.

**FIX-PLAN (Blueprint-Daten-Migration):** In jedem FIN-Buildable-BP mit Config-UI `mInteractWidgetSoftClass` auf das frühere Interakt-Widget setzen (Wert von altem `mInteractWidgetClass`).
- Weg A (Editor): jedes betroffene `Build_*`-BP öffnen, Soft-Property setzen.
- Weg B (C++): `mInteractWidgetSoftClass` per PostLoad/Konstruktor setzen (braucht AccessTransformer-Zugriff auf die geschützte Property + Wissen welches Widget pro Buildable). Systemischer, falls eine gemeinsame Stelle existiert.
- Betroffene BPs identifizieren: alle FIN-`Build_*` die früher `mInteractWidgetClass` setzten (Computer Case nutzt `UFINComputerCaseWidget`-BP).
**Zyklus:** Fix → re-cook → Game-Restart. **Blockiert** den In-Game-Lua-Test-Loop (EEPROM-Flash nur über diese UI). = Priorität #1 nächste Session.

### 🟡 BUG #13 — Recipe-null-Flood beim Welt-Laden (nicht-blockierend)
`FGRecipe::GetRecipeName: class was nullptr` ~28× in 5s beim Laden, dann Stille. Nur Warnungen. Rezepte cooken korrekt (`Recipe_CodeableMerger` etc. im .pak) → vermutlich Schematic-/Unlock-Referenz auf null-Recipe-Klasse. Separat untersuchen.


### ✅ Bug #12 GEFIXT — Config-UI (Property-Redirect)
mInteractWidgetClass->mInteractWidgetSoftClass CoreRedirect in DefaultFicsItNetworks.ini.
Migration am gecookten Asset verifiziert. In-game bestaetigt: Config-UI oeffnet (alle FIN-Buildables).

### ✅ Bug #14 GEFIXT — EEPROM erstes Einsetzen kein Live-Update
UFINComputerCaseWidget::NativeConstruct bindet OnEEPROMUpdate (AddUniqueDynamic) + pusht GetEEPROM().
In-game bestaetigt: EEPROM wird sofort angezeigt.

### ✅ Bug #15 GEFIXT — UI-Input-Lock nach netFunc-Fehler (a156452)
Reflection-Funktion wirft FFIRException -> wurde in C++-catch gefangen -> `luaL_error` (longjmp)
WAEHREND der catch noch aktiv war -> kaputter EH-Zustand -> Spiel-Eingabe blockiert (Klicks tot,
nur Alt-Tab half). Fix (LuaRef.cpp): catch zuerst verlassen, DANN luaL_error. Passiv bewiesen:
ganze Session laufend netFunc-Fehler im Reflection-Walk, Eingabe nie geklemmt. Betrifft den
PARALLEL-Thread-Pfad (RT_Parallel / luaL_error).

### ✅ BUG #16 GEFIXT — runtime-0-Throw liefert fangbaren Lua-Fehler statt [Fatal]
**Vollstaendig gefixt + in-game verifiziert 2026-06-22:** `identifier:addWaypoint("not-a-guid")` (in pcall)
liefert jetzt `ok=false, err='invalid GUID string'` (die ECHTE Meldung), **kein `[Fatal]`, kein Crash/
Freeze**, Heartbeat laeuft stabil weiter (R628->630). DREI zusammenwirkende Teile:
1. `FFINFutureReflection::Execute()` (FINFuture.cpp, laeuft GAME-Thread) komplett in try/catch -> faengt
   die geworfene FFIRException ab (sonst [Fatal], weil kein C++-catch in den Lua-C-Frames) -> setzt
   `bError`/`ErrorMessage` (FINFuture.h) + `bDone` IMMER (sonst haengt await ewig).
2. `lua_futureStructContinue` (LuaFuture.cpp) prueft nach IsDone `HasError()` -> `luaL_error(echte Msg)`
   -> die await-Coroutine wird damit zu Future_Failed.
3. **DER FEHLENDE TEIL** (per UE_LOG-Diagnose gefunden): `poll_continue` (LuaFuture.cpp) schloss den
   Coroutine-Thread bei JEDEM Resume-Ende per `lua_closethread` — also auch bei ERROR-Status -> der
   Fehler wurde verworfen, der Thread zurueckgesetzt, await sah kein Future_Failed mehr -> generisches
   "poll reported finished, but future is not ready". Fix: `lua_closethread` nur noch bei `status==LUA_OK`;
   bei Fehler bleibt der Thread im Failed-Zustand -> await propagiert die echte Meldung. War ein
   VORBESTEHENDER latenter Bug (await ist laut Doc-Comment "propagates error") den erst eine werfende
   runtime-0-Funktion sichtbar machte. Betrifft jetzt ALLE Futures positiv (Lua-async-Fehler propagieren
   auch sauber). Diagnose-UE_LOGs wieder entfernt. Urspruengliche Analyse:
ENTDECKT + BESTAETIGT 2026-06-22 beim #15-Test. Eine Reflection-Funktion mit runtime=0 (Game-Thread,
via Future) die eine FFIRException wirft (z.B. `addWaypoint`/`insertWaypoint`: `if(!FGuid::Parse) throw`)
liefert die Exception NICHT als Lua-Fehler aus -> `pcall` faengt sie NICHT -> sie wird `[Fatal]` und
HALTED den Computer-Kernel (eingefroren, Heartbeat stoppt). Beweis im Spiel-Log:
`[REPL] --- seq inputlock ---` direkt gefolgt von `[Fatal] invalid GUID string`, obwohl der Aufruf
in `pcall` lag. Game-Thread selbst laeuft weiter (Spiel reagiert), nur der Computer ist tot -> Reboot/
Power-Cycle noetig. ANDERS als #15 (Parallel-Thread). FIX-Richtung: im Game-Thread-Future-Executor den
Body in try/catch, Exception in den Future-Fehlerkanal legen -> propagiert sauber als fangbarer Lua-Fehler
statt [Fatal]. Betrifft ALLE runtime-0-Funcs mit InVal/throw (addWaypoint, insertWaypoint, linkTo mit
ungueltigem Arg, ...). Siehe property-setter-threading + ue56-exception-crash Memories.
