// Copyright Offworld Industries Ltd. All Rights Reserved.

#include "OWI_ReferenceRulesHandler.h"

#include "Runtime/AssetRegistry/Public/AssetRegistryModule.h"
#include "Runtime/AssetRegistry/Public/IAssetRegistry.h"

bool FOWIReferenceRule::IsViolated(const FString& AssetPath, const TArray<FName>& References, FString& ValidationError) const
{
	for (const FName& Reference : References)
	{
		const FString AssetFilePath = ConvertFilePath(AssetFile);
		if (IsException(Reference)
			|| AssetPath.StartsWith(AssetFolder.Path) == false // skip files outside the folder we care about
			|| AssetPath.Equals(AssetFilePath)) // skip the file that may not be reference itself
		{
			continue;
		}

		if (Reference.ToString().StartsWith(MayNotReferenceFolder.Path)
			&& AssetPath.StartsWith(MayNotReferenceFolder.Path) == false)
		{
			ValidationError = FString::Printf(TEXT("%s references %s in folder %s."), *AssetPath, *Reference.ToString(), *MayNotReferenceFolder.Path);
			return true;
		}

		const FString MayNotReferenceFilePath = ConvertFilePath(MayNotReferenceFile);
		if (Reference.ToString().Equals(MayNotReferenceFilePath))
		{
			ValidationError = FString::Printf(TEXT("%s references %s."), *AssetPath, *Reference.ToString());
			return true;
		}
	}

	return false;
}

bool FOWIReferenceRule::CanBeApplied(const FString& AssetPath) const
{
	const FString AssetFilePath = ConvertFilePath(AssetFile);
	return AssetPath.StartsWith(AssetFolder.Path) || AssetPath.Equals(AssetFilePath);
}

FString FOWIReferenceRule::ConvertFilePath(const FFilePath& FilePath)
{
	FString OutString = FilePath.FilePath;
	if (FPaths::IsRelative(FilePath.FilePath))
	{
		OutString = OutString.Replace(*FPaths::ProjectContentDir(), TEXT("/Game/"));
	}

	return FPaths::GetBaseFilename(OutString, false);
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
	
	for (const FFilePath& ExceptionFile : ExceptionFiles)
	{
		const FString ExceptionFilePath = ConvertFilePath(ExceptionFile);
		if (Reference.ToString().Equals(ExceptionFilePath))
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

	if (Settings->bCheckCircularDependencies
		|| Settings->BlackListFolders.Num() > 0
		|| Settings->BlackListFiles.Num() > 0)
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
		for (const FDirectoryPath& BlackListItem : Settings->BlackListFolders)
		{
			if (Reference.ToString().StartsWith(BlackListItem.Path)
				&& AssetPath.StartsWith(BlackListItem.Path) == false)
			{
				ValidationError = FString::Printf(TEXT("%s references %s in folder %s."), *AssetPath, *Reference.ToString(), *BlackListItem.Path);
				return true;
			}
		}
		
		for (const FFilePath& BlackListItem : Settings->BlackListFiles)
		{
			const FString BlackListItemPath = FOWIReferenceRule::ConvertFilePath(BlackListItem);
			if (Reference.ToString().Equals(BlackListItemPath)
				&& AssetPath.Equals(BlackListItemPath) == false)
			{
				ValidationError = FString::Printf(TEXT("%s references %s."), *AssetPath, *Reference.ToString());
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
			ValidationError = FString::Printf(TEXT("%s has circular dependency."), *AssetPath);
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
