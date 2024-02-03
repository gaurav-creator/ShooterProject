#pragma once

#include "Gate.generated.h"
USTRUCT(BlueprintType)
struct FGate
{
	GENERATED_BODY()

public:

	FORCEINLINE FGate();

	explicit FORCEINLINE FGate(bool bStartedClosed);

	FORCEINLINE void Open() {bGateOpen = true;}

	FORCEINLINE void Closed() {bGateOpen = false;}
	
	FORCEINLINE void Toggle() { bGateOpen = !bGateOpen ;}

	FORCEINLINE bool IsOpen() const {return  bGateOpen;}

private:
	UPROPERTY(VisibleAnywhere)
	bool bGateOpen;
};

FORCEINLINE FGate::FGate() : bGateOpen(false)
{}

FORCEINLINE FGate::FGate(const bool bStartedClosed ) : bGateOpen(!bStartedClosed)
{}
