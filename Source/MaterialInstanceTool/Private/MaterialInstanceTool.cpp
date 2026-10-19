// Copyright Epic Games, Inc. All Rights Reserved.

#include "MaterialInstanceTool.h"

#include "ContentBrowserMenuContexts.h"
#include "Framework/Application/SlateApplication.h"
#include "Framework/Docking/TabManager.h"
#include "ToolMenus.h"
#include "Widgets/Docking/SDockTab.h"

#include "Engine/SkeletalMesh.h"
#include "Engine/StaticMesh.h"
#include "Modules/ModuleManager.h"

#include "MaterialInstanceToolStyle.h"
#include "SMaterialInstanceToolWidget.h"

#define LOCTEXT_NAMESPACE "MaterialInstanceTool"

static const FName MaterialInstanceToolTabId(TEXT("MaterialInstanceTool"));

void FMaterialInstanceToolModule::StartupModule()
{
	FMaterialInstanceToolStyle::Initialize();

	FGlobalTabmanager::Get()->RegisterNomadTabSpawner(
		MaterialInstanceToolTabId,
		FOnSpawnTab::CreateRaw(this, &FMaterialInstanceToolModule::OnSpawnPluginTab))
		.SetDisplayName(LOCTEXT("MaterialInstanceToolTabTitle", "Material Instance Tool"))
		.SetMenuType(ETabSpawnerMenuType::Enabled);

	MenuStartupCallbackHandle = UToolMenus::RegisterStartupCallback(
		FSimpleMulticastDelegate::FDelegate::CreateRaw(this, &FMaterialInstanceToolModule::RegisterMenus));
}

void FMaterialInstanceToolModule::ShutdownModule()
{
	if (UToolMenus::TryGet())
	{
		UToolMenus::UnRegisterStartupCallback(MenuStartupCallbackHandle);
	}

	if (FSlateApplication::IsInitialized())
	{
		FGlobalTabmanager::Get()->UnregisterNomadTabSpawner(MaterialInstanceToolTabId);
	}

	FMaterialInstanceToolStyle::Shutdown();
}

void FMaterialInstanceToolModule::RegisterMenus()
{
	FToolMenuOwnerScoped OwnerScoped(this);

	if (UToolMenu* Menu = UToolMenus::Get()->ExtendMenu(TEXT("LevelEditor.MainMenu.Tools")))
	{
		FToolMenuSection& Section = Menu->FindOrAddSection(TEXT("MaterialInstanceTool"));
		Section.AddMenuEntry(
			TEXT("OpenMaterialInstanceTool"),
			LOCTEXT("OpenMaterialInstanceToolLabel", "Material Instance Tool"),
			LOCTEXT("OpenMaterialInstanceToolTip", "Batch create named material instances for every material slot of a mesh."),
			FSlateIcon(FMaterialInstanceToolStyle::GetStyleSetName(), TEXT("MaterialInstanceTool.Icon")),
			FUIAction(FExecuteAction::CreateLambda([]()
			{
				FGlobalTabmanager::Get()->TryInvokeTab(MaterialInstanceToolTabId);
			}))
		);
	}

	if (UToolMenu* ToolBar = UToolMenus::Get()->ExtendMenu(TEXT("LevelEditor.LevelEditorToolBar.User")))
	{
		FToolMenuSection& Section = ToolBar->FindOrAddSection(TEXT("MaterialInstanceTool"));
		Section.AddEntry(FToolMenuEntry::InitToolBarButton(
			TEXT("OpenMaterialInstanceTool"),
			FUIAction(FExecuteAction::CreateLambda([]()
			{
				FGlobalTabmanager::Get()->TryInvokeTab(MaterialInstanceToolTabId);
			})),
			LOCTEXT("ToolbarButtonLabel", "Material Instance Tool"),
			LOCTEXT("ToolbarButtonTip", "Batch create named material instances for every material slot of a mesh."),
			FSlateIcon(FMaterialInstanceToolStyle::GetStyleSetName(), TEXT("MaterialInstanceTool.Icon"))
		));
	}

	RegisterAssetContextMenus();
}

