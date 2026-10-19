// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleInterface.h"

class FMaterialInstanceToolModule : public IModuleInterface
{
public:
	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

	/**
	 * Opens (or focuses) the tool tab. If a mesh asset is provided, it is
	 * pre-selected in the panel. Used by the asset context menu entries.
	 */
	void OpenToolWithMesh(UObject* MeshAsset);

private:
	void RegisterMenus();
	void RegisterAssetContextMenus();
	TSharedRef<class SDockTab> OnSpawnPluginTab(const class FSpawnTabArgs& Args);

	FDelegateHandle MenuStartupCallbackHandle;

	/** Mesh passed to the panel the next time the tab is spawned. */
	TWeakObjectPtr<UObject> PendingMesh;

	/** The currently open panel, if any. */
	TWeakPtr<class SMaterialInstanceToolWidget> ToolWidget;
};
