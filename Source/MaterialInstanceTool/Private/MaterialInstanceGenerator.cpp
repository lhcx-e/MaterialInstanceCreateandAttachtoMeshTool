// Copyright Epic Games, Inc. All Rights Reserved.

#include "MaterialInstanceGenerator.h"

#include "AssetToolsModule.h"
#include "EditorAssetLibrary.h"
#include "Factories/MaterialInstanceConstantFactoryNew.h"
#include "Materials/MaterialInstanceConstant.h"
#include "Materials/MaterialInterface.h"
#include "Misc/PackageName.h"
#include "Modules/ModuleManager.h"
#include "UObject/Package.h"

#include "Engine/SkeletalMesh.h"
#include "Engine/SkinnedAssetCommon.h"
#include "Engine/StaticMesh.h"

namespace MaterialInstanceTool
{
	/** Replace characters that are not allowed in an asset name. */
	static FString SanitizeAssetName(const FString& InName)
	{
		FString Out = InName;
		const FString InvalidChars = TEXT("/\\:*?\"<>|. \t\r\n");
		for (const TCHAR Char : InvalidChars)
		{
			Out.ReplaceCharInline(Char, TEXT('_'));
		}
		while (Out.StartsWith(TEXT("_")))
		{
			Out.RightChopInline(1);
		}
		while (Out.EndsWith(TEXT("_")))
		{
			Out.LeftChopInline(1);
		}
		if (Out.IsEmpty())
		{
			Out = TEXT("Slot");
		}
		return Out;
	}

	/** Long package path of an asset, e.g. /Game/MMD/Character/Leimi. */
	static FString GetAssetFolder(UObject* Asset)
	{
		if (!Asset)
		{
			return FString();
		}
		return FPackageName::GetLongPackagePath(Asset->GetPathName());
	}
}

