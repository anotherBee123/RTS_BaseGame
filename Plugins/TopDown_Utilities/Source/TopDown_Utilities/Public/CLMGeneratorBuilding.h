#pragma once

#include "CoreMinimal.h"
#include "CLMBaseBuilding.h"
#include "CLMGeneratorBuilding.generated.h"

UCLASS()
class TOPDOWN_UTILITIES_API ACLMGeneratorBuilding : public ACLMBaseBuilding
{
	GENERATED_BODY()

public:
	ACLMGeneratorBuilding();

protected:
	virtual void BeginPlay() override;

	void GenerateMana();

	UPROPERTY(EditDefaultsOnly, Category = "Generator", meta = (ClampMin = "0"))
	int32 ManaPerTick = 2;

	UPROPERTY(EditDefaultsOnly, Category = "Generator", meta = (ClampMin = "0.1"))
	float ManaTickInterval = 1.0f;

	FTimerHandle TimerHandleGenerateMana;
};
