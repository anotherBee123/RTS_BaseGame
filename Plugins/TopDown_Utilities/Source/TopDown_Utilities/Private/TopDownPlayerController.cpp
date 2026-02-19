// Fill out your copyright notice in the Description page of Project Settings.
#include "TopDownPlayerController.h"
#include "InputMappingContext.h"
#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"
#include "InputActionValue.h"
#include "CLMBasePawn.h"
#include "CLMBaseBuilding.h"
#include "TopDownHUD.h"
#include "RTSResourceComponent.h"
#include "Engine/World.h"


ATopDownPlayerController::ATopDownPlayerController()
{
    bShowMouseCursor = true;
    PrimaryActorTick.bCanEverTick = true;

    ResourceComponent = CreateDefaultSubobject<URTSResourceComponent>(TEXT("ResourceComponent"));
}

URTSResourceComponent* ATopDownPlayerController::GetResourceComponent() const
{
    return ResourceComponent;
}

void ATopDownPlayerController::BeginPlay()
{
    Super::BeginPlay();

    TopDownHUD = Cast<ATopDownHUD>(GetHUD());
}

void ATopDownPlayerController::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);

    UpdateBuildingPreview();
}

void ATopDownPlayerController::SetupInputComponent()
{
    Super::SetupInputComponent();


    if (!DefaultInputMappingContext)
    {
        return;
    }

    UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer());

    if (Subsystem)
    {
        Subsystem->AddMappingContext(DefaultInputMappingContext, 0);
        UE_LOG(LogTemp, Display, TEXT("Input mapping context added."));
    }
    if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(InputComponent)){

		//bind select function to move input 
		EnhancedInputComponent->BindAction(SelectAction, ETriggerEvent::Completed, this, &ATopDownPlayerController::Select);


        //bind to rectangle select
        EnhancedInputComponent->BindAction(SelectAction, ETriggerEvent::Started, this, &ATopDownPlayerController::SelectStart);
        EnhancedInputComponent->BindAction(SelectAction, ETriggerEvent::Triggered, this, &ATopDownPlayerController::SelectOnGoing);
        EnhancedInputComponent->BindAction(SelectAction, ETriggerEvent::Completed, this, &ATopDownPlayerController::SelectEnd);

        //bind command action
        EnhancedInputComponent->BindAction(CommandAction, ETriggerEvent::Completed, this, &ATopDownPlayerController::CommandSelectedActors);

    }


}

void ATopDownPlayerController::Select(const FInputActionValue& Value)
{
    if (bIsPlacingBuilding)
    {
        ConfirmBuildingPlacement();
        return;
    }

    UE_LOG(LogTemp, Display, TEXT("SelectAction"));

    FHitResult HitResult;
    GetHitResultUnderCursor(ECollisionChannel::ECC_Camera, false, HitResult);

    if (SelectedActor)
    {
        if (SelectedActor->GetClass()->ImplementsInterface(USelectableInterface::StaticClass()))
        {
            ISelectableInterface::Execute_SelectActor(SelectedActor, false);

        }
    }

    SelectedActor = HitResult.GetActor();

    if (SelectedActor)
    {
        UE_LOG(LogTemp, Display, TEXT("Selected Actor: %s"), *SelectedActor->GetName());
        
        //deselect prvious actor
        



        //select new actor
        if (SelectedActor->GetClass()->ImplementsInterface(USelectableInterface::StaticClass()))
        {
            ISelectableInterface::Execute_SelectActor(SelectedActor, true);

        }
    }



}


