#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "CommonENUMS.h"
#include "UnitProductionData.generated.h"

class UTexture2D;

USTRUCT(BlueprintType)
struct FUnitProductionData : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
	EPawnType UnitType = EPawnType::Krag;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
	TSubclassOf<APawn> PawnClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
	TSoftObjectPtr<UTexture2D> Icon;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
	int32 ManaCost = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
	float BuildTime = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
	TArray<int32> AllowedFactions;
};

USTRUCT(BlueprintType)
struct FQueuedUnitProduction
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
	EPawnType UnitType = EPawnType::Krag;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
	float RemainingTime = 0.0f;
};
