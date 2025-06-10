// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerCharacter.h"
#include "Camera/CameraComponent.h"
#include "GameFrameWork/SpringArmComponent.h"
#include "Kismet/GameplayStatics.h"
#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"
#include "GameFramework/CharacterMovementComponent.h"

// Sets default values
APlayerCharacter::APlayerCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));

	CameraBoom->SetupAttachment(GetMesh());
	CameraBoom->TargetArmLength = 600;
	CameraBoom->bUsePawnControlRotation = true;

	Camera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	Camera->bUsePawnControlRotation = false;

}

// Called when the game starts or when spawned
void APlayerCharacter::BeginPlay()
{
	Super::BeginPlay();

	PlayerController = Cast<APlayerController>(GetController());

	if (PlayerController)
	{
		UEnhancedInputLocalPlayerSubsystem* subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer());
		if (subsystem && InputMappingContext)
		{
			subsystem->AddMappingContext(InputMappingContext, 0);
		}
	}
	
}

// Called every frame
void APlayerCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void APlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(PlayerInputComponent))
	{
		if (MoveAction)
		{
			EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &APlayerCharacter::Move);
		}
		if (LookAction)
		{
			EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &APlayerCharacter::Look);
		}
		if (RunAction)
		{
			EnhancedInputComponent->BindAction(RunAction, ETriggerEvent::Triggered, this, &APlayerCharacter::Run);
			EnhancedInputComponent->BindAction(RunAction, ETriggerEvent::Completed, this, &APlayerCharacter::StopRun);
		}
		if (FireAction)
		{
			EnhancedInputComponent->BindAction(FireAction, ETriggerEvent::Triggered, this, &APlayerCharacter::Shoot);
		}
	}

}


void APlayerCharacter::Move(const FInputActionValue& value)
{
	FVector2D Movement = value.Get<FVector2D>();

	if (PlayerController)
	{	
		// Get the Yaw rotation from the controller
		const FRotator rotation = Controller->GetControlRotation();
		const FRotator YawRotator(0, rotation.Yaw, 0);

		//Separate the forward vector and right vector from the Rotator
		const FVector ForwardVector = FRotationMatrix(YawRotator).GetUnitAxis(EAxis::X);
		const FVector RightVector = FRotationMatrix(YawRotator).GetUnitAxis(EAxis::Y);

		// Add the movement input to the directions extracted.
		AddMovementInput(ForwardVector, Movement.X);
		AddMovementInput(RightVector, Movement.Y);

	}
}

void APlayerCharacter::Run(const FInputActionValue& value)
{
	const bool currentValue = value.Get<bool>();

	if (PlayerController && currentValue)
	{
		GetCharacterMovement()->MaxWalkSpeed = 950.f;
	}
}

void APlayerCharacter::StopRun(const FInputActionValue& value)
{

	if (PlayerController)
	{
		GetCharacterMovement()->MaxWalkSpeed = 550.f;
	}
}

void APlayerCharacter::Look(const FInputActionValue& value)
{
	FVector2D CurrentValue = value.Get<FVector2D>();

	if (!CurrentValue.IsZero() && PlayerController)
	{
		float currentYawValue = CurrentValue.X * UGameplayStatics::GetWorldDeltaSeconds(this) * TurnRate;
		float currentPitchValue = CurrentValue.Y * UGameplayStatics::GetWorldDeltaSeconds(this) * TurnRate * -1;

		AddControllerYawInput(currentYawValue);
		AddControllerPitchInput(currentPitchValue);
	}
}

void APlayerCharacter::Shoot(const FInputActionValue& value)
{
}

