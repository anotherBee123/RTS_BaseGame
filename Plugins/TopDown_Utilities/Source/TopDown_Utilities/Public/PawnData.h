// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CommonENUMS.h"
#include "PawnData.generated.h"

/**
 * 
 */
USTRUCT(BlueprintType)
struct FPawnData : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource")
	EPawnType PawnType;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource")
	FString DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource")
	TSoftObjectPtr<UTexture2D> Texture;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource")
	int32 BaseHealth;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource")
	int32 DPS;

	FPawnData() : PawnType(EPawnType::Krag), DisplayName(TEXT("")), Texture(nullptr), BaseHealth(100), DPS(10) {}

	FPawnData(EPawnType Type, FString Name, UTexture2D* Tex, int32 Default, int32 InDPS)
		: PawnType(Type), DisplayName(Name), Texture(Tex), BaseHealth(Default), DPS(InDPS){
		
		}



};
