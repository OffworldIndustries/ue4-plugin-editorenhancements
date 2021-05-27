// Copyright Offworld Industries Ltd. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "Engine/EngineTypes.h"
#include "Misc/Paths.h"
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

/**
 * Class that handles UnReleased Assets. Allows to to Release/Unrelease Folders
 */
UCLASS()
class OWI_EDITORENHANCEMENTS_API UOWIUnReleasedAssetsHandler final : public UObject
{
	GENERATED_BODY()

public:

	/**
	 * @brief Release the folder with the given path
	 * @param FolderPath The Path of the folder to release
	 */
	void ReleaseFolder(FString FolderPath);

	/**
	* @brief Unrelease the folder with the given path
	* @param FolderPath The Path of the folder to unrelease
	*/
	void UnreleaseFolder(FString FolderPath);

	/**
	 * @brief Update all folder colors when the Editor starts
	 */
	void UpdateFolderColorsOnStart();

	/**
	 * @brief Get the state of a given set of folders
	 * @param Folders The folders to check
	 * @return The State of the folders
	 */
	EPathState GetFolderState(TArray<FString> Folders);
	
private:

	UPROPERTY()
	UOWIUnReleasedAssetsHandlerSettings* Settings = nullptr;

	UPROPERTY()
	UProjectPackagingSettings* PackagingSettings = nullptr;

	UPROPERTY()
	UOWIReferenceRulesHandlerSettings* ReferenceRulesSettings = nullptr;

	/**
	 * @brief Load the settings from the config file
	 * @return True if loading was successfull
	 */
	bool LoadSettings();

	/**
	 * @brief Save the settings to the config file
	 */
	void SaveSettings() const;
	
	/**
	 * @brief Add a given folder to the Mod SDK Blacklist
	 * @param FolderPath The Folder to add
	 */
	void AddFolderToModSDKBlacklist(FString FolderPath) const;

	/**
	* @brief Remove a given folder to the Mod SDK Blacklist
	* @param FolderPath The Folder to remove
	*/
	void RemoveFolderFromModSDKBlacklist(FString FolderPath) const;

	/**
	 * @brief Load the Mod SDK file
	 * @param Content Insert the content of the file here
	 * @return True if loading was successfull
	 */
	bool LoadModSDKFile(FString& Content) const;

	/**
	 * @brief Save the content to the ModSDK file
	 * @param Content The content to save
	 */
	void SaveModSDKFile(FString Content) const;

	/**
	 * @brief Convert a folder path to the format we need for the Mod SDK file
	 * @param FolderPath The path to convert
	 * @return The compatible FolderPath
	 */
	static FString GetModSDKCompatibleFolderName(FString FolderPath);

	/**
	 * @brief Update the local settings
	 * @param FolderPath The folder to update
	 * @param NewPathState The new state for the folder
	 */
	void UpdateInternalConfig(FString FolderPath, EPathState NewPathState) const;

	/**
	 * @brief Update the color of a given folder
	 * @param FolderPath The folder to update
	 * @param PathState The state of the folder
	 */
	void UpdateColor(FString FolderPath, EPathState PathState) const;

	/**
	 * @brief Handle what happens when a subfolder is being released
	 * @param CurrentFolderPath The parent folder
	 * @param ReleaseFolderPath The subfolder
	 */
	void HandleSubfolderRelease(FString CurrentFolderPath, FString ReleaseFolderPath);

	/**
	 * @brief Update the parent folder depending on the children's states
	 * @param FolderPath Path of the childfolder
	 * @param bRelease true if the folder should be released
	 */
	void UpdateParentFolder(FString FolderPath, const bool bRelease);

	/**
	 * @brief Get all the subfolders of a given folder
	 * @param FolderPath The folder to get the subfolders for
	 * @return List of all subfolders
	 */
	static TArray<FString> GetSubfolders(FString FolderPath);
};
