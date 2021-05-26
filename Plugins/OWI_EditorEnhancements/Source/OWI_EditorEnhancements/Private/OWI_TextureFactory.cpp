// Copyright Offworld Industries Ltd. All Rights Reserved.

#include "OWI_TextureFactory.h"

#include "AssetImportTask.h"
#include "AssetToolsModule.h"
#include "Dialogs/CustomDialog.h"
#include "UObject/Class.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Input/SCheckBox.h"
#include "Widgets/Input/SSpinBox.h"
#include "Widgets/Input/STextComboBox.h"
#include "Widgets/Layout/SUniformGridPanel.h"

DEFINE_LOG_CATEGORY(LogOWITextureImporter);

class FAssetToolsModule;
class IAssetTools;

UOWITextureFactory::UOWITextureFactory(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer),
    CurrentImageImportSettings(nullptr)
{
    // We need a higher priority than the base class
    ++ImportPriority;
}

UObject* UOWITextureFactory::FactoryCreateBinary(UClass* Class, UObject* InParent, FName Name, const EObjectFlags Flags,
                                                 UObject* Context, const TCHAR* Type, const uint8*& Buffer, const uint8* BufferEnd, FFeedbackContext* Warn)
{
    if (false == Name.ToString().StartsWith("T_"))
    {
        Name = *FString("T_").Append(Name.ToString());
    }
    
    UObject* Object = Super::FactoryCreateBinary(Class, InParent, Name, Flags, Context, Type, Buffer, BufferEnd, Warn);
    
    UTexture* Texture = CastChecked<UTexture>(Object);
    if (false == IsValid(Texture))
    {
        // Something went wrong - return the original Object
        UE_LOG(LogOWITextureImporter, Error, TEXT("Couldn't import as Texture"));
        return Object;
    }

    if (bUseSameSettingsForAll == false)
    {
        const TSharedPtr<SOWITextureImportSettingsDialog> TextureImportSettingsDialog = SNew(SOWITextureImportSettingsDialog)
            .FileName(CurrentFilename).Image(Texture);
        if (TextureImportSettingsDialog == nullptr)
        {
            // Something went wrong - return the original Object
            UE_LOG(LogOWITextureImporter, Error, TEXT("Unable to show Texture Import Settings Dialog"));
            return Object;
        }
    
        CurrentImageImportSettings = TextureImportSettingsDialog->GetTextureImportSettings();
        bUseSameSettingsForAll = TextureImportSettingsDialog->UseSameSettingsForAll();
    }

    if (CurrentImageImportSettings == nullptr)
    {
        // Something went wrong - return the original Object
        UE_LOG(LogOWITextureImporter, Error, TEXT("Unable to set Texture Import Settings"));
        return Object;
    }
        
    Texture->CompressionSettings = CurrentImageImportSettings->CompressionSettings;
    Texture->SRGB = CurrentImageImportSettings->bSRGB;
    Texture->bFlipGreenChannel = CurrentImageImportSettings->bFlipGreenChannel;
    Texture->MipGenSettings = CurrentImageImportSettings->MipGenSettings;
    Texture->LODBias = CurrentImageImportSettings->LODBias;
    Texture->LODGroup = CurrentImageImportSettings->LODGroup;
    
    return Texture;
}

