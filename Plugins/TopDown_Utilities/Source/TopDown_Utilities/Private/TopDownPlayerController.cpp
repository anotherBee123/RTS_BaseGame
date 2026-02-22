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
#include "Engine/DataTable.h"
#include "DrawDebugHelpers.h"

// Lifecycle / setup


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

TArray<AActor*> ATopDownPlayerController::GetSelectedActorsForUI() const
{
    return SelectedActors;
}

AActor* ATopDownPlayerController::GetPrimarySelectedActor() const
{
    if (SelectedActors.Num() > 0)
    {
        return SelectedActors[0];
    }
    return SelectedActor;
}

// Engine lifecycle
void ATopDownPlayerController::BeginPlay()
{
    Super::BeginPlay();

    TopDownHUD = Cast<ATopDownHUD>(GetHUD());
}

void ATopDownPlayerController::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);

    UpdateBuildingPreview();

    if (bIsCommandDragActive)
    {
        DrawFormationPreviewLines();
    }
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
        EnhancedInputComponent->BindAction(CommandAction, ETriggerEvent::Started, this, &ATopDownPlayerController::StartCommandDrag);
        EnhancedInputComponent->BindAction(CommandAction, ETriggerEvent::Triggered, this, &ATopDownPlayerController::UpdateCommandDrag);
        EnhancedInputComponent->BindAction(CommandAction, ETriggerEvent::Completed, this, &ATopDownPlayerController::CommandSelectedActors);
        EnhancedInputComponent->BindAction(CommandAction, ETriggerEvent::Canceled, this, &ATopDownPlayerController::CancelCommandDrag);

    }


}

// Selection input
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

    for (AActor* SomeActor : SelectedActors)
    {
        if (SomeActor && SomeActor->GetClass()->ImplementsInterface(USelectableInterface::StaticClass()))
        {
            ISelectableInterface::Execute_SelectActor(SomeActor, false);
        }
    }
    SelectedActors.Empty();

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
            SelectedActors.AddUnique(SelectedActor);

        }
    }

    OnActorsSelected.Broadcast(SelectedActors);



}


void ATopDownPlayerController::CommandSelectedActors(const FInputActionValue& Value)
{
    if (bIsPlacingBuilding)
    {
        return;
    }

    const bool bShouldIssueFormationMove =
        bIsCommandDragActive &&
        bHasCommandDragHit &&
        CommandDragActors.Num() > 1 &&
        CommandDragDestinations.Num() == CommandDragActors.Num() &&
        FVector::Dist2D(CommandDragStartLocation, CommandDragCurrentLocation) >= FormationDragMinLength;

    if (bShouldIssueFormationMove)
    {
        IssueFormationMoveCommand();
        ClearCommandDrag();
        return;
    }

    FHitResult HitResult;
    if (TryGetCommandHit(HitResult))
    {
        IssueLegacyMoveCommand(HitResult.Location);
    }

    ClearCommandDrag();
}

void ATopDownPlayerController::StartCommandDrag(const FInputActionValue& Value)
{
    if (bIsPlacingBuilding || SelectedActors.Num() <= 1)
    {
        ClearCommandDrag();
        return;
    }

    FHitResult HitResult;
    if (!TryGetCommandHit(HitResult))
    {
        ClearCommandDrag();
        return;
    }

    TArray<AActor*> CommandableActors;
    GatherCommandableSelectedActors(CommandableActors);
    if (CommandableActors.Num() <= 1)
    {
        ClearCommandDrag();
        return;
    }

    bIsCommandDragActive = true;
    bHasCommandDragHit = true;
    CommandDragStartLocation = HitResult.Location;
    CommandDragCurrentLocation = HitResult.Location;

    CommandDragActors.Empty();
    for (AActor* Actor : CommandableActors)
    {
        CommandDragActors.Add(Actor);
    }

    BuildLineFormationDestinations(CommandDragStartLocation, CommandDragCurrentLocation, CommandDragActors.Num(), CommandDragDestinations);
}