void ATopDownPlayerController::CommandSelectedActors(const FInputActionValue& Value)
{
    if (bIsPlacingBuilding)
    {
        return;
    }
    
    
    FHitResult HitResult;
    GetHitResultUnderCursor(ECollisionChannel::ECC_Camera, false, HitResult);
            
    if(!HitResult.bBlockingHit)
    {
        return;
    }

    if (SelectedActors.Num()>0)
    {

        int i = SelectedActors.Num() / -2;
        for (AActor* SomeActor : SelectedActors)
        {
            if (SomeActor)
            {
                if (SomeActor->GetClass()->ImplementsInterface(UFactionInterface::StaticClass()))
                {
                    int ActorFaction = IFactionInterface::Execute_GetFaction(SomeActor);
                    if (FactionID != ActorFaction)
                    {
                        continue;
                    }
                }

                if (SomeActor->GetClass()->ImplementsInterface(UNavigableInterface::StaticClass()))
                {
                    INavigableInterface::Execute_MoveToLocation(SomeActor, HitResult.Location + FVector(0, 100*i, 0));
                    i++;

                }
            }
        }   
    } 
    else if (SelectedActor)
    {
        UE_LOG(LogTemp, Display, TEXT("Commaned Actor: %s"), *SelectedActor->GetName());

        bool bIsCorrectFaction = true;

        if (SelectedActor->GetClass()->ImplementsInterface(UFactionInterface::StaticClass()))
        {
            int ActorFaction = IFactionInterface::Execute_GetFaction(SelectedActor);
            if (FactionID != ActorFaction)
            {
                bIsCorrectFaction = false;
            }
        }
        if (bIsCorrectFaction && SelectedActor->GetClass()->ImplementsInterface(UNavigableInterface::StaticClass()))
        {
            INavigableInterface::Execute_MoveToLocation(SelectedActor, HitResult.Location);

        }


    }



}

void ATopDownPlayerController::SelectStart(const FInputActionValue &Value)
{
    if (bIsPlacingBuilding)
    {
        return;
    }

    float MouseX, MouseY;
    GetMousePosition(MouseX, MouseY);
    SelectionStartPosition = FVector2D(MouseX, MouseY);

    
}

void ATopDownPlayerController::SelectOnGoing(const FInputActionValue &Value)
{
    if (bIsPlacingBuilding)
    {
        return;
    }

    float MouseX, MouseY;
    GetMousePosition(MouseX, MouseY);
    SelectionSize = FVector2D(MouseX - SelectionStartPosition.X, MouseY - SelectionStartPosition.Y);
    if (TopDownHUD)
    {
        TopDownHUD->ShowSelectionRect(SelectionStartPosition, SelectionSize);
    }

}

void ATopDownPlayerController::SelectEnd(const FInputActionValue &Value)
{
    if (bIsPlacingBuilding)
    {
        return;
    }

    if (TopDownHUD)
    {
        //select new actors
        TopDownHUD->HideSelectionRect();
        FTimerHandle TimerHandleSelectMultipleActors;
        GetWorld()->GetTimerManager().SetTimer(TimerHandleSelectMultipleActors, this, &ATopDownPlayerController::SelectMultipleActors, 0.05f, false);
    }
}

void ATopDownPlayerController::SelectMultipleActors()
{
    if (TopDownHUD)
    {
        //deselect actors
        for (AActor* SomeActor : SelectedActors)
        {
            if (SomeActor)
            {
                if (SomeActor->GetClass()->ImplementsInterface(USelectableInterface::StaticClass()))
                {
                    ISelectableInterface::Execute_SelectActor(SomeActor, false);

                }
            }
        }
        
        SelectedActors.Empty();
        //select new actors
        TArray<ACLMBasePawn*> AllSelectedActors = TopDownHUD->GetSelectedActors();

        if (AllSelectedActors.Num()==1)
        {
            AActor* SomeActor = AllSelectedActors[0];
            if (SomeActor->GetClass()->ImplementsInterface(USelectableInterface::StaticClass()))
                {
                    ISelectableInterface::Execute_SelectActor(SomeActor, true);
                    SelectedActors.AddUnique(SomeActor);

                }
        }
        else{
        for (AActor* SomeActor : AllSelectedActors)
        {
            if (SomeActor)
            {
                if (SomeActor->GetClass()->ImplementsInterface(UFactionInterface::StaticClass()))
                {
                    int32 ActorFaction = IFactionInterface::Execute_GetFaction(SomeActor);
                    if (FactionID != ActorFaction)
                    {
                        continue;
                    }
                }
                if (SomeActor->GetClass()->ImplementsInterface(USelectableInterface::StaticClass()))
                {
                    ISelectableInterface::Execute_SelectActor(SomeActor, true);
                    SelectedActors.AddUnique(SomeActor);
                }
            }
        }
        OnActorsSelected.Broadcast(SelectedActors);
    }   
}
}

