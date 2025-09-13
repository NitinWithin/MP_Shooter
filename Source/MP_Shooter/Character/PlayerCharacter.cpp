// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerCharacter.h"
#include "Camera/CameraComponent.h"
#include "GameFrameWork/SpringArmComponent.h"
#include "Kismet/GameplayStatics.h"
#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/WidgetComponent.h"
#include "Net/UnrealNetwork.h"
#include "MP_Shooter/Weapon/Weapon.h"
#include "MP_Shooter/ShooterComponents/CombatComponent.h"
#include "Components/CapsuleComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "MP_Shooter/MP_Shooter.h"

// Sets default values
APlayerCharacter::APlayerCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;

	cameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));	

	cameraBoom->SetupAttachment(GetMesh());
	cameraBoom->TargetArmLength = 600;
	cameraBoom->bUsePawnControlRotation = true;

	camera->SetupAttachment(cameraBoom, USpringArmComponent::SocketName);
	camera->bUsePawnControlRotation = false;

	overheadWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("NetworkRole"));
	overheadWidget->SetupAttachment(RootComponent);

	combat = CreateDefaultSubobject<UCombatComponent>(TEXT("CombatComponent"));
	combat->SetIsReplicated(true);
	GetCharacterMovement()->RotationRate = FRotator(0.f, 0.f, 850.f);
	
	walkingSpeed = 350.f;
	runningSpeed = 950.f;
	turnRate = 100;
	turnThreshold = 0.5f;
	cameraThreshold = 200.f;

	bIsRunning = false;

	GetCapsuleComponent()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECR_Ignore);
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECR_Ignore);
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECR_Block);
	GetMesh()->SetCollisionObjectType(ECC_SkeletalMesh);

	NetUpdateFrequency = 66.f;
	MinNetUpdateFrequency = 33.f;
}

void APlayerCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	// Call the Super
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	// Add properties to replicated for the derived class
	DOREPLIFETIME_CONDITION(APlayerCharacter, overlappingWeapon, COND_OwnerOnly);
}

void APlayerCharacter::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	if (combat)
	{
		combat->playerCharacter = this;
	}
}

void APlayerCharacter::BeginPlay()
{
	Super::BeginPlay();

	PlayerController = Cast<APlayerController>(GetController());

	if (PlayerController)
	{
		UEnhancedInputLocalPlayerSubsystem* subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer());
		if (subsystem && inputMappingContext)
		{
			subsystem->AddMappingContext(inputMappingContext, 0);
		}
	}
	
	bUseControllerRotationYaw = false;
	GetCharacterMovement()->bOrientRotationToMovement = true;	
	GetCharacterMovement()->NavAgentProps.bCanCrouch = true;

	turningInPlace = ETurningInPlace::ETIP_NotTurning;
}

void APlayerCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (GetLocalRole() > ROLE_SimulatedProxy && IsLocallyControlled())
	{
		AimOffset(DeltaTime);
	}
	else
	{
		
		timeSinceReplication += DeltaTime;
		if (timeSinceReplication > 0.15f)
		{
			OnRep_ReplicatedMovement();
		}
		Calculate_AO_Pitch();
	}
	HidePlayerWhenCameraClose();
}

void APlayerCharacter::OnRep_ReplicatedMovement()
{
	Super::OnRep_ReplicatedMovement();

	TurnSimProxies();
	timeSinceReplication = 0.f;
}

