// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "SelectableInterface.h"
#include "NavigableInterface.h"
#include "BasePawnInterface.h"
#include "CommonENUMS.h"
#include "FactionInterface.h"
#include "CLMBasePawn.generated.h"


class UCapsuleComponent;
class USkeletalMeshComponent;
class UFloatingPawnMovement;

UCLASS()
class TOPDOWN_UTILITIES_API ACLMBasePawn : public APawn, public ISelectableInterface, public INavigableInterface, public IBasePawnInterface, public IFactionInterface
{
	GENERATED_BODY()

private:
	//capsule component
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Collision, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UCapsuleComponent> CapsuleComponent;

	//skeetal mesh
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Pawn, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USkeletalMeshComponent> SkeletalMesh;


	//floating pawn movement component
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Pawn, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UFloatingPawnMovement> FloatingPawnMovement;


	//cyclidner
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Pawn, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UStaticMeshComponent> SelectedIndicator;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Pawn, meta = (AllowPrivateAccess = "true"))
	EPawnType PawnType = EPawnType::Krag;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Pawn, meta = (AllowPrivateAccess = "true"))
	int32 FactionID = 1;




public:
	// Sets default values for this pawn's properties
	ACLMBasePawn();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	void OrientPawnToMovementDirection();



	//navigation
	FVector MoveTargetLocation = FVector::ZeroVector;
	bool bMoving = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Pawn, meta = (AllowPrivateAccess = "true"))
	float AcceptanceDistance = 50.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Pawn, meta = (AllowPrivateAccess = "true"))
	float CharacterTurnSpeed = 5.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Pawn, meta = (AllowPrivateAccess = "true"))
	float FixRotation = 45.f;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	UFUNCTION()
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	//select actor
	UFUNCTION()
	void SelectActorLocal(const bool Select);

	void SelectActor_Implementation(const bool Select) override;

	void MoveToLocation_Implementation(const FVector TargetLocation) override;

	EPawnType GetPawnType_Implementation() override;

	void SetFaction_Implementation(int32 NewFaction) override;

	int32 GetFaction_Implementation() override;

};
