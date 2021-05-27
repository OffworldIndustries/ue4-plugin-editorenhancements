// Copyright Offworld Industries Ltd. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Framework/MultiBox/MultiBoxExtender.h"

class FOWIContentBrowserMenuExtension;

/**
 * The Module for the Editor Enhancements. Used to sync the UnReleased folder colors.
 */
class OWI_EDITORENHANCEMENTS_API FOWIEditorEnhancementsModule final : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

private:
	
	TSharedRef<FExtender> ContentBrowserExtender(const TArray<FString>&Path);
	
	TSharedPtr<FOWIContentBrowserMenuExtension> Extension;
};
