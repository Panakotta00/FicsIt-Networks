# FicsIt-Networks Wwise objects

Work units of the FicsIt-Networks objects in the SML Wwise project (Wwise 2025.1.6, as used by Satisfactory 1.2).

Setup:
1. Copy each `*/FicsItNetworks.wwu` and the `Originals` folder into the same folder of `SatisfactoryModLoader_WwiseProject`.
2. Generate the SoundBank: `WwiseConsole.exe generate-soundbank <SatisfactoryModLoader_WwiseProject.wproj> --platform Windows --bank FicsItNetworks_Soundbank`
3. Run `patch-wwise-bank.ps1`. It sets the project ID of the bank to the one of the game's Init bank (3134), otherwise Wwise refuses to load it (error 92, AK_InitBankNotLoaded).

Mods can't add busses (they live in the game's Init bank), so FIN sounds output to `Master Audio Bus`.
The module and computer sounds are the former SoundCues/SoundWaves (`Originals/SFX/FicsItNetworks`), their volume, attenuation and random pitch/volume variation are taken over from the cues.
The computer beep pitch is set through the RTPC `FIN_Beep_Pitch` (cents).
The speaker uses the "Wwise Audio Input" source plugin, which FIN registers at startup (`AkAudioInput.dll` is not in the game's Init bank plugin list).
