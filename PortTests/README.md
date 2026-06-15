# FIN Port-Tests (Regressions-Suite für den 1.2/UE5.6-Port)

Self-contained In-Game-Testharness. Jeder beim Port gefundene Bug bekommt hier
einen Test, damit er nicht stillschweigend zurückkommt (z. B. beim nächsten
Spiel-Patch). Läuft **im Spiel** über einen dedizierten FIN-Test-Computer und
meldet die Ergebnisse an einen kleinen lokalen Server.

## Aufbau

| Datei | Rolle |
|---|---|
| `testserver.py` | Minimaler HTTP-Server: serviert die `.lua`-Tests + sammelt Ergebnisse (`POST /log`). Self-contained, **keine** Planner-Abhängigkeit. |
| `boot_modtest.lua` | EEPROM-Bootloader für den Test-Computer — holt `modtest.lua`, online-only, mit Retry. |
| `modtest.lua` | Test-Runner (Framework) + Test-Suite. Pro verifizierter API / gefixtem Bug ein Test. |

## Ausführen

1. Planner-Server (`fin/server.py`) stoppen, falls er auf 8080 läuft (Port-Konflikt).
2. In diesem Ordner: `python testserver.py`  (bindet 127.0.0.1:8080).
3. Dedizierten FIN-Test-Computer bauen: **Computer Case + CPU T1 (Lua) + RAM T1 + Internet Card + Strom**.
4. EEPROM mit dem Inhalt von `boot_modtest.lua` flashen, Computer starten.
5. Ergebnisse: erscheinen in der `testserver.py`-Konsole **und** unter `http://127.0.0.1:8080/results`.

Bei Code-Änderung an `modtest.lua` reicht ein **Computer-Neustart** (online-only, kein Cache).

## Test-Status
- `PASS` — API/Verhalten wie erwartet.
- `FAIL` — Regression / Port-Bug. Begründung steht dabei.
- `SKIP` — Vorbedingung fehlt (z. B. keine Maschine am Netzwerk). Aktivieren durch das genannte Setup.

## Bug-Katalog (Port 1.2 → Test-Mapping)

| # | Bug (gefunden beim Port) | Test | Art |
|---|---|---|---|
| 1 | TString-Kollision (UE5.6 `TString<>` vs Lua) | implizit: Lua-VM läuft (`core: lua-vm`) | auto |
| 2 | `bLegacyPublicIncludePaths` / Includes | implizit: Mod kompiliert + lädt | build |
| 3 | `SA_FIELD_NAME` entfernt | implizit: Serialisierung kompiliert | build |
| 4 | `AFGTargetPoint` int32 speed/wait | Vehicle-API (derzeit gestubbt) | TODO |
| 5 | `UFGRecipe::GetIngredients` → CDO | `port: recipe getIngredients` | auto (skip ok) |
| 6 | `GetConnector` → 0 (Conveyor) | `port: FactoryConnection .type` | hw (Maschine nötig) |
| 7 | `CanUploadItemsToCentralStorage` → Limit>0 | `port: CentralStorage canUpload` | hw |
| 8 | Railroad `FFGRailroadSignalBlock` friend | `port: railroad signal block` | hw (Gleis/Signal) |
| 9 | `FINComputerCase` CDO-Crash (Cook) | implizit: Computer existiert/bootet | manual/build |
| 10 | `bWarningsAsErrors` (Port-Schuld) | n/a (build) | — |
| 11 | BuildId-Mismatch | implizit: Mod lädt im Spiel | manual |

## Manuelle Checkliste (nicht aus Lua testbar)
- [ ] Mod lädt im SML-Screen (grün), 2 Mods.
- [ ] Welt lädt ohne Crash.
- [ ] Alle FIN-Buildables im Baumenü, Icons rendern.
- [ ] **Computer-Config-UI öffnet (E)** — *aktuell BUG, in Untersuchung.*
- [ ] GPU/Screen rendert (Slate `OnPaint`).
- [ ] Eris: Save mit FIN-Computern speichern + laden ohne Crash.
- [ ] Railroad: Train-Manager-Szenario.
