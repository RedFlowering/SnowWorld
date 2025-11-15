// Copyright 2022 HGsofts, Ltd. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "NumberRendererDefine.h"
#include "UObject/NoExportTypes.h"
#include "Engine/DeveloperSettings.h"
#include "NumberRendererSettings.generated.h"

class ANumberRenderActor;

USTRUCT()
struct FNumberFontTypeName
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY()
	TEnumAsByte<enum ENumberFontType> Type;

	UPROPERTY()
	FName Name;

	FNumberFontTypeName()
		: Type(NumberFontType_Max)
	{
	}
	FNumberFontTypeName(ENumberFontType InType, const FName& InName)
		: Type(InType)
		, Name(InName)
	{
	}
};

UCLASS(config = NumberRendererSettings, defaultconfig)
class NUMBERRENDERER_API UNumberRendererSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	static UNumberRendererSettings* Get();

public:
	UNumberRendererSettings();

	void UpdateSettings();

	virtual void PostInitProperties() override;

#if WITH_EDITOR
	virtual bool CanEditChange(const FProperty* Property) const override;
	virtual void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent) override;

	/** this changes displayname meta data. That means we won't need it outside of editor*/
	EPropertyChangeType::Type LoadTypes();

#endif // WITH_EDITOR

public:
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = NumberFontType)
	TMap< TEnumAsByte<enum ENumberFontType>, TSoftClassPtr<ANumberRenderActor> > NumberRenderActorClasses;

	UPROPERTY(config, EditAnywhere, Category = NumberFontType)
	TArray<FNumberFontTypeName> NumberFontTypes;
};
