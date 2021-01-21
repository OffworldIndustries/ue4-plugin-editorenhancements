// Copyright Offworld Industries

#include "OWI_NamingConventionHandler.h"
#include "Engine/Blueprint.h"
#include "Materials/Material.h"
#include "Materials/MaterialInstance.h"
#include "Engine/SkeletalMesh.h"
#include "Engine/EngineTypes.h"

bool UOWINamingConventionHandler::NamingConventionCanBeApplied(UObject* Asset)
{
    Settings = const_cast<UOWINamingConventionHandlerSettings*>(GetDefault<UOWINamingConventionHandlerSettings>());
	
    if (Asset == nullptr || Settings == nullptr || Settings->bEnabled == false)
    {
        return false;
    }

	bool IsWhitelisted = false;

	// Check if Asset is in a folder that should be checked
	for (const FDirectoryPath FolderName : Settings->FolderWhitelist)
	{
		if (Asset->GetPathName().StartsWith(FolderName.Path))
		{
			IsWhitelisted = true;
			break;
		}
	}

	if (IsWhitelisted == false)
	{
		return false;
	}

    // Check if Asset is in a folder that shouldn't be checked
    for (const FDirectoryPath FolderName : Settings->FolderBlacklist)
    {
        if (Asset->GetPathName().StartsWith(FolderName.Path))
        {
            return false;
        }
    }

    return GetUObjectPrefixSuffix(Asset) != nullptr;
}

bool UOWINamingConventionHandler::NamingConventionIsViolated(UObject* Asset, FString& ValidationError)
{
	if (NamingConventionCanBeApplied(Asset) == false)
	{
		return false;
	}
    
    bool bResult = false;
    
    FOWIPrefixSuffix* PrefixSuffix = GetUObjectPrefixSuffix(Asset);
	
    if (PrefixSuffix != nullptr)
    {
		const FString Name{ Asset->GetName() };
        if (PrefixSuffix->Prefix.IsEmpty() == false && Name.StartsWith(*PrefixSuffix->Prefix) == false)
        {
            ValidationError = Name + " doesn't start with " + *PrefixSuffix->Prefix;
            bResult = true;
        }
        
        if (PrefixSuffix->OptionalSuffix.IsEmpty() == false && Name.EndsWith(*PrefixSuffix->OptionalSuffix) == false)
        {
            ValidationError.Append(bResult ? FString(" and ") : Name).Append(" doesn't end with ").Append(*PrefixSuffix->OptionalSuffix);
            bResult = true;
        }
    }

    return bResult;
}

void UOWINamingConventionHandler::RenameAsset(UObject* Asset)
{
    if (Asset == nullptr)
    {
        return;
    }
	
    FOWIPrefixSuffix* PrefixSuffix = GetUObjectPrefixSuffix(Asset);
	
    if (PrefixSuffix != nullptr)
    {
        const FString Name{ Asset->GetName() };
        FString NewName = Name;
        if (Name.StartsWith(*PrefixSuffix->Prefix) == false)
        {
            NewName = *PrefixSuffix->Prefix + NewName;
        }
        
        if (PrefixSuffix->OptionalSuffix.IsEmpty() == false && Name.EndsWith(*PrefixSuffix->OptionalSuffix) == false)
        {
            NewName = NewName + *PrefixSuffix->OptionalSuffix;
        }

        if (NewName.Equals(Name) == false)
        {
        	Asset->Rename(*NewName, Asset->GetOuter(), REN_DontCreateRedirectors);
        }
    }
}

FOWIPrefixSuffix* UOWINamingConventionHandler::GetUObjectPrefixSuffix(UObject* Object)
{
	if (Settings == nullptr)
	{
		Settings = const_cast<UOWINamingConventionHandlerSettings*>(GetDefault<UOWINamingConventionHandlerSettings>());
	}
	
	if (Object == nullptr || Settings == nullptr)
	{
		return nullptr;
	}

	FOWIPrefixSuffix* PrefixSuffix = CheckMaterialInstanceTypes(Cast<UMaterialInstance>(Object));

	if (PrefixSuffix == nullptr)
	{
		PrefixSuffix = CheckMaterialTypes(Cast<UMaterial>(Object));
	}
	
	if (PrefixSuffix == nullptr)
	{
		PrefixSuffix = CheckSkeletalMeshTypes(Cast<USkeletalMesh>(Object));
	}

	if (PrefixSuffix == nullptr)
	{
		PrefixSuffix = CheckBlueprintTypes(Cast<UBlueprint>(Object));
	}
	
	if (PrefixSuffix == nullptr)
	{
		PrefixSuffix = Settings->UObjectPrefixMap.Find(Object);
	}
	
	if (PrefixSuffix == nullptr)
	{
		PrefixSuffix = GetUClassPrefixSuffix(Object->GetClass());
	}

	return PrefixSuffix;
}