void ATopDownPlayerController::StartPlacingBuilding(TSubclassOf<ACLMBaseBuilding> BuildingClass)
{
    if (!BuildingClass)
    {
        return;
    }

    CancelBuildingPlacement();

    PendingBuildingClass = BuildingClass;
    bIsPlacingBuilding = true;

    FActorSpawnParameters SpawnParams;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
    PreviewBuilding = GetWorld()->SpawnActor<ACLMBaseBuilding>(BuildingClass, FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
    if (PreviewBuilding)
    {
        PreviewBuilding->SetPreview(true);
        PreviewBuilding->SetPlacementValid(false);
    }
}

void ATopDownPlayerController::ConfirmBuildingPlacement()
{
    if (!bIsPlacingBuilding || !PreviewBuilding || !bLastPlacementValid)
    {
        return;
    }

    const int32 Cost = PreviewBuilding->GetBuildCostMana();
    if (Cost > 0 && ResourceComponent && !ResourceComponent->SpendMana(Cost))
    {
        return;
    }

    const FVector SpawnLocation = PreviewBuilding->GetActorLocation();
    const FRotator SpawnRotation = PreviewBuilding->GetActorRotation();

    FActorSpawnParameters SpawnParams;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

    ACLMBaseBuilding* NewBuilding = GetWorld()->SpawnActor<ACLMBaseBuilding>(PendingBuildingClass, SpawnLocation, SpawnRotation, SpawnParams);
    if (NewBuilding)
    {
        IFactionInterface::Execute_SetFaction(NewBuilding, FactionID);
        NewBuilding->SetOwningTopDownController(this);
        OnBuildingPlaced.Broadcast(NewBuilding);
    }

    CancelBuildingPlacement();
}

void ATopDownPlayerController::CancelBuildingPlacement()
{
    bIsPlacingBuilding = false;
    bLastPlacementValid = false;
    PendingBuildingClass = nullptr;

    if (PreviewBuilding)
    {
        PreviewBuilding->Destroy();
        PreviewBuilding = nullptr;
    }
}

bool ATopDownPlayerController::IsPlacingBuilding() const
{
    return bIsPlacingBuilding;
}

bool ATopDownPlayerController::UpdateBuildingPreview()
{
    if (!bIsPlacingBuilding || !PreviewBuilding)
    {
        return false;
    }

    FHitResult HitResult;
    const bool bHit = GetHitResultUnderCursorByChannel(BuildTraceChannel, false, HitResult);
    if (!bHit)
    {
        PreviewBuilding->SetActorHiddenInGame(true);
        bLastPlacementValid = false;
        return false;
    }

    PreviewBuilding->SetActorHiddenInGame(false);
    PreviewBuilding->SetActorLocation(HitResult.Location);

    bLastPlacementValid = IsPlacementValid(HitResult);
    PreviewBuilding->SetPlacementValid(bLastPlacementValid);
    return true;
}

bool ATopDownPlayerController::IsPlacementValid(const FHitResult& HitResult) const
{
    if (!PreviewBuilding)
    {
        return false;
    }

    if (HitResult.ImpactNormal.Z < MinSurfaceNormalZ)
    {
        return false;
    }

    if (BuildRange > 0.0f)
    {
        const APawn* Pawn = GetPawn();
        if (Pawn)
        {
            const float Distance = FVector::Dist(Pawn->GetActorLocation(), HitResult.Location);
            if (Distance > BuildRange)
            {
                return false;
            }
        }
    }

    const FVector Extent = PreviewBuilding->GetPlacementBoundsExtent();
    FCollisionShape Shape = FCollisionShape::MakeBox(Extent);

    FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(BuildingPlacement), false);
    QueryParams.AddIgnoredActor(PreviewBuilding);

    FCollisionObjectQueryParams ObjectParams;
    ObjectParams.AddObjectTypesToQuery(ECC_WorldStatic);
    ObjectParams.AddObjectTypesToQuery(ECC_WorldDynamic);

    const bool bOverlaps = GetWorld()->OverlapAnyTestByObjectType(HitResult.Location, FQuat::Identity, ObjectParams, Shape, QueryParams);
    return !bOverlaps;
}

void ATopDownPlayerController::SetFaction_Implementation(int32 NewFaction)
{
    FactionID = NewFaction;
}

int32 ATopDownPlayerController::GetFaction_Implementation()
{
    return FactionID;
}