FMaterialInstanceGeneratorResult FMaterialInstanceGenerator::Generate(
	UMaterialInterface* ParentMaterial,
	UObject* MeshAsset,
	const FString& InstancePrefix,
	bool bOverwriteExisting)
{
	FMaterialInstanceGeneratorResult Result;

	if (!ParentMaterial)
	{
		Result.LogLines.Add(TEXT("[Error] No parent material selected."));
		return Result;
	}

	if (!MeshAsset)
	{
		Result.LogLines.Add(TEXT("[Error] No target mesh selected."));
		return Result;
	}

	UStaticMesh* StaticMesh = Cast<UStaticMesh>(MeshAsset);
	USkeletalMesh* SkeletalMesh = Cast<USkeletalMesh>(MeshAsset);
	if (!StaticMesh && !SkeletalMesh)
	{
		Result.LogLines.Add(TEXT("[Error] The selected asset is neither a Static Mesh nor a Skeletal Mesh."));
		return Result;
	}

	// Instances are created next to the mesh, in its own folder.
	const FString OutFolder = MaterialInstanceTool::GetAssetFolder(MeshAsset);
	if (OutFolder.IsEmpty() || !FPackageName::IsValidLongPackageName(OutFolder))
	{
		Result.LogLines.Add(FString::Printf(
			TEXT("[Error] Could not determine the folder of '%s'."),
			*MeshAsset->GetPathName()));
		return Result;
	}
	Result.OutputFolder = OutFolder;

	// Make sure the output folder exists before creating assets inside it.
	UEditorAssetLibrary::MakeDirectory(OutFolder);

	// Collect the material slot names of the mesh.
	TArray<FName> SlotNames;
	if (SkeletalMesh)
	{
		for (const FSkeletalMaterial& Material : SkeletalMesh->GetMaterials())
		{
			SlotNames.Add(Material.MaterialSlotName);
		}
	}
	else
	{
		for (const FStaticMaterial& Material : StaticMesh->GetStaticMaterials())
		{
			SlotNames.Add(Material.MaterialSlotName);
		}
	}

	Result.SlotCount = SlotNames.Num();
	if (SlotNames.Num() == 0)
	{
		Result.bSuccess = true;
		Result.LogLines.Add(TEXT("[Warning] The mesh has no material slots."));
		return Result;
	}

	Result.LogLines.Add(FString::Printf(
		TEXT("Mesh '%s' has %d material slot(s). Parent material: %s"),
		*MeshAsset->GetName(), SlotNames.Num(), *ParentMaterial->GetName()));

	FAssetToolsModule& AssetToolsModule = FModuleManager::LoadModuleChecked<FAssetToolsModule>(TEXT("AssetTools"));
	IAssetTools& AssetTools = AssetToolsModule.Get();

	// Working copy so all skeletal material changes can be written back in one call.
	TArray<FSkeletalMaterial> WorkingSkeletalMaterials;
	if (SkeletalMesh)
	{
		WorkingSkeletalMaterials = SkeletalMesh->GetMaterials();
	}

	TArray<TWeakObjectPtr<UObject>> CreatedInstances;

	for (int32 Index = 0; Index < SlotNames.Num(); ++Index)
	{
		const FString SlotName = SlotNames[Index].IsNone()
			? FString::Printf(TEXT("Slot%d"), Index)
			: SlotNames[Index].ToString();
		const FString InstanceName = MaterialInstanceTool::SanitizeAssetName(InstancePrefix + SlotName);
		const FString FullPath = FString::Printf(TEXT("%s/%s"), *OutFolder, *InstanceName);

		UMaterialInstanceConstant* Instance = nullptr;

		if (UEditorAssetLibrary::DoesAssetExist(FullPath))
		{
			if (bOverwriteExisting)
			{
				if (!UEditorAssetLibrary::DeleteAsset(FullPath))
				{
					Result.LogLines.Add(FString::Printf(TEXT("[Error] Could not delete existing asset '%s'."), *FullPath));
					continue;
				}
			}
			else
			{
				Instance = Cast<UMaterialInstanceConstant>(UEditorAssetLibrary::LoadAsset(FullPath));
				if (Instance)
				{
					++Result.ReusedCount;
					Result.LogLines.Add(FString::Printf(TEXT("[Reuse]  Slot '%s' -> %s"), *SlotName, *InstanceName));
				}
			}
		}

		if (!Instance)
		{
			UMaterialInstanceConstantFactoryNew* Factory = NewObject<UMaterialInstanceConstantFactoryNew>();
			Factory->InitialParent = ParentMaterial;

			Instance = Cast<UMaterialInstanceConstant>(AssetTools.CreateAsset(
				InstanceName,
				OutFolder,
				UMaterialInstanceConstant::StaticClass(),
				Factory));

			if (!Instance)
			{
				Result.LogLines.Add(FString::Printf(TEXT("[Error] Could not create '%s'."), *FullPath));
				continue;
			}

			++Result.CreatedCount;
			CreatedInstances.Add(Instance);
			Result.LogLines.Add(FString::Printf(TEXT("[Create] Slot '%s' -> %s"), *SlotName, *InstanceName));
		}

		// Assign the instance back to the mesh slot.
		if (SkeletalMesh)
		{
			WorkingSkeletalMaterials[Index].MaterialInterface = Instance;
		}
		else
		{
			StaticMesh->SetMaterial(Index, Instance);
		}
	}

	// Write the skeletal material array back in one go.
	if (SkeletalMesh)
	{
		SkeletalMesh->SetMaterials(WorkingSkeletalMaterials);
	}

	// Save every newly created instance plus the mesh itself.
	TArray<UObject*> AssetsToSave;
	AssetsToSave.Reserve(CreatedInstances.Num() + 1);
	for (const TWeakObjectPtr<UObject>& Instance : CreatedInstances)
	{
		if (Instance.IsValid())
		{
			AssetsToSave.Add(Instance.Get());
		}
	}
	AssetsToSave.Add(MeshAsset);
	UEditorAssetLibrary::SaveLoadedAssets(AssetsToSave, false);

	Result.bSuccess = true;
	Result.LogLines.Add(FString::Printf(
		TEXT("Done. Created %d, reused %d, out of %d slot(s). Output folder: %s"),
		Result.CreatedCount, Result.ReusedCount, Result.SlotCount, *OutFolder));

	return Result;
}
