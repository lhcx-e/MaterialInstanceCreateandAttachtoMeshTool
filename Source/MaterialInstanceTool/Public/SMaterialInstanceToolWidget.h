// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "AssetRegistry/AssetData.h"
#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"

class UMaterialInterface;
class UObject;

/**
 * Dockable tool panel: pick a parent material and a target mesh, then generate
 * one named material instance per material slot. Instances are created in the
 * same folder as the mesh.
 */
class SMaterialInstanceToolWidget : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SMaterialInstanceToolWidget)
	{
	}
	SLATE_END_ARGS()

	/** Constructs the widget */
	void Construct(const FArguments& InArgs);

	/**
	 * Pre-selects a mesh asset (e.g. when the tool was opened from the mesh's
	 * asset context menu). Refreshes the slot preview and output folder info.
	 */
	void SetTargetMesh(UObject* MeshAsset);

private:
	/** Builds a two-column label/content row. */
	static TSharedRef<SWidget> MakeRow(const FText& Label, TSharedRef<SWidget> Content);

	/** Input delegates */
	FString GetParentMaterialPath() const;
	void OnParentMaterialChanged(const FAssetData& AssetData);
	FString GetMeshPath() const;
	void OnMeshChanged(const FAssetData& AssetData);
	FText GetInstancePrefixText() const;
	void OnInstancePrefixTextChanged(const FText& NewText);
	ECheckBoxState GetOverwriteCheckState() const;
	void OnOverwriteCheckStateChanged(ECheckBoxState NewState);

	/** Attribute getters */
	FText GetSlotPreviewText() const;
	FText GetTargetFolderText() const;
	FText GetLogText() const;

	/** Actions */
	FReply OnGenerateClicked();
	bool IsGenerateEnabled() const;

	/** Helpers */
	void RefreshSlotPreview();
	void RefreshTargetFolder();
	void AppendLog(const TArray<FString>& Lines);

private:
	TWeakObjectPtr<UMaterialInterface> ParentMaterial;
	TWeakObjectPtr<UObject> TargetMesh;
	FString InstancePrefix = TEXT("MI_");
	bool bOverwriteExisting = false;

	FText SlotPreviewText;
	FText TargetFolderText;
	FText LogText;

	TSharedPtr<class SMultiLineEditableTextBox> LogTextBox;
};