void SOWITextureImportSettingsDialog::Construct(const FArguments& InArgs)
{
    bUseSameSettingsForAll = false;
    
    Settings = const_cast<UOWITextureFactorySettings*>(GetDefault<UOWITextureFactorySettings>());
    if (Settings == nullptr || Settings->bEnabled == false || Settings->PresetImportSettings.Num() == 0)
    {
        return;
    }

    // Make sure we have a clean state
    Options_ImportPreset.Empty();
    Options_CompressionSettings.Empty();
    Options_MipGenSettings.Empty();
    Options_LODGroup.Empty();
    
    // Add all possible options to the Combo Box
    for (const TPair<FString, FOWITextureImportSettings>& Pair : Settings->PresetImportSettings)
    {
        Options_ImportPreset.Add(MakeShareable<FString>(new FString(Pair.Key)));
    }
    
    CurrentImageImportSettings = Settings->PresetImportSettings.Find(*Options_ImportPreset[0]);

    Options_CompressionSettings = GetEnumNames(ENUMNAME_TEXTURECOMPRESSIONSETTINGS);
    Options_MipGenSettings = GetEnumNames(ENUMNAME_TEXTUREMIPGENSETTINGS);
    Options_LODGroup = GetEnumNames(ENUMNAME_TEXTUREGROUP);

    FSlateBrush PreviewBrush;
    PreviewBrush.SetResourceObject(InArgs._Image);
    PreviewBrush.ImageSize.X = 128; //InArgs._Image->GetSurfaceWidth();
    PreviewBrush.ImageSize.Y = 128; //InArgs._Image->GetSurfaceHeight();
    PreviewBrush.DrawAs = ESlateBrushDrawType::Image;

    const TSharedRef<SVerticalBox> DialogContents = SNew(SVerticalBox)
    + SVerticalBox::Slot().FillHeight(1).Padding(5, 5).AutoHeight()
    [
        SNew(SImage).Image(&PreviewBrush)
    ]
    + SVerticalBox::Slot().FillHeight(2).Padding(5, 5).AutoHeight()
    [
        SNew(SUniformGridPanel).SlotPadding(FMargin(5.0f))
        +SUniformGridPanel::Slot(0,0).VAlign(VAlign_Center).HAlign(HAlign_Left)
        [ 
            SNew(STextBlock).Text(FText::FromString("Filename:"))
        ]
        +SUniformGridPanel::Slot(1,0).VAlign(VAlign_Center).HAlign(HAlign_Right)
        [
            SNew(STextBlock).Text(FText::FromString(InArgs._FileName))
        ]
        +SUniformGridPanel::Slot(0, 1).VAlign(VAlign_Center).HAlign(HAlign_Left)
        [ 
            SNew(STextBlock).Text(FText::FromString("Selected Preset"))
        ]
        +SUniformGridPanel::Slot(1,1).VAlign(VAlign_Center).HAlign(HAlign_Right)
        [
            SAssignNew(ComboBox_ImportPreset, STextComboBox)
                .OptionsSource(&Options_ImportPreset)
                .InitiallySelectedItem(Options_ImportPreset[0])
                .OnSelectionChanged(this, &SOWITextureImportSettingsDialog::HandleImageImportSettingSelectionChanged)
        ]
        +SUniformGridPanel::Slot(0,2).VAlign(VAlign_Center).HAlign(HAlign_Left)
        [
            SNew(STextBlock).Text(FText::FromString("Compression Settings"))
        ]
        +SUniformGridPanel::Slot(1,2).VAlign(VAlign_Center).HAlign(HAlign_Right)
        [
            SAssignNew(ComboBox_CompressionSettings, STextComboBox)
                .OptionsSource(&Options_CompressionSettings)
                .InitiallySelectedItem(Options_CompressionSettings[CurrentImageImportSettings->CompressionSettings])
                .OnSelectionChanged(this, &SOWITextureImportSettingsDialog::HandleCompressionSettingSelectionChanged)
        ]
        +SUniformGridPanel::Slot(0,3).VAlign(VAlign_Center).HAlign(HAlign_Left)
        [
            SNew(STextBlock).Text(FText::FromString("sRGB"))
        ]
        +SUniformGridPanel::Slot(1,3).VAlign(VAlign_Center).HAlign(HAlign_Right)
        [
            SNew(SCheckBox)
                .IsChecked(this, &SOWITextureImportSettingsDialog::IsSRGBChecked)
                .OnCheckStateChanged(this, &SOWITextureImportSettingsDialog::SRGBCheckedChanged)
        ]
        +SUniformGridPanel::Slot(0,4).VAlign(VAlign_Center).HAlign(HAlign_Left)
        [
                SNew(STextBlock).Text(FText::FromString("Flip Green Channel"))
        ]
        +SUniformGridPanel::Slot(1,4).VAlign(VAlign_Center).HAlign(HAlign_Right)
        [
            SNew(SCheckBox)
                .IsChecked(this, &SOWITextureImportSettingsDialog::IsFlipGreenChannelChecked)
                .OnCheckStateChanged(this, &SOWITextureImportSettingsDialog::FlipGreenChannelCheckedChanged)
        ]
        +SUniformGridPanel::Slot(0,5).VAlign(VAlign_Center).HAlign(HAlign_Left)
        [
                SNew(STextBlock).Text(FText::FromString("Mip Gen Settings"))
        ]
        +SUniformGridPanel::Slot(1,5).VAlign(VAlign_Center).HAlign(HAlign_Right)
        [
            SAssignNew(ComboBox_MipGenSettings, STextComboBox)
                .OptionsSource(&Options_MipGenSettings)
                .InitiallySelectedItem(Options_MipGenSettings[CurrentImageImportSettings->MipGenSettings])
                .OnSelectionChanged(this, &SOWITextureImportSettingsDialog::HandleMipGenSettingsSelectionChanged)
        ]
        +SUniformGridPanel::Slot(0,6).VAlign(VAlign_Center).HAlign(HAlign_Left)
        [
            SNew(STextBlock).Text(FText::FromString("LOD Bias"))
        ]
        +SUniformGridPanel::Slot(1,6).VAlign(VAlign_Center).HAlign(HAlign_Right)
        [
            SNew(SSpinBox<int32>)
                .MinSliderValue(0)
                .MaxSliderValue(5)
                .MinValue(0)
                .MaxValue(5)
                .OnValueChanged(this, &SOWITextureImportSettingsDialog::LODBiasValueChanged)  
                .Value(this, &SOWITextureImportSettingsDialog::GetLODBiasValue)
                .MinDesiredWidth(225)
        ]
        +SUniformGridPanel::Slot(0,7).VAlign(VAlign_Center).HAlign(HAlign_Left)
        [
            SNew(STextBlock).Text(FText::FromString("Texture Group"))
        ]
        +SUniformGridPanel::Slot(1,7).VAlign(VAlign_Center).HAlign(HAlign_Right)
        [
            SAssignNew(ComboBox_LODGroup, STextComboBox)
                .OptionsSource(&Options_LODGroup)
                .InitiallySelectedItem(Options_LODGroup[CurrentImageImportSettings->LODGroup])
                .OnSelectionChanged(this, &SOWITextureImportSettingsDialog::HandleLODGroupSettingsSelectionChanged)
        ]
    ];
    
    TSharedRef<SCustomDialog> CustomDialog = SNew(SCustomDialog)
        .Title(FText::FromString("OWI Texture Importer"))
        .DialogContent(DialogContents)
        .Buttons({ SCustomDialog::FButton(FText::FromString("Ignore Importer")), SCustomDialog::FButton(FText::FromString("Import")), SCustomDialog::FButton(FText::FromString("Import All"))});

    const int DialogResult = CustomDialog->ShowModal();
    if (DialogResult == 0)
    {
        // User has selected Cancel -> Use default settings
        CurrentImageImportSettings = nullptr;
    }
    
    bUseSameSettingsForAll = DialogResult == 2;
}

