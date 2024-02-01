
#pragma once

#include "CoreMinimal.h"
#include "Projectile.h"
#include "ProjectileBullet.generated.h"

UCLASS()
class SHOOTERPROJECT_API AProjectileBullet : public AProjectile
{
	GENERATED_BODY()

protected:
	virtual void BeginPlay() override;

	virtual void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp,
	                   FVector NormalImpulse, const FHitResult& Hit) override;

public:
	AProjectileBullet();
	virtual void Tick(float DeltaTime) override;
	
};
