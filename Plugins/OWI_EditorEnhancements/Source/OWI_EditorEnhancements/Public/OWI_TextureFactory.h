// Copyright Offworld Industries Ltd. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "Engine/DeveloperSettings.h"
#include "Factories/TextureFactory.h"
#include "UObject/NoExportTypes.h"
#include "Widgets/Input/STextComboBox.h"

#include "OWI_TextureFactory.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogOWITextureImporter, Log, All);

class STextureDialog;
class SCustomDialog;
class STextComboBox;

/**
 * Struct that represents the Import Settings for a texture.
 */
USTRUCT()
struct OWI_EDITORENHANCEMENTS_API FOWITextureImportSettings
{
	GENERATED_BODY()

public:
	
    UPROPERTY(Config, NoClear, EditAnywhere, Category = "OWI Texture Import")
	TEnumAsByte<enum TextureCompressionSettings> CompressionSettings = TC_Default;
	
	UPROPERTY(Config, NoClear, EditAnywhere, Category = "OWI Texture Import")
	bool bSRGB = true;
	
	UPROPERTY(Config, NoClear, EditAnywhere, Category = "OWI Texture Import")
	bool bFlipGreenChannel = false;
	
	UPROPERTY(Config, NoClear, EditAnywhere, Category = "OWI Texture Import")
	TEnumAsByte<enum TextureMipGenSettings> MipGenSettings = TMGS_FromTextureGroup;
	
	UPROPERTY(Config, NoClear, EditAnywhere, Category = "OWI Texture Import")
	int32 LODBias = 0;
	
	UPROPERTY(Config, NoClear, EditAnywhere, Category = "OWI Texture Import")
	TEnumAsByte<enum TextureGroup> LODGroup = TEXTUREGROUP_World;
};

/**
* Class to configure the UTRTextureFactory
*/
UCLASS(Config = Editor, defaultconfig, meta = (DisplayName = "OWI Texture Import"))
class OWI_EDITORENHANCEMENTS_API UOWITextureFactorySettings final : public UDeveloperSettings
{
	GENERATED_BODY()

public:
		
	UPROPERTY(EditAnywhere, Config, Category = "OWI Texture Import")
	bool bEnabled = true;
	
	UPROPERTY(EditAnywhere, Config, NoClear, Category = "OWI Texture Import", Meta = (EditCondition = "bEnabled"))
	TMap<FString, FOWITextureImportSettings> PresetImportSettings;
};

/**
 * Custom Texture importer that allows to set settings on how to import Textures
 */
UCLASS(customconstructor, collapsecategories, hidecategories = Object)
class OWI_EDITORENHANCEMENTS_API UOWITextureFactory final : public UTextureFactory
{
	GENERATED_BODY()
	
public:

	explicit UOWITextureFactory(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	//~ Begin UFactory Interface
	virtual UObject* FactoryCreateBinary( UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context, const TCHAR* Type, const uint8*& Buffer, const uint8* BufferEnd, FFeedbackContext* Warn ) override;
	//~ End UFactory Interface

private:
	
	bool bUseSameSettingsForAll = false;

	FOWITextureImportSettings* CurrentImageImportSettings;
	
};

/**
 * Class that will create and show a dialog when importing a texture to set the settings.
 */
class OWI_EDITORENHANCEMENTS_API SOWITextureImportSettingsDialog final : public SCompoundWidget
{
	
public:
	
	SLATE_BEGIN_ARGS(SOWITextureImportSettingsDialog) {}
	SLATE_ARGUMENT(FString, FileName)
	SLATE_ARGUMENT(UTexture*, Image)
	SLATE_END_ARGS()
	
	void Construct(const FArguments& InArgs);

	/**
	 * @brief Get the Texture Import Settings
	 * @return The FOWITextureImportSettings being used.
	 */
	FOWITextureImportSettings* GetTextureImportSettings() const;
	
	bool UseSameSettingsForAll() const;
	
private:

	bool bUseSameSettingsForAll = false;
	
	const TCHAR* ENUMNAME_TEXTURECOMPRESSIONSETTINGS = TEXT("TextureCompressionSettings");
	const TCHAR* ENUMNAME_TEXTUREMIPGENSETTINGS = TEXT("TextureMipGenSettings");
	const TCHAR* ENUMNAME_TEXTUREGROUP = TEXT("TextureGroup");
	