void ATopDownPlayerController::UpdateCommandDrag(const FInputActionValue& Value)
{
    if (!bIsCommandDragActive)
    {
        return;
    }

    FHitResult HitResult;
    if (!TryGetCommandHit(HitResult))
    {
        bHasCommandDragHit = false;
        CommandDragDestinations.Empty();
        return;
    }

    bHasCommandDragHit = true;
    CommandDragCurrentLocation = HitResult.Location;
    BuildLineFormationDestinations(CommandDragStartLocation, CommandDragCurrentLocation, CommandDragActors.Num(), CommandDragDestinations);
    DrawFormationPreviewLines();
}

void ATopDownPlayerController::CancelCommandDrag(const FInputActionValue& Value)
{
    ClearCommandDrag();
}

void ATopDownPlayerController::ClearCommandDrag()
{
    bIsCommandDragActive = false;
    bHasCommandDragHit = false;
    CommandDragActors.Empty();
    CommandDragDestinations.Empty();
}

bool ATopDownPlayerController::TryGetCommandHit(FHitResult& OutHitResult)
{
    return GetHitResultUnderCursor(ECollisionChannel::ECC_Camera, false, OutHitResult) && OutHitResult.bBlockingHit;
}

void ATopDownPlayerController::GatherCommandableSelectedActors(TArray<AActor*>& OutActors) const
{
    OutActors.Reset();

    for (AActor* SomeActor : SelectedActors)
    {
        if (!SomeActor)
        {
            continue;
        }

        if (SomeActor->GetClass()->ImplementsInterface(UFactionInterface::StaticClass()))
        {
            const int32 ActorFaction = IFactionInterface::Execute_GetFaction(SomeActor);
            if (FactionID != ActorFaction)
            {
                continue;
            }
        }

        if (SomeActor->GetClass()->ImplementsInterface(UNavigableInterface::StaticClass()))
        {
            OutActors.Add(SomeActor);
        }
    }
}

void ATopDownPlayerController::BuildLineFormationDestinations(const FVector& StartLocation, const FVector& EndLocation, int32 Count, TArray<FVector>& OutDestinations) const
{
    OutDestinations.Reset();
    if (Count <= 0)
    {
        return;
    }

    if (Count == 1)
    {
        OutDestinations.Add(EndLocation);
        return;
    }

    for (int32 Index = 0; Index < Count; ++Index)
    {
        const float Alpha = static_cast<float>(Index) / static_cast<float>(Count - 1);
        OutDestinations.Add(FMath::Lerp(StartLocation, EndLocation, Alpha));
    }
}

void ATopDownPlayerController::BuildFormationAssignmentByProximity(TArray<int32>& OutDestinationIndexPerActor) const
{
    OutDestinationIndexPerActor.Reset();

    const int32 Count = FMath::Min(CommandDragActors.Num(), CommandDragDestinations.Num());
    if (Count <= 0)
    {
        return;
    }

    OutDestinationIndexPerActor.Init(INDEX_NONE, Count);
    TArray<bool> bDestinationUsed;
    bDestinationUsed.Init(false, Count);

    // Greedy nearest-slot assignment keeps formation readable and reduces path crossing.
    for (int32 ActorIndex = 0; ActorIndex < Count; ++ActorIndex)
    {
        const AActor* SomeActor = CommandDragActors[ActorIndex].Get();
        if (!SomeActor)
        {
            continue;
        }

        int32 BestDestinationIndex = INDEX_NONE;
        float BestDistanceSq = TNumericLimits<float>::Max();
        for (int32 DestinationIndex = 0; DestinationIndex < Count; ++DestinationIndex)
        {
            if (bDestinationUsed[DestinationIndex])
            {
                continue;
            }

            const float DistSq = FVector::DistSquared2D(SomeActor->GetActorLocation(), CommandDragDestinations[DestinationIndex]);
            if (DistSq < BestDistanceSq)
            {
                BestDistanceSq = DistSq;
                BestDestinationIndex = DestinationIndex;
            }
        }

        if (BestDestinationIndex != INDEX_NONE)
        {
            bDestinationUsed[BestDestinationIndex] = true;
            OutDestinationIndexPerActor[ActorIndex] = BestDestinationIndex;
        }
    }
}

