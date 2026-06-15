// FIN-1.2-PORT: KOMPLETT DEAKTIVIERT (temporär).
//
// Das Self-Driving-Vehicle-System wurde in Satisfactory 1.2 ersetzt:
//   WEG:  UFGSplinePathMovementComponent, AFGWheeledVehicle::GetInfo()->GetSimulationMovement(),
//         FGWheeledVehicleInfo(.h), AFGDrivingTargetList (Wegpunkt-Liste),
//         IsAutopilotEnabled/Server_ToggleAutoPilot (Autopilot war Methode am Vehicle)
//   NEU:  UFGVehicleAutopilotComponent, UFGVehiclePathPreset, AFGVehiclePathSegment/-Node,
//         FGVehiclePathValidation (buildbares Pfad-Netz statt aufgezeichneter Route)
//
// Die alte Reflection (Vehicle / WheeledVehicle / TargetList / DockingStation) lässt sich
// NICHT mechanisch portieren — die zugrundeliegende Spiel-API existiert nicht mehr.
// Das Original liegt als FIRSourceStatic_Vehicle.cpp.disabled daneben (Anforderungs-Referenz).
//
// Neuimplementierung gegen die neue 1.2-API = geplantes Feature
// "Fahrzeug-Routen/Stations (neu in 1.2)" — siehe PORT-1.2-FEATURE-VEHICLE-API.md.
//
// TODO FIN-1.2-PORT(vehicle): Reflection neu aufbauen. Kandidaten, die evtl. ohne das
// Pfad-System weiter funktionieren und früh re-aktiviert werden könnten:
//   - AFGBuildableDockingStation (Fuel/Inv/Docked/Undock/LoadMode) — unabhängig vom Pfad-System
//   - AFGVehicle.health/maxHealth/isSelfDriving — Basis-Props
// Jeweils gegen die 1.2-Header verifizieren, dann einzeln re-registrieren.

#include "Reflection/Source/FIRSourceStaticMacros.h"

// (Bewusst leer — registriert vorerst keine Vehicle-Typen.)
