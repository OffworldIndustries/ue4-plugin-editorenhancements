// Copyright Offworld Industries

#pragma once

#include "CoreMinimal.h"
#include "Paths.h"
#include "Engine/DeveloperSettings.h"
#include "Engine/EngineTypes.h"
#include "UObject/NoExportTypes.h"
#include "OWI_UnReleasedAssetsHandler.generated.h"

class UOWIReferenceRulesHandlerSettings;
class UProjectPackagingSettings;

DECLARE_LOG_CATEGORY_EXTERN(LogUnReleasedAssetHandler, Log, All);

enum EPathState
{
	None,
    Released,
    Unreleased,
    Mixed
};

/**
* Class to configure the UOWIUnReleasedAssetsHandler
*/
UCLASS(Config = Editor, defaultconfig, meta = (DisplayName = "OWI UnReleased Assets"))
class OWI_EDITORENHANCEMENTS_API UOWIUnReleasedAssetsHandlerSettings final : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	
	UPROPERTY(EditAnywhere, Config, NoClear, Category = "OWI UnReleased Assets")
	FLinearColor UnreleasedFolderColor = FLinearColor::Red;
	
	UPROPERTY(EditAnywhere, Config, NoClear, Category = "OWI UnReleased Assets")
	FLinearColor MixedFolderColor = FLinearColor::Yellow;

	UPROPERTY(EditAnywhere, Config, NoClear, Category = "OWI UnReleased Assets", Meta = (ContentDir))
	FDirectoryPath AssetFolderToNotBeReferenced = {"/Game" };

	UPROPERTY(EditAnywhere, Config, NoClear, Category = "OWI UnReleased Assets", Meta = (FilePath, FilePathFilter="sh"))
	FFilePath ModSDKFile = { FPaths::ProjectDir().Append("Tools/ModSDK/create_file_list.sh") };

	UPROPERTY(EditAnywhere, Config, NoClear, Category = "OWI UnReleased Assets")
	FString ModSDKBlacklistStringStart = { "declare -a blacklist=(" };

	UPROPERTY(VisibleAnywhere, Config, NoClear, Category = "OWI UnReleased Assets", Meta = (DisplayName="Unreleased Folders (read-only)"))
	TArray<FString> UnreleasedFolders;

	UPROPERTY(Config)
	TArray<FString> ReleasedFolders;

	UPROPERTY(Config)
	TArray<FString> MixedFolders;
};

UCLASS()
class OWI_EDITORENHANCEMENTS_API UOWIUnReleasedAssetsHandler final : public UObject
{
	GENERATED_BODY()

public:

	void ReleaseFolder(FString FolderPath);

	void UnreleaseFolder(FString FolderPath);

	void UpdateFolderColorsOnStart();

	EPathState GetFolderState(TArray<FString> Folders);
	
private:

	UPROPERTY()
	UOWIUnReleasedAssetsHandlerSettings* Settings = nullptr;

	UPROPERTY()
	UProjectPackagingSettings* PackagingSettings = nullptr;

	UPROPERTY()
	UOWIReferenceRulesHandlerSettings* ReferenceRulesSettings = nullptr;

	bool LoadSettings();

	void SaveSettings() const;
	
	void AddFolderToModSDKBlacklist(FString FolderPath) const;

	void RemoveFolderFromModSDKBlacklist(FString FolderPath) const;

	bool LoadModSDKFile(FString& Content) const;

	void SaveModSDKFile(FString Content) const;

	static FString GetModSDKCompatibleFolderName(FString FolderPath);

	void UpdateInternalConfig(FString FolderPath, EPathState NewPathState) const;

	void UpdateColor(FString FolderPath, EPathState PathState) const;

	void HandleSubfolderRelease(FString CurrentFolderPath, FString ReleaseFolderPath);

	void UpdateParentFolder(FString FolderPath, const bool bRelease);

	static TArray<FString> GetSubfolders(FString FolderPath);
};
