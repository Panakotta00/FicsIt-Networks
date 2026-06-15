# Referenz: Altes Vehicle-Skript-API (≤1.1) — Anforderungs-Spec für 1.2-Neuimplementierung

> Erfasst 2026-06-15 aus `FIRSourceStatic_Vehicle.cpp` (Stand vor dem Port).
> Zweck: festhalten, **was Scripts früher konnten**, damit wir beim Bau gegen das neue
> 1.2-Pfad-System (`UFGVehicleAutopilotComponent`, `UFGVehiclePathPreset`,
> `AFGVehiclePathSegment`/`-Node`) prüfen können, was nötig ist für gleichwertige Nutzung.

## Was Scripts mit dem alten System konnten

### `Vehicle` (AFGVehicle, Basis)
| API | Art | Funktion |
|---|---|---|
| `health` / `maxHealth` | read | Fahrzeug-Gesundheit |
| `isSelfDriving` | read | fährt gerade autonom? |

### `WheeledVehicle` (AFGWheeledVehicle)
| API | Art | Funktion |
|---|---|---|
| `isAutopilotEnabled` | read/write | Autopilot an-/ausschalten (`Server_ToggleAutoPilot`) |
| `getFuelInv` / `getStorageInv` | call | Treibstoff-/Lager-Inventar |
| `isValidFuel(item)` | call | ist Item gültiger Treibstoff? |
| `getCurrentTarget()` | call | **Index des Wegpunkts**, zu dem das Fahrzeug gerade fährt |
| `setCurrentTarget(index)` | call | **Fahrzeug zu Wegpunkt-Index kommandieren** ⟵ brauchte den privaten `SetTarget`-Zugriff |
| `getTargetList()` | call | liefert die Routen-Liste (`TargetList`) |
| `speed` / `burnRatio` / `hasFuel` | read | Fahrdaten |

### `TargetList` (AFGDrivingTargetList) — die Route
| API | Art | Funktion |
|---|---|---|
| `getTarget(index)` | call | Wegpunkt-Struct an Index |
| `getTargets()` | call | **alle** Wegpunkte als Array |
| `addTarget(tp)` | call | Wegpunkt anhängen |
| `removeTarget(index)` | call | Wegpunkt löschen |
| `setTarget(index, tp)` | call | Wegpunkt überschreiben (Pos/Rot/Speed/Wait) |
| `setTargets(array)` | call | **komplette Route ersetzen** |

### `TargetPoint`-Struct (FFIRTargetPoint)
`Pos` (Vector), `Rot` (Rotator), `Speed` (float → in 1.2 als int32 gespeichert!), `Wait` (float)

### `DockingStation` (AFGBuildableDockingStation)
`getFuelInv`, `getInv`, `getDocked`, `undock()`, `isLoadMode` (r/w), `isLoadUnloading` (r)

## Kern-Capability (das, was nachgebildet werden muss)
**Vollständig programmatische Routen-Kontrolle:** Script kann die gesamte Wegpunkt-Route eines
selbstfahrenden Trucks **auslesen, einzeln/komplett verändern, Wegpunkte hinzufügen/löschen**,
und das Fahrzeug **gezielt zu einem Wegpunkt schicken** — plus Autopilot-Toggle und Docking-Steuerung.

## OFFENE KERNFRAGE für die 1.2-Neuimplementierung
1.2-Pfade sind **gebaute** Segment-/Node-Netze (`AFGVehiclePathSegment`/`FGVehiclePathNode`),
nicht mehr eine editierbare Wegpunktliste. **Zu klären:**
- Lässt sich das Pfad-Netz zur Laufzeit per Code überhaupt noch erzeugen/ändern (Segmente spawnen)?
  Oder sind Pfade jetzt rein bau-/hologramm-basiert (→ Script kann nur noch *lesen* + Autopilot steuern)?
- Gibt es `UFGVehiclePathPreset` als programmatisch wählbare „Routen" (näher am alten setTargets)?
- Mapping alt→neu: `setCurrentTarget` → ? , `addTarget`/`setTargets` → ? , `getTargets` → Segment-Iteration?

→ Diese Fragen beantworten wir in der Feature-Phase (nach Absprache), bevor wir bauen.