// Called to bind functionality to input
void APlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(PlayerInputComponent))
	{
		if (moveAction)
		{
			EnhancedInputComponent->BindAction(moveAction, ETriggerEvent::Triggered, this, &APlayerCharacter::Move);
		}
		if (lookAction)
		{
			EnhancedInputComponent->BindAction(lookAction, ETriggerEvent::Triggered, this, &APlayerCharacter::Look);
		}
		if (runAction)
		{
			EnhancedInputComponent->BindAction(runAction, ETriggerEvent::Started, this, &APlayerCharacter::Run);
			EnhancedInputComponent->BindAction(runAction, ETriggerEvent::Completed, this, &APlayerCharacter::StopRun);
		}
		if (fireAction)
		{
			EnhancedInputComponent->BindAction(fireAction, ETriggerEvent::Started, this, &APlayerCharacter::Shoot);
			EnhancedInputComponent->BindAction(fireAction, ETriggerEvent::Completed, this, &APlayerCharacter::StopShooting);
		}
		if (jumpAction)
		{
			EnhancedInputComponent->BindAction(jumpAction, ETriggerEvent::Triggered, this, &ACharacter::Jump);
		}
		if (interactAction)
		{
			EnhancedInputComponent->BindAction(interactAction, ETriggerEvent::Triggered, this, &APlayerCharacter::Interact);
		}
		if (crouchAction)
		{
			EnhancedInputComponent->BindAction(crouchAction, ETriggerEvent::Started, this, &APlayerCharacter::CrouchButtonPressed);
			EnhancedInputComponent->BindAction(crouchAction, ETriggerEvent::Completed, this, &APlayerCharacter::CrouchButtonReleased);
		}
		if (aimAction)
		{
			EnhancedInputComponent->BindAction(aimAction, ETriggerEvent::Started, this, &APlayerCharacter::AimButtonPressed);
			EnhancedInputComponent->BindAction(aimAction, ETriggerEvent::Completed, this, &APlayerCharacter::AimButtonReleased);
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
		bIsRunning = true;
		Running(bIsRunning);
	}
}

void APlayerCharacter::StopRun(const FInputActionValue& value)
{
	if (PlayerController)
	{
		bIsRunning = false;
		Running(bIsRunning);
	}
}

void APlayerCharacter::Look(const FInputActionValue& value)
{
	FVector2D CurrentValue = value.Get<FVector2D>();

	if (!CurrentValue.IsZero() && PlayerController)
	{
		float currentYawValue = CurrentValue.X * UGameplayStatics::GetWorldDeltaSeconds(this) * turnRate;
		float currentPitchValue = CurrentValue.Y * UGameplayStatics::GetWorldDeltaSeconds(this) * turnRate * -1;

		AddControllerYawInput(currentYawValue);
		AddControllerPitchInput(currentPitchValue);
	}
}

void APlayerCharacter::Interact(const FInputActionValue& value)
{
	const bool Currentvalue = value.Get<bool>();
	if (Currentvalue && combat)
	{
		if (HasAuthority())
		{
			combat->EquipWeapon(overlappingWeapon);
		}
		else
		{
			ServerInteractButtonPressed();
		}
	}
}

void APlayerCharacter::CrouchButtonPressed(const FInputActionValue& value)
{
	Crouch();
}

void APlayerCharacter::CrouchButtonReleased(const FInputActionValue& value)
{
	UnCrouch();
}

void APlayerCharacter::AimButtonPressed(const FInputActionValue& value)
{
	if (combat)
	{
		combat->SetAiming(true);
		GetCharacterMovement()->bOrientRotationToMovement = false;
	}
}

void APlayerCharacter::AimButtonReleased(const FInputActionValue& value)
{
	if (combat)
	{
		combat->SetAiming(false);
		GetCharacterMovement()->bOrientRotationToMovement = true;
	}
}

void APlayerCharacter::Shoot(const FInputActionValue& value)
{
	if (combat)
	{
		combat->Shoot(true);
	}
}

void APlayerCharacter::StopShooting(const FInputActionValue& value)
{
	if (combat)
	{
		combat->Shoot(false);
	}
}

float APlayerCharacter::Calculate_Speed()
{
	FVector Velocity = GetVelocity();
	Velocity.Z = 0;
	return Velocity.Size();
}

