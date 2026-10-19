// Copyright Epic Games, Inc. All Rights Reserved.

#include "SMaterialInstanceToolWidget.h"

#include "MaterialInstanceGenerator.h"
#include "PropertyCustomizationHelpers.h"
#include "Styling/AppStyle.h"

#include "Engine/SkeletalMesh.h"
#include "Engine/SkinnedAssetCommon.h"
#include "Engine/StaticMesh.h"
#include "Engine/StreamableRenderAsset.h"
#include "Materials/MaterialInterface.h"
#include "Misc/PackageName.h"

#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SCheckBox.h"
#include "Widgets/Input/SEditableTextBox.h"
#include "Widgets/Input/SMultiLineEditableTextBox.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SSeparator.h"
#include "Widgets/Text/STextBlock.h"

#define LOCTEXT_NAMESPACE "MaterialInstanceTool"

void SMaterialInstanceToolWidget::Construct(const FArguments& InArgs)
{
	ChildSlot
	[
		SNew(SBorder)
		.Padding(FMargin(12.0f))
		.BorderImage(FAppStyle::GetBrush("ToolPanel.GroupBorder"))
		[
			SNew(SVerticalBox)

			// Header
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(FMargin(0.0f, 0.0f, 0.0f, 8.0f))
			[
				SNew(STextBlock)
				.Text(LOCTEXT("Header", "Create one material instance per material slot, named after the slot. Instances are created next to the mesh."))
				.AutoWrapText(true)
				.Font(FAppStyle::GetFontStyle("NormalFontBold"))
			]

			// Target mesh
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(FMargin(0.0f, 4.0f))
			[
				MakeRow(LOCTEXT("MeshLabel", "Target Mesh"),
					SNew(SObjectPropertyEntryBox)
					.AllowedClass(UStreamableRenderAsset::StaticClass())
					.AllowClear(true)
					.DisplayThumbnail(true)
					.ObjectPath(this, &SMaterialInstanceToolWidget::GetMeshPath)
					.OnObjectChanged(this, &SMaterialInstanceToolWidget::OnMeshChanged)
					.ToolTipText(LOCTEXT("MeshTip", "Right-click a mesh in the Content Browser to open this tool with it, or pick one here. Static Mesh and Skeletal Mesh are both supported.")))
			]

			// Slot preview
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(FMargin(0.0f, 2.0f, 0.0f, 4.0f))
			[
				SNew(STextBlock)
				.Text(this, &SMaterialInstanceToolWidget::GetSlotPreviewText)
				.AutoWrapText(true)
				.ColorAndOpacity(FSlateColor::UseSubduedForeground())
			]

			// Parent material
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(FMargin(0.0f, 4.0f))
			[
				MakeRow(LOCTEXT("ParentMaterialLabel", "Parent Material"),
					SNew(SObjectPropertyEntryBox)
					.AllowedClass(UMaterialInterface::StaticClass())
					.AllowClear(true)
					.DisplayThumbnail(true)
					.ObjectPath(this, &SMaterialInstanceToolWidget::GetParentMaterialPath)
					.OnObjectChanged(this, &SMaterialInstanceToolWidget::OnParentMaterialChanged)
					.ToolTipText(LOCTEXT("ParentMaterialTip", "The material used as the parent of every generated instance.")))
			]

			// Target folder (read-only, derived from the mesh)
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(FMargin(0.0f, 4.0f))
			[
				MakeRow(LOCTEXT("TargetFolderLabel", "Output Folder"),
					SNew(STextBlock)
					.Text(this, &SMaterialInstanceToolWidget::GetTargetFolderText)
					.ColorAndOpacity(FSlateColor::UseSubduedForeground()))
			]

			// Instance prefix
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(FMargin(0.0f, 4.0f))
			[
				MakeRow(LOCTEXT("PrefixLabel", "Instance Name Prefix"),
					SNew(SEditableTextBox)
					.Text(this, &SMaterialInstanceToolWidget::GetInstancePrefixText)
					.OnTextChanged(this, &SMaterialInstanceToolWidget::OnInstancePrefixTextChanged)
					.HintText(LOCTEXT("PrefixHint", "MI_")))
			]

			// Overwrite
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(FMargin(0.0f, 4.0f))
			[
				MakeRow(LOCTEXT("OverwriteLabel", "Overwrite Existing"),
					SNew(SCheckBox)
					.IsChecked(this, &SMaterialInstanceToolWidget::GetOverwriteCheckState)
					.OnCheckStateChanged(this, &SMaterialInstanceToolWidget::OnOverwriteCheckStateChanged)
					.Content()
					[
						SNew(STextBlock)
						.Text(LOCTEXT("OverwriteCheck", "Delete and recreate instances that already exist"))
					])
			]

			// Generate button
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(FMargin(0.0f, 12.0f, 0.0f, 4.0f))
			[
				SNew(SButton)
				.ButtonStyle(FAppStyle::Get(), "PrimaryButton")
				.ContentPadding(FMargin(12.0f, 6.0f))
				.IsEnabled(this, &SMaterialInstanceToolWidget::IsGenerateEnabled)
				.OnClicked(this, &SMaterialInstanceToolWidget::OnGenerateClicked)
				.HAlign(HAlign_Center)
				[
					SNew(STextBlock)
					.Text(LOCTEXT("GenerateButton", "Generate Material Instances"))
				]
			]

			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(FMargin(0.0f, 4.0f))
			[
				SNew(SSeparator)
			]

			// Log
			+ SVerticalBox::Slot()
			.FillHeight(1.0f)
			.Padding(FMargin(0.0f, 4.0f, 0.0f, 0.0f))
			[
				SAssignNew(LogTextBox, SMultiLineEditableTextBox)
				.Text(this, &SMaterialInstanceToolWidget::GetLogText)
				.IsReadOnly(true)
				.AlwaysShowScrollbars(true)
				.AutoWrapText(true)
			]
		]
	];
}

