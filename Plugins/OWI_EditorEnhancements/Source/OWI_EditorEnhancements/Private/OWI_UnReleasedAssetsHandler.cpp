// Copyright Offworld Industries

#include "OWI_UnReleasedAssetsHandler.h"
#include "ConfigCacheIni.h"
#include "ContentBrowserModule.h"
#include "FileHelper.h"
#include "OWI_ReferenceRulesHandler.h"
#include "Editor/ContentBrowser/Private/ContentBrowserUtils.h"
#include "Engine/EngineTypes.h"
#include "Settings/ProjectPackagingSettings.h"
#include "EditorAssetLibrary.h"
#include "TokenizedMessage.h"

DEFINE_LOG_CATEGORY(LogUnReleasedAssetHandler);

bool UOWIUnReleasedAssetsHandler::LoadSettings()
{
	Settings = GetMutableDefault<UOWIUnReleasedAssetsHandlerSettings>();
	ReferenceRulesSettings = GetMutableDefault<UOWIReferenceRulesHandlerSettings>();
	PackagingSettings = GetMutableDefault<UProjectPackagingSettings>();

	if (Settings == nullptr || ReferenceRulesSettings == nullptr || PackagingSettings == nullptr)
	{
		UE_LOG(LogUnReleasedAssetHandler, Error, TEXT("Could not read all settings!"));
		return false;
	}

	// Sort the Folders
	Settings->UnreleasedFolders.HeapSort();
	Settings->MixedFolders.HeapSort();
	Settings->ReleasedFolders.HeapSort();
	
	return true;
}

void UOWIUnReleasedAssetsHandler::SaveSettings() const
{
	Settings->UpdateDefaultConfigFile(Settings->GetDefaultConfigFilename());
	ReferenceRulesSettings->UpdateDefaultConfigFile(ReferenceRulesSettings->GetDefaultConfigFilename());
	PackagingSettings->UpdateDefaultConfigFile(PackagingSettings->GetDefaultConfigFilename());
}

void UOWIUnReleasedAssetsHandler::ReleaseFolder(const FString FolderPath)
{
	if (!LoadSettings())
	{
		return;
	}
		
	// Remove Folder from reference rules
	ReferenceRulesSettings->ReferenceRules.RemoveAll([&](const auto& Rule)
    {
        return Rule.AssetFolder.Path.Equals(Settings->AssetFolderToNotBeReferenced.Path)
            && Rule.MayNotReferenceFolder.Path.Equals(FolderPath);
    });

	// Remove Folder from never cooking
	PackagingSettings->DirectoriesToNeverCook.RemoveAll([&FolderPath](const auto& TempDirectoryPath)
	{
		return TempDirectoryPath.Path.Equals(FolderPath);
	});
	
	// Remove Folder from Mod SDK Blacklist (Does this really need to be done? Not mentioned on Confluence page)
	RemoveFolderFromModSDKBlacklist(GetModSDKCompatibleFolderName(FolderPath));	

	// Update the internal config
	UpdateInternalConfig(FolderPath, Released);
	
	// Also update the parent folder
	UpdateParentFolder(FolderPath, true);
	
	// Save the Settings
	SaveSettings();
}

void UOWIUnReleasedAssetsHandler::UnreleaseFolder(const FString FolderPath)
{
	if (!LoadSettings())
	{
		return;
	}
	
	const FDirectoryPath DirectoryPath = { FolderPath };
	
	// Add Folder to reference rules if not yet in there
	const FOWIReferenceRule TempRule { Settings->AssetFolderToNotBeReferenced, DirectoryPath };
	if (!ReferenceRulesSettings->ReferenceRules.ContainsByPredicate([&TempRule](const FOWIReferenceRule& TempInnerRule)
        {
            return TempRule.AssetFolder.Path.Equals(TempInnerRule.AssetFolder.Path)
                && TempRule.MayNotReferenceFolder.Path.Equals(TempInnerRule.MayNotReferenceFolder.Path);
        }))
	{
		ReferenceRulesSettings->ReferenceRules.Add(TempRule);
	}
	
	// Add Folder to never cook if not yet in there
	if (!PackagingSettings->DirectoriesToNeverCook.ContainsByPredicate([&DirectoryPath](const FDirectoryPath TempDirectoryPath)
        {
            return DirectoryPath.Path.Equals(TempDirectoryPath.Path);
        }))
	{
		PackagingSettings->DirectoriesToNeverCook.Add(DirectoryPath);
	}
	
	// Add Folder to Mod SDK Blacklist
	AddFolderToModSDKBlacklist(GetModSDKCompatibleFolderName(FolderPath));

	// Update the internal config
	UpdateInternalConfig(FolderPath, Unreleased);

	// Update the internal config
	UpdateParentFolder(FolderPath, false);
	
	// Save the Settings
	SaveSettings();
}

