// Fill out your copyright notice in the Description page of Project Settings.

#include "CLMBasePawn.h"
#include "Components/SceneComponent.h"
#include "Engine/CollisionProfile.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "GameFramework/FloatingPawnMovement.h"
#include "AIController.h"


// Sets default values
ACLMBasePawn::ACLMBasePawn()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	//create capsule	
	CapsuleComponent = CreateDefaultSubobject<UCapsuleComponent>(TEXT("Capsule Component"));
	RootComponent = CapsuleComponent;
	CapsuleComponent->SetCollisionProfileName(UCollisionProfile::Pawn_ProfileName);

	//create skeletal mesh
	SkeletalMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("SkeletalMesh"));
	SkeletalMesh->SetupAttachment(RootComponent);

	//floating point movment
	FloatingPawnMovement = CreateDefaultSubobject<UFloatingPawnMovement>(TEXT("FloatingPawnMovement"));


	//create selected indicator
	SelectedIndicator = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("SelectedIndicator"));
	SelectedIndicator->SetupAttachment(RootComponent);
	SelectedIndicator->SetHiddenInGame(true);
	SelectedIndicator->SetCollisionEnabled(ECollisionEnabled::NoCollision);






}

// Called when the game starts or when spawned
void ACLMBasePawn::BeginPlay()
{
	Super::BeginPlay();
	
}


void ACLMBasePawn::OrientPawnToMovementDirection()
{
	//early return if not moving
	if (!bMoving)
	{
		return;
	}	

	FVector MoveDirection = (MoveTargetLocation - GetActorLocation());

	//if reached destination
	if (MoveDirection.Length()<AcceptanceDistance)
	{
		bMoving = false;
		return;
	}


	MoveDirection.Normalize(1);

	//AddMovementInput(MoveDirection, 1.f);
	FRotator IntermediateRotation = FRotator::ZeroRotator;

	FRotator DesiredRotation = UKismetMathLibrary::MakeRotFromX(MoveDirection);
	
	IntermediateRotation.Roll = 0;
	IntermediateRotation.Pitch = 0;
	IntermediateRotation.Yaw = DesiredRotation.Yaw + FixRotation;
	
	

	FRotator NewRotation = FMath::RInterpTo(GetActorRotation(), IntermediateRotation, GetWorld()->GetDeltaSeconds(), CharacterTurnSpeed);
	
	SetActorRotation(NewRotation);




}


// Called every frame
void ACLMBasePawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	OrientPawnToMovementDirection();

}

// Called to bind functionality to input
void ACLMBasePawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}



void ACLMBasePawn::SelectActorLocal(const bool select)
{
	SelectedIndicator->SetHiddenInGame(!select);
}

void ACLMBasePawn::SelectActor_Implementation(const bool Select)
{
	SelectedIndicator->SetHiddenInGame(!Select);
}

void ACLMBasePawn::MoveToLocation_Implementation(const FVector TargetLocation)
{
	UE_LOG(LogTemp, Display, TEXT("Navigating......."));
	MoveTargetLocation = TargetLocation+ FVector(0, 0, GetDefaultHalfHeight());
	
	//SetActorLocation(MoveTargetLocation);

	bMoving = true;

	AAIController* PawnAIController = Cast<AAIController>(GetController());
	PawnAIController->MoveToLocation(TargetLocation, AcceptanceDistance);


}

EPawnType ACLMBasePawn::GetPawnType_Implementation()
{
    return PawnType;
}

void ACLMBasePawn::SetFaction_Implementation(int32 NewFaction)
{
	FactionID = NewFaction;
}

int32 ACLMBasePawn::GetFaction_Implementation()
{
    return FactionID;
}
