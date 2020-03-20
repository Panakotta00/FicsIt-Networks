// Fill out your copyright notice in the Description page of Project Settings.

using UnrealBuildTool;
using System.Collections.Generic;

public class FicsItNetworksEditorTarget : TargetRules
{
	public FicsItNetworksEditorTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Editor;
        ExtraModuleNames.AddRange( new string[] { "FactoryGame", "SML", "FicsItNetworks" } );
	}
}
