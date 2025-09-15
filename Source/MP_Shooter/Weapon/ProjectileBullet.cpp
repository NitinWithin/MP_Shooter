// Fill out your copyright notice in the Description page of Project Settings.


#include "ProjectileBullet.h"
#include <Kismet/GameplayStatics.h>
#include <GameFramework/Character.h>

void AProjectileBullet::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	ACharacter* OwnerCharacter = Cast<ACharacter>(GetOwner());

	if (OwnerCharacter)
	{
		AController* OwnerController = OwnerCharacter->GetController();
		if (OwnerController)
		{
			UGameplayStatics::ApplyDamage(OtherActor, Damage, OwnerController, this, UDamageType::StaticClass());
		}
	}


	//The parent version has Destroy() in it and hence is called after our functionality executes here.
	Super::OnHit(HitComp, OtherActor, OtherComp, NormalImpulse, Hit);
}