void UOWIUnReleasedAssetsHandler::UpdateFolderColorsOnStart()
{
	if (!LoadSettings())
	{
		return;
	}

	for (auto& Path : Settings->UnreleasedFolders)
	{
		UpdateColor(Path, Unreleased);
	}

	for (auto& Path : Settings->ReleasedFolders)
	{
		UpdateColor(Path, Released);
	}

	for (auto& Path : Settings->MixedFolders)
	{
		UpdateColor(Path, Mixed);
	}
}

EPathState UOWIUnReleasedAssetsHandler::GetFolderState(TArray<FString> Folders)
{
	EPathState PathState = None;
	if (!LoadSettings())
	{
		return PathState;
	}
	
	for (auto& Path : Folders)
	{
		if (Settings->UnreleasedFolders.ContainsByPredicate([&Path](const auto& Folder)
	        {
	            return Path.StartsWith(Folder);
	        }))
		{
			if (PathState == None)
			{
				PathState = Unreleased;
			} else if (PathState == Released)
			{
				PathState = Mixed;
				break;
			}  
		} else
		{
			if (PathState == None)
			{
				PathState = Released;
			} else if (PathState == Unreleased)
			{
				PathState = Mixed;
				break;
			}
		}
	}
	
	return PathState;
}

void UOWIUnReleasedAssetsHandler::AddFolderToModSDKBlacklist(const FString FolderPath) const
{
	FString Content;
	if (LoadModSDKFile(Content))
	{
		const int32 BlacklistStart = Content.Find(Settings->ModSDKBlacklistStringStart) + Settings->ModSDKBlacklistStringStart.Len();
		Content.InsertAt(BlacklistStart, FolderPath);
		SaveModSDKFile(Content);
	}
}

void UOWIUnReleasedAssetsHandler::RemoveFolderFromModSDKBlacklist(const FString FolderPath) const
{
	FString Content;
	if (LoadModSDKFile(Content))
	{
		const int32 FolderPathStart = Content.Find(FolderPath);
		Content.RemoveAt(FolderPathStart, FolderPath.Len());
		SaveModSDKFile(Content);
	}
}

bool UOWIUnReleasedAssetsHandler::LoadModSDKFile(FString& Content) const
{
	if (!FPaths::FileExists(Settings->ModSDKFile.FilePath))
	{
		UE_LOG(LogUnReleasedAssetHandler, Error, TEXT("Could not find ModSDK file: %s!"), *Settings->ModSDKFile.FilePath);
		return false;
	}
	
	FFileHelper::LoadFileToString(Content, *Settings->ModSDKFile.FilePath);
	return true;
}

void UOWIUnReleasedAssetsHandler::SaveModSDKFile(const FString Content) const
{
	if (!FFileHelper::SaveStringToFile(Content, *Settings->ModSDKFile.FilePath))
	{
		UE_LOG(LogUnReleasedAssetHandler, Error, TEXT("Could not save ModSDK file: %s!"), *Settings->ModSDKFile.FilePath);
	}
}

FString UOWIUnReleasedAssetsHandler::GetModSDKCompatibleFolderName(FString FolderPath)
{
	FolderPath.RemoveFromStart("/Game");
	return "\n\t\"" + FolderPath + "\"";
}

void UOWIUnReleasedAssetsHandler::UpdateInternalConfig(const FString FolderPath, const EPathState NewPathState) const
{
	if (NewPathState == Released)
	{
		Settings->ReleasedFolders.AddUnique(FolderPath);
		Settings->UnreleasedFolders.Remove(FolderPath);
		Settings->MixedFolders.Remove(FolderPath);
	} else  if (NewPathState == Unreleased)
	{
		Settings->ReleasedFolders.Remove(FolderPath);
		Settings->UnreleasedFolders.AddUnique(FolderPath);
		Settings->MixedFolders.Remove(FolderPath);
	} else  if (NewPathState == Mixed)
	{
		Settings->ReleasedFolders.Remove(FolderPath);
		Settings->UnreleasedFolders.Remove(FolderPath);
		Settings->MixedFolders.AddUnique(FolderPath);
	}
	
	// Update the Folder Color
	UpdateColor(FolderPath, NewPathState);
}