TSharedRef<SWidget> SMaterialInstanceToolWidget::MakeRow(const FText& Label, TSharedRef<SWidget> Content)
{
	return SNew(SHorizontalBox)

		+ SHorizontalBox::Slot()
		.FillWidth(0.35f)
		.VAlign(VAlign_Center)
		.Padding(FMargin(0.0f, 0.0f, 10.0f, 0.0f))
		[
			SNew(STextBlock)
			.Text(Label)
		]

		+ SHorizontalBox::Slot()
		.FillWidth(0.65f)
		.VAlign(VAlign_Center)
		[
			Content
		];
}

FString SMaterialInstanceToolWidget::GetParentMaterialPath() const
{
	return ParentMaterial.IsValid() ? ParentMaterial->GetPathName() : FString();
}

void SMaterialInstanceToolWidget::OnParentMaterialChanged(const FAssetData& AssetData)
{
	ParentMaterial = Cast<UMaterialInterface>(AssetData.GetAsset());
}

FString SMaterialInstanceToolWidget::GetMeshPath() const
{
	return TargetMesh.IsValid() ? TargetMesh->GetPathName() : FString();
}

void SMaterialInstanceToolWidget::OnMeshChanged(const FAssetData& AssetData)
{
	TargetMesh = AssetData.GetAsset();

	RefreshSlotPreview();
	RefreshTargetFolder();
}

void SMaterialInstanceToolWidget::SetTargetMesh(UObject* MeshAsset)
{
	TargetMesh = MeshAsset;

	RefreshSlotPreview();
	RefreshTargetFolder();
}

FText SMaterialInstanceToolWidget::GetInstancePrefixText() const
{
	return FText::FromString(InstancePrefix);
}

void SMaterialInstanceToolWidget::OnInstancePrefixTextChanged(const FText& NewText)
{
	InstancePrefix = NewText.ToString();
}

ECheckBoxState SMaterialInstanceToolWidget::GetOverwriteCheckState() const
{
	return bOverwriteExisting ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
}

void SMaterialInstanceToolWidget::OnOverwriteCheckStateChanged(ECheckBoxState NewState)
{
	bOverwriteExisting = (NewState == ECheckBoxState::Checked);
}

FText SMaterialInstanceToolWidget::GetSlotPreviewText() const
{
	return SlotPreviewText;
}

FText SMaterialInstanceToolWidget::GetTargetFolderText() const
{
	return TargetFolderText;
}

FText SMaterialInstanceToolWidget::GetLogText() const
{
	return LogText;
}

bool SMaterialInstanceToolWidget::IsGenerateEnabled() const
{
	return ParentMaterial.IsValid() && TargetMesh.IsValid();
}

FReply SMaterialInstanceToolWidget::OnGenerateClicked()
{
	const FMaterialInstanceGeneratorResult Result = FMaterialInstanceGenerator::Generate(
		ParentMaterial.Get(),
		TargetMesh.Get(),
		InstancePrefix,
		bOverwriteExisting);

	AppendLog(Result.LogLines);

	return FReply::Handled();
}

void SMaterialInstanceToolWidget::RefreshSlotPreview()
{
	const UObject* Mesh = TargetMesh.Get();
	if (!Mesh)
	{
		SlotPreviewText = FText::GetEmpty();
		return;
	}

	TArray<FName> Slots;
	if (const USkeletalMesh* SkeletalMesh = Cast<USkeletalMesh>(Mesh))
	{
		for (const FSkeletalMaterial& Material : SkeletalMesh->GetMaterials())
		{
			Slots.Add(Material.MaterialSlotName);
		}
	}
	else if (const UStaticMesh* StaticMesh = Cast<UStaticMesh>(Mesh))
	{
		for (const FStaticMaterial& Material : StaticMesh->GetStaticMaterials())
		{
			Slots.Add(Material.MaterialSlotName);
		}
	}

	if (Slots.Num() == 0)
	{
		SlotPreviewText = LOCTEXT("NoSlots", "This mesh has no material slots.");
		return;
	}

	FString Names;
	for (int32 Index = 0; Index < Slots.Num(); ++Index)
	{
		if (Index > 0)
		{
			Names += TEXT(", ");
		}
		Names += Slots[Index].IsNone() ? FString::Printf(TEXT("Slot%d"), Index) : Slots[Index].ToString();
	}

	SlotPreviewText = FText::FromString(FString::Printf(TEXT("%d slot(s): %s"), Slots.Num(), *Names));
}

void SMaterialInstanceToolWidget::RefreshTargetFolder()
{
	const UObject* Mesh = TargetMesh.Get();
	if (!Mesh)
	{
		TargetFolderText = LOCTEXT("NoFolderYet", "Select a mesh to determine the output folder.");
		return;
	}

	// Instances live in the same folder as the mesh.
	const FString Folder = FPackageName::GetLongPackagePath(Mesh->GetPathName());
	if (Folder.IsEmpty())
	{
		TargetFolderText = LOCTEXT("FolderUnknown", "Could not determine the mesh folder.");
		return;
	}

	TargetFolderText = FText::FromString(Folder);
}

void SMaterialInstanceToolWidget::AppendLog(const TArray<FString>& Lines)
{
	FString Combined;
	for (const FString& Line : Lines)
	{
		Combined += Line;
		Combined += TEXT("\n");
	}
	LogText = FText::FromString(LogText.ToString() + Combined);

	if (LogTextBox.IsValid())
	{
		LogTextBox->SetText(LogText);
	}
}

#undef LOCTEXT_NAMESPACE
