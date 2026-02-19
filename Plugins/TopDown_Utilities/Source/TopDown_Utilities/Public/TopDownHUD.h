// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "FactionInterface.h"
#include "GameFramework/HUD.h"
#include "TopDownHUD.generated.h"

class ACLMBasePawn;
/**
 * 
 */
UCLASS()
class TOPDOWN_UTILITIES_API ATopDownHUD : public AHUD, public IFactionInterface
{
	GENERATED_BODY()


private:

	bool bDrawSelectionRect = false;
	bool bSelectActors = false;
	FVector2D SelectionRectStart;
	FVector2D SelectionRectSize;
	FLinearColor SelectionRectColor = FLinearColor(1.0f, 0.7732f, 0.74777f, 0.25f);

	TArray<ACLMBasePawn*> SelectedActors;

	void SelectActorsInRect();



protected:
	
	virtual void DrawHUD() override;

public:

	void ShowSelectionRect(const FVector2D InSelectionRectStart, const FVector2D InSelectionRectSize);
	void HideSelectionRect();


	TArray<ACLMBasePawn*> GetSelectedActors();
	
	
};