void UOWIUnReleasedAssetsHandler::UpdateColor(const FString FolderPath, const EPathState PathState) const
{
	// Keeping this here in case Epic Games ever decides to make ContentBrowserUtils available
	// If you really need this add CONTENTBROWSER_API in front of the function in the header file
	// This will make it so the Color is immediately updated without having to restart the Editor
	// You may then remove the lines below it as ContentBrowserUtils does basically the same thing
	TSharedPtr<FLinearColor> Color;
	switch (PathState)
	{
		case Unreleased:
			{
				Color = MakeShareable(new FLinearColor(Settings->UnreleasedFolderColor));
				break;
			}
		case Mixed: 
			{
				Color = MakeShareable(new FLinearColor(Settings->MixedFolderColor));
				break;
			}
		case Released:
		case None:
		default:
			{
				Color = nullptr;
				break;
			}
	}
	ContentBrowserUtils::SaveColor(FolderPath, Color, true);
	
	/*
	if(FPaths::FileExists(GEditorPerProjectIni))
	{
		switch (PathState)
		{
		default:
		case None:
		case Released: 
			GConfig->RemoveKey(TEXT("PathColor"), *FolderPath, GEditorPerProjectIni);
			break;
		case Unreleased:
			GConfig->SetString(TEXT("PathColor"), *FolderPath, *Settings->UnreleasedFolderColor.ToString(), GEditorPerProjectIni);
			break;
		case Mixed:
			GConfig->SetString(TEXT("PathColor"), *FolderPath, *Settings->MixedFolderColor.ToString(), GEditorPerProjectIni);
			break;
		}
	}*/
}

void UOWIUnReleasedAssetsHandler::HandleSubfolderRelease(const FString CurrentFolderPath, const FString ReleaseFolderPath)
{	
	ReleaseFolder(CurrentFolderPath);
	UpdateInternalConfig(CurrentFolderPath, Mixed);
	// Check out all the folders within this folder
	TArray<FString> Subfolders = GetSubfolders(CurrentFolderPath);
	
	for (auto& Subfolder : Subfolders)
	{
		if (ReleaseFolderPath.Equals(Subfolder))
		{
			continue;
		}
		
		if (ReleaseFolderPath.StartsWith(Subfolder))
		{
			HandleSubfolderRelease(Subfolder, ReleaseFolderPath);
		} else
		{
			UnreleaseFolder(Subfolder);
		}
	}
}

void UOWIUnReleasedAssetsHandler::UpdateParentFolder(const FString FolderPath, const bool bRelease)
{
	FString ParentFolder;
	const bool bHasUnreleasedParent = Settings->UnreleasedFolders.ContainsByPredicate([&FolderPath, &ParentFolder](const auto& TempFolderPath)
	{
		if (FolderPath.StartsWith(TempFolderPath) && !FolderPath.Equals(TempFolderPath))
		{
			ParentFolder = TempFolderPath;
			return true;
		}
		
		return false;
	});
	
	if (bRelease && bHasUnreleasedParent)
	{
		// A parent of this folder was unreleased
		HandleSubfolderRelease(ParentFolder, FolderPath);
	}
	
	const bool bHasMixedParent = Settings->MixedFolders.ContainsByPredicate([&FolderPath, &ParentFolder](const auto& TempFolderPath)
    {
        if (FolderPath.StartsWith(TempFolderPath) && !FolderPath.Equals(TempFolderPath))
        {
            ParentFolder = TempFolderPath;
            return true;
        }
		
        return false;
    });

	if (bHasMixedParent)
	{
		// Update the color of the parent folder
		const TArray<FString> ChildFolders = GetSubfolders(ParentFolder);
		const EPathState ChildState = GetFolderState(ChildFolders);
		UpdateInternalConfig(ParentFolder, ChildState);
	}
}

TArray<FString> UOWIUnReleasedAssetsHandler::GetSubfolders(const FString FolderPath)
{
	TArray<FString> Subfolders = UEditorAssetLibrary::ListAssets(FolderPath, false, true).FilterByPredicate([](const auto& FoundAssetName)
        {
            return !FoundAssetName.Contains(".");
        });
	for (auto& Subfolder : Subfolders)
	{
		Subfolder.RemoveFromEnd("/");
	}
	
	return Subfolders;
}
