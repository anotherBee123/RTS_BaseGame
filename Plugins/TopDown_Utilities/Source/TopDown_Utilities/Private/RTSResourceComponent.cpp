#include "RTSResourceComponent.h"

URTSResourceComponent::URTSResourceComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

int32 URTSResourceComponent::GetMana() const
{
	return Mana;
}

void URTSResourceComponent::AddMana(int32 Amount)
{
	if (Amount == 0)
	{
		return;
	}

	const int32 OldMana = Mana;
	Mana = FMath::Max(0, Mana + Amount);
	OnManaChanged.Broadcast(Mana, Mana - OldMana);
}

bool URTSResourceComponent::SpendMana(int32 Amount)
{
	if (Amount <= 0)
	{
		return true;
	}

	if (Mana < Amount)
	{
		return false;
	}

	const int32 OldMana = Mana;
	Mana -= Amount;
	OnManaChanged.Broadcast(Mana, Mana - OldMana);
	return true;
}

void URTSResourceComponent::SetMana(int32 NewMana)
{
	const int32 OldMana = Mana;
	Mana = FMath::Max(0, NewMana);
	OnManaChanged.Broadcast(Mana, Mana - OldMana);
}
