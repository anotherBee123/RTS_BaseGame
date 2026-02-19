#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "BuildingData.generated.h"

class ACLMBaseBuilding;
class UTexture2D;

USTRUCT(BlueprintType)
struct FBuildingData : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Building")
	FName BuildingId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Building")
	TSubclassOf<ACLMBaseBuilding> BuildingClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Building")
	TSoftObjectPtr<UTexture2D> Icon;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Building")
	int32 ManaCost = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Building")
	float BuildTime = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Building")
	TArray<int32> AllowedFactions;
};