void FMaterialInstanceToolModule::RegisterAssetContextMenus()
{
	FToolMenuOwnerScoped OwnerScoped(this);

	// Right-click entry on Skeletal Mesh and Static Mesh assets in the Content Browser.
	static const TCHAR* ContextMenuNames[] =
	{
		TEXT("ContentBrowser.AssetContextMenu.SkeletalMesh"),
		TEXT("ContentBrowser.AssetContextMenu.StaticMesh"),
	};

	for (const TCHAR* ContextMenuName : ContextMenuNames)
	{
		UToolMenu* Menu = UToolMenus::Get()->ExtendMenu(ContextMenuName);
		if (!Menu)
		{
			continue;
		}

		FToolMenuSection& Section = Menu->FindOrAddSection(TEXT("MaterialInstanceTool"));
		Section.AddDynamicEntry(TEXT("CreateMaterialInstancesForMesh"),
			FNewToolMenuSectionDelegate::CreateLambda([](FToolMenuSection& InSection)
			{
				const UContentBrowserAssetContextMenuContext* Context =
					InSection.FindContext<UContentBrowserAssetContextMenuContext>();
				if (!Context)
				{
					return;
				}

				// Only offer the entry when the first selected asset is a mesh we support.
				FAssetData MeshAssetData;
				for (const FAssetData& AssetData : Context->SelectedAssets)
				{
					if (AssetData.IsInstanceOf(USkeletalMesh::StaticClass())
						|| AssetData.IsInstanceOf(UStaticMesh::StaticClass()))
					{
						MeshAssetData = AssetData;
						break;
					}
				}

				if (!MeshAssetData.IsValid())
				{
					return;
				}

				InSection.AddMenuEntry(
					TEXT("CreateMaterialInstancesForMeshEntry"),
					LOCTEXT("CreateMaterialInstancesLabel", "Create Material Instances..."),
					LOCTEXT("CreateMaterialInstancesTip",
						"Open the Material Instance Tool with this mesh selected. One material instance per material slot is created in the mesh's own folder."),
					FSlateIcon(FMaterialInstanceToolStyle::GetStyleSetName(), TEXT("MaterialInstanceTool.Icon")),
					FUIAction(FExecuteAction::CreateLambda([MeshAssetData]()
					{
						// Load the mesh on click (right-clicking alone should not load assets).
						UObject* Mesh = MeshAssetData.GetAsset();
						if (!Mesh)
						{
							return;
						}

						if (FModuleManager::Get().IsModuleLoaded(TEXT("MaterialInstanceTool")))
						{
							FModuleManager::GetModuleChecked<FMaterialInstanceToolModule>(
								TEXT("MaterialInstanceTool")).OpenToolWithMesh(Mesh);
						}
					}))
				);
			}));
	}
}

void FMaterialInstanceToolModule::OpenToolWithMesh(UObject* MeshAsset)
{
	PendingMesh = MeshAsset;

	// If the panel is already open, feed the mesh to it directly.
	if (ToolWidget.IsValid())
	{
		ToolWidget.Pin()->SetTargetMesh(MeshAsset);
	}

	// Opens the tab, or brings it to the front if it is already open.
	FGlobalTabmanager::Get()->TryInvokeTab(MaterialInstanceToolTabId);
}

TSharedRef<SDockTab> FMaterialInstanceToolModule::OnSpawnPluginTab(const FSpawnTabArgs& Args)
{
	TSharedRef<SMaterialInstanceToolWidget> Widget = SNew(SMaterialInstanceToolWidget);

	// Consume any mesh that was requested before the tab existed.
	if (PendingMesh.IsValid())
	{
		Widget->SetTargetMesh(PendingMesh.Get());
		PendingMesh.Reset();
	}

	ToolWidget = Widget;

	return SNew(SDockTab)
		.TabRole(ETabRole::NomadTab)
		[
			Widget
		];
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FMaterialInstanceToolModule, MaterialInstanceTool)
