// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "InputActionValue.h"
#include "MP_Shooter/MP_ShooterTypes/TurningInPlace.h"
#include "MP_Shooter/Interfaces/CrosshairInteractionInterface.h"
#include "PlayerCharacter.generated.h"

class UInputAction;
class AWeapon;

UCLASS()
class MP_SHOOTER_API APlayerCharacter : public ACharacter, public ICrosshairInteractionInterface
{
	GENERATED_BODY()

public:
	APlayerCharacter();

	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void PostInitializeComponents() override;

	virtual void OnRep_ReplicatedMovement() override;

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
	void StopShooting(const FInputActionValue& value);
	/*Input*/

	void AimOffset(float DeltaTime);
	void Calculate_AO_Pitch();
	float Calculate_Speed();
	void TurnSimProxies();

	void PlayHitReactMontage();

private:
	UPROPERTY(VisibleAnywhere, Category = "Camera")
	class USpringArmComponent* cameraBoom;
	UPROPERTY(VisibleAnywhere, Category = "Camera")
	class UCameraComponent* camera;
											
	
	APlayerController* PlayerController;

	/* INPUT */

	UPROPERTY(EditAnywhere, Category = "Input")
	class UInputMappingContext* inputMappingContext;

	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction* moveAction;

	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction* runAction;

	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction* lookAction;

	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction* jumpAction;

	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction* fireAction;

	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction* interactAction;

	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction* crouchAction;

	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction* aimAction;

	UPROPERTY(EditAnywhere, Category = "Input")
	float turnRate;

	/* INPUT */

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	class UWidgetComponent*overheadWidget;

	UPROPERTY(ReplicatedUsing = OnRep_OverlappingWeapon)
	AWeapon* overlappingWeapon;

	UFUNCTION()
	void OnRep_OverlappingWeapon(AWeapon* LastWeapon);

	UPROPERTY(VisibleAnywhere)
	class UCombatComponent* combat;

	UFUNCTION(Server, Reliable)
	void ServerInteractButtonPressed();

	void Running(bool IsRunning);

	UFUNCTION(Server, Reliable)
	void ServerRunning(bool IsRunning);

	UPROPERTY(Replicated)
	bool bIsRunning;

	float AO_Yaw;
	float interp_AO_Yaw;
	float AO_Pitch;
	FRotator startAimRotation;

	ETurningInPlace turningInPlace;
	void TurnInPlace(float DeltaTime);

	UPROPERTY(EditAnywhere, Category = "Combat")
	class UAnimMontage* fireWeaponMontage;

	UPROPERTY(EditAnywhere, Category = "Combat")
	UAnimMontage* hitReactMontage;

	void HidePlayerWhenCameraClose();

	UPROPERTY(EditAnywhere, Category = "Camera")
	float cameraThreshold;

	bool bRotateRootBone;
	float turnThreshold;
	FRotator proxyRotationLastFrame;
	FRotator proxyRotation;
	float proxyYaw;
	float timeSinceReplication;

public:	
	void SetOverlappingWeapon(AWeapon* Weapon);
	bool IsWeaponEquipped();
	bool IsAiming();
	bool IsPlayerRunning();
	
	FORCEINLINE float GET_AO_YAW() const { return AO_Yaw; };
	FORCEINLINE float GET_AO_Pitch() const { return AO_Pitch; };
	FORCEINLINE ETurningInPlace Get_TurningInPlace() const { return turningInPlace; };
	FORCEINLINE UCameraComponent* GetFollowCamera() const { return camera; }
	FORCEINLINE bool bShouldRotateRootBone() { return bRotateRootBone; }

	UPROPERTY(EditAnywhere)
	float walkingSpeed;

	UPROPERTY(EditAnywhere)
	float runningSpeed;

	AWeapon* GetEquippedWeapon();

	FVector GetHitTarget() const;

	void PlayFireMontage(bool IsAiming);

	UFUNCTION(NetMulticast, Unreliable)
	void Multicast_HitReact();
};
