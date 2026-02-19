// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "SelectableInterface.h"
#include "GameFramework/Actor.h"
#include "FactionInterface.h"
#include "CLMBaseBuilding.generated.h"

class UBoxComponent;
class UStaticMeshComponent;
class UMaterialInterface;
class ATopDownPlayerController;

UCLASS()
class TOPDOWN_UTILITIES_API ACLMBaseBuilding : public AActor, public ISelectableInterface, public IFactionInterface
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ACLMBaseBuilding();

private:

	//mesh component
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Building, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UStaticMeshComponent> BuildingMesh;

	//selected indicator
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Building, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UStaticMeshComponent> SelectedIndicator;

	//placement bounds
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Building, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UBoxComponent> PlacementBounds;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Pawn, meta = (AllowPrivateAccess="true"))
	int32 FactionID = 1;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Building, meta = (AllowPrivateAccess = "true", ClampMin = "0"))
	int32 BuildCostMana = 0;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Building, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UMaterialInterface> PreviewValidMaterial;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Building, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UMaterialInterface> PreviewInvalidMaterial;

	UPROPERTY()
	TWeakObjectPtr<ATopDownPlayerController> OwningTopDownController;

	bool bIsPreview = false;
	bool bPlacementValid = false;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	//implementing selectable interface
	void SelectActor_Implementation(const bool Select) override;

	UFUNCTION(BlueprintCallable, Category = "Building")
	void SetPreview(bool bPreview);

	UFUNCTION(BlueprintCallable, Category = "Building")
	void SetPlacementValid(bool bValid);

	UFUNCTION(BlueprintCallable, Category = "Building")
	FVector GetPlacementBoundsExtent() const;

	UFUNCTION(BlueprintCallable, Category = "Building")
	int32 GetBuildCostMana() const;

	UFUNCTION(BlueprintCallable, Category = "Building")
	void SetOwningTopDownController(ATopDownPlayerController* InController);

	UFUNCTION(BlueprintCallable, Category = "Building")
	ATopDownPlayerController* GetOwningTopDownController() const;

	void SetFaction_Implementation(int32 NewFaction) override;

	int32 GetFaction_Implementation() override;

};
