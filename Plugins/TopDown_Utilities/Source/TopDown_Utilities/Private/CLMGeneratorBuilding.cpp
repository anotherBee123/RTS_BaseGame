#include "CLMGeneratorBuilding.h"
#include "RTSResourceComponent.h"
#include "TopDownPlayerController.h"

ACLMGeneratorBuilding::ACLMGeneratorBuilding()
{
	PrimaryActorTick.bCanEverTick = false;
}

void ACLMGeneratorBuilding::BeginPlay()
{
	Super::BeginPlay();

	if (ManaTickInterval > 0.0f)
	{
		GetWorld()->GetTimerManager().SetTimer(TimerHandleGenerateMana, this, &ACLMGeneratorBuilding::GenerateMana, ManaTickInterval, true);
	}
}

void ACLMGeneratorBuilding::GenerateMana()
{
	ATopDownPlayerController* OwningController = GetOwningTopDownController();
	if (!OwningController)
	{
		OwningController = Cast<ATopDownPlayerController>(GetWorld()->GetFirstPlayerController());
	}

	if (OwningController && OwningController->GetResourceComponent())
	{
		OwningController->GetResourceComponent()->AddMana(ManaPerTick);
	}
}
