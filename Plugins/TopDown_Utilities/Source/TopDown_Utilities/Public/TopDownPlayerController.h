// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "FactionInterface.h"
#include "TopDownPlayerController.generated.h"

/**
 * 
 */


class UInputMappingContext;
class UInputAction;
struct FInputActionValue;
class ACLMBasePawn;
class ATopDownHUD;
class ACLMBaseBuilding;
class URTSResourceComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnActorsSelectedDelegate, const TArray<AActor*>&, SelectedActors);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnBuildingPlaced, ACLMBaseBuilding*, Building);


UCLASS()
class TOPDOWN_UTILITIES_API ATopDownPlayerController : public APlayerController, public IFactionInterface
{
	GENERATED_BODY()

public:

	ATopDownPlayerController();

	UPROPERTY(EditDefaultsOnly, Category = "Input", meta = (AllowPrivateAccess="true"))
	UInputMappingContext* DefaultInputMappingContext;

	UFUNCTION(BlueprintCallable, Category = "Resources")
	URTSResourceComponent* GetResourceComponent() const;

	UFUNCTION(BlueprintCallable, Category = "Building")
	void StartPlacingBuilding(TSubclassOf<ACLMBaseBuilding> BuildingClass);

	UFUNCTION(BlueprintCallable, Category = "Building")
	void ConfirmBuildingPlacement();

	UFUNCTION(BlueprintCallable, Category = "Building")
	void CancelBuildingPlacement();

	UFUNCTION(BlueprintCallable, Category = "Building")
	bool IsPlacingBuilding() const;

	UPROPERTY(BlueprintAssignable, Category = "Delegates")
	FOnBuildingPlaced OnBuildingPlaced;

private:

	//select action input
	UPROPERTY(EditDefaultsOnly, Category = "Input", meta = (AllowPrivateAccess="true"))
	TObjectPtr<UInputAction> SelectAction;

	UPROPERTY(BlueprintAssignable, Category = "Delegates")
	FOnActorsSelectedDelegate OnActorsSelected;

	//command action
	UPROPERTY(EditDefaultsOnly, Category = "Input", meta = (AllowPrivateAccess="true"))
	TObjectPtr<UInputAction> CommandAction;

	UPROPERTY()
	TObjectPtr<AActor> SelectedActor;

	UPROPERTY()
	TObjectPtr<ATopDownHUD> TopDownHUD;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Pawn, meta = (AllowPrivateAccess="true"))
	int32 FactionID = 1;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Resources", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<URTSResourceComponent> ResourceComponent;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Building", meta = (AllowPrivateAccess = "true", ClampMin = "0.0"))
	float BuildRange = 4000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Building", meta = (AllowPrivateAccess = "true", ClampMin = "0.0", ClampMax = "1.0"))
	float MinSurfaceNormalZ = 0.7f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Building", meta = (AllowPrivateAccess = "true"))
	TEnumAsByte<ECollisionChannel> BuildTraceChannel = ECollisionChannel::ECC_Visibility;

	UPROPERTY()
	TObjectPtr<ACLMBaseBuilding> PreviewBuilding;

	UPROPERTY()
	TSubclassOf<ACLMBaseBuilding> PendingBuildingClass;

	bool bIsPlacingBuilding = false;
	bool bLastPlacementValid = false;

	//box select

	FVector2D SelectionStartPosition;
	FVector2D SelectionSize;
	TArray<AActor*> SelectedActors;

	//end


protected: 

	virtual void BeginPlay() override;

	virtual void SetupInputComponent() override;

	virtual void Tick(float DeltaSeconds) override;

	void Select(const FInputActionValue& Value);

	void CommandSelectedActors(const FInputActionValue& Value);

	bool UpdateBuildingPreview();
	bool IsPlacementValid(const FHitResult& HitResult) const;

	//box select
	void SelectStart(const FInputActionValue& Value);
	void SelectOnGoing(const FInputActionValue& Value);
	void SelectEnd(const FInputActionValue& Value);
	void SelectMultipleActors();
	//end

	void SetFaction_Implementation(int32 NewFaction) override;

	int32 GetFaction_Implementation() override;

};
