// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "InputActionValue.h"
#include "MP_Shooter/MP_ShooterTypes/TurningInPlace.h"
#include "PlayerCharacter.generated.h"

class UInputAction;
class AWeapon;

UCLASS()
class MP_SHOOTER_API APlayerCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	APlayerCharacter();

	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void PostInitializeComponents() override;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
	/*Input*/
	void Move(const FInputActionValue& value);
	void Run(const FInputActionValue& value);
	void StopRun(const FInputActionValue& value);
	void Look(const FInputActionValue& value);
	void Interact(const FInputActionValue& value);
	void CrouchButtonPressed(const FInputActionValue& value);
	void CrouchButtonReleased(const FInputActionValue& value);
	void AimButtonPressed(const FInputActionValue& value);
	void AimButtonReleased(const FInputActionValue& value);
	void Shoot(const FInputActionValue& value);
	/*Input*/

	void AimOffset(float DeltaTime);

private:
	UPROPERTY(VisibleAnywhere, Category = "Camera")
	class USpringArmComponent* CameraBoom;
	UPROPERTY(VisibleAnywhere, Category = "Camera")
	class UCameraComponent* Camera;
											
	
	APlayerController* PlayerController;

	/* INPUT */

	UPROPERTY(EditAnywhere, Category = "Input")
	class UInputMappingContext* InputMappingContext;

	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction* MoveAction;

	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction* RunAction;

	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction* LookAction;

	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction* JumpAction;

	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction* FireAction;

	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction* InteractAction;

	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction* CrouchAction;

	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction* AimAction;

	UPROPERTY(EditAnywhere, Category = "Input")
	float TurnRate = 100;

	/* INPUT */

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	class UWidgetComponent* OverheadWidget;

	UPROPERTY(ReplicatedUsing = OnRep_OverlappingWeapon)
	AWeapon* OverlappingWeapon;

	UFUNCTION()
	void OnRep_OverlappingWeapon(AWeapon* LastWeapon);

	UPROPERTY(VisibleAnywhere)
	class UCombatComponent* Combat;

	UFUNCTION(Server, Reliable)
	void ServerInteractButtonPressed();

	void Running(bool IsRunning);

	UFUNCTION(Server, Reliable)
	void ServerRunning(bool IsRunning);

	UPROPERTY(Replicated)
	bool bIsRunning;

	float AO_Yaw;
	float Interp_AO_Yaw;
	float AO_Pitch;
	FRotator StartAimRotation;

	ETurningInPlace TurningInPlace;
	void TurnInPlace(float DeltaTime);

public:	
	void SetOverlappingWeapon(AWeapon* Weapon);
	bool IsWeaponEquipped();
	bool IsAiming();
	bool IsPlayerRunning();
	FORCEINLINE float GET_AO_YAW() const { return AO_Yaw; };
	FORCEINLINE float GET_AO_Pitch() const { return AO_Pitch; };

	UPROPERTY(EditAnywhere)
	float WalkingSpeed;

	UPROPERTY(EditAnywhere)
	float RunningSpeed;

	AWeapon* GetEquippedWeapon();
	FORCEINLINE ETurningInPlace Get_TurningInPlace() const { return TurningInPlace; };
};