FOWIPrefixSuffix* UOWINamingConventionHandler::GetUClassPrefixSuffix(const UClass* ObjectClass)
{
	if (Settings == nullptr)
	{
		Settings = const_cast<UOWINamingConventionHandlerSettings*>(GetDefault<UOWINamingConventionHandlerSettings>());
	}
	
	if (ObjectClass == nullptr || Settings == nullptr)
	{
		// We reached the end of the chain
		return nullptr;
	}

	FOWIPrefixSuffix* PrefixSuffix = Settings->UClassPrefixMap.Find(ObjectClass);

	if (PrefixSuffix == nullptr)
	{
		PrefixSuffix = GetGenericPrefixSuffix(ObjectClass->GetDisplayNameText().ToString());
	}

	return PrefixSuffix == nullptr ? GetUClassPrefixSuffix(ObjectClass->GetSuperClass()) : PrefixSuffix;
}

FOWIPrefixSuffix* UOWINamingConventionHandler::GetGenericPrefixSuffix(const FString& ClassName)
{
	if (Settings == nullptr)
	{
		Settings = const_cast<UOWINamingConventionHandlerSettings*>(GetDefault<UOWINamingConventionHandlerSettings>());
	}

	return Settings == nullptr ? nullptr : Settings->GenericPrefixMap.Find(ClassName);
}

FOWIPrefixSuffix* UOWINamingConventionHandler::CheckBlueprintTypes(const UBlueprint* Blueprint)
{
	if (Settings == nullptr)
	{
		Settings = const_cast<UOWINamingConventionHandlerSettings*>(GetDefault<UOWINamingConventionHandlerSettings>());
	}
	
	if (Blueprint == nullptr || Settings == nullptr)
	{
		return nullptr;
	}

	FOWIPrefixSuffix* PrefixSuffix = Settings->EBlueprintTypePrefixMap.Find(Blueprint->BlueprintType);

	if (PrefixSuffix == nullptr)
	{
		for (TPair<UObject*, FOWIPrefixSuffix>& Pair : Settings->UObjectPrefixMap)
		{
			if (Blueprint->ParentClass && Pair.Key && Blueprint->ParentClass->GetName().Equals(Pair.Key->GetName()))
			{
				PrefixSuffix = &Pair.Value;
				break;
			}
		}
	}

	if (PrefixSuffix == nullptr)
	{
		PrefixSuffix = GetGenericPrefixSuffix(Blueprint->ParentClass->GetName());
	}

	return PrefixSuffix;
}

FOWIPrefixSuffix* UOWINamingConventionHandler::CheckMaterialTypes(const UMaterial* Material)
{
	if (Settings == nullptr)
	{
		Settings = const_cast<UOWINamingConventionHandlerSettings*>(GetDefault<UOWINamingConventionHandlerSettings>());
	}
	
	if (Material == nullptr || Settings == nullptr)
	{
		return nullptr;
	}

	return Material->IsPostProcessMaterial() ? &Settings->Material_WithPostProcessing : &Settings->Material_WithoutPostProcessing;
}

FOWIPrefixSuffix* UOWINamingConventionHandler::CheckMaterialInstanceTypes(UMaterialInstance* MaterialInstance)
{
	if (Settings == nullptr)
	{
		Settings = const_cast<UOWINamingConventionHandlerSettings*>(GetDefault<UOWINamingConventionHandlerSettings>());
	}
	
	if (MaterialInstance == nullptr || Settings == nullptr)
	{
		return nullptr;
	}

	return MaterialInstance->GetBaseMaterial()->IsPostProcessMaterial() ? &Settings->MaterialInstance_WithPostProcessing : &Settings->MaterialInstance_WithoutPostProcessing;
}

FOWIPrefixSuffix* UOWINamingConventionHandler::CheckSkeletalMeshTypes(USkeletalMesh* SkeletalMeshInstance)
{
	if (Settings == nullptr)
	{
		Settings = const_cast<UOWINamingConventionHandlerSettings*>(GetDefault<UOWINamingConventionHandlerSettings>());
	}
	
	if (SkeletalMeshInstance == nullptr || Settings == nullptr)
	{
		return nullptr;
	}

	return SkeletalMeshInstance->MorphTargets.Num() > 0 ? &Settings->SkeletalMesh_WithMorphTargets : &Settings->SkeletalMesh_WithoutMorphTargets;
}