FOWITextureImportSettings* SOWITextureImportSettingsDialog::GetTextureImportSettings() const
{
    return CurrentImageImportSettings;
}

bool SOWITextureImportSettingsDialog::UseSameSettingsForAll() const
{
    return bUseSameSettingsForAll;
}

void SOWITextureImportSettingsDialog::HandleImageImportSettingSelectionChanged(const TSharedPtr<FString> NewSelection, ESelectInfo::Type SelectionType)
{
    if (NewSelection != nullptr)
    {
        CurrentImageImportSettings = Settings->PresetImportSettings.Find(*NewSelection);
        ComboBox_CompressionSettings->SetSelectedItem(Options_CompressionSettings[CurrentImageImportSettings->CompressionSettings]);
        ComboBox_MipGenSettings->SetSelectedItem(Options_MipGenSettings[CurrentImageImportSettings->MipGenSettings]);
        ComboBox_LODGroup->SetSelectedItem(Options_LODGroup[CurrentImageImportSettings->LODGroup]);
    }
}

void SOWITextureImportSettingsDialog::HandleCompressionSettingSelectionChanged(const TSharedPtr<FString> NewSelection, ESelectInfo::Type SelectionType) const
{
    if (NewSelection != nullptr)
    {
        CurrentImageImportSettings->CompressionSettings = static_cast<TextureCompressionSettings>(GetEnumValueByName(*NewSelection, Options_CompressionSettings));
    }
}

