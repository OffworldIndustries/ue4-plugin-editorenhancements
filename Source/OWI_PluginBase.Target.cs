// Fill out your copyright notice in the Description page of Project Settings.

using UnrealBuildTool;
using System.Collections.Generic;

public class OWI_PluginBaseTarget : TargetRules
{
	public OWI_PluginBaseTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Game;

		ExtraModuleNames.AddRange( new string[] { "OWI_PluginBase" } );
	}
}
