
#pragma once
#include "CoreMinimal.h"
#include "CommonENUMS.generated.h"

UENUM(BlueprintType)
enum class EResourceTypes : uint8
{
	Mana UMETA(DisplayName = "Mana"),
	ManaStone UMETA(DisplayName = "ManaStone")

};



UENUM(BlueprintType)
enum class EPawnType : uint8
{
	Krag UMETA(DisplayName = "Krag"),
	Mauler UMETA(DisplayName = "Mauler")

};