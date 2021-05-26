// Copyright Offworld Industries Ltd. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

/**
 * Class to allow for a UE4 menu extension
 */
class OWI_EDITORENHANCEMENTS_API FOWIContentBrowserMenuExtension
{
	
public:
	
	/**
	 * @brief Adds an Entry to the Menu
	 * @param MenuBuilder The FMenuBuilder to use
	 * @param SelectedPaths The paths the user has selected
	 */
	void AddMenuEntry(FMenuBuilder& MenuBuilder, TArray<FString> SelectedPaths);

	/**
	 * @brief Fills the given submenu with Unrelease and Release items
	 * @param MenuBuilder The FMenuBuilder to use
	 * @param SelectedPaths The Paths the user has selected
	 */
	void FillSubmenu(FMenuBuilder& MenuBuilder, TArray<FString> SelectedPaths) const;
	
	/**
	 * @brief "Release" has been clicked for the given paths
	 * @param SelectedPaths The paths to release
	 */
	void OnReleasedClicked(TArray<FString> SelectedPaths) const;
	
	/**
	* @brief "Unrelease" has been clicked for the given paths
	* @param SelectedPaths The paths to unrelease
	*/
	void OnUnreleasedClicked(TArray<FString> SelectedPaths) const;

};
