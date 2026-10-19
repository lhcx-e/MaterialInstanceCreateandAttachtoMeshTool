// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

class UMaterialInterface;
class UObject;

/** Result of a batch material-instance generation run. */
struct FMaterialInstanceGeneratorResult
{
	bool bSuccess = false;
	int32 CreatedCount = 0;
	int32 ReusedCount = 0;
	int32 SlotCount = 0;
	FString OutputFolder;
	TArray<FString> LogLines;
};

/**
 * Core logic: for every material slot of a mesh asset, create (or reuse) a
 * material instance parented to the chosen parent material, named after the
 * slot, and assign it back to the mesh.
 *
 * Instances are always created in the same folder as the mesh itself.
 */
class FMaterialInstanceGenerator
{
public:
	/**
	 * @param ParentMaterial       The parent material used for every created instance.
	 * @param MeshAsset            A UStaticMesh or USkeletalMesh.
	 * @param InstancePrefix       Prefix for instance names, e.g. MI_ (slot "Face" -> MI_Face)
	 * @param bOverwriteExisting   Delete and recreate instances that already exist.
	 */
	static FMaterialInstanceGeneratorResult Generate(
		UMaterialInterface* ParentMaterial,
		UObject* MeshAsset,
		const FString& InstancePrefix,
		bool bOverwriteExisting);
};
