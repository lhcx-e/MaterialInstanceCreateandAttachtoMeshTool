// Copyright Epic Games, Inc. All Rights Reserved.

#include "MaterialInstanceToolStyle.h"

#include "Interfaces/IPluginManager.h"
#include "Styling/SlateStyleMacros.h"
#include "Styling/SlateStyleRegistry.h"

#define RootToContentDir Style->RootToContentDir

TSharedPtr<FSlateStyleSet> FMaterialInstanceToolStyle::StyleInstance = nullptr;

void FMaterialInstanceToolStyle::Initialize()
{
	if (!StyleInstance.IsValid())
	{
		StyleInstance = Create();
		FSlateStyleRegistry::RegisterSlateStyle(*StyleInstance);
	}
}

void FMaterialInstanceToolStyle::Shutdown()
{
	if (StyleInstance.IsValid())
	{
		FSlateStyleRegistry::UnRegisterSlateStyle(*StyleInstance);
		ensure(StyleInstance.IsUnique());
		StyleInstance.Reset();
	}
}

const ISlateStyle& FMaterialInstanceToolStyle::Get()
{
	check(StyleInstance.IsValid());
	return *StyleInstance;
}

FName FMaterialInstanceToolStyle::GetStyleSetName()
{
	static const FName StyleSetName(TEXT("MaterialInstanceToolStyle"));
	return StyleSetName;
}

TSharedRef<FSlateStyleSet> FMaterialInstanceToolStyle::Create()
{
	TSharedRef<FSlateStyleSet> Style = MakeShareable(new FSlateStyleSet(GetStyleSetName()));

	if (const TSharedPtr<IPlugin> Plugin = IPluginManager::Get().FindPlugin(TEXT("MaterialInstanceTool")))
	{
		Style->SetContentRoot(Plugin->GetBaseDir() / TEXT("Resources"));
	}

	Style->Set(TEXT("MaterialInstanceTool.Icon"), new IMAGE_BRUSH(TEXT("Icon64"), FVector2D(64.0f, 64.0f)));

	return Style;
}

#undef RootToContentDir