void ATopDownPlayerController::DrawFormationPreviewLines() const
{
    if (!GetWorld() || !bHasCommandDragHit)
    {
        return;
    }

    const int32 Count = FMath::Min(CommandDragActors.Num(), CommandDragDestinations.Num());
    TArray<int32> DestinationIndexPerActor;
    BuildFormationAssignmentByProximity(DestinationIndexPerActor);

    for (int32 Index = 0; Index < Count; ++Index)
    {
        const AActor* SomeActor = CommandDragActors[Index].Get();
        if (!SomeActor || !DestinationIndexPerActor.IsValidIndex(Index))
        {
            continue;
        }

        const int32 DestinationIndex = DestinationIndexPerActor[Index];
        if (!CommandDragDestinations.IsValidIndex(DestinationIndex))
        {
            continue;
        }

        const FVector& Destination = CommandDragDestinations[DestinationIndex];
        DrawDebugLine(GetWorld(), SomeActor->GetActorLocation(), Destination, FormationDebugLineColor, false, 0.0f, 0, 2.0f);
        DrawDebugSphere(GetWorld(), Destination, 22.0f, 8, FormationDebugLineColor, false, 0.0f, 0, 1.5f);
    }
}

void ATopDownPlayerController::IssueFormationMoveCommand()
{
    const int32 Count = FMath::Min(CommandDragActors.Num(), CommandDragDestinations.Num());
    TArray<int32> DestinationIndexPerActor;
    BuildFormationAssignmentByProximity(DestinationIndexPerActor);

    for (int32 Index = 0; Index < Count; ++Index)
    {
        AActor* SomeActor = CommandDragActors[Index].Get();
        if (!SomeActor || !DestinationIndexPerActor.IsValidIndex(Index))
        {
            continue;
        }

        const int32 DestinationIndex = DestinationIndexPerActor[Index];
        if (!CommandDragDestinations.IsValidIndex(DestinationIndex))
        {
            continue;
        }

        INavigableInterface::Execute_MoveToLocation(SomeActor, CommandDragDestinations[DestinationIndex]);
    }
}

