#pragma once

// Copyright Offworld Industries Ltd. All Rights Reserved.

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"

#include "OWI_NamingConventionHandler.generated.h"

struct FDirectoryPath;
enum EBlueprintType;
class UMaterial;
class UMaterialInstance;
class USkeletalMesh;

/**
 * Struct that represents a Prefix and a Suffix
 */
USTRUCT()
struct OWI_EDITORENHANCEMENTS_API FOWIPrefixSuffix
{
    GENERATED_BODY()

public:
	
    UPROPERTY(Config, Noclear, EditAnywhere, Category = "OWI Naming Conventions")
    FString Prefix;  
      
    UPROPERTY(Config, Noclear, EditAnywhere, Category = "OWI Naming Conventions", Meta = (DisplayName = "Optional Suffix"))
    FString OptionalSuffix;
	
};

/**
* Class to configure the UOWINamingConventionHandler
*/
UCLASS(Config = Editor, defaultconfig, meta = (DisplayName = "OWI Naming Conventions"))
class OWI_EDITORENHANCEMENTS_API UOWINamingConventionHandlerSettings final : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	
    UPROPERTY(EditAnywhere, Config, Category = "OWI Naming Conventions")
    bool bEnabled = true;

	UPROPERTY(EditAnywhere, Config, NoClear, Category = "OWI Naming Conventions", Meta = (EditCondition = "bEnabled"))
	TMap<FString, FOWIPrefixSuffix> GenericPrefixMap;

	UPROPERTY(EditAnywhere, Config, NoClear, Category = "OWI Naming Conventions", Meta = (EditCondition = "bEnabled"))
	TMap<UObject*, FOWIPrefixSuffix> UObjectPrefixMap;

    UPROPERTY(EditAnywhere, Config, NoClear, Category = "OWI Naming Conventions", Meta = (EditCondition = "bEnabled"))
    TMap<UClass*, FOWIPrefixSuffix> UClassPrefixMap;

	UPROPERTY(EditAnywhere, Config, NoClear, Category = "OWI Naming Conventions", Meta = (EditCondition = "bEnabled"))
	TMap<TEnumAsByte<enum EBlueprintType>, FOWIPrefixSuffix> EBlueprintTypePrefixMap;

	UPROPERTY(EditAnywhere, Config, NoClear, Category = "OWI Naming Conventions", Meta = (EditCondition = "bEnabled"))
	FOWIPrefixSuffix Material_WithPostProcessing;

	UPROPERTY(EditAnywhere, Config, NoClear, Category = "OWI Naming Conventions", Meta = (EditCondition = "bEnabled"))
	FOWIPrefixSuffix Material_WithoutPostProcessing;

	UPROPERTY(EditAnywhere, Config, NoClear, Category = "OWI Naming Conventions", Meta = (EditCondition = "bEnabled"))
	FOWIPrefixSuffix MaterialInstance_WithPostProcessing;

	UPROPERTY(EditAnywhere, Config, NoClear, Category = "OWI Naming Conventions", Meta = (EditCondition = "bEnabled"))
	FOWIPrefixSuffix MaterialInstance_WithoutPostProcessing;

	UPROPERTY(EditAnywhere, Config, NoClear, Category = "OWI Naming Conventions", Meta = (EditCondition = "bEnabled"))
	FOWIPrefixSuffix SkeletalMesh_WithMorphTargets;

	UPROPERTY(EditAnywhere, Config, NoClear, Category = "OWI Naming Conventions", Meta = (EditCondition = "bEnabled"))
	FOWIPrefixSuffix SkeletalMesh_WithoutMorphTargets;

	UPROPERTY(EditAnywhere, Config, NoClear, Category = "OWI Naming Conventions", Meta = (EditCondition = "bEnabled", ContentDir))
	TArray<FDirectoryPath> FolderWhitelist;

	UPROPERTY(EditAnywhere, Config, NoClear, Category = "OWI Naming Conventions", Meta = (EditCondition = "bEnabled", ContentDir))
	TArray<FDirectoryPath> FolderBlacklist;
	
};

/**
 * Handler for the Naming Conventions that checks names of files if they match the specs
 */
UCLASS()
class OWI_EDITORENHANCEMENTS_API UOWINamingConventionHandler final : public UObject
{
    GENERATED_BODY()

public:
    
    /**
     * \brief Can any naming convention be applied to this Asset?
     * \param Asset The Asset to check
     * \return True if a naming convention can be applied
     */
    bool NamingConventionCanBeApplied(UObject* Asset);

    /**
     * \brief Is any naming convention violated?
     * \param Asset The Asset to check
	 * \param ValidationError The Error Text if any
     * \return True if a naming convention is violated
     */
    bool NamingConventionIsViolated(UObject* Asset, FString& ValidationError);

    /**
     * @brief Rename the Asset to match Naming Conventions
     * @param Asset The Asset to rename
     */
	void RenameAsset(UObject* Asset);

private:

	// Pointer holding the current settings
	UPROPERTY()
	UOWINamingConventionHandlerSettings* Settings = nullptr;
	
	/**
	 * @brief Get the closest PrefixSuffix by using the Parent Object
	 * @param Object The object to search for
	 * \return The closest PrefixSuffix or nullptr if none found
	 */
	FOWIPrefixSuffix* GetUObjectPrefixSuffix(UObject* Object);

    /**
     * @brief Get the closets PrefixSuffix by using the Parent Class
     * @param ObjectClass The class of the object to search for
     * \return The closest PrefixSuffix or nullptr if none found
     */
	FOWIPrefixSuffix* GetUClassPrefixSuffix(const UClass* ObjectClass);

	/**
	 * @brief Get the closest PrefixSuffix by using the Class Name
	 * @param ClassName The FString to search for
     * \return The closest PrefixSuffix or nullptr if none found
	 */
	FOWIPrefixSuffix* GetGenericPrefixSuffix(const FString& ClassName);

	/**
	 * @brief Get the PrefixSuffix if it is a Blueprint
	 * @param Blueprint The Blueprint to check
	 * \return The closest PrefixSuffix or nullptr if none found
	 */
	FOWIPrefixSuffix* CheckBlueprintTypes(const UBlueprint* Blueprint);

	/**
	 * @brief Get the PrefixSuffix if it is a Material
	 * @param Material The Material to search check
	 * \return The closest PrefixSuffix or nullptr if none found
	 */
	FOWIPrefixSuffix* CheckMaterialTypes(const UMaterial* Material);

	/**
	 * @brief Get the PrefixSuffix if it is a Material Interface
	 * @param MaterialInstance The Material to search check
	 * \return The closest PrefixSuffix or nullptr if none found
	 */
	FOWIPrefixSuffix* CheckMaterialInstanceTypes(UMaterialInstance* MaterialInstance);

	/**
	 * @brief Get the PrefixSuffix if it is a Skeletal Mesh
	 * @param SkeletalMeshInstance The Skeletal Mesh to search check
	 * \return The closest PrefixSuffix or nullptr if none found
	 */
	FOWIPrefixSuffix* CheckSkeletalMeshTypes(USkeletalMesh* SkeletalMeshInstance);
};
