// Copyright Offworld Industries

#pragma once

#include "CoreMinimal.h"
#include "Framework/MultiBox/MultiBoxExtender.h"
#include "Modules/ModuleManager.h"

class FOWIContentBrowserMenuExtension;

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
