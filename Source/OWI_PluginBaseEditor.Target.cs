// Fill out your copyright notice in the Description page of Project Settings.

using UnrealBuildTool;

public class OWI_PluginBaseEditorTarget : TargetRules
{
	public OWI_PluginBaseEditorTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Editor;

		ExtraModuleNames.AddRange( new [] { "OWI_PluginBase" } );
	}
}