	// Holds the combo box in the STextComboBox dialog.
	TSharedPtr<STextComboBox> ComboBox_ImportPreset;
		
	// Holds the text options for the STextComboBox dialog.
	TArray<TSharedPtr<FString>> Options_ImportPreset;
	
	// Holds the combo box in the STextComboBox dialog.
	TSharedPtr<STextComboBox> ComboBox_CompressionSettings;
		
	// Holds the text options for the STextComboBox dialog.
	TArray<TSharedPtr<FString>> Options_CompressionSettings;
	
	// Holds the combo box in the STextComboBox dialog.
	TSharedPtr<STextComboBox> ComboBox_MipGenSettings;
		
	// Holds the text options for the STextComboBox dialog.
	TArray<TSharedPtr<FString>> Options_MipGenSettings;
	
	// Holds the combo box in the STextComboBox dialog.
	TSharedPtr<STextComboBox> ComboBox_LODGroup;
		
	// Holds the text options for the STextComboBox dialog.
	TArray<TSharedPtr<FString>> Options_LODGroup;

	// All available Settings
	UOWITextureFactorySettings* Settings = nullptr;

	// The currently selected ImageImportSetting
	FOWITextureImportSettings* CurrentImageImportSettings = nullptr;
	
	/**
	 * @brief The ImportSettings selection has changed
	 * @param NewSelection The newly selected settings
	 * @param SelectionType On the selection was made
	 */
	void HandleImageImportSettingSelectionChanged(TSharedPtr<FString> NewSelection, ESelectInfo::Type SelectionType);

	/**
	* @brief The Compression settings selection has changed
	* @param NewSelection The newly selected settings
	* @param SelectionType On the selection was made
	*/
	void HandleCompressionSettingSelectionChanged(TSharedPtr<FString> NewSelection, ESelectInfo::Type SelectionType) const;

	/**
	* @brief The MipGen settings selection has changed
	* @param NewSelection The newly selected settings
	* @param SelectionType On the selection was made
	*/
	void HandleMipGenSettingsSelectionChanged(TSharedPtr<FString> NewSelection, ESelectInfo::Type SelectionType) const;

	/**
	* @brief The LOD group settings selection has changed
	* @param NewSelection The newly selected settings
	* @param SelectionType On the selection was made
	*/
	void HandleLODGroupSettingsSelectionChanged(TSharedPtr<FString> NewSelection, ESelectInfo::Type SelectionType) const;

	/**
	 * @brief Get the Names of the Enums
	 * @param Name Name of the Enum to convert
	 * @return The names of the enums
	 */
	TArray<TSharedPtr<FString>> GetEnumNames(const TCHAR* Name) const;

	/**
	 * @brief Convert a String to the corresponding enum
	 * @param Name The name to check for
	 * @param Options The possible enums
	 * @return The enum found
	 */
	static int32 GetEnumValueByName(FString Name, TArray<TSharedPtr<FString>> Options);

	/**
	 * @brief Is SRGB checked?
	 * @return State of the RGB checkbox
	 */
	ECheckBoxState IsSRGBChecked() const;
	
	/**
	 * @brief The value of the SRGB checkbox changed
	 * @param CheckType The new state
	 */
	void SRGBCheckedChanged(ECheckBoxState CheckType) const;

	/**
	* @brief Is "flip green channel" checked?
	* @return State of the "flip green channel" checkbox
	*/
	ECheckBoxState IsFlipGreenChannelChecked() const;
	
	/**
	* @brief The value of the "flip green channel" checkbox changed
	* @param CheckType The new state
	*/
	void FlipGreenChannelCheckedChanged(ECheckBoxState CheckType) const;

	/**
	 * @brief The value of the LOD Bias value has changed
	 * @param NewValue The new value for LOD Bias
	 */
	void LODBiasValueChanged(int32 NewValue) const;
	
	/**
	 * @brief Get the value for LOD Bias
	 * @return The value for LOD Bias
	 */
	int32 GetLODBiasValue() const;
};