// Copyright Offworld Industries

#pragma once

#include "CoreMinimal.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"

class OWI_EDITORENHANCEMENTS_API FOWIContentBrowserMenuExtension
{
	
public:
	
	void AddMenuEntry(FMenuBuilder& MenuBuilder, TArray<FString> SelectedPaths);
	void FillSubmenu(FMenuBuilder& MenuBuilder, TArray<FString> SelectedPaths);
	void OnReleasedClicked(TArray<FString> SelectedPaths) const;
	void OnUnreleasedClicked(TArray<FString> SelectedPaths) const;

};