void ATopDownPlayerController::IssueLegacyMoveCommand(const FVector& Destination)
{
    if (SelectedActors.Num() > 0)
    {
        int32 i = SelectedActors.Num() / -2;
        for (AActor* SomeActor : SelectedActors)
        {
            if (SomeActor)
            {
                if (SomeActor->GetClass()->ImplementsInterface(UFactionInterface::StaticClass()))
                {
                    const int32 ActorFaction = IFactionInterface::Execute_GetFaction(SomeActor);
                    if (FactionID != ActorFaction)
                    {
                        continue;
                    }
                }

                if (SomeActor->GetClass()->ImplementsInterface(UNavigableInterface::StaticClass()))
                {
                    INavigableInterface::Execute_MoveToLocation(SomeActor, Destination + FVector(0, 100 * i, 0));
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
            const int32 ActorFaction = IFactionInterface::Execute_GetFaction(SelectedActor);
            if (FactionID != ActorFaction)
            {
                bIsCorrectFaction = false;
            }
        }
        if (bIsCorrectFaction && SelectedActor->GetClass()->ImplementsInterface(UNavigableInterface::StaticClass()))
        {
            INavigableInterface::Execute_MoveToLocation(SelectedActor, Destination);
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
    SelectionSize = FVector2D::ZeroVector;
    bIsSelectionDragActive = false;

    
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
    bIsSelectionDragActive = SelectionSize.SizeSquared() >= FMath::Square(SelectionDragThreshold);

    if (TopDownHUD && bIsSelectionDragActive)
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

    if (!bIsSelectionDragActive)
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
    }   
        SelectedActor = SelectedActors.Num() > 0 ? SelectedActors[0] : nullptr;
        OnActorsSelected.Broadcast(SelectedActors);
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
    PendingBuildingId = NAME_None;
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

TArray<FBuildingData> ATopDownPlayerController::GetAvailableBuildingsForFaction() const
{
    TArray<FBuildingData> Result;
    if (!BuildingDataTable)
    {
        return Result;
    }

    static const FString Context(TEXT("BuildingData"));
    TArray<FBuildingData*> Rows;
    BuildingDataTable->GetAllRows(Context, Rows);

    for (const FBuildingData* Row : Rows)
    {
        if (!Row || !Row->BuildingClass)
        {
            continue;
        }

        if (Row->AllowedFactions.Num() > 0 && !Row->AllowedFactions.Contains(FactionID))
        {
            continue;
        }

        Result.Add(*Row);
    }

    Result.Sort([](const FBuildingData& A, const FBuildingData& B)
    {
        return A.BuildingId.LexicalLess(B.BuildingId);
    });

    return Result;
}

bool ATopDownPlayerController::StartPlacingBuildingById(FName BuildingId)
{
    if (BuildingId.IsNone() || !BuildingDataTable)
    {
        return false;
    }

    static const FString Context(TEXT("BuildingData"));
    const FBuildingData* Row = BuildingDataTable->FindRow<FBuildingData>(BuildingId, Context);
    if (!Row || !Row->BuildingClass)
    {
        return false;
    }

    if (Row->AllowedFactions.Num() > 0 && !Row->AllowedFactions.Contains(FactionID))
    {
        return false;
    }

    StartPlacingBuilding(Row->BuildingClass);
    PendingBuildingId = Row->BuildingId;
    return true;
}

bool ATopDownPlayerController::StartPlacingBuildingBySlot(int32 SlotIndex)
{
    if (SlotIndex < 0)
    {
        return false;
    }

    const TArray<FBuildingData> AvailableBuildings = GetAvailableBuildingsForFaction();
    if (!AvailableBuildings.IsValidIndex(SlotIndex))
    {
        return false;
    }

    return StartPlacingBuildingById(AvailableBuildings[SlotIndex].BuildingId);
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

    const FVector BuildSpawnLocation = PreviewBuilding->GetActorLocation();
    const FRotator BuildSpawnRotation = PreviewBuilding->GetActorRotation();

    FActorSpawnParameters SpawnParams;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

    ACLMBaseBuilding* NewBuilding = GetWorld()->SpawnActor<ACLMBaseBuilding>(PendingBuildingClass, BuildSpawnLocation, BuildSpawnRotation, SpawnParams);
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
    PendingBuildingId = NAME_None;

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

void ATopDownPlayerController::ToggleBuildingMenu()
{
    bShowBuildingMenu = !bShowBuildingMenu;
}

bool ATopDownPlayerController::IsBuildingMenuVisible() const
{
    return bShowBuildingMenu;
}

FName ATopDownPlayerController::GetPendingBuildingId() const
{
    return PendingBuildingId;
}

int32 ATopDownPlayerController::GetCurrentMana() const
{
    return ResourceComponent ? ResourceComponent->GetMana() : 0;
}

void ATopDownPlayerController::TrySelectBuildingSlot(int32 SlotIndex)
{
    if (!bShowBuildingMenu)
    {
        return;
    }

    StartPlacingBuildingBySlot(SlotIndex);
}

void ATopDownPlayerController::SelectBuildingSlot1() { TrySelectBuildingSlot(0); }
void ATopDownPlayerController::SelectBuildingSlot2() { TrySelectBuildingSlot(1); }
void ATopDownPlayerController::SelectBuildingSlot3() { TrySelectBuildingSlot(2); }
void ATopDownPlayerController::SelectBuildingSlot4() { TrySelectBuildingSlot(3); }
void ATopDownPlayerController::SelectBuildingSlot5() { TrySelectBuildingSlot(4); }
void ATopDownPlayerController::SelectBuildingSlot6() { TrySelectBuildingSlot(5); }
void ATopDownPlayerController::SelectBuildingSlot7() { TrySelectBuildingSlot(6); }
void ATopDownPlayerController::SelectBuildingSlot8() { TrySelectBuildingSlot(7); }
void ATopDownPlayerController::SelectBuildingSlot9() { TrySelectBuildingSlot(8); }

void ATopDownPlayerController::CancelBuildingPlacementInput()
{
    CancelBuildingPlacement();
}

bool ATopDownPlayerController::UpdateBuildingPreview()
{
    if (!bIsPlacingBuilding || !PreviewBuilding)
    {
        return false;
    }

    FHitResult HitResult;
    const bool bHit = GetHitResultUnderCursor(BuildTraceChannel, false, HitResult);
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
        const APawn* ControlledPawn = GetPawn();
        if (ControlledPawn)
        {
            const float Distance = FVector::Dist(ControlledPawn->GetActorLocation(), HitResult.Location);
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
