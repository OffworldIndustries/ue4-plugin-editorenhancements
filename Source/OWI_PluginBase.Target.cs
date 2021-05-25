// Fill out your copyright notice in the Description page of Project Settings.

using UnrealBuildTool;

public class OWI_PluginBaseTarget : TargetRules
{
	public OWI_PluginBaseTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Game;

		ExtraModuleNames.AddRange( new[] { "OWI_PluginBase" } );
	}
}
