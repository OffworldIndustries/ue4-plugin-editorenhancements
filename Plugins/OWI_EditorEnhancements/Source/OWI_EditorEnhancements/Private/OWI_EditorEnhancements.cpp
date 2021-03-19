// Copyright Offworld Industries

#include "OWI_EditorEnhancements.h"
#include "ContentBrowserModule.h"
#include "OWI_ContentBrowserMenuExtension.h"
#include "OWI_UnReleasedAssetsHandler.h"

#define LOCTEXT_NAMESPACE "OWI_EditorEnhancementsModule"

void FOWIEditorEnhancementsModule::StartupModule()
{
	FContentBrowserModule& ContentBrowserModule = FModuleManager::LoadModuleChecked<FContentBrowserModule>(TEXT("ContentBrowser"));
	ContentBrowserModule.GetAllPathViewContextMenuExtenders()
		.Add(FContentBrowserMenuExtender_SelectedPaths::CreateRaw(this, &FOWIEditorEnhancementsModule::ContentBrowserExtender));

	// Unfortunately the ColorPath changes can't be synchronised so make sure we mark all folder accordingly
	UOWIUnReleasedAssetsHandler* UnReleasedAssetsHandler = NewObject<UOWIUnReleasedAssetsHandler>();
	UnReleasedAssetsHandler->UpdateFolderColorsOnStart();

	// Custom Shaders folder for USH/USF shaders
	FString ShaderDirectory = FPaths::Combine(FPaths::ProjectDir(), TEXT("Shaders"));
	if (FPaths::DirectoryExists(ShaderDirectory) && FPaths::ValidatePath(ShaderDirectory))
	{
		AddShaderSourceDirectoryMapping("/Shaders", ShaderDirectory);
	}
}

void FOWIEditorEnhancementsModule::ShutdownModule()
{
	ResetAllShaderSourceDirectoryMappings();
}
TSharedRef<FExtender> FOWIEditorEnhancementsModule::ContentBrowserExtender(const TArray<FString>& Path)
{
	Extension = MakeShareable(new FOWIContentBrowserMenuExtension());
	TSharedPtr<FExtender> MenuExtender = MakeShareable(new FExtender());
	// For a more generic approach use "FolderContext" as ExtensionHook but this will sort it further up in the list
	MenuExtender->AddMenuExtension("PathContextSourceControl", EExtensionHook::After, TSharedPtr<FUICommandList>(),
                       FMenuExtensionDelegate::CreateSP(Extension.ToSharedRef(), &FOWIContentBrowserMenuExtension::AddMenuEntry, Path));
	return MenuExtender.ToSharedRef();
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FOWIEditorEnhancementsModule, OWI_EditorEnhancements)
