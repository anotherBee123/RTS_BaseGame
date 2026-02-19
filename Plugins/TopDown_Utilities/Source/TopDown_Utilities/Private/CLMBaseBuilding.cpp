// Fill out your copyright notice in the Description page of Project Settings.


#include "CLMBaseBuilding.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "TopDownPlayerController.h"

// Sets default values
ACLMBaseBuilding::ACLMBaseBuilding()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	//create mesh component
	BuildingMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BuildingMesh"));
	RootComponent = BuildingMesh;

	//placement bounds
	PlacementBounds = CreateDefaultSubobject<UBoxComponent>(TEXT("PlacementBounds"));
	PlacementBounds->SetupAttachment(RootComponent);
	PlacementBounds->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	//selected indicator
	SelectedIndicator  = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("SelectedIndicator"));
	SelectedIndicator->SetupAttachment(RootComponent);
	SelectedIndicator->SetHiddenInGame(true);
	SelectedIndicator->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

// Called when the game starts or when spawned
void ACLMBaseBuilding::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ACLMBaseBuilding::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ACLMBaseBuilding::SelectActor_Implementation(const bool Select)
{
	SelectedIndicator->SetHiddenInGame(!Select);
}

void ACLMBaseBuilding::SetPreview(bool bPreview)
{
	bIsPreview = bPreview;
	SetActorEnableCollision(!bPreview);
	SetPlacementValid(bPlacementValid);
}

void ACLMBaseBuilding::SetPlacementValid(bool bValid)
{
	bPlacementValid = bValid;
	if (!bIsPreview)
	{
		return;
	}

	if (BuildingMesh)
	{
		if (bValid && PreviewValidMaterial)
		{
			BuildingMesh->SetMaterial(0, PreviewValidMaterial);
		}
		else if (!bValid && PreviewInvalidMaterial)
		{
			BuildingMesh->SetMaterial(0, PreviewInvalidMaterial);
		}
		BuildingMesh->SetRenderCustomDepth(true);
		BuildingMesh->SetCustomDepthStencilValue(bValid ? 1 : 2);
	}
}

FVector ACLMBaseBuilding::GetPlacementBoundsExtent() const
{
	return PlacementBounds ? PlacementBounds->GetScaledBoxExtent() : FVector(100.0f, 100.0f, 100.0f);
}

int32 ACLMBaseBuilding::GetBuildCostMana() const
{
	return BuildCostMana;
}

void ACLMBaseBuilding::SetOwningTopDownController(ATopDownPlayerController* InController)
{
	OwningTopDownController = InController;
}

ATopDownPlayerController* ACLMBaseBuilding::GetOwningTopDownController() const
{
	return OwningTopDownController.Get();
}

void ACLMBaseBuilding::SetFaction_Implementation(int32 NewFaction)
{
	FactionID = NewFaction;
}

int32 ACLMBaseBuilding::GetFaction_Implementation()
{
    return FactionID;
}
