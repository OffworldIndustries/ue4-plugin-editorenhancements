// Copyright Offworld Industries

#include "OWI_ReferenceRulesHandler.h"
#include "Runtime/AssetRegistry/Public/AssetRegistryModule.h"
#include "Runtime/AssetRegistry/Public/IAssetRegistry.h"

bool FOWIReferenceRule::IsViolated(const FString& AssetPath, const TArray<FName>& References, FString& ValidationError) const
{
	for (const FName& Reference : References)
	{
		if (Reference.ToString().StartsWith(MayNotReferenceFolder.Path)
			&& AssetPath.StartsWith(MayNotReferenceFolder.Path) == false
			&& IsException(Reference) == false)
		{
			ValidationError = AssetPath + " references " + MayNotReferenceFolder.Path;
			return true;
		}
	}

	return false;
}

bool FOWIReferenceRule::CanBeApplied(const FString& AssetPath) const
{
	return AssetPath.StartsWith(AssetFolder.Path);
}

bool FOWIReferenceRule::IsException(const FName& Reference) const
{
	for (const FDirectoryPath& ExceptionFolder : ExceptionFolders)
	{
		if (Reference.ToString().StartsWith(ExceptionFolder.Path))
		{
			return true;
		}
	}

	return false;
}

bool UOWIReferenceRulesHandler::RuleIsViolated(const FString& AssetPath, FString& ValidationError)
{
	if (Settings == nullptr)
	{
		Settings = const_cast<UOWIReferenceRulesHandlerSettings*>(GetDefault<UOWIReferenceRulesHandlerSettings>());
		if (Settings == nullptr)
		{
			return false;
		}
	}
	
	if (RuleCanBeApplied(AssetPath) == false)
	{
		return false;
	}

	FAssetRegistryModule& AssetRegistry = FModuleManager::LoadModuleChecked<FAssetRegistryModule>(AssetRegistryConstants::ModuleName);

	const FAssetData AssetData = AssetRegistry.Get().GetAssetByObjectPath(*AssetPath);

	return CheckAllRules(AssetPath, AssetRegistry, AssetData, true, ValidationError)
		|| CheckAllRules(AssetPath, AssetRegistry, AssetData, false, ValidationError);
}

bool UOWIReferenceRulesHandler::RuleCanBeApplied(const FString& AssetPath)
{
	if (Settings == nullptr)
	{
		Settings = const_cast<UOWIReferenceRulesHandlerSettings*>(GetDefault<UOWIReferenceRulesHandlerSettings>());
		if (Settings == nullptr)
		{
			return false;
		}
	}
	
	if (Settings->bEnabled == false)
	{
		return false;
	}

	if (Settings->bCheckCircularDependencies || Settings->BlackList.Num() > 0)
	{
		// In case we have a blacklist or need to check circular dependencies we need to check everything
		return true;
	}

	// Otherwise we only need to check this asset if it is in a path we have a rule for
	for (const FOWIReferenceRule& Rule : Settings->ReferenceRules)
	{
		if (Rule.CanBeApplied(AssetPath))
		{
			return true;
		}
	}

	return false;
}

bool UOWIReferenceRulesHandler::CheckAllRules(const FString& AssetPath, const FAssetRegistryModule& AssetRegistry, const FAssetData& AssetData, const bool bHardDependencyType, FString& ValidationError)
{
	TArray<FName> References = TArray<FName>();
	AssetRegistry.Get().GetDependencies(AssetData.PackageName, References, bHardDependencyType ? EAssetRegistryDependencyType::Hard : EAssetRegistryDependencyType::Soft);

	if (Settings == nullptr)
	{
		Settings = const_cast<UOWIReferenceRulesHandlerSettings*>(GetDefault<UOWIReferenceRulesHandlerSettings>());
		if (Settings == nullptr)
		{
			return false;
		}
	}	

	if (IsBlackListed(AssetPath, References, ValidationError))
	{
		return true;
	}
	
	if (bHardDependencyType && Settings->bCheckCircularDependencies && HasCircularDependency(*AssetPath, AssetRegistry, AssetData, ValidationError))
	{
		return true;
	}
	
	for (const FOWIReferenceRule& Rule : Settings->ReferenceRules)
	{
		if (Rule.IsViolated(AssetPath, References, ValidationError))
		{
			return true;
		}
	}

	return false;
}

bool UOWIReferenceRulesHandler::IsBlackListed(const FString& AssetPath, const TArray<FName>& References, FString& ValidationError)
{
	if (Settings == nullptr)
	{
		Settings = const_cast<UOWIReferenceRulesHandlerSettings*>(GetDefault<UOWIReferenceRulesHandlerSettings>());
		if (Settings == nullptr)
		{
			return false;
		}
	}
	
	for (const FName& Reference : References)
	{
		for (const FDirectoryPath& BlackListItem : Settings->BlackList)
		{
			if (Reference.ToString().StartsWith(BlackListItem.Path)
				&& AssetPath.StartsWith(BlackListItem.Path) == false)
			{
				ValidationError = AssetPath + " references " + Reference.ToString();
				return true;
			}
		}
	}

	return false;
}

bool UOWIReferenceRulesHandler::HasCircularDependency(const FString& AssetPath, const FAssetRegistryModule& AssetRegistry, const FAssetData& AssetData, FString& ValidationError)
{
	TArray<FName> References;
	AssetRegistry.Get().GetDependencies(AssetData.PackageName, References, EAssetRegistryDependencyType::Hard);

	for (const FName& Reference : References)
	{
		if (Reference.ToString().StartsWith("/Game/") == false || AssetData.PackageName.IsEqual(Reference))
		{
			continue;
		}

		if (Reference.ToString().StartsWith(AssetPath))
		{
			ValidationError = AssetPath + " has circular dependency.";
			return true;
		}

		const FAssetData& NextAssetData = AssetRegistry.Get().GetAssetByObjectPath(Reference);
		if (HasCircularDependency(AssetPath, AssetRegistry, NextAssetData, ValidationError))
		{
			return true;
		}
	}

	return false;
}
