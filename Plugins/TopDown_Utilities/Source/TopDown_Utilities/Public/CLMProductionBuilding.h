#pragma once

#include "CoreMinimal.h"
#include "CLMBaseBuilding.h"
#include "UnitProductionData.h"
#include "CLMProductionBuilding.generated.h"

class UArrowComponent;
class UDataTable;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnProductionQueueChanged, const TArray<FQueuedUnitProduction>&, NewQueue);

UCLASS()
class TOPDOWN_UTILITIES_API ACLMProductionBuilding : public ACLMBaseBuilding
{
	GENERATED_BODY()

public:
	ACLMProductionBuilding();

	UFUNCTION(BlueprintCallable, Category = "Production")
	TArray<FUnitProductionData> GetAvailableUnitsForFaction(int32 InFaction) const;

	UFUNCTION(BlueprintCallable, Category = "Production")
	bool QueueUnitProduction(EPawnType UnitType);

	UFUNCTION(BlueprintCallable, Category = "Production")
	const TArray<FQueuedUnitProduction>& GetProductionQueue() const;

	UPROPERTY(BlueprintAssignable, Category = "Production")
	FOnProductionQueueChanged OnProductionQueueChanged;

protected:
	virtual void Tick(float DeltaTime) override;
	virtual void BeginPlay() override;

	void StartNextProduction();
	void FinishCurrentProduction();

	const FUnitProductionData* FindUnitData(EPawnType UnitType) const;

	UPROPERTY(EditDefaultsOnly, Category = "Production")
	UDataTable* UnitProductionDataTable;

	UPROPERTY(EditDefaultsOnly, Category = "Production")
	float UnitSpawnSpacing = 120.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Production", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UArrowComponent> UnitSpawnPoint;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Production", meta = (AllowPrivateAccess = "true"))
	TArray<FQueuedUnitProduction> ProductionQueue;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Production", meta = (AllowPrivateAccess = "true"))
	bool bIsProducing = false;
};
