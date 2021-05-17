// Copyright Offworld Industries Ltd. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/EngineTypes.h"
#include "Engine/DeveloperSettings.h"
#include "OWI_ReferenceRulesHandler.generated.h"

struct FAssetData;
class FAssetRegistryModule;

/** Configurable properties for reference rule. */
USTRUCT()
struct OWI_EDITORENHANCEMENTS_API FOWIReferenceRule
{
	GENERATED_BODY()

public:
	
	UPROPERTY(Config, NoClear, EditAnywhere, Category = "OWI Reference Rules", Meta = (ContentDir))
    FDirectoryPath AssetFolder;
    
	UPROPERTY(Config, NoClear, EditAnywhere, Category = "OWI Reference Rules", Meta = (ContentDir))
	FDirectoryPath MayNotReferenceFolder;
    
	UPROPERTY(Config, NoClear, EditAnywhere, Category = "OWI Reference Rules", Meta = (ContentDir, TitleProperty = "Path"))
	TArray<FDirectoryPath> ExceptionFolders;
	
	UPROPERTY(Config, NoClear, EditAnywhere, Category = "OWI Reference Rules", Meta = (ContentDir))
	FFilePath AssetFile;
    
	UPROPERTY(Config, NoClear, EditAnywhere, Category = "OWI Reference Rules", Meta = (ContentDir))
	FFilePath MayNotReferenceFile;
    
	UPROPERTY(Config, NoClear, EditAnywhere, Category = "OWI Reference Rules", Meta = (ContentDir, TitleProperty = "Path"))
	TArray<FFilePath> ExceptionFiles;
		
	/**
	* \brief Is this rule violated for the given Asset?
	* \param AssetPath The Asset path to check
	* \param References The references to check
	* \param ValidationError The Error Text if any
	* \return True if the rule is violated, false otherwise
	*/
	bool IsViolated(const FString& AssetPath, const TArray<FName>& References, FString& ValidationError) const;

	/**
	 * \brief Can this rule be applied?
	 * \param AssetPath The AssetPath to check for
	 * \return True if this rule can be applied, false otherwise
	 */
	bool CanBeApplied(const FString& AssetPath) const;
	
	/**
	* \brief Convert a FilePath to a usable String (removes relative path and extension)
	* \param FilePath The FilePath to convert
	* \return The String with absolute /Game/ path and w/out extension
	*/
	static FString ConvertFilePath(const FFilePath& FilePath);
	
private:

	/**
	 * \brief Is there an exception for the given Reference?
	 * \param Reference The reference to check against the exceptions
	 * \return True if the given reference has an exception
	 */
	bool IsException(const FName& Reference) const;
};

/**
* Class to configure the UTRTextureFactory
*/
UCLASS(Config = Editor, defaultconfig, meta = (DisplayName = "OWI Reference Rules"))
class OWI_EDITORENHANCEMENTS_API UOWIReferenceRulesHandlerSettings final : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	
	UPROPERTY(EditAnywhere, Config, Category = "OWI Reference Rules")
	bool bEnabled = true;

	UPROPERTY(EditAnywhere, Config, Category = "OWI Reference Rules", Meta = (EditCondition = "bEnabled"))
	bool bCheckCircularDependencies = false;

	UPROPERTY(EditAnywhere, Config, NoClear, Category = "OWI Reference Rules", Meta = (EditCondition = "bEnabled", TitleProperty = "AssetFolder"))
	TArray<FOWIReferenceRule> ReferenceRules;

	UPROPERTY(EditAnywhere, Config, NoClear, Category = "OWI Reference Rules", Meta = (EditCondition = "bEnabled", ContentDir))
	TArray<FDirectoryPath> BlackListFolders;

	UPROPERTY(EditAnywhere, Config, NoClear, Category = "OWI Reference Rules", Meta = (EditCondition = "bEnabled", ContentDir))
	TArray<FFilePath> BlackListFiles;
};

UCLASS(Config = Editor, defaultconfig, meta = (DisplayName = "OWI Reference Rules"))
class OWI_EDITORENHANCEMENTS_API UOWIReferenceRulesHandler final : public UObject
{
	GENERATED_BODY()

public:

	/**
	 * \brief Is any rule violated?
	 * \param AssetPath The Asset path to check
	 * \param ValidationError The Error Text if any
	 * \return True if at least one rule was violated
	 */
	bool RuleIsViolated(const FString& AssetPath, FString& ValidationError);
	
	/**
	* \brief Can a rule be applied?
	* \param AssetPath The Asset path to check
	* \return True if at least one rule can be applied
	*/
	bool RuleCanBeApplied(const FString& AssetPath);

private:

	// All available Settings
	UPROPERTY()
	UOWIReferenceRulesHandlerSettings* Settings = nullptr;
	
	/**
	 * \brief Checks all rules
	 * \param AssetPath The path of the Asset to check
	 * \param AssetRegistry The Asset Registry to use
	 * \param AssetData The Asset data to use
	 * \param bHardDependencyType Should be checked for hard references?
	 * \param ValidationError Validation error text if any
	 * \return True if there are violations
	 */
	bool CheckAllRules(const FString& AssetPath, const FAssetRegistryModule& AssetRegistry, const FAssetData& AssetData, const bool bHardDependencyType, FString& ValidationError);

	/**
	 * \brief Is any of the references blacklisted?
	 * \param AssetPath The path of the Asset to check
	 * \param References The references to check
	 * \param ValidationError THe validation error to write to
	 * \return True if any of the given references is blacklisted
	 */
	bool IsBlackListed(const FString& AssetPath, const TArray<FName>& References, FString& ValidationError);

	/**
	 * \brief Is any of the references a circular reference?
	 * \param AssetPath The path of the first Asset to check
	 * \param AssetRegistry The Asset Registry to use
	 * \param AssetData The Asset data to use
	 * \param ValidationError Validation error text if any
	 * \return True if any of the given references is circular
	 */
	static bool HasCircularDependency(const FString& AssetPath, const FAssetRegistryModule& AssetRegistry, const FAssetData& AssetData, FString& ValidationError);
};
