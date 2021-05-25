// Copyright Offworld Industries Ltd. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

class OWI_EDITORENHANCEMENTS_API FOWIContentBrowserMenuExtension
{
	
public:
	
	void AddMenuEntry(FMenuBuilder& MenuBuilder, TArray<FString> SelectedPaths);
	void FillSubmenu(FMenuBuilder& MenuBuilder, TArray<FString> SelectedPaths) const;
	void OnReleasedClicked(TArray<FString> SelectedPaths) const;
	void OnUnreleasedClicked(TArray<FString> SelectedPaths) const;

};