void APlayerCharacter::AimOffset(float DeltaTime)
{
	if (combat && combat->equippedWeapon == nullptr)
	{
		return;
	}
	float speed = Calculate_Speed();

	bool bisInAir = GetCharacterMovement()->IsFalling();

	if (speed == 0.f && !bisInAir) // Standing still & not jumping
	{
		bRotateRootBone = true;
		FRotator CurrentAimRotation = FRotator(0.f, GetBaseAimRotation().Yaw, 0.f);
		FRotator DeltaRotation = UKismetMathLibrary::NormalizedDeltaRotator(CurrentAimRotation, startAimRotation);
		AO_Yaw = DeltaRotation.Yaw;
		bUseControllerRotationYaw = true;
		if (turningInPlace == ETurningInPlace::ETIP_NotTurning)
		{
			interp_AO_Yaw = AO_Yaw;
		}
		TurnInPlace(DeltaTime);
	}
	if (speed > 0.f || bisInAir) // Running or Jumping 
	{
		bRotateRootBone = false;
		startAimRotation = FRotator(0.f, GetBaseAimRotation().Yaw, 0.f);
		AO_Yaw = 0.f;
		bUseControllerRotationYaw = true;
		
		turningInPlace = ETurningInPlace::ETIP_NotTurning;
	}

	Calculate_AO_Pitch();
}

bool APlayerCharacter::IsAiming()
{
	return (combat && combat->bAiming);
}

void APlayerCharacter::Calculate_AO_Pitch()
{
	AO_Pitch = GetBaseAimRotation().Pitch;
	if (AO_Pitch > 90.f && !IsLocallyControlled())
	{
		//Change values on Server to correct orientation of Pitch
		FVector2D InRange(270.f, 360.f);
		FVector2D OutRange(-90.f, 0.f);
		AO_Pitch = FMath::GetMappedRangeValueClamped(InRange, OutRange, AO_Pitch);
	}
}

void APlayerCharacter::TurnSimProxies()
{
	if (combat == nullptr || combat->equippedWeapon == nullptr)
	{
		return;
	}
	bUseControllerRotationYaw = true;
	bRotateRootBone = false;
	float speed = Calculate_Speed();
	if (speed > 0.f)
	{
		turningInPlace = ETurningInPlace::ETIP_NotTurning;
		return;
	}

	proxyRotationLastFrame = proxyRotation;
	proxyRotation = GetActorRotation();

	/*UE_LOG(LogTemp, Error, TEXT("ProxyRot : %f"), proxyRotation);
	UE_LOG(LogTemp, Error, TEXT("ProxyRotLF : %f"), proxyRotationLastFrame);*/
	proxyYaw = UKismetMathLibrary::NormalizedDeltaRotator(
			proxyRotation, 
			proxyRotationLastFrame
			).Yaw;
	//UE_LOG(LogTemp, Error, TEXT("ProxyYaw : %f"), proxyYaw);

	if (FMath::Abs(proxyYaw) > turnThreshold)
	{
		
		UE_LOG(LogTemp, Error, TEXT("Turning."));
		if (proxyYaw > turnThreshold)
		{
		UE_LOG(LogTemp, Error, TEXT("Turning Right."));
			turningInPlace = ETurningInPlace::ETIP_Right;
		}
		else if (proxyYaw < -turnThreshold)
		{
		UE_LOG(LogTemp, Error, TEXT("Turning Left."));
			turningInPlace = ETurningInPlace::ETIP_Left;
		}
		else
		{
		UE_LOG(LogTemp, Error, TEXT("Niot Turning."));
			turningInPlace = ETurningInPlace::ETIP_NotTurning;
		}
		return;
	}
	turningInPlace = ETurningInPlace::ETIP_NotTurning;
}

void APlayerCharacter::TurnInPlace(float DeltaTime)
{
	if (AO_Yaw > 45.f)
	{
		turningInPlace = ETurningInPlace::ETIP_Right;
	}
	else if (AO_Yaw < -45.f)
	{
		turningInPlace = ETurningInPlace::ETIP_Left;
	}
	if (turningInPlace != ETurningInPlace::ETIP_NotTurning)
	{
		interp_AO_Yaw = FMath::FInterpTo(interp_AO_Yaw, 0.f, DeltaTime, 4.f);
		AO_Yaw = interp_AO_Yaw;
		// When to stop turning
		if (FMath::Abs(AO_Yaw) < 5.f)
		{
			turningInPlace = ETurningInPlace::ETIP_NotTurning;
			startAimRotation = FRotator(0.f, GetBaseAimRotation().Yaw, 0.f);
		}
	}
}

