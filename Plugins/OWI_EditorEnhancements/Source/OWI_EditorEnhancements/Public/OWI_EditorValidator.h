// Copyright Offworld Industries

#pragma once

#include "CoreMinimal.h"
#include "EditorValidatorBase.h"
#include "OWI_EditorValidator.generated.h"

UCLASS()
class OWI_EDITORENHANCEMENTS_API UOWIEditorValidator final : public UEditorValidatorBase
{
	GENERATED_BODY()

public:

	virtual bool IsEnabled() const override;
	
protected:
	
	virtual bool CanValidateAsset_Implementation(UObject* InAsset) const override;
	
	virtual EDataValidationResult ValidateLoadedAsset_Implementation(UObject* InAsset, TArray<FText>& ValidationErrors) override;

private:
	
	const FString ErrorPrefix = "LogOwiAssetValidation: ";
};
