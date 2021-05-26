// Copyright Offworld Industries Ltd. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "EditorValidatorBase.h"

#include "OWI_EditorValidator.generated.h"

/**
 * The class to act as a Validator for the project
 */
UCLASS()
class OWI_EDITORENHANCEMENTS_API UOWIEditorValidator final : public UEditorValidatorBase
{
	GENERATED_BODY()

public:

	/**
	 * @brief Is this Validator enabled?
	 * @return True if enabled
	 */
	virtual bool IsEnabled() const override;
	
protected:
	
	/**
	 * @brief Can this Asset be validated by this Validator?
	 * @param InAsset The Asset to check for
	 * @return True if the Asset can be validated
	 */
	virtual bool CanValidateAsset_Implementation(UObject* InAsset) const override;
	
	/**
	 * @brief Validate the given Asset with this Validator
	 * @param InAsset The Asset to validate
	 * @param ValidationErrors Validation Errors that appeared
	 * @return The Result of the Validation
	 */
	virtual EDataValidationResult ValidateLoadedAsset_Implementation(UObject* InAsset, TArray<FText>& ValidationErrors) override;

private:
	
	const FString ErrorPrefix = "LogOwiAssetValidation: ";
};
