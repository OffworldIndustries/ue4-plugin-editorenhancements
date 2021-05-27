// Copyright Offworld Industries Ltd. All Rights Reserved.

#include "OWI_EditorValidator.h"

#include "OWI_NamingConventionHandler.h"
#include "OWI_ReferenceRulesHandler.h"

bool UOWIEditorValidator::IsEnabled() const
{
	return GetDefault<UOWINamingConventionHandlerSettings>()->bEnabled || GetDefault<UOWIReferenceRulesHandlerSettings>()->bEnabled;
}

bool UOWIEditorValidator::CanValidateAsset_Implementation(UObject* InAsset) const
{	
	if (nullptr == InAsset)
	{
		return false;
	}

	if (NewObject<UOWIReferenceRulesHandler>()->RuleCanBeApplied(InAsset->GetPathName()))
	{
		return true;
	}

	if (NewObject<UOWINamingConventionHandler>()->NamingConventionCanBeApplied(InAsset))
	{
		return true;
	}

	return false;
}

EDataValidationResult UOWIEditorValidator::ValidateLoadedAsset_Implementation(UObject* InAsset, TArray<FText>& ValidationErrors)
{
	FString ValidationError;
	if (nullptr == InAsset)
	{
		AssetFails(InAsset, FText::FromString(ErrorPrefix + "Asset is nullptr"), ValidationErrors);
		return EDataValidationResult::Invalid;
	}

	if (NewObject<UOWIReferenceRulesHandler>()->RuleIsViolated(InAsset->GetPathName(), ValidationError))
	{
		AssetFails(InAsset, FText::FromString(ErrorPrefix + ValidationError), ValidationErrors);
		return EDataValidationResult::Invalid;
	}

	if (NewObject<UOWINamingConventionHandler>()->NamingConventionIsViolated(InAsset, ValidationError))
	{
		AssetFails(InAsset, FText::FromString(ErrorPrefix + ValidationError), ValidationErrors);
		return EDataValidationResult::Invalid;
	}
	
    AssetPasses(InAsset);
    return EDataValidationResult::Valid;
}