void APlayerCharacter::Running(bool IsRunning)
{
	ServerRunning(bIsRunning);
}

void APlayerCharacter::ServerRunning_Implementation(bool IsRunning)
{
	if (IsRunning)
	{
		GetCharacterMovement()->MaxWalkSpeed = runningSpeed;
	}
	else
	{
		GetCharacterMovement()->MaxWalkSpeed = walkingSpeed;
	}
}

bool APlayerCharacter::IsPlayerRunning()
{
	return bIsRunning;
}

void APlayerCharacter::ServerInteractButtonPressed_Implementation()
{
	if (combat)
	{
		combat->EquipWeapon(overlappingWeapon);
	}
}

void APlayerCharacter::OnRep_OverlappingWeapon(AWeapon* LastWeapon)
{
	if (overlappingWeapon)
	{
		overlappingWeapon->ShowPickUpWidget(true);
	}
	if (LastWeapon)
	{
		LastWeapon->ShowPickUpWidget(false);
	}
}

void APlayerCharacter::SetOverlappingWeapon(AWeapon* Weapon)
{
	if (overlappingWeapon)
	{
		overlappingWeapon->ShowPickUpWidget(false);
	}
	overlappingWeapon = Weapon;

	if (IsLocallyControlled()) // Ensure this is only set on the server
	{
		if (overlappingWeapon)
		{
			overlappingWeapon->ShowPickUpWidget(true);
		}
	}
}

bool APlayerCharacter::IsWeaponEquipped()
{
	return (combat && combat->equippedWeapon);
}

AWeapon* APlayerCharacter::GetEquippedWeapon()
{
	if (combat == nullptr)
	{
		return nullptr;
	}
	return combat->equippedWeapon;
}

void APlayerCharacter::HidePlayerWhenCameraClose()
{
	if (!IsLocallyControlled())
	{
		return;
	}
	if ((camera->GetComponentLocation() - GetActorLocation()).Size() < cameraThreshold)
	{
		GetMesh()->SetVisibility(false);
		if (combat && combat->equippedWeapon && combat->equippedWeapon->GetWeaponMesh())
		{
			combat->equippedWeapon->GetWeaponMesh()->bOwnerNoSee = true;
		}
	}
	else
	{
		GetMesh()->SetVisibility(true);
		if (combat && combat->equippedWeapon && combat->equippedWeapon->GetWeaponMesh())
		{
			combat->equippedWeapon->GetWeaponMesh()->bOwnerNoSee = false;
		}
	}
}

void APlayerCharacter::PlayFireMontage(bool IsAiming)
{
	if (combat == nullptr || combat->equippedWeapon == nullptr)
	{
		return;
	}

	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && fireWeaponMontage)
	{
		AnimInstance->Montage_Play(fireWeaponMontage);
		FName SectionName;
		SectionName = IsAiming ? FName("AimFire") : FName("HipFire");
		AnimInstance->Montage_JumpToSection(SectionName);
	}
}

void APlayerCharacter::PlayHitReactMontage()
{
	if (combat == nullptr || combat->equippedWeapon == nullptr)
	{
		return;
	}

	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && hitReactMontage)
	{
		AnimInstance->Montage_Play(hitReactMontage);
		FName SectionName("FromFront");
		//SectionName = IsAiming ? FName("AimFire") : FName("HipFire");
		AnimInstance->Montage_JumpToSection(SectionName);
	}
}

void APlayerCharacter::Multicast_HitReact_Implementation()
{
	PlayHitReactMontage();
}

FVector APlayerCharacter::GetHitTarget() const
{
	if (combat == nullptr)
	{
		return FVector();
	}

	return combat->hitTarget;
}
