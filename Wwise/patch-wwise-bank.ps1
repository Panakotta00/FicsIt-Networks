# Setzt die Projekt-ID im Header (BKHD) einer erzeugten Wwise-Soundbank auf die des Satisfactory-Wwise-Projekts.
#
# Hintergrund: Wwise lädt eine Soundbank nur, wenn ihre Projekt-ID zur geladenen Init-Bank passt, sonst
# meldet es "Failed to load SoundBank: 92 (The Init bank was not loaded yet ...)". Mod-Banken werden aus dem
# SML-Wwise-Projekt erzeugt (Projekt-ID 0), das Spiel lädt aber seine eigene Init-Bank (Projekt-ID 3134,
# ausgelesen aus FactoryGame/Content/WwiseAudio/Init.bnk in FactoryGame-Windows.pak, Satisfactory CL 502094).
#
# Nach jedem "WwiseConsole generate-soundbank" und vor dem Paketieren ausführen.
param(
	[string]$Bank = "$PSScriptRoot\..\..\..\..\SatisfactoryModLoader_WwiseProject\GeneratedSoundBanks\Windows\FicsItNetworks_Soundbank.bnk",
	[uint32]$ProjectId = 3134
)
$bytes = [IO.File]::ReadAllBytes($Bank)
if ([Text.Encoding]::ASCII.GetString($bytes, 0, 4) -ne 'BKHD') { throw "$Bank beginnt nicht mit BKHD" }
# BKHD: Tag(4) Größe(4) | Version(4) BankID(4) LanguageID(4) AltValues(4) ProjectID(4) ...
$offset = 24
$old = [BitConverter]::ToUInt32($bytes, $offset)
[BitConverter]::GetBytes($ProjectId).CopyTo($bytes, $offset)
[IO.File]::WriteAllBytes($Bank, $bytes)
"$(Split-Path $Bank -Leaf): Projekt-ID $old -> $ProjectId"
