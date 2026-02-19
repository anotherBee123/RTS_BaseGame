// Fill out your copyright notice in the Description page of Project Settings.


#include "ControllerPawn.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "InputActionValue.h"
#include "EnhancedInputComponent.h"
#include "GameFramework/FloatingPawnMovement.h"
// Sets default values
AControllerPawn::AControllerPawn()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;


	//Create Capsule
	CapsuleComponent = CreateDefaultSubobject<UCapsuleComponent>(TEXT("CapsuleComponent"));
	RootComponent = CapsuleComponent;

	//Create Spring Arm

	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));


	//Create Camera
	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));

	//Attach Camera to Spring arm
	SpringArm->SetupAttachment(RootComponent);
	Camera->SetupAttachment(SpringArm, USpringArmComponent::SocketName);


	//Create floating movement
	FloatingPawnMovement = CreateDefaultSubobject<UFloatingPawnMovement>(TEXT("Floating Movement"));






}

// Called when the game starts or when spawned
void AControllerPawn::BeginPlay()
{
	Super::BeginPlay();
	
}

void AControllerPawn::Move(const FInputActionValue& Value)
{
	const FVector2D MovementInput = Value.Get<FVector2D>();
	if (Controller)
	{
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		const FVector Forward = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		const FVector Right = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);



		AddMovementInput(Forward, MovementInput.Y);
		AddMovementInput(Right, MovementInput.X);
		

	}

}

void AControllerPawn::Zoom(const FInputActionValue& Value)
{
	const float ZoomDirection = Value.Get<float>();

	if (Controller!=nullptr)
	{
		float DesiredOrthoWidth	= Camera->OrthoWidth + ZoomDirection*CameraZoomSpeed;
		DesiredOrthoWidth = FMath::Clamp(DesiredOrthoWidth, MinCameraOrthoWidth, MaxCameraOrthoWidth);
		Camera->OrthoWidth = DesiredOrthoWidth;
	}
}

void AControllerPawn::EdgeScrollWithMouse()
{
	APlayerController* PlayerController = Cast<APlayerController>(Controller);
	if (!PlayerController)
	{
		return;
	} 

	float MouseX=0, MouseY=0;


	if (PlayerController->GetMousePosition(MouseX, MouseY))
	{
		UE_LOG(LogTemp, Warning, TEXT("Mouse Pos: %f %f"), MouseX, MouseY);
		FVector2D ViewportSize;
		if (GEngine && GEngine->GameViewport)
		{
			GEngine->GameViewport->GetViewportSize(ViewportSize);

			float EdgeThreshold = 20.0f;
			FVector2D MovementInput = FVector2D::ZeroVector;

			if (MouseX < EdgeThreshold)
			{
				UE_LOG(LogTemp, Warning, TEXT("Left"));

				MovementInput.X = -3.f;
			}

			if (MouseX > (ViewportSize.X - EdgeThreshold))
			{
				MovementInput.X = 3.f;
			}
			if (MouseY < EdgeThreshold)
			{
				MovementInput.Y = 3.f;
			}
			if (MouseY > (ViewportSize.Y - EdgeThreshold))
			{
				MovementInput.Y = -3.f;
			}

			if  (!MovementInput.IsZero())
			{
				Move(FInputActionValue(MovementInput));
			}
		}
	}
}

// Called every frame
void AControllerPawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	EdgeScrollWithMouse();

}

// Called to bind functionality to input
void AControllerPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent)){

		//bind move function to move input 
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, 	&AControllerPawn::Move);

		//bind zoom function to zoom input action
		EnhancedInputComponent->BindAction(ZoomAction, ETriggerEvent::Triggered, this, &AControllerPawn::Zoom);

	}

}


