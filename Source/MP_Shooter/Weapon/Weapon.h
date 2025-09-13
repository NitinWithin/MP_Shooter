// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Weapon.generated.h"

UENUM(BlueprintType)
enum class EWeaponState :uint8
{
	EWS_Initial UMETA(DisplayName = "Initial State"),
	EWS_Equipped UMETA(DisplayName = "Equipped"),
	EWS_Dropped UMETA(DisplayName = "Dropped"),
	EWS_MAX UMETA(DisplayName = "DefaultMAX"),
};

UCLASS()
class MP_SHOOTER_API AWeapon : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AWeapon();
	void ShowPickUpWidget(bool bShowWidget);
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UFUNCTION()
	virtual void OnSphereOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult
	);

	UFUNCTION()
	virtual void OnSphereEndOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex
	);

private:
	UPROPERTY(VisibleAnywhere, Category = "Weapon Properties")
	USkeletalMeshComponent* WeaponMesh;

	UPROPERTY(VisibleAnywhere, Category = "Weapon Properties")
	class USphereComponent* AreaSphere;

	UPROPERTY(ReplicatedUsing = OnRep_WeaponState,VisibleAnywhere, Category = "Weapon Properties")
	EWeaponState WeaponState;

	UPROPERTY(VisibleAnywhere, Category = "Weapon Properties")
	class UWidgetComponent* PickUpWidget;

	UFUNCTION()
	void OnRep_WeaponState();

	UPROPERTY(EditAnywhere, Category = "Weapon Properties")
	class UAnimationAsset* FireAnimation;

	UPROPERTY(EditAnywhere)
	TSubclassOf<class AAmmoShell> AmmoCasingClass;

	UPROPERTY(EditAnywhere, Category = "Weapon Properties")
	float WeaponCrossHairSpreadDefault;

	UPROPERTY(EditAnywhere, Category = "Weapon Properties")
	float CrossHairShootingSpread;

/*
	Zoomed FOV for weapons while aiming
*/
	UPROPERTY(EditAnywhere, Category = "Weapon Zoom")
	float ZoomFov;
	
	UPROPERTY(EditAnywhere, Category = "Weapon Zoom")
	float ZoomInterpSpeed;

	/*Automotic Fire*/
	UPROPERTY(EditAnywhere, Category = "Combat")
	float fireRateDelay;

	UPROPERTY(EditAnywhere, Category = "Combat")
	bool bAutomaticFire;


public:	
	void SetWeaponState(EWeaponState state);
	FORCEINLINE USphereComponent* GetAreaSphere() const { return AreaSphere; }
	FORCEINLINE USkeletalMeshComponent* GetWeaponMesh() const { return WeaponMesh; }
	FORCEINLINE float GetZoomFOV() const { return ZoomFov; }
	FORCEINLINE float GetZoomInterpSpeed() const { return ZoomInterpSpeed; }
	FORCEINLINE float GetDefaultWeaponSpread() const { return WeaponCrossHairSpreadDefault; }
	FORCEINLINE float GetShootingSpread() const { return CrossHairShootingSpread; }
	FORCEINLINE float GetFireRateDelay() const { return fireRateDelay; }
	FORCEINLINE bool bAutomaticWeaponFire() const { return bAutomaticFire; }


	virtual void Fire(const FVector& HitTarget);

	/*Textures for the weapon crosshairs*/

	UPROPERTY(EditAnywhere, Category = "CrossHair")
	class UTexture2D* CrosshairCenter;

	UPROPERTY(EditAnywhere, Category = "CrossHair")
	UTexture2D* CrosshairTop;

	UPROPERTY(EditAnywhere, Category = "CrossHair")
	UTexture2D* CrosshairBottom;

	UPROPERTY(EditAnywhere, Category = "CrossHair")
	UTexture2D* CrosshairRight;

	UPROPERTY(EditAnywhere, Category = "CrossHair")
	UTexture2D* CrosshairLeft;
};
