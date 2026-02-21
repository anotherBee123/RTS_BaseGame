// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "FactionInterface.h"
#include "BuildingData.h"
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
class UDataTable;

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

	UFUNCTION(BlueprintCallable, Category = "Building")
	TArray<FBuildingData> GetAvailableBuildingsForFaction() const;

	UFUNCTION(BlueprintCallable, Category = "Building")
	bool StartPlacingBuildingById(FName BuildingId);

	UFUNCTION(BlueprintCallable, Category = "Building")
	bool StartPlacingBuildingBySlot(int32 SlotIndex);

	UFUNCTION(BlueprintCallable, Category = "Building")
	void ToggleBuildingMenu();

	UFUNCTION(BlueprintCallable, Category = "Building")
	bool IsBuildingMenuVisible() const;

	UFUNCTION(BlueprintCallable, Category = "Building")
	FName GetPendingBuildingId() const;

	UFUNCTION(BlueprintCallable, Category = "Resources")
	int32 GetCurrentMana() const;

	UPROPERTY(BlueprintAssignable, Category = "Delegates")
	FOnBuildingPlaced OnBuildingPlaced;

	UPROPERTY(BlueprintAssignable, Category = "Delegates")
	FOnActorsSelectedDelegate OnActorsSelected;

	UFUNCTION(BlueprintCallable, Category = "Selection")
	TArray<AActor*> GetSelectedActorsForUI() const;

	UFUNCTION(BlueprintCallable, Category = "Selection")
	AActor* GetPrimarySelectedActor() const;

private:

	//select action input
	UPROPERTY(EditDefaultsOnly, Category = "Input", meta = (AllowPrivateAccess="true"))
	TObjectPtr<UInputAction> SelectAction;

	//command action
	UPROPERTY(EditDefaultsOnly, Category = "Input", meta = (AllowPrivateAccess="true"))
	TObjectPtr<UInputAction> CommandAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Command", meta = (AllowPrivateAccess="true", ClampMin="0.0"))
	float FormationDragMinLength = 150.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Command", meta = (AllowPrivateAccess="true"))
	FColor FormationDebugLineColor = FColor::Cyan;

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

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Building", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UDataTable> BuildingDataTable;

	UPROPERTY()
	TObjectPtr<ACLMBaseBuilding> PreviewBuilding;

	UPROPERTY()
	TSubclassOf<ACLMBaseBuilding> PendingBuildingClass;

	UPROPERTY()
	FName PendingBuildingId;

	bool bIsPlacingBuilding = false;
	bool bLastPlacementValid = false;
	bool bShowBuildingMenu = true;

	//box select

	FVector2D SelectionStartPosition;
	FVector2D SelectionSize;
	bool bIsSelectionDragActive = false;
	float SelectionDragThreshold = 8.0f;
	TArray<AActor*> SelectedActors;

	bool bIsCommandDragActive = false;
	bool bHasCommandDragHit = false;
	FVector CommandDragStartLocation = FVector::ZeroVector;
	FVector CommandDragCurrentLocation = FVector::ZeroVector;
	TArray<TWeakObjectPtr<AActor>> CommandDragActors;
	TArray<FVector> CommandDragDestinations;

	//end


protected: 

	virtual void BeginPlay() override;

	virtual void SetupInputComponent() override;

	virtual void Tick(float DeltaSeconds) override;

	void Select(const FInputActionValue& Value);

	void CommandSelectedActors(const FInputActionValue& Value);
	void StartCommandDrag(const FInputActionValue& Value);
	void UpdateCommandDrag(const FInputActionValue& Value);
	void CancelCommandDrag(const FInputActionValue& Value);
	void ClearCommandDrag();
	bool TryGetCommandHit(FHitResult& OutHitResult);
	void GatherCommandableSelectedActors(TArray<AActor*>& OutActors) const;
	void BuildLineFormationDestinations(const FVector& StartLocation, const FVector& EndLocation, int32 Count, TArray<FVector>& OutDestinations) const;
	void BuildFormationAssignmentByProximity(TArray<int32>& OutDestinationIndexPerActor) const;
	void DrawFormationPreviewLines() const;
	void IssueFormationMoveCommand();
	void IssueLegacyMoveCommand(const FVector& Destination);

	bool UpdateBuildingPreview();
	bool IsPlacementValid(const FHitResult& HitResult) const;
	void TrySelectBuildingSlot(int32 SlotIndex);
	void SelectBuildingSlot1();
	void SelectBuildingSlot2();
	void SelectBuildingSlot3();
	void SelectBuildingSlot4();
	void SelectBuildingSlot5();
	void SelectBuildingSlot6();
	void SelectBuildingSlot7();
	void SelectBuildingSlot8();
	void SelectBuildingSlot9();
	void CancelBuildingPlacementInput();

	//box select
	void SelectStart(const FInputActionValue& Value);
	void SelectOnGoing(const FInputActionValue& Value);
	void SelectEnd(const FInputActionValue& Value);
	void SelectMultipleActors();
	//end

	void SetFaction_Implementation(int32 NewFaction) override;

	int32 GetFaction_Implementation() override;

};
