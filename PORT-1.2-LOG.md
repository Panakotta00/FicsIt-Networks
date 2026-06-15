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

\* Build-8-Ergebnis wird unten bestätigt.