void SOWITextureImportSettingsDialog::HandleMipGenSettingsSelectionChanged(const TSharedPtr<FString> NewSelection, ESelectInfo::Type SelectionType) const
{
    if (NewSelection != nullptr)
    {
        CurrentImageImportSettings->MipGenSettings = static_cast<TextureMipGenSettings>(GetEnumValueByName(*NewSelection, Options_MipGenSettings));
    }
}

void SOWITextureImportSettingsDialog::HandleLODGroupSettingsSelectionChanged(const TSharedPtr<FString> NewSelection, ESelectInfo::Type SelectionType) const
{
    if (NewSelection != nullptr)
    {
        CurrentImageImportSettings->LODGroup = static_cast<TextureGroup>(GetEnumValueByName(*NewSelection, Options_LODGroup));
    }
}

TArray<TSharedPtr<FString>> SOWITextureImportSettingsDialog::GetEnumNames(const TCHAR* Name) const
{
    TArray<TSharedPtr<FString>> Names;
    const UEnum* EnumPtr = FindObject<UEnum>(ANY_PACKAGE, Name, true);
    if (EnumPtr != nullptr)
    {
        FString Left, Right;
        for (int64 i = 0; i < EnumPtr->GetMaxEnumValue(); ++i)
        {
            EnumPtr->GetNameStringByIndex(i).Split(TEXT("_"), &Left, &Right);            
            Names.Add(MakeShareable<FString>(new FString(Right)));
        }
    }
    
    return Names;
}

int32 SOWITextureImportSettingsDialog::GetEnumValueByName(const FString Name, const TArray<TSharedPtr<FString>> Options)
{
    int32 Result = 0;
    for (const TSharedPtr<FString> Option : Options)
    {
        if (Option->Equals(Name, ESearchCase::IgnoreCase))
        {
            return Result;
        }
        ++Result;
    }
    
    return -1;
}

ECheckBoxState SOWITextureImportSettingsDialog::IsSRGBChecked() const
{
    return CurrentImageImportSettings->bSRGB ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
}

void SOWITextureImportSettingsDialog::SRGBCheckedChanged(const ECheckBoxState CheckType) const
{
    CurrentImageImportSettings->bSRGB = CheckType == ECheckBoxState::Checked;
}

ECheckBoxState SOWITextureImportSettingsDialog::IsFlipGreenChannelChecked() const
{
    return CurrentImageImportSettings->bFlipGreenChannel ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
}

void SOWITextureImportSettingsDialog::FlipGreenChannelCheckedChanged(const ECheckBoxState CheckType) const
{
    CurrentImageImportSettings->bFlipGreenChannel = CheckType == ECheckBoxState::Checked;
}

void SOWITextureImportSettingsDialog::LODBiasValueChanged(const int32 NewValue) const
{
    CurrentImageImportSettings->LODBias = NewValue;
}

int32 SOWITextureImportSettingsDialog::GetLODBiasValue() const
{
    return CurrentImageImportSettings->LODBias;
}
