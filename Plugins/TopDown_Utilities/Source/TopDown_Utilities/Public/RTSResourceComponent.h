#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "RTSResourceComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnManaChanged, int32, NewMana, int32, Delta);

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TOPDOWN_UTILITIES_API URTSResourceComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	URTSResourceComponent();

	UFUNCTION(BlueprintCallable, Category = "Resources")
	int32 GetMana() const;

	UFUNCTION(BlueprintCallable, Category = "Resources")
	void AddMana(int32 Amount);

	UFUNCTION(BlueprintCallable, Category = "Resources")
	bool SpendMana(int32 Amount);

	UFUNCTION(BlueprintCallable, Category = "Resources")
	void SetMana(int32 NewMana);

	UPROPERTY(BlueprintAssignable, Category = "Resources")
	FOnManaChanged OnManaChanged;

private:
	UPROPERTY(EditAnywhere, Category = "Resources", meta = (ClampMin = "0"))
	int32 Mana = 0;
};
