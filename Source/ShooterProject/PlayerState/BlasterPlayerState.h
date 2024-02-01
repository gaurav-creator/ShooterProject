
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "BlasterPlayerState.generated.h"


UCLASS()
class SHOOTERPROJECT_API ABlasterPlayerState : public APlayerState
{
	GENERATED_BODY()
public:
	virtual void GetLifetimeReplicatedProps(TArray< FLifetimeProperty >& OutLifetimeProps) const override;
	/**
	* Replication notifies
	*/
	virtual void OnRep_Score() override;
	void AddToScore(float ScoreAmount);

	UFUNCTION()
	virtual void OnRep_Defeats();
	void AddToDefeats(int32 DefeatsAmount);

private:
	UPROPERTY()
	class AALSCharacter* Character;
	UPROPERTY()
	class AALSPlayerController* Controller;

	UPROPERTY(ReplicatedUsing = OnRep_Defeats)
	int32 Defeats;
};
