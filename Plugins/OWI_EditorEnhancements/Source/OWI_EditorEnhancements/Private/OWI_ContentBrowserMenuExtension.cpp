// Copyright Offworld Industries

#include "OWI_ContentBrowserMenuExtension.h"
#include "OWI_UnReleasedAssetsHandler.h"

#define LOCTEXT_NAMESPACE "OWI_ContentBrowserMenuExtension"

void FOWIContentBrowserMenuExtension::AddMenuEntry(FMenuBuilder& MenuBuilder, const TArray<FString> SelectedPaths)
{
	UOWIUnReleasedAssetsHandler* UnReleasedAssetsHandler = NewObject<UOWIUnReleasedAssetsHandler>();
	const EPathState PathState = UnReleasedAssetsHandler->GetFolderState(SelectedPaths); 

	if (PathState == None)
	{
		return;
	}
	
	const bool PluralFolders = SelectedPaths.Num() > 1;
	
	FString Text;
	const FString FolderText = FString(" Folder").Append(PluralFolders ? "s" : "");
	FString TooltipText = FString("Mark th").Append(PluralFolders ? "is" : "ese").Append(FolderText).Append(" as ");
    switch (PathState)
    {
	    case None:
	    	return;
	    case Released:
	    	Text = "Unrelease";
    		break;
	    case Unreleased:
	    	Text = "Release";
    		break;
	    case Mixed:
	    	Text = "UnRelease";
    		break;
    }
	TooltipText.Append(Text).Append("d");
	Text.Append(FolderText);
	
	MenuBuilder.BeginSection(NAME_None, FText::FromString("OWI Editor Enhancements"));
	{
    	if (PathState != Mixed)
    	{
    		MenuBuilder.AddMenuEntry
             (
                 FText::FromString(Text),
                 FText::FromString(TooltipText),
                 FSlateIcon(),
                 FUIAction(FExecuteAction::CreateRaw(this,
                 	PathState == Released ? &FOWIContentBrowserMenuExtension::OnUnreleasedClicked : &FOWIContentBrowserMenuExtension::OnReleasedClicked,
                 	SelectedPaths))
             );
    	} else
    	{
    		MenuBuilder.AddSubMenu(FText::FromString("UnRelease Folders"),
            FText::FromString(TooltipText),
            FNewMenuDelegate::CreateRaw(this, &FOWIContentBrowserMenuExtension::FillSubmenu, SelectedPaths));
    	}
	}
	MenuBuilder.EndSection();
}

void FOWIContentBrowserMenuExtension::FillSubmenu(FMenuBuilder& MenuBuilder, const TArray<FString> SelectedPaths)
{
	MenuBuilder.AddMenuEntry(
        FText::FromString("Release Folders"),
        FText::FromString("Mark these Folders as Released"),
        FSlateIcon(),
        FUIAction(FExecuteAction::CreateRaw(this, &FOWIContentBrowserMenuExtension::OnReleasedClicked, SelectedPaths))
    );
	MenuBuilder.AddMenuEntry(
        FText::FromString("Unrelease Folders"),
        FText::FromString("Mark these Folders as Unreleased"),
        FSlateIcon(),
        FUIAction(FExecuteAction::CreateRaw(this, &FOWIContentBrowserMenuExtension::OnUnreleasedClicked, SelectedPaths))
    );
}

void FOWIContentBrowserMenuExtension::OnReleasedClicked(TArray<FString> SelectedPaths) const
{
	UOWIUnReleasedAssetsHandler* UnReleasedAssetsHandler = NewObject<UOWIUnReleasedAssetsHandler>();
	for (const auto Path : SelectedPaths)
	{
		UnReleasedAssetsHandler->ReleaseFolder(Path);
	}
}

void FOWIContentBrowserMenuExtension::OnUnreleasedClicked(TArray<FString> SelectedPaths) const
{
	UOWIUnReleasedAssetsHandler* UnReleasedAssetsHandler = NewObject<UOWIUnReleasedAssetsHandler>();
	for (const auto Path : SelectedPaths)
	{
		UnReleasedAssetsHandler->UnreleaseFolder(Path);
	}
}


#undef LOCTEXT_NAMESPACE
