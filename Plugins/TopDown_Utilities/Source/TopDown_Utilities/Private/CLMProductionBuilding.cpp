#include "CLMProductionBuilding.h"
#include "Components/ArrowComponent.h"
#include "Engine/DataTable.h"
#include "RTSResourceComponent.h"
#include "TopDownPlayerController.h"
#include "FactionInterface.h"

ACLMProductionBuilding::ACLMProductionBuilding()
{
	PrimaryActorTick.bCanEverTick = true;

	UnitSpawnPoint = CreateDefaultSubobject<UArrowComponent>(TEXT("UnitSpawnPoint"));
	UnitSpawnPoint->SetupAttachment(RootComponent);
	UnitSpawnPoint->SetRelativeLocation(FVector(200.0f, 0.0f, 0.0f));
}

void ACLMProductionBuilding::BeginPlay()
{
	Super::BeginPlay();
}

const FUnitProductionData* ACLMProductionBuilding::FindUnitData(EPawnType UnitType) const
{
	if (!UnitProductionDataTable)
	{
		return nullptr;
	}

	static const FString Context(TEXT("UnitProductionData"));
	TArray<FUnitProductionData*> Rows;
	UnitProductionDataTable->GetAllRows(Context, Rows);
	for (const FUnitProductionData* Row : Rows)
	{
		if (Row && Row->UnitType == UnitType)
		{
			return Row;
		}
	}
	return nullptr;
}

TArray<FUnitProductionData> ACLMProductionBuilding::GetAvailableUnitsForFaction(int32 InFaction) const
{
	TArray<FUnitProductionData> Result;
	if (!UnitProductionDataTable)
	{
		return Result;
	}

	static const FString Context(TEXT("UnitProductionData"));
	TArray<FUnitProductionData*> Rows;
	UnitProductionDataTable->GetAllRows(Context, Rows);
	for (const FUnitProductionData* Row : Rows)
	{
		if (!Row)
		{
			continue;
		}
		if (Row->AllowedFactions.Num() > 0 && !Row->AllowedFactions.Contains(InFaction))
		{
			continue;
		}
		Result.Add(*Row);
	}
	return Result;
}

bool ACLMProductionBuilding::QueueUnitProduction(EPawnType UnitType)
{
	const FUnitProductionData* UnitData = FindUnitData(UnitType);
	if (!UnitData)
	{
		return false;
	}

    const int32 BuildingFaction = IFactionInterface::Execute_GetFaction(this);
    if (UnitData->AllowedFactions.Num() > 0 && !UnitData->AllowedFactions.Contains(BuildingFaction))
    {
        return false;
    }

	ATopDownPlayerController* OwningController = GetOwningTopDownController();
	if (!OwningController || !OwningController->GetResourceComponent())
	{
		return false;
	}

	if (!OwningController->GetResourceComponent()->SpendMana(UnitData->ManaCost))
	{
		return false;
	}

	FQueuedUnitProduction NewItem;
	NewItem.UnitType = UnitType;
	NewItem.RemainingTime = FMath::Max(0.1f, UnitData->BuildTime);
	ProductionQueue.Add(NewItem);
	OnProductionQueueChanged.Broadcast(ProductionQueue);

	if (!bIsProducing)
	{
		StartNextProduction();
	}

	return true;
}

const TArray<FQueuedUnitProduction>& ACLMProductionBuilding::GetProductionQueue() const
{
	return ProductionQueue;
}

void ACLMProductionBuilding::StartNextProduction()
{
	bIsProducing = ProductionQueue.Num() > 0;
}

void ACLMProductionBuilding::FinishCurrentProduction()
{
	if (ProductionQueue.Num() == 0)
	{
		bIsProducing = false;
		return;
	}

	const FQueuedUnitProduction Finished = ProductionQueue[0];
	ProductionQueue.RemoveAt(0);
	OnProductionQueueChanged.Broadcast(ProductionQueue);

	const FUnitProductionData* UnitData = FindUnitData(Finished.UnitType);
	if (UnitData && UnitData->PawnClass)
	{
		FVector SpawnLocation = UnitSpawnPoint ? UnitSpawnPoint->GetComponentLocation() : GetActorLocation();
		FRotator SpawnRotation = UnitSpawnPoint ? UnitSpawnPoint->GetComponentRotation() : GetActorRotation();
		SpawnLocation += SpawnRotation.Vector() * UnitSpawnSpacing;
		GetWorld()->SpawnActor<APawn>(UnitData->PawnClass, SpawnLocation, SpawnRotation);
	}

	bIsProducing = ProductionQueue.Num() > 0;
}

void ACLMProductionBuilding::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (ProductionQueue.Num() == 0)
	{
		bIsProducing = false;
		return;
	}

	bIsProducing = true;
	ProductionQueue[0].RemainingTime -= DeltaTime;
	if (ProductionQueue[0].RemainingTime <= 0.0f)
	{
		FinishCurrentProduction();
	}
}
